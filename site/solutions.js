"use strict";

const SITE_RELEASE = "20261121-submission-archive";
const VALID_KINDS = new Set(["standard", "interactive"]);

const state = {
  kind: "standard",
  problems: [],
  entries: [],
  filteredProblems: [],
  selectedProblemId: null,
  selectedEntry: null,
  sourceCache: new Map(),
};

const elements = {
  kindTabs: document.querySelectorAll("[data-kind]"),
  archiveNote: document.querySelector("#archive-note"),
  search: document.querySelector("#solution-search"),
  track: document.querySelector("#track-filter"),
  model: document.querySelector("#model-filter"),
  clear: document.querySelector("#clear-filters"),
  list: document.querySelector("#problem-list"),
  empty: document.querySelector("#problem-empty"),
  count: document.querySelector("#result-count"),
  detailEmpty: document.querySelector("#detail-empty"),
  detail: document.querySelector("#detail-content"),
  detailId: document.querySelector("#detail-id"),
  detailTitle: document.querySelector("#detail-title"),
  problemLink: document.querySelector("#problem-link"),
  variantLabel: document.querySelector("#variant-label"),
  variantList: document.querySelector("#variant-list"),
  variantCount: document.querySelector("#variant-count"),
  sourceMode: document.querySelector("#source-mode"),
  sourceModel: document.querySelector("#source-model"),
  sourceHarness: document.querySelector("#source-harness"),
  usage: document.querySelector("#source-usage"),
  raw: document.querySelector("#raw-source"),
  copy: document.querySelector("#copy-source"),
  code: document.querySelector("#source-code"),
  loading: document.querySelector("#loading-source"),
  status: document.querySelector("#source-status"),
  statPrograms: document.querySelector("#stat-programs"),
  statProblems: document.querySelector("#stat-problems"),
  statModels: document.querySelector("#stat-models"),
};

const padId = (id) => String(id).padStart(2, "0");
const entryKey = (entry) => [
  entry.kind,
  entry.problemId,
  entry.track,
  entry.modelKey,
  entry.harness || "",
  entry.submissionId || "",
].join(":");

function activeEntries() {
  return state.entries.filter((entry) => entry.kind === state.kind);
}

function activeProblems() {
  const trackName = state.kind === "standard" ? "Standard" : "Interactive";
  return state.problems.filter((problem) => problem.track === trackName);
}

function matchingEntries() {
  const query = elements.search.value.trim().toLowerCase();
  const track = elements.track.value;
  const model = elements.model.value;
  return activeEntries().filter((entry) => {
    const haystack = `${entry.problemId} ${entry.title} ${entry.model} ${entry.track} ${entry.harness || ""}`.toLowerCase();
    return (!query || haystack.includes(query))
      && (track === "all" || entry.track === track)
      && (model === "all" || entry.modelKey === model);
  });
}

function updateUrl(entry) {
  const url = new URL(window.location.href);
  url.searchParams.set("kind", state.kind);
  if (entry) {
    url.searchParams.set("problem", entry.problemId);
    url.searchParams.set("track", entry.track);
    url.searchParams.set("model", entry.modelKey);
  } else {
    ["problem", "track", "model"].forEach((key) => url.searchParams.delete(key));
  }
  history.replaceState(null, "", url);
}

function populateModels() {
  const current = elements.model.value;
  const models = [...new Map(activeEntries().map((entry) => [entry.modelKey, entry.model])).entries()]
    .sort((a, b) => a[1].localeCompare(b[1]));
  elements.model.replaceChildren(new Option("All models", "all"));
  models.forEach(([key, label]) => elements.model.append(new Option(label, key)));
  elements.model.value = models.some(([key]) => key === current) ? current : "all";
}

function updateArchiveContext() {
  elements.kindTabs.forEach((button) => {
    const active = button.dataset.kind === state.kind;
    button.classList.toggle("is-active", active);
    button.setAttribute("aria-selected", String(active));
    button.tabIndex = active ? 0 : -1;
  });
  const entries = activeEntries();
  elements.archiveNote.textContent = state.kind === "standard"
    ? "Standard publishes only C++ programs that received 100 points under the current judge data."
    : "Interactive publishes the one current program used to score each model, mode, and problem; partial-score programs remain visible.";
  elements.variantLabel.textContent = state.kind === "standard"
    ? "Full-score variants"
    : "Current scoring variants";
  elements.statPrograms.textContent = entries.length;
  elements.statProblems.textContent = new Set(entries.map((entry) => entry.problemId)).size;
  elements.statModels.textContent = new Set(entries.map((entry) => entry.modelKey)).size;
  populateModels();
}

function renderProblems() {
  const byProblem = new Map();
  matchingEntries().forEach((entry) => {
    if (!byProblem.has(entry.problemId)) byProblem.set(entry.problemId, []);
    byProblem.get(entry.problemId).push(entry);
  });
  state.filteredProblems = activeProblems()
    .filter((problem) => byProblem.has(problem.id))
    .map((problem) => ({ ...problem, variants: byProblem.get(problem.id) }));
  elements.count.textContent = state.filteredProblems.length;
  elements.empty.hidden = state.filteredProblems.length > 0;
  elements.list.replaceChildren();

  state.filteredProblems.forEach((problem) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = "problem-row";
    button.dataset.problemId = problem.id;
    button.classList.toggle("is-active", problem.id === state.selectedProblemId);
    button.innerHTML = `
      <span class="problem-row-id">#${padId(problem.id)}</span>
      <span class="problem-row-title"></span>
      <span class="problem-row-count">${problem.variants.length}</span>`;
    button.querySelector(".problem-row-title").textContent = problem.title;
    button.addEventListener("click", () => selectProblem(problem.id));
    elements.list.append(button);
  });

  if (!state.filteredProblems.length) {
    state.selectedProblemId = null;
    state.selectedEntry = null;
    elements.detail.hidden = true;
    elements.detailEmpty.hidden = false;
    updateUrl(null);
    return;
  }
  if (!state.filteredProblems.some((problem) => problem.id === state.selectedProblemId)) {
    selectProblem(state.filteredProblems[0].id);
  } else {
    renderDetail();
  }
}

function selectProblem(problemId, preferredEntry = null) {
  state.selectedProblemId = Number(problemId);
  const problem = state.filteredProblems.find((item) => item.id === state.selectedProblemId);
  if (!problem) return;
  state.selectedEntry = preferredEntry
    && problem.variants.some((entry) => entryKey(entry) === entryKey(preferredEntry))
    ? preferredEntry
    : problem.variants[0];
  elements.list.querySelectorAll(".problem-row").forEach((row) => {
    row.classList.toggle("is-active", Number(row.dataset.problemId) === state.selectedProblemId);
  });
  renderDetail();
}

function renderDetail() {
  const problem = state.filteredProblems.find((item) => item.id === state.selectedProblemId);
  if (!problem || !state.selectedEntry) return;
  elements.detailEmpty.hidden = true;
  elements.detail.hidden = false;
  elements.detailId.textContent = `${state.kind === "standard" ? "Standard" : "Interactive"} problem #${padId(problem.id)}`;
  elements.detailTitle.textContent = problem.title;
  elements.problemLink.href = state.kind === "standard"
    ? `./?problem=${problem.id}#catalog`
    : `./interactive-results.html?id=${problem.id}&scope=${state.selectedEntry.track}`;
  elements.problemLink.textContent = state.kind === "standard" ? "Problem metadata" : "Point results";
  elements.variantCount.textContent = `${problem.variants.length} program${problem.variants.length === 1 ? "" : "s"}`;
  elements.variantList.replaceChildren();

  problem.variants
    .slice()
    .sort((a, b) => a.track.localeCompare(b.track) || a.model.localeCompare(b.model))
    .forEach((entry) => {
      const button = document.createElement("button");
      button.type = "button";
      button.className = "variant-button";
      button.classList.toggle("is-active", entryKey(entry) === entryKey(state.selectedEntry));
      button.setAttribute("role", "listitem");
      button.innerHTML = `
        <span class="variant-mode">${entry.track}</span>
        <span class="variant-model"></span>
        <span class="variant-harness"></span>`;
      button.querySelector(".variant-model").textContent = entry.model;
      button.querySelector(".variant-harness").textContent = entry.harness || "Direct API";
      button.addEventListener("click", () => {
        state.selectedEntry = entry;
        renderDetail();
      });
      elements.variantList.append(button);
    });
  loadSource(state.selectedEntry);
}

async function loadSource(entry) {
  const requestKey = entryKey(entry);
  elements.sourceMode.textContent = entry.track;
  elements.sourceModel.textContent = entry.model;
  elements.sourceHarness.textContent = entry.harness ? `Harness: ${entry.harness}` : "";
  renderMetrics(entry);
  elements.raw.href = entry.path;
  elements.loading.hidden = false;
  elements.code.textContent = "";
  elements.code.removeAttribute("data-highlighted");
  elements.status.textContent = "";
  updateUrl(entry);
  try {
    let source = state.sourceCache.get(entry.path);
    if (source === undefined) {
      const response = await fetch(entry.path);
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      source = await response.text();
      state.sourceCache.set(entry.path, source);
    }
    if (!state.selectedEntry || entryKey(state.selectedEntry) !== requestKey) return;
    elements.code.textContent = source;
    if (window.hljs && source.length <= 120000) window.hljs.highlightElement(elements.code);
    elements.status.textContent = entry.kind === "standard"
      ? `${entry.bytes.toLocaleString()} bytes · SHA-256 ${entry.sha256.slice(0, 12)}…`
      : `${source.length.toLocaleString()} characters · UOJ submission #${entry.submissionId}`;
  } catch (error) {
    elements.code.textContent = "// Source unavailable.";
    elements.status.textContent = `Could not load this source (${error.message}).`;
  } finally {
    if (state.selectedEntry && entryKey(state.selectedEntry) === requestKey) elements.loading.hidden = true;
  }
}

function renderMetrics(entry) {
  const metrics = [];
  if (entry.kind === "interactive") {
    metrics.push(["Problem score", Number(entry.score).toLocaleString("en-US", { maximumFractionDigits: 2 })]);
    metrics.push(["Correct points", `${entry.successfulPoints}/${entry.pointCount}`]);
    metrics.push(["Scoring points", `${entry.validScoringPoints}/${entry.pointCount}`]);
  } else {
    if (entry.track === "agent") {
      metrics.push(["Interaction rounds", entry.rounds]);
      metrics.push(["Input tokens", entry.inputTokens]);
      metrics.push(["Cached input", entry.cachedInputTokens]);
    }
    metrics.push(["Output tokens", entry.outputTokens]);
  }
  elements.usage.replaceChildren();
  metrics.forEach(([label, value]) => {
    const group = document.createElement("div");
    const term = document.createElement("dt");
    const amount = document.createElement("dd");
    term.textContent = label;
    amount.textContent = typeof value === "number" && Number.isFinite(value)
      ? value.toLocaleString("en-US")
      : value ?? "Unavailable";
    group.append(term, amount);
    elements.usage.append(group);
  });
}

function switchKind(kind) {
  if (!VALID_KINDS.has(kind) || kind === state.kind) return;
  state.kind = kind;
  state.selectedProblemId = null;
  state.selectedEntry = null;
  updateArchiveContext();
  renderProblems();
}

async function copySource() {
  const source = elements.code.textContent;
  if (!source) return;
  try {
    await navigator.clipboard.writeText(source);
    elements.copy.textContent = "Copied";
    setTimeout(() => { elements.copy.textContent = "Copy"; }, 1400);
  } catch {
    elements.status.textContent = "Copy failed; use the Raw link to open the source.";
  }
}

async function initialize() {
  try {
    const [problemResponse, standardResponse, interactiveResponse] = await Promise.all([
      fetch(`./data/problems.json?v=${SITE_RELEASE}`, { cache: "no-store" }),
      fetch(`./data/accepted-solutions.json?v=${SITE_RELEASE}`, { cache: "no-store" }),
      fetch(`./data/interactive-submissions.json?v=${SITE_RELEASE}`, { cache: "no-store" }),
    ]);
    if (!problemResponse.ok || !standardResponse.ok || !interactiveResponse.ok) {
      throw new Error("Archive metadata is unavailable.");
    }
    const [problems, standardManifest, interactiveManifest] = await Promise.all([
      problemResponse.json(), standardResponse.json(), interactiveResponse.json(),
    ]);
    const standardProblemIds = new Set(
      problems.filter((problem) => problem.track === "Standard").map((problem) => problem.id),
    );
    const interactiveProblemIds = new Set(
      problems.filter((problem) => problem.track === "Interactive").map((problem) => problem.id),
    );
    state.problems = problems.slice().sort((a, b) => a.id - b.id);
    state.entries = [
      ...standardManifest.entries
        .filter((entry) => standardProblemIds.has(entry.problemId))
        .map((entry) => ({ ...entry, kind: "standard" })),
      ...interactiveManifest.entries
        .filter((entry) => interactiveProblemIds.has(entry.problemId))
        .map((entry) => ({ ...entry, kind: "interactive" })),
    ].sort((a, b) => a.kind.localeCompare(b.kind)
      || a.problemId - b.problemId
      || a.track.localeCompare(b.track)
      || a.model.localeCompare(b.model));

    const params = new URLSearchParams(window.location.search);
    state.kind = VALID_KINDS.has(params.get("kind")) ? params.get("kind") : "standard";
    updateArchiveContext();
    const requestedTrack = params.get("track");
    const requestedModel = params.get("model");
    if (["direct", "agent"].includes(requestedTrack)) elements.track.value = requestedTrack;
    if ([...elements.model.options].some((option) => option.value === requestedModel)) {
      elements.model.value = requestedModel;
    }
    renderProblems();

    const requestedProblem = Number(params.get("problem"));
    const requestedEntry = activeEntries().find((entry) => entry.problemId === requestedProblem
      && (!requestedTrack || entry.track === requestedTrack)
      && (!requestedModel || entry.modelKey === requestedModel));
    if (requestedEntry && state.filteredProblems.some((problem) => problem.id === requestedProblem)) {
      selectProblem(requestedProblem, requestedEntry);
    }
  } catch (error) {
    elements.detailEmpty.replaceChildren();
    const marker = document.createElement("span");
    const message = document.createElement("p");
    marker.textContent = "!";
    message.textContent = error.message;
    elements.detailEmpty.append(marker, message);
  }
}

[elements.search, elements.track, elements.model].forEach((control) => {
  control.addEventListener(control === elements.search ? "input" : "change", renderProblems);
});
elements.kindTabs.forEach((button) => {
  button.addEventListener("click", () => switchKind(button.dataset.kind));
});
elements.clear.addEventListener("click", () => {
  elements.search.value = "";
  elements.track.value = "all";
  elements.model.value = "all";
  renderProblems();
});
elements.copy.addEventListener("click", copySource);

initialize();
