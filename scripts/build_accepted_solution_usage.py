#!/usr/bin/env python3
"""Build token-usage metadata for the published accepted-solution archive.

Run this script inside the Ubuntu WSL distribution as root so it can read
agent session files left by Docker-based Harbor runs.
"""

from __future__ import annotations

import csv
import argparse
import hashlib
import json
import os
import re
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
WORKSPACE_ROOT = REPO_ROOT.parent
MANIFEST_PATH = REPO_ROOT / "site/data/accepted-solutions.json"
OUTPUT_PATH = REPO_ROOT / "site/data/accepted-solution-usage.json"
KIMI_MANIFEST_PATH = (
    WORKSPACE_ROOT
    / "deliverables/AlgoScale_leaderboard_passed_cpp_Opus_Kimi_Terra_20261114/manifest.csv"
)
OLD_ORIGINS_PATH = WORKSPACE_ROOT / "output/uoj_reorder_20261012/problem-origins.csv"
CURRENT_ORIGINS_PATH = WORKSPACE_ROOT / "uoj-local/problem-origins.csv"
DEEPSEEK_DIRECT_SUMMARY = (
    WORKSPACE_ROOT
    / "artifacts/local_uoj_deepseek_flash_direct_128k_20261115/summary.csv"
)
DEEPSEEK_AGENT_JOB = Path(
    "/home/sjw/harbor-deepseek-harness-jobs/"
    "uoj-74-deepseek-flash-deepseek-harness-20261115-135457"
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def trajectory_path(trial_root: Path) -> Path:
    agent_dir = trial_root / "agent"
    for name in (
        "codex.txt",
        "claude-code.txt",
        "kimi-code.txt",
        "deepseek-harness.txt",
    ):
        candidate = agent_dir / name
        if candidate.exists():
            return candidate
    return agent_dir / "claude-code.txt"


def count_agent_rounds(path: Path) -> int:
    """Count model turns using the same rules as the Harbor live monitor."""
    assistant_messages: set[str] = set()
    codex_messages: set[str] = set()
    legacy_rounds = 0
    if not path.exists():
        return 0

    with path.open(encoding="utf-8", errors="replace") as stream:
        for line in stream:
            try:
                event = json.loads(line)
            except json.JSONDecodeError:
                if line.strip() == "dsh: reasoning:":
                    legacy_rounds += 1
                continue
            if not isinstance(event, dict):
                continue

            # Kimi Code emits top-level assistant roles.
            if event.get("role") == "assistant":
                legacy_rounds += 1

            # Codex CLI emits a pair of lifecycle events for each item. Deduplicate
            # by item ID so started/completed count as one model turn.
            if event.get("type") in {"item.started", "item.completed"}:
                item = event.get("item")
                if isinstance(item, dict):
                    item_id = item.get("id")
                    if isinstance(item_id, str) and item.get("type") == "agent_message":
                        codex_messages.add(item_id)

            # Claude Code can emit several stream chunks for one assistant message.
            if event.get("type") == "assistant":
                message = event.get("message")
                if isinstance(message, dict):
                    message_id = message.get("id")
                    if isinstance(message_id, str):
                        assistant_messages.add(message_id)

    if codex_messages:
        return len(codex_messages)
    if assistant_messages:
        return len(assistant_messages)
    return legacy_rounds


def result_usage(result_path: Path) -> dict[str, int] | None:
    try:
        result = json.loads(result_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None

    agent = result.get("agent_result") or {}
    if agent.get("n_input_tokens") is not None and agent.get("n_output_tokens") is not None:
        return {
            "inputTokens": int(agent["n_input_tokens"]),
            "cachedInputTokens": int(agent.get("n_cache_tokens") or 0),
            "outputTokens": int(agent["n_output_tokens"]),
        }

    if result.get("prompt_tokens") is not None and result.get("completion_tokens") is not None:
        return {
            "inputTokens": int(result["prompt_tokens"]),
            "outputTokens": int(result["completion_tokens"]),
        }
    return None


def index_direct_runs() -> dict[str, dict[str, int]]:
    index: dict[str, dict[str, int]] = {}
    artifact_root = WORKSPACE_ROOT / "artifacts"
    for answer in artifact_root.rglob("answer.cpp"):
        usage = result_usage(answer.with_name("result.json"))
        if usage:
            usage.pop("cachedInputTokens", None)
            index[digest(answer)] = usage
    return index


def index_harbor_runs(accepted_hashes: set[str]) -> tuple[
    dict[str, dict[str, int]], dict[str, int], dict[int, dict[str, int]]
]:
    index: dict[str, dict[str, int]] = {}
    rounds_by_digest: dict[str, int] = {}
    deepseek_by_problem: dict[int, dict[str, int]] = {}
    home = Path("/home/sjw")
    skipped = {".cache", "node_modules", ".git", "venv", ".venv"}
    for harbor_root in home.glob("harbor-*"):
        for root, dirs, files in os.walk(harbor_root):
            dirs[:] = [directory for directory in dirs if directory not in skipped]
            root_path = Path(root)
            if "result.json" in files:
                result_path = root_path / "result.json"
                try:
                    result = json.loads(result_path.read_text(encoding="utf-8"))
                    agent = result.get("agent_result") or {}
                    usage = None
                    if agent.get("n_input_tokens") is not None and agent.get("n_output_tokens") is not None:
                        usage = {
                            "inputTokens": int(agent["n_input_tokens"]),
                            "cachedInputTokens": int(agent.get("n_cache_tokens") or 0),
                            "outputTokens": int(agent["n_output_tokens"]),
                        }
                    model_name = str(
                        ((result.get("agent_info") or {}).get("model_info") or {}).get("name")
                        or ""
                    ).lower()
                    reward = (
                        ((result.get("verifier_result") or {}).get("rewards") or {}).get("uoj")
                    )
                    match = re.search(r"uoj-local-(\d+)", str(result.get("task_name") or ""))
                    if usage and "deepseek" in model_name and reward == 1.0 and match:
                        deepseek_by_problem[int(match.group(1))] = usage
                except (OSError, json.JSONDecodeError):
                    pass
            if root_path.parts[-2:] == ("artifacts", "app") and "solution.cpp" in files:
                trial_root = root_path.parent.parent
                solution_path = root_path / "solution.cpp"
                solution_digest = digest(solution_path)
                usage = result_usage(trial_root / "result.json")
                if usage:
                    index[solution_digest] = usage
                if solution_digest in accepted_hashes:
                    rounds = count_agent_rounds(trajectory_path(trial_root))
                    previous = rounds_by_digest.get(solution_digest)
                    if previous is not None and previous != rounds:
                        raise RuntimeError(
                            f"conflicting round counts for {solution_digest}: "
                            f"{previous} and {rounds}"
                        )
                    rounds_by_digest[solution_digest] = rounds
    return index, rounds_by_digest, deepseek_by_problem


def deepseek_harness_usage() -> dict[int, dict[str, int]]:
    """Read per-problem usage emitted by the offline DeepSeek harness run."""
    usage_by_problem: dict[int, dict[str, int]] = {}
    if not DEEPSEEK_AGENT_JOB.is_dir():
        return usage_by_problem
    for trial_root in DEEPSEEK_AGENT_JOB.glob("uoj-local-*__*"):
        match = re.match(r"uoj-local-(\d+)__", trial_root.name)
        if not match:
            continue
        usage_path = trial_root / "agent/deepseek-harness-usage.json"
        try:
            payload = json.loads(usage_path.read_text(encoding="utf-8"))
            totals = payload["record"]["rows"]["tokenUsage"]["val"]["totals"]
        except (OSError, json.JSONDecodeError, KeyError, TypeError):
            continue
        usage_by_problem[int(match.group(1))] = {
            "inputTokens": int(totals.get("uncachedInputTokens") or 0),
            "cachedInputTokens": int(totals.get("cacheReadTokens") or 0),
            "outputTokens": int(totals.get("outputTokens") or 0),
        }
    return usage_by_problem


def kimi_source_records() -> dict[int, str]:
    records: dict[int, str] = {}
    with KIMI_MANIFEST_PATH.open(encoding="utf-8-sig", newline="") as handle:
        for row in csv.DictReader(handle):
            if row["mode"] == "agent" and row["model_key"] == "kimi-k3-1-pro":
                records[int(row["problem_id"])] = row["source_record"]
    return records


def kimi_usage(problem_id: int, records: dict[int, str]) -> dict[str, int] | None:
    source_record = records.get(problem_id)
    if not source_record:
        return None
    trial_root = Path("/home/sjw/harbor-kimi-jobs") / source_record
    wire_files = list(trial_root.glob("agent/.kimi-code/sessions/**/agents/main/wire.jsonl"))
    if not wire_files:
        return None

    input_tokens = 0
    cached_input_tokens = 0
    output_tokens = 0
    records_seen = 0
    for wire_file in wire_files:
        with wire_file.open(encoding="utf-8", errors="replace") as handle:
            for raw_line in handle:
                try:
                    event = json.loads(raw_line)
                except json.JSONDecodeError:
                    continue
                if event.get("type") != "usage.record" or event.get("usageScope") != "turn":
                    continue
                usage = event.get("usage") or {}
                other = int(usage.get("inputOther") or 0)
                cache_read = int(usage.get("inputCacheRead") or 0)
                cache_creation = int(usage.get("inputCacheCreation") or 0)
                input_tokens += other + cache_read + cache_creation
                cached_input_tokens += cache_read
                output_tokens += int(usage.get("output") or 0)
                records_seen += 1
    if not records_seen:
        return None
    return {
        "inputTokens": input_tokens,
        "cachedInputTokens": cached_input_tokens,
        "outputTokens": output_tokens,
    }


def normalize(text: str | None) -> str:
    return re.sub(r"[^\w]+", "", (text or "").casefold())


def current_to_old_problem_ids() -> dict[int, int]:
    with OLD_ORIGINS_PATH.open(encoding="utf-8-sig", newline="") as handle:
        old_rows = list(csv.DictReader(handle))
    with CURRENT_ORIGINS_PATH.open(encoding="utf-8-sig", newline="") as handle:
        current_rows = list(csv.DictReader(handle))

    old_by_url = {
        row["original_url"]: int(row["local_problem_id"])
        for row in old_rows
        if row["original_url"]
    }
    old_by_title = {
        normalize(row["original_title"]): int(row["local_problem_id"])
        for row in old_rows
        if normalize(row["original_title"])
    }
    mapping: dict[int, int] = {}
    for row in current_rows:
        old_id = old_by_url.get(row["original_url"])
        if old_id is None:
            old_id = old_by_title.get(normalize(row["original_title"]))
        if old_id is not None:
            mapping[int(row["local_problem_id"])] = old_id
    return mapping


def deepseek_direct_usage() -> dict[int, dict[str, int]]:
    usage: dict[int, dict[str, int]] = {}
    with DEEPSEEK_DIRECT_SUMMARY.open(encoding="utf-8-sig", newline="") as handle:
        for row in csv.DictReader(handle):
            if row.get("score") and float(row["score"]) == 100:
                usage[int(row["problem_id"])] = {
                    "outputTokens": int(row["completion_tokens"])
                }
    return usage


def existing_usage() -> dict[tuple[str, str, int], dict[str, int]]:
    if not OUTPUT_PATH.exists():
        return {}
    payload = json.loads(OUTPUT_PATH.read_text(encoding="utf-8"))
    return {
        (entry["track"], entry["modelKey"], int(entry["problemId"])): {
            key: int(entry[key])
            for key in ("inputTokens", "cachedInputTokens", "outputTokens", "rounds")
            if entry.get(key) is not None
        }
        for entry in payload.get("entries", [])
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--reuse-existing",
        action="store_true",
        help="Reuse token records for unchanged solutions and refresh DeepSeek Direct only.",
    )
    args = parser.parse_args()
    manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    accepted_agent_hashes = {
        entry["sha256"] for entry in manifest["entries"] if entry["track"] == "agent"
    }
    preserved_usage = existing_usage() if args.reuse_existing else {}
    direct_index = {} if args.reuse_existing else index_direct_runs()
    if args.reuse_existing:
        harbor_index, rounds_by_digest, deepseek_agent_by_old_id = {}, {}, {}
        deepseek_agent_by_current_id = deepseek_harness_usage()
        kimi_records: dict[int, str] = {}
        current_to_old: dict[int, int] = {}
    else:
        harbor_index, rounds_by_digest, deepseek_agent_by_old_id = index_harbor_runs(
            accepted_agent_hashes
        )
        deepseek_agent_by_current_id = deepseek_harness_usage()
        kimi_records = kimi_source_records()
        current_to_old = current_to_old_problem_ids()
    deepseek_direct_by_problem_id = deepseek_direct_usage()
    entries: list[dict[str, Any]] = []

    for entry in manifest["entries"]:
        usage: dict[str, int] | None
        usage_key = (entry["track"], entry["modelKey"], int(entry["problemId"]))
        refresh_deepseek = (
            entry["track"] == "direct" and entry["modelKey"] == "deepseek-flash"
        )
        if args.reuse_existing and not refresh_deepseek:
            usage = preserved_usage.get(usage_key)
        elif entry["track"] == "direct":
            usage = direct_index.get(entry["sha256"])
            if usage is None and entry["modelKey"] == "deepseek-flash":
                usage = deepseek_direct_by_problem_id.get(int(entry["problemId"]))
        else:
            usage = harbor_index.get(entry["sha256"])
            if usage is None and entry["modelKey"] == "kimi-k3-1-pro":
                usage = kimi_usage(int(entry["problemId"]), kimi_records)
            if usage is None and entry["modelKey"] == "deepseek-flash":
                problem_id = int(entry["problemId"])
                usage = deepseek_agent_by_current_id.get(problem_id)
                if usage is None:
                    old_id = current_to_old.get(problem_id)
                    usage = deepseek_agent_by_old_id.get(old_id) if old_id else None
            if usage is not None and entry["sha256"] in rounds_by_digest:
                usage = dict(usage)
                usage["rounds"] = rounds_by_digest[entry["sha256"]]
            elif usage is not None and args.reuse_existing:
                preserved_rounds = (preserved_usage.get(usage_key) or {}).get("rounds")
                if preserved_rounds is not None:
                    usage = dict(usage)
                    usage["rounds"] = preserved_rounds
        if usage is None:
            continue
        record: dict[str, Any] = {
            "track": entry["track"],
            "modelKey": entry["modelKey"],
            "problemId": int(entry["problemId"]),
            "outputTokens": usage["outputTokens"],
        }
        if entry["track"] == "agent":
            record["inputTokens"] = usage["inputTokens"]
            record["cachedInputTokens"] = usage.get("cachedInputTokens", 0)
            if usage.get("rounds") is not None:
                record["rounds"] = usage["rounds"]
        entries.append(record)

    entries.sort(key=lambda item: (item["problemId"], item["track"], item["modelKey"]))
    deepseek_direct_entries = [
        entry for entry in entries
        if entry["track"] == "direct" and entry["modelKey"] == "deepseek-flash"
    ]
    deepseek_agent_entries = [
        entry for entry in entries
        if entry["track"] == "agent" and entry["modelKey"] == "deepseek-flash"
    ]
    assert len(deepseek_direct_entries) == 20
    assert sum(entry["outputTokens"] for entry in deepseek_direct_entries) == 1_155_002
    assert len(deepseek_agent_entries) == 38
    assert sum(entry["inputTokens"] for entry in deepseek_agent_entries) == 911_998
    assert sum(entry["cachedInputTokens"] for entry in deepseek_agent_entries) == 331_736_192
    assert sum(entry["outputTokens"] for entry in deepseek_agent_entries) == 6_523_506
    agent_entries = [entry for entry in entries if entry["track"] == "agent"]
    assert len(agent_entries) == len(accepted_agent_hashes)
    assert all(entry.get("rounds", 0) > 0 for entry in agent_entries)

    payload = {
        "updated": manifest["updated"],
        "covered": len(entries),
        "total": len(manifest["entries"]),
        "entries": entries,
    }
    OUTPUT_PATH.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote token usage for {len(entries)}/{len(manifest['entries'])} accepted solutions.")


if __name__ == "__main__":
    main()
