#include <bits/stdc++.h>
using namespace std;
using ull = unsigned long long;
using i128 = __int128_t;

static const ull CAP = 1000000000000000001ULL; // 1e18 + 1 > max k

static inline ull mulc(ull a, ull b) {
    if (a == 0 || b == 0) return 0;
    i128 r = (i128)a * (i128)b;
    return r > (i128)CAP ? CAP : (ull)r;
}
static inline ull addc(ull a, ull b) {
    ull s = a + b;
    return s > CAP ? CAP : s;
}

int main() {
    int n;
    ull k;
    if (scanf("%d %llu", &n, &k) != 2) return 0;

    // ---------- binomial coefficients (capped) ----------
    vector<vector<ull>> C(n + 1, vector<ull>(n + 1, 0));
    for (int S = 0; S <= n; S++) {
        C[S][0] = 1;
        for (int m = 1; m <= S; m++) {
            ull v = C[S - 1][m - 1];
            if (m <= S - 1) v = addc(v, C[S - 1][m]);
            C[S][m] = v;
        }
    }

    // ---------- Euler zigzag numbers A[m]: #alternating perms of length m
    // with a fixed starting direction (both directions have equal counts).
    // U[i][j]: pattern starts with ascent, first element has rank j among i.
    // D[i][j]: pattern starts with descent,  first element has rank j.
    vector<ull> A(n + 1, 0);
    vector<vector<ull>> U(n + 1, vector<ull>(n + 2, 0));
    vector<vector<ull>> D(n + 1, vector<ull>(n + 2, 0));
    A[0] = 1;
    if (n >= 1) {
        U[1][1] = 1;
        D[1][1] = 1;
        A[1] = 1;
    }
    for (int i = 2; i <= n; i++) {
        // U[i][j] = sum_{t=j}^{i-1} D[i-1][t]
        for (int j = i - 1; j >= 1; j--) U[i][j] = addc(U[i][j + 1], D[i - 1][j]);
        // D[i][j] = sum_{t=1}^{j-1} U[i-1][t]
        for (int j = 1; j <= i; j++) D[i][j] = addc(D[i][j - 1], U[i - 1][j - 1]);
        ull s = 0;
        for (int j = 1; j <= i; j++) s = addc(s, U[i][j]);
        A[i] = s;
    }

    auto w = [&](int S, int m) -> ull {
        // min( C(S,m) * A[m], CAP )
        if (m == 0) return 1;
        if (m < 0 || S < m) return 0;
        return mulc(C[S][m], A[m]);
    };

    // total number of valid permutations
    ull total0 = addc(A[n], A[n]);
    if (k > total0) {
        printf("-1\n");
        return 0;
    }

    vector<int> T(n + 1, 0); // T[i] = placement time (0 => free/unplaced)
    vector<int> q(n + 1, 0); // q[index] = value placed there
    bool validA = true, validB = true;

    vector<int> istart(n + 1), iend(n + 1), isz(n + 1), iidOf(n + 1);
    vector<ull> S(n + 1), pref(n + 2), suf1(n + 2);

    bool done = false;
    for (int j = 1; j <= n; j++) {
        // ---- build maximal free intervals ----
        int r = 0;
        for (int i = 1; i <= n;) {
            if (T[i] != 0) { i++; continue; }
            int a = i;
            while (i <= n && T[i] == 0) i++;
            int b = i - 1;
            istart[r] = a; iend[r] = b; isz[r] = b - a + 1;
            for (int t = a; t <= b; t++) iidOf[t] = r;
            r++;
        }
        // prefix sums of interval sizes
        {
            int s = 0;
            for (int t = 0; t < r; t++) { s += isz[t]; S[t] = (ull)s; }
        }
        pref[0] = 1;
        for (int t = 0; t < r; t++) pref[t + 1] = mulc(pref[t], w((int)S[t], isz[t]));
        suf1[r] = 1;
        for (int t = r - 1; t >= 0; t--) suf1[t] = mulc(suf1[t + 1], w((int)S[t] - 1, isz[t]));

        // ---- try candidate indices in increasing order ----
        int chosen = -1;
        for (int x = 1; x <= n; x++) {
            if (T[x] != 0) continue;

            bool lEx = (x > 1), rEx = (x < n);
            bool lP = lEx && T[x - 1] != 0, lF = lEx && T[x - 1] == 0;
            bool rP = rEx && T[x + 1] != 0, rF = rEx && T[x + 1] == 0;
            bool okA = validA, okB = validB;
            bool hasPlacedNbr = lP || rP;
            bool hasFreeNbr = lF || rF;
            int par = x & 1;
            // pattern A: asc at odd index, desc at even index
            if (hasPlacedNbr && par != 0) okA = false; // need x even
            if (hasFreeNbr && par != 1) okA = false;   // need x odd
            // pattern B: desc at odd index, asc at even index
            if (hasPlacedNbr && par != 1) okB = false; // need x odd
            if (hasFreeNbr && par != 0) okB = false;   // need x even
            if (!okA && !okB) continue;

            int u = iidOf[x];
            int a = istart[u], b = iend[u], M = isz[u];
            int before = (u == 0 ? 0 : (int)S[u - 1]); // free slots in earlier intervals
            ull local;
            if (M == 1) local = 1;
            else if (x == a || x == b) local = w((int)S[u] - 1, M - 1);
            else local = mulc(w(before + (x - a), x - a), w((int)S[u] - 1, b - x));

            ull base = mulc(mulc(pref[u], local), suf1[u + 1]);
            ull tot = base;
            if (okA && okB) tot = addc(base, base);

            if (k > tot) {
                k -= tot;
            } else {
                chosen = x;
                validA = okA;
                validB = okB;
                break;
            }
        }

        if (chosen < 0) { done = true; break; }
        T[chosen] = j;
        q[chosen] = j;
    }

    if (done) {
        printf("-1\n");
        return 0;
    }

    // invert: p[q[i]] = i
    vector<int> p(n + 1, 0);
    for (int i = 1; i <= n; i++) p[q[i]] = i;
    for (int i = 1; i <= n; i++) printf("%d%c", p[i], i == n ? '\n' : ' ');
    return 0;
}
