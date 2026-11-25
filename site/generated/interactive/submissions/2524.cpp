#include <bits/stdc++.h>
using namespace std;

int n;
int R[101][101];

bool solve_candidate(const vector<int>& S, vector<int>& ans) {
    vector<vector<int>> V(n + 1);
    for (int p = 1; p <= n; ++p) {
        for (int x = 0; x < n; ++x) {
            if (!S[x]) continue;
            bool ok = true;
            for (int t = 1; t <= n - 1; ++t) {
                int y = (x + t) % n;
                if (R[p][t] != S[y]) {
                    ok = false;
                    break;
                }
            }
            if (ok) V[p].push_back(x);
        }
        if (V[p].empty()) return false;
    }

    vector<int> x(n + 1, -1);
    vector<int> c(n, 0);
    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return V[a].size() < V[b].size();
    });

    function<bool(int)> dfs = [&](int idx) -> bool {
        if (idx == n) {
            for (int v = 0; v < n; ++v) {
                if ((c[v] > 0) != (S[v] == 1)) return false;
            }
            for (int p = 1; p <= n; ++p) {
                int val = x[p];
                if (R[p][n] == 0) {
                    if (c[val] != 1) return false;
                } else {
                    if (c[val] < 2) return false;
                }
            }
            ans = x;
            return true;
        }

        int p = order[idx];
        for (int val : V[p]) {
            if (R[p][n] == 0 && c[val] > 0) continue;
            c[val]++;
            x[p] = val;

            bool ok = true;
            for (int v = 0; v < n; ++v) {
                if (S[v] && c[v] == 0) {
                    bool can = false;
                    for (int j = idx + 1; j < n; ++j) {
                        int q = order[j];
                        for (int u : V[q]) {
                            if (u == v) {
                                can = true;
                                break;
                            }
                        }
                        if (can) break;
                    }
                    if (!can) {
                        ok = false;
                        break;
                    }
                }
            }

            if (ok && dfs(idx + 1)) return true;

            c[val]--;
            x[p] = -1;
        }
        return false;
    };

    return dfs(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    // Phase 0: create a duplicate
    int p = 1;
    while (true) {
        cout << "? " << p << "\n";
        cout.flush();
        int r;
        cin >> r;
        if (r == -1) return 0;
        if (r == 1) break;
    }

    // Reconstruction
    for (int i = 1; i <= n; ++i) {
        for (int t = 1; t <= n; ++t) {
            cout << "? " << i << "\n";
            cout.flush();
            int r;
            cin >> r;
            if (r == -1) return 0;
            R[i][t] = r;
        }
    }

    int K = 0;
    for (int i = 1; i <= n; ++i) {
        int ones = 0;
        for (int t = 1; t <= n; ++t) ones += R[i][t];
        K = max(K, ones);
    }

    int p0 = -1;
    for (int i = 1; i <= n; ++i) {
        int ones = 0;
        for (int t = 1; t <= n; ++t) ones += R[i][t];
        if (ones == K) {
            p0 = i;
            break;
        }
    }

    vector<int> A(n);
    for (int i = 0; i < n; ++i) A[i] = R[p0][i + 1];

    vector<int> ans;
    bool found = false;
    for (int shift = 0; shift < n; ++shift) {
        vector<int> S(n, 0);
        for (int v = 0; v < n; ++v) {
            S[v] = A[(v + shift) % n];
        }
        int sz = accumulate(S.begin(), S.end(), 0);
        if (sz != K) continue;
        if (solve_candidate(S, ans)) {
            found = true;
            break;
        }
    }

    if (!found) return 0;

    // Choose target minimizing total steps
    int best_target = 0;
    long long best_cost = LLONG_MAX;
    for (int t = 0; t < n; ++t) {
        long long cost = 0;
        for (int i = 1; i <= n; ++i) {
            cost += (t - ans[i] + n) % n;
        }
        if (cost < best_cost) {
            best_cost = cost;
            best_target = t;
        }
    }

    // Execute moves
    for (int i = 1; i <= n; ++i) {
        int steps = (best_target - ans[i] + n) % n;
        for (int s = 0; s < steps; ++s) {
            cout << "? " << i << "\n";
            cout.flush();
            int r;
            cin >> r;
            if (r == -1) return 0;
        }
    }

    cout << "!\n";
    cout.flush();
    return 0;
}
