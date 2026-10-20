#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int G = 3;

int modpow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

int rootPow[24], invRootPow[24], invLenPow[24];

void initNTT() {
    invLenPow[0] = 1;
    for (int k = 1; k <= 23; k++) {
        int len = 1 << k;
        rootPow[k] = modpow(G, (MOD - 1) / len);
        invRootPow[k] = modpow(rootPow[k], MOD - 2);
        invLenPow[k] = modpow(len, MOD - 2);
    }
}

void ntt(vector<int>& a, bool invert) {
    int n = (int)a.size();

    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    int k = 1;
    for (int len = 2; len <= n; len <<= 1, k++) {
        int wlen = invert ? invRootPow[k] : rootPow[k];
        int half = len >> 1;

        for (int i = 0; i < n; i += len) {
            long long w = 1;
            for (int j = 0; j < half; j++) {
                int u = a[i + j];
                int v = (long long)a[i + j + half] * w % MOD;

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
        int inv_n = invLenPow[__builtin_ctz((unsigned)n)];
        for (int& x : a) {
            x = (long long)x * inv_n % MOD;
        }
    }
}

vector<int> convolution(const vector<int>& a, const vector<int>& b, int need = -1) {
    if (a.empty() || b.empty() || need == 0) return {};

    int n1 = (int)a.size();
    int n2 = (int)b.size();

    if (need != -1) {
        n1 = min(n1, need);
        n2 = min(n2, need);
    }

    if (n1 <= 0 || n2 <= 0) return {};

    int full = n1 + n2 - 1;
    int outlen = (need == -1 ? full : min(need, full));

    if (1LL * n1 * n2 <= 4096 || min(n1, n2) <= 32) {
        vector<int> c(outlen, 0);
        for (int i = 0; i < n1; i++) {
            if (i >= outlen) break;
            int maxj = min(n2, outlen - i);
            for (int j = 0; j < maxj; j++) {
                c[i + j] = (c[i + j] + (long long)a[i] * b[j]) % MOD;
            }
        }
        return c;
    }

    int sz = 1;
    while (sz < full) sz <<= 1;

    vector<int> fa(sz, 0), fb(sz, 0);
    for (int i = 0; i < n1; i++) fa[i] = a[i];
    for (int i = 0; i < n2; i++) fb[i] = b[i];

    ntt(fa, false);
    ntt(fb, false);

    for (int i = 0; i < sz; i++) {
        fa[i] = (long long)fa[i] * fb[i] % MOD;
    }

    ntt(fa, true);
    fa.resize(outlen);
    return fa;
}

vector<int> INVINT;

// returns exp(f) modulo x^n, assuming f[0] = 0
vector<int> fps_exp(const vector<int>& f) {
    int n = (int)f.size();
    if (n == 0) return {};

    vector<int> b(1, 1);
    vector<int> invb(1, 1);

    int cur = 1;

    while (cur < n) {
        int m = min(cur * 2, n);
        int high = m - cur;

        // Extend inverse of b (old, padded with zeros) to length m
        vector<int> inv_ext(m, 0);
        for (int i = 0; i < cur; i++) inv_ext[i] = invb[i];

        if (high > 0) {
            vector<int> prod = convolution(b, invb, m);
            prod.resize(m, 0);

            vector<int> qhigh(high, 0);
            for (int i = 0; i < high; i++) {
                int val = prod[cur + i];
                qhigh[i] = (val == 0 ? 0 : MOD - val);
            }

            vector<int> corr = convolution(invb, qhigh, high);
            corr.resize(high, 0);

            for (int i = 0; i < high; i++) {
                inv_ext[cur + i] = corr[i];
            }
        }

        // Compute log(b) coefficients at degrees cur ... m-1
        vector<int> log_high(high, 0);

        if (cur > 1) {
            vector<int> der(cur - 1);
            for (int i = 1; i < cur; i++) {
                der[i - 1] = (long long)i * b[i] % MOD;
            }

            int need = m - 1;
            vector<int> c1 = convolution(der, invb, need);

            vector<int> inv_high(high);
            for (int i = 0; i < high; i++) inv_high[i] = inv_ext[cur + i];

            int need2 = max(0, need - cur);
            vector<int> c2;
            if (need2 > 0 && high > 0) {
                c2 = convolution(der, inv_high, need2);
            }

            for (int i = 0; i < high; i++) {
                int idx = cur + i;
                int t = idx - 1;

                int coeff = 0;
                if (t >= 0 && t < (int)c1.size()) coeff += c1[t];
                if (t >= cur) {
                    int t2 = t - cur;
                    if (t2 < (int)c2.size()) {
                        coeff += c2[t2];
                        if (coeff >= MOD) coeff -= MOD;
                    }
                }
                if (coeff >= MOD) coeff -= MOD;

                log_high[i] = (long long)coeff * INVINT[idx] % MOD;
            }
        }

        // q = 1 - log(b) + f, write q = 1 + x^cur * qhigh
        vector<int> qhigh(high, 0);
        for (int i = 0; i < high; i++) {
            int idx = cur + i;
            int val = f[idx] - log_high[i];
            if (val < 0) val += MOD;
            qhigh[i] = val;
        }

        // b_new = b * q
        vector<int> bnew(m, 0);
        for (int i = 0; i < cur; i++) bnew[i] = b[i];

        if (high > 0) {
            vector<int> conv_bq = convolution(b, qhigh, high);
            conv_bq.resize(high, 0);

            for (int i = 0; i < high; i++) {
                bnew[cur + i] = conv_bq[i];
            }
        }

        // inv_new = inverse(b_new) modulo x^m
        vector<int> invnew(m, 0);
        for (int i = 0; i < cur; i++) invnew[i] = invb[i];

        if (high > 0) {
            vector<int> conv_iq = convolution(invb, qhigh, high);
            conv_iq.resize(high, 0);

            for (int i = 0; i < high; i++) {
                int val = inv_ext[cur + i] - conv_iq[i];
                if (val < 0) val += MOD;
                invnew[cur + i] = val;
            }
        }

        b.swap(bnew);
        invb.swap(invnew);
        cur = m;
    }

    b.resize(n);
    return b;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    initNTT();

    int n;
    cin >> n;

    vector<int> a(n);
    int m = 0;

    for (int i = 0; i < n; i++) {
        cin >> a[i];
        m += a[i];
    }

    int H = m / 2;

    // Precompute modular inverses up to H + 1
    int invSize = max(H + 2, 2);
    INVINT.assign(invSize, 0);
    INVINT[1] = 1;
    for (int i = 2; i < invSize; i++) {
        INVINT[i] = MOD - (long long)(MOD / i) * INVINT[MOD % i] % MOD;
    }

    int positiveCount = 0;
    vector<int> freq(H + 1, 0);

    for (int x : a) {
        if (x > 0) {
            positiveCount++;
            int e = x + 1;
            if (e <= H) freq[e]++;
        }
    }

    vector<int> sumE(H + 1, 0);

    for (int e = 2; e <= H; e++) {
        if (freq[e] == 0) continue;

        int add = (long long)freq[e] * e % MOD;

        for (int d = e; d <= H; d += e) {
            sumE[d] += add;
            if (sumE[d] >= MOD) sumE[d] -= MOD;
        }
    }

    vector<int> fLog(H + 1, 0);

    for (int d = 1; d <= H; d++) {
        int val = positiveCount - sumE[d];
        if (val < 0) val += MOD;

        fLog[d] = (long long)val * INVINT[d] % MOD;
    }

    // N[k] = coefficient of x^k in product_i (1 + x + ... + x^{a_i})
    vector<int> Ncoef = fps_exp(fLog);
    Ncoef.resize(H + 1, 0);

    int ans = 0;

    int comb = 1;      // C(m, 0)
    int prevComb = 0;  // C(m, -1)
    int prevN = 0;     // N[-1]

    for (int k = 0; k <= H; k++) {
        int fk = comb - prevComb;
        if (fk < 0) fk += MOD;

        int d = Ncoef[k] - prevN;
        if (d < 0) d += MOD;

        ans += (long long)fk * d % MOD;
        if (ans >= MOD) ans -= MOD;

        prevComb = comb;
        prevN = Ncoef[k];

        if (k < H) {
            comb = (long long)comb * (m - k) % MOD * INVINT[k + 1] % MOD;
        }
    }

    cout << ans << '\n';
    return 0;
}
