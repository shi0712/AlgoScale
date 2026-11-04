#!/usr/bin/env python3
"""Publish translated original statements into the static AlgoScale site."""

from __future__ import annotations

import argparse
import json
import re
import zipfile
from pathlib import Path


ENTRY_RE = re.compile(r"^AlgoScale_English/problems/(\d+)\.md$")


def parse_args() -> argparse.Namespace:
    repository = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser()
    parser.add_argument("archive", type=Path)
    parser.add_argument(
        "--catalog",
        type=Path,
        default=repository / "site/data/problems.json",
    )
    parser.add_argument(
        "--reference-report",
        type=Path,
        default=repository / "site/data/reference-assets.json",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=repository / "site/references",
    )
    return parser.parse_args()


def normalized_markdown(raw: bytes, entry: str) -> str:
    try:
        text = raw.decode("utf-8-sig")
    except UnicodeDecodeError as exc:
        raise ValueError(f"statement is not UTF-8: {entry}") from exc
    text = text.replace("\r\n", "\n").replace("\r", "\n")
    return "\n".join(line.rstrip() for line in text.splitlines()).rstrip() + "\n"


def write_json(path: Path, value: object) -> None:
    path.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def main() -> None:
    args = parse_args()
    catalog = json.loads(args.catalog.read_text(encoding="utf-8"))
    report = json.loads(args.reference_report.read_text(encoding="utf-8"))
    problems = {int(item["id"]): item for item in catalog}
    report_items = {int(item["id"]): item for item in report}

    with zipfile.ZipFile(args.archive) as archive:
        entries: dict[int, str] = {}
        for info in archive.infolist():
            match = ENTRY_RE.fullmatch(info.filename)
            if not match or info.is_dir():
                continue
            problem_id = int(match.group(1))
            if problem_id in entries:
                raise ValueError(f"duplicate statement for problem #{problem_id}")
            entries[problem_id] = info.filename

        if len(entries) != 79:
            raise ValueError(f"expected 79 statements, found {len(entries)}")
        unknown = sorted(set(entries) - set(problems))
        if unknown:
            raise ValueError(f"statement IDs are missing from the catalog: {unknown}")

        published_ids = set(entries)
        for problem_id, problem in problems.items():
            assets = problem.setdefault("referenceAssets", {})
            if problem_id not in published_ids:
                assets.pop("originalStatement", None)

        for problem_id, entry in sorted(entries.items()):
            destination = args.output / f"{problem_id:03d}" / "original-statement.md"
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_text(
                normalized_markdown(archive.read(entry), entry),
                encoding="utf-8",
                newline="\n",
            )
            href = f"./references/{problem_id:03d}/original-statement.md"
            problems[problem_id].setdefault("referenceAssets", {})[
                "originalStatement"
            ] = href

            report_item = report_items.get(problem_id)
            if report_item is not None:
                report_item.setdefault("assets", {})["originalStatement"] = href
                report_item.setdefault("sourceEntries", {})[
                    "originalStatement"
                ] = entry
                report_item["statementSourceArchive"] = args.archive.name

    write_json(args.catalog, catalog)
    write_json(args.reference_report, report)
    print(f"published {len(entries)} original statements from {args.archive.name}")


if __name__ == "__main__":
    main()
