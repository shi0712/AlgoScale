#include <bits/stdc++.h>
using namespace std;

constexpr int MOD = 998244353;
constexpr int G = 3;
using Poly = vector<int>;

int addmod(int a, int b) {
    int s = a + b;
    return s >= MOD ? s - MOD : s;
}
int submod(int a, int b) {
    int s = a - b;
    return s < 0 ? s + MOD : s;
}
int mulmod(int a, int b) { return int(1LL * a * b % MOD); }
int modpow(int a, int e) {
    int r = 1;
    for (; e; e >>= 1, a = mulmod(a, a))
        if (e & 1) r = mulmod(r, a);
    return r;
}
void trim(Poly &a) {
    while (!a.empty() && a.back() == 0) a.pop_back();
}

void ntt(Poly &a, bool invert) {
    const int n = int(a.size());
    static Poly roots{0, 1};
    static vector<Poly> revs(24);
    if (int(roots.size()) < n) {
        int s = __builtin_ctz((unsigned)roots.size());
        roots.resize(n);
        while ((1 << s) < n) {
            int z = modpow(G, (MOD - 1) >> (s + 1));
            for (int i = 1 << (s - 1); i < (1 << s); ++i) {
                roots[i << 1] = roots[i];
                roots[i << 1 | 1] = mulmod(roots[i], z);
            }
            ++s;
        }
    }
    int lg = __builtin_ctz((unsigned)n);
    Poly &rev = revs[lg];
    if (int(rev.size()) != n) {
        rev.resize(n);
        for (int i = 1; i < n; ++i)
            rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (lg - 1));
    }
    for (int i = 0; i < n; ++i)
        if (i < rev[i]) swap(a[i], a[rev[i]]);
    for (int len = 1; len < n; len <<= 1) {
        for (int i = 0; i < n; i += len << 1) {
            for (int j = 0; j < len; ++j) {
                int u = a[i + j];
                int v = mulmod(a[i + j + len], roots[len + j]);
                a[i + j] = addmod(u, v);
                a[i + j + len] = submod(u, v);
            }
        }
    }
    if (invert) {
        reverse(a.begin() + 1, a.end());
        int invn = modpow(n, MOD - 2);
        for (int &x : a) x = mulmod(x, invn);
    }
}

Poly multiply(const Poly &a, const Poly &b) {
    if (a.empty() || b.empty()) return {};
    if (min(a.size(), b.size()) <= 24) {
        Poly c(a.size() + b.size() - 1);
        for (int i = 0; i < int(a.size()); ++i)
            if (a[i])
                for (int j = 0; j < int(b.size()); ++j)
                    c[i + j] = addmod(c[i + j], mulmod(a[i], b[j]));
        trim(c);
        return c;
    }
    int need = int(a.size() + b.size() - 1), len = 1;
    while (len < need) len <<= 1;
    Poly x(a), y(b);
    x.resize(len); y.resize(len);
    ntt(x, false); ntt(y, false);
    for (int i = 0; i < len; ++i) x[i] = mulmod(x[i], y[i]);
    ntt(x, true);
    x.resize(need);
    trim(x);
    return x;
}

Poly inverse_series(const Poly &a, int need) {
    Poly r{modpow(a[0], MOD - 2)};
    while (int(r.size()) < need) {
        int len = min(need, int(r.size()) * 2);
        Poly f(a.begin(), a.begin() + min(int(a.size()), len));
        Poly t = multiply(f, r);
        t.resize(len);
        for (int &x : t) if (x) x = MOD - x;
        t[0] = addmod(t[0], 2);
        r = multiply(r, t);
        r.resize(len);
    }
    return r;
}

// b 为首一多项式；其翻转多项式的逆在不同枚举中可以复用。
Poly remainder(Poly a, const Poly &b, Poly &cached_inv) {
    trim(a);
    const int d = int(b.size()) - 1;
    if (int(a.size()) <= d) return a;
    int need = int(a.size()) - d;
    if (d <= 16 || 1LL * d * need <= 4096) {
        for (int i = int(a.size()) - 1; i >= d; --i) {
            int v = a[i];
            if (!v) continue;
            for (int j = 0; j < d; ++j)
                a[i - d + j] = submod(a[i - d + j], mulmod(v, b[j]));
        }
        a.resize(d);
        trim(a);
        return a;
    }
    if (int(cached_inv.size()) < need) {
        int len = 1;
        while (len < need) len <<= 1;
        Poly rb(b.rbegin(), b.rend());
        cached_inv = inverse_series(rb, len);
    }
    Poly ra(need), ri(cached_inv.begin(), cached_inv.begin() + need);
    for (int i = 0; i < need; ++i) ra[i] = a[a.size() - 1 - i];
    Poly quotient = multiply(ra, ri);
    quotient.resize(need);
    reverse(quotient.begin(), quotient.end());
    Poly prod = multiply(quotient, b);
    prod.resize(d);
    a.resize(d);
    for (int i = 0; i < d; ++i) a[i] = submod(a[i], prod[i]);
    trim(a);
    return a;
}

void add_shifted(Poly &a, const Poly &b, int shift) {
    if (b.empty()) return;
    if (a.size() < b.size() + shift) a.resize(b.size() + shift);
    for (int i = 0; i < int(b.size()); ++i)
        a[i + shift] = addmod(a[i + shift], b[i]);
}

struct FastDP {
    static constexpr int SMALL = 12;
    int q, k, start = 0, active = 0, sum = 0;
    vector<int> b, r, f;
    const vector<Poly> &pw;
    vector<Poly> product, inv_product;

    FastDP(const vector<int> &points, int upper, const vector<Poly> &powers)
        : q(int(points.size())), k(upper), b(points), r(q), f(q),
          pw(powers), product(4 * q), inv_product(4 * q) {
        build(1, 0, q - 1);
    }

    void build(int id, int l, int rr) {
        if (rr - l + 1 <= SMALL) {
            Poly p{1};
            for (int i = l; i <= rr; ++i) {
                Poly t(p.size() + 1);
                for (int j = 0; j < int(p.size()); ++j) {
                    t[j] = submod(t[j], mulmod(b[i], p[j]));
                    t[j + 1] = addmod(t[j + 1], p[j]);
                }
                p.swap(t);
            }
            product[id] = move(p);
            return;
        }
        int mid = (l + rr) / 2;
        build(id * 2, l, mid);
        build(id * 2 + 1, mid + 1, rr);
        product[id] = multiply(product[id * 2], product[id * 2 + 1]);
    }

    // Q 以 r[l] 为指数基准，表示左侧已计算状态的贡献。
    // 返回 sum_{i in [l,rr]} f[i] * z^(r[rr]-r[i])。
    Poly solve(int id, int l, int rr, const Poly &Q) {
        if (rr < start) return {};
        if (rr - l + 1 <= SMALL) {
            int first = max(l, start);
            for (int i = first; i <= rr; ++i) {
                int v = 0;
                for (int j = int(Q.size()) - 1; j >= 0; --j)
                    v = addmod(mulmod(v, b[i]), Q[j]);
                v = mulmod(v, pw[b[i]][r[i] - r[l]]);
                if (i == start) v = addmod(v, pw[b[i]][r[i]]);
                for (int j = first; j < i; ++j)
                    v = addmod(v, mulmod(f[j], pw[b[i]][r[i] - r[j]]));
                f[i] = v ? MOD - v : 0;
                sum = addmod(sum, mulmod(f[i], pw[k][active - r[i]]));
            }
            Poly ret(r[rr] - r[first] + 1);
            for (int i = first; i <= rr; ++i)
                ret[r[rr] - r[i]] = addmod(ret[r[rr] - r[i]], f[i]);
            trim(ret);
            return ret;
        }
        int mid = (l + rr) / 2;
        // start 之前的状态都为 0，因此此时 Q 也为 0。
        if (mid < start) return solve(id * 2 + 1, mid + 1, rr, {});

        Poly leftQ = remainder(Q, product[id * 2], inv_product[id * 2]);
        Poly leftP = solve(id * 2, l, mid, leftQ);

        Poly rightQ;
        add_shifted(rightQ, Q, r[mid + 1] - r[l]);
        add_shifted(rightQ, leftP, r[mid + 1] - r[mid]);
        rightQ = remainder(move(rightQ), product[id * 2 + 1],
                           inv_product[id * 2 + 1]);
        Poly rightP = solve(id * 2 + 1, mid + 1, rr, rightQ);
        add_shifted(rightP, leftP, r[rr] - r[mid]);
        trim(rightP);
        return rightP;
    }

    int run(int s, int h, const vector<int> &prefix_counts) {
        start = s; active = h; r = prefix_counts; sum = 0;
        solve(1, 0, q - 1, {});
        return sum;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m, k;
    if (!(cin >> n >> m >> k)) return 0;
    vector<int> L(k + 1, n + 1), R(k + 1, 0);
    for (int i = 1, a; i <= n; ++i) {
        cin >> a;
        L[a] = min(L[a], i);
        R[a] = i;
    }
    vector<pair<int, int>> trains(m);
    vector<int> farthest(k + 1, 0), total_le(k + 1, 0);
    for (auto &[p, x] : trains) {
        cin >> p >> x;
        farthest[x] = max(farthest[x], p);
        ++total_le[x];
    }
    for (int c = 1; c <= k; ++c) {
        farthest[c] = max(farthest[c], farthest[c - 1]);
        total_le[c] += total_le[c - 1];
    }

    // 按等级递增扫描，只保留 L 严格下降的必要约束。
    vector<pair<int, int>> event;
    int bestL = n + 1;
    for (int c = 1; c < k; ++c) {
        if (L[c] >= R[c] || farthest[c] >= R[c]) continue;
        if (L[c] < bestL) {
            event.emplace_back(c, L[c]);
            bestL = L[c];
        }
    }
    int answer = modpow(k, m);
    if (event.empty()) {
        cout << answer << '\n';
        return 0;
    }
    reverse(event.begin(), event.end());
    int q = int(event.size());

    vector<Poly> powers(k + 1, Poly(m + 1, 1));
    for (int a = 0; a <= k; ++a)
        for (int e = 1; e <= m; ++e)
            powers[a][e] = mulmod(powers[a][e - 1], a);

    // pref[i][c]：满足 p < L_i 且 x <= c 的列车数量。
    sort(trains.begin(), trains.end());
    vector<Poly> pref(q, Poly(k + 1));
    vector<int> freq(k + 1);
    int ptr = 0;
    for (int i = 0; i < q; ++i) {
        while (ptr < m && trains[ptr].first < event[i].second) {
            ++freq[trains[ptr].second];
            ++ptr;
        }
        for (int c = 1; c <= k; ++c)
            pref[i][c] = pref[i][c - 1] + freq[c];
    }
    vector<int> points(q), counts(q);
    for (int i = 0; i < q; ++i) points[i] = k - event[i].first;
    FastDP dp(points, k, powers);

    for (int s = 0; s < q; ++s) {
        int c = event[s].first;
        int h = m - total_le[c];
        for (int i = 0; i < q; ++i)
            counts[i] = pref[i][k] - pref[i][c];
        int value = dp.run(s, h, counts);
        answer = addmod(answer, mulmod(value, powers[k - c][m - h]));
    }
    cout << answer << '\n';
    return 0;
}
