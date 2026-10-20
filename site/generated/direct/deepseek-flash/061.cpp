#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1'000'000'007;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, Q;
    cin >> n >> Q;

    string s;
    cin >> s;

    int q = 0;
    for (char c : s) if (c == '?') ++q;

    int W = q + 1;
    int N = W * W;

    vector<int> da(N), db(N), dc(N);
    vector<int> na(N), nb(N), nc(N);

    int t = 0;

    // Initialize with the first position.
    if (s[0] == '?') {
        t = 1;
        da[1 * W + 0] = 1; // last is 'a'
        db[0 * W + 1] = 1; // last is 'b'
        dc[0 * W + 0] = 1; // last is 'c'
    } else if (s[0] == 'a') {
        da[0] = 1;
    } else if (s[0] == 'b') {
        db[0] = 1;
    } else {
        dc[0] = 1;
    }

    for (int pos = 1; pos < n; ++pos) {
        char ch = s[pos];

        if (ch == '?') {
            int tnew = t + 1;

            // i = 0
            {
                int base = 0;
                int jmax = tnew;
                for (int j = 0; j <= jmax; ++j) {
                    int nbv = 0;
                    if (j > 0) {
                        int v = da[base + j - 1] + dc[base + j - 1];
                        if (v >= MOD) v -= MOD;
                        nbv = v;
                    }

                    int ncv = 0;
                    if (j < jmax) {
                        int v = da[base + j] + db[base + j];
                        if (v >= MOD) v -= MOD;
                        ncv = v;
                    }

                    na[base + j] = 0;
                    nb[base + j] = nbv;
                    nc[base + j] = ncv;
                }
            }

            for (int i = 1; i <= tnew; ++i) {
                int base = i * W;
                int prev = (i - 1) * W;
                int jmax = tnew - i;

                for (int j = 0; j <= jmax; ++j) {
                    int idx = base + j;

                    int v = db[prev + j] + dc[prev + j];
                    if (v >= MOD) v -= MOD;
                    na[idx] = v;

                    int nbv = 0;
                    if (j > 0) {
                        int v2 = da[base + j - 1] + dc[base + j - 1];
                        if (v2 >= MOD) v2 -= MOD;
                        nbv = v2;
                    }
                    nb[idx] = nbv;

                    int ncv = 0;
                    if (j < jmax) {
                        int v3 = da[idx] + db[idx];
                        if (v3 >= MOD) v3 -= MOD;
                        ncv = v3;
                    }
                    nc[idx] = ncv;
                }
            }

            da.swap(na);
            db.swap(nb);
            dc.swap(nc);
            t = tnew;
        } else {
            for (int i = 0; i <= t; ++i) {
                int base = i * W;
                int jmax = t - i;

                if (ch == 'a') {
                    for (int j = 0; j <= jmax; ++j) {
                        int idx = base + j;
                        int v = db[idx] + dc[idx];
                        if (v >= MOD) v -= MOD;
                        na[idx] = v;
                        nb[idx] = 0;
                        nc[idx] = 0;
                    }
                } else if (ch == 'b') {
                    for (int j = 0; j <= jmax; ++j) {
                        int idx = base + j;
                        int v = da[idx] + dc[idx];
                        if (v >= MOD) v -= MOD;
                        nb[idx] = v;
                        na[idx] = 0;
                        nc[idx] = 0;
                    }
                } else {
                    for (int j = 0; j <= jmax; ++j) {
                        int idx = base + j;
                        int v = da[idx] + db[idx];
                        if (v >= MOD) v -= MOD;
                        nc[idx] = v;
                        na[idx] = 0;
                        nb[idx] = 0;
                    }
                }
            }

            da.swap(na);
            db.swap(nb);
            dc.swap(nc);
        }
    }

    // C[i][j] = number of valid sequences where '?' receives
    // i copies of 'a' and j copies of 'b'.
    vector<int> C(N, 0);

    for (int i = 0; i <= q; ++i) {
        int base = i * W;
        int jmax = q - i;

        for (int j = 0; j <= jmax; ++j) {
            int v = da[base + j] + db[base + j];
            if (v >= MOD) v -= MOD;
            v += dc[base + j];
            if (v >= MOD) v -= MOD;
            C[base + j] = v;
        }
    }

    vector<int> rowSum(q + 1, 0), colSum(q + 1, 0);
    int total = 0;

    for (int i = 0; i <= q; ++i) {
        int base = i * W;
        int jmax = q - i;

        for (int j = 0; j <= jmax; ++j) {
            int val = C[base + j];

            total += val;
            if (total >= MOD) total -= MOD;

            int v = rowSum[i] + val;
            if (v >= MOD) v -= MOD;
            rowSum[i] = v;

            v = colSum[j] + val;
            if (v >= MOD) v -= MOD;
            colSum[j] = v;
        }
    }

    vector<int> rowSuf(q + 2, 0), colSuf(q + 2, 0);

    for (int i = q; i >= 0; --i) {
        rowSuf[i] = rowSuf[i + 1] + rowSum[i];
        if (rowSuf[i] >= MOD) rowSuf[i] -= MOD;
    }

    for (int j = q; j >= 0; --j) {
        colSuf[j] = colSuf[j + 1] + colSum[j];
        if (colSuf[j] >= MOD) colSuf[j] -= MOD;
    }

    // 2D suffix sums of C.
    int W2 = q + 2;
    vector<int> suff(W2 * W2, 0);

    for (int i = q; i >= 0; --i) {
        int base = i * W2;
        int below = (i + 1) * W2;
        int cbase = i * W;

        for (int j = q; j >= 0; --j) {
            int val = C[cbase + j];

            val += suff[below + j];
            if (val >= MOD) val -= MOD;

            val += suff[base + j + 1];
            if (val >= MOD) val -= MOD;

            val -= suff[below + j + 1];
            if (val < 0) val += MOD;

            suff[base + j] = val;
        }
    }

    int RW = q + 2;

    vector<int> rowPref(W * RW, 0);
    vector<int> colPref(W * RW, 0);

    for (int i = 0; i <= q; ++i) {
        int pref = 0;
        int rbase = i * RW;
        int cbase = i * W;

        rowPref[rbase] = 0;

        for (int j = 0; j <= q; ++j) {
            pref += C[cbase + j];
            if (pref >= MOD) pref -= MOD;
            rowPref[rbase + j + 1] = pref;
        }
    }

    for (int j = 0; j <= q; ++j) {
        int pref = 0;
        int rbase = j * RW;

        colPref[rbase] = 0;

        for (int i = 0; i <= q; ++i) {
            pref += C[i * W + j];
            if (pref >= MOD) pref -= MOD;
            colPref[rbase + i + 1] = pref;
        }
    }

    // prefF[L][i] = sum of C[r][c] where r >= i and r + c < L
    // prefG[L][j] = sum of C[r][c] where c >= j and r + c < L
    vector<int> prefF((q + 1) * RW, 0);
    vector<int> prefG((q + 1) * RW, 0);

    for (int L = 0; L <= q; ++L) {
        int baseL = L * RW;

        for (int i = q; i >= 0; --i) {
            int jmax = L - 1 - i;
            int val = 0;

            if (jmax >= 0) {
                if (jmax > q) jmax = q;
                val = rowPref[i * RW + (jmax + 1)];
            }

            int sum = val + prefF[baseL + i + 1];
            if (sum >= MOD) sum -= MOD;
            prefF[baseL + i] = sum;
        }

        for (int j = q; j >= 0; --j) {
            int imax = L - 1 - j;
            int val = 0;

            if (imax >= 0) {
                if (imax > q) imax = q;
                val = colPref[j * RW + (imax + 1)];
            }

            int sum = val + prefG[baseL + j + 1];
            if (sum >= MOD) sum -= MOD;
            prefG[baseL + j] = sum;
        }
    }

    string output;
    output.reserve((size_t)Q * 12);

    for (int qi = 0; qi < Q; ++qi) {
        int x, y, z;
        cin >> x >> y >> z;

        int A = 0, B = 0;

        // A: count of 'a' > x
        if (x < q) A = rowSuf[x + 1];

        // B: count of 'b' > y
        if (y < q) B = colSuf[y + 1];

        // C: count of 'c' > z  <=> i + j < q - z
        int L = q - z;

        int Ccnt = 0, AC = 0, BC = 0;

        if (L > 0) {
            if (L > q) {
                Ccnt = total;
                AC = A;
                BC = B;
            } else {
                int baseL = L * RW;

                Ccnt = prefF[baseL + 0];

                int xi = x + 1;
                if (xi < 0) xi = 0;
                if (xi > q + 1) xi = q + 1;
                AC = prefF[baseL + xi];

                int yj = y + 1;
                if (yj < 0) yj = 0;
                if (yj > q + 1) yj = q + 1;
                BC = prefG[baseL + yj];
            }
        }

        int AB = 0;
        if (x < q && y < q) {
            int xi = x + 1;
            int yj = y + 1;
            AB = suff[xi * W2 + yj];
        }

        int ans = total;

        ans -= A;
        if (ans < 0) ans += MOD;

        ans -= B;
        if (ans < 0) ans += MOD;

        ans -= Ccnt;
        if (ans < 0) ans += MOD;

        ans += AB;
        if (ans >= MOD) ans -= MOD;

        ans += AC;
        if (ans >= MOD) ans -= MOD;

        ans += BC;
        if (ans >= MOD) ans -= MOD;

        output.append(to_string(ans));
        output.push_back('\n');
    }

    cout << output;
    return 0;
}
