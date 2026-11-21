import unittest
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from build_interactive_leaderboard import (  # noqa: E402
    DataError,
    build,
    build_problem_details,
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
        self.assertEqual(agent["rank"], 2)
        self.assertEqual(direct["rank"], 1)

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


if __name__ == "__main__":
    unittest.main()
