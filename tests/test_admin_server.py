import json
import tempfile
import threading
import unittest
from http.client import HTTPConnection
from http.server import ThreadingHTTPServer
from pathlib import Path

from scripts.admin_server import AdminHandler, CatalogStore


class AdminServerTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary_directory = tempfile.TemporaryDirectory()
        root = Path(self.temporary_directory.name)
        self.catalog = root / "problems.json"
        self.catalog.write_text(
            json.dumps(
                [
                    {
                        "id": 1,
                        "title": "Existing",
                        "originalTitle": "Source",
                        "originalUrl": "https://example.com/problem/1",
                        "platform": "Example",
                        "originalComplexity": "O(n^2)",
                        "algoscaleComplexity": "O(nlogn)",
                        "track": "Standard",
                        "metadataStatus": "verified",
                        "generatedCode": {"direct": {"model": "./accepted.cpp"}},
                        "referenceAssets": {
                            "solution": "./references/001/solution.md",
                            "standardSolution": "./references/001/std.cpp",
                        },
                    }
                ]
            ),
            encoding="utf-8",
        )
        static = root / "static"
        static.mkdir()
        for name in ("index.html", "styles.css", "app.js"):
            (static / name).write_text(name, encoding="utf-8")

        AdminHandler.store = CatalogStore(self.catalog)
        AdminHandler.static_dir = static
        self.server = ThreadingHTTPServer(("127.0.0.1", 0), AdminHandler)
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()

    def tearDown(self) -> None:
        self.server.shutdown()
        self.server.server_close()
        self.thread.join(timeout=2)
        self.temporary_directory.cleanup()

    def request(self, method: str, path: str, payload=None, *, authorized=True):
        headers = {}
        body = None
        if payload is not None:
            body = json.dumps(payload)
            headers["Content-Type"] = "application/json"
        if authorized:
            headers["X-AlgoScale-Admin"] = "1"
        connection = HTTPConnection("127.0.0.1", self.server.server_port, timeout=3)
        connection.request(method, path, body=body, headers=headers)
        response = connection.getresponse()
        raw = response.read()
        connection.close()
        return response.status, json.loads(raw) if raw else None

    def test_crud_preserves_generated_code(self) -> None:
        status, problems = self.request("GET", "/api/problems")
        self.assertEqual(status, 200)
        self.assertEqual(len(problems), 1)

        created_payload = {
            "title": "Created",
            "originalTitle": "Original",
            "originalUrl": "https://qoj.ac/problem/42",
            "platform": "QOJ",
            "originalComplexity": r"\mathcal{O}(n^2)",
            "algoscaleComplexity": r"\mathcal{O}(n\log n)",
            "track": "Interactive",
            "metadataStatus": "verified",
        }
        status, created = self.request("POST", "/api/problems", created_payload)
        self.assertEqual(status, 201)
        self.assertEqual(created["id"], 2)

        update_payload = dict(created_payload, title="Updated", track="Standard")
        status, updated = self.request("PUT", "/api/problems/1", update_payload)
        self.assertEqual(status, 200)
        self.assertEqual(updated["title"], "Updated")
        self.assertEqual(
            updated["generatedCode"], {"direct": {"model": "./accepted.cpp"}}
        )
        self.assertEqual(
            updated["referenceAssets"],
            {
                "solution": "./references/001/solution.md",
                "standardSolution": "./references/001/std.cpp",
            },
        )

        status, deleted = self.request("DELETE", "/api/problems/2")
        self.assertEqual(status, 200)
        self.assertEqual(deleted, {"deleted": 2})

    def test_mutation_requires_header_and_valid_fields(self) -> None:
        payload = {"title": "Blocked"}
        status, error = self.request(
            "POST", "/api/problems", payload, authorized=False
        )
        self.assertEqual(status, 403)
        self.assertIn("X-AlgoScale-Admin", error["error"])

        payload.update(track="Unknown")
        status, error = self.request("POST", "/api/problems", payload)
        self.assertEqual(status, 400)
        self.assertIn("track", error["error"])


if __name__ == "__main__":
    unittest.main()
