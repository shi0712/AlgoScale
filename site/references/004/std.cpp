#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using Poly = vector<int>;
constexpr int MOD = 1000000007;

int power_mod(int a, int e, int mod = MOD) {
    int r = 1;
    while (e) {
        if (e & 1) r = (ll)r * a % mod;
        a = (ll)a * a % mod;
        e >>= 1;
    }
    return r;
}

void trim(Poly &a) {
    while (a.size() > 1 && a.back() == 0) a.pop_back();
}

// 三个 NTT 模数的原根均为 3。
template<int P>
void ntt(vector<int> &a, bool inverse) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int wlen = power_mod(3, (P - 1) / len, P);
        if (inverse) wlen = power_mod(wlen, P - 2, P);
        for (int i = 0; i < n; i += len) {
            int w = 1;
            for (int j = 0; j < len / 2; ++j) {
                int u = a[i + j];
                int v = (ll)a[i + j + len / 2] * w % P;
                int x = u + v;
                if (x >= P) x -= P;
                int y = u - v;
                if (y < 0) y += P;
                a[i + j] = x;
                a[i + j + len / 2] = y;
                w = (ll)w * wlen % P;
            }
        }
    }
    if (inverse) {
        int invn = power_mod(n, P - 2, P);
        for (int &x : a) x = (ll)x * invn % P;
    }
}

template<int P>
vector<int> convolution_prime(const Poly &a, const Poly &b, int len) {
    vector<int> x(len), y(len);
    for (int i = 0; i < (int)a.size(); ++i) x[i] = a[i] % P;
    for (int i = 0; i < (int)b.size(); ++i) y[i] = b[i] % P;
    ntt<P>(x, false);
    ntt<P>(y, false);
    for (int i = 0; i < len; ++i) x[i] = (ll)x[i] * y[i] % P;
    ntt<P>(x, true);
    return x;
}

Poly multiply(const Poly &a, const Poly &b) {
    if ((a.size() == 1 && a[0] == 0) ||
        (b.size() == 1 && b[0] == 0)) return {0};
    int need = (int)a.size() + (int)b.size() - 1;
    Poly c(need);
    if (min(a.size(), b.size()) <= 16 || a.size() * b.size() <= 4096) {
        for (int i = 0; i < (int)a.size(); ++i)
            for (int j = 0; j < (int)b.size(); ++j)
                c[i + j] = (c[i + j] + (ll)a[i] * b[j]) % MOD;
        trim(c);
        return c;
    }
    int len = 1;
    while (len < need) len <<= 1;
    constexpr int P1 = 998244353;
    constexpr int P2 = 1004535809;
    constexpr int P3 = 469762049;
    auto x = convolution_prime<P1>(a, b, len);
    auto y = convolution_prime<P2>(a, b, len);
    auto z = convolution_prime<P3>(a, b, len);
    static const int inv12 = power_mod(P1, P2 - 2, P2);
    static const int p12_3 = (ll)P1 * P2 % P3;
    static const int inv123 = power_mod(p12_3, P3 - 2, P3);
    static const int p12_mod = (ll)P1 * P2 % MOD;
    for (int i = 0; i < need; ++i) {
        ll t1 = (y[i] - (ll)x[i] + P2) * inv12 % P2;
        ll r3 = (x[i] + (ll)P1 * t1) % P3;
        ll t2 = (z[i] - r3 + P3) * inv123 % P3;
        c[i] = (x[i] + (ll)(P1 % MOD) * (t1 % MOD)
                     + (ll)p12_mod * t2) % MOD;
    }
    trim(c);
    return c;
}

void add_to(Poly &a, const Poly &b) {
    if (a.size() < b.size()) a.resize(b.size());
    for (int i = 0; i < (int)b.size(); ++i) {
        a[i] += b[i];
        if (a[i] >= MOD) a[i] -= MOD;
    }
}

int evaluate(const Poly &a, int x) {
    int ans = 0;
    for (int i = (int)a.size() - 1; i >= 0; --i)
        ans = ((ll)ans * x + a[i]) % MOD;
    return ans;
}

struct Solver {
    struct Piece {
        int start;
        Poly f, shifted;  // 本段的 f(x)、f(x-1)，使用全局变量 x
    };
    struct Node {
        vector<Piece> pieces;
    };
    struct Split {
        int root, left, right;
    };

    int n;
    vector<int> A, B, fact, ifact, beta;
    vector<vector<int>> id;
    vector<Node> nodes;

    Solver(int n, vector<int> A, vector<int> B)
        : n(n), A(move(A)), B(move(B)), fact(n + 2), ifact(n + 2),
          beta(n + 1), id(n + 2, vector<int>(n + 2, -1)) {
        fact[0] = 1;
        for (int i = 1; i <= n + 1; ++i) fact[i] = (ll)fact[i - 1] * i % MOD;
        ifact[n + 1] = power_mod(fact[n + 1], MOD - 2);
        for (int i = n + 1; i >= 1; --i) ifact[i - 1] = (ll)ifact[i] * i % MOD;

        // beta(t) = t / (1 - exp(-t))。
        // 其倒数的第 k 项系数为 (-1)^k / (k+1)!。
        beta[0] = 1;
        for (int i = 1; i <= n; ++i) {
            int sum = 0;
            for (int k = 1; k <= i; ++k) {
                int v = (ll)ifact[k + 1] * beta[i - k] % MOD;
                if (k & 1) {
                    sum -= v;
                    if (sum < 0) sum += MOD;
                } else {
                    sum += v;
                    if (sum >= MOD) sum -= MOD;
                }
            }
            beta[i] = sum ? MOD - sum : 0;
        }
        Node empty;
        empty.pieces.push_back({1, {1}, {1}});
        nodes.push_back(move(empty));  // 空区间的计数函数恒为 1。
    }

    // 离散求和：返回 q，使 q(0)=0 且 q(x)-q(x-1)=g(x)。
    Poly antidifference(const Poly &g) const {
        if (g.size() == 1 && g[0] == 0) return {0};
        int d = (int)g.size() - 1;
        Poly a(d + 1), b(d + 1);
        for (int i = 0; i <= d; ++i) {
            a[d - i] = (ll)g[i] * fact[i] % MOD;
            b[i] = beta[i];
        }
        Poly c = multiply(a, b);
        c.resize(d + 1);
        Poly q(d + 2);
        for (int i = 0; i <= d; ++i)
            q[i + 1] = (ll)c[d - i] * ifact[i + 1] % MOD;
        trim(q);
        return q;
    }

    // 用阶乘卷积计算 f(x-1)。
    Poly shift_minus_one(const Poly &f) const {
        int d = (int)f.size() - 1;
        if (d == 0) return f;
        Poly a(d + 1), b(d + 1);
        for (int i = 0; i <= d; ++i) {
            a[d - i] = (ll)f[i] * fact[i] % MOD;
            b[i] = (i & 1) ? MOD - ifact[i] : ifact[i];
        }
        Poly c = multiply(a, b);
        c.resize(d + 1);
        Poly ans(d + 1);
        for (int i = 0; i <= d; ++i)
            ans[i] = (ll)c[d - i] * ifact[i] % MOD;
        trim(ans);
        return ans;
    }

    const Piece &get_piece(int u, int x) const {
        const auto &v = nodes[u].pieces;
        int lo = 0, hi = (int)v.size();
        while (lo + 1 < hi) {
            int mid = (lo + hi) / 2;
            if (v[mid].start <= x) lo = mid;
            else hi = mid;
        }
        return v[lo];
    }

    int solve(int l, int r) {
        if (l > r) return 0;
        if (id[l][r] != -1) return id[l][r];

        vector<Split> splits;
        int low = max(l, (l + r - 1) / 2);
        int high = min(r, (l + r + 2) / 2);
        for (int k = low; k <= high; ++k) {
            int left = solve(l, k - 1);
            int right = solve(k + 1, r);
            splits.push_back({k, left, right});
        }

        vector<int> cuts{1};
        for (int i = l; i <= r; ++i) {
            cuts.push_back(A[i]);
            cuts.push_back(B[i] + 1);
        }
        sort(cuts.begin(), cuts.end());
        cuts.erase(unique(cuts.begin(), cuts.end()), cuts.end());

        Node cur;
        cur.pieces.reserve(cuts.size());
        int carry = 0;  // F(cuts[j]-1)，初始为 F(0)=0。
        for (int j = 0; j + 1 < (int)cuts.size(); ++j) {
            int lo = cuts[j], hi = cuts[j + 1] - 1;
            Poly g{0};
            for (const auto &sp : splits) {
                if (lo < A[sp.root] || lo > B[sp.root]) continue;
                const auto &left = get_piece(sp.left, lo);
                const auto &right = get_piece(sp.right, lo);
                add_to(g, multiply(left.f, right.shifted));
            }
            trim(g);
            Poly f = antidifference(g);
            int delta = carry - evaluate(f, lo - 1);
            if (delta < 0) delta += MOD;
            f[0] += delta;
            if (f[0] >= MOD) f[0] -= MOD;
            carry = evaluate(f, hi);
            Poly shifted = shift_minus_one(f);
            cur.pieces.push_back({lo, move(f), move(shifted)});
        }
        // 超过最大的 B 后，计数函数恒定。
        cur.pieces.push_back({cuts.back(), {carry}, {carry}});
        int u = (int)nodes.size();
        nodes.push_back(move(cur));
        id[l][r] = u;
        return u;
    }

    int run() {
        int root = solve(1, n);
        return nodes[root].pieces.back().f[0];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    vector<int> A(n + 1), B(n + 1);
    for (int i = 1; i <= n; ++i) cin >> A[i] >> B[i];
    Solver solver(n, move(A), move(B));
    cout << solver.run() << '\n';
    return 0;
}
