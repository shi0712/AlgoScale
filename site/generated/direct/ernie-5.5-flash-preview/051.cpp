#include <bits/stdc++.h>
using namespace std;

const int MOD = 1000000007;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, Q;
    cin >> n >> Q;
    string s;
    cin >> s;

    int m = 0;
    for (char c : s) if (c == '?') ++m;

    // No question marks: only one possible assignment (the fixed string itself)
    if (m == 0) {
        bool valid = true;
        for (int i = 1; i < n; ++i)
            if (s[i] == s[i - 1]) valid = false;
        int ans = valid ? 1 : 0;
        while (Q--) {
            int x, y, z;
            cin >> x >> y >> z;
            cout << ans << '\n';
        }
        return 0;
    }

    int stride = m + 1;
    int sz = stride * stride;

    vector<int> curA(sz, 0), curB(sz, 0), curC(sz, 0);
    vector<int> nxtA(sz, 0), nxtB(sz, 0), nxtC(sz, 0);

    int t = 0; // number of '?' processed so far

    // Process the first character separately (no previous character)
    char first = s[0];
    if (first == '?') {
        curA[1 * stride + 0] = 1; // i = 1, j = 0
        curB[0 * stride + 1] = 1; // i = 0, j = 1
        curC[0 * stride + 0] = 1; // i = 0, j = 0
        t = 1;
    } else {
        if (first == 'a') curA[0] = 1;
        else if (first == 'b') curB[0] = 1;
        else curC[0] = 1;
        t = 0;
    }

    for (int pos = 1; pos < n; ++pos) {
        char ch = s[pos];
        if (ch == '?') {
            int new_t = t + 1;
            for (int i = 0; i <= new_t; ++i) {
                int maxj = new_t - i;
                int base = i * stride;
                for (int j = 0; j <= maxj; ++j) {
                    int idx = base + j;
                    int a = 0, b = 0, c = 0;

                    if (i > 0) {
                        int prev = idx - stride;
                        a = curB[prev] + curC[prev];
                        if (a >= MOD) a -= MOD;
                    }
                    if (j > 0) {
                        int prev = idx - 1;
                        b = curA[prev] + curC[prev];
                        if (b >= MOD) b -= MOD;
                    }
                    if (i + j <= t) {
                        c = curA[idx] + curB[idx];
                        if (c >= MOD) c -= MOD;
                    }

                    nxtA[idx] = a;
                    nxtB[idx] = b;
                    nxtC[idx] = c;
                }
            }
            swap(curA, nxtA);
            swap(curB, nxtB);
            swap(curC, nxtC);
            t = new_t;
        } else {
            for (int i = 0; i <= t; ++i) {
                int maxj = t - i;
                int base = i * stride;
                for (int j = 0; j <= maxj; ++j) {
                    int idx = base + j;
                    if (ch == 'a') {
                        int val = curB[idx] + curC[idx];
                        if (val >= MOD) val -= MOD;
                        curA[idx] = val;
                        curB[idx] = 0;
                        curC[idx] = 0;
                    } else if (ch == 'b') {
                        int val = curA[idx] + curC[idx];
                        if (val >= MOD) val -= MOD;
                        curB[idx] = val;
                        curA[idx] = 0;
                        curC[idx] = 0;
                    } else { // 'c'
                        int val = curA[idx] + curB[idx];
                        if (val >= MOD) val -= MOD;
                        curC[idx] = val;
                        curA[idx] = 0;
                        curB[idx] = 0;
                    }
                }
            }
        }
    }

    // R[i][j] = sum_{b=0..j} F[i][b]
    // P[i][j] = sum_{a=0..i, b=0..j} F[a][b]
    vector<vector<int>> R(m + 1, vector<int>(m + 1, 0));
    vector<vector<int>> P(m + 1, vector<int>(m + 1, 0));

    for (int i = 0; i <= m; ++i) {
        int rowSum = 0;
        int base = i * stride;
        for (int j = 0; j <= m; ++j) {
            int val = 0;
            if (i + j <= m) {
                int idx = base + j;
                val = curA[idx] + curB[idx];
                if (val >= MOD) val -= MOD;
                val += curC[idx];
                if (val >= MOD) val -= MOD;
            }
            rowSum += val;
            if (rowSum >= MOD) rowSum -= MOD;
            R[i][j] = rowSum;

            int p = (i > 0 ? P[i - 1][j] : 0) + R[i][j];
            if (p >= MOD) p -= MOD;
            P[i][j] = p;
        }
    }

    while (Q--) {
        int x, y, z;
        cin >> x >> y >> z;

        if (x > m) x = m;
        if (y > m) y = m;

        int L = m - z;
        if (L < 0) L = 0;

        int total = P[x][y];
        int sub = 0;

        if (L > 0) {
            int maxi = min(x, L - 1);
            for (int i = 0; i <= maxi; ++i) {
                int jmax = L - 1 - i;
                if (jmax > y) jmax = y;
                if (jmax >= 0) {
                    sub += R[i][jmax];
                    if (sub >= MOD) sub -= MOD;
                }
            }
        }

        int ans = total - sub;
        if (ans < 0) ans += MOD;
        cout << ans << '\n';
    }

    return 0;
}
