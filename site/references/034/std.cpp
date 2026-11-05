#include <bits/stdc++.h>
using namespace std;

using LD = long double;
static const int MAXN = 20;

int n;
int w[MAXN][MAXN];
int idv[MAXN];
uint64_t adjMask[MAXN];
LD L[MAXN][MAXN];
LD invD[MAXN];
LD pvec[MAXN], qvec[MAXN], zvec[MAXN];
LD answer = 0;

inline bool feasible(int k) {
    LD y[MAXN];
    for (int i = k - 1; i >= 0; --i) {
        LD cur = qvec[i];
        for (int j = i + 1; j < k; ++j) cur -= L[j][i] * y[j];
        // Mathematically y_i >= 0 is required. A tiny tolerance only absorbs
        // roundoff near a lower-dimensional optimum.
        if (cur < -1e-18L) return false;
        y[i] = cur;
    }
    return true;
}

void dfs(int k, uint64_t cand, LD t, LD invBase) {
    if (0.5L / t > answer) {
        if (feasible(k)) answer = 0.5L / t;
    }

    while (cand) {
        int v = __builtin_ctzll(cand);
        cand &= cand - 1;
        idv[k] = v;

        // z = L^{-1} b, where b_i = w[idv[i]][v].
        for (int i = 0; i < k; ++i) {
            LD cur = (LD)w[idv[i]][v];
            for (int j = 0; j < i; ++j) cur -= L[i][j] * zvec[j];
            zvec[i] = cur;
        }

        // The first D block is [[0,a],[a,0]].
        L[k][0] = zvec[1] * invBase;
        L[k][1] = zvec[0] * invBase;
        for (int i = 2; i < k; ++i) L[k][i] = zvec[i] * invD[i];

        LD s = 0;
        for (int i = 0; i < k; ++i) s -= zvec[i] * L[k][i];

        // Every prefix of a minimum-support optimum is strictly conditionally
        // negative definite, so each scalar pivot after the 2x2 base is < 0.
        if (!(s < 0)) continue;
        invD[k] = 1.0L / s;

        LD pk = 1.0L;
        for (int i = 0; i < k; ++i) pk -= L[k][i] * pvec[i];
        pvec[k] = pk;
        qvec[k] = pk * invD[k];

        LD nt = t + pk * qvec[k];
        if (!(nt > 0)) continue;

        uint64_t child = cand & adjMask[v];
        dfs(k + 1, child, nt, invBase);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            cin >> w[i][j];

    for (int i = 0; i < n; ++i) {
        uint64_t m = 0;
        for (int j = 0; j < n; ++j)
            if (i != j && w[i][j] > 0) m |= 1ULL << j;
        adjMask[i] = m;
    }

    // Supports of size 2.
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            answer = max(answer, (LD)w[i][j] * 0.25L);

    if (n >= 2) {
        const uint64_t ALL = (1ULL << n) - 1;
        for (int i = 0; i < n; ++i) {
            uint64_t higher = ALL & ~((1ULL << (i + 1)) - 1);
            uint64_t cand = adjMask[i] & higher;
            while (cand) {
                int j = __builtin_ctzll(cand);
                cand &= cand - 1;

                idv[0] = i;
                idv[1] = j;
                L[1][0] = 0;
                pvec[0] = pvec[1] = 1;

                LD invBase = 1.0L / (LD)w[i][j];
                qvec[0] = qvec[1] = invBase;
                LD t = 2.0L * invBase;

                uint64_t child = cand & adjMask[j];
                if (child) dfs(2, child, t, invBase);
            }
        }
    }

    cout << fixed << setprecision(12) << answer << '\n';
    return 0;
}
