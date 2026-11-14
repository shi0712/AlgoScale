#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int ROOT = 3;

int mod_pow(int a, int e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = 1LL * a * a % MOD;
        e >>= 1;
    }
    return int(r);
}

void ntt(vector<int>& a, bool invert) {
    int n = int(a.size());
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int wlen = mod_pow(ROOT, (MOD - 1) / len);
        if (invert) wlen = mod_pow(wlen, MOD - 2);
        for (int i = 0; i < n; i += len) {
            int w = 1;
            for (int j = 0; j < len / 2; ++j) {
                int u = a[i + j];
                int v = int(1LL * a[i + j + len / 2] * w % MOD);
                int x = u + v;
                if (x >= MOD) x -= MOD;
                int y = u - v;
                if (y < 0) y += MOD;
                a[i + j] = x;
                a[i + j + len / 2] = y;
                w = int(1LL * w * wlen % MOD);
            }
        }
    }
    if (invert) {
        int inv_n = mod_pow(n, MOD - 2);
        for (int& x : a) x = int(1LL * x * inv_n % MOD);
    }
}

vector<int> fact_values, invfact_values;
array<vector<int>, 20> invfact_ntt_cache;

vector<int> multiply_small(const vector<int>& a, const vector<int>& b) {
    int n = int(a.size()), m = int(b.size());
    vector<int> c(n + m - 1, 0);
    for (int s = 0; s < n + m - 1; ++s) {
        int lo = max(0, s - m + 1);
        int hi = min(n - 1, s);
        long long sum = 0;
        int count = 0;
        for (int i = lo; i <= hi; ++i) {
            sum += 1LL * a[i] * b[s - i];
            if ((++count & 7) == 0) sum %= MOD;
        }
        c[s] = int(sum % MOD);
    }
    return c;
}

vector<int> multiply(const vector<int>& a, const vector<int>& b) {
    if (a.size() == 1) {
        if (a[0] == 1) return b;
        vector<int> c = b;
        for (int& x : c) x = int(1LL * x * a[0] % MOD);
        return c;
    }
    if (b.size() == 1) {
        if (b[0] == 1) return a;
        vector<int> c = a;
        for (int& x : c) x = int(1LL * x * b[0] % MOD);
        return c;
    }
    int needed = int(a.size() + b.size() - 1);
    if (min(a.size(), b.size()) <= 32 ||
        1LL * a.size() * b.size() <= 32768) {
        return multiply_small(a, b);
    }
    int n = 1;
    while (n < needed) n <<= 1;
    vector<int> fa(n, 0), fb(n, 0);
    copy(a.begin(), a.end(), fa.begin());
    copy(b.begin(), b.end(), fb.begin());
    ntt(fa, false);
    ntt(fb, false);
    for (int i = 0; i < n; ++i) fa[i] = int(1LL * fa[i] * fb[i] % MOD);
    ntt(fa, true);
    fa.resize(needed);
    while (fa.size() > 1 && fa.back() == 0) fa.pop_back();
    return fa;
}

vector<int> complete_polynomial(int l, int r) {
    int m = min(l, r);
    vector<int> p(m + 1);
    for (int s = 0; s <= m; ++s) {
        long long value = 1LL * fact_values[l] * fact_values[r] % MOD;
        value = value * invfact_values[s] % MOD;
        value = value * invfact_values[l - s] % MOD;
        value = value * invfact_values[r - s] % MOD;
        p[s] = int(value);
    }
    return p;
}

bool is_complete(const vector<int>& p, int l, int r) {
    int m = min(l, r);
    if (int(p.size()) != m + 1) return false;
    for (int s = 0; s <= m; ++s) {
        long long value = 1LL * fact_values[l] * fact_values[r] % MOD;
        value = value * invfact_values[s] % MOD;
        value = value * invfact_values[l - s] % MOD;
        value = value * invfact_values[r - s] % MOD;
        if (p[s] != int(value)) return false;
    }
    return true;
}

const vector<int>& get_invfact_ntt(int n) {
    int index = __builtin_ctz(unsigned(n));
    vector<int>& cached = invfact_ntt_cache[index];
    if (cached.empty()) {
        cached.assign(n, 0);
        for (int i = 0; i < n / 2; ++i) cached[i] = invfact_values[i];
        ntt(cached, false);
    }
    return cached;
}

vector<int> complement_polynomial(const vector<int>& p, int l, int r) {
    int m = min(l, r);
    if (p.size() == 1) return complete_polynomial(l, r);
    if (is_complete(p, l, r)) return vector<int>{1};

    vector<int> a(p.size());
    for (int j = 0; j < int(p.size()); ++j) {
        long long value = 1LL * p[j] * fact_values[l - j] % MOD;
        value = value * fact_values[r - j] % MOD;
        if ((j & 1) && value != 0) value = MOD - value;
        a[j] = int(value);
    }

    vector<int> c;
    if (p.size() <= 32 || 1LL * p.size() * (m + 1) <= 32768) {
        c.assign(m + 1, 0);
        for (int s = 0; s <= m; ++s) {
            int hi = min(s, int(a.size()) - 1);
            long long sum = 0;
            int count = 0;
            for (int j = 0; j <= hi; ++j) {
                sum += 1LL * a[j] * invfact_values[s - j];
                if ((++count & 7) == 0) sum %= MOD;
            }
            c[s] = int(sum % MOD);
        }
    } else {
        int n = 1;
        while (n < 2 * (m + 1)) n <<= 1;
        c.assign(n, 0);
        copy(a.begin(), a.end(), c.begin());
        ntt(c, false);
        const vector<int>& f = get_invfact_ntt(n);
        for (int i = 0; i < n; ++i) c[i] = int(1LL * c[i] * f[i] % MOD);
        ntt(c, true);
        c.resize(m + 1);
    }

    for (int s = 0; s <= m; ++s) {
        long long value = 1LL * c[s] * invfact_values[l - s] % MOD;
        value = value * invfact_values[r - s] % MOD;
        c[s] = int(value);
    }
    while (c.size() > 1 && c.back() == 0) c.pop_back();
    return c;
}

vector<unsigned long long> left_values, right_values;

vector<int> solve(int al, int ar, int bl, int br,
                  unsigned long long k, int bit, bool bad) {
    int l = ar - al;
    int r = br - bl;
    if (l == 0 || r == 0) return vector<int>{1};
    if (k == 0) return bad ? vector<int>{1} : complete_polynomial(l, r);
    if (bit < 0) return bad ? complete_polynomial(l, r) : vector<int>{1};

    unsigned long long next_k = k & ((1ULL << bit) - 1);
    unsigned long long left_threshold =
        (left_values[al] & (~0ULL << (bit + 1))) | (1ULL << bit);
    unsigned long long right_threshold =
        (right_values[bl] & (~0ULL << (bit + 1))) | (1ULL << bit);
    int am = int(lower_bound(left_values.begin() + al, left_values.begin() + ar,
                             left_threshold) - left_values.begin());
    int bm = int(lower_bound(right_values.begin() + bl, right_values.begin() + br,
                             right_threshold) - right_values.begin());
    int bit_value = int((k >> bit) & 1);
    int al0, ar0, bl0, br0, al1, ar1, bl1, br1;
    if (bit_value == 0) {
        al0 = al; ar0 = am; bl0 = bl; br0 = bm;
        al1 = am; ar1 = ar; bl1 = bm; br1 = br;
    } else {
        al0 = al; ar0 = am; bl0 = bm; br0 = br;
        al1 = am; ar1 = ar; bl1 = bl; br1 = bm;
    }
    if (ar0 == al0 && br0 == bl0) {
        return solve(al1, ar1, bl1, br1, next_k, bit - 1, bad);
    }
    if (ar1 == al1 && br1 == bl1) {
        return solve(al0, ar0, bl0, br0, next_k, bit - 1, bad);
    }

    bool direct = bit_value != int(bad);
    bool child_bad = direct ? bad : !bad;
    vector<int> p0 = solve(al0, ar0, bl0, br0, next_k, bit - 1, child_bad);
    vector<int> p1 = solve(al1, ar1, bl1, br1, next_k, bit - 1, child_bad);
    vector<int> p = multiply(p0, p1);
    if (direct) return p;
    return complement_polynomial(p, l, r);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    unsigned long long k;
    cin >> n >> k;
    left_values.resize(n);
    right_values.resize(n);
    for (auto& x : left_values) cin >> x;
    for (auto& x : right_values) cin >> x;
    sort(left_values.begin(), left_values.end());
    sort(right_values.begin(), right_values.end());

    int factorial_limit = max(n, 131072);
    fact_values.resize(factorial_limit + 1);
    invfact_values.resize(factorial_limit + 1);
    fact_values[0] = 1;
    for (int i = 1; i <= factorial_limit; ++i) {
        fact_values[i] = int(1LL * fact_values[i - 1] * i % MOD);
    }
    invfact_values[factorial_limit] = mod_pow(fact_values[factorial_limit], MOD - 2);
    for (int i = factorial_limit; i > 0; --i) {
        invfact_values[i - 1] = int(1LL * invfact_values[i] * i % MOD);
    }

    vector<int> answer = solve(0, n, 0, n, k, 59, false);
    for (int s = 1; s <= n; ++s) {
        if (s > 1) cout << ' ';
        cout << (s < int(answer.size()) ? answer[s] : 0);
    }
    cout << '\n';
    return 0;
}
