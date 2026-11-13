"use strict";

const SITE_RELEASE = "20261113-kimi-k31";

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
  filter: "All",
  expanded: false,
  track: "direct",
};

const els = {
  leaderboardBody: document.querySelector("#leaderboard-body"),
  leaderboardHead: document.querySelector("#leaderboard-head"),
  leaderboardDate: document.querySelector("#leaderboard-date"),
  problemBody: document.querySelector("#problem-body"),
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

function renderLeaderboard(data) {
  const scopes = Object.fromEntries(DOMAIN_ORDER.map((domain) => [
    domain,
    state.problems.filter((problem) => problemDomains(problem).includes(domain)),
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
  const rows = (data.tracks[state.track] || []).map((entry) => ({
    ...entry,
    overall: score(entry, state.track, state.problems),
    domains: Object.fromEntries(
      DOMAIN_ORDER.map((domain) => [domain, score(entry, state.track, scopes[domain])]),
    ),
  })).sort((left, right) => (
    right.overall.solved - left.overall.solved
    || right.overall.passRate - left.overall.passRate
  ));

  const baseHeadings = [
    element("th", "leaderboard-rank-heading", "Rank"),
    element("th", "leaderboard-model-heading", "Model"),
  ];
  if (state.track === "agent") {
    baseHeadings.push(element("th", "leaderboard-harness-heading", "Harness"));
  }
  els.leaderboardHead.closest("table").dataset.track = state.track;
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
  rows.forEach((entry, index) => {
    const row = document.createElement("tr");
    const model = element("td", "leaderboard-model");
    model.append(element("strong", "leaderboard-model-name", entry.model));
    row.append(element("td", "rank", String(index + 1).padStart(2, "0")), model);
    if (state.track === "agent") {
      row.append(element("td", "leaderboard-harness", entry.framework));
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
  els.leaderboardDate.textContent = `Updated ${data.updated}`;
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
  ].map(normalize).join(" ");
  const queryMatches = haystack.includes(normalize(state.query.trim()));
  const filterMatches = state.filter === "All"
    || problem.track === state.filter
    || (state.filter === "Pending" && problem.metadataStatus === "pending");
  return queryMatches && filterMatches;
}

function renderProblems() {
  const matches = state.problems.filter(matchesProblem);
  const searching = state.query.trim() || state.filter !== "All";
  const visible = state.expanded || searching ? matches : matches.slice(0, 16);

  els.problemBody.replaceChildren();
  visible.forEach((problem) => {
    const row = document.createElement("tr");
    row.append(element("td", "problem-id", `#${String(problem.id).padStart(2, "0")}`));
    row.append(element("td", "problem-name", problem.title));

    const originalCell = document.createElement("td");
    if (problem.originalUrl) {
      const link = element("a", "original-link", `${problem.originalTitle || "Original problem"} ↗`);
      link.href = problem.originalUrl;
      link.target = "_blank";
      link.rel = "noreferrer";
      originalCell.append(link);
    } else {
      originalCell.append(pendingNode());
    }
    originalCell.append(element("span", "platform-label", problem.platform || "Pending"));
    row.append(originalCell);

    const complexityCell = element("td", "complexity-cell");
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

    const taxonomyCell = element("td", "taxonomy-cell");
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

    const modeCell = document.createElement("td");
    modeCell.append(element("span", `mode-pill ${problem.track.toLowerCase()}`, problem.track));
    row.append(modeCell);

    const referenceCell = element("td", "reference-cell");
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

    els.problemBody.append(row);
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
  const [problemResponse, leaderboardResponse] = await Promise.all([
    fetch(`./data/problems.json?v=${SITE_RELEASE}`, { cache: "no-store" }),
    fetch(`./data/leaderboard.json?v=${SITE_RELEASE}`, { cache: "no-store" }),
  ]);
  if (!problemResponse.ok || !leaderboardResponse.ok) {
    throw new Error("Unable to load benchmark data.");
  }
  state.problems = await problemResponse.json();
  const leaderboard = await leaderboardResponse.json();
  const interactiveCount = state.problems.filter((problem) => problem.track === "Interactive").length;
  [els.heroProblemCount, els.statProblemCount].filter(Boolean).forEach((node) => {
    node.textContent = String(state.problems.length);
  });
  els.statInteractiveCount.textContent = String(interactiveCount);

  document.querySelectorAll(".tab").forEach((tab) => {
    tab.addEventListener("click", () => {
      state.track = tab.dataset.track;
      document.querySelectorAll(".tab").forEach((item) => {
        const active = item === tab;
        item.classList.toggle("is-active", active);
        item.setAttribute("aria-selected", String(active));
      });
      renderLeaderboard(leaderboard);
    });
  });
  renderLeaderboard(leaderboard);
  renderProblems();
}

els.search.addEventListener("input", (event) => {
  state.query = event.target.value;
  renderProblems();
});

document.querySelectorAll(".filter").forEach((button) => {
  button.addEventListener("click", () => {
    state.filter = button.dataset.filter;
    document.querySelectorAll(".filter").forEach((item) => item.classList.toggle("is-active", item === button));
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
