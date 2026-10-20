"use strict";

const state = {
  problems: [],
  query: "",
  filter: "All",
  expanded: false,
  track: "direct",
};

const els = {
  leaderboardBody: document.querySelector("#leaderboard-body"),
  leaderboardDate: document.querySelector("#leaderboard-date"),
  problemBody: document.querySelector("#problem-body"),
  visibleCount: document.querySelector("#visible-count"),
  search: document.querySelector("#problem-search"),
  showAll: document.querySelector("#show-all"),
  empty: document.querySelector("#empty-state"),
  nav: document.querySelector(".nav"),
  menu: document.querySelector(".menu-button"),
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

function complexityNode(value, isNew = false) {
  if (!value) return pendingNode();
  return element("span", `complexity${isNew ? " complexity-new" : ""}`, value);
}

function renderLeaderboard(data) {
  const rows = data.tracks[state.track] || [];
  els.leaderboardBody.replaceChildren();
  rows.forEach((entry, index) => {
    const row = document.createElement("tr");
    row.append(
      element("td", "rank", String(index + 1).padStart(2, "0")),
      element("td", "model", entry.model),
      element("td", "framework", entry.framework),
      element("td", "score", `${entry.solved}/${entry.total}`),
      element("td", "rate", `${entry.passRate.toFixed(1)}%`),
      element("td", "note", entry.note),
    );
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

    const originalComplexity = document.createElement("td");
    originalComplexity.append(complexityNode(problem.originalComplexity));
    row.append(originalComplexity);

    const newComplexity = document.createElement("td");
    newComplexity.append(complexityNode(problem.algoscaleComplexity, true));
    row.append(newComplexity);

    const modeCell = document.createElement("td");
    modeCell.append(element("span", `mode-pill ${problem.track.toLowerCase()}`, problem.track));
    row.append(modeCell);

    const codeCell = document.createElement("td");
    const codeTracks = problem.generatedCode || {};
    const codeCount = ["direct", "agent"].reduce((count, track) => {
      const models = codeTracks[track];
      return count + (models && typeof models === "object" ? Object.keys(models).length : 0);
    }, 0);
    if (codeCount) {
      const details = element("details", "code-menu");
      details.append(element("summary", "code-summary", `View code (${codeCount})`));
      const panel = element("div", "code-panel");
      [["Direct", codeTracks.direct], ["Agent", codeTracks.agent]].forEach(([trackLabel, models]) => {
        if (!models || typeof models !== "object" || !Object.keys(models).length) return;
        const group = element("div", "code-group");
        group.append(element("span", "code-track-label", trackLabel));
        Object.entries(models).forEach(([model, href]) => {
          const link = element("a", "code-link", `${model} ↗`);
          link.href = href;
          link.target = "_blank";
          link.rel = "noreferrer";
          group.append(link);
        });
        panel.append(group);
      });
      details.append(panel);
      codeCell.append(details);
    } else {
      codeCell.append(element("span", "code-unavailable", "—"));
    }
    row.append(codeCell);
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
    fetch("./data/problems.json"),
    fetch("./data/leaderboard.json"),
  ]);
  if (!problemResponse.ok || !leaderboardResponse.ok) {
    throw new Error("Unable to load benchmark data.");
  }
  state.problems = await problemResponse.json();
  const leaderboard = await leaderboardResponse.json();

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
