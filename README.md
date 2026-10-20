# AlgoScale

AlgoScale is a competitive-programming benchmark for a specific form of algorithmic reasoning: a source problem is preserved, but its input scale is increased so that the original expected complexity no longer passes. Models must discover and implement the stronger algorithm.

The current release contains **88 UOJ-judged problems** and two evaluation tracks:

- **Direct generation** — one problem statement in, one C++20 solution out, without tools or feedback.
- **Agent mode** — a coding agent may edit, compile, test, and iterate inside a Harbor sandbox before final judging.

The project page publishes each problem's source link, source title, original expected complexity, and AlgoScale target complexity. Unknown metadata is explicitly marked `pending`.

For accepted benchmark submissions, the catalog links directly to the exact
`.cpp` source received by the UOJ judge. Failed, partial-score, and unjudged
submissions are not published. No model response text, reasoning trace, logs,
hidden judge material, or reference solution is published.

## Project page

After GitHub Pages is enabled, the site is available at:

<https://shi0712.github.io/AlgoScale/>

## Preview locally

From the repository root:

```powershell
python -m http.server 8000 --directory site
```

Then open <http://127.0.0.1:8000/>.

## Rebuild the problem catalog

The public JSON catalog is generated from the maintained origin CSV and complexity workbook:

```powershell
python scripts/build_problem_catalog.py `
  --origins-csv "path\to\problem-origins.csv" `
  --complexity-xlsx "path\to\problem-origins.pending.xlsx" `
  --output site\data\problems.json
```

The generator requires positive, unique problem IDs and sorts the catalog by ID. It
joins complexity metadata by the maintained local title, preventing stale IDs in
older workbooks from silently misaligning rows.

## Local metadata admin

Run the local-only management UI from the repository root:

```powershell
python scripts/admin_server.py --open
```

The server listens only on `http://127.0.0.1:8787/`. It supports creating,
editing, searching, and deleting problem metadata, and writes changes atomically
to `site/data/problems.json`. Accepted-code mappings are read-only in the form
and are preserved when a row is edited. Use Git to review, commit, and push the
result before it appears on the public site.

## Publish generated C++ from another model

Each model gets its own directory. Exporting a new run adds that model to the
existing catalog without replacing previously published sources:

```powershell
python scripts/export_generated_cpp.py `
  --model-slug "model-name" `
  --direct-summary "path\to\summary.json" `
  --catalog site\data\problems.json `
  --output-root site\generated
```

For an agent run, replace `--direct-summary` with one or more `--agent-job`
arguments pointing to Harbor job directories. Direct and agent inputs may also
be supplied together. Only the exact `.cpp` source from a 100-point UOJ
submission is copied into the public site; stale non-accepted files for that
model and track are removed.

## Repository layout

```text
site/                         Static GitHub Pages site
  data/problems.json          Public 88-problem catalog
  data/leaderboard.json       Preliminary evaluation results
scripts/build_problem_catalog.py
scripts/export_generated_cpp.py  Export exact UOJ-submitted model sources
.github/workflows/pages.yml   GitHub Pages deployment
```

## Reproducibility policy

Correctness is established by the native UOJ judge packages, including validators, checkers, interactive protocols, and resource limits. The benchmark release will retain model outputs, verdicts, tool trajectories, token usage, and separately labeled infrastructure retries. Hidden tests and reference solutions remain outside agent workspaces.

## Status

The website currently shows preliminary pilot results. Paper-frozen evaluation matrices and complete metadata will replace them before release.
