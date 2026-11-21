# AlgoScale

AlgoScale is a competitive-programming benchmark for a specific form of algorithmic reasoning: a source problem is preserved, but its input scale is increased so that the original expected complexity no longer passes. Models must discover and implement the stronger algorithm.

The current release contains **74 UOJ-judged problems**, split into Standard and
Interactive benchmark types, and two evaluation modes:

- **Direct generation** — one problem statement in, one C++20 solution out, without tools or feedback.
- **Agent mode** — a coding agent may edit, compile, test, and iterate inside a Harbor sandbox before final judging.

The project page publishes each problem's source link, source title, original expected complexity, and AlgoScale target complexity. Unknown metadata is explicitly marked `pending`.

Both benchmark types publish Direct, Agent, and Combined leaderboards. Standard
results are ranked by pass rate and exclude every Interactive task from their
numerators and denominators. The Combined view places Direct and Agent
configurations in one table without merging them.

Interactive tasks use point-level dynamic scoring. Every point receives a score
from 0 to 100 based on its interaction threshold and the best participating
model record. Point scores are averaged within a problem, then problem scores
are averaged across the suite. Direct and Agent views maintain separate point
records; Combined uses records shared across both modes. Repeated runs of the
same model, mode, and harness are combined pointwise by their best valid result.

For accepted benchmark submissions, the catalog links directly to the exact
`.cpp` source received by the UOJ judge. Failed, partial-score, and unjudged
submissions are not published. No model response text, reasoning trace, logs,
or hidden judge material is published.

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

## Rebuild leaderboards

Remove Interactive tasks from the Standard leaderboard after updating results:

```powershell
python scripts/build_standard_leaderboard.py
```

Point-level Interactive source data lives in
`site/data/interactive-results.json`. It records problem data versions, every
point threshold, and every submission's point verdict, protocol validity, and
interaction count. Rebuild the three Interactive views with:

Every Interactive package must follow the
[`Interactive judge contract`](docs/INTERACTOR_CONTRACT.md); the score formula
and record-sharing rules are specified in
[`Interactive leaderboard scoring`](docs/INTERACTIVE_SCORING.md).

```powershell
python scripts/build_interactive_leaderboard.py
```

The generated `site/data/interactive-leaderboard.json` contains compact Direct,
Agent, and Combined board summaries. Per-problem point records and model results
are written to `site/data/interactive/problems/NNN.json` and are loaded only when
a visitor opens that Interactive task. Reference solutions validate judge
packages but are not entered as record-setting submissions.

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

## Publish official solutions and reference programs

The author packages use several internal names (`solution.md`, `editorial.md`,
`std.cpp`, `std17.cpp`, and others). Normalize all retained packages into the public
reference layout with:

```powershell
python scripts/publish_reference_assets.py
```

The script discovers the locally retained author ZIPs, extracts each editorial
and reference implementation as UTF-8, writes them under
`site/references/NNN/`, and updates `referenceAssets` in the public catalog. It
fails instead of silently publishing a mismatched package. The site renders
Markdown and LaTeX in a dedicated solution viewer and syntax-highlights C++.

Each problem also publishes an English original statement and its AlgoScale
statement. The reference viewer provides Compare, Original, and AlgoScale
modes; titles come from the catalog and are not repeated as the first line of
either Markdown body. The normalized layout is:

```text
site/references/NNN/
  original-statement.md
  algoscale-statement.md
  solution.md
  std.cpp
```

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
  data/problems.json          Public 117-problem catalog (66 Standard, 51 Interactive)
  data/leaderboard.json       Standard-only preliminary evaluation results
  data/interactive-results.json  Point-level Interactive source results
  data/interactive-leaderboard.json  Generated Interactive leaderboards
  data/interactive/problems/NNN.json  Lazy-loaded per-problem point results
scripts/build_problem_catalog.py
scripts/build_standard_leaderboard.py
scripts/build_interactive_leaderboard.py
scripts/export_generated_cpp.py  Export exact UOJ-submitted model sources
.github/workflows/pages.yml   GitHub Pages deployment
```

## Reproducibility policy

Correctness is established by the native UOJ judge packages, including validators, checkers, interactive protocols, and resource limits. The benchmark release will retain model outputs, verdicts, tool trajectories, token usage, and separately labeled infrastructure retries. Hidden tests and reference solutions remain outside agent workspaces.

## Status

The website currently shows preliminary pilot results. Paper-frozen evaluation matrices and complete metadata will replace them before release.
