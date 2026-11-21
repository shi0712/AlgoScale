#include <bits/stdc++.h>
using namespace std;

static int n;
static int W;

static int ask(const vector<int>& pos) {
    cout << "? " << pos.size();
    for (int x : pos) cout << ' ' << (x + 1);
    cout << endl;
    int r;
    if (!(cin >> r)) _exit(0);
    if (r < 0) _exit(0);
    return r;
}

struct Sys {
    int n, W;
    vector<vector<uint64_t>> basis;
    vector<int> brhs;
    vector<char> hasp;
    int rank;
    bool bad;
    Sys(int n_) : n(n_), W((n_ + 63) / 64), basis(n_, vector<uint64_t>(W, 0)), brhs(n_, 0), hasp(n_, 0), rank(0), bad(false) {}
    bool add(const vector<uint64_t>& v_, int rv) {
        vector<uint64_t> v = v_;
        int r = rv;
        while (true) {
            int p = -1;
            for (int w = W - 1; w >= 0; --w) if (v[w]) { p = w * 64 + 63 - __builtin_clzll(v[w]); break; }
            if (p < 0) { if (r) bad = true; return false; }
            if (hasp[p]) { for (int w = 0; w < W; ++w) v[w] ^= basis[p][w]; r ^= brhs[p]; }
            else { basis[p] = v; brhs[p] = r; hasp[p] = 1; rank++; return true; }
        }
    }
    vector<int> particular() const {
        vector<int> b(n, 0);
        for (int p = 0; p < n; ++p) if (hasp[p]) {
            int val = brhs[p];
            for (int j = 0; j < p; ++j) if (((basis[p][j >> 6] >> (j & 63)) & 1) && b[j]) val ^= 1;
            b[p] = val;
        }
        return b;
    }
    vector<vector<int>> nullspace() const {
        vector<vector<int>> out;
        for (int f = 0; f < n; ++f) if (!hasp[f]) {
            vector<int> z(n, 0);
            z[f] = 1;
            for (int p = 0; p < n; ++p) if (hasp[p]) {
                int val = 0;
                for (int j = 0; j < p; ++j) if (((basis[p][j >> 6] >> (j & 63)) & 1) && z[j]) val ^= 1;
                z[p] = val;
            }
            out.push_back(z);
        }
        return out;
    }
};

static mt19937 rng(20240101);

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    if (n == 2) { cout << "! 1 2" << endl; return 0; }
    W = (n + 63) / 64;

    // ---- parity ----
    vector<int> par(n, 0);
    for (int i = 1; i < n; ++i) {
        int a = ask({0, i});
        par[i] = par[0] ^ (a == 1 ? 0 : 1);
    }

    // ---- level plan ----
    vector<int> levelM;
    int curM = 2;
    bool skipCase = false;
    while (2 * curM < n) {
        if (2 * curM == n - 2 && curM >= 4) { skipCase = true; break; }
        levelM.push_back(curM);
        curM *= 2;
    }

    // ---- anchors (lazy) ----
    bool haveAnchors = false;
    vector<int> ancPos, ancVal;
    auto fetchAnchors = [&]() {
        if (haveAnchors) return;
        haveAnchors = true;
        vector<int> order(n);
        iota(order.begin(), order.end(), 0);
        shuffle(order.begin(), order.end(), rng);
        vector<int> found;
        int done = 0;
        vector<char> asked(n, 0);
        for (int idx = 0; idx < n && (int)found.size() < 2; ++idx) {
            int x = order[idx];
            vector<int> pos;
            pos.reserve(n - 1);
            for (int i = 0; i < n; ++i) if (i != x) pos.push_back(i);
            int a = ask(pos);
            asked[x] = 1;
            ++done;
            if (a == 1) found.push_back(x);
            if ((int)found.size() == 1 && done == n - 1) {
                for (int i = 0; i < n; ++i) if (!asked[i]) { found.push_back(i); break; }
            }
        }
        ancPos = found;
        int v0 = (par[found[0]] == 1) ? 1 : n;
        int v1 = (v0 == 1) ? n : 1;
        ancVal = {v0, v1};
    };
    if (n % 4 == 0 || skipCase) fetchAnchors();

    // ---- levels ----
    vector<int> r = par;
    int m = 2;
    for (int lm : levelM) {
        int k = 2 * m;
        int target = n - 2;
        vector<vector<int>> byres(m);
        for (int i = 0; i < n; ++i) byres[r[i]].push_back(i);
        vector<int> base;
        base.reserve(k);
        for (int c = 0; c < m; ++c) { base.push_back(byres[c][0]); base.push_back(byres[c][1]); }
        vector<char> inBase(n, 0);
        for (int t = 0; t < k; ++t) inBase[base[t]] = 1;
        vector<vector<int>> baseIdxOfClass(m);
        for (int t = 0; t < k; ++t) baseIdxOfClass[r[base[t]]].push_back(t);
        vector<int> outs;
        for (int i = 0; i < n; ++i) if (!inBase[i]) outs.push_back(i);

        // ---- plan queries locally (no interaction) ----
        Sys local(n);
        vector<vector<int>> chosen;
        int cap = 6 * n + 1200;
        int planned = 0;
        int stall = 0;
        auto consider = [&](const vector<int>& q) -> bool {
            vector<uint64_t> vec(W, 0);
            for (int i : q) vec[i >> 6] |= 1ULL << (i & 63);
            int before = local.rank;
            local.add(vec, 0);
            ++planned;
            if (local.rank > before) { chosen.push_back(q); stall = 0; }
            else ++stall;
            return local.rank >= target;
        };
        bool reached = false;
        reached = consider(base);
        // phase A: single replacements inside each class
        for (int c = 0; c < m && !reached && planned < cap; ++c) {
            vector<int> outside;
            for (int p : byres[c]) if (!inBase[p]) outside.push_back(p);
            if (outside.empty()) continue;
            int i0 = baseIdxOfClass[c][0], i1 = baseIdxOfClass[c][1];
            for (size_t t = 0; t < outside.size() && !reached && planned < cap; ++t) {
                vector<int> q = base;
                q[i0] = outside[t];
                reached = consider(q);
                if (!reached && t == 0 && planned < cap) {
                    vector<int> q2 = base;
                    q2[i1] = outside[t];
                    reached = consider(q2);
                }
            }
        }
        // phase B: systematic double swaps
        int pass = 0;
        while (!reached && planned < cap && pass < 40) {
            int before = local.rank;
            ++pass;
            for (int j = 0; j < m && !reached && planned < cap; ++j) {
                if (outs.size() < 2) break;
                int a1 = outs[j % outs.size()], a2 = outs[(j + 1) % outs.size()];
                if (a1 == a2) continue;
                int C = (r[a1] + r[a2]) % m;
                int r1 = (j + pass) % m, r2 = ((C - r1) % m + m) % m;
                if (baseIdxOfClass[r1].empty() || baseIdxOfClass[r2].empty()) continue;
                int bi1 = baseIdxOfClass[r1][0];
                int bi2;
                if (r1 == r2) { if (baseIdxOfClass[r1].size() < 2) continue; bi2 = baseIdxOfClass[r1][1]; }
                else bi2 = baseIdxOfClass[r2][0];
                if (bi1 == bi2) continue;
                vector<int> q = base;
                q[bi1] = a1; q[bi2] = a2;
                reached = consider(q);
                if (!reached && r1 != r2 && byres[r1].size() == 2 && planned < cap) {
                    vector<int> q2 = base;
                    q2[baseIdxOfClass[r1][1]] = a1;
                    q2[bi2] = a2;
                    reached = consider(q2);
                }
            }
            if (local.rank == before) break;
        }
        // phase C: randomized double swaps
        if (!reached) {
            int st2 = 0, lastRank = local.rank;
            while (!reached && planned < cap && st2 < 500 && outs.size() >= 2) {
                int a1 = outs[rng() % outs.size()], a2 = outs[rng() % outs.size()];
                if (a1 == a2) continue;
                int C = (r[a1] + r[a2]) % m;
                int r1 = (int)(rng() % m), r2 = ((C - r1) % m + m) % m;
                if (baseIdxOfClass[r1].empty() || baseIdxOfClass[r2].empty()) continue;
                int bi1 = baseIdxOfClass[r1][0];
                int bi2;
                if (r1 == r2) { if (baseIdxOfClass[r1].size() < 2) continue; bi2 = baseIdxOfClass[r1][1]; }
                else bi2 = baseIdxOfClass[r2][0];
                if (bi1 == bi2) continue;
                vector<int> q = base;
                q[bi1] = a1; q[bi2] = a2;
                reached = consider(q);
                if (!reached && r1 != r2 && byres[r1].size() == 2 && planned < cap) {
                    vector<int> q2 = base;
                    q2[baseIdxOfClass[r1][1]] = a1;
                    q2[bi2] = a2;
                    reached = consider(q2);
                }
                if (local.rank == lastRank) ++st2; else { st2 = 0; lastRank = local.rank; }
            }
        }
        // phase D: fully random valid queries
        if (!reached) {
            int st3 = 0, lastRank = local.rank;
            while (!reached && planned < cap && st3 < 800) {
                vector<int> idx(n);
                iota(idx.begin(), idx.end(), 0);
                vector<int> pick;
                pick.reserve(k);
                for (int t = 0; t < k - 1; ++t) {
                    int j = t + (int)(rng() % (unsigned)(n - t));
                    swap(idx[t], idx[j]);
                    pick.push_back(idx[t]);
                }
                int A = 0;
                for (int i : pick) A += r[i];
                int need = ((-A) % m + m) % m;
                vector<char> ch(n, 0);
                for (int i : pick) ch[i] = 1;
                int z = -1;
                const vector<int>& lst = byres[need];
                int start = (int)(rng() % (unsigned)lst.size());
                for (int t = 0; t < (int)lst.size(); ++t) {
                    int pp = lst[(start + t) % lst.size()];
                    if (!ch[pp]) { z = pp; break; }
                }
                if (z < 0) continue;
                pick.push_back(z);
                reached = consider(pick);
                if (local.rank == lastRank) ++st3; else { st3 = 0; lastRank = local.rank; }
            }
        }
        if (!reached) break; // rank deficient: stop levels, resolve later
        // ---- submit chosen queries ----
        Sys sys(n);
        for (auto& q : chosen) {
            int A = 0;
            for (int i : q) A += r[i];
            int qq = (A / m) & 1;
            int ans = ask(q);
            vector<uint64_t> vec(W, 0);
            for (int i : q) vec[i >> 6] |= 1ULL << (i & 63);
            sys.add(vec, qq ^ (1 - ans));
        }
        // ---- solve level ----
        int dim = n - sys.rank;
        vector<int> want(2 * m, 0);
        for (int v = 1; v <= n; ++v) want[v % (2 * m)]++;
        vector<int> part = sys.particular();
        vector<vector<int>> ns = sys.nullspace();
        vector<int> chosenB;
        int foundCount = 0;
        auto trySolve = [&](bool useAnchors) {
            int total = 1 << dim;
            for (int mask = 0; mask < total; ++mask) {
                vector<int> cand = part;
                for (int t = 0; t < dim; ++t) if ((mask >> t) & 1) {
                    const vector<int>& z = ns[t];
                    for (int i = 0; i < n; ++i) if (z[i]) cand[i] ^= 1;
                }
                vector<int> cnt2(2 * m, 0);
                for (int i = 0; i < n; ++i) cnt2[r[i] + m * cand[i]]++;
                if (cnt2 != want) continue;
                if (useAnchors) {
                    bool ok = true;
                    for (int t = 0; t < 2; ++t) {
                        int p = ancPos[t];
                        if (r[p] + m * cand[p] != ancVal[t] % (2 * m)) { ok = false; break; }
                    }
                    if (!ok) continue;
                }
                if (foundCount == 0) chosenB = cand;
                ++foundCount;
            }
        };
        if (dim <= 13) {
            trySolve(haveAnchors);
            if (foundCount != 1 && !haveAnchors) {
                fetchAnchors();
                foundCount = 0;
                trySolve(true);
            }
        }
        if (foundCount >= 1) {
            for (int i = 0; i < n; ++i) r[i] = r[i] + m * chosenB[i];
        } else {
            for (int i = 0; i < n; ++i) r[i] = r[i] + m * part[i];
        }
        m = k;
    }

    // ---- final resolution ----
    int M = m;
    vector<int> knownVal(n, -1);
    vector<vector<int>> classPos(M);
    for (int i = 0; i < n; ++i) classPos[r[i]].push_back(i);
    if (2 * M - n < 2) fetchAnchors();
    if (haveAnchors) {
        for (int t = 0; t < 2; ++t) knownVal[ancPos[t]] = ancVal[t];
    }
    struct Task { vector<int> pos; vector<int> vals; };
    vector<Task> tasks;
    for (int res = 0; res < M; ++res) {
        if (classPos[res].empty()) continue;
        vector<int> vals;
        for (int v = (res == 0 ? M : res); v <= n; v += M) vals.push_back(v);
        vector<int> pos;
        for (int p : classPos[res]) {
            if (knownVal[p] > 0) {
                for (size_t t = 0; t < vals.size(); ++t) if (vals[t] == knownVal[p]) { vals.erase(vals.begin() + t); break; }
            } else pos.push_back(p);
        }
        if (vals.size() == 1) {
            for (int p : pos) knownVal[p] = vals[0];
            continue;
        }
        if (pos.empty()) continue;
        tasks.push_back(Task{pos, vals});
    }
    vector<int> knownPositions;
    for (int i = 0; i < n; ++i) if (knownVal[i] > 0) knownPositions.push_back(i);
    vector<int> t3[3];
    auto rebuild3 = [&]() {
        for (int a = 0; a < 3; ++a) t3[a].clear();
        for (int p : knownPositions) {
            int c = knownVal[p] % 3;
            if ((int)t3[c].size() < 2) t3[c].push_back(p);
        }
    };
    rebuild3();

    auto findFillers = [&](int k, int need, int x) -> vector<int> {
        vector<int> fill;
        if (k == 3) {
            for (int a = 0; a < 3 && fill.empty(); ++a) {
                int b = ((need - a) % 3 + 3) % 3;
                if (a == b) {
                    if (t3[a].size() >= 2 && t3[a][0] != x && t3[a][1] != x) fill = {t3[a][0], t3[a][1]};
                } else if (!t3[a].empty() && !t3[b].empty() && t3[a][0] != x && t3[b][0] != x) {
                    fill = {t3[a][0], t3[b][0]};
                }
            }
            return fill;
        }
        int K = k - 1;
        vector<int> pool;
        for (int p : knownPositions) if (p != x) pool.push_back(p);
        if ((int)pool.size() < K) return fill;
        for (int attempt = 0; attempt < 2000 && fill.empty(); ++attempt) {
            for (int t = 0; t < K; ++t) {
                int j = t + (int)(rng() % (unsigned)(pool.size() - t));
                swap(pool[t], pool[j]);
            }
            int s = 0;
            for (int t = 0; t < K; ++t) s += knownVal[pool[t]] % k;
            if (s % k != need) continue;
            fill.assign(pool.begin(), pool.begin() + K);
        }
        return fill;
    };

    auto resolveOne = [&](int x, vector<int> cand) -> int {
        while (cand.size() > 1) {
            bool done = false;
            for (int k = 3; k <= 21 && !done; k += 2) {
                if ((int)knownPositions.size() < k - 1) continue;
                vector<int> ccount(k, 0);
                for (int v : cand) ccount[((v % k) + k) % k]++;
                vector<pair<int, int>> cls;
                for (int c = 0; c < k; ++c) if (ccount[c] > 0 && ccount[c] < (int)cand.size()) cls.push_back({ccount[c], c});
                sort(cls.begin(), cls.end());
                for (auto& pr : cls) {
                    int c = pr.second;
                    int need = ((-c) % k + k) % k;
                    vector<int> fill = findFillers(k, need, x);
                    if (fill.empty()) continue;
                    vector<int> q = fill;
                    q.push_back(x);
                    int ans = ask(q);
                    vector<int> nc;
                    for (int v : cand) {
                        bool in = (((v % k) + k) % k == c);
                        if ((ans == 1) == in) nc.push_back(v);
                    }
                    if (nc.empty()) return -2;
                    cand = nc;
                    done = true;
                    break;
                }
            }
            if (!done) return -1;
        }
        return cand[0];
    };

    bool progress = true;
    int guard = 0;
    while (progress && guard < 400) {
        progress = false;
        ++guard;
        for (auto& T : tasks) {
            if (T.pos.size() < 2) continue;
            int v = resolveOne(T.pos[0], T.vals);
            if (v < 0) continue;
            int x = T.pos[0];
            knownVal[x] = v;
            knownPositions.push_back(x);
            for (size_t t = 0; t < T.vals.size(); ++t) if (T.vals[t] == v) { T.vals.erase(T.vals.begin() + t); break; }
            T.pos.erase(T.pos.begin());
            if (T.pos.size() == 1 && T.vals.size() == 1) {
                knownVal[T.pos[0]] = T.vals[0];
                knownPositions.push_back(T.pos[0]);
                T.pos.clear();
                T.vals.clear();
            }
            progress = true;
            rebuild3();
        }
    }

    vector<int> p(n);
    for (int i = 0; i < n; ++i) p[i] = knownVal[i];
    {
        vector<char> used(n + 1, 0);
        for (int i = 0; i < n; ++i) if (p[i] > 0) used[p[i]] = 1;
        for (int i = 0; i < n; ++i) if (p[i] <= 0) {
            int start = (r[i] == 0 ? M : r[i]);
            for (int v = start; v <= n; v += M) if (!used[v]) { p[i] = v; used[v] = 1; break; }
        }
    }
    if (p[0] > n / 2) for (int i = 0; i < n; ++i) p[i] = n + 1 - p[i];
    cout << "!";
    for (int i = 0; i < n; ++i) cout << ' ' << p[i];
    cout << endl;
    return 0;
}
