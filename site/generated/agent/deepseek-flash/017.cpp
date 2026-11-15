#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;

    vector<int> tr(n);
    for (int i = 0; i < n; ++i) cin >> tr[i];

    vector<vector<long long>> a(n, vector<long long>(m));
    vector<vector<long long>> d(n, vector<long long>(m));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j) cin >> a[i][j];
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j) cin >> d[i][j];

    long long perRank[18] = {};
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j) perRank[j] += a[i][j];

    const int N = 1 << m;
    vector<long long> sumDefReg(N, 0); // sum of regular-suit local deficiencies
    vector<long long> Atrump(N, 0);
    vector<long long> Dtrump(N, 0);
    vector<unsigned char> trumpFail(N, 0);

    vector<long long> AA(N), DD(N), df(N);

    for (int i = 0; i < n; ++i) {
        const auto &ai = a[i];
        const auto &di = d[i];
        if (tr[i]) {
            for (int mask = 1; mask < N; ++mask) {
                int r = __builtin_ctz((unsigned)mask);
                int pm = mask & (mask - 1);
                long long A = AA[pm] + ai[r];
                long long D = DD[pm] + di[r];
                AA[mask] = A;
                DD[mask] = D;
                long long v = A - D + di[r];
                long long old = df[pm];
                df[mask] = (old > v ? old : v);

                if (df[mask] > 0) trumpFail[mask] = 1;
                Atrump[mask] += A;
                Dtrump[mask] += D;
            }
        } else {
            for (int mask = 1; mask < N; ++mask) {
                int r = __builtin_ctz((unsigned)mask);
                int pm = mask & (mask - 1);
                long long A = AA[pm] + ai[r];
                long long D = DD[pm] + di[r];
                AA[mask] = A;
                DD[mask] = D;
                long long v = A - D + di[r];
                long long old = df[pm];
                df[mask] = (old > v ? old : v);

                sumDefReg[mask] += df[mask];
            }
        }
    }

    vector<char> defenderWinsRank(m, 0);
    for (int mask = 1; mask < N; ++mask) {
        if (!trumpFail[mask] &&
            sumDefReg[mask] + Atrump[mask] <= Dtrump[mask]) {
            int x = mask;
            while (x) {
                int r = __builtin_ctz((unsigned)x);
                defenderWinsRank[r] = 1;
                x &= x - 1;
            }
        }
    }

    long long answer = 0;
    for (int r = 0; r < m; ++r)
        if (!defenderWinsRank[r]) answer += perRank[r];

    cout << answer << '\n';
    return 0;
}
