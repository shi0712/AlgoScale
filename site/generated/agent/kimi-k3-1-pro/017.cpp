#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;
using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> trump(n);
    for (int &x : trump) cin >> x;

    vector<vector<int64>> attacker(n, vector<int64>(m));
    vector<vector<int64>> defender(n, vector<int64>(m));
    for (auto &row : attacker)
        for (int64 &x : row) cin >> x;
    for (auto &row : defender)
        for (int64 &x : row) cin >> x;

    const int size = 1 << m;
    vector<int64> regularDeficit(size, 0), trumpSpare(size, 0);
    vector<bool> trumpBad(size, false);
    vector<int64> a(size), d(size), deficit(size);

    for (int suit = 0; suit < n; ++suit) {
        a[0] = d[0] = deficit[0] = 0;
        for (int mask = 1; mask < size; ++mask) {
            int rank = __builtin_ctz(static_cast<unsigned>(mask));
            int previous = mask & (mask - 1);
            a[mask] = a[previous] + attacker[suit][rank];
            d[mask] = d[previous] + defender[suit][rank];
            deficit[mask] = max({int64(0), deficit[previous],
                                 a[mask] - d[previous]});

            if (trump[suit]) {
                trumpSpare[mask] += d[mask] - a[mask];
                if (deficit[mask] > 0) trumpBad[mask] = true;
            } else {
                regularDeficit[mask] += deficit[mask];
            }
        }
    }

    vector<bool> winningRank(m, false);
    for (int mask = 1; mask < size; ++mask) {
        if (trumpBad[mask] || regularDeficit[mask] > trumpSpare[mask])
            continue;
        for (int rank = 0; rank < m; ++rank)
            if (mask & (1 << rank)) winningRank[rank] = true;
    }

    int64 answer = 0;
    for (int suit = 0; suit < n; ++suit)
        for (int rank = 0; rank < m; ++rank)
            if (!winningRank[rank]) answer += attacker[suit][rank];

    cout << answer << '\n';
    return 0;
}
