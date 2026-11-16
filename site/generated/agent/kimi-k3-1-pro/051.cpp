#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;

inline int addmod(int a, int b) {
    int s = a + b;
    if (s >= MOD) s -= MOD;
    return s;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, Q;
    cin >> n >> Q;
    string s;
    cin >> s;

    int m = count(s.begin(), s.end(), '?');
    int B = m + 1;
    int N = B * B;

    vector<int> a(N), b(N), c(N), na(N), nb(N), nc(N);
    int used = 0;

    if (s[0] == '?') {
        used = 1;
        a[B] = 1;
        b[1] = 1;
        c[0] = 1;
    } else {
        int col = s[0] - 'a';
        if (col == 0) a[0] = 1;
        if (col == 1) b[0] = 1;
        if (col == 2) c[0] = 1;
    }

    for (int pos = 1; pos < n; ++pos) {
        if (s[pos] == '?') {
            ++used;
            for (int i = 0; i <= used; ++i) {
                for (int j = 0; i + j <= used; ++j) {
                    int k = used - i - j;
                    int id = i * B + j;
                    int va = 0, vb = 0, vc = 0;

                    if (i > 0) {
                        int p = id - B;
                        int total = addmod(addmod(a[p], b[p]), c[p]);
                        va = total - a[p];
                        if (va < 0) va += MOD;
                    }
                    if (j > 0) {
                        int p = id - 1;
                        int total = addmod(addmod(a[p], b[p]), c[p]);
                        vb = total - b[p];
                        if (vb < 0) vb += MOD;
                    }
                    if (k > 0) {
                        int total = addmod(addmod(a[id], b[id]), c[id]);
                        vc = total - c[id];
                        if (vc < 0) vc += MOD;
                    }

                    na[id] = va;
                    nb[id] = vb;
                    nc[id] = vc;
                }
            }
            a.swap(na);
            b.swap(nb);
            c.swap(nc);
        } else {
            int col = s[pos] - 'a';
            for (int i = 0; i <= used; ++i) {
                for (int j = 0; i + j <= used; ++j) {
                    int id = i * B + j;
                    if (col == 0) {
                        int v = addmod(b[id], c[id]);
                        b[id] = c[id] = 0;
                        a[id] = v;
                    } else if (col == 1) {
                        int v = addmod(a[id], c[id]);
                        a[id] = c[id] = 0;
                        b[id] = v;
                    } else {
                        int v = addmod(a[id], b[id]);
                        a[id] = b[id] = 0;
                        c[id] = v;
                    }
                }
            }
        }
    }

    vector<int> exact(N, 0);
    for (int i = 0; i <= m; ++i) {
        for (int j = 0; i + j <= m; ++j) {
            int id = i * B + j;
            exact[id] = addmod(addmod(a[id], b[id]), c[id]);
        }
    }

    vector<int> rect(N, 0), prefA(N, 0), prefB(N, 0);
    for (int i = 0; i <= m; ++i) {
        for (int j = 0; j <= m; ++j) {
            int id = i * B + j;
            int v = exact[id];
            if (i > 0) v = addmod(v, rect[id - B]);
            if (j > 0) v = addmod(v, rect[id - 1]);
            if (i > 0 && j > 0) {
                v -= rect[id - B - 1];
                if (v < 0) v += MOD;
            }
            rect[id] = v;
        }
    }

    for (int sum = 0; sum <= m; ++sum) {
        for (int i = 0; i <= m; ++i) {
            int id = sum * B + i;
            int va = (i <= sum ? exact[i * B + (sum - i)] : 0);
            int vb = (i <= sum ? exact[(sum - i) * B + i] : 0);
            if (sum > 0) {
                va = addmod(va, prefA[id - B]);
                vb = addmod(vb, prefB[id - B]);
            }
            if (i > 0) {
                va = addmod(va, prefA[id - 1]);
                vb = addmod(vb, prefB[id - 1]);
            }
            if (sum > 0 && i > 0) {
                va -= prefA[id - B - 1];
                if (va < 0) va += MOD;
                vb -= prefB[id - B - 1];
                if (vb < 0) vb += MOD;
            }
            prefA[id] = va;
            prefB[id] = vb;
        }
    }

    auto getRect = [&](int x, int y) -> int {
        if (x < 0 || y < 0) return 0;
        x = min(x, m);
        y = min(y, m);
        return rect[x * B + y];
    };
    auto getPrefA = [&](int sum, int x) -> int {
        if (sum < 0 || x < 0) return 0;
        sum = min(sum, m);
        x = min(x, m);
        return prefA[sum * B + x];
    };
    auto getPrefB = [&](int sum, int y) -> int {
        if (sum < 0 || y < 0) return 0;
        sum = min(sum, m);
        y = min(y, m);
        return prefB[sum * B + y];
    };

    int total = getRect(m, m);
    while (Q--) {
        int x, y, z;
        cin >> x >> y >> z;
        x = min(x, m);
        y = min(y, m);
        z = min(z, m);

        int badA = total - getRect(x, m);
        if (badA < 0) badA += MOD;
        int badB = total - getRect(m, y);
        if (badB < 0) badB += MOD;

        int sumLimit = m - z - 1;
        int badC = getPrefA(sumLimit, m);

        int badAB = total - getRect(x, m) - getRect(m, y) + getRect(x, y);
        badAB %= MOD;
        if (badAB < 0) badAB += MOD;
        int badAC = badC - getPrefA(sumLimit, x);
        if (badAC < 0) badAC += MOD;
        int badBC = badC - getPrefB(sumLimit, y);
        if (badBC < 0) badBC += MOD;

        int ans = total;
        ans -= badA;
        if (ans < 0) ans += MOD;
        ans -= badB;
        if (ans < 0) ans += MOD;
        ans -= badC;
        if (ans < 0) ans += MOD;
        ans = addmod(ans, badAB);
        ans = addmod(ans, badAC);
        ans = addmod(ans, badBC);
        cout << ans << '\n';
    }

    return 0;
}
