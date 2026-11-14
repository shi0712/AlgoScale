#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> trump(n);
    for (int &x : trump) cin >> x;

    vector<vector<int64>> a(n, vector<int64>(m));
    vector<vector<int64>> d(n, vector<int64>(m));

    for (int s = 0; s < n; ++s)
        for (int r = 0; r < m; ++r)
            cin >> a[s][r];

    for (int s = 0; s < n; ++s)
        for (int r = 0; r < m; ++r)
            cin >> d[s][r];

    const int S = 1 << m;

    vector<int64> attackerByRank(m, 0);
    vector<int64> trumpAByRank(m, 0), trumpDByRank(m, 0);

    for (int s = 0; s < n; ++s) {
        for (int r = 0; r < m; ++r) {
            attackerByRank[r] += a[s][r];
            if (trump[s]) {
                trumpAByRank[r] += a[s][r];
                trumpDByRank[r] += d[s][r];
            }
        }
    }

    // Highest set bit for every mask.
    vector<unsigned char> highest(S, 0);
    for (int mask = 2; mask < S; ++mask)
        highest[mask] = highest[mask >> 1] + 1;

    vector<int64> trumpA(S, 0), trumpD(S, 0);
    for (int mask = 1; mask < S; ++mask) {
        int h = highest[mask];
        int rest = mask ^ (1 << h);
        trumpA[mask] = trumpA[rest] + trumpAByRank[h];
        trumpD[mask] = trumpD[rest] + trumpDByRank[h];
    }

    vector<int64> deficit(S, 0);
    vector<unsigned char> bad(S, 0);
    vector<int64> need(S, 0);

    for (int s = 0; s < n; ++s) {
        need[0] = 0;

        for (int mask = 1; mask < S; ++mask) {
            int h = highest[mask];
            int rest = mask ^ (1 << h);

            int64 value = max(
                a[s][h],
                need[rest] + a[s][h] - d[s][h]
            );
            need[mask] = value;

            if (trump[s]) {
                if (value > 0) bad[mask] = 1;
            } else {
                deficit[mask] += value;
            }
        }
    }

    vector<unsigned char> defenderWinsRank(m, 0);

    for (int mask = 1; mask < S; ++mask) {
        if (bad[mask]) continue;
        if (trumpA[mask] + deficit[mask] > trumpD[mask]) continue;

        int bits = mask;
        while (bits) {
            int r = __builtin_ctz(static_cast<unsigned>(bits));
            defenderWinsRank[r] = 1;
            bits &= bits - 1;
        }
    }

    int64 answer = 0;
    for (int r = 0; r < m; ++r) {
        if (!defenderWinsRank[r])
            answer += attackerByRank[r];
    }

    cout << answer << '\n';
    return 0;
}
