#include <bits/stdc++.h>
using namespace std;

constexpr int MAXN = 110;
constexpr int INF = 10000000;

struct DSU {
    array<int, MAXN> p;
    DSU() { iota(p.begin(), p.end(), 0); }
    int get(int x) { return p[x] == x ? x : p[x] = get(p[x]); }
    void join(int a, int b) { p[get(a)] = get(b); }
};

struct Balance {
    int result = 0; // -1 unbalanced, +1 balanced, 0 needs more information
    int a = -1, b = -1;
};

struct Towns {
    int n, tip, length;
    int label[MAXN];
    int depth[MAXN]{}, lo[MAXN][MAXN]{}, hi[MAXN][MAXN]{};
    int dist[MAXN][MAXN]{}, uses[MAXN]{};
    vector<array<int, 3>> edges;
    map<int, bitset<MAXN>> canceled;

    int query(int a, int b) {
        a = label[a];
        b = label[b];
        if (dist[a][b]) return dist[a][b];
        cout << "? " << a << ' ' << b << endl;
        int d;
        if (!(cin >> d) || d <= 0 || d > 1000000) exit(0);
        return dist[a][b] = dist[b][a] = d;
    }

    // Under root 0, every pair query gives the exact depth of an LCA.
    // At each depth, sharing an ancestor is an equivalence relation.
    void measure(int a, int b) {
        int c = (depth[a] + depth[b] - query(a, b)) / 2;
        ++uses[a];
        ++uses[b];
        edges.push_back({a, b, c});
        int left[MAXN], right[MAXN];
        for (int i = 0; i < n; ++i) {
            left[i] = lo[a][i];
            right[i] = lo[b][i];
        }
        for (int i = 1; i < n; ++i) {
            for (int j = 1; j < i; ++j) {
                int via = max(min(left[i], right[j]), min(right[i], left[j]));
                lo[i][j] = lo[j][i] = max(lo[i][j], min(via, c));
            }
        }
        // An exact split also separates the two entire clusters strictly below it.
        for (auto [u, v, h] : edges) {
            int members[MAXN], count = 0;
            for (int i = 1; i < n; ++i)
                if (lo[u][i] > h) members[count++] = i;
            for (int j = 1; j < n; ++j) if (lo[v][j] > h) {
                for (int k = 0; k < count; ++k)
                    hi[members[k]][j] = hi[j][members[k]] = h;
            }
        }
    }

    int low(int i) const { return i == tip ? length : lo[tip][i]; }
    int high(int i) const { return i == tip ? length : hi[tip][i]; }
    int ecc(int s, int extra) const { return max(length - s, s + extra); }
    int bestIn(int l, int h, int extra) const {
        int m = (length - extra) / 2;
        return min(ecc(clamp(m, l, h), extra), ecc(clamp(m + 1, l, h), extra));
    }

    bool distinctAt(int s, int i, int j) const {
        return (max(low(i), low(j)) >= s && hi[i][j] <= s) ||
               (min(high(i), high(j)) < s && max(low(i), low(j)) >= s) ||
               (min(high(i), high(j)) <= s && max(low(i), low(j)) > s);
    }

    Balance balance(int s) {
        int below = 0, above = 0, maybeBelow = 0, maybeAbove = 0, maybeAt = 0;
        for (int i = 0; i < n; ++i) {
            below += high(i) < s;
            above += low(i) > s;
            maybeBelow += low(i) < s;
            maybeAbove += high(i) > s;
            maybeAt += low(i) <= s && high(i) >= s;
        }
        int half = n / 2;
        if (max(above, below) > half) return {-1};
        if (max({maybeBelow, maybeAbove, maybeAt}) <= half) return {1};

        DSU sets;
        for (int i = 0; i < n; ++i) {
            if (high(i) < s) sets.join(i, 0);
            if (low(i) > s) sets.join(i, tip);
            for (int j = 1; j < i; ++j)
                if (lo[i][j] > s) sets.join(i, j);
        }
        int id[MAXN], ids[MAXN], reps[MAXN], weight[MAXN]{}, groups = 0;
        fill(ids, ids + n, -1);
        for (int i = 0; i < n; ++i) {
            int root = sets.get(i);
            if (ids[root] == -1) {
                ids[root] = groups;
                reps[groups++] = i;
            }
            int g = id[i] = ids[root];
            if (low(i) >= s && low(reps[g]) < s) reps[g] = i;
            if (++weight[g] > half) return {-1};
        }

        using Mask = bitset<MAXN>;
        Mask different[MAXN];
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < i; ++j) {
                if (id[i] == id[j]) continue;
                if (distinctAt(s, i, j)) {
                    different[id[i]].set(id[j]);
                    different[id[j]].set(id[i]);
                }
            }
        }
        // In a collection of pairwise different groups, at most one can
        // belong to any actual component. Pack such collections greedily.
        int order[MAXN], degree[MAXN];
        for (int g = 0; g < groups; ++g) {
            degree[g] = (int)different[g].count();
            order[g] = g;
        }
        sort(order, order + groups, [&](int a, int b) {
            if (weight[a] != weight[b]) return weight[a] > weight[b];
            return make_pair(degree[a], a) < make_pair(degree[b], b);
        });
        // Branch on an undecided group only while the extra inference is cheap.
        // Exhausting the computation budget leaves the majority possible.
        int budget = 160;
        auto canExceed = [&](auto &&self, Mask members, int limit) -> bool {
            if (limit < 0 || budget == 0) return true;
            --budget;
            Mask allowed[MAXN], chosen;
            int bins = 0, upper = 0, lower = 0, pivot = -1, most = -1;
            for (int k = 0; k < groups; ++k) {
                int g = order[k];
                if (!members[g]) continue;
                if ((chosen & different[g]).none()) {
                    chosen.set(g);
                    lower += weight[g];
                    if (lower > limit) return true;
                }
                int j = 0;
                while (j < bins && !allowed[j][g]) ++j;
                if (j == bins) {
                    upper += weight[g];
                    allowed[bins++] = different[g];
                } else allowed[j] &= different[g];
                int neighbors = (members & different[g]).count();
                if (neighbors > most) most = neighbors, pivot = g;
            }
            if (upper <= limit) return false;
            members.reset(pivot);
            return self(self, members & ~different[pivot], limit - weight[pivot]) ||
                   self(self, members, limit);
        };
        Mask all;
        for (int g = 0; g < groups; ++g) all.set(g);
        if (!canExceed(canExceed, all, half)) return {1};

        // Cancel known-different towns. A strict majority must still be
        // represented among the survivors.
        auto &used = canceled[s];
        int live[MAXN][MAXN], rem[MAXN]{};
        for (int i = 0; i < n; ++i)
            if (!used[i]) live[id[i]][rem[id[i]]++] = i;
        for (int k = 0; k < groups; ++k) {
            int a = order[k];
            for (int j = k + 1; j < groups && rem[a]; ++j) {
                int b = order[j];
                if (!different[a][b]) continue;
                while (rem[a] && rem[b]) {
                    used.set(live[a][--rem[a]]);
                    used.set(live[b][--rem[b]]);
                }
            }
        }
        int count = 0;
        bool possible[MAXN]{};
        for (int g = 0; g < groups; ++g) {
            if (!rem[g]) continue;
            Mask compatible = all & ~different[g];
            compatible.reset(g);
            budget = max(budget, 1);
            possible[g] = canExceed(canExceed, compatible, half - weight[g]);
            count += possible[g];
        }
        if (!count) return {1};

        int best = -INF, qa = -1, qb = -1;
        for (int a = 0; a < groups; ++a) for (int b = 0; b < a; ++b) {
            if (different[a][b] || (!possible[a] && !possible[b])) continue;
            int i = reps[a], j = reps[b];
            int score = 0;
            if (rem[a] && rem[b]) score += 4000;
            if (possible[a] && possible[b]) score += 200;
            score += 12 * min(weight[a], weight[b]) + max(weight[a], weight[b]);
            if (max(low(i), low(j)) >= s) score += 400;
            if (score <= best) continue;
            if (max(low(i), low(j)) >= s && lo[i][j] != hi[i][j]) {
                qa = i, qb = j;
            } else {
                // Fix one projection to distinguish the rootward component.
                if (low(i) == high(i)) swap(i, j);
                if (low(i) == high(i)) continue;
                qa = tip, qb = i;
            }
            best = score;
        }
        return {0, qa, qb};
    }

    // A landmark whose projection is at least threshold determines x_i
    // exactly if x_i is smaller, and certifies the lower bound otherwise.
    // Varying the landmark also collects comparisons between side branches.
    void locate(int i, int threshold, int preferred) {
        int anchor = tip, best = -INF;
        if (preferred >= 0 && low(i) < threshold) {
            auto &used = canceled[preferred];
            for (int u = 0; u < n; ++u) if (!used[u]) {
                for (int v = u + 1; v < n; ++v) if (!used[v] && distinctAt(preferred, u, v)) {
                    used.set(u);
                    used.set(v);
                    break;
                }
            }
            for (int k = 1; k < n; ++k) {
                if (k == i || low(k) < threshold || lo[i][k] == hi[i][k]) continue;
                int score = -uses[k];
                if (!used[i] && !used[k]) score += 1000;
                if (k == tip) score += 500;
                if (low(k) == preferred) score += 15;
                if (score > best) best = score, anchor = k;
            }
        }
        measure(i, anchor);
    }

    // Three mutually maximum-distance towns force a city at their common
    // midpoint. Each additional diametral pair contributes at most one town
    // to any component there; for odd n the initial triple also contributes one.
    bool diameterPairs() {
        constexpr int diameter = 1000000;
        vector<pair<int, int>> pairs, pending;
        if (n % 2 == 0) pairs.emplace_back(0, 1);
        int start = n % 2 == 0 ? 2 : 3;
        for (int i = start; i < n; i += 2) {
            if (query(i, i + 1) == diameter) pairs.emplace_back(i, i + 1);
            else pending.emplace_back(i, i + 1);
            if (pending.size() > 1 + pairs.size() / 4) return false;
        }
        auto height = [&](int i) {
            if (i < 3) return diameter / 2;
            int d = query(0, i);
            if (d < diameter) d = max(d, query(1, i));
            return d - diameter / 2;
        };
        for (auto [i, j] : pending) {
            int a = height(i), b = height(j);
            if (query(i, j) == a + b) continue;
            // Both are on one branch. Try exchanging partners with a pair
            // known to have both endpoints at the maximum radius.
            bool repaired = false;
            for (int k = 0; k < min(3, (int)pairs.size()); ++k) {
                auto [u, v] = pairs[k];
                int w = diameter / 2;
                if ((query(i, u) == a + w && query(j, v) == b + w) ||
                    (query(i, v) == a + w && query(j, u) == b + w)) {
                    pairs.erase(pairs.begin() + k);
                    repaired = true;
                    break;
                }
            }
            if (!repaired) return false;
        }
        return true;
    }

    void solve() {
        cin >> n;
        if (!cin || n < 6 || n > MAXN) return;
        iota(label, label + n, 0);
        // A short arm of the initial tripod makes a useful root landmark.
        // With a unit cherry the other leaf already has identical distances.
        bool tripod = false;
        int first = query(0, 1);
        if ((n >= 12 || first == 1000000) && first != 2 && query(0, 2) != 2) {
            int opposite[3] = {query(1, 2), query(0, 2), query(0, 1)};
            if (*min_element(opposite, opposite + 3) == 1000000 && diameterPairs()) {
                cout << "! 500000" << endl;
                return;
            }
            int root = max_element(opposite, opposite + 3) - opposite;
            swap(label[0], label[root]);
            tripod = true;
        }
        tip = 1;
        for (int i = 1; i < n; ++i) {
            depth[i] = query(0, i);
            if (depth[i] > depth[tip]) tip = i;
        }
        length = depth[tip];
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i && j) {
                    lo[i][j] = max(1, (depth[i] + depth[j] - 1000000 + 1) / 2);
                    hi[i][j] = min(depth[i], depth[j]) - 1;
                }
                if (i == j) lo[i][j] = hi[i][j] = INF;
            }
        }
        for (int k = 1; k < n; ++k)
            for (int i = 1; i < n; ++i)
                for (int j = 1; j < i; ++j)
                    lo[i][j] = lo[j][i] = max(lo[i][j], min(lo[i][k], lo[k][j]));

        if (tripod) {
            for (int i = 1; i < n; ++i)
                for (int j = 1; j < i; ++j)
                    if (dist[label[i]][label[j]] && lo[i][j] != hi[i][j]) measure(i, j);
        }

        for (;;) {
            int dl = 0, dh = 0;
            for (int i = 1; i < n; ++i) {
                dl = max(dl, depth[i] - 2 * high(i));
                dh = max(dh, depth[i] - 2 * low(i));
            }
            for (auto [u, v, h] : edges)
                dl = max(dl, min(depth[u], depth[v]) - 2 * h);
            // On the spine the eccentricity is max(length - x, x + delta),
            // with delta = max_i(depth[i] - 2*x_i), including town 0.
            // A maximizer of delta projects at or before (length-delta)/2.
            // Thus at every spine point a farthest town is reached along the
            // spine (possibly the tip itself). Moving off the spine at that
            // point strictly increases eccentricity. Degree >= 3 makes every
            // internal spine vertex the projection of some non-endpoint town.
            int lowerR = INF, upperR = INF, knownR = INF, knownHigh = INF, preferred = -1;
            for (int i = 1; i < n; ++i) if (i != tip) {
                int l = low(i), h = high(i);
                lowerR = min(lowerR, bestIn(l, h, dl));
                upperR = min(upperR, max(length - l, h + dh));
                if (l != h) continue;
                if (ecc(l, dl) < knownR || (ecc(l, dl) == knownR && l > preferred))
                    knownR = ecc(l, dl), preferred = l;
                knownHigh = min(knownHigh, ecc(l, dh));
            }
            for (auto [u, v, s] : edges) if (low(u) >= s || low(v) >= s) {
                upperR = min(upperR, ecc(s, dh));
                if (ecc(s, dl) < knownR || (ecc(s, dl) == knownR && s > preferred))
                    knownR = ecc(s, dl), preferred = s;
                knownHigh = min(knownHigh, ecc(s, dh));
            }
            if (lowerR != upperR) {
                int pick = -1;
                pair<int,int> best{-INF, -INF};
                for (int i = 1; i < n; ++i) if (low(i) != high(i)) {
                    int d = depth[i] - 2 * low(i);
                    bool diameter = knownHigh > knownR && d > dl;
                    if (!diameter && bestIn(low(i), high(i), dl) >= knownR) continue;
                    pair<int,int> priority{diameter ? 1 : 0, d};
                    if (priority > best) best = priority, pick = i;
                }
                int threshold = knownR == INF ? length :
                    max(knownR - dl, (depth[pick] - dl + 1) / 2);
                locate(pick, threshold, preferred);
                continue;
            }

            int r = lowerR;
            struct Candidate { int s; bool exists, sure; Balance bal; };
            vector<Candidate> candidates;
            // At a hub, even the lower eccentricity bound must equal r.
            // At least one of its two linear terms therefore equals r.
            for (int s : {length - r, r - dl}) {
                if (!candidates.empty() && candidates.back().s == s) continue;
                bool can = false, exists = false;
                for (int i = 1; i < n; ++i) if (i != tip) {
                    if (low(i) <= s && s <= high(i)) can = true;
                    if (low(i) == s && high(i) == s) exists = true;
                }
                for (auto [u, v, h] : edges)
                    if (s == h && (low(u) >= s || low(v) >= s)) exists = true;
                if (can) candidates.push_back({s, exists, exists && ecc(s, dh) <= r, balance(s)});
            }
            bool allYes = true, allNo = true, sureYes = false;
            for (auto &c : candidates) {
                allYes &= c.bal.result == 1;
                allNo &= c.bal.result == -1;
                sureYes |= c.sure && c.bal.result == 1;
            }
            if (allYes || sureYes || allNo) {
                cout << "! " << (allNo ? -r : r) << endl;
                return;
            }
            // Settle a guaranteed hub first, then a possible balanced hub.
            auto priority = [](const Candidate &c) {
                if (c.bal.result == -1) return -1;
                return 4 * int(c.sure) + 2 * int(c.bal.result == 1) + int(c.exists);
            };
            auto c = *max_element(candidates.begin(), candidates.end(), [&](auto a, auto b) {
                return priority(a) < priority(b);
            });
            if (!c.exists) {
                int i = 1;
                while (i == tip || low(i) > c.s || high(i) < c.s) ++i;
                locate(i, c.s + 1, c.s);
            } else if (!c.sure) {
                int i = 1;
                while (depth[i] - 2 * low(i) <= r - c.s) ++i;
                locate(i, (depth[i] - (r - c.s) + 1) / 2, c.s);
            } else if (c.bal.a >= 0) {
                measure(c.bal.a, c.bal.b);
            } else {
                // Complete any remaining projection before comparing branches.
                int i = 1;
                while (low(i) == high(i)) ++i;
                measure(tip, i);
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Towns towns;
    towns.solve();
}
