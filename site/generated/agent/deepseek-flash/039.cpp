// Problem: count subsets A of S_n = Z_n with A +_n (S_n \ A) = Z_n,
// subject to membership constraints on m given positions.
//
// Key observations
// ----------------
// 1) g is NOT representable as a + b with a in A, b in Z_n\A  <=>
//       for all a in A: g - a in A  <=>  g - A = A
//    (the last equivalence also holds for A empty / A = Z_n).
//    Hence  A +_n A^c = Z_n  <=>  no "reflection" sigma_g(x) = g - x fixes A.
//
// 2) Inclusion-exclusion over the set of reflections {sigma_g : g in Z_n}:
//       answer = sum_{T subset Z_n} (-1)^{|T|} #(compatible A fixed by every sigma_g, g in T)
//    Grouping T by the subgroup D(T) of the dihedral group it generates gives
//       answer = sum_{H dihedral} c(H) * F(H),
//    where F(H) = #compatible A invariant under H and
//       sum_{H' <= H} c(H') = sum_{T : D(T) <= H} (-1)^{|T|} = 0  (H dihedral),
//    with c(trivial) = 1.  Solving this recursion on the subgroup lattice:
//       c_k = -prod_{p | k} (1 - p)   for a dihedral group whose rotation
//    subgroup has order k | n  (the value only depends on k).
//
// 3) Dihedral subgroups of order 2k: rotation subgroup K = <n/k> plus a
//    reflection coset c + K; there are exactly N = n/k of them, indexed by
//    j = c mod N.  A set invariant under such an H is a union of full cosets of
//    K, so it corresponds to B subset Z_N symmetric under i -> j - i, subject
//    to constraints projected modulo N.  The number of such B is
//        2^{(N + f_j)/2 - r + pairs_j}
//    where (N + f_j)/2 is the number of orbits of i -> j - i on Z_N
//    (f_j = #{i : 2i = j mod N}), r = #projected constrained indices and
//    pairs_j = #2-element orbits fully inside the constrained index set
//    (computable for all j at once by a cyclic convolution, i.e. an NTT).
//    A projection conflict (two constrained positions in the same coset asking
//    for different membership) makes the whole term vanish.
//
// Complexity: O(sum_{k | n} (n/k) log(n/k) + m * tau(n)) per test case.

#include <bits/stdc++.h>
using namespace std;
using ll = long long;

static const int MOD = 998244353;
static const int PRIMITIVE_ROOT = 3;

static inline int pw(int b, long long e, int m = MOD) {
    long long r = 1, x = b % m;
    while (e) {
        if (e & 1) r = r * x % m;
        x = x * x % m;
        e >>= 1;
    }
    return (int)r;
}

// ------------------------------- NTT ---------------------------------------
static vector<vector<int>> RF, RI;  // twiddle tables per level

static void init_roots(int maxlog) {
    RF.assign(maxlog + 1, {});
    RI.assign(maxlog + 1, {});
    for (int L = 1; L <= maxlog; L++) {
        int half = 1 << (L - 1);
        int w = pw(PRIMITIVE_ROOT, (MOD - 1) >> L);
        int wi = pw(w, MOD - 2);
        RF[L].resize(half);
        RI[L].resize(half);
        RF[L][0] = 1;
        RI[L][0] = 1;
        for (int j = 1; j < half; j++) {
            RF[L][j] = (int)((ll)RF[L][j - 1] * w % MOD);
            RI[L][j] = (int)((ll)RI[L][j - 1] * wi % MOD);
        }
    }
}

static void ntt(vector<int>& a, bool inv) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    int logn = 0;
    while ((1 << logn) < n) logn++;
    for (int L = 1; L <= logn; L++) {
        int len = 1 << L, half = len >> 1;
        const vector<int>& tw = inv ? RI[L] : RF[L];
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; j++) {
                int u = a[i + j];
                int v = (int)((ll)a[i + j + half] * tw[j] % MOD);
                int x = u + v;
                if (x >= MOD) x -= MOD;
                int y = u - v;
                if (y < 0) y += MOD;
                a[i + j] = x;
                a[i + j + half] = y;
            }
        }
    }
    if (inv) {
        int ninv = pw(n, MOD - 2);
        for (int& x : a) x = (int)((ll)x * ninv % MOD);
    }
}

// ------------------------------ helpers ------------------------------------
static const int MAXN = 200005;
static int spf[MAXN];

static void init_spf() {
    for (int i = 0; i < MAXN; i++) spf[i] = i;
    for (int i = 2; (ll)i * i < MAXN; i++)
        if (spf[i] == i)
            for (int j = i * i; j < MAXN; j += i)
                if (spf[j] == j) spf[j] = i;
}

// c_k = -prod_{p | k} (1 - p)  (mod MOD)
static int coeff(int k) {
    ll res = 1;
    int x = k;
    while (x > 1) {
        int p = spf[x];
        res = res * ((1 - p) % MOD) % MOD;
        while (x % p == 0) x /= p;
    }
    res = (MOD - res) % MOD;
    return (int)res;
}

int main() {
    init_spf();
    init_roots(20);

    int T;
    if (scanf("%d", &T) != 1) return 0;

    vector<int> pow2, xs, ts;
    while (T--) {
        int n, m;
        if (scanf("%d %d", &n, &m) != 2) return 0;
        xs.resize(m);
        ts.resize(m);
        for (int i = 0; i < m; i++)
            if (scanf("%d %d", &xs[i], &ts[i]) != 2) return 0;

        pow2.assign(n + 2, 1);
        for (int i = 1; i <= n + 1; i++) pow2[i] = (int)((ll)pow2[i - 1] * 2 % MOD);

        int ans = pow2[n - m];  // all sets compatible with the constraints

        vector<int> divs;
        for (int d = 1; (ll)d * d <= n; d++) {
            if (n % d == 0) {
                divs.push_back(d);
                if (d != n / d) divs.push_back(n / d);
            }
        }
        sort(divs.begin(), divs.end());

        vector<int> P, Q, R, cR, cPQ;

        for (int k : divs) {          // k = order of the rotation subgroup
            int N = n / k;            // number of cosets / of subgroups
            P.assign(N, 0);           // projected indices forced into A
            Q.assign(N, 0);           // projected indices forced out of A
            bool conflict = false;
            for (int idx = 0; idx < m; idx++) {
                int i = xs[idx] % N;
                if (ts[idx] == 1) {
                    if (Q[i]) conflict = true;
                    P[i] = 1;
                } else {
                    if (P[i]) conflict = true;
                    Q[i] = 1;
                }
            }
            if (conflict) continue;   // no invariant set can exist

            R.assign(N, 0);
            int r = 0;
            for (int i = 0; i < N; i++) {
                R[i] = P[i] | Q[i];
                r += R[i];
            }

            cR.assign(N, 0);          // #(ordered pairs of R summing to j mod N)
            cPQ.assign(N, 0);         // #(pairs (i in P, i' in Q) with i+i' = j)

            if (2 * N - 1 <= 64) {
                for (int i = 0; i < N; i++) {
                    if (!R[i]) continue;
                    for (int j = 0; j < N; j++) {
                        if (!R[j]) continue;
                        int s = i + j;
                        if (s >= N) s -= N;
                        cR[s]++;
                    }
                }
                for (int i = 0; i < N; i++) {
                    if (!P[i]) continue;
                    for (int j = 0; j < N; j++) {
                        if (!Q[j]) continue;
                        int s = i + j;
                        if (s >= N) s -= N;
                        cPQ[s]++;
                    }
                }
            } else {
                int sz = 1;
                while (sz < 2 * N - 1) sz <<= 1;
                vector<int> fa(sz, 0), fb(sz, 0), fc(sz, 0);
                for (int i = 0; i < N; i++) {
                    fa[i] = P[i];
                    fb[i] = Q[i];
                }
                ntt(fa, false);
                ntt(fb, false);
                for (int i = 0; i < sz; i++) {
                    int s = fa[i] + fb[i];
                    if (s >= MOD) s -= MOD;                  // transform of R
                    fc[i] = (int)((ll)s * s % MOD);          // R * R
                    fa[i] = (int)((ll)fa[i] * fb[i] % MOD);  // P * Q
                }
                ntt(fc, true);
                ntt(fa, true);
                int lim = 2 * N - 1;
                for (int j = 0; j < N; j++) {
                    int a = fc[j], b = fa[j];
                    if (j + N < lim) {  // fold indices modulo N
                        a += fc[j + N];
                        if (a >= MOD) a -= MOD;
                        b += fa[j + N];
                        if (b >= MOD) b -= MOD;
                    }
                    cR[j] = a;
                    cPQ[j] = b;
                }
            }

            ll S = 0;
            for (int j = 0; j < N; j++) {
                if (cPQ[j] != 0) continue;  // mirrored constraints disagree
                int f, fR;                  // fixed points of i -> j-i (all / in R)
                if (N & 1) {
                    int i = ((j & 1) ? (j + N) : j) >> 1;  // unique 2i == j (mod N)
                    f = 1;
                    fR = R[i];
                } else if (j & 1) {
                    f = 0;
                    fR = 0;
                } else {
                    f = 2;
                    fR = R[j >> 1] + R[(j >> 1) + (N >> 1)];
                }
                int freeOrbits = (N + f) / 2 - r + (cR[j] - fR) / 2;
                S += pow2[freeOrbits];
            }
            S %= MOD;
            ans = (int)((ans + (ll)coeff(k) * S) % MOD);
        }
        printf("%d\n", ans);
    }
    return 0;
}
