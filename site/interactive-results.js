"use strict";

const SITE_RELEASE = "20261121-point-results";
const params = new URLSearchParams(window.location.search);
const problemId = Number(params.get("id"));
const allowedScopes = new Set(["direct", "agent", "combined"]);
let activeScope = allowedScopes.has(params.get("scope")) ? params.get("scope") : "combined";
let detailData = null;

const els = {
  loading: document.querySelector("#loading"),
  error: document.querySelector("#error"),
  results: document.querySelector("#results"),
  problemNumber: document.querySelector("#problem-number"),
  problemTitle: document.querySelector("#problem-title"),
  problemDescription: document.querySelector("#problem-description"),
  originalLink: document.querySelector("#original-link"),
  statementLink: document.querySelector("#statement-link"),
  scopeNote: document.querySelector("#scope-note"),
  summaryPoints: document.querySelector("#summary-points"),
  summaryCompetitors: document.querySelector("#summary-competitors"),
  summaryRecords: document.querySelector("#summary-records"),
  summaryMetric: document.querySelector("#summary-metric"),
  status: document.querySelector("#status"),
  updated: document.querySelector("#updated"),
  pointList: document.querySelector("#point-list"),
};

function element(tag, className, text) {
  const node = document.createElement(tag);
  if (className) node.className = className;
  if (text !== undefined) node.textContent = text;
  return node;
}

function formatNumber(value, digits = 0) {
  if (value === null || value === undefined || Number.isNaN(Number(value))) return "—";
  return Number(value).toLocaleString("en-US", {
    minimumFractionDigits: 0,
    maximumFractionDigits: digits,
  });
}

function formatScale(raw) {
  if (!raw) return "—";
  return String(raw).split("_").map((part) => {
    const match = /^([A-Za-z]+)(\d+)$/.exec(part);
    if (!match) return part;
    return `${match[1]} = ${Number(match[2]).toLocaleString("en-US")}`;
  }).join(" · ");
}

function competitorLabel(item) {
  const mode = item.mode === "agent" ? "Agent" : "Direct";
  return `${item.model} · ${mode}`;
}

function resultStatus(result) {
  if (result.isRecord) return ["Record", "record"];
  if (result.valid) return ["Valid", "valid"];
  if (result.interactionCount !== null) return ["Zero score", "invalid"];
  return ["No valid result", "invalid"];
}

function renderResultCard(result) {
  const [statusText, statusClass] = resultStatus(result);
  const card = element("article", `result-card${result.isRecord ? " is-record" : ""}${!result.valid ? " is-invalid" : ""}`);
  const head = element("div", "result-card-head");
  const identity = element("div");
  identity.append(
    element("h3", "result-model", result.model),
    element("span", "result-mode", result.mode === "agent"
      ? `Agent · ${result.harness || "Unspecified harness"}`
      : "Direct"),
  );
  head.append(identity, element("span", `status-pill ${statusClass}`, statusText));

  const numbers = element("div", "result-numbers");
  const interactions = element("div");
  interactions.append(
    element("span", "", "Interactions"),
    element("strong", "", formatNumber(result.interactionCount, 6)),
  );
  const score = element("div");
  score.append(
    element("span", "", "Point score"),
    element("strong", "", formatNumber(result.score, 2)),
  );
  numbers.append(interactions, score);
  card.append(head, numbers);
  if (result.sourceSubmissionId) {
    card.append(element("p", "submission-id", `UOJ submission #${result.sourceSubmissionId}`));
  }
  return card;
}

function renderPoint(point) {
  const item = element("article", "point-item");
  const summary = element("button", "point-summary");
  summary.type = "button";
  summary.setAttribute("aria-expanded", "false");

  const holders = element("div", "holder-list");
  if (point.recordHolders.length) {
    point.recordHolders.forEach((holder) => {
      holders.append(element("span", "holder", competitorLabel(holder)));
    });
  } else {
    holders.append(element("span", "no-record", "No valid record"));
  }

  summary.append(
    element("span", "point-number", `#${String(point.testId).padStart(2, "0")}`),
    element("span", "point-scale", formatScale(point.scale)),
    element("span", "point-value", formatNumber(point.threshold, 6)),
    element("span", "point-value record-value", formatNumber(point.record, 6)),
    holders,
    element("span", "view-results", `${point.results.length} result${point.results.length === 1 ? "" : "s"}`),
  );

  const detail = element("div", "point-results");
  detail.hidden = true;
  const grid = element("div", "result-grid");
  point.results.forEach((result) => grid.append(renderResultCard(result)));
  detail.append(grid);

  summary.addEventListener("click", () => {
    const open = summary.getAttribute("aria-expanded") === "true";
    summary.setAttribute("aria-expanded", String(!open));
    detail.hidden = open;
  });
  item.append(summary, detail);
  return item;
}

function renderScope() {
  const scope = detailData.scopes[activeScope];
  document.querySelectorAll("[data-scope]").forEach((button) => {
    const active = button.dataset.scope === activeScope;
    button.classList.toggle("is-active", active);
    button.setAttribute("aria-selected", String(active));
    button.tabIndex = active ? 0 : -1;
  });

  const scopeDescriptions = {
    direct: "Pᵢ is calculated from Direct submissions only.",
    agent: "Pᵢ is calculated from Agent submissions only.",
    combined: "Pᵢ is shared by Direct and Agent submissions.",
  };
  els.scopeNote.textContent = `${scopeDescriptions[activeScope]} Repeated runs use each configuration's best valid result on every point.`;
  els.summaryPoints.textContent = formatNumber(scope.points.length);
  els.summaryCompetitors.textContent = formatNumber(scope.competitorCount);
  els.summaryRecords.textContent = formatNumber(scope.points.filter((point) => point.record !== null).length);
  els.summaryMetric.textContent = detailData.problem.metric.description;
  els.pointList.replaceChildren(...scope.points.map(renderPoint));
}

function showError(message) {
  els.loading.hidden = true;
  els.results.hidden = true;
  els.error.hidden = false;
  els.error.textContent = message;
}

async function loadResults() {
  if (!Number.isInteger(problemId) || problemId <= 0) {
    throw new Error("The interactive result URL is invalid.");
  }
  const detailName = String(problemId).padStart(3, "0");
  const [catalogResponse, detailResponse] = await Promise.all([
    fetch(`./data/problems.json?v=${SITE_RELEASE}`, { cache: "no-store" }),
    fetch(`./data/interactive/problems/${detailName}.json?v=${SITE_RELEASE}`, { cache: "no-store" }),
  ]);
  if (!catalogResponse.ok) throw new Error("The problem catalog could not be loaded.");
  if (!detailResponse.ok) throw new Error("Point results have not been published for this problem.");

  const problems = await catalogResponse.json();
  const problem = problems.find((item) => Number(item.id) === problemId && item.track === "Interactive");
  if (!problem) throw new Error(`Interactive problem #${problemId} does not exist.`);
  detailData = await detailResponse.json();

  els.problemNumber.textContent = `Interactive problem #${String(problemId).padStart(3, "0")}`;
  els.problemTitle.textContent = problem.title;
  els.problemDescription.textContent = `${detailData.problem.pointCount} test points measured by ${detailData.problem.metric.description}. Open any point to compare every evaluated configuration.`;
  els.originalLink.href = problem.originalUrl;
  els.statementLink.href = `./reference.html?id=${problem.id}&asset=algoscaleStatement&v=${SITE_RELEASE}`;
  els.status.textContent = detailData.status;
  els.updated.textContent = detailData.updated;
  document.title = `${problem.title} · Interactive results · AlgoScale`;

  document.querySelectorAll("[data-scope]").forEach((button) => {
    button.addEventListener("click", () => {
      activeScope = button.dataset.scope;
      const url = new URL(window.location.href);
      url.searchParams.set("scope", activeScope);
      window.history.replaceState({}, "", url);
      renderScope();
    });
  });

  renderScope();
  els.loading.hidden = true;
  els.results.hidden = false;
}

loadResults().catch((error) => showError(error.message));
