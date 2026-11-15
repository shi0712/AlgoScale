#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int PRIM = 3;

static vector<int> INV;  // modular inverses 1..K

static inline int pw(long long a, long long e) {
    long long r = 1; a %= MOD;
    while (e) { if (e & 1) r = r * a % MOD; a = a * a % MOD; e >>= 1; }
    return (int)r;
}

void ntt(vector<int>& a, bool inv) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int wlen = pw(PRIM, (MOD - 1) / len);
        if (inv) wlen = pw(wlen, MOD - 2);
        int half = len >> 1;
        for (int i = 0; i < n; i += len) {
            long long w = 1;
            for (int j = 0; j < half; j++) {
                int u = a[i + j];
                int v = (int)(a[i + j + half] * w % MOD);
                int x = u + v; if (x >= MOD) x -= MOD;
                int y = u - v; if (y < 0) y += MOD;
                a[i + j] = x; a[i + j + half] = y;
                w = w * wlen % MOD;
            }
        }
    }
    if (inv) {
        int ninv = pw(n, MOD - 2);
        for (int& x : a) x = (int)((long long)x * ninv % MOD);
    }
}

vector<int> mul(const vector<int>& a, const vector<int>& b, int limit) {
    if (a.empty() || b.empty() || limit <= 0) return {};
    int na = (int)a.size(), nb = (int)b.size();
    int need = na + nb - 1;
    if (need > limit) need = limit;
    if (min(na, nb) <= 40) {
        vector<int> res(need, 0);
        for (int i = 0; i < na; i++) {
            if (a[i] == 0) continue;
            int jmax = min(nb, need - i);
            long long ai = a[i];
            for (int j = 0; j < jmax; j++)
                res[i + j] = (int)((res[i + j] + ai * b[j]) % MOD);
        }
        return res;
    }
    int sz = 1, full = na + nb - 1;
    while (sz < full) sz <<= 1;
    vector<int> fa(a.begin(), a.end()), fb(b.begin(), b.end());
    fa.resize(sz); fb.resize(sz);
    ntt(fa, false); ntt(fb, false);
    for (int i = 0; i < sz; i++) fa[i] = (int)((long long)fa[i] * fb[i] % MOD);
    ntt(fa, true);
    fa.resize(need);
    return fa;
}

vector<int> poly_inv(const vector<int>& a, int n) {
    vector<int> res(1, pw(a[0], MOD - 2));
    int m = 1;
    while (m < n) {
        int m2 = min(2 * m, n);
        vector<int> tmp(a.begin(), a.begin() + min((int)a.size(), m2));
        vector<int> t = mul(tmp, res, m2);
        for (int& x : t) if (x) x = MOD - x;
        t[0] += 2; if (t[0] >= MOD) t[0] -= MOD;
        res = mul(res, t, m2);
        res.resize(m2);
        m = m2;
    }
    return res;
}

vector<int> poly_log(const vector<int>& a, int n) {
    vector<int> der(a.size() > 1 ? a.size() - 1 : 0);
    for (int i = 1; i < (int)a.size(); i++)
        der[i - 1] = (int)((long long)a[i] * i % MOD);
    vector<int> inv = poly_inv(a, n);
    vector<int> q = mul(der, inv, n > 1 ? n - 1 : 0);
    vector<int> res(n, 0);
    for (int i = 0; i < (int)q.size() && i + 1 < n; i++)
        res[i + 1] = (int)((long long)q[i] * (i + 1 <= (int)INV.size() - 1 ? INV[i + 1] : pw(i + 1, MOD - 2)) % MOD);
    return res;
}

vector<int> poly_exp(const vector<int>& f, int n) {
    vector<int> res(1, 1);
    int m = 1;
    while (m < n) {
        int m2 = min(2 * m, n);
        vector<int> lg = poly_log(res, m2);
        vector<int> t(m2, 0);
        for (int i = 0; i < m2; i++) {
            int fi = (i < (int)f.size() ? f[i] : 0);
            int v = fi - lg[i]; if (v < 0) v += MOD;
            t[i] = v;
        }
        t[0] += 1; if (t[0] >= MOD) t[0] -= MOD;
        res = mul(res, t, m2);
        res.resize(m2);
        m = m2;
    }
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    vector<int> a(n);
    long long m = 0;
    int maxa = 0;
    for (int i = 0; i < n; i++) {
        cin >> a[i];
        m += a[i];
        if (a[i] > maxa) maxa = a[i];
    }
    if (m == 0) { cout << 0 << "\n"; return 0; }
    int K = (int)(m / 2);

    INV.assign(K + 2, 1);
    for (int i = 2; i <= K + 1; i++)
        INV[i] = (int)((long long)(MOD - MOD / i) * INV[MOD % i] % MOD);

    // cntB[b] = number of values v with a_v + 1 == b
    vector<int> cntB(maxa + 2, 0);
    for (int i = 0; i < n; i++) cntB[a[i] + 1]++;
    vector<int>().swap(a);

    // S[d] = sum_{v: b_v | d} b_v
    vector<int> S(K + 1, 0);
    int limB = min(maxa + 1, K);
    for (int b = 1; b <= limB; b++) {
        if (cntB[b] == 0) continue;
        int val = (int)((long long)cntB[b] * b % MOD);
        for (int d = b; d <= K; d += b) {
            int x = S[d] + val; if (x >= MOD) x -= MOD;
            S[d] = x;
        }
    }
    vector<int>().swap(cntB);

    // g[d] = (n - S[d]) / d
    vector<int> g(K + 1, 0);
    int nm = (int)(n % MOD);
    for (int d = 1; d <= K; d++) {
        int num = nm - S[d]; if (num < 0) num += MOD;
        g[d] = (int)((long long)num * INV[d] % MOD);
    }
    vector<int>().swap(S);

    vector<int> Ncoef = (K == 0) ? vector<int>{1} : poly_exp(g, K + 1);
    vector<int>().swap(g);

    // Answer = sum_{k=0}^{K} (C(m,k)-C(m,k-1)) * (N(k)-N(k-1))
    long long ans = 0;
    int Ck = 1;      // C(m,0)
    int Cprev = 0;   // C(m,-1)
    int Nprev = 0;   // N(-1)
    for (int k = 0; k <= K; k++) {
        if (k > 0) {
            // C(m,k) = C(m,k-1) * (m-k+1) / k
            Ck = (int)((long long)Cprev * ((m - k + 1) % MOD) % MOD * INV[k] % MOD);
        }
        int Nk = Ncoef[k];
        long long d1 = Ck - Cprev; if (d1 < 0) d1 += MOD;
        long long d2 = Nk - Nprev; if (d2 < 0) d2 += MOD;
        ans += d1 * d2 % MOD;
        Cprev = Ck; Nprev = Nk;
    }
    cout << ans % MOD << "\n";
    return 0;
}
