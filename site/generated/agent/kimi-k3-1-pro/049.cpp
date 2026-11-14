#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;

int addmod(int a, int b) {
    int s = a + b;
    if (s >= MOD) s -= MOD;
    return s;
}

int submod(int a, int b) {
    int s = a - b;
    if (s < 0) s += MOD;
    return s;
}

int mulmod(int a, int b) {
    return int(1LL * a * b % MOD);
}

int powmod(int a, int e) {
    int r = 1;
    while (e > 0) {
        if (e & 1) r = mulmod(r, a);
        a = mulmod(a, a);
        e >>= 1;
    }
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        int n;
        cin >> n;
        vector<int> p(n);
        for (int &x : p) cin >> x;

        bool sorted = true;
        for (int i = 0; i < n; ++i) {
            if (p[i] != i + 1) {
                sorted = false;
                break;
            }
        }
        if (sorted) {
            cout << 0 << '\n';
            continue;
        }

        vector<int> inv(n + 1);
        inv[1] = 1;
        for (int i = 2; i <= n; ++i) {
            inv[i] = mulmod(MOD - MOD / i, inv[MOD % i]);
        }

        int q = mulmod(n, inv[n - 1]);
        int r = inv[n - 1];
        int z = mulmod(inv[n], inv[n]);

        vector<int> qp(2 * n + 1);
        qp[0] = 1;
        for (int i = 1; i <= 2 * n; ++i) qp[i] = mulmod(qp[i - 1], q);

        vector<vector<int>> d(n, vector<int>(n + 1));
        for (int a = 1; a < n; ++a) {
            int qm = 1, qprev = 0, rprev = 0;
            for (int m = 0; m < n; ++m) {
                int rc = mulmod(qp[a + m - 1], qm);
                d[m][a] = submod(rc, rprev);
                rprev = rc;

                if (m + 1 < n) {
                    int coef1 = addmod(1, mulmod(a - 1, z));
                    coef1 = addmod(coef1, mulmod(m, submod(2, z)));
                    int coef2 = mulmod(m, submod(z, 1));
                    int qnext = addmod(mulmod(coef1, qm), mulmod(coef2, qprev));
                    qnext = mulmod(qnext, inv[m + 1]);
                    qprev = qm;
                    qm = qnext;
                }
            }
        }

        vector<char> inPrefix(n + 1, false);
        vector<int> parts;
        int sum = 0;
        for (int i = 1; i < n; ++i) {
            inPrefix[p[i - 1]] = true;
            parts.clear();
            int count = 0;
            for (int value = 1; value <= n; ++value) {
                if (!inPrefix[value]) continue;
                ++count;
                int part = value - count;
                if (part > 0) parts.push_back(part);
            }

            int g = 0;
            int k = int(parts.size());
            for (int j = 0; j < k; ++j) {
                g = addmod(g, d[k - 1 - j][parts[j]]);
            }
            sum = addmod(sum, g);
        }

        cout << addmod(1, mulmod(r, sum)) << '\n';
    }
    return 0;
}
