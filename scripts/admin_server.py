#!/usr/bin/env python3
"""Local-only CRUD server for AlgoScale public problem metadata."""

from __future__ import annotations

import argparse
import json
import os
import re
import tempfile
import threading
import webbrowser
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse


MUTATION_HEADER = "X-AlgoScale-Admin"
ALLOWED_TRACKS = {"Standard", "Interactive"}
ALLOWED_STATUSES = {"verified", "pending"}
PROBLEM_ROUTE = re.compile(r"^/api/problems/(\d+)$")
MAX_BODY_BYTES = 1_000_000


def parse_args() -> argparse.Namespace:
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser()
    parser.add_argument("--catalog", type=Path, default=root / "site/data/problems.json")
    parser.add_argument("--static-dir", type=Path, default=root / "admin")
    parser.add_argument("--port", type=int, default=8787)
    parser.add_argument("--open", action="store_true", help="open the admin UI in a browser")
    return parser.parse_args()


def clean_text(value, field: str, *, required: bool = False, maximum: int = 500) -> str:
    if value is None:
        value = ""
    if not isinstance(value, str):
        raise ValueError(f"{field} must be text")
    value = value.strip()
    if required and not value:
        raise ValueError(f"{field} is required")
    if len(value) > maximum:
        raise ValueError(f"{field} exceeds {maximum} characters")
    return value


def validate_problem(payload: dict, *, existing: dict | None, problem_id: int) -> dict:
    if not isinstance(payload, dict):
        raise ValueError("request body must be a JSON object")
    original_url = clean_text(payload.get("originalUrl"), "originalUrl", maximum=1000)
    if original_url:
        parsed = urlparse(original_url)
        if parsed.scheme not in {"http", "https"} or not parsed.netloc:
            raise ValueError("originalUrl must be an http(s) URL")
    track = clean_text(payload.get("track") or "Standard", "track")
    if track not in ALLOWED_TRACKS:
        raise ValueError(f"track must be one of {sorted(ALLOWED_TRACKS)}")
    status = clean_text(payload.get("metadataStatus") or "pending", "metadataStatus")
    if status not in ALLOWED_STATUSES:
        raise ValueError(f"metadataStatus must be one of {sorted(ALLOWED_STATUSES)}")
    return {
        "id": problem_id,
        "title": clean_text(payload.get("title"), "title", required=True),
        "originalTitle": clean_text(payload.get("originalTitle"), "originalTitle"),
        "originalUrl": original_url or None,
        "platform": clean_text(payload.get("platform") or "Pending", "platform"),
        "originalComplexity": clean_text(
            payload.get("originalComplexity"), "originalComplexity"
        ) or None,
        "algoscaleComplexity": clean_text(
            payload.get("algoscaleComplexity"), "algoscaleComplexity"
        ) or None,
        "track": track,
        "metadataStatus": status,
        "generatedCode": (existing or {}).get("generatedCode", {}),
        "referenceAssets": (existing or {}).get("referenceAssets", {}),
    }


class CatalogStore:
    def __init__(self, path: Path):
        self.path = path.resolve()
        self.lock = threading.RLock()

    def read(self) -> list[dict]:
        with self.lock:
            data = json.loads(self.path.read_text(encoding="utf-8"))
            if not isinstance(data, list):
                raise ValueError("catalog root must be an array")
            return sorted(data, key=lambda item: int(item["id"]))

    def write(self, problems: list[dict]) -> None:
        payload = json.dumps(problems, ensure_ascii=False, indent=2) + "\n"
        with self.lock:
            self.path.parent.mkdir(parents=True, exist_ok=True)
            descriptor, temporary_name = tempfile.mkstemp(
                prefix=f".{self.path.name}.", suffix=".tmp", dir=self.path.parent
            )
            try:
                with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as handle:
                    handle.write(payload)
                    handle.flush()
                    os.fsync(handle.fileno())
                os.replace(temporary_name, self.path)
            finally:
                if os.path.exists(temporary_name):
                    os.unlink(temporary_name)


class AdminHandler(BaseHTTPRequestHandler):
    store: CatalogStore
    static_dir: Path

    def log_message(self, fmt: str, *args) -> None:
        print(f"[{self.log_date_time_string()}] {fmt % args}")

    def send_json(self, status: int, payload) -> None:
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()
        self.wfile.write(body)

    def send_static(self, filename: str, content_type: str) -> None:
        path = self.static_dir / filename
        if not path.is_file():
            self.send_error(HTTPStatus.NOT_FOUND)
            return
        body = path.read_bytes()
        self.send_response(HTTPStatus.OK)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header(
            "Content-Security-Policy",
            "default-src 'self'; script-src 'self'; style-src 'self'; "
            "img-src 'self' data:; connect-src 'self'; frame-ancestors 'none'",
        )
        self.end_headers()
        self.wfile.write(body)

    def read_payload(self) -> dict:
        if self.headers.get(MUTATION_HEADER) != "1":
            raise PermissionError(f"missing {MUTATION_HEADER} header")
        if not self.headers.get("Content-Type", "").startswith("application/json"):
            raise ValueError("Content-Type must be application/json")
        try:
            length = int(self.headers.get("Content-Length", "0"))
        except ValueError as exc:
            raise ValueError("invalid Content-Length") from exc
        if length <= 0 or length > MAX_BODY_BYTES:
            raise ValueError("invalid request body size")
        return json.loads(self.rfile.read(length).decode("utf-8"))

    def do_GET(self) -> None:
        if self.path in {"/", "/index.html"}:
            self.send_static("index.html", "text/html; charset=utf-8")
        elif self.path == "/styles.css":
            self.send_static("styles.css", "text/css; charset=utf-8")
        elif self.path == "/app.js":
            self.send_static("app.js", "text/javascript; charset=utf-8")
        elif self.path == "/api/problems":
            self.send_json(HTTPStatus.OK, self.store.read())
        elif self.path == "/api/health":
            self.send_json(HTTPStatus.OK, {"ok": True})
        else:
            self.send_error(HTTPStatus.NOT_FOUND)

    def do_POST(self) -> None:
        if self.path != "/api/problems":
            self.send_error(HTTPStatus.NOT_FOUND)
            return
        try:
            payload = self.read_payload()
            problems = self.store.read()
            next_id = max((int(item["id"]) for item in problems), default=0) + 1
            created = validate_problem(payload, existing=None, problem_id=next_id)
            problems.append(created)
            self.store.write(problems)
            self.send_json(HTTPStatus.CREATED, created)
        except PermissionError as exc:
            self.send_json(HTTPStatus.FORBIDDEN, {"error": str(exc)})
        except (ValueError, json.JSONDecodeError) as exc:
            self.send_json(HTTPStatus.BAD_REQUEST, {"error": str(exc)})

    def do_PUT(self) -> None:
        match = PROBLEM_ROUTE.match(self.path)
        if match is None:
            self.send_error(HTTPStatus.NOT_FOUND)
            return
        try:
            payload = self.read_payload()
            problem_id = int(match.group(1))
            problems = self.store.read()
            index = next(
                (i for i, problem in enumerate(problems) if int(problem["id"]) == problem_id),
                None,
            )
            if index is None:
                self.send_json(HTTPStatus.NOT_FOUND, {"error": "problem not found"})
                return
            updated = validate_problem(payload, existing=problems[index], problem_id=problem_id)
            problems[index] = updated
            self.store.write(problems)
            self.send_json(HTTPStatus.OK, updated)
        except PermissionError as exc:
            self.send_json(HTTPStatus.FORBIDDEN, {"error": str(exc)})
        except (ValueError, json.JSONDecodeError) as exc:
            self.send_json(HTTPStatus.BAD_REQUEST, {"error": str(exc)})

    def do_DELETE(self) -> None:
        match = PROBLEM_ROUTE.match(self.path)
        if match is None:
            self.send_error(HTTPStatus.NOT_FOUND)
            return
        if self.headers.get(MUTATION_HEADER) != "1":
            self.send_json(HTTPStatus.FORBIDDEN, {"error": f"missing {MUTATION_HEADER} header"})
            return
        problem_id = int(match.group(1))
        problems = self.store.read()
        remaining = [problem for problem in problems if int(problem["id"]) != problem_id]
        if len(remaining) == len(problems):
            self.send_json(HTTPStatus.NOT_FOUND, {"error": "problem not found"})
            return
        self.store.write(remaining)
        self.send_json(HTTPStatus.OK, {"deleted": problem_id})


def main() -> None:
    args = parse_args()
    if not args.catalog.is_file():
        raise SystemExit(f"catalog not found: {args.catalog}")
    if not args.static_dir.is_dir():
        raise SystemExit(f"admin static directory not found: {args.static_dir}")
    AdminHandler.store = CatalogStore(args.catalog)
    AdminHandler.static_dir = args.static_dir.resolve()
    server = ThreadingHTTPServer(("127.0.0.1", args.port), AdminHandler)
    url = f"http://127.0.0.1:{args.port}/"
    print(f"AlgoScale admin: {url}")
    print(f"Catalog: {AdminHandler.store.path}")
    print("Changes are local until committed and pushed with Git.")
    if args.open:
        webbrowser.open(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
