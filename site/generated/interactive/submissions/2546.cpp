#include <bits/stdc++.h>
using namespace std;

vector<long long> c;
int n;

bool solve_group(const vector<int>& idx) {
    int m = idx.size();
    long long ans[9][9][9];
    memset(ans, -1, sizeof(ans));
    for (int i = 0; i < m; ++i) {
        for (int j = i + 1; j < m; ++j) {
            for (int k = j + 1; k < m; ++k) {
                cout << "? " << idx[i] + 1 << " " << idx[j] + 1 << " " << idx[k] + 1 << "\n";
                cout.flush();
                long long res;
                if (!(cin >> res)) return false;
                if (res == -1) return false;
                ans[i][j][k] = res;
            }
        }
    }

    vector<int> perm(m);
    iota(perm.begin(), perm.end(), 0);
    do {
        vector<int> pos(m);
        for (int i = 0; i < m; ++i) pos[perm[i]] = i;

        long double S[9][9];
        bool has[9][9] = {false};
        for (int i = 0; i < m; ++i) {
            for (int j = i + 1; j < m; ++j) {
                for (int k = j + 1; k < m; ++k) {
                    int p[3] = {pos[i], pos[j], pos[k]};
                    sort(p, p + 3);
                    int a = p[0], c_idx = p[2];
                    if (c_idx - a >= 2 && !has[a][c_idx]) {
                        has[a][c_idx] = true;
                        S[a][c_idx] = ans[i][j][k];
                    }
                }
            }
        }

        vector<vector<pair<int, long double>>> adj(m);
        for (int a = 0; a < m; ++a) {
            for (int c_idx = a + 2; c_idx < m; ++c_idx) {
                if (has[a][c_idx]) {
                    adj[a].push_back({c_idx, S[a][c_idx]});
                    adj[c_idx].push_back({a, S[a][c_idx]});
                }
            }
        }

        vector<long double> alpha(m, 0), beta(m, 0);
        vector<bool> vis(m, false);
        queue<int> q;
        alpha[0] = 1;
        beta[0] = 0;
        vis[0] = true;
        q.push(0);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (auto& e : adj[u]) {
                int v = e.first;
                long double sum = e.second;
                if (!vis[v]) {
                    vis[v] = true;
                    alpha[v] = -alpha[u];
                    beta[v] = sum - beta[u];
                    q.push(v);
                }
            }
        }

        bool ok = true;
        for (int i = 0; i < m; ++i) if (!vis[i]) { ok = false; break; }
        if (!ok) continue;

        long double t = 0;
        bool t_set = false;
        for (int a = 0; a < m && ok; ++a) {
            for (int c_idx = a + 2; c_idx < m; ++c_idx) {
                if (has[a][c_idx]) {
                    long double coeff = alpha[a] + alpha[c_idx];
                    long double rhs = S[a][c_idx] - beta[a] - beta[c_idx];
                    if (fabs(coeff) < 1e-9) {
                        if (fabs(rhs) > 1e-6) { ok = false; break; }
                    } else {
                        long double t_cand = rhs / coeff;
                        if (!t_set) {
                            t = t_cand;
                            t_set = true;
                        } else if (fabs(t - t_cand) > 1e-6) {
                            ok = false; break;
                        }
                    }
                }
            }
        }
        if (!ok || !t_set) continue;

        vector<long double> y(m);
        for (int i = 0; i < m; ++i) y[i] = alpha[i] * t + beta[i];

        for (int i = 0; i < m; ++i) {
            if (y[i] < -1e-6 || y[i] > 1e9 + 1e-6) { ok = false; break; }
            long double r = round(y[i]);
            if (fabs(y[i] - r) > 1e-5) { ok = false; break; }
            if (i > 0 && y[i] < y[i - 1] - 1e-6) { ok = false; break; }
        }
        if (!ok) continue;

        for (int i = 0; i < m; ++i) {
            c[idx[perm[i]]] = (long long)round(y[i]);
        }
        return true;
    } while (next_permutation(perm.begin(), perm.end()));

    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;
    c.assign(n, 0);

    int start = 0;
    while (n - start > 9) {
        vector<int> group;
        for (int i = 0; i < 5; ++i) group.push_back(start + i);
        start += 5;
        if (!solve_group(group)) return 0;
    }

    vector<int> group;
    for (int i = start; i < n; ++i) group.push_back(i);
    if (!solve_group(group)) return 0;

    cout << "!";
    for (int i = 0; i < n; ++i) cout << " " << c[i];
    cout << "\n";
    cout.flush();
    return 0;
}
