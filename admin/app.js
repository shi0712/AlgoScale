"use strict";

const state = { problems: [], query: "", editingId: null };
const byId = (id) => document.getElementById(id);
const els = {
  body: byId("problem-body"), search: byId("search"), empty: byId("empty"),
  total: byId("total-count"), standard: byId("standard-count"),
  interactive: byId("interactive-count"), pending: byId("pending-count"),
  add: byId("add-problem"), editor: byId("editor"), form: byId("problem-form"),
  mode: byId("editor-mode"), editorTitle: byId("editor-title"), problemId: byId("problem-id"),
  title: byId("title"), originalTitle: byId("original-title"), platform: byId("platform"),
  originalUrl: byId("original-url"), originalComplexity: byId("original-complexity"),
  targetComplexity: byId("target-complexity"), track: byId("track"),
  metadataStatus: byId("metadata-status"), remove: byId("delete-problem"), toast: byId("toast"),
};

function node(tag, className, text) {
  const item = document.createElement(tag);
  if (className) item.className = className;
  if (text !== undefined) item.textContent = text;
  return item;
}

function notify(message, error = false) {
  els.toast.textContent = message;
  els.toast.className = `toast show${error ? " error" : ""}`;
  clearTimeout(notify.timer);
  notify.timer = setTimeout(() => { els.toast.className = "toast"; }, 2800);
}

async function api(path, options = {}) {
  const response = await fetch(path, options);
  const payload = await response.json().catch(() => ({}));
  if (!response.ok) throw new Error(payload.error || `Request failed (${response.status})`);
  return payload;
}

function updateStats() {
  els.total.textContent = state.problems.length;
  els.standard.textContent = state.problems.filter((p) => p.track === "Standard").length;
  els.interactive.textContent = state.problems.filter((p) => p.track === "Interactive").length;
  els.pending.textContent = state.problems.filter((p) => p.metadataStatus === "pending").length;
}

function render() {
  const query = state.query.trim().toLocaleLowerCase();
  const problems = state.problems.filter((problem) => [
    problem.id, problem.title, problem.originalTitle, problem.platform,
    problem.originalComplexity, problem.algoscaleComplexity,
  ].join(" ").toLocaleLowerCase().includes(query));
  els.body.replaceChildren();
  problems.forEach((problem) => {
    const row = document.createElement("tr");
    row.append(node("td", "id", `#${String(problem.id).padStart(2, "0")}`));
    const titleCell = node("td", "title", problem.title);
    titleCell.append(node("span", "subtle", problem.originalTitle || "No original title"));
    row.append(titleCell);
    const source = node("td", "", problem.platform || "Pending");
    source.append(node("span", "subtle", problem.originalUrl || "No source URL"));
    row.append(source);
    const complexity = node("td", "complexity-admin", problem.originalComplexity || "pending");
    complexity.append(document.createTextNode("  →  "));
    complexity.append(node("b", "", problem.algoscaleComplexity || "pending"));
    row.append(complexity);
    const type = document.createElement("td");
    type.append(node("span", "pill", problem.track));
    row.append(type);
    const status = document.createElement("td");
    status.append(node("span", `pill ${problem.metadataStatus}`, problem.metadataStatus));
    row.append(status);
    const action = document.createElement("td");
    const edit = node("button", "edit-button", "Edit");
    edit.type = "button";
    edit.addEventListener("click", () => openEditor(problem));
    action.append(edit);
    row.append(action);
    els.body.append(row);
  });
  els.empty.hidden = problems.length !== 0;
  updateStats();
}

function openEditor(problem = null) {
  state.editingId = problem ? problem.id : null;
  els.mode.textContent = problem ? `Edit problem #${problem.id}` : "Create record";
  els.editorTitle.textContent = problem ? problem.title : "New problem";
  els.problemId.value = problem?.id || "";
  els.title.value = problem?.title || "";
  els.originalTitle.value = problem?.originalTitle || "";
  els.platform.value = problem?.platform === "Pending" ? "" : problem?.platform || "";
  els.originalUrl.value = problem?.originalUrl || "";
  els.originalComplexity.value = problem?.originalComplexity || "";
  els.targetComplexity.value = problem?.algoscaleComplexity || "";
  els.track.value = problem?.track || "Standard";
  els.metadataStatus.value = problem?.metadataStatus || "pending";
  els.remove.hidden = !problem;
  els.editor.showModal();
  els.title.focus();
}

function formPayload() {
  return {
    title: els.title.value, originalTitle: els.originalTitle.value,
    platform: els.platform.value, originalUrl: els.originalUrl.value,
    originalComplexity: els.originalComplexity.value,
    algoscaleComplexity: els.targetComplexity.value,
    track: els.track.value, metadataStatus: els.metadataStatus.value,
  };
}

async function reload() {
  state.problems = await api("/api/problems");
  render();
}

els.search.addEventListener("input", (event) => { state.query = event.target.value; render(); });
els.add.addEventListener("click", () => openEditor());
[byId("close-editor"), byId("cancel-editor")].forEach((button) => {
  button.addEventListener("click", () => els.editor.close());
});
els.form.addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    const path = state.editingId ? `/api/problems/${state.editingId}` : "/api/problems";
    await api(path, {
      method: state.editingId ? "PUT" : "POST",
      headers: { "Content-Type": "application/json", "X-AlgoScale-Admin": "1" },
      body: JSON.stringify(formPayload()),
    });
    els.editor.close();
    await reload();
    notify(state.editingId ? "Problem updated." : "Problem created.");
  } catch (error) { notify(error.message, true); }
});
els.remove.addEventListener("click", async () => {
  const problem = state.problems.find((item) => item.id === state.editingId);
  if (!problem || !window.confirm(`Delete #${problem.id} ${problem.title}? This removes its metadata row.`)) return;
  try {
    await api(`/api/problems/${problem.id}`, {
      method: "DELETE", headers: { "X-AlgoScale-Admin": "1" },
    });
    els.editor.close();
    await reload();
    notify("Problem deleted.");
  } catch (error) { notify(error.message, true); }
});

reload().catch((error) => notify(error.message, true));
