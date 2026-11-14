#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using u64 = unsigned long long;

namespace NTT {
    constexpr int MOD = 998244353;
    constexpr int G = 3;

    int power(int a, int e) {
        int r = 1;
        while (e) {
            if (e & 1) r = (int)((int64)r * a % MOD);
            a = (int)((int64)a * a % MOD);
            e >>= 1;
        }
        return r;
    }

    vector<int> roots{0, 1};
    vector<int> rev;
    int lastSize = 0;

    void transform(vector<int>& a) {
        int n = (int)a.size();
        if (n == 1) return;

        if (lastSize != n) {
            lastSize = n;
            rev.assign(n, 0);
            int lg = __builtin_ctz((unsigned)n);
            for (int i = 1; i < n; ++i)
                rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (lg - 1));
        }

        if ((int)roots.size() < n) {
            int k = __builtin_ctz((unsigned)roots.size());
            roots.resize(n);
            while ((1 << k) < n) {
                int z = power(G, (MOD - 1) >> (k + 1));
                for (int i = 1 << (k - 1); i < (1 << k); ++i) {
                    roots[2 * i] = roots[i];
                    roots[2 * i + 1] = (int)((int64)roots[i] * z % MOD);
                }
                ++k;
            }
        }

        for (int i = 0; i < n; ++i)
            if (i < rev[i]) swap(a[i], a[rev[i]]);

        for (int len = 1; len < n; len <<= 1) {
            for (int i = 0; i < n; i += 2 * len) {
                for (int j = 0; j < len; ++j) {
                    int u = a[i + j];
                    int v = (int)((int64)a[i + j + len] * roots[len + j] % MOD);
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

    void inverse(vector<int>& a) {
        transform(a);
        reverse(a.begin() + 1, a.end());
        int inv = power((int)a.size(), MOD - 2);
        for (int& x : a) x = (int)((int64)x * inv % MOD);
    }
}

int64 extendedGcd(int64 a, int64 b, int64& x, int64& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    int64 x1, y1;
    int64 g = extendedGcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

int64 inverseModulo(int64 a, int64 mod) {
    if (mod == 1) return 0;
    int64 x, y;
    extendedGcd(a, mod, x, y);
    x %= mod;
    if (x < 0) x += mod;
    return x;
}

struct Level {
    u64 L, M;
};

struct Solver {
    int p;
    u64 n, c, P;
    int digits;
    int choose[5][5]{};
    vector<int> cd;
    vector<Level> levels;
    vector<pair<int, int>> changes;

    u64 Q, R;
    int lowDigits;
    vector<pair<u64, int>> lowSupport;
    vector<int> lowDense, high;
    vector<int> highTransform;
    int transformSize = 1;

    static constexpr int logs[11] = {0, 0, 8, 7, 3, 7, 6, 3, 7, 2, 5};

    Solver(u64 n_, u64 c_, int p_) : p(p_), n(n_), c(c_) {
        for (int i = 0; i < p; ++i) {
            choose[i][0] = choose[i][i] = 1;
            for (int j = 1; j < i; ++j)
                choose[i][j] = (choose[i - 1][j - 1] + choose[i - 1][j]) % p;
        }

        P = 1;
        digits = 0;
        while (P <= c) {
            P *= p;
            ++digits;
        }

        u64 t = c;
        for (int i = 0; i < digits; ++i) {
            cd.push_back((int)(t % p));
            t /= p;
        }

        for (u64 L = 1; L <= n; L *= 11)
            levels.push_back({L, L * 11});

        for (int d = 1; d <= 11; ++d) {
            int delta = logs[d % 11] - logs[d - 1];
            delta %= p;
            if (delta < 0) delta += p;
            if (delta) changes.push_back({d, delta});
        }

        selectSplit();
        buildWeights();
    }

    int lucas(u64 z) const {
        int result = 1;
        for (int i = 0; i < digits; ++i) {
            int d = (int)(z % p);
            if (d < cd[i]) return 0;
            result = result * choose[d][cd[i]] % p;
            z /= p;
        }
        return result;
    }

    void selectSplit() {
        long double best = numeric_limits<long double>::infinity();
        u64 q = 1, support = 1;

        for (int s = 0; s <= digits; ++s) {
            u64 r = P / q;
            if (r <= 262144) {
                int len = 1, lg = 0;
                while ((u64)len < 2 * r - 1) {
                    len <<= 1;
                    ++lg;
                }

                bool dense = q <= 262144;
                long double cost = dense ? (long double)q : 0;
                cost += (long double)r * 3 + support;

                for (auto level : levels) {
                    u64 D = (n / level.M) % P;
                    if (!D) continue;

                    long double direct =
                        (long double)D * changes.size() *
                        (dense ? 3 : 2 * digits);

                    long double sparse =
                        (long double)support * changes.size() * 9;

                    if (D / q && r > 1)
                        sparse += (long double)len * lg * 2;

                    cost += min(direct, sparse);
                }

                if (cost < best) {
                    best = cost;
                    Q = q;
                    R = r;
                    lowDigits = s;
                }
            }

            if (s < digits) {
                support *= p - cd[s];
                q *= p;
            }
        }
    }

    void buildWeights() {
        lowSupport.push_back({0, 1});
        u64 place = 1;

        for (int i = 0; i < lowDigits; ++i) {
            vector<pair<u64, int>> next;
            next.reserve(lowSupport.size() * (p - cd[i]));
            for (auto [x, w] : lowSupport) {
                for (int d = cd[i]; d < p; ++d)
                    next.push_back({
                        x + place * d,
                        w * choose[d][cd[i]] % p
                    });
            }
            lowSupport.swap(next);
            place *= p;
        }

        if (Q <= 262144) {
            lowDense.assign((size_t)Q, 0);
            for (auto [x, w] : lowSupport) lowDense[(size_t)x] = w;
        }

        high.assign(1, 1);
        int oldSize = 1;
        for (int i = lowDigits; i < digits; ++i) {
            vector<int> next(oldSize * p, 0);
            for (int d = cd[i]; d < p; ++d)
                for (int x = 0; x < oldSize; ++x)
                    next[d * oldSize + x] =
                        high[x] * choose[d][cd[i]] % p;
            high.swap(next);
            oldSize *= p;
        }

        while ((u64)transformSize < 2 * R - 1) transformSize <<= 1;
    }

    vector<int> makeH(u64 A, u64 V) {
        vector<int> H((size_t)R, 0);
        if (!V) return H;

        if (highTransform.empty()) {
            highTransform.assign(transformSize, 0);
            copy(high.begin(), high.end(), highTransform.begin());
            NTT::transform(highTransform);
        }

        vector<int> work(transformSize, 0);
        u64 step = (R - A % R) % R;
        u64 x = 0;
        for (u64 v = 0; v < V; ++v) {
            work[(size_t)x] = 1;
            x += step;
            if (x >= R) x -= R;
        }

        NTT::transform(work);
        for (int i = 0; i < transformSize; ++i)
            work[i] = (int)((int64)work[i] * highTransform[i] % NTT::MOD);
        NTT::inverse(work);

        for (u64 i = 0; i < R; ++i) {
            int value = work[(size_t)i];
            if (i + R < (u64)transformSize)
                value += work[(size_t)(i + R)];
            H[(size_t)i] = value % p;
        }
        return H;
    }

    int directSum(u64 A, u64 B, u64 D) const {
        int64 sum = 0;
        u64 z = B;

        if (!lowDense.empty()) {
            for (u64 i = 0; i < D; ++i) {
                sum += lowDense[(size_t)(z % Q)] * high[(size_t)(z / Q)];
                z += A;
                if (z >= P) z -= P;
            }
        } else {
            for (u64 i = 0; i < D; ++i) {
                sum += lucas(z);
                z += A;
                if (z >= P) z -= P;
            }
        }
        return (int)(sum % p);
    }

    int sparseSum(
        u64 A, u64 B, u64 D, u64 invA,
        const vector<int>& H
    ) const {
        u64 V = D / Q, U = D % Q;
        u64 offset = (invA * (B % Q)) % Q;
        u64 shift = R == 1 ? 0 : ((A % R) * V) % R;
        int64 sum = 0;

        for (auto [x, weight] : lowSupport) {
            u64 u = (invA * x) % Q;
            if (u >= offset) u -= offset;
            else u += Q - offset;

            if (!V && u >= U) continue;

            u64 h = R == 1 ? 0 : ((A * u + B) / Q) % R;
            if (V) sum += weight * H[(size_t)h];

            if (u < U) {
                u64 h2 = h + shift;
                if (h2 >= R) h2 -= R;
                sum += weight * high[(size_t)h2];
            }
        }

        return (int)(sum % p);
    }

    int solve() {
        int answer = 0;

        for (auto level : levels) {
            u64 L = level.L, M = level.M;
            u64 blocks = n / M;
            u64 remainder = n % M;
            u64 D = blocks % P;

            if (D) {
                u64 A = (P - M % P) % P;

                long double directCost =
                    (long double)D * changes.size() *
                    (lowDense.empty() ? 2 * digits : 3);

                long double sparseCost =
                    (long double)lowSupport.size() * changes.size() * 9;

                if (D / Q && R > 1) {
                    int lg = __builtin_ctz((unsigned)transformSize);
                    sparseCost +=
                        (long double)transformSize * lg * 2;
                }

                bool useDirect = directCost <= sparseCost;
                vector<int> H;
                u64 invA = 0;

                if (!useDirect) {
                    invA = inverseModulo((int64)(A % Q), (int64)Q);
                    if (D / Q) H = makeH(A, D / Q);
                }

                for (auto [d, delta] : changes) {
                    u64 B = (n + c - (u64)d * L) % P;
                    int value = useDirect
                        ? directSum(A, B, D)
                        : sparseSum(A, B, D, invA, H);
                    answer = (answer + delta * value) % p;
                }
            }

            for (auto [d, delta] : changes) {
                u64 boundary = (u64)d * L;
                if (boundary <= remainder) {
                    int value = lucas(remainder + c - boundary);
                    answer = (answer + delta * value) % p;
                }
            }
        }

        return answer;
    }
};

constexpr int Solver::logs[11];

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    constexpr int powersOfTwo[10] = {1, 2, 4, 8, 5, 10, 9, 7, 3, 6};

    while (T--) {
        u64 n, c;
        cin >> n >> c;

        int e2 = Solver(n, c, 2).solve();
        int e5 = Solver(n, c, 5).solve();

        int exponent = e5;
        if ((exponent & 1) != e2) exponent += 5;

        cout << powersOfTwo[exponent] << '\n';
    }

    return 0;
}
