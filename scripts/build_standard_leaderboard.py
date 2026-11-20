from __future__ import annotations

import argparse
import json
import tempfile
from pathlib import Path
from typing import Any


def build(problems: list[dict[str, Any]], leaderboard: dict[str, Any]) -> dict[str, Any]:
    standard_ids = {
        int(problem["id"])
        for problem in problems
        if problem.get("track") == "Standard"
    }
    if not standard_ids:
        raise ValueError("problem catalog contains no Standard problems")

    for track in ("direct", "agent"):
        entries = leaderboard.get("tracks", {}).get(track)
        if not isinstance(entries, list):
            raise ValueError(f"leaderboard track {track!r} must be an array")
        for entry in entries:
            raw_ids = entry.get("solvedProblemIds")
            if not isinstance(raw_ids, list):
                raise ValueError(f"{track}/{entry.get('model')} has no solvedProblemIds")
            solved_ids = sorted({int(problem_id) for problem_id in raw_ids} & standard_ids)
            entry["solvedProblemIds"] = solved_ids
            entry["solved"] = len(solved_ids)
            entry["total"] = len(standard_ids)
            entry["passRate"] = round(100 * len(solved_ids) / len(standard_ids), 1)
    return leaderboard


def atomic_write(path: Path, payload: dict[str, Any]) -> None:
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", newline="", delete=False, dir=path.parent
    ) as output:
        json.dump(payload, output, ensure_ascii=False, indent=2)
        output.write("\n")
        temporary = Path(output.name)
    temporary.replace(path)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Remove Interactive tasks from the Standard leaderboard"
    )
    parser.add_argument("--problems", type=Path, default=Path("site/data/problems.json"))
    parser.add_argument(
        "--leaderboard", type=Path, default=Path("site/data/leaderboard.json")
    )
    args = parser.parse_args()
    problems = json.loads(args.problems.read_text(encoding="utf-8"))
    leaderboard = json.loads(args.leaderboard.read_text(encoding="utf-8"))
    result = build(problems, leaderboard)
    atomic_write(args.leaderboard, result)
    total = result["tracks"]["direct"][0]["total"] if result["tracks"]["direct"] else 0
    print(f"standard leaderboard: {total} problems")


if __name__ == "__main__":
    main()
