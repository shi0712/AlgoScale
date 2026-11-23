"use strict";

const params = new URLSearchParams(window.location.search);
const submissionId = params.get("submission") || "";
const problemId = Number(params.get("problem"));
const release = "20261123-interactive-token-usage";
const els = {
  back: document.querySelector("#back-link"),
  problemLabel: document.querySelector("#problem-label"),
  problemTitle: document.querySelector("#problem-title"),
  mode: document.querySelector("#mode"),
  model: document.querySelector("#model"),
  harness: document.querySelector("#harness"),
  submissionId: document.querySelector("#submission-id"),
  usage: document.querySelector("#submission-usage"),
  error: document.querySelector("#error"),
  panel: document.querySelector("#source-panel"),
  status: document.querySelector("#source-status"),
  code: document.querySelector("#code"),
  raw: document.querySelector("#raw"),
  copy: document.querySelector("#copy"),
};

function fail(message) {
  els.problemTitle.textContent = "Source unavailable";
  els.error.textContent = message;
  els.error.hidden = false;
  els.panel.hidden = true;
}

function renderUsage(submission) {
  const metrics = [];
  if (submission.mode === "agent") {
    if (submission.rounds != null) metrics.push(["Interaction rounds", submission.rounds]);
    if (submission.inputTokens != null) metrics.push(["Input tokens", submission.inputTokens]);
    if (submission.cachedInputTokens != null) metrics.push(["Cached input", submission.cachedInputTokens]);
  }
  if (submission.outputTokens != null) metrics.push(["Output tokens", submission.outputTokens]);
  els.usage.replaceChildren();
  els.usage.hidden = metrics.length === 0;
  metrics.forEach(([label, value]) => {
    const group = document.createElement("div");
    const term = document.createElement("dt");
    const amount = document.createElement("dd");
    term.textContent = label;
    amount.textContent = typeof value === "number" && Number.isFinite(value)
      ? value.toLocaleString("en-US")
      : value;
    group.append(term, amount);
    els.usage.append(group);
  });
}

async function initialize() {
  if (!/^\d+$/.test(submissionId) || !Number.isInteger(problemId) || problemId <= 0) {
    throw new Error("This interactive submission URL is invalid.");
  }
  const detailName = String(problemId).padStart(3, "0");
  const response = await fetch(`./data/interactive/problems/${detailName}.json?v=${release}`, { cache: "no-store" });
  if (!response.ok) throw new Error("The submission metadata could not be loaded.");
  const detail = await response.json();
  const submission = (detail.submissions || []).find((item) => String(item.submissionId) === submissionId);
  if (!submission) throw new Error(`Submission #${submissionId} is not published for this problem.`);

  els.back.href = `./interactive-results.html?id=${problemId}&scope=${submission.mode}`;
  els.problemLabel.textContent = `Interactive problem #${String(problemId).padStart(3, "0")}`;
  els.problemTitle.textContent = detail.problem.title;
  els.mode.textContent = submission.mode === "agent" ? "Agent" : "Direct";
  els.model.textContent = submission.model;
  els.harness.textContent = submission.harness ? `Harness: ${submission.harness}` : "";
  els.submissionId.textContent = `UOJ submission #${submissionId}`;
  renderUsage(submission);
  document.title = `#${submissionId} · ${detail.problem.title} · AlgoScale`;

  const sourceResponse = await fetch(submission.sourceCodeUrl, { cache: "no-store" });
  if (!sourceResponse.ok) throw new Error(`The submitted source could not be loaded (HTTP ${sourceResponse.status}).`);
  const source = await sourceResponse.text();
  els.code.textContent = source;
  els.raw.href = submission.sourceCodeUrl;
  els.status.textContent = `${source.length.toLocaleString("en-US")} characters · exact submitted answer.code`;
  els.panel.hidden = false;
  if (window.hljs && source.length <= 120000) window.hljs.highlightElement(els.code);

  els.copy.addEventListener("click", async () => {
    try {
      await navigator.clipboard.writeText(source);
      els.copy.textContent = "Copied";
      setTimeout(() => { els.copy.textContent = "Copy"; }, 1400);
    } catch {
      els.status.textContent = "Copy failed; use Raw to open the source directly.";
    }
  });
}

initialize().catch((error) => fail(error.message));
