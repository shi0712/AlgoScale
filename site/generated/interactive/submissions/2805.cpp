#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;

    explicit DSU(int n) : p(n), sz(n, 1) {
        iota(p.begin(), p.end(), 0);
    }

    int find(int x) {
        return p[x] == x ? x : p[x] = find(p[x]);
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
        return true;
    }

    bool same(int a, int b) {
        return find(a) == find(b);
    }
};

struct Block {
    vector<int> edges;
    vector<int> vertices;
    vector<vector<int>> adj;
};

static void appendInt(string &s, int x) {
    char buf[16];
    int len = 0;
    do {
        buf[len++] = char('0' + x % 10);
        x /= 10;
    } while (x);
    while (len) s.push_back(buf[--len]);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;

    vector<int> eu(m), ev(m);
    vector<vector<int>> graph(n);

    for (int i = 0; i < m; ++i) {
        cin >> eu[i] >> ev[i];
        graph[eu[i]].push_back(i);
        graph[ev[i]].push_back(i);
    }

    auto other = [&](int e, int v) {
        return eu[e] == v ? ev[e] : eu[e];
    };

    // Vertex-biconnected blocks.
    vector<int> tin(n, 0), low(n, 0), edgeStack;
    vector<vector<int>> rawBlocks;
    int timer = 0;

    function<void(int, int)> tarjan = [&](int v, int parentEdge) {
        tin[v] = low[v] = ++timer;

        for (int e : graph[v]) {
            if (e == parentEdge) continue;
            int u = other(e, v);

            if (!tin[u]) {
                edgeStack.push_back(e);
                tarjan(u, e);
                low[v] = min(low[v], low[u]);

                if (low[u] >= tin[v]) {
                    vector<int> block;
                    while (true) {
                        int x = edgeStack.back();
                        edgeStack.pop_back();
                        block.push_back(x);
                        if (x == e) break;
                    }
                    rawBlocks.push_back(move(block));
                }
            } else if (tin[u] < tin[v]) {
                edgeStack.push_back(e);
                low[v] = min(low[v], tin[u]);
            }
        }
    };

    tarjan(0, -1);

    vector<Block> blocks;
    blocks.reserve(rawBlocks.size());

    for (auto &component : rawBlocks) {
        Block b;
        b.edges = move(component);
        b.adj.resize(n);

        vector<char> present(n, false);
        for (int e : b.edges) {
            b.adj[eu[e]].push_back(e);
            b.adj[ev[e]].push_back(e);
            if (!present[eu[e]]) {
                present[eu[e]] = true;
                b.vertices.push_back(eu[e]);
            }
            if (!present[ev[e]]) {
                present[ev[e]] = true;
                b.vertices.push_back(ev[e]);
            }
        }
        blocks.push_back(move(b));
    }

    // Bridges are present in every spanning tree, so process them first.
    vector<int> blockOrder(blocks.size());
    iota(blockOrder.begin(), blockOrder.end(), 0);
    stable_sort(blockOrder.begin(), blockOrder.end(), [&](int a, int b) {
        return blocks[a].edges.size() == 1 &&
               blocks[b].edges.size() != 1;
    });

    vector<int> seen(n, 0);
    int seenTimer = 0;

    auto buildTree = [&](const Block &b, int banned) -> vector<int> {
        ++seenTimer;
        vector<int> result;
        result.reserve(b.vertices.size());

        int start = -1;
        for (int v : b.vertices) {
            if (v != banned) {
                start = v;
                break;
            }
        }
        if (start == -1) return result;

        vector<int> st;
        st.push_back(start);
        seen[start] = seenTimer;

        while (!st.empty()) {
            int v = st.back();
            st.pop_back();

            for (int e : b.adj[v]) {
                int u = other(e, v);
                if (u == banned) continue;
                if (seen[u] == seenTimer) continue;

                seen[u] = seenTimer;
                result.push_back(e);
                st.push_back(u);
            }
        }

        return result;
    };

    // A global spanning tree is the union of one spanning tree per block.
    vector<vector<int>> localTree(blocks.size());
    vector<pair<int, int>> position(blocks.size());
    vector<int> globalTree;
    globalTree.reserve(n - 1);

    for (int id : blockOrder) {
        localTree[id] = buildTree(blocks[id], -1);
        int start = (int)globalTree.size();
        for (int e : localTree[id]) globalTree.push_back(e);
        position[id] = {start, (int)globalTree.size()};
    }

    DSU dsu(n);
    vector<int> status(m, -1);
    vector<int> royalEdges;
    royalEdges.reserve(n - 1);

    auto otherEndpoint = [&](int e) {
        return eu[e] ^ ev[e] ^ eu[e];
    };

    auto endpoints = [&](int e, int &a, int &b) {
        a = eu[e];
        b = ev[e];
    };

    auto ask = [&](int blockId, const vector<int> &local) -> int {
        auto [l, r] = position[blockId];
        for (int i = l; i < r; ++i) {
            globalTree[i] = local[i - l];
        }

        string command;
        command.reserve((size_t)n * 8 + 2);
        command.push_back('?');
        for (int e : globalTree) {
            command.push_back(' ');
            appendInt(command, e);
        }
        command.push_back('\n');

        cout.write(command.data(), command.size());
        cout.flush();

        int response;
        if (!(cin >> response)) exit(0);
        if (response < 0) exit(0);
        return response;
    };

    auto printAnswer = [&]() {
        string command;
        command.reserve((size_t)n * 8 + 2);
        command.push_back('!');
        for (int e : royalEdges) {
            command.push_back(' ');
            appendInt(command, e);
        }
        command.push_back('\n');

        cout.write(command.data(), command.size());
        cout.flush();
    };

    mt19937 rng(712367821);

    for (int blockId : blockOrder) {
        Block &b = blocks[blockId];

        if (b.edges.size() == 1) {
            int e = b.edges[0];
            status[e] = 1;
            royalEdges.push_back(e);
            dsu.unite(eu[e], ev[e]);
            if ((int)royalEdges.size() == n - 1) {
                printAnswer();
                return 0;
            }
            continue;
        }

        vector<int> knownRoyal(b.vertices.size(), 0);
        unordered_map<int, int> localIndex;
        localIndex.reserve(b.vertices.size() * 2);
        for (int i = 0; i < (int)b.vertices.size(); ++i) {
            localIndex[b.vertices[i]] = i;
        }

        auto mark = [&](int e, int value) -> bool {
            if (status[e] != -1) return false;
            status[e] = value;

            if (value == 1) {
                royalEdges.push_back(e);
                dsu.unite(eu[e], ev[e]);
                ++knownRoyal[localIndex[eu[e]]];
                ++knownRoyal[localIndex[ev[e]]];
            }
            return (int)royalEdges.size() == n - 1;
        };

        int root = b.vertices[0];
        for (int v : b.vertices) {
            if (b.adj[v].size() > b.adj[root].size()) root = v;
        }

        vector<char> processed(n, false);
        processed[root] = true;

        auto usableCandidates = [&](int u) {
            vector<int> candidates;
            for (int e : b.adj[u]) {
                if (status[e] != -1) continue;
                int v = other(e, u);
                if (processed[v]) continue;

                if (dsu.same(u, v)) {
                    // A known royal path already connects the endpoints.
                    status[e] = 0;
                } else {
                    candidates.push_back(e);
                }
            }
            shuffle(candidates.begin(), candidates.end(), rng);
            return candidates;
        };

        auto classifyByValues = [&](const vector<pair<int, int>> &samples,
                                    int minimum, int maximum) -> bool {
            for (auto [e, response] : samples) {
                bool royal = maximum > minimum && response == maximum;
                if (mark(e, royal ? 1 : 0)) return true;
            }
            return false;
        };

        // Process the root of this block.
        {
            int u = root;
            vector<int> candidates = usableCandidates(u);
            vector<int> forest = buildTree(b, u);

            vector<pair<int, int>> samples;
            samples.reserve(candidates.size());
            int minimum = INT_MAX, maximum = INT_MIN;
            bool varied = false;

            for (int e : candidates) {
                vector<int> local = forest;
                local.push_back(e);
                int response = ask(blockId, local);

                samples.emplace_back(e, response);
                minimum = min(minimum, response);
                maximum = max(maximum, response);

                if (minimum != maximum) {
                    if (!varied) {
                        varied = true;
                        if (classifyByValues(samples, minimum, maximum)) {
                            printAnswer();
                            return 0;
                        }
                    } else {
                        if (mark(e, response == maximum ? 1 : 0)) {
                            printAnswer();
                            return 0;
                        }
                    }
                }
            }

            if (!varied && !candidates.empty()) {
                // The block-restriction of the hidden tree is connected.
                // Thus this vertex has at least one royal edge in the block.
                for (auto [e, response] : samples) {
                    if (mark(e, 1)) {
                        printAnswer();
                        return 0;
                    }
                }
            }
        }

        // Expand a connected prefix of the block.
        while (true) {
            bool haveNext = false;
            int chosen = -1;
            int bestRoyal = -1;
            int bestDegree = -1;

            for (int v : b.vertices) {
                if (processed[v]) continue;

                bool adjacent = false;
                for (int e : b.adj[v]) {
                    if (processed[other(e, v)]) {
                        adjacent = true;
                        break;
                    }
                }
                if (!adjacent) continue;

                int score = knownRoyal[localIndex[v]];
                int degree = (int)b.adj[v].size();
                if (!haveNext || score > bestRoyal ||
                    (score == bestRoyal && degree > bestDegree)) {
                    haveNext = true;
                    chosen = v;
                    bestRoyal = score;
                    bestDegree = degree;
                }
            }

            if (!haveNext) break;

            int u = chosen;
            int anchor = -1;

            for (int e : b.adj[u]) {
                int v = other(e, u);
                if (!processed[v] || status[e] == -1) continue;
                if (anchor == -1 || status[e] == 1) anchor = e;
                if (status[e] == 1) break;
            }

            vector<int> candidates = usableCandidates(u);
            vector<int> forest = buildTree(b, u);

            vector<pair<int, int>> samples;
            samples.reserve(candidates.size());
            int minimum = INT_MAX, maximum = INT_MIN;
            bool varied = false;

            for (int e : candidates) {
                vector<int> local = forest;
                local.push_back(e);
                int response = ask(blockId, local);

                samples.emplace_back(e, response);
                minimum = min(minimum, response);
                maximum = max(maximum, response);

                if (minimum != maximum) {
                    if (!varied) {
                        varied = true;
                        if (classifyByValues(samples, minimum, maximum)) {
                            printAnswer();
                            return 0;
                        }
                    } else {
                        if (mark(e, response == maximum ? 1 : 0)) {
                            printAnswer();
                            return 0;
                        }
                    }
                }
            }

            if (!varied && !candidates.empty()) {
                if (knownRoyal[localIndex[u]] == 0) {
                    // No previously processed incident edge is royal, so at
                    // least one candidate is royal. All candidates have the
                    // same response, hence all of them are royal.
                    for (auto [e, response] : samples) {
                        if (mark(e, 1)) {
                            printAnswer();
                            return 0;
                        }
                    }
                } else {
                    // Compare the common forest with a tree using the known
                    // anchor edge.
                    vector<int> baseline = forest;
                    baseline.push_back(anchor);
                    int baseResponse = ask(blockId, baseline);
                    int anchorValue = status[anchor];

                    for (auto [e, response] : samples) {
                        int value;
                        if (baseResponse == response) {
                            value = anchorValue;
                        } else {
                            value = 1 - anchorValue;
                        }
                        if (mark(e, value)) {
                            printAnswer();
                            return 0;
                        }
                    }
                }
            }

            processed[u] = true;
        }
    }

    printAnswer();
    return 0;
}
