import json
import unittest
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DOMAINS = {
    "Algorithmic Paradigms": 23,
    "Computational Geometry": 7,
    "Data Structures": 16,
    "Dynamic Programming": 26,
    "Graph Theory": 24,
    "Mathematics": 39,
    "Search & Enumeration": 7,
}


class TaxonomyTest(unittest.TestCase):
    def setUp(self) -> None:
        self.problems = json.loads(
            (ROOT / "site/data/problems.json").read_text(encoding="utf-8")
        )

    def test_every_problem_has_english_taxonomy(self) -> None:
        self.assertEqual(len(self.problems), 74)
        for problem in self.problems:
            taxonomy = problem.get("taxonomy")
            self.assertIsInstance(taxonomy, dict, problem["id"])
            self.assertIn(taxonomy.get("primaryDomain"), DOMAINS, problem["id"])
            self.assertTrue(taxonomy.get("coreTags"), problem["id"])
            values = [
                taxonomy["primaryDomain"],
                *taxonomy.get("relatedDomains", []),
                *taxonomy.get("coreTags", []),
            ]
            for value in values:
                self.assertFalse(
                    any("\u4e00" <= character <= "\u9fff" for character in value),
                    f"#{problem['id']} contains untranslated taxonomy: {value}",
                )

    def test_primary_and_related_domain_union_counts(self) -> None:
        counts = Counter()
        for problem in self.problems:
            taxonomy = problem["taxonomy"]
            domains = {
                taxonomy["primaryDomain"],
                *taxonomy.get("relatedDomains", []),
            }
            counts.update(domains)
        self.assertEqual(dict(counts), DOMAINS)

    def test_leaderboard_model_keys_match_published_accepted_code(self) -> None:
        leaderboard = json.loads(
            (ROOT / "site/data/leaderboard.json").read_text(encoding="utf-8")
        )
        for track, entries in leaderboard["tracks"].items():
            for entry in entries:
                solved = sum(
                    entry["modelKey"] in problem.get("generatedCode", {}).get(track, {})
                    for problem in self.problems
                )
                self.assertEqual(solved, entry["solved"], (track, entry["model"]))
                self.assertEqual(len(self.problems), entry["total"])


if __name__ == "__main__":
    unittest.main()
