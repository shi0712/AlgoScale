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

    vector<vector<int64>> attacker(n, vector<int64>(m));
    vector<vector<int64>> defender(n, vector<int64>(m));
    vector<int64> rankCount(m, 0);

    for (int i = 0; i < n; ++i) {
        for (int r = 0; r < m; ++r) {
            cin >> attacker[i][r];
            rankCount[r] += attacker[i][r];
        }
    }

    for (int i = 0; i < n; ++i) {
        for (int r = 0; r < m; ++r) {
            cin >> defender[i][r];
        }
    }

    // For each rank: attacker trump cards minus defender trump cards.
    vector<int64> trumpBalance(m, 0);
    for (int i = 0; i < n; ++i) {
        if (trump[i]) {
            for (int r = 0; r < m; ++r) {
                trumpBalance[r] += attacker[i][r] - defender[i][r];
            }
        }
    }

    const int states = 1 << m;

    // Eventually:
    // need[S] = unmatched regular attackers - surplus trump defenders.
    vector<int64> need(states, 0);
    for (int r = 0; r < m; ++r) {
        int bit = 1 << r;
        for (int mask = 0; mask < bit; ++mask) {
            need[bit | mask] = need[mask] + trumpBalance[r];
        }
    }

    vector<unsigned char> forbidden(states, 0);
    vector<int64> unmatched(states, 0);

    for (int i = 0; i < n; ++i) {
        unmatched[0] = 0;

        if (trump[i]) {
            for (int r = 0; r < m; ++r) {
                const int bit = 1 << r;
                const int64 a = attacker[i][r];
                const int64 d = defender[i][r];

                for (int mask = 0; mask < bit; ++mask) {
                    int64 remaining = unmatched[mask] - d;
                    int64 value = a + max<int64>(remaining, 0);

                    unmatched[bit | mask] = value;
                    if (value != 0) forbidden[bit | mask] = 1;
                }
            }
        } else {
            for (int r = 0; r < m; ++r) {
                const int bit = 1 << r;
                const int64 a = attacker[i][r];
                const int64 d = defender[i][r];

                for (int mask = 0; mask < bit; ++mask) {
                    int64 remaining = unmatched[mask] - d;
                    int64 value = a + max<int64>(remaining, 0);

                    unmatched[bit | mask] = value;
                    need[bit | mask] += value;
                }
            }
        }
    }

    int losingRanks = 0;
    for (int mask = 0; mask < states; ++mask) {
        if (!forbidden[mask] && need[mask] <= 0) {
            losingRanks |= mask;
        }
    }

    int64 answer = 0;
    for (int r = 0; r < m; ++r) {
        if (!(losingRanks & (1 << r))) {
            answer += rankCount[r];
        }
    }

    cout << answer << '\n';
    return 0;
}
