import unittest
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from build_interactive_leaderboard import (  # noqa: E402
    DataError,
    build,
    build_problem_details,
    build_submission_archive,
    compact_leaderboard,
)


def fixture():
    return {
        "schemaVersion": 1,
        "updated": "2026-11-20",
        "status": "Preliminary",
        "problems": [
            {
                "problemId": 80,
                "title": "Interactive Fixture",
                "categories": ["Mathematics", "Search & Enumeration"],
                "dataVersion": "v1",
                "points": [
                    {"testId": 1, "threshold": 100, "metric": "queries"},
                    {"testId": 2, "threshold": 50, "metric": "queries"},
                ],
            }
        ],
        "submissions": [
            {
                "submissionId": "d1",
                "model": "Model A",
                "modelKey": "model-a",
                "mode": "direct",
                "problemId": 80,
                "dataVersion": "v1",
                "points": [
                    {
                        "testId": 1,
                        "verdict": "Accepted",
                        "protocolValid": True,
                        "interactionCount": 40,
                    },
                    {
                        "testId": 2,
                        "verdict": "Wrong Answer",
                        "protocolValid": True,
                        "interactionCount": 10,
                    },
                ],
            },
            {
                "submissionId": "d2",
                "uojScore": 100,
                "model": "Model A",
                "modelKey": "model-a",
                "mode": "direct",
                "problemId": 80,
                "dataVersion": "v1",
                "points": [
                    {
                        "testId": 1,
                        "verdict": "Accepted",
                        "protocolValid": True,
                        "interactionCount": 30,
                    },
                    {
                        "testId": 2,
                        "verdict": "Accepted",
                        "protocolValid": True,
                        "interactionCount": 40,
                    },
                ],
            },
            {
                "submissionId": "a1",
                "uojScore": 100,
                "model": "Model B",
                "modelKey": "model-b",
                "mode": "agent",
                "harness": "Harness X",
                "problemId": 80,
                "dataVersion": "v1",
                "points": [
                    {
                        "testId": 1,
                        "verdict": "Accepted",
                        "protocolValid": True,
                        "interactionCount": 20,
                    },
                    {
                        "testId": 2,
                        "verdict": "Accepted",
                        "protocolValid": True,
                        "interactionCount": 50,
                    },
                ],
            },
        ],
    }


class InteractiveLeaderboardTest(unittest.TestCase):
    def test_pointwise_best_and_separate_records(self):
        result = build(fixture())
        direct = result["boards"]["direct"]["rows"]
        self.assertEqual(len(direct), 1)
        self.assertEqual(direct[0]["score"], 100.0)
        self.assertEqual(direct[0]["records"], 2)
        points = direct[0]["problemScores"][0]["points"]
        self.assertEqual(points[0]["interactionCount"], 30.0)
        self.assertEqual(points[0]["sourceSubmissionId"], "d2")

        agent = result["boards"]["agent"]["rows"]
        self.assertEqual(agent[0]["score"], 50.0)
        self.assertEqual(agent[0]["records"], 1)
        self.assertFalse(agent[0]["problemScores"][0]["points"][1]["valid"])

    def test_combined_record_recalculates_every_score(self):
        result = build(fixture())
        rows = result["boards"]["combined"]["rows"]
        agent = next(row for row in rows if row["mode"] == "agent")
        direct = next(row for row in rows if row["mode"] == "direct")
        self.assertEqual(agent["score"], 50.0)
        # Point 1: 100 * ((100 - 30) / (100 - 20))^2 = 76.5625.
        # Point 2 remains a Direct record, so the problem mean is 88.28125.
        self.assertEqual(direct["score"], 88.28125)
        self.assertEqual(direct["fullScoreRate"], 100.0)
        self.assertEqual(agent["fullScoreRate"], 100.0)
        self.assertEqual(agent["rank"], 2)
        self.assertEqual(direct["rank"], 1)

    def test_full_score_requires_a_uoj_100_and_all_points_within_the_limit(self):
        for uoj_score, points in (
            (99, [(50, True), (50, True)]),
            (None, [(50, True), (50, True)]),
            (100, [(50, True), (51, True)]),
            (100, [(50, True), (50, False)]),
            (100, [(50, True)]),
        ):
            with self.subTest(uoj_score=uoj_score, points=points):
                data = fixture()
                submission = data["submissions"][2]
                submission["uojScore"] = uoj_score
                submission["points"] = [
                    {
                        "testId": index,
                        "verdict": "Accepted",
                        "protocolValid": valid,
                        "interactionCount": count,
                    }
                    for index, (count, valid) in enumerate(points, start=1)
                ]
                row = build(data)["boards"]["agent"]["rows"][0]
                self.assertEqual(row["fullScoreRate"], 0.0)

    def test_category_scores_average_only_tagged_problems_in_each_scope(self):
        data = fixture()
        data["problems"].append({
            **data["problems"][0],
            "problemId": 81,
            "categories": ["Search & Enumeration"],
        })
        result = compact_leaderboard(build(data))
        self.assertEqual(result["categories"], ["Mathematics", "Search & Enumeration"])
        row = result["boards"]["agent"]["rows"][0]
        self.assertEqual(row["categoryScores"], {"Mathematics": 50.0, "Search & Enumeration": 25.0})
        self.assertEqual(row["score"], 25.0)
        self.assertEqual(row["fullScoreRate"], 50.0)

    def test_equal_scores_share_competition_rank(self):
        data = fixture()
        clone = dict(data["submissions"][2])
        clone.update(
            {
                "submissionId": "a2",
                "model": "Model C",
                "modelKey": "model-c",
            }
        )
        data["submissions"].append(clone)
        rows = build(data)["boards"]["agent"]["rows"]
        self.assertEqual([row["rank"] for row in rows], [1, 1])

    def test_retries_are_not_combined_pointwise(self):
        data = fixture()
        data["submissions"] = [
            {
                **data["submissions"][0],
                "submissionId": "r1",
                "points": [
                    {"testId": 1, "verdict": "Accepted", "protocolValid": True, "interactionCount": 20},
                    {"testId": 2, "verdict": "Wrong Answer", "protocolValid": False, "interactionCount": 1},
                ],
            },
            {
                **data["submissions"][0],
                "submissionId": "r2",
                "points": [
                    {"testId": 1, "verdict": "Wrong Answer", "protocolValid": False, "interactionCount": 1},
                    {"testId": 2, "verdict": "Accepted", "protocolValid": True, "interactionCount": 30},
                ],
            },
        ]
        result = build(data)
        self.assertEqual(result["boards"]["direct"]["rows"][0]["fullScoreRate"], 0.0)
        points = result["boards"]["direct"]["rows"][0]["problemScores"][0]["points"]
        # r1 is selected as the better whole program; r2 must not fill its failed point.
        self.assertEqual(points[0]["sourceSubmissionId"], "r1")
        self.assertIsNone(points[1]["sourceSubmissionId"])
        detail = build_problem_details(data, result)[80]
        self.assertEqual([item["submissionId"] for item in detail["submissions"]], ["r1"])

    def test_stale_data_version_is_rejected(self):
        data = fixture()
        data["submissions"][0]["dataVersion"] = "old"
        with self.assertRaises(DataError):
            build(data)

    def test_compact_homepage_and_problem_detail_payloads(self):
        data = fixture()
        result = build(data)
        compact = compact_leaderboard(result)
        self.assertNotIn("problemScores", compact["boards"]["direct"]["rows"][0])

        details = build_problem_details(data, result)[80]
        combined = details["scopes"]["combined"]
        self.assertEqual(details["problem"]["pointCount"], 2)
        self.assertEqual(combined["points"][0]["record"], 20.0)
        self.assertEqual(
            [holder["model"] for holder in combined["points"][0]["recordHolders"]],
            ["Model B"],
        )
        direct_result = next(
            item for item in combined["points"][0]["results"]
            if item["mode"] == "direct"
        )
        self.assertEqual(direct_result["interactionCount"], 30.0)
        self.assertEqual(direct_result["sourceSubmissionId"], "d2")
        self.assertEqual(len(details["submissions"]), 2)
        retry = next(
            item for item in details["submissions"]
            if item["submissionId"] == "d2"
        )
        self.assertEqual(retry["successfulPoints"], 2)
        self.assertEqual(retry["validScoringPoints"], 2)
        self.assertTrue(retry["allPointsSuccessful"])
        self.assertEqual(
            retry["sourceCodeUrl"],
            "./generated/interactive/submissions/d2.cpp",
        )
        archive = build_submission_archive({80: details})
        archived_retry = next(
            item for item in archive["entries"]
            if item["submissionId"] == "d2"
        )
        self.assertEqual(archived_retry["score"], 100.0)
        self.assertEqual(archived_retry["track"], "direct")
        self.assertEqual(
            archived_retry["path"],
            "./generated/interactive/submissions/d2.cpp",
        )


if __name__ == "__main__":
    unittest.main()
