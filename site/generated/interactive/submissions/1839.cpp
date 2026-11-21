#include <bits/stdc++.h>
using namespace std;

using Bits = __uint128_t;
constexpr int INF = 1000000000;
constexpr int DR[4] = {-1, 0, 1, 0};
constexpr int DC[4] = {0, 1, 0, -1};
const char* CMD[] = {"left", "right", "step"};

struct Cell {
    int r, c, comp;
};
struct Component {
    int lo, hi, shape;
};
struct Shape {
    int base, first, copies, center;
    bool different;
};

int R, C, N;
int id[100][100];
vector<Cell> cells;
vector<Component> comps;
vector<Shape> shapes;
vector<array<int, 3>> go;
vector<int> prevStep, sight, cls, copies, sameCell, finish;
vector<double> info;
long long searchWork = 0, rolloutWork = 0;
constexpr long long SEARCH_LIMIT = 44000000;
constexpr long long ROLLOUT_LIMIT = 14000000;

int readObservation() {
    int d;
    if (!(cin >> d) || d < 0 || d > 99) exit(0);
    return d;
}
int command(int a) {
    cout << CMD[a] << '\n' << flush;
    return readObservation();
}
void report(int p) {
    if (p < 0) cout << "no\n" << flush;
    else cout << "yes " << cells[p].r + 1 << ' ' << cells[p].c + 1 << '\n' << flush;
    exit(0);
}

// -2 means that another action is necessary.
int terminal(const vector<int>& b) {
    int p = sameCell[cls[b.front()]];
    bool good = false;
    for (int s : b) {
        if (sameCell[cls[s]] != p) p = -1;
        good |= finish[s] >= 0;
    }
    if (p >= 0) return p;
    return good ? -2 : -1;
}

pair<int, int> rotateRight(int r, int c, int k) {
    if (k == 0) return {r, c};
    if (k == 1) return {c, -r};
    if (k == 2) return {-r, -c};
    return {-c, r};
}

void prepare(const vector<string>& grid) {
    memset(id, -1, sizeof id);
    for (int r = 0; r < R; ++r) for (int c = 0; c < C; ++c) {
        if (grid[r][c] != '.' || id[r][c] >= 0) continue;
        int lo = (int)cells.size(), comp = (int)comps.size();
        id[r][c] = lo;
        cells.push_back({r, c, comp});
        for (int i = lo; i < (int)cells.size(); ++i) {
            Cell p = cells[i];
            for (int f = 0; f < 4; ++f) {
                int x = p.r + DR[f], y = p.c + DC[f];
                if (x < 0 || x >= R || y < 0 || y >= C ||
                    grid[x][y] != '.' || id[x][y] >= 0) continue;
                id[x][y] = (int)cells.size();
                cells.push_back({x, y, comp});
            }
        }
        comps.push_back({lo, (int)cells.size(), -1});
    }
    N = 4 * (int)cells.size();
    go.resize(N);
    prevStep.assign(N, -1);
    sight.assign(N, -1);
    cls.resize(N);
    map<vector<int>, int> dictionary;
    vector<int> classShape;
    for (int i = 0; i < (int)comps.size(); ++i) {
        auto& co = comps[i];
        for (int f = 0; f < 4; ++f) {
            int minr = 1000, minc = 1000;
            for (int p = co.lo; p < co.hi; ++p) {
                auto [x, y] = rotateRight(cells[p].r, cells[p].c, (4 - f) & 3);
                minr = min(minr, x);
                minc = min(minc, y);
            }
            vector<pair<int, int>> a;
            for (int p = co.lo; p < co.hi; ++p) {
                auto [x, y] = rotateRight(cells[p].r, cells[p].c, (4 - f) & 3);
                a.emplace_back((x - minr) * 128 + y - minc, p);
            }
            sort(a.begin(), a.end());
            vector<int> key;
            key.reserve(a.size());
            for (auto [x, p] : a) key.push_back(x);
            auto [it, added] = dictionary.try_emplace(move(key), (int)shapes.size());
            int g = it->second;
            if (added) {
                shapes.push_back({(int)sameCell.size(), i, 0, -1, false});
                sameCell.resize(sameCell.size() + a.size(), -2);
                classShape.resize(sameCell.size(), g);
            }
            Shape& sh = shapes[g];
            ++sh.copies;
            sh.different |= sh.first != i;
            if (f == 0) co.shape = g;
            for (int j = 0; j < (int)a.size(); ++j) {
                int p = a[j].second, k = sh.base + j;
                cls[4 * p + f] = k;
                if (sameCell[k] == -2) sameCell[k] = p;
                else if (sameCell[k] != p) sameCell[k] = -1;
            }
        }
    }
    copies.resize(sameCell.size());
    for (int k = 0; k < (int)sameCell.size(); ++k) {
        Shape& sh = shapes[classShape[k]];
        copies[k] = sh.copies;
        if (sh.copies > 1 && sameCell[k] >= 0) sh.center = sameCell[k];
    }
    for (int s = 0; s < N; ++s) {
        int f = s & 3, base = s & ~3;
        const auto& p = cells[s / 4];
        int x = p.r + DR[f], y = p.c + DC[f];
        int t = (x >= 0 && x < R && y >= 0 && y < C && id[x][y] >= 0)
                    ? 4 * id[x][y] + f : -1;
        go[s] = {base + ((f + 3) & 3), base + ((f + 1) & 3), t};
        if (t >= 0) prevStep[t] = s;
    }
    for (int s = 0; s < N; ++s) {
        if (sight[s] >= 0) continue;
        int t = s;
        while (t >= 0 && sight[t] < 0) t = go[t][2];
        int value = (t < 0 ? -1 : sight[t]);
        t = s;
        while (t >= 0 && sight[t] < 0) {
            sight[t] = ++value + 0; // Filled in the opposite order below.
            t = go[t][2];
        }
        // The run just visited starts at s and ends just before a known suffix.
        // Recompute its distances in forward order from its total length.
        int suffix = (t < 0 ? -1 : sight[t]);
        int v = value;
        for (int u = s; u != t; u = go[u][2]) sight[u] = v--;
        (void)suffix;
    }
    finish.assign(N, -1);
    vector<int> q;
    for (const auto& co : comps) {
        const Shape& sh = shapes[co.shape];
        if (sh.copies == 1) {
            fill(finish.begin() + 4 * co.lo, finish.begin() + 4 * co.hi, 0);
        } else if (!sh.different && sh.center >= 0) {
            for (int f = 0; f < 4; ++f) {
                int s = 4 * sh.center + f;
                finish[s] = 0;
                q.push_back(s);
            }
        }
    }
    for (size_t i = 0; i < q.size(); ++i) {
        int s = q[i];
        for (int p : {go[s][0], go[s][1], prevStep[s]}) {
            if (p >= 0 && finish[p] < 0) {
                finish[p] = finish[s] + 1;
                q.push_back(p);
            }
        }
    }
    info.resize(N + 1);
    for (int k = 2; k <= N; ++k) info[k] = 1.25 + 1.55 * log2(k);
}

int knownAction(int s) {
    for (int a = 0; a < 3; ++a) {
        int t = go[s][a];
        if (t >= 0 && finish[t] == finish[s] - 1) return a;
    }
    return -1;
}

bool exact(vector<int> b) {
    int k = (int)b.size();
    if (k > 11) return false;
    sort(b.begin(), b.end(), [](int s, int t) {
        const auto& a = comps[cells[s / 4].comp];
        const auto& b = comps[cells[t / 4].comp];
        return a.hi - a.lo < b.hi - b.lo;
    });
    int M = 1 << k, goodMask = 0;
    vector<int> weight(M), lo(k), len(k);
    for (int i = 0; i < k; ++i) {
        const auto& co = comps[cells[b[i] / 4].comp];
        lo[i] = 4 * co.lo;
        len[i] = 4 * (co.hi - co.lo);
        if (finish[b[i]] >= 0) goodMask |= 1 << i;
    }
    long long work = 0;
    for (int m = 1; m < M; ++m) {
        int i = __builtin_ctz((unsigned)m);
        weight[m] = weight[m & (m - 1)] + copies[cls[b[i]]];
        if ((m & (m - 1)) && (m & goodMask))
            work += (long long)len[i] * __builtin_popcount((unsigned)m);
    }
    if (work > 2700000) return false;

    vector<vector<int>> cost(M);
    vector<vector<signed char>> policy(M);
    auto val = [&](int m, int s) -> int {
        if (!(m & goodMask)) return 0;
        int i = __builtin_ctz((unsigned)m);
        if (!(m & (m - 1))) return weight[m] * max(0, finish[s]);
        return cost[m][s - lo[i]];
    };

    for (int m = 1; m < M; ++m) {
        if (!(m & (m - 1)) || !(m & goodMask)) continue;
        int i = __builtin_ctz((unsigned)m), L = lo[i], Z = len[i];
        Cell base = cells[b[i] / 4];
        int js[11], nk = 0;
        for (int u = m; u; u &= u - 1) js[nk++] = __builtin_ctz((unsigned)u);
        cost[m].assign(Z, INF);
        policy[m].assign(Z, -1);
        auto& dp = cost[m];
        auto& pol = policy[m];
        vector<unsigned char> valid(Z);
        vector<pair<int, int>> sources;
        for (int z = 0; z < Z; ++z) {
            int s = L + z;
            const auto& p = cells[s / 4];
            int st[11];
            st[0] = s;
            int target = sameCell[cls[s]];
            bool ok = true;
            for (int j = 1; j < nk; ++j) {
                int old = b[js[j]], rot = (old - b[i]) & 3;
                auto [x, y] = rotateRight(p.r - base.r, p.c - base.c, rot);
                const auto& o = cells[old / 4];
                x += o.r;
                y += o.c;
                if (x < 0 || x >= R || y < 0 || y >= C ||
                    id[x][y] < 0 || cells[id[x][y]].comp != o.comp) {
                    ok = false;
                    break;
                }
                int t = 4 * id[x][y] + ((s + rot) & 3);
                if (sight[t] != sight[s]) {
                    ok = false;
                    break;
                }
                st[j] = t;
                if (sameCell[cls[t]] != target) target = -1;
            }
            if (!ok) continue;
            valid[z] = 1;
            if (target >= 0) {
                dp[z] = 0;
            } else {
                for (int a = 0; a < 2 + (sight[s] > 0); ++a) {
                    int masks[100] = {}, first[100], seen[11], ns = 0;
                    for (int j = 0; j < nk; ++j) {
                        int t = go[st[j]][a], d = sight[t];
                        if (!masks[d]) {
                            first[d] = t;
                            seen[ns++] = d;
                        }
                        masks[d] |= 1 << js[j];
                    }
                    if (ns == 1) continue;
                    long long c = weight[m];
                    for (int j = 0; j < ns; ++j) {
                        int d = seen[j];
                        c += val(masks[d], first[d]);
                    }
                    if (c < dp[z]) dp[z] = (int)c, pol[z] = a;
                }
            }
            if (dp[z] < INF) sources.emplace_back(dp[z], z);
        }
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>>
            pq(greater<pair<int, int>>(), move(sources));
        while (!pq.empty()) {
            auto [v, z] = pq.top();
            pq.pop();
            if (v != dp[z]) continue;
            int s = z + L, next = v + weight[m];
            int pred[] = {go[s][1], go[s][0], prevStep[s]};
            for (int a = 0; a < 3; ++a) {
                if (pred[a] < 0) continue;
                int p = pred[a] - L;
                if (valid[p] && next < dp[p]) {
                    dp[p] = next;
                    pol[p] = a;
                    pq.emplace(next, p);
                }
            }
        }
    }

    int m = M - 1;
    if (val(m, b[0]) >= INF) return false;
    for (;;) {
        vector<int> cur;
        for (int u = m; u; u &= u - 1) cur.push_back(b[__builtin_ctz((unsigned)u)]);
        int res = terminal(cur);
        if (res != -2) report(res);
        int i = __builtin_ctz((unsigned)m);
        int a = (m & (m - 1)) ? policy[m][b[i] - lo[i]] : knownAction(b[i]);
        if (a < 0) return false; // Unreachable for a finite nonterminal entry.
        int d = command(a);
        for (int u = m; u; u &= u - 1) {
            int j = __builtin_ctz((unsigned)u);
            b[j] = go[b[j]][a];
            if (sight[b[j]] != d) m ^= 1 << j;
        }
        if (!m) exit(0);
    }
}

struct Probe {
    double rank, tail, nav;
    int s, dep;
    vector<int> path;
};
struct Hypothesis {
    int r, c, rot, w;
};
struct Scanner {
    vector<Hypothesis> h;
    int start, weight = 0;
    double nav0 = 0;
    explicit Scanner(const vector<int>& b) {
        start = b[0];
        int row = cells[start / 4].r, col = cells[start / 4].c;
        h.reserve(b.size());
        for (int s : b) {
            int k = (s - start) & 3;
            auto [x, y] = rotateRight(row, col, k);
            int w = copies[cls[s]];
            h.push_back({cells[s / 4].r - x, cells[s / 4].c - y, k, w});
            weight += w;
            nav0 += w * max(0, finish[s]);
        }
        nav0 /= weight;
    }
    int project(int s, int i) const {
        const auto& p = cells[s / 4];
        const auto& a = h[i];
        auto [x, y] = rotateRight(p.r, p.c, a.rot);
        return 4 * id[x + a.r][y + a.c] + ((s + a.rot) & 3);
    }
    vector<Probe> scan(bool mainSearch, long long limit, int keep) const {
        struct Node {int s, parent, action, depth;};
        struct Bucket {int w, count, p; bool good;};
        vector<Node> q;
        vector<unsigned char> visited(N);
        q.push_back({start, -1, -1, 0});
        visited[start] = 1;
        vector<Probe> best;
        int firstDepth = -1;
        double exactBound = 1e30;
        long long work = 0;
        int k = (int)h.size();
        double base = info[k];
        Bucket bucket[100];
        int stamp[100] = {};
        for (int j = 0; j < (int)q.size(); ++j) {
            auto [s, par, act, dep] = q[j];
            if (dep > exactBound + 1e-9) break;
            if (firstDepth >= 0 &&
                dep > firstDepth + (mainSearch ? min(8, 4 + firstDepth / 3) : 2)) break;
            if (j && work + k > limit && (!mainSearch || firstDepth >= 0)) break;
            if (mainSearch && searchWork + work + k > SEARCH_LIMIT) break;
            if (j) {
                work += k;
                int seen[100], ns = 0;
                long long walk = 0;
                const auto& p = cells[s / 4];
                int xs[] = {p.r, p.c, -p.r, -p.c};
                int ys[] = {p.c, -p.r, -p.c, p.r};
                for (const auto& a : h) {
                    int t = 4 * id[xs[a.rot] + a.r][ys[a.rot] + a.c] + ((s + a.rot) & 3);
                    int d = sight[t], f = finish[t], pos = sameCell[cls[t]];
                    auto& z = bucket[d];
                    if (stamp[d] != j) {
                        stamp[d] = j;
                        seen[ns++] = d;
                        z = {a.w, 1, pos, f >= 0};
                    } else {
                        z.w += a.w;
                        ++z.count;
                        if (z.p != pos) z.p = -1;
                        z.good |= f >= 0;
                    }
                    if (f > 0) walk += (long long)f * a.w;
                }
                if (ns > 1 || bucket[seen[0]].p >= 0) {
                    if (firstDepth < 0) firstDepth = dep;
                    double tail = 0;
                    for (int i = 0; i < ns; ++i) {
                        auto& z = bucket[seen[i]];
                        if (z.good && z.p < 0) tail += z.w * info[z.count];
                    }
                    tail /= weight;
                    double nav = (double)walk / weight;
                    if (tail < 1e-12) exactBound = min(exactBound, dep + nav);
                    double rank = (base - tail + 0.45 * (nav0 - nav)) / (dep + 0.55);
                    if ((int)best.size() < keep || rank > best.back().rank) {
                        Probe cand{rank, tail, nav, s, dep, {}};
                        for (int u = j; q[u].parent != -1; u = q[u].parent)
                            cand.path.push_back(q[u].action);
                        reverse(cand.path.begin(), cand.path.end());
                        auto it = lower_bound(best.begin(), best.end(), rank,
                                              [](const Probe& p, double v) {return p.rank > v;});
                        best.insert(it, move(cand));
                        if ((int)best.size() > keep) best.pop_back();
                    }
                    if (dep == 1 && tail < 1e-12 && nav < 1e-12) {
                        if (best.front().s != s) {
                            for (size_t i = 1; i < best.size(); ++i)
                                if (best[i].s == s) swap(best[0], best[i]);
                        }
                        break;
                    }
                    continue;
                }
            }
            for (int a = 0; a < 3; ++a) {
                int t = go[s][a];
                if (t >= 0 && !visited[t]) {
                    visited[t] = 1;
                    q.push_back({t, j, a, dep + 1});
                }
            }
        }
        if (mainSearch) searchWork += work;
        else rolloutWork += work;
        return best;
    }
};

vector<int> chooseRoute(const vector<int>& b) {
    int k = (int)b.size();
    Scanner sc(b);
    auto options = sc.scan(true, 115000 + 40LL * k, 5);
    if (options.empty()) return {};
    if (options.size() == 1 ||
        (options[0].dep == 1 && options[0].nav + options[0].tail < 1e-12))
        return options[0].path;

    int best = 0;
    double bestValue = 1e30;
    long long deadline = min(ROLLOUT_LIMIT, rolloutWork + 115000);
    bool look = k <= 2400 && rolloutWork + 10000 <= ROLLOUT_LIMIT;
    for (int i = 0; i < (int)options.size(); ++i) {
        const auto& p = options[i];
        double rem = p.tail + p.nav;
        if (look && p.tail > 0 && rolloutWork < deadline) {
            vector<int> part[100];
            for (int j = 0; j < k; ++j) {
                int t = sc.project(p.s, j);
                part[sight[t]].push_back(t);
            }
            for (auto& v : part) {
                if (v.size() <= 1 || terminal(v) != -2) continue;
                int w = 0;
                double nav = 0;
                for (int t : v) {
                    int a = copies[cls[t]];
                    w += a;
                    nav += a * max(0, finish[t]);
                }
                nav /= w;
                if (w * 32 < sc.weight || rolloutWork >= deadline) continue;
                Scanner sub(v);
                auto next = sub.scan(false, min(17000LL, deadline - rolloutWork), 2);
                double after = 1e30;
                for (const auto& z : next) after = min(after, z.dep + z.tail + z.nav);
                if (after < 1e29)
                    rem += (double)w / sc.weight * (after - info[v.size()] - nav);
            }
        }
        // Favor efficient information gathering unless the predicted remaining
        // route, including travel to a symmetry center, is shorter.
        double gain = max(0.10, (info[k] - p.tail) / info[k]);
        double value = p.dep + rem + (p.tail > 0 ? 0.17 * (p.dep + .55) / gain : 0);
        if (value < bestValue - 1e-9 ||
            (abs(value - bestValue) < 1e-9 && p.rank > options[best].rank)) {
            bestValue = value;
            best = i;
        }
    }
    return options[best].path;
}

pair<int, int> toFrame(int r, int c, int f) {
    if (f == 0) return {r, c};
    if (f == 1) return {C - 1 - c, r};
    if (f == 2) return {R - 1 - r, C - 1 - c};
    return {c, R - 1 - r};
}
pair<int, int> fromFrame(int r, int c, int f) {
    if (f == 0) return {r, c};
    if (f == 1) return {c, C - 1 - r};
    if (f == 2) return {R - 1 - r, C - 1 - c};
    return {R - 1 - c, r};
}

// A bounded-work reserve strategy: map the reachable component in the
// initial coordinate frame, intersecting whole rows of possible origins.
void explore(vector<int>& b, int& d) {
    vector<Bits> masks(4 * 4 * 100 * 100);
    auto index = [](int k, int h, int d, int row) {
        return ((k * 4 + h) * 100 + d) * 100 + row;
    };
    Bits active[4][100] = {}, good[4][100] = {};
    vector<unsigned char> use(sameCell.size());
    for (int s : b) use[cls[s]] = 1;
    int rows[4] = {R, C, R, C};
    for (int s = 0; s < N; ++s) {
        int k = s & 3;
        auto [x, y] = toFrame(cells[s / 4].r, cells[s / 4].c, k);
        Bits bit = Bits(1) << y;
        if (use[cls[s]]) {
            active[k][x] |= bit;
            if (finish[s] >= 0) good[k][x] |= bit;
        }
        for (int h = 0; h < 4; ++h)
            masks[index(k, h, sight[(s & ~3) + ((k + h) & 3)], x)] |= bit;
    }
    int x = 0, y = 0, h = 0;
    auto filter = [&]() {
        bool possible = false;
        int count = 0;
        for (int k = 0; k < 4; ++k) {
            const Bits* m = &masks[index(k, h, d, 0)];
            for (int i = 0; i < rows[k]; ++i) {
                Bits& a = active[k][i];
                if (!a) continue;
                int row = i + x;
                if (row < 0 || row >= rows[k]) {
                    a = 0;
                    continue;
                }
                Bits v = m[row];
                a &= (y >= 0 ? v >> y : v << -y);
                possible |= (a & good[k][i]) != 0;
                if (count <= 4) count += __builtin_popcountll((uint64_t)a)
                                      + __builtin_popcountll((uint64_t)(a >> 64));
            }
        }
        if (!possible) report(-1);
        if (count > 4) return false;
        b.clear();
        for (int k = 0; k < 4; ++k) for (int i = 0; i < rows[k]; ++i) {
            Bits a = active[k][i];
            while (a) {
                uint64_t low = (uint64_t)a;
                int j = low ? __builtin_ctzll(low) : 64 + __builtin_ctzll((uint64_t)(a >> 64));
                a &= a - 1;
                auto [r, c] = fromFrame(i + x, j + y, k);
                int s = 4 * id[r][c] + ((h + k) & 3);
                bool seen = false;
                for (int t : b) seen |= cls[t] == cls[s];
                if (!seen) b.push_back(s);
            }
        }
        return true;
    };
    if (filter()) return;
    constexpr int S = 203, O = 101;
    unsigned char walls[S][S] = {}, visited[S][S] = {};
    vector<int> parent(1, -1);
    visited[O][O] = 1;
    auto act = [&](int a) {
        d = command(a);
        if (a == 0) h = (h + 3) & 3;
        else if (a == 1) h = (h + 1) & 3;
        else x += DR[h], y += DC[h];
        return filter();
    };
    auto face = [&](int f) {
        while (h != f)
            if (act(((f - h) & 3) == 3 ? 0 : 1)) return true;
        return false;
    };
    for (;;) {
        if (!d) walls[x + O][y + O] |= 1 << h;
        if (d && !visited[x + O + DR[h]][y + O + DC[h]]) {
            int back = h ^ 2;
            if (act(2)) return;
            visited[x + O][y + O] = 1;
            parent.push_back(back);
            continue;
        }
        int f = -1;
        for (int j = 0; j < 4; ++j) {
            int z = (h + j) & 3;
            if (!(walls[x + O][y + O] & (1 << z)) &&
                !visited[x + O + DR[z]][y + O + DC[z]]) {
                f = z;
                break;
            }
        }
        if (f >= 0) {
            if (face(f)) return;
        } else {
            int back = parent.back();
            if (back < 0) {
                // A fully mapped component has only one remaining class.
                // A feasible class has at most four members, so filter()
                // has already returned (or reported "no").
                exit(0);
            }
            parent.pop_back();
            if (face(back) || act(2)) return;
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> R >> C)) return 0;
    vector<string> grid(R);
    for (auto& row : grid) if (!(cin >> row)) return 0;
    int d = readObservation();
    prepare(grid);
    vector<int> belief;
    vector<unsigned char> seen(sameCell.size());
    for (int s = 0; s < N; ++s) {
        if (sight[s] == d && !seen[cls[s]]) {
            belief.push_back(s);
            seen[cls[s]] = 1;
        }
    }
    for (;;) {
        if (belief.empty()) return 0;
        int res = terminal(belief);
        if (res != -2) report(res);
        if (belief.size() == 1) {
            int s = belief[0], a = knownAction(s);
            d = command(a);
            belief[0] = go[s][a];
            if (sight[belief[0]] != d) return 0;
            continue;
        }
        if (exact(belief)) return 0;
        auto it = min_element(belief.begin(), belief.end(), [](int s, int t) {
            const auto& a = comps[cells[s / 4].comp];
            const auto& b = comps[cells[t / 4].comp];
            return a.hi - a.lo < b.hi - b.lo;
        });
        iter_swap(belief.begin(), it);
        vector<int> route = chooseRoute(belief);
        if (route.empty()) {
            explore(belief, d);
            continue;
        }
        for (int a : route) {
            d = command(a);
            int z = 0;
            for (int s : belief) {
                int t = go[s][a];
                if (t >= 0 && sight[t] == d) belief[z++] = t;
            }
            bool split = z < (int)belief.size();
            belief.resize(z);
            if (belief.empty()) return 0;
            res = terminal(belief);
            if (res != -2) report(res);
            if (split) break;
        }
    }
}
