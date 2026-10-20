#include <bits/stdc++.h>
using namespace std;

static const int MOD = 1000000007;

int mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n;
        cin >> n;

        vector<int> l(n), r(n), lenMod(n);
        vector<int> Lr(n), Rr(n);

        for (int i = 0; i < n; ++i) cin >> l[i];
        for (int i = 0; i < n; ++i) cin >> r[i];

        int K = 0;
        long long totalWays = 1;

        for (int i = 0; i < n; ++i) {
            long long len = (long long)r[i] - l[i] + 1;
            lenMod[i] = len % MOD;
            totalWays = totalWays * (len % MOD) % MOD;
            K = max(K, r[i]);
        }

        int maxN = n + 5;
        vector<int> fact(maxN + 1), invfact(maxN + 1);
        fact[0] = 1;
        for (int i = 1; i <= maxN; ++i) fact[i] = 1LL * fact[i - 1] * i % MOD;
        invfact[maxN] = mod_pow(fact[maxN], MOD - 2);
        for (int i = maxN; i >= 1; --i) invfact[i - 1] = 1LL * invfact[i] * i % MOD;

        vector<int> coord;
        coord.reserve(2 * n + 2);
        coord.push_back(1);
        coord.push_back(K + 1);

        for (int i = 0; i < n; ++i) {
            int a = l[i] + 1;
            int b = r[i] + 1;
            if (a <= K + 1) coord.push_back(a);
            if (b <= K + 1) coord.push_back(b);
        }

        sort(coord.begin(), coord.end());
        coord.erase(unique(coord.begin(), coord.end()), coord.end());

        vector<char> st(n);
        vector<int> va, vb, vc, vals, qcum;
        vector<int> pref(n + 10), suff(n + 10);

        auto lagrange_consecutive = [&](const vector<int>& yval, int D, long long x) -> int {
            // yval[i] = Q(i), i=0..D, degree <= D
            if (0 <= x && x <= D) return yval[(int)x];

            int xm = (int)(x % MOD);
            pref[0] = 1;
            for (int i = 0; i <= D; ++i) {
                int cur = xm - i;
                if (cur < 0) cur += MOD;
                pref[i + 1] = 1LL * pref[i] * cur % MOD;
            }

            suff[D + 1] = 1;
            for (int i = D; i >= 0; --i) {
                int cur = xm - i;
                if (cur < 0) cur += MOD;
                suff[i] = 1LL * suff[i + 1] * cur % MOD;
            }

            int ans = 0;
            for (int i = 0; i <= D; ++i) {
                int num = 1LL * pref[i] * suff[i + 1] % MOD;
                int term = 1LL * yval[i] * num % MOD;
                term = 1LL * term * invfact[i] % MOD;
                term = 1LL * term * invfact[D - i] % MOD;

                if ((D - i) & 1) {
                    ans -= term;
                    if (ans < 0) ans += MOD;
                } else {
                    ans += term;
                    if (ans >= MOD) ans -= MOD;
                }
            }
            return ans;
        };

        int sumF = 0;

        for (int ci = 0; ci + 1 < (int)coord.size(); ++ci) {
            int L = coord[ci];
            int R = coord[ci + 1] - 1;
            if (L > R) continue;

            int segLen = R - L + 1;
            int d = 0;

            for (int i = 0; i < n; ++i) {
                if (L <= l[i]) {
                    st[i] = 0; // forced high
                } else if (L <= r[i]) {
                    st[i] = 1; // variable
                    ++d;
                } else {
                    st[i] = 2; // forced low
                }
            }

            if (d == 0) {
                int mat[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};

                for (int i = 0; i < n; ++i) {
                    int old0 = mat[0], old1 = mat[1], old2 = mat[2];
                    int old3 = mat[3], old4 = mat[4], old5 = mat[5];
                    int old6 = mat[6], old7 = mat[7], old8 = mat[8];

                    if (st[i] == 0) {
                        int h = lenMod[i];
                        mat[0] = mat[1] = mat[2] = 0;
                        mat[3] = 1LL * h * old0 % MOD;
                        mat[4] = 1LL * h * old1 % MOD;
                        mat[5] = 1LL * h * old2 % MOD;
                        mat[6] = mat[7] = mat[8] = 0;
                    } else {
                        int c = lenMod[i];
                        mat[0] = 1LL * c * ((old0 + old6) % MOD) % MOD;
                        mat[1] = 1LL * c * ((old1 + old7) % MOD) % MOD;
                        mat[2] = 1LL * c * ((old2 + old8) % MOD) % MOD;
                        mat[3] = mat[4] = mat[5] = 0;
                        mat[6] = 1LL * c * old3 % MOD;
                        mat[7] = 1LL * c * old4 % MOD;
                        mat[8] = 1LL * c * old5 % MOD;
                    }
                }

                int F = mat[0] + mat[3] + mat[6];
                if (F >= MOD) F %= MOD;
                sumF += 1LL * F * (segLen % MOD) % MOD;
                if (sumF >= MOD) sumF -= MOD;
                continue;
            }

            int q = min(segLen, d + 1);

            va.assign(q, 0);
            vb.assign(q, 0);
            vc.assign(q, 0);
            for (int j = 0; j < q; ++j) va[j] = 1;

            int pos = 0;
            while (pos < n) {
                if (st[pos] == 1) {
                    int baseC = L - l[pos];
                    int baseH = r[pos] + 1 - L;

                    for (int j = 0; j < q; ++j) {
                        int oldA = va[j], oldB = vb[j], oldC = vc[j];

                        int cc = baseC + j;
                        int hh = baseH - j;
                        if (cc >= MOD) cc %= MOD;
                        if (hh < 0) hh += MOD;
                        else if (hh >= MOD) hh %= MOD;

                        int sumAC = oldA + oldC;
                        if (sumAC >= MOD) sumAC -= MOD;

                        va[j] = 1LL * cc * sumAC % MOD;
                        vb[j] = 1LL * hh * oldA % MOD;
                        vc[j] = 1LL * cc * oldB % MOD;
                    }
                    ++pos;
                } else {
                    int mat[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};

                    while (pos < n && st[pos] != 1) {
                        int old0 = mat[0], old1 = mat[1], old2 = mat[2];
                        int old3 = mat[3], old4 = mat[4], old5 = mat[5];
                        int old6 = mat[6], old7 = mat[7], old8 = mat[8];

                        if (st[pos] == 0) {
                            int h = lenMod[pos];
                            mat[0] = mat[1] = mat[2] = 0;
                            mat[3] = 1LL * h * old0 % MOD;
                            mat[4] = 1LL * h * old1 % MOD;
                            mat[5] = 1LL * h * old2 % MOD;
                            mat[6] = mat[7] = mat[8] = 0;
                        } else {
                            int c = lenMod[pos];
                            mat[0] = 1LL * c * ((old0 + old6) % MOD) % MOD;
                            mat[1] = 1LL * c * ((old1 + old7) % MOD) % MOD;
                            mat[2] = 1LL * c * ((old2 + old8) % MOD) % MOD;
                            mat[3] = mat[4] = mat[5] = 0;
                            mat[6] = 1LL * c * old3 % MOD;
                            mat[7] = 1LL * c * old4 % MOD;
                            mat[8] = 1LL * c * old5 % MOD;
                        }
                        ++pos;
                    }

                    for (int j = 0; j < q; ++j) {
                        int oldA = va[j], oldB = vb[j], oldC = vc[j];

                        va[j] = (1LL * mat[0] * oldA + 1LL * mat[1] * oldB + 1LL * mat[2] * oldC) % MOD;
                        vb[j] = (1LL * mat[3] * oldA + 1LL * mat[4] * oldB + 1LL * mat[5] * oldC) % MOD;
                        vc[j] = (1LL * mat[6] * oldA + 1LL * mat[7] * oldB + 1LL * mat[8] * oldC) % MOD;
                    }
                }
            }

            vals.resize(q);
            for (int j = 0; j < q; ++j) {
                int cur = va[j] + vb[j];
                if (cur >= MOD) cur -= MOD;
                cur += vc[j];
                if (cur >= MOD) cur -= MOD;
                vals[j] = cur;
            }

            if (q == segLen) {
                int add = 0;
                for (int x : vals) {
                    add += x;
                    if (add >= MOD) add -= MOD;
                }
                sumF += add;
                if (sumF >= MOD) sumF -= MOD;
            } else {
                int D = q; // d + 1
                qcum.resize(D + 1);
                qcum[0] = 0;
                for (int k = 1; k <= D; ++k) {
                    qcum[k] = qcum[k - 1] + vals[k - 1];
                    if (qcum[k] >= MOD) qcum[k] -= MOD;
                }

                long long y = (long long)R - L + 1;
                int add = lagrange_consecutive(qcum, D, y);
                sumF += add;
                if (sumF >= MOD) sumF -= MOD;
            }
        }

        int ans = (1LL * (K % MOD) * totalWays % MOD - sumF + MOD) % MOD;
        cout << ans << '\n';
    }

    return 0;
}
