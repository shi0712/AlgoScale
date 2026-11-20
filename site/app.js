"use strict";

const SITE_RELEASE = "20261120-interactive-scores-v2";

const DOMAIN_ORDER = [
  "Algorithmic Paradigms",
  "Computational Geometry",
  "Data Structures",
  "Dynamic Programming",
  "Graph Theory",
  "Mathematics",
  "Search & Enumeration",
];

const state = {
  problems: [],
  query: "",
  catalogKind: "Standard",
  expanded: false,
  leaderboardKind: "standard",
  track: "direct",
};

const els = {
  leaderboardBody: document.querySelector("#leaderboard-body"),
  leaderboardHead: document.querySelector("#leaderboard-head"),
  leaderboardDate: document.querySelector("#leaderboard-date"),
  leaderboardStatus: document.querySelector("#leaderboard-status"),
  leaderboardDescription: document.querySelector("#leaderboard-description"),
  leaderboardTableWrap: document.querySelector("#leaderboard-table-wrap"),
  leaderboardEmpty: document.querySelector("#leaderboard-empty"),
  interactiveMethod: document.querySelector("#interactive-method"),
  interactiveFormula: document.querySelector("#interactive-formula"),
  evaluationConfig: document.querySelector("#evaluation-config"),
  problemBody: document.querySelector("#problem-body"),
  problemHead: document.querySelector("#problem-head"),
  catalogTable: document.querySelector("#catalog-table"),
  catalogDescription: document.querySelector("#catalog-description"),
  visibleCount: document.querySelector("#visible-count"),
  search: document.querySelector("#problem-search"),
  showAll: document.querySelector("#show-all"),
  empty: document.querySelector("#empty-state"),
  nav: document.querySelector(".nav"),
  menu: document.querySelector(".menu-button"),
  heroProblemCount: document.querySelector("#hero-problem-count"),
  statProblemCount: document.querySelector("#stat-problem-count"),
  statInteractiveCount: document.querySelector("#stat-interactive-count"),
};

function element(tag, className, text) {
  const node = document.createElement(tag);
  if (className) node.className = className;
  if (text !== undefined) node.textContent = text;
  return node;
}

function normalize(value) {
  return String(value || "").toLocaleLowerCase();
}

function pendingNode() {
  return element("span", "pending", "Pending");
}

function complexityLatex(value) {
  let source = String(value || "").trim().replace(/^\$+|\$+$/g, "");
  if (!source || source.toLowerCase() === "null") return "";
  if (source.toLowerCase() === "no solution") return "\\mathrm{No\\ Solution}";

  // The admin accepts explicit LaTeX. Preserve deliberately formatted values,
  // while still normalizing the compact notation imported from the workbook.
  if (/\\(?:mathcal|mathrm|operatorname|Theta|Omega|tilde|widetilde)\b/.test(source)) {
    return source;
  }

  source = source
    .replace(
      /5\(n-1\)\/3下取整/g,
      "\\left\\lfloor\\frac{5(n-1)}{3}\\right\\rfloor",
    )
    .replaceAll("≤", "\\le ")
    .replaceAll("<=", "\\le ")
    .replaceAll("下取整", "\\;\\text{(floor)}")
    .replace(/min\{([^{}]*)\}/g, "\\min\\left\\{$1\\right\\}")
    .replace(/\^22\^/g, "^2 2^")
    .replace(/\^\(1\+o\(1\)\)/g, "^{1+o(1)}")
    .replace(/\^([0-9]+(?:\.[0-9]+)?)/g, "^{$1}")
    .replace(/sqrt\(([^()]*)\)/g, "\\sqrt{$1}")
    .replace(/sqrt([A-Za-z0-9]+)/g, "\\sqrt{$1}")
    .replace(/log\^\{([^{}]+)\}/g, "\\log^{$1}")
    .replace(/log(?=[A-Za-z({])/g, "\\log ")
    .replace(/,\s*/g, ",\\;")
    .replace(/\s*=\s*/g, " = ");

  // One legacy cell has a missing closing parenthesis in its source workbook.
  if (source === "O(n2^w(\\sqrt{V})") source = "O(n2^w\\sqrt{V})";

  const exponential = source.match(/^Oexp\((.*)\)(.*)$/);
  if (exponential) {
    return `\\mathcal{O}_{\\mathrm{exp}}\\!\\left(${exponential[1]}\\right)${exponential[2]}`;
  }
  const bigO = source.match(/^O\((.*)\)(.*)$/);
  if (bigO) {
    return `\\mathcal{O}\\!\\left(${bigO[1]}\\right)${bigO[2]}`;
  }
  return source;
}

function complexityNode(value, isNew = false) {
  const latex = complexityLatex(value);
  if (!latex) return pendingNode();
  const node = element("span", `complexity${isNew ? " complexity-new" : ""}`);
  if (window.katex && latex) {
    window.katex.render(latex, node, {
      throwOnError: false,
      strict: false,
      output: "htmlAndMathml",
    });
  } else {
    node.textContent = value;
  }
  node.title = value;
  return node;
}

function problemDomains(problem) {
  const taxonomy = problem.taxonomy || {};
  return [taxonomy.primaryDomain, ...(taxonomy.relatedDomains || [])].filter(Boolean);
}

function modelKey(entry) {
  return entry.modelKey || normalize(entry.model)
    .replace(/[^a-z0-9]+/g, "-")
    .replace(/^-|-$/g, "");
}

function competitionRanks(rows, scoreSelector) {
  let previousScore = null;
  let previousRank = 0;
  rows.forEach((row, index) => {
    const score = scoreSelector(row);
    if (previousScore === null || score !== previousScore) {
      previousRank = index + 1;
      previousScore = score;
    }
    row.rank = previousRank;
  });
  return rows;
}

function entriesForTrack(data) {
  if (state.track === "combined") {
    return [
      ...(data.tracks.direct || []).map((entry) => ({ ...entry, mode: "direct", harness: null })),
      ...(data.tracks.agent || []).map((entry) => ({
        ...entry,
        mode: "agent",
        harness: entry.framework || entry.harness || null,
      })),
    ];
  }
  return (data.tracks[state.track] || []).map((entry) => ({
    ...entry,
    mode: state.track,
    harness: state.track === "agent" ? entry.framework || entry.harness || null : null,
  }));
}

function renderStandardLeaderboard(data) {
  const standardProblems = state.problems.filter((problem) => problem.track === "Standard");
  const scopes = Object.fromEntries(DOMAIN_ORDER.map((domain) => [
    domain,
    standardProblems.filter((problem) => problemDomains(problem).includes(domain)),
  ]));
  const score = (entry, track, scope) => {
    const key = modelKey(entry);
    const explicitSolved = Array.isArray(entry.solvedProblemIds)
      ? new Set(entry.solvedProblemIds.map(Number))
      : null;
    const solved = scope.filter((problem) => (
      explicitSolved
        ? explicitSolved.has(problem.id)
        : Boolean(problem.generatedCode?.[track]?.[key])
    )).length;
    return { solved, total: scope.length, passRate: scope.length ? (solved * 100) / scope.length : 0 };
  };
  const rows = entriesForTrack(data).map((entry) => ({
    ...entry,
    overall: score(entry, entry.mode, standardProblems),
    domains: Object.fromEntries(
      DOMAIN_ORDER.map((domain) => [domain, score(entry, entry.mode, scopes[domain])]),
    ),
  })).sort((left, right) => (
    right.overall.solved - left.overall.solved
    || left.model.localeCompare(right.model)
    || left.mode.localeCompare(right.mode)
    || String(left.harness || "").localeCompare(String(right.harness || ""))
  ));
  competitionRanks(rows, (entry) => entry.overall.solved);

  const baseHeadings = [
    element("th", "leaderboard-rank-heading", "Rank"),
    element("th", "leaderboard-model-heading", "Model"),
  ];
  if (state.track === "combined") {
    baseHeadings.push(element("th", "leaderboard-mode-heading", "Mode"));
  }
  if (state.track === "agent" || state.track === "combined") {
    baseHeadings.push(element("th", "leaderboard-harness-heading", "Harness"));
  }
  els.leaderboardHead.closest("table").dataset.track = state.track;
  els.leaderboardHead.closest("table").dataset.kind = "standard";
  els.leaderboardHead.replaceChildren(...baseHeadings);
  DOMAIN_ORDER.forEach((domain) => {
    const heading = element("th", "domain-heading");
    heading.append(element("span", "domain-heading-name", domain));
    els.leaderboardHead.append(heading);
  });
  const overallHeading = element("th", "domain-heading overall-heading");
  overallHeading.append(element("span", "domain-heading-name", "Overall"));
  els.leaderboardHead.append(overallHeading);

  els.leaderboardBody.replaceChildren();
  rows.forEach((entry) => {
    const row = document.createElement("tr");
    const model = element("td", "leaderboard-model");
    model.append(element("strong", "leaderboard-model-name", entry.model));
    row.append(element("td", "rank", String(entry.rank).padStart(2, "0")), model);
    if (state.track === "combined") {
      row.append(element("td", "leaderboard-mode", entry.mode));
    }
    if (state.track === "agent" || state.track === "combined") {
      row.append(element("td", "leaderboard-harness", entry.harness || "—"));
    }
    DOMAIN_ORDER.forEach((domain) => {
      const result = entry.domains[domain];
      const cell = element("td", "domain-score");
      cell.append(element("strong", "domain-pass-rate", `${result.passRate.toFixed(1)}%`));
      row.append(cell);
    });
    const overall = element("td", "domain-score overall-score");
    overall.append(element("strong", "domain-pass-rate", `${entry.overall.passRate.toFixed(1)}%`));
    row.append(overall);
    els.leaderboardBody.append(row);
  });
  els.leaderboardDescription.textContent = `${standardProblems.length} standard problems. Interactive tasks are excluded from every domain and Overall denominator.`;
  return rows.length;
}

function renderInteractiveLeaderboard(data) {
  const board = data.boards[state.track] || { problemCount: 0, pointCount: 0, rows: [] };
  const rows = board.rows || [];
  const headings = [
    element("th", "leaderboard-rank-heading", "Rank"),
    element("th", "leaderboard-model-heading", "Model"),
  ];
  if (state.track === "combined") {
    headings.push(element("th", "leaderboard-mode-heading", "Mode"));
  }
  if (state.track === "agent" || state.track === "combined") {
    headings.push(element("th", "leaderboard-harness-heading", "Harness"));
  }
  headings.push(
    element("th", "interactive-score-heading", "Score"),
    element("th", "interactive-records-heading", "Records"),
    element("th", "interactive-valid-heading", "Valid points"),
  );
  const table = els.leaderboardHead.closest("table");
  table.dataset.track = state.track;
  table.dataset.kind = "interactive";
  els.leaderboardHead.replaceChildren(...headings);
  els.leaderboardBody.replaceChildren();

  rows.forEach((entry) => {
    const row = document.createElement("tr");
    const model = element("td", "leaderboard-model");
    model.append(element("strong", "leaderboard-model-name", entry.model));
    row.append(element("td", "rank", String(entry.rank).padStart(2, "0")), model);
    if (state.track === "combined") {
      row.append(element("td", "leaderboard-mode", entry.mode));
    }
    if (state.track === "agent" || state.track === "combined") {
      row.append(element("td", "leaderboard-harness", entry.harness || "—"));
    }
    row.append(
      element("td", "interactive-score", Number(entry.score).toFixed(4)),
      element("td", "interactive-stat", String(entry.records)),
      element("td", "interactive-stat", `${entry.validPoints}/${entry.totalPoints}`),
    );
    els.leaderboardBody.append(row);
  });

  const recordScope = state.track === "combined"
    ? "Direct and Agent submissions share each point record."
    : `Point records are calculated from ${state.track === "direct" ? "Direct" : "Agent"} submissions only.`;
  els.leaderboardDescription.textContent = board.problemCount
    ? `${board.problemCount} interactive problems and ${board.pointCount} scored points. ${recordScope}`
    : `Point-level interaction results have not been imported yet. ${recordScope}`;
  return rows.length;
}

function renderLeaderboard(standardData, interactiveData) {
  const interactive = state.leaderboardKind === "interactive";
  els.evaluationConfig.hidden = state.track !== "direct";
  els.interactiveMethod.hidden = !interactive;
  const rowCount = interactive
    ? renderInteractiveLeaderboard(interactiveData)
    : renderStandardLeaderboard(standardData);
  const activeData = interactive ? interactiveData : standardData;
  els.leaderboardDate.textContent = `Updated ${activeData.updated}`;
  els.leaderboardStatus.textContent = activeData.status || "Preliminary";
  els.leaderboardTableWrap.hidden = rowCount === 0;
  els.leaderboardEmpty.hidden = rowCount !== 0;
}

function matchesProblem(problem) {
  const haystack = [
    problem.id,
    problem.title,
    problem.originalTitle,
    problem.platform,
    problem.originalComplexity,
    problem.algoscaleComplexity,
    problem.taxonomy?.primaryDomain,
    ...(problem.taxonomy?.relatedDomains || []),
    ...(problem.taxonomy?.coreTags || []),
    ...(problem.originalTags || []),
  ].map(normalize).join(" ");
  const queryMatches = haystack.includes(normalize(state.query.trim()));
  return queryMatches && problem.track === state.catalogKind;
}

function appendOriginalLinks(cell, problem) {
  const links = problem.originalLinks?.length
    ? problem.originalLinks
    : problem.originalUrl
      ? [{ label: problem.originalTitle || "Original problem", url: problem.originalUrl }]
      : [];
  if (!links.length) {
    cell.append(pendingNode());
  } else {
    const list = element("div", "original-link-list");
    links.forEach((entry) => {
      const link = element("a", "original-link", `${entry.label} ↗`);
      link.href = entry.url;
      link.target = "_blank";
      link.rel = "noreferrer";
      list.append(link);
    });
    cell.append(list);
  }
  cell.append(element("span", "platform-label", problem.platform || "Pending"));
}

function catalogCell(className, label, text) {
  const cell = element("td", className, text);
  cell.dataset.label = label;
  return cell;
}

function renderStandardProblem(problem) {
  const row = document.createElement("tr");
  row.append(catalogCell("problem-id", "ID", `#${String(problem.id).padStart(2, "0")}`));
  row.append(catalogCell("problem-name", "AlgoScale problem", problem.title));

  const originalCell = catalogCell("", "Original problem");
  appendOriginalLinks(originalCell, problem);
  row.append(originalCell);

  const complexityCell = catalogCell("complexity-cell", "Complexity shift");
  const shift = element("div", "complexity-shift");
  const originalComplexity = element("div", "complexity-stage complexity-stage-original");
  originalComplexity.append(
    element("span", "complexity-label", "Original"),
    complexityNode(problem.originalComplexity),
  );
  const targetComplexity = element("div", "complexity-stage complexity-stage-target");
  targetComplexity.append(
    element("span", "complexity-label", "AlgoScale"),
    complexityNode(problem.algoscaleComplexity, true),
  );
  shift.append(originalComplexity, element("span", "complexity-arrow", "→"), targetComplexity);
  complexityCell.append(shift);
  row.append(complexityCell);

  const taxonomyCell = catalogCell("taxonomy-cell", "Classification");
  const taxonomy = problem.taxonomy || {};
  if (taxonomy.primaryDomain) {
    const primary = element("div", "taxonomy-group");
    primary.append(
      element("span", "taxonomy-label", "Primary"),
      element("span", "taxonomy-chip taxonomy-primary", taxonomy.primaryDomain),
    );
    taxonomyCell.append(primary);

    if (taxonomy.relatedDomains?.length) {
      const related = element("div", "taxonomy-group");
      related.append(element("span", "taxonomy-label", "Related"));
      const chips = element("div", "taxonomy-chips");
      taxonomy.relatedDomains.forEach((domain) => {
        chips.append(element("span", "taxonomy-chip taxonomy-related", domain));
      });
      related.append(chips);
      taxonomyCell.append(related);
    }

    if (taxonomy.coreTags?.length) {
      const topics = element("div", "taxonomy-group taxonomy-topics");
      topics.append(element("span", "taxonomy-label", "Core topics"));
      const chips = element("div", "taxonomy-chips");
      taxonomy.coreTags.forEach((tag) => {
        chips.append(element("span", "taxonomy-chip taxonomy-core", tag));
      });
      topics.append(chips);
      taxonomyCell.append(topics);
    }
  } else {
    taxonomyCell.append(pendingNode());
  }
  row.append(taxonomyCell);

  const modeCell = catalogCell("", "Mode");
  modeCell.append(element("span", `mode-pill ${problem.track.toLowerCase()}`, problem.track));
  row.append(modeCell);

  const referenceCell = catalogCell("reference-cell", "Reference");
  const referenceAssets = problem.referenceAssets || {};
  if (referenceAssets.originalStatement && referenceAssets.algoscaleStatement) {
    const statement = element("a", "reference-link reference-link-statement", "Compare");
    statement.href = `./reference.html?id=${problem.id}&asset=statementComparison&v=${SITE_RELEASE}`;
    referenceCell.append(statement);
  }
  if (referenceAssets.solution) {
    const solution = element("a", "reference-link reference-link-solution", "Solution");
    solution.href = `./reference.html?id=${problem.id}&asset=solution&v=${SITE_RELEASE}`;
    referenceCell.append(solution);
  }
  if (referenceAssets.standardSolution) {
    const standard = element("a", "reference-link", "std.cpp");
    standard.href = `./reference.html?id=${problem.id}&asset=standardSolution&v=${SITE_RELEASE}`;
    referenceCell.append(standard);
  }
  if (!referenceCell.childElementCount) {
    referenceCell.append(element("span", "code-unavailable", "—"));
  }
  row.append(referenceCell);
  return row;
}

function renderInteractiveProblem(problem) {
  const row = document.createElement("tr");
  row.append(catalogCell("problem-id", "ID", `#${String(problem.id).padStart(2, "0")}`));
  row.append(catalogCell("problem-name", "Title", problem.title));

  const originalCell = catalogCell("", "Original problem");
  appendOriginalLinks(originalCell, problem);
  row.append(originalCell);

  const statementCell = catalogCell("reference-cell", "New statement");
  if (problem.referenceAssets?.algoscaleStatement) {
    const statement = element("a", "reference-link reference-link-statement", "New statement");
    statement.href = `./reference.html?id=${problem.id}&asset=algoscaleStatement&v=${SITE_RELEASE}`;
    statementCell.append(statement);
  } else {
    statementCell.append(pendingNode());
  }
  row.append(statementCell);

  const zeroCell = catalogCell("zero-line-cell", "Zero-score line");
  const zeroCard = element("div", "zero-line-card");
  zeroCard.append(
    element("span", "complexity-label", "Zero-score line"),
    complexityNode(problem.zeroScoreLine || problem.originalComplexity, true),
  );
  zeroCell.append(zeroCard);
  row.append(zeroCell);
  return row;
}

function renderProblems() {
  const matches = state.problems.filter(matchesProblem);
  const searching = Boolean(state.query.trim());
  const visible = state.expanded || searching ? matches : matches.slice(0, 16);

  const interactive = state.catalogKind === "Interactive";
  els.catalogTable.dataset.kind = interactive ? "interactive" : "standard";
  els.problemHead.replaceChildren();
  const headings = interactive
    ? ["ID", "Title", "Original problem", "New statement", "Zero-score line"]
    : ["ID", "AlgoScale problem", "Original problem", "Complexity shift", "Classification", "Mode", "Reference"];
  headings.forEach((heading, index) => {
    els.problemHead.append(element("th", index === 0 ? "problem-id" : "", heading));
  });
  els.catalogDescription.textContent = interactive
    ? "Interactive tasks are listed separately with their original links, enhanced statements, and zero-score lines."
    : "Standard tasks retain complexity shifts, classifications, and reference assets. Interactive tasks are not mixed into this catalog."
  els.search.placeholder = interactive
    ? "Search interactive title, platform, or zero-score line"
    : "Search title, domain, tag, platform, or complexity";

  els.problemBody.replaceChildren();
  visible.forEach((problem) => {
    els.problemBody.append(interactive
      ? renderInteractiveProblem(problem)
      : renderStandardProblem(problem));
  });

  els.visibleCount.textContent = String(matches.length);
  els.empty.hidden = matches.length !== 0;
  els.showAll.hidden = searching || matches.length <= 16;
  if (!els.showAll.hidden) {
    els.showAll.firstChild.textContent = state.expanded
      ? "Show first 16 problems "
      : `Show all ${matches.length} problems `;
    els.showAll.querySelector("span").textContent = state.expanded ? "↑" : "↓";
  }
}

async function loadData() {
  const [problemResponse, leaderboardResponse, interactiveResponse] = await Promise.all([
    fetch(`./data/problems.json?v=${SITE_RELEASE}`, { cache: "no-store" }),
    fetch(`./data/leaderboard.json?v=${SITE_RELEASE}`, { cache: "no-store" }),
    fetch(`./data/interactive-leaderboard.json?v=${SITE_RELEASE}`, { cache: "no-store" }),
  ]);
  if (!problemResponse.ok || !leaderboardResponse.ok || !interactiveResponse.ok) {
    throw new Error("Unable to load benchmark data.");
  }
  state.problems = await problemResponse.json();
  const leaderboard = await leaderboardResponse.json();
  const interactiveLeaderboard = await interactiveResponse.json();
  const interactiveCount = state.problems.filter((problem) => problem.track === "Interactive").length;
  [els.heroProblemCount, els.statProblemCount].filter(Boolean).forEach((node) => {
    node.textContent = String(state.problems.length);
  });
  els.statInteractiveCount.textContent = String(interactiveCount);

  document.querySelectorAll(".leaderboard-kind-tab").forEach((tab) => {
    tab.addEventListener("click", () => {
      state.leaderboardKind = tab.dataset.leaderboardKind;
      document.querySelectorAll(".leaderboard-kind-tab").forEach((item) => {
        const active = item === tab;
        item.classList.toggle("is-active", active);
        item.setAttribute("aria-selected", String(active));
      });
      renderLeaderboard(leaderboard, interactiveLeaderboard);
    });
  });
  document.querySelectorAll(".mode-tab").forEach((tab) => {
    tab.addEventListener("click", () => {
      state.track = tab.dataset.track;
      document.querySelectorAll(".mode-tab").forEach((item) => {
        const active = item === tab;
        item.classList.toggle("is-active", active);
        item.setAttribute("aria-selected", String(active));
      });
      renderLeaderboard(leaderboard, interactiveLeaderboard);
    });
  });
  if (window.katex && els.interactiveFormula) {
    window.katex.render(
      "s_i(x)=\\begin{cases}0,&\\text{invalid or }x\\ge T_i,\\\\100\\left(\\frac{T_i-x}{T_i-P_i}\\right)^2,&P_i<x<T_i,\\\\100,&x=P_i<T_i.\\end{cases}",
      els.interactiveFormula,
      { throwOnError: false, strict: false, output: "htmlAndMathml" },
    );
  }
  renderLeaderboard(leaderboard, interactiveLeaderboard);
  renderProblems();
}

els.search.addEventListener("input", (event) => {
  state.query = event.target.value;
  renderProblems();
});

document.querySelectorAll(".catalog-kind-tab").forEach((button) => {
  button.addEventListener("click", () => {
    state.catalogKind = button.dataset.catalogKind;
    state.expanded = false;
    document.querySelectorAll(".catalog-kind-tab").forEach((item) => {
      const active = item === button;
      item.classList.toggle("is-active", active);
      item.setAttribute("aria-selected", String(active));
    });
    renderProblems();
  });
});

els.showAll.addEventListener("click", () => {
  state.expanded = !state.expanded;
  renderProblems();
});

els.menu.addEventListener("click", () => {
  const open = els.nav.classList.toggle("is-open");
  els.menu.setAttribute("aria-expanded", String(open));
  els.menu.setAttribute("aria-label", open ? "Close menu" : "Open menu");
});

els.nav.addEventListener("click", (event) => {
  if (event.target.closest("a")) {
    els.nav.classList.remove("is-open");
    els.menu.setAttribute("aria-expanded", "false");
  }
});

loadData().catch((error) => {
  console.error(error);
  els.empty.hidden = false;
  els.empty.textContent = "Benchmark data could not be loaded. Serve this directory through a local HTTP server.";
});
