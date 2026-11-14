const state = {
  problems: [],
  entries: [],
  filteredProblems: [],
  selectedProblemId: null,
  selectedEntry: null,
  sourceCache: new Map(),
};

const elements = {
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
  variantList: document.querySelector("#variant-list"),
  variantCount: document.querySelector("#variant-count"),
  sourceMode: document.querySelector("#source-mode"),
  sourceModel: document.querySelector("#source-model"),
  sourceHarness: document.querySelector("#source-harness"),
  raw: document.querySelector("#raw-source"),
  copy: document.querySelector("#copy-source"),
  code: document.querySelector("#source-code"),
  loading: document.querySelector("#loading-source"),
  status: document.querySelector("#source-status"),
};

const padId = (id) => String(id).padStart(2, "0");
const entryKey = (entry) => `${entry.problemId}:${entry.track}:${entry.modelKey}`;

function matchingEntries() {
  const query = elements.search.value.trim().toLowerCase();
  const track = elements.track.value;
  const model = elements.model.value;

  return state.entries.filter((entry) => {
    const haystack = `${entry.problemId} ${entry.title} ${entry.model} ${entry.track} ${entry.harness || ""}`.toLowerCase();
    return (!query || haystack.includes(query))
      && (track === "all" || entry.track === track)
      && (model === "all" || entry.modelKey === model);
  });
}

function updateUrl(entry) {
  if (!entry) return;
  const url = new URL(window.location.href);
  url.searchParams.set("problem", entry.problemId);
  url.searchParams.set("track", entry.track);
  url.searchParams.set("model", entry.modelKey);
  history.replaceState(null, "", url);
}

function renderProblems() {
  const entries = matchingEntries();
  const byProblem = new Map();
  entries.forEach((entry) => {
    if (!byProblem.has(entry.problemId)) byProblem.set(entry.problemId, []);
    byProblem.get(entry.problemId).push(entry);
  });

  state.filteredProblems = state.problems
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

  const chosen = preferredEntry && problem.variants.some((entry) => entryKey(entry) === entryKey(preferredEntry))
    ? preferredEntry
    : problem.variants[0];
  state.selectedEntry = chosen;

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
  elements.detailId.textContent = `Problem #${padId(problem.id)}`;
  elements.detailTitle.textContent = problem.title;
  elements.problemLink.href = `./?problem=${problem.id}#catalog`;
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
    if (window.hljs) window.hljs.highlightElement(elements.code);
    elements.status.textContent = `${entry.bytes.toLocaleString()} bytes · SHA-256 ${entry.sha256.slice(0, 12)}…`;
  } catch (error) {
    elements.code.textContent = "// Source unavailable.";
    elements.status.textContent = `Could not load this source (${error.message}).`;
  } finally {
    if (state.selectedEntry && entryKey(state.selectedEntry) === requestKey) elements.loading.hidden = true;
  }
}

function populateModels() {
  const models = [...new Map(state.entries.map((entry) => [entry.modelKey, entry.model])).entries()]
    .sort((a, b) => a[1].localeCompare(b[1]));
  models.forEach(([key, label]) => {
    const option = document.createElement("option");
    option.value = key;
    option.textContent = label;
    elements.model.append(option);
  });
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
    const [problemResponse, solutionResponse] = await Promise.all([
      fetch("./data/problems.json"),
      fetch("./data/accepted-solutions.json"),
    ]);
    if (!problemResponse.ok || !solutionResponse.ok) throw new Error("Archive metadata is unavailable.");
    const [problems, manifest] = await Promise.all([problemResponse.json(), solutionResponse.json()]);
    state.problems = problems.slice().sort((a, b) => a.id - b.id);
    state.entries = manifest.entries.slice().sort((a, b) => a.problemId - b.problemId || a.track.localeCompare(b.track) || a.model.localeCompare(b.model));

    document.querySelector("#stat-programs").textContent = state.entries.length;
    document.querySelector("#stat-problems").textContent = new Set(state.entries.map((entry) => entry.problemId)).size;
    document.querySelector("#stat-models").textContent = new Set(state.entries.map((entry) => entry.modelKey)).size;
    populateModels();

    const params = new URLSearchParams(window.location.search);
    const requestedProblem = Number(params.get("problem"));
    const requestedTrack = params.get("track");
    const requestedModel = params.get("model");
    if (["direct", "agent"].includes(requestedTrack)) elements.track.value = requestedTrack;
    if ([...elements.model.options].some((option) => option.value === requestedModel)) elements.model.value = requestedModel;
    renderProblems();

    const requestedEntry = state.entries.find((entry) => entry.problemId === requestedProblem
      && (!requestedTrack || entry.track === requestedTrack)
      && (!requestedModel || entry.modelKey === requestedModel));
    if (requestedEntry && state.filteredProblems.some((problem) => problem.id === requestedProblem)) {
      selectProblem(requestedProblem, requestedEntry);
    }
  } catch (error) {
    elements.detailEmpty.innerHTML = `<span>!</span><p>${error.message}</p>`;
  }
}

[elements.search, elements.track, elements.model].forEach((control) => {
  control.addEventListener(control === elements.search ? "input" : "change", renderProblems);
});
elements.clear.addEventListener("click", () => {
  elements.search.value = "";
  elements.track.value = "all";
  elements.model.value = "all";
  renderProblems();
});
elements.copy.addEventListener("click", copySource);

initialize();
