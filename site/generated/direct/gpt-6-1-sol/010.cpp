#include <bits/stdc++.h>
using namespace std;

namespace Poly {
    constexpr int MOD = 998244353;
    constexpr int ROOT = 3;
    using P = vector<int>;

    int power(int a, long long e) {
        int r = 1;
        while (e) {
            if (e & 1) r = (long long)r * a % MOD;
            a = (long long)a * a % MOD;
            e >>= 1;
        }
        return r;
    }

    void trim(P& a) {
        while (a.size() > 1 && a.back() == 0) a.pop_back();
    }

    void ntt(P& a) {
        int n = (int)a.size();
        if (n == 1) return;

        static vector<int> rev;
        static vector<int> roots{0, 1};

        if ((int)rev.size() != n) {
            rev.resize(n);
            int lg = __builtin_ctz((unsigned)n);
            for (int i = 0; i < n; ++i) {
                rev[i] = (rev[i >> 1] >> 1) |
                         ((i & 1) << (lg - 1));
            }
        }

        if ((int)roots.size() < n) {
            int k = __builtin_ctz((unsigned)roots.size());
            roots.resize(n);
            while ((1 << k) < n) {
                int z = power(ROOT, (MOD - 1) >> (k + 1));
                for (int i = 1 << (k - 1); i < (1 << k); ++i) {
                    roots[2 * i] = roots[i];
                    roots[2 * i + 1] = (long long)roots[i] * z % MOD;
                }
                ++k;
            }
        }

        for (int i = 0; i < n; ++i) {
            if (i < rev[i]) swap(a[i], a[rev[i]]);
        }

        for (int len = 1; len < n; len <<= 1) {
            for (int i = 0; i < n; i += 2 * len) {
                for (int j = 0; j < len; ++j) {
                    int u = a[i + j];
                    int v = (long long)a[i + j + len] *
                            roots[len + j] % MOD;
                    int x = u + v;
                    if (x >= MOD) x -= MOD;
                    int y = u - v;
                    if (y < 0) y += MOD;
                    a[i + j] = x;
                    a[i + j + len] = y;
                }
            }
        }
    }

    void intt(P& a) {
        reverse(a.begin() + 1, a.end());
        ntt(a);
        int inv = power((int)a.size(), MOD - 2);
        for (int& x : a) x = (long long)x * inv % MOD;
    }

    P multiply(const P& a, const P& b) {
        if (a.empty() || b.empty()) return {};

        if (a.size() == 1) {
            P c = b;
            for (int& x : c) x = (long long)x * a[0] % MOD;
            return c;
        }
        if (b.size() == 1) {
            P c = a;
            for (int& x : c) x = (long long)x * b[0] % MOD;
            return c;
        }

        int need = (int)a.size() + (int)b.size() - 1;

        if (min(a.size(), b.size()) <= 16 ||
            (long long)a.size() * b.size() <= 1024) {
            P c(need);
            for (int i = 0; i < (int)a.size(); ++i) {
                for (int j = 0; j < (int)b.size(); ++j) {
                    c[i + j] =
                        (c[i + j] + (long long)a[i] * b[j]) % MOD;
                }
            }
            return c;
        }

        int n = 1;
        while (n < need) n <<= 1;

        P fa(a.begin(), a.end()), fb(b.begin(), b.end());
        fa.resize(n);
        fb.resize(n);
        ntt(fa);
        ntt(fb);
        for (int i = 0; i < n; ++i)
            fa[i] = (long long)fa[i] * fb[i] % MOD;
        intt(fa);
        fa.resize(need);
        return fa;
    }

    // Inverse modulo x^need.
    P inverse(const P& f, int need) {
        P g{f[0] == 1 ? 1 : power(f[0], MOD - 2)};

        for (int m = 1; m < need; m <<= 1) {
            int len = 2 * m;
            P a(len), b(len);

            for (int i = 0; i < len && i < (int)f.size(); ++i)
                a[i] = f[i];
            for (int i = 0; i < m; ++i)
                b[i] = g[i];

            ntt(a);
            ntt(b);
            for (int i = 0; i < len; ++i)
                a[i] = (long long)a[i] * b[i] % MOD;
            intt(a);

            fill(a.begin(), a.begin() + m, 0);
            ntt(a);
            for (int i = 0; i < len; ++i)
                a[i] = (long long)a[i] * b[i] % MOD;
            intt(a);

            g.resize(len);
            for (int i = m; i < len; ++i)
                g[i] = a[i] == 0 ? 0 : MOD - a[i];
        }

        g.resize(need);
        return g;
    }

    // b is monic.
    P remainder(P a, const P& b) {
        trim(a);
        if (a.size() < b.size()) return a;

        int quotientSize = (int)a.size() - (int)b.size() + 1;
        int degreeB = (int)b.size() - 1;

        if (quotientSize <= 32 || b.size() <= 65) {
            for (int i = (int)a.size() - 1; i >= degreeB; --i) {
                int coefficient = a[i];
                if (!coefficient) continue;

                int start = i - degreeB;
                for (int j = 0; j < degreeB; ++j) {
                    int sub = (long long)coefficient * b[j] % MOD;
                    int& x = a[start + j];
                    x -= sub;
                    if (x < 0) x += MOD;
                }
            }
            a.resize(degreeB);
            if (a.empty()) a.push_back(0);
            trim(a);
            return a;
        }

        P reversedA(quotientSize);
        for (int i = 0; i < quotientSize; ++i)
            reversedA[i] = a[(int)a.size() - 1 - i];

        P reversedB(min((int)b.size(), quotientSize));
        for (int i = 0; i < (int)reversedB.size(); ++i)
            reversedB[i] = b[(int)b.size() - 1 - i];

        P inverseB = inverse(reversedB, quotientSize);
        P quotient = multiply(reversedA, inverseB);
        quotient.resize(quotientSize);
        reverse(quotient.begin(), quotient.end());

        P product = multiply(quotient, b);
        a.resize(degreeB);
        for (int i = 0; i < degreeB; ++i) {
            a[i] -= product[i];
            if (a[i] < 0) a[i] += MOD;
        }

        if (a.empty()) a.push_back(0);
        trim(a);
        return a;
    }

    // Multiply p by x + c.
    void appendLinear(P& p, int c) {
        int oldSize = (int)p.size();
        p.push_back(0);
        for (int i = oldSize; i >= 1; --i) {
            p[i] = (p[i - 1] + (long long)c * p[i]) % MOD;
        }
        p[0] = (long long)p[0] * c % MOD;
    }

    int evaluate(const P& p, int x) {
        int result = 0;
        for (int i = (int)p.size() - 1; i >= 0; --i)
            result = ((long long)result * x + p[i]) % MOD;
        return result;
    }
}

constexpr int MOD = Poly::MOD;
constexpr int MAXV = 200000;

int fact[MAXV + 1], invFact[MAXV + 1];

struct PrefixProductEvaluator {
    using P = Poly::P;
    static constexpr int BLOCK = 64;

    int n, N;
    const vector<int>& event;
    const vector<int>& count;
    const vector<int>& invFalling;

    vector<P> factors;
    vector<P> points;
    int answer = 0;

    PrefixProductEvaluator(
        int n_,
        const vector<int>& event_,
        const vector<int>& count_,
        const vector<int>& invFalling_
    ) : n(n_), N(2 * n_), event(event_), count(count_),
        invFalling(invFalling_), factors(4 * n_ + 8),
        points(4 * n_ + 8) {}

    void build(int node, int l, int r) {
        if (r - l + 1 <= BLOCK) {
            factors[node] = {1};
            points[node] = {1};

            for (int t = l; t <= r; ++t) {
                if (event[t] != -1)
                    Poly::appendLinear(factors[node], event[t]);

                int x = N - t;
                Poly::appendLinear(points[node], MOD - x);
            }
            return;
        }

        int mid = (l + r) / 2;
        build(node * 2, l, mid);
        build(node * 2 + 1, mid + 1, r);

        factors[node] =
            Poly::multiply(factors[node * 2], factors[node * 2 + 1]);
        points[node] =
            Poly::multiply(points[node * 2], points[node * 2 + 1]);
    }

    void solve(int node, int l, int r, P prefix) {
        prefix = Poly::remainder(move(prefix), points[node]);

        if (r - l + 1 <= BLOCK) {
            vector<int> localFactors;
            localFactors.reserve(r - l + 1);

            for (int t = l; t <= r; ++t) {
                if (event[t] != -1)
                    localFactors.push_back(event[t]);

                int x = N - t;
                int value = Poly::evaluate(prefix, x);

                for (int d : localFactors)
                    value = (long long)value * (x + d) % MOD;

                value = (long long)value * invFalling[count[t]] % MOD;
                answer += value;
                if (answer >= MOD) answer -= MOD;
            }
            return;
        }

        int mid = (l + r) / 2;

        solve(node * 2, l, mid, prefix);

        P rightPrefix = Poly::multiply(prefix, factors[node * 2]);
        solve(node * 2 + 1, mid + 1, r, move(rightPrefix));
    }

    int run() {
        build(1, 1, n);
        solve(1, 1, n, P{1});
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    fact[0] = 1;
    for (int i = 1; i <= MAXV; ++i)
        fact[i] = (long long)fact[i - 1] * i % MOD;

    invFact[MAXV] = Poly::power(fact[MAXV], MOD - 2);
    for (int i = MAXV; i >= 1; --i)
        invFact[i - 1] = (long long)invFact[i] * i % MOD;

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;
        int N = 2 * n;

        vector<int> a(m + 1);
        for (int i = 1; i <= m; ++i) cin >> a[i];

        vector<long long> f(m + 2);
        vector<int> nextOccurrence(N + 1, 0);

        for (int i = m; i >= 1; --i) {
            f[i] = (i == m ? 0 : f[i + 1] + 1);
            int j = nextOccurrence[a[i]];
            if (j)
                f[i] = max(f[i], f[j] + n + 1LL);
            nextOccurrence[a[i]] = i;
        }

        vector<int> weights;
        for (int i = 1; i <= m; ++i) {
            long long w = f[1] - f[i];
            if (w >= n) break;
            weights.push_back((int)w);
        }

        int k = (int)weights.size();
        vector<int> d(k);
        bool allZero = true;
        for (int i = 0; i < k; ++i) {
            d[i] = weights[i] - i;
            if (d[i] != 0) allZero = false;
        }

        vector<int> invFalling(k + 1);
        for (int i = 0; i <= k; ++i) {
            invFalling[i] =
                (long long)fact[N - i] * invFact[N] % MOD;
        }

        int sumProbabilities = 0;

        if (allZero) {
            for (int t = 1; t <= n; ++t) {
                int used = min(t, k);
                int probability = Poly::power(N - t, used);
                probability =
                    (long long)probability * invFalling[used] % MOD;

                sumProbabilities += probability;
                if (sumProbabilities >= MOD)
                    sumProbabilities -= MOD;
            }
        } else if (k <= 48) {
            int used = 0;
            for (int t = 1; t <= n; ++t) {
                while (used < k && weights[used] < t) ++used;

                int probability = 1;
                for (int i = 0; i < used; ++i)
                    probability =
                        (long long)probability * (N - t + d[i]) % MOD;

                probability =
                    (long long)probability * invFalling[used] % MOD;

                sumProbabilities += probability;
                if (sumProbabilities >= MOD)
                    sumProbabilities -= MOD;
            }
        } else {
            vector<int> event(n + 1, -1);
            for (int i = 0; i < k; ++i)
                event[weights[i] + 1] = d[i];

            vector<int> count(n + 1);
            for (int t = 1; t <= n; ++t)
                count[t] = count[t - 1] + (event[t] != -1);

            PrefixProductEvaluator evaluator(
                n, event, count, invFalling
            );
            sumProbabilities = evaluator.run();
        }

        long long answer = (f[1] + 1) % MOD;
        answer = (answer + n - sumProbabilities + MOD) % MOD;
        cout << answer << '\n';
    }

    return 0;
}
