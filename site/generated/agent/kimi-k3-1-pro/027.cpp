#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int G = 3;

int addmod(int a, int b) { int s = a + b; return s >= MOD ? s - MOD : s; }
int submod(int a, int b) { int s = a - b; return s < 0 ? s + MOD : s; }
int mulmod(long long a, long long b) { return int(a * b % MOD); }
int powmod(int a, long long e) {
    int r = 1;
    while (e) {
        if (e & 1) r = mulmod(r, a);
        a = mulmod(a, a);
        e >>= 1;
    }
    return r;
}

void ntt(vector<int>& a, bool invert) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int wlen = powmod(G, (MOD - 1) / len);
        if (invert) wlen = powmod(wlen, MOD - 2);
        for (int i = 0; i < n; i += len) {
            int w = 1;
            for (int j = 0; j < len / 2; ++j) {
                int u = a[i + j];
                int v = mulmod(a[i + j + len / 2], w);
                a[i + j] = addmod(u, v);
                a[i + j + len / 2] = submod(u, v);
                w = mulmod(w, wlen);
            }
        }
    }
    if (invert) {
        int invn = powmod(n, MOD - 2);
        for (int& x : a) x = mulmod(x, invn);
    }
}

using Poly = vector<int>;

void trim(Poly& a) {
    while (!a.empty() && a.back() == 0) a.pop_back();
}

Poly multiply(const Poly& a, const Poly& b) {
    if (a.empty() || b.empty()) return {};
    if ((long long)a.size() * b.size() <= 4096) {
        Poly c(a.size() + b.size() - 1);
        for (int i = 0; i < (int)a.size(); ++i) {
            long long ai = a[i];
            for (int j = 0; j < (int)b.size(); ++j)
                c[i + j] = (c[i + j] + ai * b[j]) % MOD;
        }
        trim(c);
        return c;
    }
    int n = 1;
    while (n < (int)a.size() + (int)b.size() - 1) n <<= 1;
    vector<int> fa(a.begin(), a.end()), fb(b.begin(), b.end());
    fa.resize(n); fb.resize(n);
    ntt(fa, false); ntt(fb, false);
    for (int i = 0; i < n; ++i) fa[i] = mulmod(fa[i], fb[i]);
    ntt(fa, true);
    fa.resize(a.size() + b.size() - 1);
    trim(fa);
    return fa;
}

Poly addpoly(const Poly& a, const Poly& b, int sign = 1) {
    Poly c(max(a.size(), b.size()));
    for (int i = 0; i < (int)c.size(); ++i) {
        int x = i < (int)a.size() ? a[i] : 0;
        int y = i < (int)b.size() ? b[i] : 0;
        c[i] = sign == 1 ? addmod(x, y) : submod(x, y);
    }
    trim(c);
    return c;
}

Poly shiftpoly(const Poly& a) {
    if (a.empty()) return {};
    Poly c(a.size() + 1);
    for (int i = 0; i < (int)a.size(); ++i) c[i + 1] = a[i];
    return c;
}

Poly inverse(const Poly& a, int n) {
    Poly r(1, powmod(a[0], MOD - 2));
    int m = 1;
    while (m < n) {
        m <<= 1;
        Poly aa(a.begin(), a.begin() + min((int)a.size(), m));
        Poly t = multiply(aa, r);
        t.resize(m);
        for (int& x : t) x = x ? MOD - x : 0;
        t[0] = addmod(t[0], 2);
        r = multiply(r, t);
        r.resize(m);
    }
    r.resize(n);
    return r;
}

Poly reversedPoly(Poly a) {
    reverse(a.begin(), a.end());
    return a;
}

Poly remainderWithInverse(const Poly& a, const Poly& b, const Poly& invRevB) {
    if (a.size() < b.size()) return a;
    int n = (int)a.size() - (int)b.size() + 1;
    Poly ar = reversedPoly(a);
    ar.resize(n);
    Poly q = multiply(ar, invRevB);
    q.resize(n);
    reverse(q.begin(), q.end());
    Poly r = multiply(b, q);
    r.resize(b.size() - 1);
    for (int i = 0; i < (int)r.size(); ++i) r[i] = submod(a[i], r[i]);
    trim(r);
    return r;
}

Poly remainderNaive(Poly a, const Poly& b) {
    if (a.size() < b.size()) return a;
    int ib = powmod(b.back(), MOD - 2);
    for (int i = (int)a.size() - 1; i >= (int)b.size() - 1; --i) {
        int q = mulmod(a[i], ib);
        if (!q) continue;
        for (int j = 0; j < (int)b.size(); ++j)
            a[i - j] = submod(a[i - j], mulmod(q, b[b.size() - 1 - j]));
    }
    a.resize(b.size() - 1);
    trim(a);
    return a;
}

vector<int> fact, ifact;

struct Matrix {
    Poly a, b, c, d;
};

Matrix identityMatrix() { return {{1}, {}, {}, {1}}; }

Matrix multiplyMatrix(const Matrix& x, const Matrix& y) {
    Matrix z;
    z.a = addpoly(multiply(x.a, y.a), multiply(x.b, y.c));
    z.b = addpoly(multiply(x.a, y.b), multiply(x.b, y.d));
    z.c = addpoly(multiply(x.c, y.a), multiply(x.d, y.c));
    z.d = addpoly(multiply(x.c, y.b), multiply(x.d, y.d));
    return z;
}

struct IndependentTree {
    int n = 0;
    vector<Matrix> t;

    IndependentTree() = default;
    explicit IndependentTree(const vector<int>& weights) {
        n = (int)weights.size();
        if (!n) return;
        t.resize(4 * n);
        build(1, 0, n, weights);
    }

    void build(int v, int l, int r, const vector<int>& weights) {
        if (r - l == 1) {
            t[v] = {{1}, {0, weights[l]}, {1}, {}};
            return;
        }
        int m = (l + r) / 2;
        build(v * 2, l, m, weights);
        build(v * 2 + 1, m, r, weights);
        t[v] = multiplyMatrix(t[v * 2 + 1], t[v * 2]);
    }

    Matrix query(int v, int l, int r, int ql) const {
        if (ql <= l) return identityMatrix();
        if (r <= ql) return t[v];
        int m = (l + r) / 2;
        Matrix left = query(v * 2, l, m, ql);
        Matrix right = query(v * 2 + 1, m, r, ql);
        return multiplyMatrix(right, left);
    }

    Poly prefix(int len) const {
        if (len <= 0) return {1};
        Matrix m = query(1, 0, n, len);
        return addpoly(m.a, m.b);
    }
};

struct EvalTree {
    int n;
    vector<Poly> prod, invrev;

    explicit EvalTree(int points) : n(points), prod(4 * points), invrev(4 * points) {
        build(1, 0, points);
    }

    void build(int v, int l, int r) {
        if (r - l == 1) {
            prod[v] = {submod(0, l), 1};
            invrev[v] = {1};
            return;
        }
        int m = (l + r) / 2;
        build(v * 2, l, m);
        build(v * 2 + 1, m, r);
        prod[v] = multiply(prod[v * 2], prod[v * 2 + 1]);
        Poly rev = reversedPoly(prod[v]);
        invrev[v] = inverse(rev, (int)prod[v].size());
    }

    Poly rem(const Poly& a, int v) const {
        if (a.size() < prod[v].size()) return a;
        if (prod[v].size() <= 48 || a.size() <= 64) return remainderNaive(a, prod[v]);
        return remainderWithInverse(a, prod[v], invrev[v]);
    }

    void evaluate(int v, int l, int r, const Poly& a, vector<int>& out) const {
        if (r - l == 1) {
            long long value = 0;
            for (int i = (int)a.size() - 1; i >= 0; --i) value = (value * l + a[i]) % MOD;
            out[l] = (int)value;
            return;
        }
        int m = (l + r) / 2;
        evaluate(v * 2, l, m, rem(a, v * 2), out);
        evaluate(v * 2 + 1, m, r, rem(a, v * 2 + 1), out);
    }

    vector<int> evaluate(const Poly& a) const {
        vector<int> out(n);
        evaluate(1, 0, n, a, out);
        return out;
    }
};

array<Poly, 3> pureRowsSmall(int a) {
    int N = a + 2;
    vector<Poly> rows(N + 1);
    rows[0] = {1};
    for (int n = 2; n <= N; ++n) {
        rows[n].assign(n / 2 + 1, 0);
        int invn = powmod(n, MOD - 2);
        for (int k = 1; k <= n / 2; ++k) {
            int x = k < (int)rows[n - 1].size() ? rows[n - 1][k] : 0;
            int y = k < (int)rows[n - 2].size() ? rows[n - 2][k] : 0;
            int z = k - 1 < (int)rows[n - 2].size() ? rows[n - 2][k - 1] : 0;
            int val = submod(mulmod(n - 1, x), mulmod(k, y));
            val = addmod(val, mulmod(k, z));
            rows[n][k] = mulmod(val, invn);
        }
    }
    return {rows[a], rows[a + 1], rows[a + 2]};
}

Poly qFromIndependent(const Poly& indep, int n) {
    if (n == 0) return {1};
    if (n == 1) return {};
    Poly q(indep.size() + 1);
    for (int s = 0; s < (int)indep.size(); ++s) {
        int r = s + 1;
        int val = mulmod(fact[n - 1], indep[s]);
        q[r] = (r & 1) ? submod(0, val) : val;
    }
    trim(q);
    return q;
}

Poly pureFromQ(const Poly& q, int n, const EvalTree& eval) {
    int K = n / 2;
    vector<int> values = eval.evaluate(q);
    Poly A(K + 1), B(K + 1);
    for (int j = 0; j <= K; ++j) {
        int v = mulmod(values[j], ifact[j]);
        A[j] = (j & 1) ? submod(0, v) : v;
        B[j] = ifact[j];
    }
    Poly c = multiply(A, B);
    c.resize(K + 1);
    Poly p(K + 1);
    for (int k = 0; k <= K; ++k)
        p[k] = mulmod(mulmod(fact[k], ifact[n]), c[k]);
    trim(p);
    return p;
}

array<Poly, 3> pureRows(int a) {
    if (a <= 96) return pureRowsSmall(a);
    vector<int> weights(a - 1);
    for (int i = 2; i <= a; ++i) weights[i - 2] = powmod(i, MOD - 2);
    IndependentTree indep(weights);
    Poly qa = qFromIndependent(indep.prefix(a - 3), a);
    Poly qa1 = qFromIndependent(indep.prefix(a - 2), a + 1);
    Poly qa2 = qFromIndependent(indep.prefix(a - 1), a + 2);
    EvalTree eval(a / 2 + 2);
    return {pureFromQ(qa, a, eval), pureFromQ(qa1, a + 1, eval), pureFromQ(qa2, a + 2, eval)};
}

Poly pureA0(int a, const array<Poly, 3>& rows) {
    Poly res(a / 2 + 1);
    for (int r = 0; r < (int)res.size(); ++r) {
        if (r + 1 >= (int)rows[1].size()) break;
        res[r] = mulmod(mulmod(a + 1, powmod(r + 1, MOD - 2)), rows[1][r + 1]);
    }
    trim(res);
    return res;
}

Poly pureA1(int a, const array<Poly, 3>& rows) {
    Poly res(a / 2 + 1);
    for (int r = 0; r < (int)res.size(); ++r) {
        if (r + 1 >= (int)rows[2].size()) break;
        int val = mulmod(mulmod(a + 2, powmod(r + 1, MOD - 2)), rows[2][r + 1]);
        if (r < (int)rows[0].size()) val = submod(val, rows[0][r]);
        res[r] = val;
    }
    trim(res);
    return res;
}

Poly blockPolynomial(int a, int b, bool bonus) {
    if (a == 0 || b == 0) {
        int n = a + b;
        auto rows = pureRows(n);
        if (bonus) return shiftpoly(pureA0(n, rows));
        return rows[0];
    }
    auto ra = pureRows(a), rb = pureRows(b);
    Poly result = multiply(ra[0], rb[0]);
    Poly a0 = pureA0(a, ra), a1 = pureA1(a, ra);
    Poly b0 = pureA0(b, rb), b1 = pureA1(b, rb);
    Poly mixed = addpoly(multiply(a0, b1), multiply(a1, b0));
    mixed = addpoly(mixed, multiply(a1, b1), -1);
    result = addpoly(result, shiftpoly(mixed));
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    string s;
    cin >> n >> s;
    fact.resize(n + 3);
    ifact.resize(n + 3);
    fact[0] = 1;
    for (int i = 1; i <= n + 2; ++i) fact[i] = mulmod(fact[i - 1], i);
    ifact[n + 2] = powmod(fact[n + 2], MOD - 2);
    for (int i = n + 2; i; --i) ifact[i - 1] = mulmod(ifact[i], i);

    vector<Poly> blocks;
    int i = 0;
    while (i < n) {
        int j = i;
        while (j + 1 < n && !(s[j] == 'L' && s[j + 1] == 'R')) ++j;
        int a = 0, b = 0;
        for (int k = i; k <= j; ++k) (s[k] == 'R' ? a : b)++;
        bool bonus = (a == 0 && i == 0) || (b == 0 && j == n - 1);
        blocks.push_back(blockPolynomial(a, b, bonus));
        i = j + 1;
    }

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    vector<Poly> polys = blocks;
    for (int k = 0; k < (int)polys.size(); ++k) pq.push({(int)polys[k].size(), k});
    int serial = (int)polys.size();
    while (pq.size() > 1) {
        auto [sa, ia] = pq.top(); pq.pop();
        auto [sb, ib] = pq.top(); pq.pop();
        polys.push_back(multiply(polys[ia], polys[ib]));
        pq.push({(int)polys.back().size(), serial++});
    }
    Poly answer = polys[pq.top().second];
    answer.resize(n + 1, 0);
    for (int k = 1; k <= n; ++k) {
        int v = k < (int)answer.size() ? answer[k] : 0;
        cout << mulmod(fact[n], v) << (k == n ? '\n' : ' ');
    }
}
