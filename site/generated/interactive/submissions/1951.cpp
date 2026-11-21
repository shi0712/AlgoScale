#include <bits/stdc++.h>
using namespace std;
using i64 = long long;
using u64 = unsigned long long;
using u128 = __uint128_t;

// ---------------------------------------------------------------
// Problem: sequence starts as a_p = p.  Jury picks 1 <= i < j < k,
// j-i > 1, reverses [i, j-1] and [j, k].
//
// Resulting permutation:
//   p <  i            : a_p = p
//   i <= p <= j-1     : a_p = i + (j-1) - p        (block1, decreasing)
//   j <= p <= k       : a_p = j + k - p            (block2, decreasing)
//   p >  k            : a_p = p
// All values of block1 are < all values of block2, so the inversion
// count of [l,r] is  T(alpha) + T(beta),  where alpha = |[l,r] n block1|,
// beta = |[l,r] n block2| and T(m) = m(m-1)/2.
//
// Let L1 = j-i (>=2) and L2 = k-j+1 (>=2), S = L1+L2.
// total = T(L1) + T(L2).
// ---------------------------------------------------------------

static inline i64 tri(i64 m) { return m * (m - 1) / 2; }

static u64 isqrt_u128(u128 x) {
    if (x == 0) return 0;
    u64 r = (u64)sqrtl((long double)x);
    while ((u128)(r + 1) * (r + 1) <= x) ++r;
    while ((u128)r * r > x) --r;
    return r;
}

// smallest L >= 0 with tri(L) >= need   (need > 0)
static i64 leastTriAtLeast(i64 need) {
    if (need <= 0) return 0;
    i64 L = (i64)((1 + isqrt_u128((u128)8 * (u128)need + 1)) / 2);
    while (tri(L) < need) ++L;
    while (L > 0 && tri(L - 1) >= need) --L;
    return L;
}

// largest L >= 0 with tri(L) <= total
static i64 largestTriAtMost(i64 total) {
    if (total <= 0) return 0;
    i64 L = leastTriAtLeast(total);
    if (tri(L) > total) --L;
    return L;
}

// smallest L >= 2 with tri(L) == rem, or -1
static i64 solveTriGe2(i64 rem) {
    if (rem < 1) return -1;
    i64 L = (i64)((1 + isqrt_u128((u128)8 * (u128)rem + 1)) / 2);
    for (i64 d = -2; d <= 2; ++d) {
        i64 t = L + d;
        if (t >= 2 && tri(t) == rem) return t;
    }
    return -1;
}

// min possible S = L1+L2 for the given total.
// max over L1+L2=S of T(L1)+T(L2) is T(S-2)+T(2) = T(S-2)+1, hence
// S >= smallest S with T(S-2)+1 >= total.
static i64 minSFromTotal(i64 total) {
    i64 need = total - 1;                 // T(S-2) >= need
    i64 t = leastTriAtLeast(need);        // smallest t with T(t) >= need
    i64 S = t + 2;
    if (S < 4) S = 4;
    return S;
}

static inline i64 mintri2Sum(i64 s) {
    i64 a = s / 2, b = s - a;
    return tri(a) + tri(b);
}

// max possible S = L1+L2 for the given total (T is convex => balanced minimum)
static i64 maxSFromTotal(i64 total, i64 n) {
    i64 S = 1 + (i64)isqrt_u128((u128)4 * (u128)total + 1);
    if (S < 4) S = 4;
    while (S > 4 && mintri2Sum(S) > total) --S;
    while (mintri2Sum(S + 1) <= total) ++S;
    if (S > n) S = n;
    if (S < 4) S = 4;
    return S;
}

struct Obs { i64 l, r, v; };

// inversion count of [l,r] for the permutation determined by (ii, L1, L2)
static i64 predictVal(i64 l, i64 r, i64 ii, i64 L1, i64 L2) {
    i64 a = 0, b = 0;
    i64 lo = max(l, ii), hi = min(r, ii + L1 - 1);
    if (lo <= hi) a = hi - lo + 1;
    lo = max(l, ii + L1); hi = min(r, ii + L1 + L2 - 1);
    if (lo <= hi) b = hi - lo + 1;
    return tri(a) + tri(b);
}

// Main strategy.  ask(l,r) performs one interaction (1 <= l <= r <= n).
static void solveAlgo(i64 n, i64 (*ask)(i64, i64), i64 &oi, i64 &oj, i64 &ok) {
    vector<Obs> obs;
    auto Q = [&](i64 l, i64 r) -> i64 {
        i64 v = ask(l, r);
        obs.push_back({l, r, v});
        return v;
    };

    i64 total = Q(1, n);
    i64 Lmax = largestTriAtMost(total);
    i64 Slb = minSFromTotal(total);
    i64 Sub = maxSFromTotal(total, n);
    if (Sub < Slb) Sub = Slb;

    // ---------- bounds on i ----------
    // prefix(m) = 0        <=> m <= i
    // prefix(m) = total    <=> m >= k                    => i <= m-S+1 <= m-Slb+1
    // 0 < prefix(m) < total<=> i < m < k                 => m-S+2 <= i <= m-1
    // suffix(m) = 0        <=> m > k                     => i <= m-S   <= m-Slb
    // suffix(m) = total    <=> m <= i
    // 0 < suffix(m) < total<=> i < m <= k                => m-Sub+1 <= i <= m-1
    // When total is small enough, enumerate every representation
    // total = T(L1)+T(L2) (ordered pairs, L1,L2 >= 2) up front.  This lets us
    // turn an "interior" answer into a tiny set of explicit hypotheses
    // (i, L1, L2) and finish without further searching.
    const i64 ENUM_LIMIT = 10000000;
    vector<pair<i64, i64>> Hpre;
    {
        i64 L2cur = Lmax;
        if (Lmax <= ENUM_LIMIT) {
            for (i64 L1 = 2; tri(L1) <= total; ++L1) {
                i64 rem = total - tri(L1);
                while (L2cur >= 2 && tri(L2cur) > rem) --L2cur;
                if (L2cur < 2) break;
                if (tri(L2cur) == rem) Hpre.push_back({L1, L2cur});
            }
        }
    }

    i64 lo = 1, hi = n - Slb + 1;      // invariant: lo <= i <= hi
    auto applyPrefix = [&](i64 mm, i64 vv) {
        if (vv == 0) {
            if (mm > lo) lo = mm;
        } else if (vv == total) {
            i64 nh = mm - Slb + 1;
            if (nh < hi) hi = nh;
        } else {
            i64 nl = mm - Sub + 2;
            if (nl > lo) lo = nl;
            if (mm - 1 < hi) hi = mm - 1;
        }
    };
    auto applySuffix = [&](i64 mm, i64 vv) {
        if (vv == 0) {
            i64 nh = mm - Slb;
            if (nh < hi) hi = nh;
        } else if (vv == total) {
            if (mm > lo) lo = mm;
        } else {
            i64 nl = mm - Sub + 1;
            if (nl > lo) lo = nl;
            if (mm - 1 < hi) hi = mm - 1;
        }
    };
    auto verify = [&](i64 ii, i64 L1, i64 L2) -> bool {
        if (ii < 1 || L1 < 2 || L2 < 2) return false;
        if (ii + L1 + L2 - 1 > n) return false;
        for (const auto &o : obs)
            if (predictVal(o.l, o.r, ii, L1, L2) != o.v) return false;
        return true;
    };

    bool solved = false;
    i64 fi = 0, fL1 = 0, fL2 = 0;

    while (lo < hi) {
        // Choose the probe m (in [lo+1,hi]) that minimises the worst remaining
        // case.  The interior outcome is cheap: block1 hits are resolved by the
        // "triangular value" test and block2 hits by one suffix probe.
        const i64 INT_COST = 3;
        auto cost = [&](i64 m) -> i64 {
            i64 c = hi - m + 1;                                  // outcome 0
            if (m - Slb + 1 >= lo) c = max(c, m - Slb - lo + 2);  // outcome total
            if (m - 1 >= lo) c = max(c, INT_COST);                // interior
            return c;
        };
        i64 cand[10];
        int cc = 0;
        auto addc = [&](i64 m) {
            if (m < lo + 1) m = lo + 1;
            if (m > hi) m = hi;
            cand[cc++] = m;
        };
        addc((lo + hi) / 2);
        addc((lo + hi) / 2 + 1);
        addc((lo + hi + Slb - 1) / 2);
        addc((lo + hi + Slb - 1) / 2 + 1);
        addc((lo + hi + Slb - 1) / 2 - 1);
        addc(lo + 1);
        addc(hi);
        i64 m = cand[0], best = cost(cand[0]);
        for (int t = 1; t < cc; ++t) {
            i64 c = cost(cand[t]);
            if (c < best || (c == best && cand[t] > m)) { best = c; m = cand[t]; }
        }
        i64 v = Q(1, m);
        applyPrefix(m, v);
        if (lo >= hi) break;
        if (v > 0 && v < total) {
            // -------- try to resolve everything from this answer --------
            if (!Hpre.empty()) {
                vector<array<i64, 3>> hyps;
                i64 sA = solveTriGe2(v);
                for (auto &pr : Hpre) {
                    i64 L1 = pr.first, L2 = pr.second;
                    if (sA >= 2 && sA <= L1) {          // m inside block1
                        i64 ii = m - sA + 1;
                        if (ii >= lo && ii <= hi && ii >= 1 && ii + L1 + L2 - 1 <= n &&
                            verify(ii, L1, L2))
                            hyps.push_back({ii, L1, L2});
                    }
                    i64 u = v - tri(L1);
                    i64 t = (u == 0) ? 1 : ((u >= 1) ? solveTriGe2(u) : -1);
                    if (t >= 1 && t <= L2) {            // m inside block2
                        i64 ii = m - t - L1 + 1;
                        if (ii >= lo && ii <= hi && ii >= 1 && ii + L1 + L2 - 1 <= n &&
                            verify(ii, L1, L2))
                            hyps.push_back({ii, L1, L2});
                    }
                }
                sort(hyps.begin(), hyps.end());
                hyps.erase(unique(hyps.begin(), hyps.end()), hyps.end());
                if (hyps.size() == 1) {
                    fi = hyps[0][0]; fL1 = hyps[0][1]; fL2 = hyps[0][2];
                    solved = true; break;
                }
                if (hyps.size() >= 2 && hyps.size() <= 24) {
                    int guard = 0;
                    while (hyps.size() > 1 && ++guard < 60) {
                        i64 ql = -1, qr = -1, bestWorst = (i64)1 << 62;
                        vector<pair<i64, i64>> cands;   // candidate queries
                        for (auto &h : hyps) {
                            i64 a = h[0];
                            i64 b = h[0] + h[1] - 1;
                            i64 c2 = h[0] + h[1] + h[2] - 1;
                            for (i64 p : {a, b, c2}) {
                                cands.push_back({1, p});
                                cands.push_back({p, n});
                            }
                        }
                        for (auto &qq : cands) {
                            i64 l0 = qq.first, r0 = qq.second;
                            if (l0 < 1 || r0 > n || l0 > r0) continue;
                            i64 worst = 0;
                            for (auto &h : hyps) {
                                i64 pv = predictVal(l0, r0, h[0], h[1], h[2]);
                                i64 cntn = 0;
                                for (auto &h2 : hyps)
                                    if (predictVal(l0, r0, h2[0], h2[1], h2[2]) == pv) ++cntn;
                                worst = max(worst, cntn);
                            }
                            if (worst < bestWorst) { bestWorst = worst; ql = l0; qr = r0; }
                        }
                        if (ql < 0) break;
                        i64 vv = Q(ql, qr);
                        vector<array<i64, 3>> nn;
                        for (auto &h : hyps)
                            if (predictVal(ql, qr, h[0], h[1], h[2]) == vv) nn.push_back(h);
                        if (nn.empty() || nn.size() == hyps.size()) break;
                        hyps.swap(nn);
                    }
                    if (hyps.size() == 1) {
                        fi = hyps[0][0]; fL1 = hyps[0][1]; fL2 = hyps[0][2];
                        solved = true; break;
                    }
                }
            }
            // -------- determine whether m lies in block1 or block2 --------
            // block1: v = T(m-i+1), so i = m-s+1 with T(s)=v.
            // Probe i: prefix(i)=0 and prefix(i+1)>0 force i = m-s+1.
            i64 s = solveTriGe2(v);
            if (s >= 2 && s <= Lmax) {
                i64 ip = m - s + 1;
                if (ip >= lo && ip <= hi) {
                    i64 p1 = Q(1, ip);
                    applyPrefix(ip, p1);
                    if (lo < hi && p1 == 0 && lo == ip) {
                        i64 p2 = Q(1, ip + 1);
                        applyPrefix(ip + 1, p2);
                    }
                    if (lo >= hi) break;   // i pinned (block1 case)
                }
            }
            // -------- now m lies in block2:  v = T(L1)+T(t), t = m-j+1 --------
            // suffix(m) = T(k-m+1).  With t' = k-m,  L2 = t+t', and
            // total - v = T(L2) - T(t) = t*t' + T(t'), which determines t.
            i64 suf = Q(m, n);
            applySuffix(m, suf);
            i64 w = solveTriGe2(suf);          // suf = T(w), w = k-m+1
            if (w >= 2) {
                i64 tp = w - 1;                // k - m
                i64 d = total - v;
                if (tp >= 1) {
                    i64 num = d - tri(tp);
                    if (num >= 0 && num % tp == 0) {
                        i64 t = num / tp;
                        i64 rem = v - tri(t);
                        i64 L1 = solveTriGe2(rem);
                        if (t >= 1 && L1 >= 2) {
                            i64 L2 = t + tp;
                            i64 jj = m - t + 1;
                            i64 ii = jj - L1;
                            i64 kk = m + tp;
                            if (ii >= 1 && kk <= n && ii + L1 + L2 - 1 == kk &&
                                verify(ii, L1, L2)) {
                                fi = ii; fL1 = L1; fL2 = L2; solved = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (lo >= hi) break;
        }
    }

    i64 i;
    if (solved) {
        i = fi;
    } else {
        i = lo;
        // ---------- determine (L1, L2) ----------
        // When total is small enough, enumerate every representation
        // total = T(L1)+T(L2) (ordered pairs, L1,L2 >= 2) and simply filter
        // by all answers observed so far; often the lengths are already
        // forced with no extra query.
        vector<pair<i64, i64>> H;
        for (auto &c : Hpre) {
            if (i + c.first + c.second - 1 > n) continue;
            bool ok = true;
            for (const auto &o : obs)
                if (predictVal(o.l, o.r, i, c.first, c.second) != o.v) { ok = false; break; }
            if (ok) H.push_back(c);
        }
        if (!H.empty()) {
            // keep splitting the surviving set with informative queries
            while (H.size() > 1) {
                i64 bestM = -1, bestWorst = (i64)1 << 62;
                if (H.size() <= 64) {
                    vector<i64> candM;
                    for (auto &c : H) {
                        candM.push_back(c.first);
                        candM.push_back(c.first + c.second - 1);
                        if (c.first > 2) candM.push_back(c.first - 1);
                    }
                    sort(candM.begin(), candM.end());
                    candM.erase(unique(candM.begin(), candM.end()), candM.end());
                    for (i64 mm : candM) {
                        if (mm < 1 || i + mm > n) continue;
                        i64 worst = 0;
                        for (auto &c : H) {
                            i64 pv = predictVal(1, i + mm, i, c.first, c.second);
                            i64 cntn = 0;
                            for (auto &c2 : H)
                                if (predictVal(1, i + mm, i, c2.first, c2.second) == pv) ++cntn;
                            worst = max(worst, cntn);
                        }
                        if (worst < bestWorst) { bestWorst = worst; bestM = mm; }
                    }
                } else {
                    sort(H.begin(), H.end());
                    bestM = H[H.size() / 2].first;
                }
                if (bestM < 1) bestM = 1;
                if (i + bestM > n) bestM = n - i;
                if (bestM < 1) break;
                i64 vv = Q(1, i + bestM);
                vector<pair<i64, i64>> nn;
                for (auto &c : H)
                    if (predictVal(1, i + bestM, i, c.first, c.second) == vv) nn.push_back(c);
                if (nn.empty() || nn.size() == H.size()) break;
                H.swap(nn);
            }
            fL1 = H[0].first;
            fL2 = H[0].second;
        } else {
            // -------- determine L1 with one probing query --------
            // X = Lmax satisfies X < S (because T(S) > total >= T(Lmax)).
            // Query [i, i+X-1]:
            //   L1 == X -> v = T(X)
            //   L1 <  X -> v = T(L1)+T(X-L1)  =>  L1^2 - X*L1 + T(X) - v = 0
            i64 X = Lmax;
            i64 v = Q(i, i + X - 1);
            vector<pair<i64, i64>> cands;   // (L1, L2)
            auto consider = [&](i64 L1) {
                if (L1 < 2 || L1 > Lmax) return;
                i64 rem = total - tri(L1);
                i64 L2 = solveTriGe2(rem);
                if (L2 < 2) return;
                if (i + L1 + L2 - 1 > n) return;
                for (const auto &o : obs)
                    if (predictVal(o.l, o.r, i, L1, L2) != o.v) return;
                for (auto &c : cands)
                    if (c.first == L1) return;
                cands.push_back({L1, L2});
            };
            i64 disc = 4 * v + 2 * X - X * X;
            if (disc >= 0) {
                u64 sq = isqrt_u128((u128)disc);
                if ((u128)sq * sq == (u128)disc) {
                    i64 ss = (i64)sq;
                    if ((X + ss) % 2 == 0) consider((X + ss) / 2);
                    if (ss != 0 && (X - ss) % 2 == 0) consider((X - ss) / 2);
                }
            }
            if (cands.empty())
                for (i64 t = max<i64>(2, X - 2); t <= min<i64>(Lmax, X + 2); ++t) consider(t);

            while (cands.size() > 1) {
                i64 mn = cands[0].first;
                for (auto &c : cands) mn = min(mn, c.first);
                i64 ql = 1, qr = i + mn;
                i64 vv = Q(ql, qr);
                vector<pair<i64, i64>> nc;
                for (auto &c : cands)
                    if (predictVal(ql, qr, i, c.first, c.second) == vv) nc.push_back(c);
                if (!nc.empty() && nc.size() < cands.size()) { cands.swap(nc); continue; }
                bool progressed = false;
                for (i64 mm = i + 1; mm <= min(n, i + 500000); ++mm) {
                    i64 q0 = predictVal(1, mm, i, cands[0].first, cands[0].second);
                    bool diff = false;
                    for (size_t t = 1; t < cands.size(); ++t)
                        if (predictVal(1, mm, i, cands[t].first, cands[t].second) != q0) { diff = true; break; }
                    if (!diff) continue;
                    i64 w = Q(1, mm);
                    vector<pair<i64, i64>> nn;
                    for (auto &c : cands)
                        if (predictVal(1, mm, i, c.first, c.second) == w) nn.push_back(c);
                    if (!nn.empty() && nn.size() < cands.size()) { cands.swap(nn); progressed = true; }
                    break;
                }
                if (!progressed) break;
            }
            fL1 = cands.empty() ? 2 : cands[0].first;
            fL2 = cands.empty() ? 2 : cands[0].second;
        }
    }
    oi = i;
    oj = i + fL1;
    ok = i + fL1 + fL2 - 1;
}

#ifndef GUESS_TEST
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    i64 n;
    if (!(cin >> n)) return 0;
    i64 i, j, k;
    solveAlgo(n, [](i64 l, i64 r) -> i64 {
        cout << "? " << l << " " << r << endl;
        i64 x;
        cin >> x;
        if (!cin) exit(0);
        return x;
    }, i, j, k);
    cout << "! " << i << " " << j << " " << k << endl;
    return 0;
}
#endif
