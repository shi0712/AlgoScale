"use strict";

const SITE_RELEASE = "20261113-solutions";

const MATH_DELIMITERS = [
  { left: "$$", right: "$$", display: true },
  { left: "\\[", right: "\\]", display: true },
  { left: "$", right: "$", display: false },
  { left: "\\(", right: "\\)", display: false },
];

// Keep TeX opaque to Markdown; otherwise escapes and emphasis change the formula.
// Code spans and code blocks still use marked's own tokenizers.
function mathToken(source, displayOnly = false) {
  const delimiter = MATH_DELIMITERS.find(({ left, display }) =>
    (!displayOnly || display) && source.startsWith(left));
  if (!delimiter) return;
  let braces = 0;
  for (let i = delimiter.left.length; i < source.length; i += 1) {
    if (braces <= 0 && source.startsWith(delimiter.right, i)) {
      return { raw: source.slice(0, i + delimiter.right.length) };
    }
    // Match KaTeX auto-render's handling of escaped characters and brace groups.
    if (source[i] === "\\") i += 1;
    else if (source[i] === "{") braces += 1;
    else if (source[i] === "}") braces -= 1;
  }
}

function escapeMathHtml(text) {
  return text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

marked.use({
  tokenizer: {
    emStrong(source, maskedSource, previousCharacter) {
      if (!/^[_*]/.test(source)) return false;
      // Outer emphasis is parsed before its inline children. Hide TeX's own
      // emphasis characters from that lookahead as well as from tokenization.
      const offset = maskedSource.length - source.length;
      const openings = /\$|\\\(|\\\[/g;
      let match;
      while ((match = openings.exec(source))) {
        const token = mathToken(source.slice(match.index));
        if (!token) continue;
        const start = offset + match.index;
        maskedSource = maskedSource.slice(0, start) + "a".repeat(token.raw.length)
          + maskedSource.slice(start + token.raw.length);
        openings.lastIndex = match.index + token.raw.length;
      }
      return marked.Tokenizer.prototype.emStrong.call(this, source, maskedSource, previousCharacter);
    },
  },
  extensions: [
    {
      name: "mathBlock",
      level: "block",
      start(source) {
        const match = /(?:^|\n) {0,3}(?:\$\$|\\\[)/.exec(source);
        return match ? match.index + (match[0].startsWith("\n") ? 1 : 0) : undefined;
      },
      tokenizer(source) {
        const indent = /^ {0,3}(?=\$\$|\\\[)/.exec(source);
        if (!indent) return;
        const token = mathToken(source.slice(indent[0].length), true);
        if (!token) return;
        const end = /^[ \t]*(?:\n|$)/.exec(source.slice(indent[0].length + token.raw.length));
        if (!end) return;
        return { type: "mathBlock", raw: indent[0] + token.raw + end[0], text: token.raw };
      },
      renderer(token) {
        return `<div>${escapeMathHtml(token.text)}</div>\n`;
      },
    },
    {
      name: "mathInline",
      level: "inline",
      start(source) {
        const index = source.search(/\$|\\\(|\\\[/);
        return index < 0 ? undefined : index;
      },
      tokenizer(source) {
        const token = mathToken(source);
        if (token) return { type: "mathInline", raw: token.raw, text: token.raw };
      },
      renderer(token) {
        return `<span>${escapeMathHtml(token.text)}</span>`;
      },
    },
  ],
});

const params = new URLSearchParams(window.location.search);
const problemId = Number(params.get("id"));
const asset = params.get("asset") || "statementComparison";
const allowedAssets = new Set([
  "statementComparison",
  "originalStatement",
  "algoscaleStatement",
  "solution",
  "standardSolution",
]);
const content = document.querySelector("#content");
const copyButton = document.querySelector("#copy");
const rawLink = document.querySelector("#raw");
const statementModes = document.querySelector("#statement-modes");
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

function renderMarkdownInto(target, markdown) {
  marked.setOptions({ gfm: true, breaks: false });
  target.innerHTML = DOMPurify.sanitize(marked.parse(markdown), {
    USE_PROFILES: { html: true },
  });
  if (window.hljs) {
    target.querySelectorAll("pre code").forEach((block) => window.hljs.highlightElement(block));
  }
  if (window.renderMathInElement) {
    window.renderMathInElement(target, {
      delimiters: MATH_DELIMITERS,
      throwOnError: false,
    });
  }
}

function renderMarkdown(markdown) {
  renderMarkdownInto(content, markdown);
}

function renderProgram(program) {
  const pre = document.createElement("pre");
  const code = document.createElement("code");
  code.className = "language-cpp";
  code.textContent = program;
  pre.append(code);
  content.replaceChildren(pre);
  if (window.hljs) window.hljs.highlightElement(code);
}

function cacheBusted(href) {
  return `${href}${href.includes("?") ? "&" : "?"}v=${SITE_RELEASE}`;
}

async function fetchText(href) {
  const response = await fetch(cacheBusted(href), { cache: "no-store" });
  if (!response.ok) throw new Error("A statement file could not be loaded.");
  return response.text();
}

function setupStatementModes(problem) {
  const modes = [
    ["statementComparison", "Compare"],
    ["originalStatement", "Original"],
    ["algoscaleStatement", "AlgoScale"],
  ];
  statementModes.replaceChildren();
  modes.forEach(([mode, label]) => {
    const link = document.createElement("a");
    link.className = `statement-mode${asset === mode ? " is-active" : ""}`;
    link.href = `./reference.html?id=${problem.id}&asset=${mode}&v=${SITE_RELEASE}`;
    link.textContent = label;
    if (asset === mode) link.setAttribute("aria-current", "page");
    statementModes.append(link);
  });
  statementModes.hidden = false;
}

function statementPane({ label, title, markdown, rawHref, sourceHref, accent = false }) {
  const pane = document.createElement("section");
  pane.className = `statement-pane${accent ? " statement-pane-algoscale" : ""}`;

  const header = document.createElement("header");
  header.className = "statement-pane-head";
  const heading = document.createElement("div");
  const eyebrow = document.createElement("span");
  eyebrow.className = "statement-pane-label";
  eyebrow.textContent = label;
  const titleNode = document.createElement("h2");
  titleNode.textContent = title;
  heading.append(eyebrow, titleNode);

  const actions = document.createElement("div");
  actions.className = "statement-pane-actions";
  if (sourceHref) {
    const source = document.createElement("a");
    source.href = sourceHref;
    source.target = "_blank";
    source.rel = "noreferrer";
    source.textContent = "Source ↗";
    actions.append(source);
  }
  const raw = document.createElement("a");
  raw.href = rawHref;
  raw.textContent = "Raw ↗";
  actions.append(raw);
  header.append(heading, actions);

  const documentNode = document.createElement("article");
  documentNode.className = "statement-document markdown-body";
  renderMarkdownInto(documentNode, markdown);
  pane.append(header, documentNode);
  return pane;
}

function renderStatementComparison(problem, originalMarkdown, algoscaleMarkdown) {
  content.className = "statement-comparison";
  content.replaceChildren(
    statementPane({
      label: "Original statement",
      title: problem.originalTitle || "Original problem",
      markdown: originalMarkdown,
      rawHref: problem.referenceAssets.originalStatement,
      sourceHref: problem.originalUrl,
    }),
    statementPane({
      label: "AlgoScale statement",
      title: problem.title,
      markdown: algoscaleMarkdown,
      rawHref: problem.referenceAssets.algoscaleStatement,
      accent: true,
    }),
  );
  rawText = [
    `# Original statement — ${problem.originalTitle || "Original problem"}`,
    originalMarkdown.trim(),
    `# AlgoScale statement — ${problem.title}`,
    algoscaleMarkdown.trim(),
    "",
  ].join("\n\n");
  copyButton.textContent = "Copy both";
  rawLink.hidden = true;
}

async function loadReference() {
  if (!Number.isInteger(problemId) || problemId <= 0 || !allowedAssets.has(asset)) {
    throw new Error("The reference URL is invalid.");
  }
  const catalogResponse = await fetch(`./data/problems.json?v=${SITE_RELEASE}`, {
    cache: "no-store",
  });
  if (!catalogResponse.ok) throw new Error("The problem catalog could not be loaded.");
  const problems = await catalogResponse.json();
  const problem = problems.find((item) => Number(item.id) === problemId);
  if (!problem) throw new Error(`Problem #${problemId} does not exist.`);
  const isStatement = ["statementComparison", "originalStatement", "algoscaleStatement"].includes(asset);
  if (isStatement) setupStatementModes(problem);

  document.querySelector("#problem-number").textContent = `Problem #${String(problem.id).padStart(3, "0")}`;
  document.querySelector("#problem-title").textContent = problem.title;
  const assetMetadata = {
    statementComparison: { label: "Original and AlgoScale statements", title: "Statement comparison" },
    originalStatement: { label: "Original problem statement · English", title: "Statement" },
    algoscaleStatement: { label: "AlgoScale problem statement · English", title: "Statement" },
    solution: { label: "Official solution", title: "Solution" },
    standardSolution: { label: "Reference implementation · C++", title: "std.cpp" },
  }[asset];
  document.querySelector("#asset-label").textContent = assetMetadata.label;
  document.title = `${problem.title} · ${assetMetadata.title} · AlgoScale`;

  if (asset === "statementComparison") {
    const originalHref = problem.referenceAssets?.originalStatement;
    const algoscaleHref = problem.referenceAssets?.algoscaleStatement;
    if (!originalHref || !algoscaleHref) throw new Error("Both statements have not been published.");
    const [originalMarkdown, algoscaleMarkdown] = await Promise.all([
      fetchText(originalHref),
      fetchText(algoscaleHref),
    ]);
    renderStatementComparison(problem, originalMarkdown, algoscaleMarkdown);
    return;
  }

  const href = problem.referenceAssets?.[asset];
  if (!href) throw new Error("This artifact has not been published.");
  rawLink.hidden = false;
  rawLink.href = href;
  copyButton.textContent = "Copy";

  rawText = await fetchText(href);
  const isMarkdown = asset !== "standardSolution";
  content.className = `document ${isMarkdown ? "markdown-body" : "code-document"}`;
  if (isMarkdown) renderMarkdown(rawText);
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
