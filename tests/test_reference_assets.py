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

    def test_every_problem_has_two_nonempty_reference_assets(self) -> None:
        self.assertEqual(len(self.catalog), 88)
        for problem in self.catalog:
            with self.subTest(problem_id=problem["id"]):
                assets = problem.get("referenceAssets", {})
                self.assertEqual(set(assets), {"solution", "standardSolution"})
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
            for problem_id in range(1, 89)
            for filename in ("solution.md", "std.cpp")
        )
        self.assertEqual(files, expected)


if __name__ == "__main__":
    unittest.main()
