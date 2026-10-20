#include <bits/stdc++.h>
using namespace std;

struct EvalRes {
    long long F;
    long long Smin;
    long long Smax;
    long long Hmin;
};

int n;
long long P, Q;
int Hmax;
int Rmax;
int PMAX, QMAX;

vector<int> cnt;
vector<int> Bcost, Bmin;
vector<long long> prefCnt, prefCost, prefHMin;

static inline void calcHalf(int y, int q, int &bmin, int &cost) {
    if (y == 0) {
        bmin = 0;
        cost = 0;
        return;
    }
    if (q == 0) {
        bmin = 32 - __builtin_clz((unsigned)y); // minimum halves to reach 0
        cost = 0;
        return;
    }

    if (y <= q) {
        bmin = 0;
    } else {
        int x = (y - 1) / q;
        bmin = 31 - __builtin_clz((unsigned)x); // floor(log2(x))
    }

    int val = y >> bmin;
    cost = val + q * bmin;
}

static inline int isqrtSmall(int x) {
    int r = (int)sqrt((double)x);
    while ((r + 1) * (r + 1) <= x) ++r;
    while (r * r > x) --r;
    return r;
}

void buildQ(int q) {
    Bcost.resize(Hmax + 1);
    Bmin.resize(Hmax + 1);

    for (int h = 0; h <= Hmax; ++h) {
        int bmin, cost;
        calcHalf(h, q, bmin, cost);
        Bcost[h] = cost;
        Bmin[h] = bmin;
    }

    prefCnt.assign(Hmax + 2, 0);
    prefCost.assign(Hmax + 2, 0);
    prefHMin.assign(Hmax + 2, 0);

    for (int h = 0; h <= Hmax; ++h) {
        long long c = cnt[h];
        prefCnt[h + 1] = prefCnt[h] + c;
        prefCost[h + 1] = prefCost[h] + c * Bcost[h];
        prefHMin[h + 1] = prefHMin[h] + c * Bmin[h];
    }
}

EvalRes evalPenalty(int p, int q) {
    long long F = 0;
    long long Smin = 0, Smax = 0, Hmin = 0;

    for (int r = 0;; ++r) {
        int L = r * r;
        if (L > Hmax) break;
        int RR = (r + 1) * (r + 1) - 1;
        if (RR > Hmax) RR = Hmax;

        // Option: perform at least one sqrt operation.
        // First sqrt takes h -> r, then we may perform more sqrts and halves.
        long long bestCost = LLONG_MAX;
        int bestSMin = INT_MAX;
        int bestSMax = -1;
        int bestH = INT_MAX;

        int val = r;
        for (int a = 0; a <= 5; ++a) {
            int b, halfCost;
            calcHalf(val, q, b, halfCost);

            long long curCost = 1LL * p * a + halfCost;

            if (curCost < bestCost) {
                bestCost = curCost;
                bestSMin = a;
                bestSMax = a;
                bestH = b;
            } else if (curCost == bestCost) {
                bestSMin = min(bestSMin, a);
                bestSMax = max(bestSMax, a);
                bestH = min(bestH, b);
            }

            int nv = isqrtSmall(val);
            if (a >= 1 && nv == val) break;
            val = nv;
        }

        long long C = 1LL * p + bestCost;

        int sqrtSMin = 1 + bestSMin;
        int sqrtSMax = 1 + bestSMax;
        int sqrtH = bestH;

        // first index with Bcost >= C
        auto first_ge = [&](int l, int r, long long v) {
            int lo = l, hi = r + 1;
            while (lo < hi) {
                int mid = (lo + hi) >> 1;
                if ((long long)Bcost[mid] < v) lo = mid + 1;
                else hi = mid;
            }
            return lo;
        };

        // first index with Bcost > C
        auto first_gt = [&](int l, int r, long long v) {
            int lo = l, hi = r + 1;
            while (lo < hi) {
                int mid = (lo + hi) >> 1;
                if ((long long)Bcost[mid] <= v) lo = mid + 1;
                else hi = mid;
            }
            return lo;
        };

        int lastLT = first_ge(L, RR, C) - 1;
        int lastLE = first_gt(L, RR, C) - 1;

        // Strictly no sqrt: Bcost < C
        if (L <= lastLT) {
            F += prefCost[lastLT + 1] - prefCost[L];
            Hmin += prefHMin[lastLT + 1] - prefHMin[L];
        }

        // Tie: Bcost == C
        int tieL = max(L, lastLT + 1);
        int tieR = lastLE;
        if (tieL <= tieR) {
            long long c = prefCnt[tieR + 1] - prefCnt[tieL];
            F += C * c;

            // min S chooses no sqrt, so nothing is added to Smin.
            // max S chooses sqrt, because sqrtSMax >= 1.
            Smax += 1LL * sqrtSMax * c;

            // min H is min(Bmin[h], sqrtH).
            auto first_bmin_gt = [&](int l, int r, int v) {
                int lo = l, hi = r + 1;
                while (lo < hi) {
                    int mid = (lo + hi) >> 1;
                    if (Bmin[mid] <= v) lo = mid + 1;
                    else hi = mid;
                }
                return lo;
            };

            int pos = first_bmin_gt(tieL, tieR, sqrtH) - 1;
            if (pos >= tieL) {
                Hmin += prefHMin[pos + 1] - prefHMin[tieL];
                Hmin += 1LL * sqrtH * (prefCnt[tieR + 1] - prefCnt[pos + 1]);
            } else {
                Hmin += 1LL * sqrtH * c;
            }
        }

        // Strictly sqrt: Bcost > C
        int sqL = max(L, lastLE + 1);
        if (sqL <= RR) {
            long long c = prefCnt[RR + 1] - prefCnt[sqL];
            F += C * c;
            Smin += 1LL * sqrtSMin * c;
            Smax += 1LL * sqrtSMax * c;
            Hmin += 1LL * sqrtH * c;
        }
    }

    // At penalty p = 0, extra no-effect sqrt operations are free.
    // For the purpose of finding the right endpoint of the subgradient,
    // treat the maximum possible sqrt count as infinite.
    if (p == 0) Smax = (long long)4e18;

    return {F, Smin, Smax, Hmin};
}

pair<long long, long long> solveForQ(int q) {
    buildQ(q);

    // p_left: smallest p with minS(p, q) <= P
    int p_left;
    EvalRes e0 = evalPenalty(0, q);

    if (e0.Smin <= P) {
        p_left = 0;
    } else {
        int lo = 0, hi = PMAX;
        while (hi - lo > 1) {
            int mid = (lo + hi) >> 1;
            EvalRes cur = evalPenalty(mid, q);
            if (cur.Smin <= P) hi = mid;
            else lo = mid;
        }
        p_left = hi;
    }

    // p_right: smallest p with maxS(p, q) < P
    int p_right;
    if (P == 0) {
        p_right = PMAX + 1;
    } else {
        EvalRes cur0 = evalPenalty(0, q);
        if (cur0.Smax < P) {
            p_right = 0;
        } else {
            int lo = 0, hi = PMAX;
            while (hi - lo > 1) {
                int mid = (lo + hi) >> 1;
                EvalRes cur = evalPenalty(mid, q);
                if (cur.Smax < P) hi = mid;
                else lo = mid;
            }
            p_right = hi;
        }
    }

    int last_max = (p_right == 0 ? -1 : p_right - 1);

    int cand[4], cc = 0;
    auto add = [&](int x) {
        if (x < 0 || x > PMAX) return;
        for (int i = 0; i < cc; ++i) if (cand[i] == x) return;
        cand[cc++] = x;
    };

    add(p_left);
    add(p_left - 1);
    add(last_max);

    long long bestD = LLONG_MIN;
    long long minH = LLONG_MAX;

    for (int i = 0; i < cc; ++i) {
        int p = cand[i];
        EvalRes cur = evalPenalty(p, q);
        long long D = cur.F - 1LL * p * P - 1LL * q * Q;

        if (D > bestD) {
            bestD = D;
            minH = cur.Hmin;
        } else if (D == bestD) {
            minH = min(minH, cur.Hmin);
        }
    }

    return {bestD, minH};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> P >> Q;

    vector<int> h(n);
    Hmax = 0;
    for (int i = 0; i < n; ++i) {
        cin >> h[i];
        Hmax = max(Hmax, h[i]);
    }

    cnt.assign(Hmax + 1, 0);
    for (int x : h) ++cnt[x];

    Rmax = isqrtSmall(Hmax);
    PMAX = Hmax + 5;
    QMAX = Hmax + 5;

    // q_left: smallest q with minH(q) <= Q
    int q_left;
    {
        auto res0 = solveForQ(0);
        if (res0.second <= Q) {
            q_left = 0;
        } else {
            int lo = 0, hi = QMAX;
            while (hi - lo > 1) {
                int mid = (lo + hi) >> 1;
                auto res = solveForQ(mid);
                if (res.second <= Q) hi = mid;
                else lo = mid;
            }
            q_left = hi;
        }
    }

    long long ans = solveForQ(q_left).first;
    if (q_left > 0) {
        ans = max(ans, solveForQ(q_left - 1).first);
    }

    cout << ans << '\n';
    return 0;
}
