#include <bits/stdc++.h>
using namespace std;

int query(int v, const string& b) {
    cout << "? " << v << " " << b << endl;
    int ans;
    if (!(cin >> ans)) return -1;
    if (ans == -1) exit(0);
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    int m = n - 1;

    vector<int> w(n + 1);
    string zero(m, '0');
    for (int v = 1; v <= n; ++v) {
        w[v] = query(v, zero);
    }

    int r = -1;
    for (int v = 1; v <= n; ++v) {
        if (w[v] == 0) {
            r = v;
            break;
        }
    }

    vector<int> depth(n + 1);
    int maxDepth = 0;
    for (int v = 1; v <= n; ++v) {
        depth[v] = w[v];
        maxDepth = max(maxDepth, depth[v]);
    }

    vector<vector<int>> byDepth(maxDepth + 1);
    for (int v = 1; v <= n; ++v) {
        byDepth[depth[v]].push_back(v);
    }

    vector<bitset<500>> P(n + 1);
    P[r].reset();

    vector<int> s(m), f(m);

    for (int d = 1; d <= maxDepth; ++d) {
        for (int v : byDepth[d]) {
            int parent = -1;
            for (int u : byDepth[d - 1]) {
                string b(m, '0');
                for (int i = 0; i < m; ++i) {
                    if (P[u][i]) b[i] = '1';
                }
                int ans = query(v, b);
                if (ans == 1) {
                    parent = u;
                    break;
                }
            }

            vector<int> candidates;
            for (int i = 0; i < m; ++i) {
                if (!P[parent][i]) candidates.push_back(i);
            }

            int lo = 0, hi = (int)candidates.size() - 1;
            int dv = depth[v];
            while (lo < hi) {
                int mid = (lo + hi) / 2;
                string b(m, '0');
                int sz = mid - lo + 1;
                for (int idx = lo; idx <= mid; ++idx) {
                    b[candidates[idx]] = '1';
                }
                int ans = query(v, b);
                int inter = (sz + dv - ans) / 2;
                if (inter == 1) {
                    hi = mid;
                } else {
                    lo = mid + 1;
                }
            }

            int e = candidates[lo];
            P[v] = P[parent];
            P[v].set(e);
            s[e] = v;
            f[e] = parent;
        }
    }

    cout << "!";
    for (int i = 0; i < m; ++i) {
        cout << " " << s[i] << " " << f[i];
    }
    cout << endl;

    return 0;
}
