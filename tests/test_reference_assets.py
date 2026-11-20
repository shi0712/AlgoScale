import json
import unittest
from pathlib import Path


class ReferenceAssetsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.repository = Path(__file__).resolve().parents[1]
        cls.site = cls.repository / "site"
        cls.catalog = json.loads(
            (cls.site / "data/problems.json").read_text(encoding="utf-8")
        )

    def test_every_problem_has_nonempty_reference_assets(self) -> None:
        self.assertEqual(len(self.catalog), 117)
        for problem in self.catalog:
            with self.subTest(problem_id=problem["id"]):
                assets = problem.get("referenceAssets", {})
                self.assertEqual(
                    set(assets),
                    {
                        "solution",
                        "standardSolution",
                        "originalStatement",
                        "algoscaleStatement",
                    },
                )
                for relative in assets.values():
                    self.assertTrue(relative.startswith("./references/"))
                    path = self.site / relative.removeprefix("./")
                    self.assertTrue(path.is_file(), path)
                    self.assertGreater(path.stat().st_size, 0, path)

    def test_reference_layout_is_exact(self) -> None:
        files = sorted(
            path.relative_to(self.site / "references").as_posix()
            for path in (self.site / "references").rglob("*")
            if path.is_file()
        )
        expected = sorted(
            f"{problem_id:03d}/{filename}"
            for problem_id in (problem["id"] for problem in self.catalog)
            for filename in (
                "solution.md",
                "std.cpp",
                "original-statement.md",
                "algoscale-statement.md",
            )
        )
        self.assertEqual(files, expected)

    def test_catalog_and_asset_manifest_ids_match(self) -> None:
        ids = [problem["id"] for problem in self.catalog]
        self.assertEqual(ids, list(range(1, 118)))
        manifest = json.loads((self.site / "data/reference-assets.json").read_text(encoding="utf-8"))
        self.assertEqual([row["id"] for row in manifest], ids)
        for item, row in zip(self.catalog, manifest):
            self.assertEqual(item["referenceAssets"], row["assets"])

    def test_interactive_catalog_metadata_is_complete_and_unique(self) -> None:
        interactive = [
            problem for problem in self.catalog if problem.get("track") == "Interactive"
        ]
        self.assertEqual(len(interactive), 51)
        identities = []
        for problem in interactive:
            with self.subTest(problem_id=problem["id"]):
                links = problem.get("originalLinks")
                self.assertIsInstance(links, list)
                self.assertTrue(links)
                self.assertTrue(all(link.get("label") and link.get("url") for link in links))
                self.assertTrue(str(problem.get("zeroScoreLine", "")).strip())
                identities.append(
                    (
                        problem["title"].casefold(),
                        tuple(link["url"] for link in links),
                    )
                )
        self.assertEqual(len(identities), len(set(identities)))


if __name__ == "__main__":
    unittest.main()
