#include <bits/stdc++.h>
using namespace std;

int n;
int f[5][5][5];
int firstResp[5][5][5];

int ask(int i, int j, int k) {
    cout << "? " << i + 1 << " " << j + 1 << " " << k + 1 << "\n";
    cout.flush();
    int r;
    if (!(cin >> r)) exit(0);
    return r;
}

void outputArray(const vector<int>& a) {
    cout << "!";
    for (int x : a) cout << " " << x;
    cout << "\n";
    cout.flush();
    exit(0);
}

void outputMinus1() {
    cout << "! -1\n";
    cout.flush();
    exit(0);
}

void reconstruct(int x, int y, int z, int r) {
    int pairs[3][2] = {{x, y}, {x, z}, {y, z}};
    for (int p = 0; p < 3; ++p) {
        int u = pairs[p][0], v = pairs[p][1];
        int third = x + y + z - u - v;
        vector<int> R(n, -2);
        for (int t = 0; t < n; ++t) {
            if (t == u || t == v) continue;
            int val;
            if (t == third) {
                val = r;
            } else {
                int a = u, b = v, c = t;
                if (a > b) swap(a, b);
                if (b > c) swap(b, c);
                if (a > b) swap(a, b);
                if (a < 5 && firstResp[a][b][c] != -1)
                    val = firstResp[a][b][c];
                else
                    val = ask(u, v, t);
            }
            R[t] = val;
        }

        vector<pair<int, int>> valid;
        for (int a = 1; a <= 4; ++a) {
            for (int b = a; b <= 4; ++b) {
                bool ok = true;
                for (int t = 0; t < n; ++t) {
                    if (t == u || t == v) continue;
                    bool exists = false;
                    for (int c = 1; c <= 4; ++c) {
                        if (f[a][b][c] == R[t]) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        ok = false;
                        break;
                    }
                }
                if (ok) valid.push_back({a, b});
            }
        }

        if (valid.size() == 1) {
            int a = valid[0].first, b = valid[0].second;
            bool good = true;
            for (int c1 = 1; c1 <= 4; ++c1)
                for (int c2 = c1 + 1; c2 <= 4; ++c2)
                    if (f[a][b][c1] == f[a][b][c2]) good = false;
            if (!good) continue;

            vector<int> ans(n);
            for (int t = 0; t < n; ++t) {
                if (t == u || t == v) continue;
                int w = -1;
                for (int c = 1; c <= 4; ++c) {
                    if (f[a][b][c] == R[t]) {
                        w = c;
                        break;
                    }
                }
                ans[t] = w;
            }

            if (a == b) {
                ans[u] = ans[v] = a;
            } else {
                bool found = false;
                for (int p1 = 0; p1 < n && !found; ++p1) {
                    if (p1 == u || p1 == v) continue;
                    for (int q = p1 + 1; q < n && !found; ++q) {
                        if (q == u || q == v) continue;
                        if (f[a][ans[p1]][ans[q]] != f[b][ans[p1]][ans[q]]) {
                            int resp = ask(u, p1, q);
                            if (resp == f[a][ans[p1]][ans[q]]) {
                                ans[u] = a;
                                ans[v] = b;
                            } else {
                                ans[u] = b;
                                ans[v] = a;
                            }
                            found = true;
                        }
                    }
                }
                if (!found) outputMinus1();
            }
            outputArray(ans);
        }
    }
    outputMinus1();
}

void caseA(int a, int b, int c) {
    vector<bool> isOne(n, false);
    isOne[a] = isOne[b] = isOne[c] = true;
    for (int i = 0; i < n; ++i) {
        if (i == a || i == b || i == c) continue;
        int r = ask(a, b, i);
        if (r == 3)
            isOne[i] = true;
        else
            isOne[i] = false;
    }

    vector<int> S;
    for (int i = 0; i < n; ++i)
        if (!isOne[i]) S.push_back(i);

    if (S.empty()) {
        vector<int> ans(n, 1);
        outputArray(ans);
    }
    if (S.size() == 1) outputMinus1();

    int m = min(4, (int)S.size());
    int p = -1, q = -1, v = 0;
    for (int i = 0; i < m; ++i) {
        for (int j = i + 1; j < m; ++j) {
            int r = ask(S[i], S[j], a);
            if (r == 15) {
                p = S[i];
                q = S[j];
                v = 2;
                break;
            }
            if (r == 25) {
                p = S[i];
                q = S[j];
                v = 3;
                break;
            }
            if (r == 63) {
                p = S[i];
                q = S[j];
                v = 4;
                break;
            }
        }
        if (p != -1) break;
    }
    if (p == -1) outputMinus1();

    vector<int> ans(n);
    for (int i = 0; i < n; ++i) {
        if (i == p || i == q) continue;
        int r = ask(p, q, i);
        int w = -1;
        for (int c = 1; c <= 4; ++c) {
            if (f[v][v][c] == r) {
                w = c;
                break;
            }
        }
        ans[i] = w;
    }
    ans[p] = ans[q] = v;
    outputArray(ans);
}

void solveSmall() {
    if (n == 3) {
        int r = ask(0, 1, 2);
        vector<array<int, 3>> matches;
        for (int a = 1; a <= 4; ++a)
            for (int b = 1; b <= 4; ++b)
                for (int c = 1; c <= 4; ++c)
                    if (f[a][b][c] == r)
                        matches.push_back({a, b, c});
        if (matches.size() == 1)
            outputArray({matches[0][0], matches[0][1], matches[0][2]});
        else
            outputMinus1();
    } else {
        int resp[4];
        resp[0] = ask(0, 1, 2);
        resp[1] = ask(0, 1, 3);
        resp[2] = ask(0, 2, 3);
        resp[3] = ask(1, 2, 3);
        vector<array<int, 4>> matches;
        for (int a = 1; a <= 4; ++a)
            for (int b = 1; b <= 4; ++b)
                for (int c = 1; c <= 4; ++c)
                    for (int d = 1; d <= 4; ++d)
                        if (f[a][b][c] == resp[0] &&
                            f[a][b][d] == resp[1] &&
                            f[a][c][d] == resp[2] &&
                            f[b][c][d] == resp[3])
                            matches.push_back({a, b, c, d});
        if (matches.size() == 1)
            outputArray({matches[0][0], matches[0][1], matches[0][2], matches[0][3]});
        else
            outputMinus1();
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    for (int a = 1; a <= 4; ++a)
        for (int b = 1; b <= 4; ++b)
            for (int c = 1; c <= 4; ++c) {
                if (a + b <= c || a + c <= b || b + c <= a) {
                    f[a][b][c] = 0;
                } else {
                    long long s = a + b + c;
                    long long val = s * (-a + b + c) * (a - b + c) * (a + b - c);
                    f[a][b][c] = (int)val;
                }
            }

    if (n == 3 || n == 4) {
        solveSmall();
        return 0;
    }

    memset(firstResp, -1, sizeof(firstResp));
    bool foundNonzero = false, foundOnes = false;
    int nz_i, nz_j, nz_k, nz_r;
    int one_i, one_j, one_k;

    for (int i = 0; i < 5; ++i)
        for (int j = i + 1; j < 5; ++j)
            for (int k = j + 1; k < 5; ++k) {
                int r = ask(i, j, k);
                firstResp[i][j][k] = r;
                if (r == 3 && !foundOnes) {
                    foundOnes = true;
                    one_i = i;
                    one_j = j;
                    one_k = k;
                }
                if (r != 0 && r != 3 && !foundNonzero) {
                    foundNonzero = true;
                    nz_i = i;
                    nz_j = j;
                    nz_k = k;
                    nz_r = r;
                }
            }

    if (foundNonzero)
        reconstruct(nz_i, nz_j, nz_k, nz_r);
    else if (foundOnes)
        caseA(one_i, one_j, one_k);
    else
        outputMinus1();

    return 0;
}
