#include <bits/stdc++.h>
using namespace std;

const long long INF = 1000000000000000001LL; // 1e18 + 1

long long addCap(long long a, long long b) {
    long long s = a + b;
    return s >= INF ? INF : s;
}

long long mulCap(long long a, long long b) {
    if (a == 0 || b == 0) return 0;
    if (a >= INF || b >= INF) return INF;
    if (a > INF / b) return INF;
    long long c = a * b;
    return c >= INF ? INF : c;
}

// Edge i is between label i and i+1.
// Return true iff the alternating pattern requires q_i < q_{i+1}.
bool edgeUp(int i, int dir) {
    if (dir == 0) return i & 1;      // d1 > 0
    else return !(i & 1);            // d1 < 0
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    long long k;
    cin >> n >> k;

    // A[len] = number of alternating permutations of length len
    // for one fixed starting direction.
    vector<vector<long long>> ent(n + 1, vector<long long>(n + 1, 0));
    vector<long long> A(n + 1, 1);

    ent[0][0] = 1;
    A[0] = 1;
    for (int m = 1; m <= n; ++m) {
        ent[m][0] = 0;
        for (int x = 1; x <= m; ++x) {
            ent[m][x] = addCap(ent[m][x - 1], ent[m - 1][m - x]);
        }
        A[m] = ent[m][m];
    }

    // Binomial coefficients capped at INF.
    vector<vector<long long>> C(n + 1, vector<long long>(n + 1, 0));
    for (int i = 0; i <= n; ++i) {
        C[i][0] = C[i][i] = 1;
        for (int j = 1; j < i; ++j) {
            C[i][j] = addCap(C[i - 1][j - 1], C[i - 1][j]);
        }
    }

    vector<int> fixedVal(n + 1, 0);
    vector<int> ans(n + 1, 0);

    // Count ways to fill all unknown labels except x,
    // after making x fixed at the current position.
    auto calcFactor = [&](int x) -> long long {
        long long ways = 1;
        int processed = 0;
        int i = 1;

        while (i <= n) {
            if (fixedVal[i] == 0 && i != x) {
                int j = i;
                while (j <= n && fixedVal[j] == 0 && j != x) ++j;

                int len = j - i;

                ways = mulCap(ways, C[processed + len][len]);
                ways = mulCap(ways, A[len]);

                processed += len;
                i = j;
            } else {
                ++i;
            }
        }

        return ways;
    };

    // Check whether candidate x can have the current smallest unused q value
    // without conflicting with the alternating pattern direction dir.
    auto candidateOK = [&](int x, int dir) -> bool {
        if (x > 1 && fixedVal[x - 1] == 0) {
            // Left neighbor is still unknown, so it will be larger than q_x.
            // Need q_{x-1} > q_x, i.e. edge x-1 is decreasing.
            if (edgeUp(x - 1, dir)) return false;
        }

        if (x < n && fixedVal[x + 1] == 0) {
            // Right neighbor is still unknown, so it will be larger than q_x.
            // Need q_x < q_{x+1}, i.e. edge x is increasing.
            if (!edgeUp(x, dir)) return false;
        }

        return true;
    };

    for (int pos = 1; pos <= n; ++pos) {
        bool validPrev[2];

        for (int dir = 0; dir < 2; ++dir) {
            bool ok = true;

            for (int i = 1; i <= n && ok; ++i) {
                if (fixedVal[i] == 0) continue;

                if (i > 1) {
                    int j = i - 1;
                    bool up = edgeUp(j, dir);

                    if (fixedVal[j] == 0) {
                        // left unknown, right fixed
                        // Need q_j > q_i, so edge must be decreasing.
                        if (up) ok = false;
                    } else {
                        if (up) {
                            if (fixedVal[j] >= fixedVal[i]) ok = false;
                        } else {
                            if (fixedVal[j] <= fixedVal[i]) ok = false;
                        }
                    }
                }

                if (i < n && ok) {
                    int j = i + 1;
                    bool up = edgeUp(i, dir);

                    if (fixedVal[j] == 0) {
                        // left fixed, right unknown
                        // Need q_i < q_j, so edge must be increasing.
                        if (!up) ok = false;
                    } else {
                        if (up) {
                            if (fixedVal[i] >= fixedVal[j]) ok = false;
                        } else {
                            if (fixedVal[i] <= fixedVal[j]) ok = false;
                        }
                    }
                }
            }

            validPrev[dir] = ok;
        }

        int chosen = -1;

        for (int x = 1; x <= n; ++x) {
            if (fixedVal[x] != 0) continue;

            long long factor = calcFactor(x);

            int validDirections = 0;
            for (int dir = 0; dir < 2; ++dir) {
                if (validPrev[dir] && candidateOK(x, dir)) {
                    ++validDirections;
                }
            }

            long long cnt = (validDirections == 0 ? 0 : mulCap(factor, validDirections));

            if (k > cnt) {
                k -= cnt;
            } else {
                chosen = x;
                fixedVal[x] = pos;
                ans[pos] = x;
                break;
            }
        }

        if (chosen == -1) {
            cout << -1 << '\n';
            return 0;
        }
    }

    for (int i = 1; i <= n; ++i) {
        if (i > 1) cout << ' ';
        cout << ans[i];
    }
    cout << '\n';

    return 0;
}
