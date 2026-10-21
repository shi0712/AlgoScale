"use strict";

const params = new URLSearchParams(window.location.search);
const problemId = Number(params.get("id"));
const asset = params.get("asset");
const allowedAssets = new Set(["solution", "standardSolution"]);
const content = document.querySelector("#content");
const copyButton = document.querySelector("#copy");
const rawLink = document.querySelector("#raw");
const toast = document.querySelector("#toast");
let rawText = "";

function notify(message) {
  toast.textContent = message;
  toast.classList.add("show");
  clearTimeout(notify.timer);
  notify.timer = setTimeout(() => toast.classList.remove("show"), 2200);
}

function showError(message) {
  content.className = "document error";
  content.replaceChildren();
  const heading = document.createElement("h2");
  heading.textContent = "Reference unavailable";
  const detail = document.createElement("p");
  detail.textContent = message;
  content.append(heading, detail);
  copyButton.hidden = true;
  rawLink.hidden = true;
}

function renderMarkdown(markdown) {
  marked.setOptions({ gfm: true, breaks: false });
  content.innerHTML = DOMPurify.sanitize(marked.parse(markdown), {
    USE_PROFILES: { html: true },
  });
  content.querySelectorAll("pre code").forEach((block) => hljs.highlightElement(block));
  renderMathInElement(content, {
    delimiters: [
      { left: "$$", right: "$$", display: true },
      { left: "\\[", right: "\\]", display: true },
      { left: "$", right: "$", display: false },
      { left: "\\(", right: "\\)", display: false },
    ],
    throwOnError: false,
  });
}

function renderProgram(program) {
  const pre = document.createElement("pre");
  const code = document.createElement("code");
  code.className = "language-cpp";
  code.textContent = program;
  pre.append(code);
  content.replaceChildren(pre);
  hljs.highlightElement(code);
}

async function loadReference() {
  if (!Number.isInteger(problemId) || problemId <= 0 || !allowedAssets.has(asset)) {
    throw new Error("The reference URL is invalid.");
  }
  const catalogResponse = await fetch("./data/problems.json");
  if (!catalogResponse.ok) throw new Error("The problem catalog could not be loaded.");
  const problems = await catalogResponse.json();
  const problem = problems.find((item) => Number(item.id) === problemId);
  if (!problem) throw new Error(`Problem #${problemId} does not exist.`);
  const href = problem.referenceAssets?.[asset];
  if (!href) throw new Error("This artifact has not been published.");

  document.querySelector("#problem-number").textContent = `Problem #${String(problem.id).padStart(3, "0")}`;
  document.querySelector("#problem-title").textContent = problem.title;
  document.querySelector("#asset-label").textContent = asset === "solution" ? "Official solution" : "Reference implementation · C++";
  document.title = `${problem.title} · ${asset === "solution" ? "Solution" : "std.cpp"} · AlgoScale`;
  rawLink.href = href;

  const assetResponse = await fetch(href);
  if (!assetResponse.ok) throw new Error("The reference file could not be loaded.");
  rawText = await assetResponse.text();
  content.className = `document ${asset === "solution" ? "markdown-body" : "code-document"}`;
  if (asset === "solution") renderMarkdown(rawText);
  else renderProgram(rawText);
}

copyButton.addEventListener("click", async () => {
  try {
    await navigator.clipboard.writeText(rawText);
    notify("Copied to clipboard.");
  } catch {
    notify("Copy failed. Use View raw instead.");
  }
});

loadReference().catch((error) => showError(error.message));
