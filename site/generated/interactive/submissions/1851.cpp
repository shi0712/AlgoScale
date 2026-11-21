#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p;
    DSU(int n = 0) : p(n, -1) {}
    int root(int x) { return p[x] < 0 ? x : p[x] = root(p[x]); }
    bool join(int a, int b) {
        a = root(a); b = root(b);
        if (a == b) return false;
        if (p[a] > p[b]) swap(a, b);
        p[a] += p[b]; p[b] = a;
        return true;
    }
};

struct Rollback {
    vector<int> p;
    vector<array<int, 4>> history;
    explicit Rollback(int n) : p(n, -1) {}
    int root(int a) const {
        while (p[a] >= 0) a = p[a];
        return a;
    }
    bool join(int a, int b) {
        a = root(a); b = root(b);
        if (a == b) return false;
        if (p[a] > p[b]) swap(a, b);
        history.push_back({a, b, p[a], p[b]});
        p[a] += p[b]; p[b] = a;
        return true;
    }
    void undo(int s) {
        while (int(history.size()) > s) {
            auto [a, b, x, y] = history.back();
            history.pop_back();
            p[a] = x; p[b] = y;
        }
    }
};

struct Group {
    vector<int> edges;
    int k;
};

struct Split {
    vector<int> a, b;
    int k, lo, hi;
    int size() const { return int(a.size() + b.size()); }
};

struct Pattern {
    struct Node {
        int r, l = -1, h = -1, leaf = -1;
        vector<int> extra;
    };
    using Row = vector<pair<int, int>>;
    vector<Node> nodes;
    vector<Row> rows;
    vector<pair<int, int>> blocks;
    vector<int> leaves, binary, frequency;

    int slot() {
        frequency.push_back(0);
        return int(frequency.size()) - 1;
    }
    pair<int, vector<Row>> build(int r) {
        int id = int(nodes.size());
        nodes.push_back(Node{r, -1, -1, -1, {}});
        if (r == 1) {
            int i = slot();
            leaves.push_back(i);
            nodes[id].leaf = i;
            return {id, {{{i, 1}}}};
        }
        int h = r / 2;
        auto [left, a] = build(h);
        auto [right, b] = build(h);
        nodes[id].l = left;
        nodes[id].h = right;
        vector<Row> out(r);
        for (int i = 0; i < h; ++i) {
            out[i] = a[i];
            out[h+i] = move(a[i]);
            for (auto [s, sign] : b[i]) {
                out[i].emplace_back(s, sign);
                out[h+i].emplace_back(s, -sign);
            }
            int x = slot();
            binary.push_back(x);
            nodes[id].extra.push_back(x);
            out[i].emplace_back(x, 1);
        }
        return {id, move(out)};
    }
    explicit Pattern(int r) {
        for (int left = r; left > 0;) {
            int p = int(bit_floor(unsigned(left)));
            auto [id, rr] = build(p);
            blocks.emplace_back(id, int(rows.size()));
            for (auto &row : rr) rows.push_back(move(row));
            left -= p;
        }
        for (const auto &row : rows)
            for (auto [s, sign] : row) ++frequency[s];
        auto cmp = [&](int a, int b) {
            return frequency[a] < frequency[b];
        };
        stable_sort(binary.begin(), binary.end(), cmp);
        stable_sort(leaves.begin(), leaves.end(), cmp);
    }
    // The two halves of a block are L+R+w and L-R.
    void decode(int id, const int *y, vector<int> &x) const {
        const Node &nd = nodes[id];
        if (nd.r == 1) {
            x[nd.leaf] = *y;
            return;
        }
        int h = nd.r / 2;
        vector<int> a(h), b(h);
        for (int i = 0; i < h; ++i) {
            int s = y[i] + y[i+h];
            int w = (s % 2 + 2) % 2;
            x[nd.extra[i]] = w;
            a[i] = (s-w)/2;
            b[i] = y[i]-a[i]-w;
        }
        decode(nd.l, a.data(), x);
        decode(nd.h, b.data(), x);
    }
};

class Solver {
    static constexpr int N = 500;
    int n, m, unknown;
    vector<int> u, v, value, royal, order, base, ref, witness;
    vector<vector<int>> adj;
    vector<char> covered;
    vector<Group> groups;
    DSU confirmed;
    mt19937 rng{0x631174b9};
    array<unique_ptr<Pattern>, N+1> patterns;
    array<int, N+1> capacity{};

    [[noreturn]] void answer(const vector<int> &edges) {
        cout << '!';
        for (int e : edges) cout << ' ' << e;
        cout << '\n' << flush;
        exit(0);
    }

    int ask(const vector<int> &tree) {
        cout << '?';
        for (int e : tree) cout << ' ' << e;
        cout << '\n' << flush;
        int res;
        if (!(cin >> res) || res < 0 || res >= n) exit(0);
        if (res == n-1) answer(tree);
        return res;
    }

    void setEdge(int e, int x) {
        if (value[e] != -1) return;
        value[e] = x;
        --unknown;
        if (x) {
            confirmed.join(u[e], v[e]);
            royal.push_back(e);
            if (int(royal.size()) == n-1) answer(royal);
        }
    }

    int other(int e, int a) const { return u[e] ^ v[e] ^ a; }

    // Any forest can be counted by completing it with classified edges.
    int forestCount(const vector<int> &edges) {
        vector<int> tree;
        tree.reserve(n-1);
        DSU d(n);
        int fixed = 0;
        for (int e : edges) {
            if (value[e] != -1) {
                fixed += value[e];
            } else {
                d.join(u[e], v[e]);
                tree.push_back(e);
            }
        }
        if (tree.empty()) return fixed;
        int filler = 0;
        for (int e : ref) {
            if (d.join(u[e], v[e])) {
                tree.push_back(e);
                filler += value[e];
                if (int(tree.size()) == n-1) break;
            }
        }
        return ask(tree) - filler + fixed;
    }

    void graphDeductions() {
        for (int e = 0; e < m; ++e)
            if (value[e] == -1 && confirmed.root(u[e]) == confirmed.root(v[e]))
                setEdge(e, 0);
        if (unknown == n-1-int(royal.size())) {
            for (int e = 0; e < m; ++e)
                if (value[e] == -1) setEdge(e, 1);
        }
        vector<int> in(n, -1), low(n);
        int timer = 0;
        auto dfs = [&](auto &&self, int a, int pe) -> void {
            in[a] = low[a] = timer++;
            for (int e : adj[a]) {
                if (e == pe || value[e] == 0) continue;
                int b = other(e, a);
                if (in[b] != -1) {
                    low[a] = min(low[a], in[b]);
                } else {
                    self(self, b, e);
                    low[a] = min(low[a], low[b]);
                    if (low[b] > in[a]) setEdge(e, 1);
                }
            }
        };
        dfs(dfs, 0, -1);
    }

    void cleanGroups() {
        int w = 0;
        for (auto &g : groups) {
            int sz = 0;
            for (int e : g.edges) {
                if (value[e] == -1) g.edges[sz++] = e;
                else g.k -= value[e];
            }
            g.edges.resize(sz);
            if (g.k == 0 || g.k == sz) {
                int x = (g.k != 0);
                for (int e : g.edges) setEdge(e, x);
            } else {
                if (&g != &groups[w]) groups[w] = move(g);
                ++w;
            }
        }
        groups.resize(w);
    }

    void probeDeductions() {
        while (true) {
            int before = unknown;
            cleanGroups();
            int rem = unknown, need = n-1-int(royal.size());
            for (const auto &g : groups) {
                rem -= int(g.edges.size());
                need -= g.k;
            }
            if (rem > 0 && (need == 0 || need == rem)) {
                for (int e : order)
                    if (!covered[e] && value[e] == -1) setEdge(e, need != 0);
            }
            if (unknown == before) return;
        }
    }

    void addProbe(vector<int> tree, int score) {
        for (int e : tree) covered[e] = true;
        groups.push_back({move(tree), score});
        probeDeductions();
    }

    bool findReference() {
        DSU d = confirmed;
        vector<int> t = royal;
        for (int e : order) {
            if (value[e] == 0 && d.join(u[e], v[e])) {
                t.push_back(e);
                if (int(t.size()) == n-1) {
                    ref = move(t);
                    return true;
                }
            }
        }
        return false;
    }

    bool newProbe() {
        DSU d = confirmed;
        vector<int> t = royal;
        int fresh = 0;
        for (int e : order) {
            if (!covered[e] && value[e] == -1 && d.join(u[e], v[e])) {
                t.push_back(e);
                ++fresh;
                if (int(t.size()) == n-1) break;
            }
        }
        for (int e : order) {
            if (int(t.size()) == n-1) break;
            if (value[e] == 0 && d.join(u[e], v[e])) t.push_back(e);
        }
        if (int(t.size()) != n-1 || fresh < 2) return false;
        int k = ask(t);
        addProbe(move(t), k);
        graphDeductions();
        probeDeductions();
        return true;
    }

    void makeReference() {
        graphDeductions();
        if (findReference()) return;

        vector<int> parent(n, -1), parentPos(n, -1), depth(n), basePos(m, -1);
        vector<bitset<N>> fromRoot(n);
        auto dfs = [&](auto &&self, int a) -> void {
            for (int e : adj[a]) {
                int b = other(e, a);
                if (parent[b] != -1) continue;
                parent[b] = a;
                depth[b] = depth[a] + 1;
                parentPos[b] = basePos[e] = int(base.size());
                fromRoot[b] = fromRoot[a];
                fromRoot[b].set(base.size());
                base.push_back(e);
                self(self, b);
            }
        };
        parent[0] = 0;
        dfs(dfs, 0);
        int baseline = ask(base);
        addProbe(base, baseline);
        if (findReference()) return;

        bool canProbe = n >= 24 && unknown >= 2*(n-1);
        if (canProbe) {
            int attempts = min(8, unknown / (n-1) - 1);
            while (attempts-- > 0) {
                if (!newProbe()) {
                    canProbe = false;
                    break;
                }
                if (findReference()) return;
            }
        }

        priority_queue<pair<int, int>> heap;
        bitset<N> open;
        auto refresh = [&]() {
            open.reset();
            for (int i = 0; i < n-1; ++i)
                if (value[base[i]] == -1) open.set(i);
        };
        refresh();
        for (int e : order) {
            if (basePos[e] >= 0) continue;
            int s = int(((fromRoot[u[e]] ^ fromRoot[v[e]]) & open).count());
            if (s) heap.emplace(s, e);
        }

        int retry = 4, lastPositive = int(royal.size());
        vector<int> t = base;
        while (open.any()) {
            int e;
            while (true) {
                auto [old, candidate] = heap.top();
                heap.pop();
                int len = int(((fromRoot[u[candidate]] ^ fromRoot[v[candidate]]) & open).count());
                if (!len) continue;
                if (len == old) { e = candidate; break; }
                heap.emplace(len, candidate);
            }

            vector<int> path, equal;
            int a = u[e], b = v[e];
            while (a != b) {
                if (depth[a] < depth[b]) swap(a, b);
                path.push_back(parentPos[a]);
                a = parent[a];
            }
            bitset<N> tied;
            while (true) {
                probeDeductions();
                if (value[e] == -1) {
                    for (int i : equal)
                        if (value[base[i]] != -1) {
                            setEdge(e, value[base[i]]);
                            break;
                        }
                }
                if (value[e] == -1) {
                    int left = 0, pos = baseline;
                    for (int f : base) {
                        if (value[f] == -1) ++left;
                        else pos -= value[f];
                    }
                    if (int(equal.size()) > pos) setEdge(e, 0);
                    else if (int(equal.size()) > left-pos) setEdge(e, 1);
                    else {
                        bool cycle = true;
                        for (int i : path)
                            if (value[base[i]] != 1 && !tied[i]) {
                                cycle = false;
                                break;
                            }
                        if (cycle) setEdge(e, 0);
                    }
                }
                if (value[e] != -1) {
                    for (int i : equal) setEdge(base[i], value[e]);
                    equal.clear();
                    tied.reset();
                }

                if (canProbe && retry && int(royal.size()) > lastPositive) {
                    --retry;
                    lastPositive = int(royal.size());
                    if (!newProbe()) canProbe = false;
                    if (findReference()) return;
                    // A probe may have resolved one of the equalities.
                    continue;
                }

                int pick = -1, anchor = -1;
                for (int i : path) {
                    if (value[base[i]] == -1 && !tied[i]) {
                        pick = i;
                        break;
                    }
                    if (value[base[i]] != -1) anchor = i;
                }
                if (pick == -1) {
                    if (equal.empty()) break;
                    pick = anchor;
                }
                int f = base[pick];
                t[pick] = e;
                int delta = ask(t) - baseline;
                t[pick] = f;
                if (delta != 0) {
                    setEdge(e, delta > 0);
                    setEdge(f, delta < 0);
                } else if (value[e] != -1) {
                    setEdge(f, value[e]);
                } else if (value[f] != -1) {
                    setEdge(e, value[f]);
                } else {
                    tied.set(pick);
                    equal.push_back(pick);
                }
            }
            if (findReference()) return;
            refresh();
        }
        ref = base;
    }

    void countRemaining() {
        graphDeductions();
        probeDeductions();
        vector<int> left;
        for (int e : order)
            if (!covered[e] && value[e] == -1) left.push_back(e);
        int k = n-1-int(royal.size());
        for (const auto &g : groups) k -= g.k;
        while (!left.empty()) {
            if (k == 0 || k == int(left.size())) {
                for (int e : left) setEdge(e, k != 0);
                break;
            }
            DSU d(n);
            Group g{{}, 0};
            int w = 0;
            for (int e : left) {
                if (confirmed.root(u[e]) == confirmed.root(v[e])) {
                    setEdge(e, 0);
                } else if (d.join(u[e], v[e])) {
                    g.edges.push_back(e);
                } else {
                    left[w++] = e;
                }
            }
            left.resize(w);
            if (g.edges.empty()) break;
            int s = int(g.edges.size());
            int lo = max(0, k-w), hi = min(k, s);
            g.k = lo == hi ? lo : forestCount(g.edges);
            k -= g.k;
            if (g.k == 0 || g.k == s) {
                for (int e : g.edges) setEdge(e, g.k != 0);
            } else {
                groups.push_back(move(g));
            }
        }
    }

    void normalize() {
        int before;
        do {
            before = unknown;
            cleanGroups();
            graphDeductions();
        } while (unknown != before);
    }

    // Build a feasible tree for the current disjoint quotas by shortest
    // augmenting paths in the graphic/partition matroid intersection.
    void quotaDeductions() {
        int r = n-1-int(royal.size());
        int g = int(groups.size());
        vector<int> color(m, -1), es;
        for (int i = 0; i < g; ++i)
            for (int e : groups[i].edges) {
                color[e] = i;
                es.push_back(e);
            }
        vector<char> chosen(m, false);
        vector<int> used(g, 0), w;
        DSU d = confirmed;
        auto take = [&](int e) {
            int c = color[e];
            if (c >= 0 && !chosen[e] && used[c] < groups[c].k && d.join(u[e], v[e])) {
                ++used[c];
                chosen[e] = true;
                w.push_back(e);
            }
        };
        for (int e : witness) take(e);
        for (int e : es) take(e);

        vector<int> par(n), dep(n), pe(n);
        auto rootForest = [&]() {
            vector<vector<pair<int, int>>> tree(n);
            auto add = [&](int e) {
                tree[u[e]].emplace_back(v[e], e);
                tree[v[e]].emplace_back(u[e], e);
            };
            for (int e : royal) add(e);
            for (int e : w) add(e);
            fill(par.begin(), par.end(), -1);
            auto dfs = [&](auto &&self, int a) -> void {
                for (auto [b, e] : tree[a]) if (b != par[a]) {
                    par[b] = a;
                    pe[b] = e;
                    dep[b] = dep[a]+1;
                    self(self, b);
                }
            };
            for (int a = 0; a < n; ++a) if (par[a] == -1) {
                par[a] = a; dep[a] = 0; pe[a] = -1;
                dfs(dfs, a);
            }
        };

        vector<int> prev(m), jump(n), que;
        while (int(w.size()) < r) {
            rootForest();
            fill(prev.begin(), prev.end(), -2);
            iota(jump.begin(), jump.end(), 0);
            vector<char> expanded(g, false);
            auto up = [&](auto &&self, int a) -> int {
                return jump[a] == a ? a : jump[a] = self(self, jump[a]);
            };
            auto expand = [&](int c, int from) {
                if (expanded[c]) return;
                expanded[c] = true;
                for (int e : groups[c].edges) if (!chosen[e]) {
                    prev[e] = from;
                    que.push_back(e);
                }
            };
            que.clear();
            for (int c = 0; c < g; ++c) if (used[c] < groups[c].k) expand(c, -1);
            int end = -1;
            for (int i = 0; i < int(que.size()); ++i) {
                int e = que[i];
                if (d.root(u[e]) != d.root(v[e])) {
                    end = e;
                    break;
                }
                int a = up(up, u[e]), b = up(up, v[e]);
                while (a != b) {
                    if (dep[a] < dep[b]) swap(a, b);
                    int f = pe[a];
                    if (color[f] >= 0) {
                        prev[f] = e;
                        expand(color[f], f);
                    }
                    jump[a] = up(up, par[a]);
                    a = jump[a];
                }
            }
            if (end == -1) return; // Feasibility guarantees an augmentation.
            for (int e = end; e != -1; e = prev[e]) chosen[e] ^= 1;
            w.clear();
            d = confirmed;
            fill(used.begin(), used.end(), 0);
            for (int e : es) if (chosen[e]) {
                w.push_back(e);
                ++used[color[e]];
                d.join(u[e], v[e]);
            }
        }
        witness = w;
        rootForest();

        vector<bitset<N>> arcs(g);
        auto path = [&](int e, auto &&visit) {
            int a = u[e], b = v[e];
            while (a != b) {
                if (dep[a] < dep[b]) swap(a, b);
                int f = pe[a];
                if (color[f] >= 0) visit(f, color[f]);
                a = par[a];
            }
        };
        for (int e : es) if (!chosen[e]) {
            int c = color[e];
            path(e, [&](int, int h) { arcs[h].set(c); });
        }

        vector<int> seq, comp(g, -1);
        vector<char> seen(g, false);
        auto dfs = [&](auto &&self, int a) -> void {
            seen[a] = true;
            for (int b = 0; b < g; ++b)
                if (arcs[a][b] && !seen[b]) self(self, b);
            seq.push_back(a);
        };
        for (int a = 0; a < g; ++a) if (!seen[a]) dfs(dfs, a);
        auto rev = [&](auto &&self, int a, int s) -> void {
            comp[a] = s;
            for (int b = 0; b < g; ++b)
                if (arcs[b][a] && comp[b] == -1) self(self, b, s);
        };
        reverse(seq.begin(), seq.end());
        for (int a : seq) if (comp[a] == -1) rev(rev, a, a);

        // Exchanges to any other feasible tree can be matched along tree
        // paths. Equal color quotas make the resulting color arcs balanced.
        vector<char> change(m, false);
        for (int e : es) if (!chosen[e]) {
            int s = comp[color[e]];
            path(e, [&](int f, int h) {
                if (comp[h] == s) change[e] = change[f] = true;
            });
        }
        for (int e : es) if (!change[e]) setEdge(e, chosen[e]);
    }

    struct Pack {
        vector<int> ids;
        int size = 0, general = 0;
    };

    int rowsFor(const Pack &p) const {
        int r = max(1, p.general);
        while (capacity[r] < int(p.ids.size())) ++r;
        return r;
    }

    pair<Pattern *, vector<int>> layout(const Pack &p, const vector<Split> &s) {
        int r = rowsFor(p);
        if (!patterns[r]) patterns[r] = make_unique<Pattern>(r);
        Pattern *mat = patterns[r].get();
        vector<int> map(mat->frequency.size(), -1), binary, general;
        for (int i : p.ids) {
            if (s[i].hi-s[i].lo == 1) binary.push_back(i);
            else general.push_back(i);
        }
        sort(binary.begin(), binary.end(), [&](int a, int b) {
            return s[a].size() > s[b].size();
        });
        int b = 0, l = 0;
        for (int slot : mat->binary) {
            if (b == int(binary.size())) break;
            map[slot] = binary[b++];
        }
        for (int i : general) map[mat->leaves[l++]] = i;
        while (b < int(binary.size())) map[mat->leaves[l++]] = binary[b++];
        return {mat, move(map)};
    }

    vector<Pack> makePacks(const vector<Split> &s) {
        vector<int> ix(s.size());
        iota(ix.begin(), ix.end(), 0);
        stable_sort(ix.begin(), ix.end(), [&](int a, int b) {
            return s[a].size() > s[b].size();
        });
        vector<Pack> packs;
        vector<Rollback> trees;
        for (int i : ix) {
            int j = 0;
            for (;; ++j) {
                if (j == int(packs.size())) {
                    packs.emplace_back();
                    trees.emplace_back(n);
                }
                if (packs[j].size + s[i].size() > n-1) continue;
                Rollback &d = trees[j];
                int checkpoint = int(d.history.size());
                bool ok = true;
                for (int sign : {1, -1}) {
                    if (!ok) break;
                    const auto &edges = sign > 0 ? s[i].a : s[i].b;
                    for (int e : edges)
                        if (!d.join(u[e], v[e])) { ok = false; break; }
                }
                if (!ok) {
                    d.undo(checkpoint);
                    continue;
                }
                packs[j].ids.push_back(i);
                packs[j].size += s[i].size();
                packs[j].general += s[i].hi-s[i].lo > 1;
                break;
            }
        }

        // A pack need only be acyclic row by row, not in its entire union.
        int work = 5000000;
        for (int i = 0; i < int(packs.size()) && work > 0; ++i) {
            if (packs[i].ids.empty()) continue;
            int attempts = 0;
            for (int j = i+1; j < int(packs.size()) && attempts < 36 && work > 0; ++j) {
                if (packs[j].ids.empty()) continue;
                int sz = packs[i].size + packs[j].size;
                if (sz > 2*(n-1)) continue;
                Pack merged = packs[i];
                merged.size = sz;
                merged.general += packs[j].general;
                merged.ids.insert(merged.ids.end(), packs[j].ids.begin(), packs[j].ids.end());
                int r = rowsFor(merged);
                int old = rowsFor(packs[i]) + rowsFor(packs[j]);
                if (r > old || (r == old && sz > n && merged.ids.size() > 2)) continue;
                ++attempts;
                auto [mat, map] = layout(merged, s);
                bool ok = true;
                for (const auto &row : mat->rows) {
                    DSU d(n);
                    work -= n/4;
                    for (auto [slot, sign] : row) {
                        int id = map[slot];
                        if (id < 0) continue;
                        const auto &edges = sign > 0 ? s[id].a : s[id].b;
                        work -= int(edges.size());
                        for (int e : edges)
                            if (!d.join(u[e], v[e])) { ok = false; break; }
                        if (!ok) break;
                    }
                    if (!ok || work <= 0) break;
                }
                if (ok && work > 0) {
                    packs[i] = move(merged);
                    packs[j].ids.clear();
                }
            }
        }
        return packs;
    }

    void solvePack(const Pack &p, vector<Split> &s) {
        auto [mat, map] = layout(p, s);
        int cols = int(map.size()), r = int(mat->rows.size());
        vector<int> lo(cols, 0), hi(cols, 0), y(r);
        auto limits = [&](const vector<int> &edges) {
            int a = 0, b = 0;
            for (int e : edges) {
                if (value[e] == -1 && confirmed.root(u[e]) == confirmed.root(v[e]))
                    setEdge(e, 0);
                a += value[e] == 1;
                b += value[e] != 0;
            }
            return pair{a, b};
        };
        for (int j = 0; j < cols; ++j) if (map[j] >= 0) {
            auto &t = s[map[j]];
            auto [a, b] = limits(t.a);
            auto [c, d] = limits(t.b);
            lo[j] = max(a, t.k-d) - t.lo;
            hi[j] = min(b, t.k-c) - t.lo;
        }

        auto range = [&](const Pattern::Row &row) {
            int a = 0, b = 0;
            for (auto [j, sign] : row)
                if (sign > 0) { a += lo[j]; b += hi[j]; }
                else { a -= hi[j]; b -= lo[j]; }
            return pair{a, b};
        };
        for (int i = 0; i < r; ++i) {
            auto [a, b] = range(mat->rows[i]);
            if (a == b) {
                y[i] = a;
                continue;
            }
            vector<int> edges;
            int offset = 0;
            for (auto [j, sign] : mat->rows[i]) {
                if (lo[j] == hi[j]) {
                    offset += sign * lo[j];
                } else {
                    const auto &t = s[map[j]];
                    const auto &part = sign > 0 ? t.a : t.b;
                    edges.insert(edges.end(), part.begin(), part.end());
                    offset -= sign > 0 ? t.lo : t.k-t.lo;
                }
            }
            y[i] = forestCount(edges) + offset;
            bool changed;
            do {
                changed = false;
                for (int z = 0; z <= i; ++z) {
                    auto [mn, mx] = range(mat->rows[z]);
                    for (auto [j, sign] : mat->rows[z]) {
                        if (lo[j] == hi[j]) continue;
                        int a0 = lo[j], b0 = hi[j];
                        if (sign > 0) {
                            lo[j] = max(a0, y[z]-mx+b0);
                            hi[j] = min(b0, y[z]-mn+a0);
                        } else {
                            lo[j] = max(a0, mn+b0-y[z]);
                            hi[j] = min(b0, mx+a0-y[z]);
                        }
                        changed |= lo[j] != a0 || hi[j] != b0;
                    }
                }
            } while (changed);
        }

        vector<int> x(cols, 0);
        for (auto [nd, start] : mat->blocks) mat->decode(nd, y.data()+start, x);
        auto add = [&](vector<int> &edges, int k) {
            if (k == 0 || k == int(edges.size())) {
                for (int e : edges) setEdge(e, k != 0);
            } else {
                groups.push_back({move(edges), k});
            }
        };
        for (int j = 0; j < cols; ++j) if (map[j] >= 0) {
            auto &t = s[map[j]];
            int k = t.lo+x[j];
            add(t.a, k);
            add(t.b, t.k-k);
        }
    }

public:
    Solver(int n_, int m_) : n(n_), m(m_), unknown(m), u(m), v(m),
        value(m, -1), order(m), adj(n), covered(m, false), confirmed(n) {}

    void run() {
        for (int e = 0; e < m; ++e) {
            if (!(cin >> u[e] >> v[e])) exit(0);
            adj[u[e]].push_back(e);
            adj[v[e]].push_back(e);
        }
        iota(order.begin(), order.end(), 0);
        if (m == n-1) answer(order);
        shuffle(order.begin(), order.end(), rng);
        for (auto &a : adj) shuffle(a.begin(), a.end(), rng);
        for (int i = 1; i <= n; ++i) {
            int p = int(bit_floor(unsigned(i)));
            capacity[i] = capacity[i-p]+p+p*(int(bit_width(unsigned(p)))-1)/2;
        }

        makeReference();
        countRemaining();
        while (true) {
            normalize();
            if (unknown <= 40000) {
                quotaDeductions();
                normalize();
            }
            vector<Split> s;
            s.reserve(groups.size());
            for (auto &g : groups) {
                int a = int(g.edges.size())/2;
                Split t;
                t.k = g.k;
                t.a.assign(g.edges.begin(), g.edges.begin()+a);
                t.b.assign(g.edges.begin()+a, g.edges.end());
                t.lo = max(0, t.k-int(t.b.size()));
                t.hi = min(t.k, a);
                s.push_back(move(t));
            }
            groups.clear();
            auto packs = makePacks(s);
            for (const auto &p : packs)
                if (!p.ids.empty()) solvePack(p, s);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m;
    if (cin >> n >> m) Solver(n, m).run();
    return 0;
}
