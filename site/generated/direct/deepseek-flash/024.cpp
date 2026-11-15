#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int G = 3;

int mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

void ntt(vector<int>& a, bool invert) {
    int n = (int)a.size();

    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    static const int INV_G = mod_pow(G, MOD - 2);

    for (int len = 2; len <= n; len <<= 1) {
        int wlen = mod_pow(invert ? INV_G : G, (MOD - 1) / len);
        int half = len >> 1;

        for (int i = 0; i < n; i += len) {
            long long w = 1;
            for (int j = 0; j < half; ++j) {
                int u = a[i + j];
                int v = (int)(w * a[i + j + half] % MOD);

                int x = u + v;
                if (x >= MOD) x -= MOD;

                int y = u - v;
                if (y < 0) y += MOD;

                a[i + j] = x;
                a[i + j + half] = y;

                w = w * wlen % MOD;
            }
        }
    }

    if (invert) {
        int inv_n = mod_pow(n, MOD - 2);
        for (int& x : a) {
            x = (int)(1LL * x * inv_n % MOD);
        }
    }
}

// Computes exp(f) modulo x^n.
// f[0] must be 0, n must be a power of two.
// invs[i] must contain modular inverse of i for 1 <= i < n.
vector<int> fps_exp(const vector<int>& f, const vector<int>& invs) {
    int n = (int)f.size();
    vector<int> g(1, 1);
    vector<int> invg(1, 1);

    for (int m = 1; m < n; m <<= 1) {
        int len = m << 1;

        vector<int> fg(len, 0), fi(len, 0);
        for (int i = 0; i < m; ++i) {
            fg[i] = g[i];
            fi[i] = invg[i];
        }

        ntt(fg, false);
        ntt(fi, false);

        vector<int> e(len);
        for (int i = 0; i < len; ++i) {
            e[i] = (int)(1LL * fg[i] * fi[i] % MOD);
        }
        ntt(e, true);

        vector<int> eHigh(m);
        for (int j = 0; j < m; ++j) {
            eHigh[j] = e[m + j];
        }

        vector<int> fe(len, 0);
        for (int j = 0; j < m; ++j) {
            fe[j] = eHigh[j];
        }
        ntt(fe, false);

        vector<int> hc(len);
        for (int i = 0; i < len; ++i) {
            hc[i] = (int)(1LL * fi[i] * fe[i] % MOD);
        }
        ntt(hc, true);

        vector<int> hHigh(m);
        for (int j = 0; j < m; ++j) {
            hHigh[j] = hc[j] == 0 ? 0 : MOD - hc[j];
        }

        vector<int> invgExt(len, 0);
        for (int i = 0; i < m; ++i) invgExt[i] = invg[i];
        for (int j = 0; j < m; ++j) invgExt[m + j] = hHigh[j];

        vector<int> fd(len, 0);
        for (int i = 1; i < m; ++i) {
            fd[i - 1] = (int)(1LL * i * g[i] % MOD);
        }

        vector<int> fve = invgExt;
        ntt(fd, false);
        ntt(fve, false);

        vector<int> logprod(len);
        for (int i = 0; i < len; ++i) {
            logprod[i] = (int)(1LL * fd[i] * fve[i] % MOD);
        }
        ntt(logprod, true);

        vector<int> cHigh(m);
        for (int j = 0; j < m; ++j) {
            int k = m + j;
            int coeff = logprod[k - 1];
            int logcoef = (int)(1LL * coeff * invs[k] % MOD);

            int val = f[k] - logcoef;
            if (val < 0) val += MOD;
            cHigh[j] = val;
        }

        vector<int> fc(len, 0);
        for (int j = 0; j < m; ++j) fc[j] = cHigh[j];
        ntt(fc, false);

        vector<int> gh(len), invc(len);
        for (int i = 0; i < len; ++i) {
            gh[i] = (int)(1LL * fg[i] * fc[i] % MOD);
            invc[i] = (int)(1LL * fi[i] * fc[i] % MOD);
        }

        ntt(gh, true);
        ntt(invc, true);

        vector<int> ng(len, 0), ninv(len, 0);

        for (int i = 0; i < m; ++i) {
            ng[i] = g[i];
            ninv[i] = invg[i];
        }

        for (int j = 0; j < m; ++j) {
            ng[m + j] = gh[j];

            int val = hHigh[j] - invc[j];
            if (val < 0) val += MOD;
            ninv[m + j] = val;
        }

        g.swap(ng);
        invg.swap(ninv);
    }

    return g;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    int m = 0;
    int positive = 0;

    vector<int> a(n);
    for (int i = 0; i < n; ++i) {
        cin >> a[i];
        m += a[i];
        if (a[i] > 0) ++positive;
    }

    int M = m / 2;

    int sz = 1;
    while (sz < M + 1) sz <<= 1;

    vector<int> inv(sz, 0);
    if (sz > 1) {
        inv[1] = 1;
        for (int i = 2; i < sz; ++i) {
            inv[i] = MOD - (int)(1LL * (MOD / i) * inv[MOD % i] % MOD);
        }
    }

    vector<int> cntB(sz, 0);

    for (int x : a) {
        if (x > 0) {
            int b = x + 1;
            if (b < sz) ++cntB[b];
        }
    }

    vector<int> sumDiv(sz, 0);

    for (int b = 2; b < sz; ++b) {
        if (cntB[b] == 0) continue;

        int add = (int)(1LL * cntB[b] * b % MOD);

        for (int d = b; d < sz; d += b) {
            sumDiv[d] += add;
            if (sumDiv[d] >= MOD) sumDiv[d] -= MOD;
        }
    }

    vector<int> f(sz, 0);

    for (int d = 1; d < sz; ++d) {
        int val = positive - sumDiv[d];
        val %= MOD;
        if (val < 0) val += MOD;

        f[d] = (int)(1LL * val * inv[d] % MOD);
    }

    vector<int> P = fps_exp(f, inv);

    long long ans = 0;

    int Bcur = 1;
    int Bprev = 0;
    int Cprev = 0;

    for (int R = 0; R <= M; ++R) {
        if (R > 0) {
            Bcur = (int)(1LL * Bcur * (m - R + 1) % MOD * inv[R] % MOD);
        }

        int Ccur = P[R];

        int diffB = Bcur - Bprev;
        if (diffB < 0) diffB += MOD;

        int diffC = Ccur - Cprev;
        if (diffC < 0) diffC += MOD;

        ans += 1LL * diffB * diffC % MOD;
        ans %= MOD;

        Bprev = Bcur;
        Cprev = Ccur;
    }

    cout << ans % MOD << '\n';
    return 0;
}
