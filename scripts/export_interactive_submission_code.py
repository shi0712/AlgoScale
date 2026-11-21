#!/usr/bin/env python3
"""Publish the exact C++ source for every imported Interactive submission."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from export_generated_cpp import read_submitted_cpp, submission_storage_paths


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Export every Interactive submission's answer.code from local UOJ"
    )
    parser.add_argument(
        "--detail-dir",
        type=Path,
        default=Path("site/data/interactive/problems"),
        help="Generated problem-detail directory; only currently displayed sources are exported",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("site/generated/interactive/submissions"),
    )
    parser.add_argument("--wsl-distribution", default="Ubuntu")
    args = parser.parse_args()

    submission_ids: set[int] = set()
    for detail_path in sorted(args.detail_dir.glob("*.json")):
        payload = json.loads(detail_path.read_text(encoding="utf-8"))
        for submission in payload.get("submissions", []):
            raw_id = str(submission.get("submissionId") or "").strip()
            if not raw_id.isdigit() or int(raw_id) <= 0:
                raise ValueError(
                    f"submissionId must be a positive UOJ integer: {raw_id!r}"
                )
            submission_ids.add(int(raw_id))

    storage_paths = submission_storage_paths(args.wsl_distribution, submission_ids)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    expected = {f"{submission_id}.cpp" for submission_id in submission_ids}
    for stale in args.output_dir.glob("*.cpp"):
        if stale.name not in expected:
            stale.unlink()

    for index, submission_id in enumerate(sorted(submission_ids), start=1):
        source = read_submitted_cpp(
            args.wsl_distribution, storage_paths[submission_id]
        )
        (args.output_dir / f"{submission_id}.cpp").write_bytes(source)
        if index % 25 == 0 or index == len(submission_ids):
            print(f"exported {index}/{len(submission_ids)} submissions")

    print(
        f"published {len(submission_ids)} Interactive C++ submissions to "
        f"{args.output_dir}"
    )


if __name__ == "__main__":
    main()
