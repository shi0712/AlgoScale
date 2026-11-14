#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int G = 3;

int modpow(int a, long long e) {
    int r = 1;
    while (e) {
        if (e & 1) r = (long long)r * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return r;
}

void ntt(vector<int> &a, bool invert) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int wlen = modpow(G, (MOD - 1) / len);
        if (invert) wlen = modpow(wlen, MOD - 2);
        for (int i = 0; i < n; i += len) {
            int w = 1;
            for (int j = 0; j < len / 2; ++j) {
                int u = a[i + j];
                int v = (long long)a[i + j + len / 2] * w % MOD;
                a[i + j] = u + v < MOD ? u + v : u + v - MOD;
                a[i + j + len / 2] = u >= v ? u - v : u - v + MOD;
                w = (long long)w * wlen % MOD;
            }
        }
    }
    if (invert) {
        int invn = modpow(n, MOD - 2);
        for (int &x : a) x = (long long)x * invn % MOD;
    }
}

vector<int> multiply(vector<int> a, vector<int> b) {
    int need = (int)a.size() + (int)b.size() - 1;
    int n = 1;
    while (n < need) n <<= 1;
    a.resize(n);
    b.resize(n);
    ntt(a, false);
    ntt(b, false);
    for (int i = 0; i < n; ++i) a[i] = (long long)a[i] * b[i] % MOD;
    ntt(a, true);
    a.resize(need);
    return a;
}

vector<int> series_inv(const vector<int> &f, int n) {
    vector<int> g(1, modpow(f[0], MOD - 2));
    for (int len = 1; len < n; len <<= 1) {
        int next = min(2 * len, n);
        vector<int> h(f.begin(), f.begin() + next);
        h = multiply(h, g);
        h.resize(next);
        for (int &x : h) x = x == 0 ? 0 : MOD - x;
        h[0] = (h[0] + 2) % MOD;
        g = multiply(g, h);
        g.resize(next);
    }
    g.resize(n);
    return g;
}

vector<int> series_log(const vector<int> &f, int n) {
    vector<int> d(n - 1);
    for (int i = 1; i < n; ++i) d[i - 1] = (long long)i * f[i] % MOD;
    vector<int> g = multiply(d, series_inv(f, n - 1));
    g.resize(n - 1);
    g.insert(g.begin(), 0);
    for (int i = 1; i < n; ++i) g[i] = (long long)g[i] * modpow(i, MOD - 2) % MOD;
    return g;
}

vector<int> series_exp(const vector<int> &f, int m) {
    int n = m + 1;
    vector<int> g(1, 1);
    for (int len = 1; len < n; len <<= 1) {
        int next = min(2 * len, n);
        g.resize(next);
        vector<int> h = series_log(g, next);
        for (int i = 0; i < next; ++i) {
            h[i] = f[i] - h[i];
            if (h[i] < 0) h[i] += MOD;
        }
        h[0] = (h[0] + 1) % MOD;
        g = multiply(g, h);
        g.resize(next);
    }
    return g;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<int> a(n);
    int m = 0;
    vector<int> cnt;
    for (int &x : a) {
        cin >> x;
        m += x;
    }
    cnt.assign(m + 1, 0);
    for (int x : a) {
        if (x + 1 <= m) ++cnt[x + 1];
    }

    vector<int> inv(max(n + m, m) + 2);
    inv[1] = 1;
    for (int i = 2; i < (int)inv.size(); ++i)
        inv[i] = MOD - (long long)(MOD / i) * inv[MOD % i] % MOD;

    vector<int> f(m + 1, 0);
    for (int b = 1; b <= m; ++b) {
        if (!cnt[b]) continue;
        for (int k = 1; k * b <= m; ++k) {
            int &x = f[k * b];
            x -= (long long)cnt[b] * inv[k] % MOD;
            if (x < 0) x += MOD;
        }
    }
    for (int i = 1; i <= m; ++i) {
        long long term = (long long)m * inv[i] % MOD;
        f[i] = (f[i] + (i & 1 ? term : MOD - term)) % MOD;
    }

    vector<int> q = series_exp(f, m);
    vector<int> fact(n + m + 1, 1);
    for (int i = 1; i <= n + m; ++i) fact[i] = (long long)fact[i - 1] * i % MOD;
    long long answer = 0;
    if (n == 1) {
        cout << q[m] << '\n';
        return 0;
    }
    for (int j = 0; j <= m; ++j) {
        int r = m - j;
        int den = 1;
        if (n > 1) {
            den = (long long)fact[n - 2 + r] * modpow(fact[r], MOD - 2) % MOD
                * modpow(fact[n - 2], MOD - 2) % MOD;
        }
        answer = (answer + (long long)q[j] * den) % MOD;
    }
    cout << answer << '\n';
}
