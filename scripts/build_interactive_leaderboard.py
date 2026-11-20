from __future__ import annotations

import argparse
import json
import tempfile
from collections import defaultdict
from dataclasses import dataclass
from decimal import Decimal, getcontext
from pathlib import Path
from typing import Any


getcontext().prec = 50

VALID_MODES = {"direct", "agent"}
SUCCESS_VERDICTS = {"accepted", "correct", "ok"}


class DataError(ValueError):
    pass


@dataclass(frozen=True, order=True)
class PointKey:
    problem_id: int
    test_id: int


@dataclass(frozen=True, order=True)
class CompetitorKey:
    model_key: str
    mode: str
    harness: str


def require_text(value: Any, field: str) -> str:
    text = str(value or "").strip()
    if not text:
        raise DataError(f"{field} must not be empty")
    return text


def positive_int(value: Any, field: str) -> int:
    if isinstance(value, bool):
        raise DataError(f"{field} must be a positive integer")
    try:
        number = int(value)
    except (TypeError, ValueError) as error:
        raise DataError(f"{field} must be a positive integer") from error
    if number <= 0 or str(number) != str(value).strip():
        raise DataError(f"{field} must be a positive integer")
    return number


def nonnegative_decimal(value: Any, field: str) -> Decimal:
    if isinstance(value, bool):
        raise DataError(f"{field} must be a non-negative number")
    try:
        number = Decimal(str(value))
    except Exception as error:
        raise DataError(f"{field} must be a non-negative number") from error
    if not number.is_finite() or number < 0:
        raise DataError(f"{field} must be a non-negative number")
    return number


def decimal_json(number: Decimal) -> float:
    return float(number.quantize(Decimal("0.000001")))


def competitor_from_submission(submission: dict[str, Any]) -> CompetitorKey:
    mode = require_text(submission.get("mode"), "submission.mode").lower()
    if mode not in VALID_MODES:
        raise DataError(f"submission.mode must be one of {sorted(VALID_MODES)}")
    harness = str(submission.get("harness") or "").strip()
    if mode == "agent" and not harness:
        raise DataError("agent submission.harness must not be empty")
    if mode == "direct":
        harness = ""
    return CompetitorKey(
        model_key=require_text(submission.get("modelKey"), "submission.modelKey"),
        mode=mode,
        harness=harness,
    )


def is_success(point: dict[str, Any]) -> bool:
    verdict = str(point.get("verdict") or "").strip().lower()
    return verdict in SUCCESS_VERDICTS and point.get("protocolValid") is True


def rank_rows(rows: list[dict[str, Any]]) -> None:
    rows.sort(
        key=lambda row: (
            -Decimal(row["_score"]),
            row["model"].casefold(),
            row["mode"],
            row.get("harness") or "",
        )
    )
    previous_score: Decimal | None = None
    previous_rank = 0
    for index, row in enumerate(rows, start=1):
        score = Decimal(row.pop("_score"))
        if previous_score is None or score != previous_score:
            previous_rank = index
            previous_score = score
        row["rank"] = previous_rank


def build(raw: dict[str, Any]) -> dict[str, Any]:
    if raw.get("schemaVersion") != 1:
        raise DataError("schemaVersion must be 1")

    point_config: dict[PointKey, dict[str, Any]] = {}
    problem_points: dict[int, list[PointKey]] = defaultdict(list)
    problem_versions: dict[int, str] = {}
    problem_titles: dict[int, str] = {}

    for problem in raw.get("problems", []):
        problem_id = positive_int(problem.get("problemId"), "problem.problemId")
        if problem_id in problem_versions:
            raise DataError(f"duplicate problemId: {problem_id}")
        data_version = require_text(problem.get("dataVersion"), "problem.dataVersion")
        problem_versions[problem_id] = data_version
        problem_titles[problem_id] = require_text(problem.get("title"), "problem.title")
        points = problem.get("points")
        if not isinstance(points, list) or not points:
            raise DataError(f"problem #{problem_id} must define at least one point")
        for point in points:
            test_id = positive_int(point.get("testId"), "point.testId")
            key = PointKey(problem_id, test_id)
            if key in point_config:
                raise DataError(f"duplicate point: problem #{problem_id}, test #{test_id}")
            threshold = nonnegative_decimal(point.get("threshold"), "point.threshold")
            if threshold <= 0:
                raise DataError("point.threshold must be greater than zero")
            point_config[key] = {
                "threshold": threshold,
                "metric": require_text(point.get("metric") or "interactions", "point.metric"),
            }
            problem_points[problem_id].append(key)
        problem_points[problem_id].sort()

    submissions = raw.get("submissions", [])
    if not isinstance(submissions, list):
        raise DataError("submissions must be an array")

    names: dict[CompetitorKey, str] = {}
    best: dict[CompetitorKey, dict[PointKey, tuple[Decimal, str]]] = defaultdict(dict)
    seen_submission_ids: set[str] = set()

    for submission in submissions:
        submission_id = require_text(submission.get("submissionId"), "submission.submissionId")
        if submission_id in seen_submission_ids:
            raise DataError(f"duplicate submissionId: {submission_id}")
        seen_submission_ids.add(submission_id)
        competitor = competitor_from_submission(submission)
        model = require_text(submission.get("model"), "submission.model")
        if competitor in names and names[competitor] != model:
            raise DataError(f"inconsistent model name for {competitor.model_key}")
        names[competitor] = model

        problem_id = positive_int(submission.get("problemId"), "submission.problemId")
        if problem_id not in problem_versions:
            raise DataError(f"submission references unknown problem #{problem_id}")
        if submission.get("dataVersion") != problem_versions[problem_id]:
            raise DataError(f"submission {submission_id} uses a stale dataVersion")

        result_points = submission.get("points")
        if not isinstance(result_points, list):
            raise DataError(f"submission {submission_id}.points must be an array")
        seen_tests: set[int] = set()
        for result in result_points:
            test_id = positive_int(result.get("testId"), "result.testId")
            if test_id in seen_tests:
                raise DataError(f"submission {submission_id} repeats test #{test_id}")
            seen_tests.add(test_id)
            point_key = PointKey(problem_id, test_id)
            if point_key not in point_config:
                raise DataError(f"submission {submission_id} references unknown test #{test_id}")
            if not is_success(result):
                continue
            interaction_count = nonnegative_decimal(
                result.get("interactionCount"), "result.interactionCount"
            )
            current = best[competitor].get(point_key)
            if current is None or interaction_count < current[0]:
                best[competitor][point_key] = (interaction_count, submission_id)

    all_competitors = sorted(names)
    scopes = {
        "direct": [key for key in all_competitors if key.mode == "direct"],
        "agent": [key for key in all_competitors if key.mode == "agent"],
        "combined": all_competitors,
    }

    boards: dict[str, Any] = {}
    total_problem_count = len(problem_points)
    total_point_count = len(point_config)

    for board_name, competitors in scopes.items():
        records: dict[PointKey, Decimal] = {}
        for point_key, config in point_config.items():
            threshold = config["threshold"]
            eligible = [
                best[key][point_key][0]
                for key in competitors
                if point_key in best[key] and best[key][point_key][0] < threshold
            ]
            if eligible:
                records[point_key] = min(eligible)

        rows: list[dict[str, Any]] = []
        for competitor in competitors:
            per_problem: list[dict[str, Any]] = []
            overall_problem_scores: list[Decimal] = []
            record_count = 0
            valid_count = 0

            for problem_id in sorted(problem_points):
                point_rows: list[dict[str, Any]] = []
                point_scores: list[Decimal] = []
                for point_key in problem_points[problem_id]:
                    config = point_config[point_key]
                    threshold: Decimal = config["threshold"]
                    record = records.get(point_key)
                    result = best[competitor].get(point_key)
                    score = Decimal(0)
                    valid = False
                    interaction_count: Decimal | None = None
                    source_submission: str | None = None

                    if result is not None:
                        interaction_count, source_submission = result
                        if interaction_count < threshold and record is not None:
                            valid = True
                            valid_count += 1
                            if interaction_count == record:
                                score = Decimal(100)
                                record_count += 1
                            else:
                                score = Decimal(100) * (
                                    (threshold - interaction_count) / (threshold - record)
                                ) ** 2

                    point_scores.append(score)
                    point_rows.append(
                        {
                            "testId": point_key.test_id,
                            "threshold": decimal_json(threshold),
                            "record": decimal_json(record) if record is not None else None,
                            "interactionCount": (
                                decimal_json(interaction_count)
                                if interaction_count is not None
                                else None
                            ),
                            "score": decimal_json(score),
                            "valid": valid,
                            "sourceSubmissionId": source_submission,
                            "metric": config["metric"],
                        }
                    )

                problem_score = sum(point_scores, Decimal(0)) / Decimal(len(point_scores))
                overall_problem_scores.append(problem_score)
                per_problem.append(
                    {
                        "problemId": problem_id,
                        "title": problem_titles[problem_id],
                        "score": decimal_json(problem_score),
                        "points": point_rows,
                    }
                )

            overall_score = (
                sum(overall_problem_scores, Decimal(0)) / Decimal(total_problem_count)
                if total_problem_count
                else Decimal(0)
            )
            rows.append(
                {
                    "model": names[competitor],
                    "modelKey": competitor.model_key,
                    "mode": competitor.mode,
                    "harness": competitor.harness or None,
                    "score": decimal_json(overall_score),
                    "records": record_count,
                    "validPoints": valid_count,
                    "totalPoints": total_point_count,
                    "problemScores": per_problem,
                    "_score": str(overall_score),
                }
            )

        rank_rows(rows)
        boards[board_name] = {
            "problemCount": total_problem_count,
            "pointCount": total_point_count,
            "rows": rows,
        }

    return {
        "schemaVersion": 1,
        "updated": require_text(raw.get("updated"), "updated"),
        "status": require_text(raw.get("status") or "Preliminary", "status"),
        "boards": boards,
    }


def atomic_write_json(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", newline="", delete=False, dir=path.parent
    ) as output:
        json.dump(payload, output, ensure_ascii=False, indent=2)
        output.write("\n")
        temporary = Path(output.name)
    temporary.replace(path)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build Direct, Agent, and Combined interactive leaderboards"
    )
    parser.add_argument(
        "--input",
        type=Path,
        default=Path("site/data/interactive-results.json"),
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("site/data/interactive-leaderboard.json"),
    )
    args = parser.parse_args()
    raw = json.loads(args.input.read_text(encoding="utf-8"))
    result = build(raw)
    atomic_write_json(args.output, result)
    print(
        f"interactive leaderboard: {result['boards']['combined']['problemCount']} problems, "
        f"{result['boards']['combined']['pointCount']} points, "
        f"{len(result['boards']['combined']['rows'])} competitors"
    )


if __name__ == "__main__":
    main()
