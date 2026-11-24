from __future__ import annotations

import argparse
import json
import re
import tempfile
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path
from typing import Any


METRIC_PREFIX = "ALGOSCALE_METRIC_V1"
PAIR_PATTERN = re.compile(r"([A-Za-z_]+)=([^\s]+)")


class ImportError(ValueError):
    pass


@dataclass(frozen=True)
class SourceSpec:
    model_key: str
    model: str
    mode: str
    harness: str
    directory: Path


def parse_source_spec(value: str) -> SourceSpec:
    parts = value.split("|", 4)
    if len(parts) != 5:
        raise argparse.ArgumentTypeError(
            "source must be MODEL_KEY|MODEL|MODE|HARNESS|DIRECTORY"
        )
    model_key, model, mode, harness, directory = (part.strip() for part in parts)
    if not model_key or not model or not directory:
        raise argparse.ArgumentTypeError("model key, model, and directory are required")
    mode = mode.lower()
    if mode not in {"direct", "agent"}:
        raise argparse.ArgumentTypeError("mode must be direct or agent")
    if mode == "agent" and not harness:
        raise argparse.ArgumentTypeError("agent sources require a harness")
    if mode == "direct":
        harness = ""
    return SourceSpec(model_key, model, mode, harness, Path(directory))


def atomic_write_json(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", newline="", delete=False, dir=path.parent
    ) as output:
        json.dump(payload, output, ensure_ascii=False, indent=2)
        output.write("\n")
        temporary = Path(output.name)
    temporary.replace(path)


def parse_metric(text: str) -> dict[str, str]:
    marker = text.find(METRIC_PREFIX)
    if marker < 0:
        raise ImportError(f"missing {METRIC_PREFIX}: {text.strip()}")
    return dict(PAIR_PATTERN.findall(text[marker + len(METRIC_PREFIX) :]))


def point_from_test(test: ET.Element) -> dict[str, Any] | None:
    test_id = int(test.attrib["num"])
    if test_id <= 0:
        return None
    result_text = " ".join((test.findtext("res") or "").split())
    if METRIC_PREFIX not in result_text:
        return {
            "testId": test_id,
            "verdict": (test.attrib.get("info") or "failed").strip().lower(),
            "protocolValid": False,
            "interactionCount": 0,
        }
    metric = parse_metric(result_text)
    if "value" not in metric:
        raise ImportError(f"test #{test_id} has no interaction value")
    status = metric.get("status", "").lower()
    accepted = status == "ok"
    return {
        "testId": test_id,
        "verdict": "accepted" if accepted else status or "failed",
        "protocolValid": accepted,
        "interactionCount": int(metric["value"]),
    }


def _integer_field(payload: dict[str, Any], *names: str) -> int | None:
    """Return the first non-negative integer usage field, if present."""
    for name in names:
        value = payload.get(name)
        if isinstance(value, bool):
            continue
        try:
            number = int(value)
        except (TypeError, ValueError):
            continue
        if number >= 0:
            return number
    return None


def usage_from_result(judgement_path: Path, mode: str) -> dict[str, int]:
    """Read non-sensitive token counters written beside a UOJ judgement.

    Direct submissions intentionally publish only completion/output tokens. Agent
    submissions may additionally publish prompt, cached-input, and round counts.
    Missing counters stay absent instead of being guessed.
    """
    result_path = judgement_path.with_name("result.json")
    if not result_path.is_file():
        return {}
    try:
        result = json.loads(result_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}
    if not isinstance(result, dict):
        return {}
    usage = result.get("usage") if isinstance(result.get("usage"), dict) else {}
    payload = {**usage, **result}
    metrics: dict[str, int] = {}
    output = _integer_field(payload, "completion_tokens", "output_tokens", "outputTokens")
    if output is not None:
        metrics["outputTokens"] = output
    if mode == "agent":
        input_tokens = _integer_field(payload, "prompt_tokens", "input_tokens", "inputTokens")
        cached = _integer_field(payload, "cached_tokens", "cached_input_tokens", "cachedInputTokens")
        rounds = _integer_field(payload, "rounds", "agent_rounds", "interaction_rounds")
        if input_tokens is not None:
            metrics["inputTokens"] = input_tokens
        if cached is not None:
            metrics["cachedInputTokens"] = cached
        if rounds is not None:
            metrics["rounds"] = rounds
    return metrics


def usage_from_harbor_trial(judgement_path: Path) -> dict[str, int]:
    """Read Claude Code usage counters from the enclosing Harbor trial."""
    trial_dir = judgement_path.parent.parent
    result_path = trial_dir / "result.json"
    if not result_path.is_file():
        return {}
    try:
        result = json.loads(result_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}
    agent = result.get("agent_result")
    if not isinstance(agent, dict):
        return {}
    metrics: dict[str, int] = {}
    for key, names in {
        "inputTokens": ("n_input_tokens",),
        "cachedInputTokens": ("n_cache_tokens",),
        "outputTokens": ("n_output_tokens",),
    }.items():
        value = _integer_field(agent, *names)
        if value is not None:
            metrics[key] = value

    log_path = trial_dir / "agent" / "claude-code.txt"
    message_ids: set[str] = set()
    anonymous_messages = 0
    if log_path.is_file():
        try:
            with log_path.open(encoding="utf-8", errors="replace") as stream:
                for line in stream:
                    try:
                        event = json.loads(line)
                    except json.JSONDecodeError:
                        continue
                    message = event.get("message")
                    if not isinstance(message, dict) or message.get("role") != "assistant":
                        continue
                    message_id = message.get("id")
                    if isinstance(message_id, str) and message_id:
                        message_ids.add(message_id)
                    else:
                        anonymous_messages += 1
        except OSError:
            pass
    rounds = len(message_ids) + anonymous_messages
    if rounds > 0:
        metrics["rounds"] = rounds
    return metrics


def load_submission(
    judgement_path: Path,
    source: SourceSpec,
    problem_versions: dict[int, str],
    accepted_only: bool,
) -> dict[str, Any] | None:
    judgement = json.loads(judgement_path.read_text(encoding="utf-8"))
    if accepted_only:
        try:
            overall_score = float(judgement.get("result", {}).get("score"))
        except (TypeError, ValueError):
            return None
        if overall_score != 100.0:
            return None
    submission_id = str(judgement["id"])
    problem_id = int(judgement["problem_id"])
    if problem_id not in problem_versions:
        raise ImportError(
            f"{judgement_path}: problem #{problem_id} is not in interactive-results.json"
        )
    details = judgement.get("result", {}).get("details") or ""
    if METRIC_PREFIX not in details:
        return None
    try:
        tests = ET.fromstring(details)
    except ET.ParseError as error:
        raise ImportError(f"{judgement_path}: invalid test details XML") from error
    points = [
        point
        for test in tests.findall("test")
        if (point := point_from_test(test)) is not None
    ]
    if not points:
        raise ImportError(f"{judgement_path}: no test results")
    submission: dict[str, Any] = {
        "submissionId": submission_id,
        "problemId": problem_id,
        "dataVersion": problem_versions[problem_id],
        "modelKey": source.model_key,
        "model": source.model,
        "mode": source.mode,
        "points": points,
    }
    if source.mode == "agent":
        submission["harness"] = source.harness
    submission.update(usage_from_result(judgement_path, source.mode))
    if source.mode == "agent":
        submission.update(usage_from_harbor_trial(judgement_path))
    return submission


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Import point-level UOJ interactive judgements into AlgoScale"
    )
    parser.add_argument(
        "--input",
        type=Path,
        default=Path("site/data/interactive-results.json"),
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("site/data/interactive-results.json"),
    )
    parser.add_argument(
        "--source",
        action="append",
        type=parse_source_spec,
        required=True,
        help="MODEL_KEY|MODEL|MODE|HARNESS|DIRECTORY; may be repeated",
    )
    parser.add_argument(
        "--accepted-only-model-key",
        action="append",
        default=[],
        help="only import UOJ score-100 submissions for this model key",
    )
    args = parser.parse_args()

    payload = json.loads(args.input.read_text(encoding="utf-8"))
    problem_versions = {
        int(problem["problemId"]): str(problem["dataVersion"])
        for problem in payload["problems"]
    }

    source_competitors = {
        (source.model_key, source.mode, source.harness) for source in args.source
    }
    accepted_only_model_keys = set(args.accepted_only_model_key)
    imported: dict[str, dict[str, Any]] = {}
    for source in args.source:
        if not source.directory.is_dir():
            raise ImportError(f"artifact directory does not exist: {source.directory}")
        for judgement_path in sorted(source.directory.rglob("judgement.json")):
            submission = load_submission(
                judgement_path,
                source,
                problem_versions,
                source.model_key in accepted_only_model_keys,
            )
            if submission is None:
                continue
            submission_id = submission["submissionId"]
            previous = imported.get(submission_id)
            if previous is not None and previous != submission:
                raise ImportError(f"conflicting submission ID: {submission_id}")
            imported[submission_id] = submission

    preserved: list[dict[str, Any]] = []
    for submission in payload.get("submissions", []):
        mode = str(submission.get("mode") or "").lower()
        harness = str(submission.get("harness") or "") if mode == "agent" else ""
        competitor = (str(submission.get("modelKey") or ""), mode, harness)
        if competitor not in source_competitors:
            preserved.append(submission)

    payload["submissions"] = sorted(
        [*preserved, *imported.values()],
        key=lambda item: (int(item["problemId"]), int(item["submissionId"])),
    )
    payload["status"] = "Preliminary"
    atomic_write_json(args.output, payload)
    print(
        f"imported {len(imported)} submissions from {len(args.source)} artifact "
        f"directories; preserved {len(preserved)} unrelated submissions"
    )


if __name__ == "__main__":
    main()
