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
    vector<vector<int64>> b(n, vector<int64>(m));
    vector<int64> rankCount(m, 0);
    for (int s = 0; s < n; ++s) {
        for (int r = 0; r < m; ++r) {
            cin >> a[s][r];
            rankCount[r] += a[s][r];
        }
    }
    for (auto &row : b)
        for (int64 &x : row) cin >> x;

    const int size = 1 << m;
    vector<int64> balance(size), deficit(size), cost(size, 0);
    vector<unsigned char> impossible(size, 0);

    // A set S is feasible if every attacker card with rank in S can be
    // matched to a distinct beating defender card whose rank also lies in S.
    // Such a matching lets the defender keep every played rank inside S.
    for (int s = 0; s < n; ++s) {
        int64 delta[18];
        for (int r = 0; r < m; ++r) delta[r] = a[s][r] - b[s][r];

        // Remove the lowest rank r, leaving only higher ranks T.
        // balance[S] = number of attacks minus defenses of this suit in S.
        // deficit[S] = minimum unmatched attacks using this suit alone.
        // Adding rank r gives deficit[S] = max(deficit[T], A[r]+balance[T]).
        if (trump[s]) {
            for (int mask = 1; mask < size; ++mask) {
                const int r = __builtin_ctz((unsigned)mask);
                const int rest = mask & (mask - 1);
                balance[mask] = balance[rest] + delta[r];
                deficit[mask] = max(deficit[rest], balance[rest] + a[s][r]);
                if (deficit[mask] > 0) impossible[mask] = 1;
                // Once trump attacks are matched, all remaining trump
                // defenses can beat any regular attack, regardless of rank.
                cost[mask] += balance[mask];
            }
        } else {
            for (int mask = 1; mask < size; ++mask) {
                const int r = __builtin_ctz((unsigned)mask);
                const int rest = mask & (mask - 1);
                balance[mask] = balance[rest] + delta[r];
                deficit[mask] = max(deficit[rest], balance[rest] + a[s][r]);
                cost[mask] += deficit[mask];
            }
        }
    }

    // Every starting rank in a feasible set loses. Conversely, if play ends
    // with the attacker losing, all ranks played form a feasible set: all
    // attacker cards of those ranks were played and successfully defended.
    int losingRanks = 0;
    for (int mask = 1; mask < size; ++mask) {
        if (!impossible[mask] && cost[mask] <= 0)
            losingRanks |= mask;
    }
    int64 answer = 0;
    for (int r = 0; r < m; ++r)
        if (!(losingRanks & (1 << r))) answer += rankCount[r];
    cout << answer << '\n';
    return 0;
}
