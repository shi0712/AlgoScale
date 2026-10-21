#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using i64 = long long;

static constexpr int MOD = 998244353;
static constexpr int G = 3;

int N;
u64 K;
vector<u64> A, B;
vector<int> fac, ifac;

int mod_pow(int a, int e) {
    long long r = 1, x = a;
    while (e) {
        if (e & 1) r = r * x % MOD;
        x = x * x % MOD;
        e >>= 1;
    }
    return (int)r;
}

void ntt(vector<int>& a, bool invert) {
    int n = (int)a.size();
    static vector<int> rev;
    static vector<int> roots{0, 1};

    if ((int)rev.size() != n) {
        int k = __builtin_ctz(n);
        rev.assign(n, 0);
        for (int i = 0; i < n; ++i)
            rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (k - 1));
    }
    if ((int)roots.size() < n) {
        int k = __builtin_ctz((int)roots.size());
        roots.resize(n);
        while ((1 << k) < n) {
            int e = mod_pow(G, (MOD - 1) >> (k + 1));
            for (int i = 1 << (k - 1); i < (1 << k); ++i) {
                roots[i << 1] = roots[i];
                roots[i << 1 | 1] = (long long)roots[i] * e % MOD;
            }
            ++k;
        }
    }
    for (int i = 0; i < n; ++i)
        if (i < rev[i]) swap(a[i], a[rev[i]]);

    for (int len = 1; len < n; len <<= 1) {
        for (int i = 0; i < n; i += len << 1) {
            for (int j = 0; j < len; ++j) {
                int u = a[i + j];
                int v = (long long)a[i + j + len] * roots[len + j] % MOD;
                int x = u + v;
                if (x >= MOD) x -= MOD;
                int y = u - v;
                if (y < 0) y += MOD;
                a[i + j] = x;
                a[i + j + len] = y;
            }
        }
    }

    if (invert) {
        reverse(a.begin() + 1, a.end());
        int inv_n = mod_pow(n, MOD - 2);
        for (int& x : a) x = (long long)x * inv_n % MOD;
    }
}

vector<int> convolution(const vector<int>& a, const vector<int>& b) {
    if (a.empty() || b.empty()) return {};
    if (a.size() == 1) {
        vector<int> c = b;
        for (int& x : c) x = (long long)x * a[0] % MOD;
        return c;
    }
    if (b.size() == 1) {
        vector<int> c = a;
        for (int& x : c) x = (long long)x * b[0] % MOD;
        return c;
    }
    if ((long long)a.size() * b.size() <= 4096) {
        vector<int> c(a.size() + b.size() - 1, 0);
        for (int i = 0; i < (int)a.size(); ++i) {
            for (int j = 0; j < (int)b.size(); ++j) {
                c[i + j] = (c[i + j] + (long long)a[i] * b[j]) % MOD;
            }
        }
        return c;
    }
    int need = (int)a.size() + (int)b.size() - 1;
    int sz = 1;
    while (sz < need) sz <<= 1;
    vector<int> x(sz), y(sz);
    copy(a.begin(), a.end(), x.begin());
    copy(b.begin(), b.end(), y.begin());
    ntt(x, false);
    ntt(y, false);
    for (int i = 0; i < sz; ++i) x[i] = (long long)x[i] * y[i] % MOD;
    ntt(x, true);
    x.resize(need);
    return x;
}

vector<int> full_board(int m, int n) {
    int s = min(m, n);
    vector<int> r(s + 1);
    // C(m,t) C(n,t) t! = m! n! / ((m-t)!(n-t)!t!)
    for (int t = 0; t <= s; ++t) {
        r[t] = (long long)fac[m] * fac[n] % MOD
             * ifac[m - t] % MOD * ifac[n - t] % MOD * ifac[t] % MOD;
    }
    return r;
}

vector<int> complement_poly(const vector<int>& r, int m, int n) {
    int s = min(m, n);
    int d = min(s, (int)r.size() - 1);
    if (d == 0 && r[0] == 1) return full_board(m, n);

    vector<int> x(d + 1), y(s + 1);
    for (int j = 0; j <= d; ++j) {
        long long v = (long long)r[j] * fac[m - j] % MOD * fac[n - j] % MOD;
        if (j & 1) v = v ? MOD - v : 0;
        x[j] = (int)v;
    }
    for (int j = 0; j <= s; ++j) y[j] = ifac[j];
    vector<int> z = convolution(x, y);
    vector<int> ans(s + 1);
    for (int t = 0; t <= s; ++t) {
        ans[t] = (long long)z[t] * ifac[m - t] % MOD * ifac[n - t] % MOD;
    }
    return ans;
}

int split_pos(const vector<u64>& v, int l, int r, int bit) {
    if (l == r) return l;
    u64 high = (v[l] >> (bit + 1)) << (bit + 1);
    u64 threshold = high | (1ULL << bit);
    return (int)(lower_bound(v.begin() + l, v.begin() + r, threshold) - v.begin());
}

// Returns the rook polynomial of either E = {xor >= K} (want_edge=true)
// or its complement (want_edge=false), restricted to A[al:ar), B[bl:br).
vector<int> solve(int al, int ar, int bl, int br, int bit, bool want_edge) {
    int m = ar - al, n = br - bl;
    if (m == 0 || n == 0) return {1};
    if (bit < 0) return want_edge ? full_board(m, n) : vector<int>{1};

    // If the remaining threshold bits are all zero, xor_suffix >= 0 is always true.
    u64 mask = (1ULL << (bit + 1)) - 1;
    if ((K & mask) == 0) return want_edge ? full_board(m, n) : vector<int>{1};

    int am = split_pos(A, al, ar, bit);
    int bm = split_pos(B, bl, br, bit);
    bool kb = (K >> bit) & 1ULL;

    // The easy object is edges when kb=1, non-edges when kb=0.
    bool easy_is_edge = kb;
    vector<int> p, q;
    if (kb) {
        p = solve(al, am, bm, br, bit - 1, true);   // A0 x B1
        q = solve(am, ar, bl, bm, bit - 1, true);   // A1 x B0
    } else {
        p = solve(al, am, bl, bm, bit - 1, false);  // A0 x B0
        q = solve(am, ar, bm, br, bit - 1, false);  // A1 x B1
    }
    vector<int> easy = convolution(p, q);
    if (want_edge == easy_is_edge) return easy;
    return complement_poly(easy, m, n);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N >> K;
    A.resize(N); B.resize(N);
    for (u64& x : A) cin >> x;
    for (u64& x : B) cin >> x;
    sort(A.begin(), A.end());
    sort(B.begin(), B.end());

    fac.assign(N + 1, 1);
    ifac.assign(N + 1, 1);
    for (int i = 1; i <= N; ++i) fac[i] = (long long)fac[i - 1] * i % MOD;
    ifac[N] = mod_pow(fac[N], MOD - 2);
    for (int i = N; i >= 1; --i) ifac[i - 1] = (long long)ifac[i] * i % MOD;

    vector<int> ans = solve(0, N, 0, N, 59, true);
    for (int t = 1; t <= N; ++t) {
        int v = t < (int)ans.size() ? ans[t] : 0;
        if (t > 1) cout << ' ';
        cout << v;
    }
    cout << '\n';
    return 0;
}
