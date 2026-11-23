#include <bits/stdc++.h>
using namespace std;

struct Candidate {
    int root = 0;
    int sum = 0;
    uint64_t valueHash = 0;
    vector<int> f, a, sz, tin, tout;
};

struct Eval {
    int worst = 0;
    int responses = 0;
};

struct Option {
    bool valid = false;
    int worst = 0;
    int responses = 0;
    int u = 0;
    vector<int> nodes;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<vector<int>> g(n + 1);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        if (!(cin >> u >> v)) return 0;
        g[u].push_back(v);
        g[v].push_back(u);
    }

    auto ask = [&](const vector<int>& nodes) -> int {
        cout << "? 1 " << nodes.size();
        for (int v : nodes) cout << ' ' << v;
        cout << '\n' << flush;
        int response;
        if (!(cin >> response)) exit(0);
        return response;
    };

    vector<int> all(n);
    iota(all.begin(), all.end(), 1);

    int leaf = 1;
    while (g[leaf].size() != 1) ++leaf;
    int leafParent = g[leaf][0];

    vector<int> baseF(n + 1, 0);
    for (int v = 1; v <= n; ++v) {
        if (v == leaf) continue;
        baseF[v] = ask(vector<int>{v});
    }

    vector<uint64_t> coeff(n + 1);
    uint64_t seed = 0x9e3779b97f4a7c15ULL;
    auto splitmix = [&]() -> uint64_t {
        uint64_t z = (seed += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    };
    for (int v = 1; v <= n; ++v) coeff[v] = splitmix() | 1ULL;

    auto buildCandidate = [&](const vector<int>& f, int root) {
        Candidate c;
        c.root = root;
        c.f = f;
        c.a.assign(n + 1, 0);
        c.sz.assign(n + 1, 1);
        c.tin.assign(n + 1, 0);
        c.tout.assign(n + 1, 0);

        vector<int> parent(n + 1, -1), order;
        vector<int> st;
        order.reserve(n);
        st.reserve(n);

        parent[root] = 0;
        st.push_back(root);
        while (!st.empty()) {
            int v = st.back();
            st.pop_back();
            c.tin[v] = (int)order.size();
            order.push_back(v);
            for (int to : g[v]) {
                if (to == parent[v]) continue;
                parent[to] = v;
                st.push_back(to);
            }
        }

        c.a[root] = f[root];
        for (int i = 1; i < n; ++i) {
            int v = order[i];
            c.a[v] = f[v] - f[parent[v]];
        }
        for (int i = n - 1; i > 0; --i) {
            int v = order[i];
            c.sz[parent[v]] += c.sz[v];
        }
        for (int v = 1; v <= n; ++v) {
            c.tout[v] = c.tin[v] + c.sz[v] - 1;
            c.sum += f[v];
            c.valueHash += uint64_t(c.a[v] + 2) * coeff[v];
        }
        return c;
    };

    vector<Candidate> candidates;
    for (int eps : {-1, 1}) {
        baseF[leaf] = baseF[leafParent] + eps;
        for (int root = 1; root <= n; ++root) {
            bool possible = (root == leaf)
                ? (abs(baseF[leaf]) == 1)
                : (abs(baseF[root]) == 1);
            if (possible) {
                candidates.push_back(buildCandidate(baseF, root));
            }
        }
    }

    if (candidates.empty()) return 0;

    mt19937 rng(0x71b54a32u);

    int maxDegreeVertex = 1;
    for (int v = 2; v <= n; ++v) {
        if (g[v].size() > g[maxDegreeVertex].size()) maxDegreeVertex = v;
    }

    vector<int> treeParent(n + 1, -1), treeOrder(1, 1), treeSize(n + 1, 1);
    treeParent[1] = 0;
    for (int i = 0; i < (int)treeOrder.size(); ++i) {
        int v = treeOrder[i];
        for (int to : g[v]) {
            if (to == treeParent[v]) continue;
            treeParent[to] = v;
            treeOrder.push_back(to);
        }
    }
    for (int i = n - 1; i > 0; --i) {
        int v = treeOrder[i];
        treeSize[treeParent[v]] += treeSize[v];
    }
    int centroid = 1, centroidBest = n + 1;
    for (int v = 1; v <= n; ++v) {
        int mx = n - treeSize[v];
        for (int to : g[v]) {
            if (treeParent[to] == v) mx = max(mx, treeSize[to]);
        }
        if (mx < centroidBest) {
            centroidBest = mx;
            centroid = v;
        }
    }

    while (true) {
        bool sameValues = true;
        for (int i = 1; i < (int)candidates.size() && sameValues; ++i) {
            for (int v = 1; v <= n; ++v) {
                if (candidates[i].a[v] != candidates[0].a[v]) {
                    sameValues = false;
                    break;
                }
            }
        }

        if (sameValues) {
            cout << '!';
            for (int v = 1; v <= n; ++v) {
                cout << ' ' << candidates[0].a[v];
            }
            cout << '\n' << flush;
            return 0;
        }

        const int C = candidates.size();

        auto responseFor = [&](const Candidate& c, int u,
                               const vector<int>& nodes) -> int {
            int result = 0;
            int inside = 0;
            for (int v : nodes) {
                result += c.f[v];
                if (u && c.tin[u] <= c.tin[v] && c.tin[v] <= c.tout[u]) {
                    ++inside;
                }
            }
            if (u) result -= 2 * c.a[u] * inside;
            return result;
        };

        auto aggregate = [&](auto&& getResponse) -> Eval {
            struct Agg {
                int count = 0;
                uint64_t firstHash = 0;
                bool mixed = false;
            };

            unordered_map<int, Agg> groups;
            groups.reserve(C * 2 + 1);

            for (const Candidate& c : candidates) {
                int response = getResponse(c);
                Agg& ag = groups[response];
                if (ag.count == 0) {
                    ag.firstHash = c.valueHash;
                } else if (ag.firstHash != c.valueHash) {
                    ag.mixed = true;
                }
                ++ag.count;
            }

            Eval e;
            e.responses = groups.size();
            for (const auto& [response, ag] : groups) {
                (void)response;
                if (ag.mixed) e.worst = max(e.worst, ag.count);
            }
            return e;
        };

        auto consider = [&](Option& option, const Eval& e, int u,
                            const vector<int>& nodes) {
            if (e.responses <= 1) return;
            if (!option.valid || e.worst < option.worst ||
                (e.worst == option.worst && e.responses > option.responses)) {
                option.valid = true;
                option.worst = e.worst;
                option.responses = e.responses;
                option.u = u;
                option.nodes = nodes;
            }
        };

        Option bestPlain, bestFlip;

        Eval plainTotal = aggregate([&](const Candidate& c) {
            return c.sum;
        });
        consider(bestPlain, plainTotal, 0, all);

        for (int v = 1; v <= n; ++v) {
            Eval e = aggregate([&](const Candidate& c) {
                return c.f[v];
            });
            consider(bestPlain, e, 0, vector<int>{v});
        }

        for (int u = 1; u <= n; ++u) {
            Eval e = aggregate([&](const Candidate& c) {
                return c.sum - 2 * c.a[u] * c.sz[u];
            });
            consider(bestFlip, e, u, all);
        }

        int currentBest = C;
        if (bestPlain.valid) currentBest = min(currentBest, bestPlain.worst);
        if (bestFlip.valid) currentBest = min(currentBest, bestFlip.worst);

        vector<int> rootList;
        rootList.reserve(C);
        for (const Candidate& c : candidates) rootList.push_back(c.root);
        sort(rootList.begin(), rootList.end());
        rootList.erase(unique(rootList.begin(), rootList.end()), rootList.end());

        auto randomSubset = [&]() -> vector<int> {
            vector<int> nodes;
            for (int v = 1; v <= n; ++v) {
                if (rng() & 1U) nodes.push_back(v);
            }
            if (nodes.empty()) nodes.push_back(1 + int(rng() % n));
            return nodes;
        };

        auto randomRootSubset = [&]() -> vector<int> {
            vector<int> nodes;
            for (int v : rootList) {
                if (rng() & 1U) nodes.push_back(v);
            }
            if (nodes.empty()) nodes.push_back(rootList[rng() % rootList.size()]);
            return nodes;
        };

        if (C > 3 && currentBest > C / 2) {
            for (int it = 0; it < 6; ++it) {
                vector<int> nodes = (it < 3) ? randomSubset() : randomRootSubset();
                Eval e = aggregate([&](const Candidate& c) {
                    return responseFor(c, 0, nodes);
                });
                consider(bestPlain, e, 0, nodes);
            }

            if (!(bestPlain.valid && bestPlain.worst <= C / 2)) {
                vector<int> vertices;
                auto addVertex = [&](int v) {
                    if (v < 1 || v > n) return;
                    if (find(vertices.begin(), vertices.end(), v) == vertices.end()) {
                        vertices.push_back(v);
                    }
                };

                if (bestFlip.valid) addVertex(bestFlip.u);
                addVertex(maxDegreeVertex);
                addVertex(centroid);
                addVertex(candidates[0].root);
                addVertex(candidates[C / 2].root);
                addVertex(candidates[rng() % C].root);
                addVertex(1 + int(rng() % n));

                for (int u : vertices) {
                    for (int it = 0; it < 4; ++it) {
                        vector<int> nodes = (it == 3) ? randomRootSubset()
                                                      : randomSubset();
                        Eval e = aggregate([&](const Candidate& c) {
                            return responseFor(c, u, nodes);
                        });
                        consider(bestFlip, e, u, nodes);
                    }
                    if (bestFlip.valid && bestFlip.worst == 0) break;
                }
            }
        }

        Option chosen;
        if (!bestPlain.valid && !bestFlip.valid) {
            bool found = false;
            for (int i = 1; i < C && !found; ++i) {
                for (int v = 1; v <= n; ++v) {
                    if (candidates[i].f[v] != candidates[0].f[v]) {
                        chosen.valid = true;
                        chosen.u = 0;
                        chosen.nodes = vector<int>{v};
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                chosen.valid = true;
                chosen.u = candidates[0].root;
                chosen.nodes = all;
            }
        } else if (!bestPlain.valid) {
            chosen = bestFlip;
        } else if (!bestFlip.valid) {
            chosen = bestPlain;
        } else {
            long long plainGain = C - bestPlain.worst;
            long long flipGain = C - bestFlip.worst;
            chosen = (flipGain > 2 * plainGain) ? bestFlip : bestPlain;
        }

        if (chosen.u) {
            cout << "? 2 " << chosen.u << '\n' << flush;
        }
        int answer = ask(chosen.nodes);

        vector<Candidate> kept;
        kept.reserve(C);
        for (Candidate& c : candidates) {
            if (responseFor(c, chosen.u, chosen.nodes) != answer) continue;

            if (chosen.u) {
                int u = chosen.u;
                int oldValue = c.a[u];
                int delta = -2 * oldValue;

                for (int v = 1; v <= n; ++v) {
                    if (c.tin[u] <= c.tin[v] && c.tin[v] <= c.tout[u]) {
                        c.f[v] += delta;
                    }
                }
                c.sum += delta * c.sz[u];
                c.a[u] = -oldValue;
                c.valueHash += static_cast<uint64_t>(
                    static_cast<int64_t>(-2 * oldValue)
                ) * coeff[u];
            }
            kept.push_back(std::move(c));
        }

        candidates.swap(kept);
        if (candidates.empty()) return 0;
    }
}
