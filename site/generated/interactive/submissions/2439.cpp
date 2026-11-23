#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll NONE = LLONG_MIN;
const ll BIG = (ll)4e12;

static int n;
static function<ll(int, int, int)> ASK;
static function<void(int, int)> ANSWER;

struct Seg {
    ll u1, v1, u2, v2;
};
static bool operator<(const Seg& a, const Seg& b) {
    return tie(a.u1, a.v1, a.u2, a.v2) < tie(b.u1, b.v1, b.u2, b.v2);
}
static bool operator==(const Seg& a, const Seg& b) {
    return a.u1 == b.u1 && a.v1 == b.v1 && a.u2 == b.u2 && a.v2 == b.v2;
}

// direction components are 0/1, length = max delta
static bool clipSeg(const Seg& s, int a, int b, ll lo, ll hi, Seg& out) {
    int du = s.u2 > s.u1, dv = s.v2 > s.v1;
    ll len = max(s.u2 - s.u1, s.v2 - s.v1);
    ll base = a * s.u1 + b * s.v1;
    int k = a * du + b * dv;
    ll t0 = 0, t1 = len;
    if (k == 0) {
        if (base < lo || base > hi) return false;
        out = s;
        return true;
    } else if (k == 1) {
        t0 = max(t0, lo - base);
        t1 = min(t1, hi - base);
    } else {
        t0 = max(t0, base - hi);
        t1 = min(t1, base - lo);
    }
    if (t0 > t1) return false;
    out = {s.u1 + t0 * du, s.v1 + t0 * dv, s.u1 + t1 * du, s.v1 + t1 * dv};
    return true;
}

static void normF(vector<Seg>& F) {
    sort(F.begin(), F.end());
    F.erase(unique(F.begin(), F.end()), F.end());
}

static vector<Seg> applyHex(const vector<Seg>& F, ll c, ll r) {
    vector<Seg> res;
    ll L[6][3] = {{1, 0, c - r}, {1, 0, c + r}, {0, 1, c - r}, {0, 1, c + r}, {1, -1, -r}, {1, -1, r}};
    for (auto& s : F) {
        Seg t = s, t2;
        if (!clipSeg(t, 1, 0, c - r, c + r, t2)) continue;
        t = t2;
        if (!clipSeg(t, 0, 1, c - r, c + r, t2)) continue;
        t = t2;
        if (!clipSeg(t, 1, -1, -r, r, t2)) continue;
        t = t2;
        for (int q = 0; q < 6; q++) {
            Seg p;
            if (clipSeg(t, (int)L[q][0], (int)L[q][1], L[q][2], L[q][2], p)) res.push_back(p);
        }
    }
    normF(res);
    return res;
}

static vector<Seg> hexInit(ll c, ll r) {
    vector<Seg> F = {{c + r, c, c + r, c + r}, {c, c + r, c + r, c + r}, {c - r, c, c, c + r},
                     {c - r, c - r, c - r, c},  {c - r, c - r, c, c - r}, {c, c - r, c + r, c}};
    normF(F);
    return F;
}

static vector<Seg> applySingle(const vector<Seg>& F, int coord, ll pa, ll pb, ll s) {
    ll m = min(pa, pb), M = max(pa, pb), d = M - m;
    vector<Seg> res;
    if (s < d) return res;
    int a = coord == 0 ? 1 : 0, b = coord == 0 ? 0 : 1;
    for (auto& sg : F) {
        Seg p;
        if (s == d) {
            if (clipSeg(sg, a, b, m, M, p)) res.push_back(p);
        } else {
            if (clipSeg(sg, a, b, M - s, M - s, p)) res.push_back(p);
            if (clipSeg(sg, a, b, m + s, m + s, p)) res.push_back(p);
        }
    }
    normF(res);
    return res;
}

static inline ll spread3(ll a, ll b, ll c) { return max({a, b, c}) - min({a, b, c}); }

typedef vector<ll> Hyp;
static vector<Hyp> H;
static double IMPK = 2.5, IMPT = 1.5;
static long long REMAINING = 0, LOCQ = 0;
static bool TIGHT = false;
struct AbortEx {};
static long long TIGHT_T = 4;
static double IMPT2 = 1e18;

static void canonDedupe() {
    for (auto& h : H) {
        ll mn = LLONG_MAX, mx = LLONG_MIN;
        for (int i = 1; i <= n; i++)
            if (h[i] != NONE) mn = min(mn, h[i]), mx = max(mx, h[i]);
        Hyp A = h, B = h;
        for (int i = 1; i <= n; i++)
            if (h[i] != NONE) A[i] = h[i] - mn, B[i] = mx - h[i];
        h = min(A, B);
    }
    sort(H.begin(), H.end());
    H.erase(unique(H.begin(), H.end()), H.end());
}

static vector<int> commonKnown() {
    vector<int> K;
    for (int i = 1; i <= n; i++) {
        bool ok = true;
        for (auto& h : H)
            if (h[i] == NONE) { ok = false; break; }
        if (ok) K.push_back(i);
    }
    return K;
}

static inline ll coordOf(const Seg& s, int i, bool second) {
    return i == 0 ? (second ? s.u2 : s.u1) : (second ? s.v2 : s.v1);
}

static void processGroup(const vector<int>& xs) {
    int k = xs.size();
    vector<int> Kc = commonKnown();
    int Hn = H.size();
    vector<ll> lo(Hn), hi(Hn);
    auto computeFrames = [&]() {
        lo.assign(H.size(), LLONG_MAX);
        hi.assign(H.size(), LLONG_MIN);
        for (size_t h = 0; h < H.size(); h++)
            for (int i = 1; i <= n; i++)
                if (H[h][i] != NONE) lo[h] = min(lo[h], H[h][i]), hi[h] = max(hi[h], H[h][i]);
    };
    computeFrames();
    vector<vector<Seg>> F(Hn);
    auto isDone = [&](size_t h, int i) {
        if (i >= k) return true;
        for (auto& s : F[h]) {
            ll a = coordOf(s, i, false), b = coordOf(s, i, true);
            if (a < lo[h] || b > hi[h]) return false;
        }
        return true;
    };
    auto allDone = [&]() {
        for (size_t h = 0; h < H.size(); h++)
            for (int i = 0; i < k; i++)
                if (!isDone(h, i)) return false;
        return true;
    };
    auto prune = [&]() {
        vector<Hyp> nH;
        vector<vector<Seg>> nF;
        for (size_t h = 0; h < H.size(); h++)
            if (!F[h].empty()) nH.push_back(H[h]), nF.push_back(F[h]);
        H = nH;
        F = nF;
        computeFrames();
    };

    if (k == 2) {
        int y = -1;
        ll bestM = LLONG_MIN;
        for (int c : Kc) {
            ll m = LLONG_MAX;
            for (size_t h = 0; h < H.size(); h++) m = min(m, min(H[h][c] - lo[h], hi[h] - H[h][c]));
            if (m > bestM) bestM = m, y = c;
        }
        int tz = -1;
        if (TIGHT) {
            int a = -1, b = -1;
            for (int c : Kc) {
                if (a == -1 || H[0][c] < H[0][a]) a = c;
                if (b == -1 || H[0][c] > H[0][b]) b = c;
            }
            if (a == b) a = Kc[0], b = Kc[1];
            y = a, tz = b;
        }
        ll r = ASK(y, xs[0], xs[1]);
        for (size_t h = 0; h < H.size(); h++) F[h] = hexInit(H[h][y], r);
        if (!allDone()) {
            int z = -1;
            ll bestZ = LLONG_MIN;
            for (int c : Kc) {
                if (c == y) continue;
                ll m = LLONG_MAX;
                for (size_t h = 0; h < H.size(); h++) {
                    ll d = llabs(H[h][c] - H[h][y]);
                    ll e = min(H[h][c] - lo[h], hi[h] - H[h][c]);
                    m = min(m, d * 4 - e);
                }
                if (m > bestZ) bestZ = m, z = c;
            }
            if (tz != -1) z = tz;
            ll s = ASK(z, xs[0], xs[1]);
            for (size_t h = 0; h < H.size(); h++) F[h] = applyHex(F[h], H[h][z], s);
            prune();
        }
    } else {
        for (size_t h = 0; h < H.size(); h++) F[h] = {{-BIG, 0, BIG, 0}};
    }

    bool locUsed = false;
    int iter = 0;
    while (true) {
        if (++iter > 80) throw AbortEx();
        prune();
        if (H.empty()) return;  // should not happen
        // find varying non-done coordinate
        int fh = -1, fi = -1;
        for (size_t h = 0; h < H.size() && fh < 0; h++)
            for (int i = 0; i < k && fh < 0; i++) {
                if (isDone(h, i)) continue;
                for (auto& s : F[h])
                    if (coordOf(s, i, false) != coordOf(s, i, true)) {
                        fh = h, fi = i;
                        break;
                    }
            }
        if (fh >= 0) {
            int a = -1, b = -1;
            for (int c : Kc) {
                if (a == -1 || H[fh][c] < H[fh][a]) a = c;
                if (b == -1 || H[fh][c] > H[fh][b]) b = c;
            }
            if (a == b) {
                a = Kc[0];
                b = Kc[1];
            }
            ll s = ASK(a, b, xs[fi]);
            for (size_t h = 0; h < H.size(); h++) F[h] = applySingle(F[h], fi, H[h][a], H[h][b], s);
            continue;
        }
        // items
        vector<map<array<ll, 2>, vector<int>>> items(H.size());
        ll amb = 0;
        for (size_t h = 0; h < H.size(); h++) {
            bool d[2] = {isDone(h, 0), isDone(h, 1)};
            for (int si = 0; si < (int)F[h].size(); si++) {
                auto& s = F[h][si];
                array<ll, 2> t = {d[0] ? NONE : s.u1, d[1] ? NONE : s.v1};
                items[h][t].push_back(si);
            }
            amb += (ll)items[h].size() - 1;
        }
        auto buildR = [&]() {
            vector<int> R;
            {
                set<int> Rs;
                for (size_t h = 0; h < H.size(); h++) {
                    int a = -1, b = -1;
                    for (int c : Kc) {
                        if (a == -1 || H[h][c] < H[h][a]) a = c;
                        if (b == -1 || H[h][c] > H[h][b]) b = c;
                    }
                    Rs.insert(a), Rs.insert(b);
                }
                vector<int> srt = Kc;
                sort(srt.begin(), srt.end(), [&](int x, int y) { return H[0][x] < H[0][y]; });
                int lim = 30;
                if ((int)srt.size() <= lim)
                    for (int c : srt) Rs.insert(c);
                else
                    for (int j = 0; j < lim; j++) Rs.insert(srt[(ll)j * (srt.size() - 1) / (lim - 1)]);
                R.assign(Rs.begin(), Rs.end());
            }
            return R;
        };
        if (amb == 0 && k == 2 && H.size() == 1 && F[0].size() >= 2 && hi[0] > lo[0]) {
            bool pts = true;
            for (auto& sg : F[0])
                if (sg.u1 != sg.u2 || sg.v1 != sg.v2) pts = false;
            if (pts) {
                ll cur = 0;
                for (int c : Kc) cur = max(cur, min(H[0][c] - lo[0], hi[0] - H[0][c]));
                ll nm = cur;
                for (auto& sg : F[0])
                    for (ll p : {sg.u1, sg.v1}) nm = max(nm, min(p - lo[0], hi[0] - p));
                double delta = (double)(nm - cur) / (double)(hi[0] - lo[0]);
                if (delta * IMPK * REMAINING > IMPT) {
                    vector<int> R = buildR();
                    array<int, 3> q = {-1, -1, -1};
                    for (int i = 0; i < 2 && q[0] < 0; i++)
                        for (size_t ai = 0; ai < R.size() && q[0] < 0; ai++)
                            for (size_t bi = ai + 1; bi < R.size() && q[0] < 0; bi++) {
                                set<ll> outs;
                                for (auto& sg : F[0])
                                    outs.insert(spread3(H[0][R[ai]], H[0][R[bi]], coordOf(sg, i, false)));
                                if (outs.size() == F[0].size()) q = {R[ai], R[bi], i};
                            }
                    if (q[0] >= 0) {
                        ll s = ASK(q[0], q[1], xs[q[2]]);
                        F[0] = applySingle(F[0], q[2], H[0][q[0]], H[0][q[1]], s);
                        continue;
                    }
                }
            } else if (!locUsed) {
                ll cur = 0;
                for (int c : Kc) cur = max(cur, min(H[0][c] - lo[0], hi[0] - H[0][c]));
                double best = 0;
                int bi = -1;
                ll bmin = 0, bmax = 0;
                for (int i = 0; i < 2; i++) {
                    ll mnU = LLONG_MAX, mxU = LLONG_MIN;
                    for (auto& sg : F[0]) mnU = min(mnU, coordOf(sg, i, false)), mxU = max(mxU, coordOf(sg, i, true));
                    double g = 0;
                    const int S = 64;
                    for (int t = 0; t < S; t++) {
                        double p = mnU + (mxU - mnU) * (t + 0.5) / S;
                        g += max(0.0, min(p - lo[0], hi[0] - p) - (double)cur);
                    }
                    g /= S;
                    if (g > best) best = g, bi = i, bmin = mnU, bmax = mxU;
                }
                double delta = best / (double)(hi[0] - lo[0]);
                if (bi >= 0 && delta * IMPK * REMAINING > IMPT2) {
                    // find two known points both <= bmin or both >= bmax
                    int a1 = -1, a2 = -1, c1 = -1, c2 = -1;
                    for (int c : Kc) {
                        ll p = H[0][c];
                        if (p <= bmin) {
                            if (a1 == -1 || p > H[0][a1]) a2 = a1, a1 = c;
                            else if (a2 == -1 || p > H[0][a2]) a2 = c;
                        }
                        if (p >= bmax) {
                            if (c1 == -1 || p < H[0][c1]) c2 = c1, c1 = c;
                            else if (c2 == -1 || p < H[0][c2]) c2 = c;
                        }
                    }
                    int qa = -1, qb = -1;
                    if (a2 != -1) qa = a1, qb = a2;
                    else if (c2 != -1) qa = c1, qb = c2;
                    if (qa != -1) {
                        locUsed = true;
                        LOCQ++;
                        ll s = ASK(qa, qb, xs[bi]);
                        F[0] = applySingle(F[0], bi, H[0][qa], H[0][qb], s);
                        continue;
                    }
                }
            }
        }
        bool branch = (amb == 0);
        if (!branch) {
            vector<int> R = buildR();
            bool constC[2] = {true, true};
            for (int i = 0; i < 2; i++) {
                if (i >= k) {
                    constC[i] = false;
                    continue;
                }
                for (size_t h = 0; h < H.size(); h++)
                    for (auto& s : F[h])
                        if (coordOf(s, i, false) != coordOf(s, i, true)) constC[i] = false;
            }
            ll bestW = LLONG_MAX, bestT = LLONG_MAX;
            array<int, 3> bestQ = {-1, -1, -1};  // type: (a,b,i) i>=0 single; (a,-1,-1) pair
            auto evalQ = [&](auto outcomeFn) -> pair<ll, ll> {
                // returns worst ambiguity, worst total
                map<ll, pair<ll, ll>> agg;  // outcome -> (amb sum, total)
                for (size_t h = 0; h < H.size(); h++) {
                    map<ll, ll> cnt;
                    for (auto& [t, segs] : items[h]) {
                        set<ll> outs;
                        for (int si : segs) outs.insert(outcomeFn(h, F[h][si]));
                        for (ll o : outs) cnt[o]++;
                    }
                    for (auto& [o, c] : cnt) {
                        agg[o].first += c - 1;
                        agg[o].second += c;
                    }
                }
                ll w = 0, t = 0;
                for (auto& [o, p] : agg) w = max(w, p.first), t = max(t, p.second);
                return {w, t};
            };
            for (int i = 0; i < k; i++) {
                if (!constC[i]) continue;
                for (size_t ai = 0; ai < R.size(); ai++)
                    for (size_t bi = ai + 1; bi < R.size(); bi++) {
                        int a = R[ai], b = R[bi];
                        auto [w, t] = evalQ([&](size_t h, const Seg& s) {
                            return spread3(H[h][a], H[h][b], coordOf(s, i, false));
                        });
                        if (w < bestW || (w == bestW && t < bestT)) bestW = w, bestT = t, bestQ = {a, b, i};
                    }
            }
            if (k == 2 && constC[0] && constC[1]) {
                for (int a : R) {
                    auto [w, t] = evalQ([&](size_t h, const Seg& s) { return spread3(H[h][a], s.u1, s.v1); });
                    if (w < bestW || (w == bestW && t < bestT)) bestW = w, bestT = t, bestQ = {a, -1, -1};
                }
            }
            if (bestQ[0] != -1 && bestW < amb) {
                if (bestQ[1] == -1) {
                    int a = bestQ[0];
                    ll s = ASK(a, xs[0], xs[1]);
                    for (size_t h = 0; h < H.size(); h++) F[h] = applyHex(F[h], H[h][a], s);
                } else {
                    int a = bestQ[0], b = bestQ[1], i = bestQ[2];
                    ll s = ASK(a, b, xs[i]);
                    for (size_t h = 0; h < H.size(); h++) F[h] = applySingle(F[h], i, H[h][a], H[h][b], s);
                }
                continue;
            }
            branch = true;
        }
        // branch
        vector<Hyp> nH;
        for (size_t h = 0; h < H.size(); h++) {
            for (auto& [t, segs] : items[h]) {
                Hyp p = H[h];
                for (int i = 0; i < k; i++) {
                    if (t[i] != NONE) {
                        p[xs[i]] = t[i];
                    } else {
                        ll v = NONE;
                        bool ok = true;
                        for (int si : segs) {
                            ll a = coordOf(F[h][si], i, false), b = coordOf(F[h][si], i, true);
                            if (a != b || (v != NONE && v != a)) {
                                ok = false;
                                break;
                            }
                            v = a;
                        }
                        if (ok && v != NONE) p[xs[i]] = v;
                    }
                }
                nH.push_back(p);
            }
        }
        H = nH;
        canonDedupe();
        if (H.size() > 300) throw AbortEx();
        return;
    }
}

static vector<int> zeroCands(const Hyp& h) {
    ll mn = LLONG_MAX, mx = LLONG_MIN;
    for (int i = 1; i <= n; i++)
        if (h[i] != NONE) mn = min(mn, h[i]), mx = max(mx, h[i]);
    int cmn = 0, cmx = 0, amn = -1, amx = -1;
    for (int i = 1; i <= n; i++)
        if (h[i] != NONE) {
            if (h[i] == mn) cmn++, amn = i;
            if (h[i] == mx) cmx++, amx = i;
        }
    vector<int> z;
    if (mn == mx) return z;
    if (cmn == 1) z.push_back(amn);
    if (cmx == 1) z.push_back(amx);
    return z;
}

static long long USED, BUDGET;
static bool GUARD;
static vector<int> CAND, ORD, RANKV;
static map<array<int, 3>, ll> CACHE;

static bool cached(int i, int j, int k) {
    array<int, 3> t = {i, j, k};
    sort(t.begin(), t.end());
    return CACHE.count(t) > 0;
}
static vector<int> TOUCH;  // last time an element appeared in a real query
static int cachedCount(const vector<int>& S) {
    int cnt = 0;
    for (int e = 0; e < 4; e++) {
        int t[3], c = 0;
        for (int j = 0; j < 4; j++)
            if (j != e) t[c++] = S[j];
        if (cached(t[0], t[1], t[2])) cnt++;
    }
    return cnt;
}
static vector<int> firstRound(const vector<int>& C) {
    int m = C.size();
    vector<char> inC(n + 1, 0);
    for (int c : C) inC[c] = 1;
    vector<int> pool;
    for (int c : C)
        if (TOUCH[c]) pool.push_back(c);
    sort(pool.begin(), pool.end(), [](int a, int b) { return TOUCH[a] > TOUCH[b]; });
    if (pool.size() > 10) pool.resize(10);
    vector<int> pads;
    if (m % 2 == 1) {
        vector<char> inP(n + 1, 0);
        for (int c : pool) inP[c] = 1;
        set<int> ps;
        for (auto& [t, v] : CACHE) {
            int k = inP[t[0]] + inP[t[1]] + inP[t[2]];
            if (k >= 2)
                for (int x : t)
                    if (!inC[x]) ps.insert(x);
        }
        vector<int> pv(ps.begin(), ps.end());
        sort(pv.begin(), pv.end(), [](int a, int b) { return TOUCH[a] > TOUCH[b]; });
        if (pv.size() > 8) pv.resize(8);
        pads = pv;
    }
    vector<int> U = pool;
    for (int p : pads) U.push_back(p);
    {
        int add = 0;
        for (int c : C) {
            if (add >= 3) break;
            if (!TOUCH[c]) U.push_back(c), add++;
        }
    }
    int maxPads = (m % 2 == 1) ? 1 : 0;
    vector<int> best;
    int bestC = -1;
    auto fill = [&](vector<int> S) {
        for (int c : C) {
            if (S.size() >= 4) break;
            if (find(S.begin(), S.end(), c) == S.end()) S.push_back(c);
        }
        for (int c : ORD) {
            if (S.size() >= 4) break;
            if (find(S.begin(), S.end(), c) == S.end()) S.push_back(c);
        }
        return S;
    };
    int u = U.size();
    if (u < 4) {
        best = fill(U);
    } else {
        for (int i = 0; i < u; i++)
            for (int j = i + 1; j < u; j++)
                for (int k = j + 1; k < u; k++)
                    for (int l = k + 1; l < u; l++) {
                        vector<int> S = {U[i], U[j], U[k], U[l]};
                        int np = 0;
                        for (int x : S)
                            if (!inC[x]) np++;
                        if (np > maxPads) continue;
                        int cc = cachedCount(S);
                        if (cc > bestC) bestC = cc, best = S;
                    }
        if (best.empty()) best = fill(pool);
    }
    return best;
}
static ll classicEst(const vector<int>& C) {
    ll m = C.size();
    if (m <= 2) return 0;
    vector<int> S = firstRound(C);
    ll cnt = 0;
    for (int e = 0; e < 4; e++) {
        int t[3], c = 0;
        for (int j = 0; j < 4; j++)
            if (j != e) t[c++] = S[j];
        if (!cached(t[0], t[1], t[2])) cnt++;
    }
    return cnt + 4 * ((m - 2 + 1) / 2 - 1);
}
static void sortByRank(vector<int>& C) {
    sort(C.begin(), C.end(), [](int a, int b) { return RANKV[a] < RANKV[b]; });
    C.erase(unique(C.begin(), C.end()), C.end());
}
static void updateCand(vector<int> opt) {
    // intersect with current candidate set; accept if estimate does not increase
    vector<char> inC(n + 1, 0), seen(n + 1, 0);
    for (int c : CAND) inC[c] = 1;
    vector<int> inter;
    for (int c : opt)
        if (inC[c] && !seen[c]) inter.push_back(c), seen[c] = 1;
    sortByRank(inter);
    if (inter.size() < CAND.size() && classicEst(inter) <= classicEst(CAND)) CAND = inter;
}
static void classicFinish() {
    GUARD = false;
    vector<int> Z = CAND;
    while (Z.size() > 2) {
        vector<int> S = firstRound(Z);
        ll q[4];
        for (int e = 0; e < 4; e++) {
            int t[3], c = 0;
            for (int j = 0; j < 4; j++)
                if (j != e) t[c++] = S[j];
            q[e] = ASK(t[0], t[1], t[2]);
        }
        int idx[4] = {0, 1, 2, 3};
        stable_sort(idx, idx + 4, [&](int x, int y) { return q[x] > q[y]; });
        int r0 = S[idx[0]], r1 = S[idx[1]];
        vector<int> nz;
        for (int c : Z)
            if (c != r0 && c != r1) nz.push_back(c);
        Z = nz;
    }
    if (Z.empty()) Z.push_back(1);
    ANSWER(Z[0], Z.size() > 1 ? Z[1] : Z[0]);
}

static vector<int> zeroCands(const Hyp& h);
static vector<int> zeroAll() {
    set<int> Z;
    for (auto& h : H)
        for (int z : zeroCands(h)) Z.insert(z);
    return vector<int>(Z.begin(), Z.end());
}

static void solveInner(unsigned seed);
static void solve(unsigned seed) {
    H.clear();
    CACHE.clear();
    TOUCH.assign(n + 1, 0);
    USED = 0;
    BUDGET = 2LL * n - 2;
    GUARD = false;
    auto raw = ASK;
    ASK = [raw](int i, int j, int k) -> ll {
        array<int, 3> t = {i, j, k};
        sort(t.begin(), t.end());
        auto it = CACHE.find(t);
        if (it != CACHE.end()) return it->second;
        if (GUARD) {
            CACHE[t] = -1;
            int oi = TOUCH[i], oj = TOUCH[j], ok = TOUCH[k];
            TOUCH[i] = TOUCH[j] = TOUCH[k] = (int)USED + 1;
            ll est = classicEst(CAND);
            CACHE.erase(t);
            TOUCH[i] = oi, TOUCH[j] = oj, TOUCH[k] = ok;
            if (USED + 1 + est > BUDGET)  throw AbortEx();
        }
        ll r = raw(i, j, k);
        USED++;
        TOUCH[i] = TOUCH[j] = TOUCH[k] = (int)USED;
        CACHE[t] = r;
        return r;
    };
    try {
        solveInner(seed);
    } catch (AbortEx&) {
        classicFinish();
    }
    ASK = raw;
}

static void splitHyps() {
    // try to separate hypotheses with a query among commonly known elements
    vector<int> Kc = commonKnown();
    if (Kc.size() < 3) return;
    vector<int> R;
    if (Kc.size() <= 24) R = Kc;
    else {
        set<int> Rs;
        for (auto& h : H) {
            int a = -1, b = -1;
            for (int c : Kc) {
                if (a == -1 || h[c] < h[a]) a = c;
                if (b == -1 || h[c] > h[b]) b = c;
            }
            Rs.insert(a), Rs.insert(b);
        }
        for (int j = 0; j < 20; j++) Rs.insert(Kc[(ll)j * (Kc.size() - 1) / 19]);
        R.assign(Rs.begin(), Rs.end());
    }
    size_t bestW = H.size();
    array<int, 3> bq = {-1, -1, -1};
    for (size_t a = 0; a < R.size(); a++)
        for (size_t b = a + 1; b < R.size(); b++)
            for (size_t c = b + 1; c < R.size(); c++) {
                if (cached(R[a], R[b], R[c])) continue;
                map<ll, size_t> cnt;
                size_t w = 0;
                for (auto& h : H) w = max(w, ++cnt[spread3(h[R[a]], h[R[b]], h[R[c]])]);
                if (w < bestW) bestW = w, bq = {R[a], R[b], R[c]};
            }
    if (bq[0] == -1) return;
    ll s = ASK(bq[0], bq[1], bq[2]);
    vector<Hyp> nH;
    for (auto& h : H)
        if (spread3(h[bq[0]], h[bq[1]], h[bq[2]]) == s) nH.push_back(h);
    H = nH;
}

static void solveInner(unsigned seed) {
    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    mt19937 rng(seed);
    shuffle(order.begin(), order.end(), rng);
    ORD = order;
    RANKV.assign(n + 1, 0);
    for (int i = 0; i < n; i++) RANKV[order[i]] = i;
    CAND = order;
    int b[4] = {order[0], order[1], order[2], order[3]};
    ll Q[4] = {-1, -1, -1, -1};
    auto askQ = [&](int e) {
        int t[3], c = 0;
        for (int j = 0; j < 4; j++)
            if (j != e) t[c++] = b[j];
        Q[e] = ASK(t[0], t[1], t[2]);
    };
    auto enumerate = [&]() {
        H.clear();
        ll D = *max_element(Q, Q + 4);
        int perm[4] = {0, 1, 2, 3};
        do {
            ll p[4];
            bool unk[4] = {false, false, false, false};
            p[perm[0]] = 0;
            p[perm[3]] = D;
            if (Q[perm[0]] >= 0) p[perm[1]] = D - Q[perm[0]]; else unk[perm[1]] = true;
            if (Q[perm[3]] >= 0) p[perm[2]] = Q[perm[3]]; else unk[perm[2]] = true;
            if (unk[perm[1]]) p[perm[1]] = p[perm[2]];
            if (unk[perm[2]]) p[perm[2]] = p[perm[1]];
            if (!(0 <= p[perm[1]] && p[perm[1]] <= p[perm[2]] && p[perm[2]] <= D)) continue;
            bool ok = true;
            for (int e = 0; e < 4 && ok; e++) {
                if (Q[e] < 0) continue;
                ll t[3];
                int c = 0;
                for (int j = 0; j < 4; j++)
                    if (j != e) t[c++] = p[j];
                if (spread3(t[0], t[1], t[2]) != Q[e]) ok = false;
            }
            if (!ok) continue;
            Hyp h(n + 1, NONE);
            for (int j = 0; j < 4; j++)
                if (!unk[j]) h[b[j]] = p[j];
            H.push_back(h);
        } while (next_permutation(perm, perm + 4));
        // forget elements not known in every hypothesis (they are interior there)
        for (int j = 0; j < 4; j++) {
            bool all = true;
            for (auto& h : H)
                if (h[b[j]] == NONE) all = false;
            if (all) continue;
            for (auto& h : H) {
                if (h[b[j]] == NONE) continue;
                ll mn = LLONG_MAX, mx = LLONG_MIN;
                for (int t = 0; t < 4; t++)
                    if (t != j && h[b[t]] != NONE) mn = min(mn, h[b[t]]), mx = max(mx, h[b[t]]);
                if (mn < h[b[j]] && h[b[j]] < mx) h[b[j]] = NONE;
            }
        }
        canonDedupe();
    };
    for (int e = 0; e < 3; e++) askQ(e);
    enumerate();
    {
        // accept only if all hypotheses agree on unique extreme indices
        bool good = true;
        int L0 = -1, R0 = -1;
        for (auto& h : H) {
            ll mn = LLONG_MAX, mx = LLONG_MIN;
            for (int j = 0; j < 4; j++)
                if (h[b[j]] != NONE) mn = min(mn, h[b[j]]), mx = max(mx, h[b[j]]);
            int cmn = 0, cmx = 0, amn = -1, amx = -1;
            for (int j = 0; j < 4; j++)
                if (h[b[j]] != NONE) {
                    if (h[b[j]] == mn) cmn++, amn = b[j];
                    if (h[b[j]] == mx) cmx++, amx = b[j];
                }
            if (mn == mx || cmn != 1 || cmx != 1) good = false;
            if (amn > amx) swap(amn, amx);
            if (L0 == -1) L0 = amn, R0 = amx;
            else if (L0 != amn || R0 != amx) good = false;
        }
        if (n == 4) good = true;
        if (!good) {
            askQ(3);
            enumerate();
        }
    }
    {
        vector<int> opt(order.begin() + 4, order.end());
        vector<int> Z = zeroAll();
        vector<int> part;
        if (Q[3] >= 0) {
            // classic elimination on the bootstrap four
            int idx[4] = {0, 1, 2, 3};
            stable_sort(idx, idx + 4, [&](int x, int y) { return Q[x] > Q[y]; });
            for (int j = 2; j < 4; j++) {
                int e = b[idx[j]];
                if (find(Z.begin(), Z.end(), e) != Z.end()) part.push_back(e);
            }
        } else {
            part = Z;
        }
        for (int c : part) opt.push_back(c);
        updateCand(opt);
    }
    GUARD = true;
    size_t pos = 4;
    while (pos < order.size()) {
        REMAINING = (order.size() - pos) / 2;
        TIGHT = (BUDGET - USED - classicEst(CAND)) < TIGHT_T;
        if (H.size() > 1 && !TIGHT && order.size() - pos >= 6) splitHyps();
        if (order.size() - pos >= 2) {
            processGroup({order[pos], order[pos + 1]});
            pos += 2;
        } else {
            processGroup({order[pos]});
            pos += 1;
        }
        vector<int> opt(order.begin() + pos, order.end());
        for (int z : zeroAll()) opt.push_back(z);
        updateCand(opt);
    }
    // final
    while (true) {
        set<int> Z;
        map<int, int> freq;
        for (auto& h : H)
            for (int z : zeroCands(h)) Z.insert(z), freq[z]++;
        if (Z.size() <= 2 || H.size() <= 1) {
            vector<int> zz(Z.begin(), Z.end());
            sort(zz.begin(), zz.end(), [&](int a, int b) { return freq[a] > freq[b]; });
            if (zz.empty()) break;
            ANSWER(zz[0], zz.size() > 1 ? zz[1] : zz[0]);
            return;
        }
        vector<int> U;
        for (int i = 1; i <= n; i++) {
            bool any = false;
            for (auto& h : H)
                if (h[i] != NONE) any = true;
            if (any) U.push_back(i);
        }
        if ((int)U.size() > 60) {
            set<int> Rs(Z.begin(), Z.end());
            vector<int> srt = commonKnown();
            sort(srt.begin(), srt.end(), [&](int x, int y) { return H[0][x] < H[0][y]; });
            if (srt.size() > 1)
                for (int j = 0; j < 50; j++) Rs.insert(srt[(ll)j * (srt.size() - 1) / 49]);
            U.assign(Rs.begin(), Rs.end());
        }
        size_t bestW = H.size();
        array<int, 3> bq = {-1, -1, -1};
        for (size_t a = 0; a < U.size(); a++)
            for (size_t b2 = a + 1; b2 < U.size(); b2++)
                for (size_t c = b2 + 1; c < U.size(); c++) {
                    map<ll, size_t> cnt;
                    size_t wild = 0, w = 0;
                    for (auto& h : H) {
                        ll x = h[U[a]], y = h[U[b2]], z = h[U[c]];
                        if (x == NONE || y == NONE || z == NONE)
                            wild++;
                        else
                            w = max(w, ++cnt[spread3(x, y, z)]);
                    }
                    w += wild;
                    if (w < bestW) bestW = w, bq = {U[a], U[b2], U[c]};
                }
        if (bq[0] == -1) break;
        ll s = ASK(bq[0], bq[1], bq[2]);
        vector<Hyp> nH;
        for (auto& h : H) {
            ll x = h[bq[0]], y = h[bq[1]], z = h[bq[2]];
            if (x == NONE || y == NONE || z == NONE || spread3(x, y, z) == s) nH.push_back(h);
        }
        H = nH;
        updateCand(zeroAll());
    }
    // fallback: classic elimination over candidate set
    updateCand(zeroAll());
    classicFinish();
}

int main() {
    if (!(cin >> n)) return 0;
    ASK = [&](int i, int j, int k) -> ll {
        cout << "? " << i << " " << j << " " << k << endl;
        ll r;
        if (!(cin >> r) || r < 0) exit(0);
        return r;
    };
    ANSWER = [&](int i, int j) { cout << "! " << i << " " << j << endl; };
    solve(20261123u);
    return 0;
}

