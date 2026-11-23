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


def submission_source_url(submission_id: str) -> str:
    """Return the deterministic public path for one submitted C++ program."""
    return f"./generated/interactive/submissions/{submission_id}.cpp"


def copy_usage_fields(source: dict[str, Any], target: dict[str, Any]) -> None:
    """Copy optional, non-sensitive usage counters into published metadata."""
    for field in ("inputTokens", "cachedInputTokens", "outputTokens", "rounds"):
        if field in source:
            target[field] = source[field]


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
    selected: dict[
        tuple[CompetitorKey, int],
        tuple[
            tuple[int, int, Decimal, int],
            str,
            dict[PointKey, Decimal],
        ],
    ] = {}
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
        successful_results: dict[PointKey, Decimal] = {}
        valid_scoring_points = 0
        successful_points = 0
        successful_interactions = Decimal(0)
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
            successful_results[point_key] = interaction_count
            successful_points += 1
            successful_interactions += interaction_count
            if interaction_count < point_config[point_key]["threshold"]:
                valid_scoring_points += 1

        numeric_id = int(submission_id) if submission_id.isdigit() else 0
        quality = (
            valid_scoring_points,
            successful_points,
            -successful_interactions,
            numeric_id,
        )
        candidate_key = (competitor, problem_id)
        current = selected.get(candidate_key)
        if current is None or quality > current[0]:
            selected[candidate_key] = (
                quality,
                submission_id,
                successful_results,
            )

    # A leaderboard row represents one concrete program per problem. Repeated runs
    # may replace that program as a whole, but their per-point results are never
    # combined into a synthetic submission.
    for (competitor, _problem_id), (_quality, submission_id, results) in selected.items():
        for point_key, interaction_count in results.items():
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


def compact_leaderboard(result: dict[str, Any]) -> dict[str, Any]:
    """Keep the homepage payload small; point data is published per problem."""
    boards: dict[str, Any] = {}
    for scope, board in result["boards"].items():
        boards[scope] = {
            "problemCount": board["problemCount"],
            "pointCount": board["pointCount"],
            "rows": [
                {key: value for key, value in row.items() if key != "problemScores"}
                for row in board["rows"]
            ],
        }
    return {
        "schemaVersion": result["schemaVersion"],
        "updated": result["updated"],
        "status": result["status"],
        "boards": boards,
    }


def build_problem_details(
    raw: dict[str, Any], result: dict[str, Any]
) -> dict[int, dict[str, Any]]:
    """Build one lazy-loadable point-result payload for each Interactive task."""
    raw_problems = {
        positive_int(problem.get("problemId"), "problem.problemId"): problem
        for problem in raw.get("problems", [])
    }
    details: dict[int, dict[str, Any]] = {}

    for problem_id, problem in sorted(raw_problems.items()):
        raw_points = {
            positive_int(point.get("testId"), "point.testId"): point
            for point in problem["points"]
        }
        scopes: dict[str, Any] = {}

        for scope, board in result["boards"].items():
            competitors: list[dict[str, Any]] = []
            per_competitor_points: list[tuple[dict[str, Any], dict[int, dict[str, Any]]]] = []

            for row in board["rows"]:
                problem_score = next(
                    item for item in row["problemScores"]
                    if item["problemId"] == problem_id
                )
                competitor = {
                    "model": row["model"],
                    "modelKey": row["modelKey"],
                    "mode": row["mode"],
                    "harness": row.get("harness"),
                    "problemScore": problem_score["score"],
                }
                competitors.append(competitor)
                per_competitor_points.append(
                    (competitor, {point["testId"]: point for point in problem_score["points"]})
                )

            point_rows: list[dict[str, Any]] = []
            for test_id, raw_point in sorted(raw_points.items()):
                results: list[dict[str, Any]] = []
                threshold: float | None = None
                record: float | None = None
                metric = require_text(raw_point.get("metric") or "interactions", "point.metric")

                for competitor, point_map in per_competitor_points:
                    point = point_map[test_id]
                    threshold = point["threshold"]
                    if point["record"] is not None:
                        record = point["record"]
                    is_record = (
                        point["valid"]
                        and point["record"] is not None
                        and point["interactionCount"] == point["record"]
                    )
                    results.append(
                        {
                            "model": competitor["model"],
                            "modelKey": competitor["modelKey"],
                            "mode": competitor["mode"],
                            "harness": competitor["harness"],
                            "interactionCount": point["interactionCount"],
                            "score": point["score"],
                            "valid": point["valid"],
                            "isRecord": is_record,
                            "sourceSubmissionId": point["sourceSubmissionId"],
                        }
                    )

                results.sort(
                    key=lambda item: (
                        not item["valid"],
                        item["interactionCount"] is None,
                        item["interactionCount"] if item["interactionCount"] is not None else float("inf"),
                        item["model"].casefold(),
                        item["mode"],
                    )
                )
                point_rows.append(
                    {
                        "testId": test_id,
                        "scale": str(raw_point.get("scale") or "").strip() or None,
                        "threshold": threshold,
                        "record": record,
                        "metric": metric,
                        "recordHolders": [
                            {
                                "model": item["model"],
                                "modelKey": item["modelKey"],
                                "mode": item["mode"],
                                "harness": item["harness"],
                            }
                            for item in results if item["isRecord"]
                        ],
                        "results": results,
                    }
                )

            scopes[scope] = {
                "competitorCount": len(competitors),
                "competitors": competitors,
                "points": point_rows,
            }

        current_submissions: dict[
            CompetitorKey, tuple[tuple[int, int, Decimal, int], dict[str, Any]]
        ] = {}
        scope_scores = {
            mode: {
                (
                    competitor["modelKey"],
                    competitor["mode"],
                    competitor.get("harness") or "",
                ): competitor["problemScore"]
                for competitor in scopes[mode]["competitors"]
            }
            for mode in VALID_MODES
        }
        for submission in raw.get("submissions", []):
            if positive_int(submission.get("problemId"), "submission.problemId") != problem_id:
                continue
            submission_id = require_text(
                submission.get("submissionId"), "submission.submissionId"
            )
            competitor = competitor_from_submission(submission)
            submission_points = submission.get("points")
            if not isinstance(submission_points, list):
                raise DataError(f"submission {submission_id}.points must be an array")
            successful_points = sum(1 for point in submission_points if is_success(point))
            valid_scoring_points = 0
            successful_interactions = Decimal(0)
            for point in submission_points:
                test_id = positive_int(point.get("testId"), "result.testId")
                if not is_success(point):
                    continue
                interactions = nonnegative_decimal(
                    point.get("interactionCount"), "result.interactionCount"
                )
                successful_interactions += interactions
                threshold = nonnegative_decimal(
                    raw_points[test_id].get("threshold"), "point.threshold"
                )
                if interactions < threshold:
                    valid_scoring_points += 1
            published_submission = {
                "submissionId": submission_id,
                "model": require_text(submission.get("model"), "submission.model"),
                "modelKey": competitor.model_key,
                "mode": competitor.mode,
                "harness": competitor.harness or None,
                "successfulPoints": successful_points,
                "validScoringPoints": valid_scoring_points,
                "pointCount": len(raw_points),
                "allPointsSuccessful": successful_points == len(raw_points),
                "score": scope_scores[competitor.mode][
                    (competitor.model_key, competitor.mode, competitor.harness)
                ],
                "sourceCodeUrl": submission_source_url(submission_id),
            }
            copy_usage_fields(submission, published_submission)
            numeric_id = int(submission_id) if submission_id.isdigit() else 0
            quality = (
                valid_scoring_points,
                successful_points,
                -successful_interactions,
                numeric_id,
            )
            current = current_submissions.get(competitor)
            if current is None or quality > current[0]:
                current_submissions[competitor] = (quality, published_submission)

        problem_submissions = [item[1] for item in current_submissions.values()]
        problem_submissions.sort(
            key=lambda item: (
                item["mode"],
                item["model"].casefold(),
                int(item["submissionId"]) if item["submissionId"].isdigit() else item["submissionId"],
            )
        )

        metric = problem.get("metric") if isinstance(problem.get("metric"), dict) else {}
        details[problem_id] = {
            "schemaVersion": 1,
            "updated": result["updated"],
            "status": result["status"],
            "problem": {
                "problemId": problem_id,
                "title": require_text(problem.get("title"), "problem.title"),
                "dataVersion": require_text(problem.get("dataVersion"), "problem.dataVersion"),
                "pointCount": len(raw_points),
                "metric": {
                    "name": str(metric.get("name") or "interactions"),
                    "description": str(metric.get("description") or metric.get("name") or "Interactions"),
                },
            },
            "submissions": problem_submissions,
            "scopes": scopes,
        }

    return details


def build_submission_archive(details: dict[int, dict[str, Any]]) -> dict[str, Any]:
    """Build the compact Interactive source index used by the submissions page."""
    entries: list[dict[str, Any]] = []
    updated = ""
    for problem_id, detail in sorted(details.items()):
        updated = detail["updated"]
        title = detail["problem"]["title"]
        for submission in detail["submissions"]:
            entry = {
                "problemId": problem_id,
                "title": title,
                "track": submission["mode"],
                "model": submission["model"],
                "modelKey": submission["modelKey"],
                "harness": submission.get("harness"),
                "submissionId": submission["submissionId"],
                "score": submission["score"],
                "successfulPoints": submission["successfulPoints"],
                "validScoringPoints": submission["validScoringPoints"],
                "pointCount": submission["pointCount"],
                "path": submission["sourceCodeUrl"],
            }
            copy_usage_fields(submission, entry)
            entries.append(entry)
    return {
        "schemaVersion": 1,
        "updated": updated,
        "entries": entries,
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


def write_problem_details(directory: Path, details: dict[int, dict[str, Any]]) -> None:
    directory.mkdir(parents=True, exist_ok=True)
    expected_names = {f"{problem_id:03d}.json" for problem_id in details}
    for stale in directory.glob("*.json"):
        if stale.name not in expected_names:
            stale.unlink()
    for problem_id, payload in details.items():
        atomic_write_json(directory / f"{problem_id:03d}.json", payload)


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
    parser.add_argument(
        "--detail-output-dir",
        type=Path,
        default=Path("site/data/interactive/problems"),
    )
    parser.add_argument(
        "--submission-output",
        type=Path,
        default=Path("site/data/interactive-submissions.json"),
    )
    args = parser.parse_args()
    raw = json.loads(args.input.read_text(encoding="utf-8"))
    result = build(raw)
    atomic_write_json(args.output, compact_leaderboard(result))
    details = build_problem_details(raw, result)
    write_problem_details(args.detail_output_dir, details)
    atomic_write_json(args.submission_output, build_submission_archive(details))
    print(
        f"interactive leaderboard: {result['boards']['combined']['problemCount']} problems, "
        f"{result['boards']['combined']['pointCount']} points, "
        f"{len(result['boards']['combined']['rows'])} competitors; "
        f"{len(result['boards']['combined']['rows'][0]['problemScores']) if result['boards']['combined']['rows'] else 0} detail files"
    )


if __name__ == "__main__":
    main()
