#!/usr/bin/env python3
"""Export exactly the C++ sources submitted by benchmark runs to the public site."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path


TRIAL_NAME = re.compile(r"^uoj-local-(\d+)__")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--direct-summary", type=Path, required=True)
    parser.add_argument(
        "--agent-job",
        type=Path,
        action="append",
        default=[],
        help="Harbor job directory; later jobs replace earlier trials for the same problem",
    )
    parser.add_argument("--catalog", type=Path, required=True)
    parser.add_argument("--output-root", type=Path, required=True)
    parser.add_argument("--wsl-distribution", default="Ubuntu")
    parser.add_argument("--model-slug", default="deepseek-flash")
    return parser.parse_args()


def load_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8"))


def direct_submissions(path: Path) -> dict[int, int]:
    rows = load_json(path)
    return {
        int(row["problem_id"]): int(row["submission_id"])
        for row in rows
        if row.get("submission_id") is not None
    }


def agent_submissions(job_paths: list[Path]) -> dict[int, int]:
    submissions: dict[int, int] = {}
    for job_path in job_paths:
        if not job_path.is_dir():
            raise FileNotFoundError(f"Harbor job directory not found: {job_path}")
        for trial in sorted(job_path.iterdir()):
            match = TRIAL_NAME.match(trial.name)
            result_path = trial / "verifier" / "result.json"
            if match is None or not result_path.is_file():
                continue
            result = load_json(result_path)
            submission_id = result.get("submission_id")
            if submission_id is not None:
                submissions[int(match.group(1))] = int(submission_id)
    return submissions


def wsl_command(distribution: str, *command: str, binary: bool = False):
    completed = subprocess.run(
        ["wsl", "-d", distribution, "--", *command],
        check=True,
        capture_output=True,
        text=not binary,
        encoding=None if binary else "utf-8",
        errors=None if binary else "replace",
    )
    return completed.stdout


def submission_storage_paths(
    distribution: str, submission_ids: set[int]
) -> dict[int, str]:
    if not submission_ids:
        return {}
    id_list = ",".join(str(value) for value in sorted(submission_ids))
    query = f"SELECT id,content FROM submissions WHERE id IN ({id_list});"
    output = wsl_command(
        distribution,
        "docker",
        "exec",
        "uoj-db",
        "mysql",
        "-uroot",
        "-proot",
        "app_uoj233",
        "-N",
        "-B",
        "-e",
        query,
    )
    paths: dict[int, str] = {}
    for line in output.splitlines():
        submission_id, content_json = line.split("\t", 1)
        content = json.loads(content_json)
        paths[int(submission_id)] = str(content["file_name"])
    missing = submission_ids - paths.keys()
    if missing:
        raise RuntimeError(f"UOJ storage metadata missing for submissions: {sorted(missing)}")
    return paths


def read_submitted_cpp(distribution: str, storage_path: str) -> bytes:
    archive = f"/opt/uoj/web/app/storage{storage_path}"
    source = wsl_command(
        distribution,
        "docker",
        "exec",
        "uoj-web",
        "unzip",
        "-p",
        archive,
        "answer.code",
        binary=True,
    )
    if not source.strip():
        raise RuntimeError(f"empty answer.code in {storage_path}")
    return source


def export_track(
    *,
    track: str,
    model_slug: str,
    submissions: dict[int, int],
    storage_paths: dict[int, str],
    distribution: str,
    output_root: Path,
) -> dict[int, str]:
    destination = output_root / track / model_slug
    destination.mkdir(parents=True, exist_ok=True)
    public_paths: dict[int, str] = {}
    for problem_id, submission_id in sorted(submissions.items()):
        source = read_submitted_cpp(distribution, storage_paths[submission_id])
        filename = f"{problem_id:03d}.cpp"
        (destination / filename).write_bytes(source)
        public_paths[problem_id] = f"./generated/{track}/{model_slug}/{filename}"
    return public_paths


def update_catalog(
    path: Path, direct_paths: dict[int, str], agent_paths: dict[int, str]
) -> None:
    catalog = load_json(path)
    for problem in catalog:
        problem_id = int(problem["id"])
        code = {}
        if problem_id in direct_paths:
            code["direct"] = direct_paths[problem_id]
        if problem_id in agent_paths:
            code["agent"] = agent_paths[problem_id]
        problem["generatedCode"] = code
    path.write_text(
        json.dumps(catalog, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )


def main() -> None:
    args = parse_args()
    direct = direct_submissions(args.direct_summary)
    agent = agent_submissions(args.agent_job)
    storage_paths = submission_storage_paths(
        args.wsl_distribution, set(direct.values()) | set(agent.values())
    )
    direct_paths = export_track(
        track="direct",
        model_slug=args.model_slug,
        submissions=direct,
        storage_paths=storage_paths,
        distribution=args.wsl_distribution,
        output_root=args.output_root,
    )
    agent_paths = export_track(
        track="agent",
        model_slug=args.model_slug,
        submissions=agent,
        storage_paths=storage_paths,
        distribution=args.wsl_distribution,
        output_root=args.output_root,
    )
    update_catalog(args.catalog, direct_paths, agent_paths)
    print(f"exported {len(direct_paths)} direct and {len(agent_paths)} agent C++ files")


if __name__ == "__main__":
    main()
