#include <bits/stdc++.h>
using namespace std;

const int MAXM = 505;

int n, m;
string b0;

int ask(int v, const string& b) {
    cout << "? " << v << ' ' << b << '\n';
    cout.flush();

    int ans;
    if (!(cin >> ans)) exit(0);
    if (ans < 0) exit(0);
    return ans;
}

int queryCount(int v, const bitset<MAXM>& S, int dep) {
    string b = b0;
    for (int i = 0; i < m; ++i) {
        if (S[i]) b[i] = (b[i] == '1' ? '0' : '1');
    }

    int ans = ask(v, b);
    int sz = (int)S.count();

    int val = dep + sz - ans;
    if (val < 0) val = 0;
    if (val > 2 * sz) val = 2 * sz;
    return val / 2;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "!\n";
        cout.flush();
        return 0;
    }

    m = n - 1;

    vector<int> z(m, 0);

    if (m == 1) {
        int a = ask(1, "1");
        z[0] = (a == 1 ? 1 : 0);
    } else if (m == 2) {
        int S = ask(1, "11");
        if (S == 0) {
            z[0] = z[1] = 0;
        } else if (S == 2) {
            z[0] = z[1] = 1;
        } else {
            int a0 = ask(1, "10");
            z[0] = (a0 == 2 ? 1 : 0);
            z[1] = 1 - z[0];
        }
    } else {
        vector<int> a(m);
        int sumA = 0;

        for (int i = 0; i < m; ++i) {
            string b(m, '0');
            b[i] = '1';
            a[i] = ask(1, b);
            sumA += a[i];
        }

        int S = (m * (m - 1) - sumA) / (m - 2);

        for (int i = 0; i < m; ++i) {
            int zi = (a[i] - (m - 1) + S) / 2;
            if (zi < 0) zi = 0;
            if (zi > 1) zi = 1;
            z[i] = zi;
        }
    }

    b0.assign(m, '0');
    for (int i = 0; i < m; ++i) {
        b0[i] = (z[i] ? '0' : '1');
    }

    if (n == 2) {
        cout << "! ";
        if (b0[0] == '1') cout << "1 2";
        else cout << "2 1";
        cout << '\n';
        cout.flush();
        return 0;
    }

    vector<int> depth(n + 1, 0);
    for (int v = 2; v <= n; ++v) {
        depth[v] = ask(v, b0);
        if (depth[v] < 0) depth[v] = 0;
        if (depth[v] >= n) depth[v] = n - 1;
    }

    vector<vector<int>> byDepth(n);
    for (int v = 1; v <= n; ++v) {
        byDepth[depth[v]].push_back(v);
    }

    int maxD = 0;
    for (int v = 1; v <= n; ++v) {
        maxD = max(maxD, depth[v]);
    }

    vector<bitset<MAXM>> anc(n + 1);
    vector<int> inc(n + 1, -1);
    vector<char> assigned(m, 0);
    vector<int> ansS(m, 0), ansF(m, 0);

    for (int d = 1; d <= maxD; ++d) {
        const vector<int>& cand = byDepth[d - 1];
        if (cand.empty()) continue;

        for (int v : byDepth[d]) {
            int p;

            if (cand.size() == 1) {
                p = cand[0];
            } else {
                int lo = 0, hi = (int)cand.size() - 1;

                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    bitset<MAXM> S;

                    for (int idx = lo; idx <= mid; ++idx) {
                        int e = inc[cand[idx]];
                        if (e >= 0) S.set(e);
                    }

                    int cnt = queryCount(v, S, depth[v]);

                    if (cnt > 0) hi = mid;
                    else lo = mid + 1;
                }

                p = cand[lo];
            }

            vector<int> comp;
            comp.reserve(m);

            for (int i = 0; i < m; ++i) {
                if (!anc[p][i] && !assigned[i]) {
                    comp.push_back(i);
                }
            }

            int e;

            if (comp.empty()) {
                e = 0;
            } else if (comp.size() == 1) {
                e = comp[0];
            } else {
                int lo = 0, hi = (int)comp.size() - 1;

                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    bitset<MAXM> S;

                    for (int idx = lo; idx <= mid; ++idx) {
                        S.set(comp[idx]);
                    }

                    int cnt = queryCount(v, S, depth[v]);

                    if (cnt > 0) hi = mid;
                    else lo = mid + 1;
                }

                e = comp[lo];
            }

            inc[v] = e;
            anc[v] = anc[p];
            anc[v].set(e);
            assigned[e] = 1;

            if (b0[e] == '1') {
                ansS[e] = p;
                ansF[e] = v;
            } else {
                ansS[e] = v;
                ansF[e] = p;
            }
        }
    }

    cout << "!";
    for (int i = 0; i < m; ++i) {
        cout << ' ' << ansS[i] << ' ' << ansF[i];
    }
    cout << '\n';
    cout.flush();

    return 0;
}
