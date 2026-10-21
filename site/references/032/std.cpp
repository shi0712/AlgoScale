#include <bits/stdc++.h>
using namespace std;

constexpr int MOD = 998244353;
using Poly = vector<int>;

int modpow(int a, int e) {
    int r = 1;
    for (; e; e >>= 1, a = 1LL * a * a % MOD)
        if (e & 1) r = 1LL * r * a % MOD;
    return r;
}

void trim(Poly &a) {
    while (!a.empty() && a.back() == 0) a.pop_back();
}

void ntt(Poly &a, bool invert) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int step = modpow(3, (MOD - 1) / len);
        if (invert) step = modpow(step, MOD - 2);
        for (int i = 0; i < n; i += len) {
            int w = 1;
            for (int j = 0; j < len / 2; ++j) {
                int u = a[i + j];
                int v = 1LL * a[i + j + len / 2] * w % MOD;
                int x = u + v, y = u - v;
                if (x >= MOD) x -= MOD;
                if (y < 0) y += MOD;
                a[i + j] = x;
                a[i + j + len / 2] = y;
                w = 1LL * w * step % MOD;
            }
        }
    }
    if (invert) {
        int inv = modpow(n, MOD - 2);
        for (int &x : a) x = 1LL * x * inv % MOD;
    }
}

Poly mul(Poly a, Poly b) {
    if (a.empty() || b.empty()) return {};
    int need = (int)a.size() + (int)b.size() - 1;
    if (min(a.size(), b.size()) <= 16) {
        Poly c(need);
        for (int i = 0; i < (int)a.size(); ++i)
            for (int j = 0; j < (int)b.size(); ++j)
                c[i + j] = (c[i + j] + 1LL * a[i] * b[j]) % MOD;
        trim(c);
        return c;
    }
    int size = 1;
    while (size < need) size <<= 1;
    a.resize(size); b.resize(size);
    ntt(a, false); ntt(b, false);
    for (int i = 0; i < size; ++i) a[i] = 1LL * a[i] * b[i] % MOD;
    ntt(a, true);
    a.resize(need);
    trim(a);
    return a;
}

Poly add(Poly a, const Poly &b) {
    a.resize(max(a.size(), b.size()));
    for (int i = 0; i < (int)b.size(); ++i) {
        a[i] += b[i];
        if (a[i] >= MOD) a[i] -= MOD;
    }
    trim(a);
    return a;
}

// Inverse modulo x^n; requires a[0] != 0.
Poly inverse(const Poly &a, int n) {
    Poly b{modpow(a[0], MOD - 2)};
    for (int len = 1; len < n; len <<= 1) {
        int size = min(2 * len, n);
        Poly cut(a.begin(), a.begin() + min((int)a.size(), size));
        Poly c = mul(cut, b);
        c.resize(size);
        for (int &x : c) if (x) x = MOD - x;
        c[0] += 2;
        if (c[0] >= MOD) c[0] -= MOD;
        b = mul(b, c);
        b.resize(size);
    }
    return b;
}

Poly remainder(Poly a, const Poly &b) {
    trim(a);
    if (a.size() < b.size()) return a;
    if (b.size() == 1) return {};
    int k = (int)a.size() - (int)b.size() + 1;
    Poly ra(a.rbegin(), a.rend()), rb(b.rbegin(), b.rend());
    ra.resize(k);
    if ((int)rb.size() > k) rb.resize(k);
    Poly q = mul(ra, inverse(rb, k));
    q.resize(k);
    reverse(q.begin(), q.end());
    Poly c = mul(q, b);
    a.resize(b.size() - 1);
    for (int i = 0; i < (int)a.size(); ++i) {
        if (i < (int)c.size()) a[i] -= c[i];
        if (a[i] < 0) a[i] += MOD;
    }
    trim(a);
    return a;
}

// Multipoint evaluation at 0, -1, ..., -(n-1).
struct Evaluator {
    int n;
    vector<Poly> tree;

    explicit Evaluator(int count) : n(count), tree(4 * count) { build(1, 0, n); }

    void build(int u, int l, int r) {
        if (r - l == 1) { tree[u] = {l, 1}; return; }
        int mid = (l + r) / 2;
        build(u * 2, l, mid);
        build(u * 2 + 1, mid, r);
        tree[u] = mul(tree[u * 2], tree[u * 2 + 1]);
    }

    void dfs(int u, int l, int r, const Poly &f, Poly &out) const {
        if (f.empty()) return;
        if (r - l <= 16) {
            for (int i = l; i < r; ++i) {
                int x = i ? MOD - i : 0, value = 0;
                for (int j = (int)f.size() - 1; j >= 0; --j)
                    value = (1LL * value * x + f[j]) % MOD;
                out[i] = value;
            }
            return;
        }
        int mid = (l + r) / 2;
        dfs(u * 2, l, mid, remainder(f, tree[u * 2]), out);
        dfs(u * 2 + 1, mid, r, remainder(f, tree[u * 2 + 1]), out);
    }

    Poly eval(const Poly &f) const {
        Poly out(n);
        dfs(1, 0, n, f, out);
        return out;
    }
};

struct Matrix { Poly a, b, c, d; };

Matrix multiply(const Matrix &x, const Matrix &y) {
    return {
        add(mul(x.a, y.a), mul(x.b, y.c)),
        add(mul(x.a, y.b), mul(x.b, y.d)),
        add(mul(x.c, y.a), mul(x.d, y.c)),
        add(mul(x.c, y.b), mul(x.d, y.d))
    };
}

// T_r * ... * T_l, where T_i = [[i, i*x], [1, 0]].
Matrix matrix_product(int l, int r) {
    if (l == r) return {{l}, {0, l}, {1}, {}};
    int mid = (l + r) / 2;
    Matrix left = matrix_product(l, mid);
    Matrix right = matrix_product(mid + 1, r);
    return multiply(right, left);
}

Poly fact, ifact, inv;

// F_m[k] = 1/m! * sum_j (-1)^j C(k,j) D_m(-j).
Poly get_F(const Poly &D, int m, const Evaluator &ev) {
    int degree = m / 2;
    Poly values = ev.eval(D), a(degree + 1), b(degree + 1);
    for (int j = 0; j <= degree; ++j) {
        a[j] = 1LL * values[j] * ifact[j] % MOD;
        if ((j & 1) && a[j]) a[j] = MOD - a[j];
        b[j] = ifact[j];
    }
    Poly f = mul(a, b);
    f.resize(degree + 1);
    for (int k = 0; k <= degree; ++k)
        f[k] = 1LL * f[k] * fact[k] % MOD * ifact[m] % MOD;
    trim(f);
    return f;
}

struct RunData { Poly F, Q; };
map<int, RunData> cache;

const RunData &get_run(int m) {
    auto it = cache.find(m);
    if (it != cache.end()) return it->second;

    Matrix p = matrix_product(1, m);
    // The second column is (D_{m+1}, D_m)^T.
    Evaluator ev((m + 1) / 2 + 1);
    Poly F = get_F(p.d, m, ev);
    Poly Q = get_F(p.b, m + 1, ev);
    for (int k = 1; k < (int)Q.size(); ++k)
        Q[k] = 1LL * Q[k] * (m + 1) % MOD * inv[k] % MOD;
    return cache.emplace(m, RunData{move(F), move(Q)}).first->second;
}

// Inward-facing block R^a L^b.
Poly get_block(int a, int b) {
    const auto &A = get_run(a);
    const auto &B = get_run(b);
    Poly p = mul(A.Q, B.Q), q = mul(A.F, B.F);
    p.resize(max(p.size(), q.size() + 1));
    for (int i = 0; i < (int)q.size(); ++i) {
        p[i] -= q[i];
        if (p[i] < 0) p[i] += MOD;
        p[i + 1] += q[i];
        if (p[i + 1] >= MOD) p[i + 1] -= MOD;
    }
    // Divide [Q_a Q_b + (x-1)F_a F_b] by x.
    assert(p[0] == 0);
    p.erase(p.begin());
    trim(p);
    return p;
}

Poly product(const vector<Poly> &f, int l, int r) {
    if (l == r) return {1};
    if (r - l == 1) return f[l];
    int mid = (l + r) / 2;
    return mul(product(f, l, mid), product(f, mid, r));
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    string s;
    if (!(cin >> n >> s)) return 0;

    fact.resize(n + 2); ifact.resize(n + 2); inv.resize(n + 2);
    fact[0] = 1;
    for (int i = 1; i <= n + 1; ++i) fact[i] = 1LL * fact[i - 1] * i % MOD;
    ifact[n + 1] = modpow(fact[n + 1], MOD - 2);
    for (int i = n + 1; i >= 1; --i) ifact[i - 1] = 1LL * ifact[i] * i % MOD;
    for (int i = 1; i <= n + 1; ++i) inv[i] = 1LL * fact[i - 1] * ifact[i] % MOD;

    vector<pair<char, int>> runs;
    for (char ch : s) {
        if (!runs.empty() && runs.back().first == ch) ++runs.back().second;
        else runs.push_back({ch, 1});
    }

    vector<Poly> factors;
    for (int i = 0; i < (int)runs.size();) {
        if (runs[i].first == 'L' || i + 1 == (int)runs.size()) {
            // A leading L-run or trailing R-run faces the outer boundary.
            factors.push_back(get_run(runs[i].second).Q);
            ++i;
        } else {
            factors.push_back(get_block(runs[i].second, runs[i + 1].second));
            i += 2;
        }
    }

    Poly ans = product(factors, 0, (int)factors.size());
    for (int k = 1; k <= n; ++k) {
        int value = k < (int)ans.size() ? 1LL * ans[k] * fact[n] % MOD : 0;
        cout << value << (k == n ? '\n' : ' ');
    }
    return 0;
}
