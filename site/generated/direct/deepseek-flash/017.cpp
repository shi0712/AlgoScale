#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> t(n);
    for (int i = 0; i < n; ++i) cin >> t[i];

    vector<vector<long long>> a(n, vector<long long>(m));
    vector<long long> totalRank(m, 0);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cin >> a[i][j];
            totalRank[j] += a[i][j];
        }
    }

    vector<vector<long long>> b(n, vector<long long>(m));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cin >> b[i][j];
        }
    }

    int M = 1 << m;
    vector<int> highest(M), lowerMask(M);

    for (int mask = 1; mask < M; ++mask) {
        int h = 31 - __builtin_clz((unsigned)mask);
        highest[mask] = h;
        lowerMask[mask] = mask ^ (1 << h);
    }

    vector<long long> F(M), W(M);
    vector<long long> needTrump(M, 0);
    vector<long long> haveTrump(M, 0);
    vector<unsigned char> bad(M, 0);

    long long Aarr[20], Barr[20];

    for (int s = 0; s < n; ++s) {
        for (int j = 0; j < m; ++j) {
            Aarr[j] = a[s][j];
            Barr[j] = b[s][j];
        }

        F[0] = 0;
        W[0] = 0;

        if (t[s] == 1) {
            for (int mask = 1; mask < M; ++mask) {
                int h = highest[mask];
                int low = lowerMask[mask];

                long long ah = Aarr[h];
                long long bh = Barr[h];

                long long f = ah;
                long long cand = ah - bh + F[low];
                if (cand > f) f = cand;
                F[mask] = f;

                long long w = W[low] + bh - ah;
                W[mask] = w;

                if (f > 0) bad[mask] = 1;
                haveTrump[mask] += w;
            }
        } else {
            for (int mask = 1; mask < M; ++mask) {
                int h = highest[mask];
                int low = lowerMask[mask];

                long long ah = Aarr[h];
                long long bh = Barr[h];

                long long f = ah;
                long long cand = ah - bh + F[low];
                if (cand > f) f = cand;
                F[mask] = f;

                needTrump[mask] += f;
            }
        }
    }

    vector<unsigned char> goodRank(m, 0);

    for (int mask = 1; mask < M; ++mask) {
        if (!bad[mask] && needTrump[mask] <= haveTrump[mask]) {
            int bits = mask;
            while (bits) {
                int bit = __builtin_ctz((unsigned)bits);
                goodRank[bit] = 1;
                bits &= bits - 1;
            }
        }
    }

    long long answer = 0;

    for (int r = 0; r < m; ++r) {
        if (!goodRank[r]) {
            answer += totalRank[r];
        }
    }

    cout << answer << '\n';
    return 0;
}
