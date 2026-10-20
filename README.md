# AlgoScale

AlgoScale is a competitive-programming benchmark for a specific form of algorithmic reasoning: a source problem is preserved, but its input scale is increased so that the original expected complexity no longer passes. Models must discover and implement the stronger algorithm.

The current release contains **88 UOJ-judged problems** and two evaluation tracks:

- **Direct generation** — one problem statement in, one C++20 solution out, without tools or feedback.
- **Agent mode** — a coding agent may edit, compile, test, and iterate inside a Harbor sandbox before final judging.

The project page publishes each problem's source link, source title, original expected complexity, and AlgoScale target complexity. Unknown metadata is explicitly marked `pending`.

For completed benchmark submissions, the catalog links directly to the exact
`.cpp` source received by the UOJ judge. No model response text, reasoning
trace, logs, hidden judge material, or reference solution is published.

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

The generator requires problem IDs `1..88` in order and joins complexity metadata by the maintained local title, preventing stale IDs in older workbooks from silently misaligning rows.

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
