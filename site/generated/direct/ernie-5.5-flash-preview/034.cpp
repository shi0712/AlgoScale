#include <bits/stdc++.h>
using namespace std;

const int MOD = 998244353;
const int MAXN = 100000;
const int ROOT = 3;

int fact[MAXN + 1], invfact[MAXN + 1], inv[MAXN + 1];
unsigned long long k;

int powmod(int a, int b) {
    int res = 1;
    while (b) {
        if (b & 1) res = (long long)res * a % MOD;
        a = (long long)a * a % MOD;
        b >>= 1;
    }
    return res;
}

void ntt(vector<int>& a, bool invert) {
    int n = a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int wlen = powmod(ROOT, (MOD - 1) / len);
        if (invert) wlen = powmod(wlen, MOD - 2);
        for (int i = 0; i < n; i += len) {
            int w = 1;
            for (int j = 0; j < len / 2; ++j) {
                int u = a[i + j];
                int v = (long long)a[i + j + len / 2] * w % MOD;
                a[i + j] = (u + v) % MOD;
                a[i + j + len / 2] = (u - v + MOD) % MOD;
                w = (long long)w * wlen % MOD;
            }
        }
    }
    if (invert) {
        int inv_n = powmod(n, MOD - 2);
        for (int& x : a) x = (long long)x * inv_n % MOD;
    }
}

vector<int> convolution(const vector<int>& a, const vector<int>& b) {
    if (a.empty() || b.empty()) return {};
    int na = a.size(), nb = b.size();
    int n = 1;
    while (n < na + nb - 1) n <<= 1;
    vector<int> fa(a.begin(), a.end()), fb(b.begin(), b.end());
    fa.resize(n);
    fb.resize(n);
    ntt(fa, false);
    ntt(fb, false);
    for (int i = 0; i < n; ++i) fa[i] = (long long)fa[i] * fb[i] % MOD;
    ntt(fa, true);
    fa.resize(na + nb - 1);
    return fa;
}

vector<int> multiply(const vector<int>& a, const vector<int>& b) {
    if (a.empty() || b.empty()) return {};
    if (a.size() == 1) {
        if (a[0] == 0) return {};
        vector<int> res(b.size());
        for (int i = 0; i < (int)b.size(); ++i)
            res[i] = (long long)a[0] * b[i] % MOD;
        return res;
    }
    if (b.size() == 1) {
        if (b[0] == 0) return {};
        vector<int> res(a.size());
        for (int i = 0; i < (int)a.size(); ++i)
            res[i] = (long long)a[i] * b[0] % MOD;
        return res;
    }
    int na = a.size(), nb = b.size();
    if ((long long)na * nb <= 10000) {
        vector<int> res(na + nb - 1, 0);
        for (int i = 0; i < na; ++i) {
            if (a[i] == 0) continue;
            for (int j = 0; j < nb; ++j) {
                if (b[j] == 0) continue;
                res[i + j] = (res[i + j] + (long long)a[i] * b[j]) % MOD;
            }
        }
        return res;
    }
    return convolution(a, b);
}

vector<int> complete_poly(int p, int q) {
    int s = min(p, q);
    vector<int> res(s + 1);
    res[0] = 1;
    for (int t = 0; t < s; ++t) {
        res[t + 1] = (long long)res[t] * (p - t) % MOD * (q - t) % MOD * inv[t + 1] % MOD;
    }
    return res;
}

vector<int> solve(vector<long long> A, vector<long long> B, int d, bool need_allowed) {
    int p = A.size(), q = B.size();
    if (p == 0 || q == 0) return {1};

    unsigned long long low_mask = (d >= 0 ? (1ULL << (d + 1)) - 1 : 0);
    if ((k & low_mask) == 0) {
        if (need_allowed) return complete_poly(p, q);
        else return {1};
    }

    int bit = (k >> d) & 1;
    bool easy_allowed = (bit == 1);

    vector<long long> A0, A1, B0, B1;
    A0.reserve(p); A1.reserve(p);
    B0.reserve(q); B1.reserve(q);
    for (long long x : A) {
        if ((x >> d) & 1) A1.push_back(x);
        else A0.push_back(x);
    }
    for (long long x : B) {
        if ((x >> d) & 1) B1.push_back(x);
        else B0.push_back(x);
    }
    vector<long long>().swap(A);
    vector<long long>().swap(B);

    vector<int> easy_poly;
    if (easy_allowed) {
        vector<int> left = solve(std::move(A0), std::move(B1), d - 1, true);
        vector<int> right = solve(std::move(A1), std::move(B0), d - 1, true);
        easy_poly = multiply(left, right);
    } else {
        vector<int> left = solve(std::move(A0), std::move(B0), d - 1, false);
        vector<int> right = solve(std::move(A1), std::move(B1), d - 1, false);
        easy_poly = multiply(left, right);
    }

    if (need_allowed == easy_allowed) {
        return easy_poly;
    }

    int s = min(p, q);
    if ((int)easy_poly.size() < s + 1) easy_poly.resize(s + 1, 0);

    if (need_allowed) {
        // forbidden -> allowed
        vector<int> Acoef(s + 1), Bcoef(s + 1);
        for (int t = 0; t <= s; ++t) {
            long long val = easy_poly[t];
            val = val * fact[p - t] % MOD * fact[q - t] % MOD;
            if (t & 1) val = MOD - val;
            Acoef[t] = val;
        }
        for (int y = 0; y <= s; ++y) Bcoef[y] = invfact[y];
        vector<int> C = multiply(Acoef, Bcoef);
        C.resize(s + 1);
        vector<int> res(s + 1);
        for (int x = 0; x <= s; ++x) {
            res[x] = (long long)C[x] * invfact[p - x] % MOD * invfact[q - x] % MOD;
        }
        return res;
    } else {
        // allowed -> forbidden
        vector<int> C(s + 1), D(s + 1);
        for (int x = 0; x <= s; ++x) {
            C[x] = (long long)easy_poly[x] * fact[p - x] % MOD * fact[q - x] % MOD;
        }
        for (int y = 0; y <= s; ++y) {
            D[y] = invfact[y];
            if (y & 1) D[y] = MOD - D[y];
        }
        vector<int> Acoef = multiply(C, D);
        Acoef.resize(s + 1);
        vector<int> res(s + 1);
        for (int t = 0; t <= s; ++t) {
            long long val = (long long)Acoef[t] * invfact[p - t] % MOD * invfact[q - t] % MOD;
            if (t & 1) val = MOD - val;
            res[t] = val;
        }
        return res;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n >> k;
    vector<long long> A(n), B(n);
    for (int i = 0; i < n; ++i) cin >> A[i];
    for (int i = 0; i < n; ++i) cin >> B[i];

    fact[0] = 1;
    for (int i = 1; i <= MAXN; ++i) fact[i] = (long long)fact[i - 1] * i % MOD;
    inv[1] = 1;
    for (int i = 2; i <= MAXN; ++i) inv[i] = MOD - (long long)(MOD / i) * inv[MOD % i] % MOD;
    invfact[MAXN] = powmod(fact[MAXN], MOD - 2);
    for (int i = MAXN - 1; i >= 0; --i) invfact[i] = (long long)invfact[i + 1] * (i + 1) % MOD;

    vector<int> res = solve(std::move(A), std::move(B), 59, true);
    res.resize(n + 1, 0);

    for (int i = 1; i <= n; ++i) {
        cout << res[i] << (i == n ? '\n' : ' ');
    }

    return 0;
}
