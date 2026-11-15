#include <bits/stdc++.h>
using namespace std;

/*
  Interactive solution for "Radio Direction Finding".

  Facts used (n is odd, k = (n-1)/2):
    f(x) = distance(x,a) + distance(x,b).
    * The set where f attains its minimum m is the short arc [A,A+m]
      (m+1 points); the set where f attains its maximum n-m is a
      contiguous arc of m points.
    * f(A+t) = m + 2*t (while below the maximum arc), and values on
      the two monotone flanks are strictly monotone.
    * Consequence: every point at distance D from a query has a
      predictable/limited set of values, which lets us filter all
      possible plateau starts symbolically.

  After one query we hypothesise that the queried point lies on the
  minimum plateau (if f <= (n-1)/2) or on the maximum plateau
  (otherwise).  Under this hypothesis the candidate plateau starts form
  an interval.  A probe at distance m/2 halves it, and exact value
  filtering cuts it again in the following steps.  If the hypothesis is
  false, the filter quickly becomes empty (a point on a monotone flank
  can share the hypothesised value only at two symmetric positions),
  and then a direct three-query reconstruction solves the problem.
  One extra neighbour query verifies the plateau hypothesis whenever
  enough budget is left.
*/

struct Solver {
    long long n;
    int qcount = 0;
    unordered_map<long long,long long> cache;
    function<long long(long long)> rawQuery; // x is 0-based

    struct Cand {
        bool isPts = false;
        long long lo = 0, hi = -1;
        long long pt[4];
        int pc = 0;
        bool empty() const { return isPts ? (pc == 0) : (lo > hi); }
        long long size() const { return isPts ? (long long)pc : (hi - lo + 1); }
        void clear() { isPts = true; pc = 0; }
    };

    long long ask(long long x) {
        long long xm = x % n; if (xm < 0) xm += n;
        auto it = cache.find(xm);
        if (it != cache.end()) return it->second;
        long long r = rawQuery(xm);
        ++qcount;
        cache[xm] = r;
        return r;
    }

    static long long dsum(long long x, long long a, long long b, long long n) {
        x %= n; if (x < 0) x += n;
        a %= n; if (a < 0) a += n;
        b %= n; if (b < 0) b += n;
        long long d1 = llabs(x - a); d1 = min(d1, n - d1);
        long long d2 = llabs(x - b); d2 = min(d2, n - d2);
        return d1 + d2;
    }

    void getPair(bool ismin, long long m, long long S, long long &A, long long &B) {
        if (ismin) { A = S; B = S + m; }
        else {
            long long k = (n - 1) / 2;
            long long R = S + m - 1;
            B = R - k;
            A = B - m;
        }
    }

    void intersectInterval(Cand &c, long long a, long long b) {
        if (c.isPts) {
            long long tmp[4]; int nc = 0;
            for (int i = 0; i < c.pc; ++i) {
                long long p = c.pt[i];
                bool ok = false;
                for (int t = -1; t <= 1; ++t) {
                    long long A = a + (long long)t * n, B = b + (long long)t * n;
                    if (A <= p && p <= B) { ok = true; break; }
                }
                if (ok) tmp[nc++] = p;
            }
            c.pc = nc;
            for (int i = 0; i < nc; ++i) c.pt[i] = tmp[i];
        } else {
            bool found = false; long long nl = 0, nh = -1;
            for (int t = -1; t <= 1; ++t) {
                long long A = a + (long long)t * n, B = b + (long long)t * n;
                long long l = max(c.lo, A), h = min(c.hi, B);
                if (l <= h) {
                    if (!found || l < nl) { nl = l; nh = h; found = true; }
                }
            }
            if (!found) { c.lo = 1; c.hi = 0; }
            else { c.lo = nl; c.hi = nh; }
        }
    }

    void intersectPoints(Cand &c, long long x1, bool h1, long long x2, bool h2) {
        long long res[2]; int rc = 0;
        if (h1) res[rc++] = ((x1 % n) + n) % n;
        if (h2) res[rc++] = ((x2 % n) + n) % n;
        if (c.isPts) {
            long long tmp[4]; int nc = 0;
            for (int i = 0; i < c.pc; ++i) {
                bool ok = false;
                for (int j = 0; j < rc; ++j) {
                    long long d = c.pt[i] % n - res[j];
                    d %= n; if (d < 0) d += n;
                    if (d == 0) { ok = true; break; }
                }
                if (ok) tmp[nc++] = c.pt[i];
            }
            c.pc = nc;
            for (int i = 0; i < nc; ++i) c.pt[i] = tmp[i];
        } else {
            long long tmp[4]; int nc = 0;
            for (int j = 0; j < rc; ++j) {
                for (int t = -1; t <= 1; ++t) {
                    long long x = res[j] + (long long)t * n;
                    if (c.lo <= x && x <= c.hi) {
                        bool dup = false;
                        for (int q = 0; q < nc; ++q) if (tmp[q] == x) dup = true;
                        if (!dup && nc < 4) tmp[nc++] = x;
                    }
                }
            }
            c.isPts = true; c.pc = nc;
            for (int i = 0; i < nc; ++i) c.pt[i] = tmp[i];
        }
    }

    // Minimum plateau [S,S+m]; observed value w at position y.
    void filterMin(Cand &c, long long y, long long w, long long m) {
        long long k = (n - 1) / 2;
        if (w == m) { intersectInterval(c, y - m, y); return; }
        if (w == n - m) { intersectInterval(c, y - k - m, y - k - 1); return; }
        if (w < m || w > n - m || ((w - m) & 1LL)) { c.clear(); return; }
        long long s = (w - m) / 2;
        if (s < 1) { c.clear(); return; }
        intersectPoints(c, y + s, true, y - s - m, true);
    }

    // Maximum plateau [S,S+m-1]; observed value w at position y.
    void filterMax(Cand &c, long long y, long long w, long long m) {
        long long k = (n - 1) / 2;
        long long vv = n - m;
        if (w == vv) { intersectInterval(c, y - m + 1, y); return; }
        if (w == m) { intersectInterval(c, y + k + 1 - m, y + k + 1); return; }
        if (w < m || w > vv) { c.clear(); return; }
        bool h1 = false, h2 = false;
        long long x1 = 0, x2 = 0;
        if (((w - m) & 1LL) == 0) {
            long long t = (w - m) / 2;
            if (1 <= t && t <= k - m) { x1 = y + k + 1 - m - t; h1 = true; }
        }
        if (((vv - w + 1) & 1LL) == 0) {
            long long u = (vv - w + 1) / 2;
            if (1 <= u && u <= k - m) { x2 = y - m + 1 - u; h2 = true; }
        }
        if (!h1 && !h2) { c.clear(); return; }
        intersectPoints(c, x1, h1, x2, h2);
    }

    long long chooseQuery(const Cand &c, bool ismin, long long m, long long p) {
        int cnt = c.pc;
        long long trials[64]; int tc = 0;
        auto add = [&](long long y) { if (tc < 64) trials[tc++] = y; };
        long long k = (n - 1) / 2;
        for (int i = 0; i < cnt; ++i) {
            long long A, B; getPair(ismin, m, c.pt[i], A, B);
            add(c.pt[i]); add(c.pt[i] + 1); add(c.pt[i] - 1);
            add(c.pt[i] + m); add(c.pt[i] - m);
            add(c.pt[i] + k + 1); add(c.pt[i] + k + m);
            add(A); add(B); add(A + 1); add(B + 1);
        }
        for (int i = 0; i < cnt; ++i)
            for (int j = i + 1; j < cnt; ++j) {
                add(c.pt[i] + (c.pt[j] - c.pt[i]) / 2);
                add(c.pt[i] - (c.pt[j] - c.pt[i]) / 2);
            }
        add(p); add(p + 1); add(p - 1); add(p + k); add(p - k);
        for (int idx = 0; idx < tc; ++idx) {
            long long y = trials[idx];
            long long va = -1, vb = -1;
            for (int i = 0; i < cnt; ++i) {
                long long A, B; getPair(ismin, m, c.pt[i], A, B);
                long long val = dsum(y, A, B, n);
                if (i == 0) va = val; else if (i == 1) vb = val;
            }
            if (cnt == 1 || va != vb) return y;
        }
        return c.pt[0];
    }

    pair<long long,long long> solve(long long p0 = 0) {
        long long p = p0 % n; if (p < 0) p += n;
        long long v = ask(p);
        bool ismin;
        long long m;
        if (v <= (n - 1) / 2) { ismin = true; m = v; }
        else { ismin = false; m = n - v; }

        Cand c;
        c.isPts = false;
        if (ismin) { c.lo = p - m; c.hi = p; }
        else { c.lo = p - m + 1; c.hi = p; }

        long long D = m / 2;
        if (D > 0) {
            long long q = ismin ? (p + D) : (p - D);
            long long w = ask(q);
            if (ismin) filterMin(c, q, w, m); else filterMax(c, q, w, m);
        }

        while (!c.empty() && c.size() > 1 && qcount < 30) {
            long long y = c.isPts ? chooseQuery(c, ismin, m, p)
                                  : c.lo + (c.hi - c.lo) / 2;
            long long w = ask(y);
            if (ismin) filterMin(c, y, w, m); else filterMax(c, y, w, m);
        }

        bool hFalse = c.empty();
        long long S = 0;
        if (!hFalse) S = c.isPts ? c.pt[0] : c.lo;

        // Only a correctly hypothesised plateau can survive many
        // queries; if little budget was spent, verify cheaply.
        if (!hFalse && qcount <= 20) {
            long long send = ismin ? (S + m) : (S + m - 1);
            if (ismin || m >= 2) {
                long long nb = (p < send) ? (p + 1) : (p - 1);
                long long w = ask(nb);
                if (w != v) hFalse = true;
            }
        }

        if (hFalse) return solveGeneric(p, v);

        long long A, B;
        getPair(ismin, m, S, A, B);
        A %= n; if (A < 0) A += n;
        B %= n; if (B < 0) B += n;
        return {A, B};
    }

    // p is on a monotone flank: f(p) = m + 2*s.
    pair<long long,long long> solveGeneric(long long p, long long v) {
        long long f1 = ask(p + 1);
        int sign;
        if (f1 < v) sign = 1;
        else {
            long long fm = ask(p - 1);
            if (fm < v) sign = -1;
            else sign = 1; // cannot happen for a valid input
        }
        long long d0 = v / 2;
        long long y = p + sign * d0;
        long long mm = ask(y);
        long long s = (v - mm) / 2;
        long long b = p + sign * s;
        long long a = b + sign * mm;
        a %= n; if (a < 0) a += n;
        b %= n; if (b < 0) b += n;
        return {a, b};
    }
};

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    while (T--) {
        long long n;
        if (scanf("%lld", &n) != 1) return 0;
        Solver S;
        S.n = n;
        S.rawQuery = [](long long x) -> long long {
            printf("? %lld\n", x + 1);
            fflush(stdout);
            long long d;
            if (scanf("%lld", &d) != 1) exit(0);
            return d;
        };
        auto res = S.solve(0);
        printf("! %lld %lld\n", res.first + 1, res.second + 1);
        fflush(stdout);
    }
    return 0;
}
