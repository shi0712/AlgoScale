#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int ROOT = 3;

using Poly = vector<int>;

int modpow(int a, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r = (long long)r * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return r;
}

void trim(Poly &a) {
    while (!a.empty() && a.back() == 0) a.pop_back();
}

int addmod(int a, int b) {
    int c = a + b;
    if (c >= MOD) c -= MOD;
    return c;
}

int submod(int a, int b) {
    int c = a - b;
    if (c < 0) c += MOD;
    return c;
}

namespace NTT {
    vector<int> roots{0, 1};
    vector<vector<int>> reversals(24);
    vector<int> invLength(24, 0);

    void forward(Poly &a) {
        int n = (int)a.size();
        if (n == 1) return;

        int lg = __builtin_ctz((unsigned)n);

        if ((int)roots.size() < n) {
            int k = __builtin_ctz((unsigned)roots.size());
            roots.resize(n);
            while ((1 << k) < n) {
                int z = modpow(ROOT, (MOD - 1) >> (k + 1));
                for (int i = 1 << (k - 1); i < (1 << k); ++i) {
                    roots[i << 1] = roots[i];
                    roots[i << 1 | 1] = (long long)roots[i] * z % MOD;
                }
                ++k;
            }
        }

        auto &rev = reversals[lg];
        if (rev.empty()) {
            rev.resize(n);
            for (int i = 1; i < n; ++i) {
                rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (lg - 1));
            }
        }

        for (int i = 0; i < n; ++i) {
            if (i < rev[i]) swap(a[i], a[rev[i]]);
        }

        for (int len = 1; len < n; len <<= 1) {
            for (int start = 0; start < n; start += len << 1) {
                for (int j = 0; j < len; ++j) {
                    int u = a[start + j];
                    int v = (long long)a[start + len + j] * roots[len + j] % MOD;
                    a[start + j] = addmod(u, v);
                    a[start + len + j] = submod(u, v);
                }
            }
        }
    }

    void inverse(Poly &a) {
        int n = (int)a.size();
        if (n == 1) return;

        reverse(a.begin() + 1, a.end());
        forward(a);

        int lg = __builtin_ctz((unsigned)n);
        if (!invLength[lg]) invLength[lg] = modpow(n, MOD - 2);
        int invn = invLength[lg];

        for (int &x : a) x = (long long)x * invn % MOD;
    }
}

Poly multiply(const Poly &a, const Poly &b) {
    if (a.empty() || b.empty()) return {};

    if (min(a.size(), b.size()) <= 16 ||
        (long long)a.size() * b.size() <= 1024) {
        Poly c(a.size() + b.size() - 1);
        for (int i = 0; i < (int)a.size(); ++i) {
            for (int j = 0; j < (int)b.size(); ++j) {
                c[i + j] = (c[i + j] + (long long)a[i] * b[j]) % MOD;
            }
        }
        trim(c);
        return c;
    }

    int need = (int)a.size() + (int)b.size() - 1;
    int n = 1;
    while (n < need) n <<= 1;

    Poly fa(n), fb(n);
    copy(a.begin(), a.end(), fa.begin());
    copy(b.begin(), b.end(), fb.begin());

    NTT::forward(fa);
    NTT::forward(fb);
    for (int i = 0; i < n; ++i) {
        fa[i] = (long long)fa[i] * fb[i] % MOD;
    }
    NTT::inverse(fa);
    fa.resize(need);
    trim(fa);
    return fa;
}

Poly addPoly(const Poly &a, const Poly &b) {
    Poly c(max(a.size(), b.size()));
    for (int i = 0; i < (int)c.size(); ++i) {
        int x = i < (int)a.size() ? a[i] : 0;
        int y = i < (int)b.size() ? b[i] : 0;
        c[i] = addmod(x, y);
    }
    trim(c);
    return c;
}

// Formal power series inverse, modulo x^need.
Poly inversePoly(const Poly &f, int need) {
    if (need == 0) return {};

    Poly b{modpow(f[0], MOD - 2)};
    int len = 1;

    while (len < need) {
        int n = len << 1;
        Poly fa(n), fb(n);

        for (int i = 0; i < n && i < (int)f.size(); ++i) fa[i] = f[i];
        copy(b.begin(), b.end(), fb.begin());

        NTT::forward(fa);
        NTT::forward(fb);
        for (int i = 0; i < n; ++i) {
            fa[i] = (long long)fa[i] * fb[i] % MOD;
        }
        NTT::inverse(fa);

        // Only the high half of f*b - 1 is needed.
        fill(fa.begin(), fa.begin() + len, 0);
        NTT::forward(fa);
        for (int i = 0; i < n; ++i) {
            fa[i] = (long long)fa[i] * fb[i] % MOD;
        }
        NTT::inverse(fa);

        b.resize(n);
        for (int i = len; i < n; ++i) {
            b[i] = fa[i] ? MOD - fa[i] : 0;
        }
        len = n;
    }

    b.resize(need);
    return b;
}

/*
 * Fast multipoint evaluation at
 *     0, -1, -2, ..., -(count-1).
 *
 * A product tree is shared by the two evaluations needed for one run.
 */
class ConsecutiveEvaluator {
    static constexpr int LEAF = 32;

    struct Node {
        int l = 0, r = 0;
        int left = -1, right = -1;
        Poly product;
        Poly invReversed;
    };

    vector<Node> tree;
    int root;
    int count;

    int build(int l, int r) {
        int id = (int)tree.size();
        tree.emplace_back();
        tree[id].l = l;
        tree[id].r = r;

        if (r - l <= LEAF) {
            Poly p{1};
            for (int j = l; j < r; ++j) {
                Poly q(p.size() + 1);
                for (int i = 0; i < (int)p.size(); ++i) {
                    q[i] = (q[i] + (long long)p[i] * j) % MOD;
                    q[i + 1] = addmod(q[i + 1], p[i]);
                }
                p.swap(q);
            }
            tree[id].product = move(p);
        } else {
            int mid = (l + r) >> 1;
            int a = build(l, mid);
            int b = build(mid, r);
            tree[id].left = a;
            tree[id].right = b;
            tree[id].product = multiply(tree[a].product, tree[b].product);
        }

        return id;
    }

    // From 1/rev(parent), obtain 1/rev(child) by multiplying rev(sibling).
    void prepare(int id) {
        int a = tree[id].left;
        if (a == -1) return;
        int b = tree[id].right;

        auto makeInverse = [&](int child, int sibling) {
            int need = tree[sibling].r - tree[sibling].l;

            Poly x(need), y(need);
            for (int i = 0; i < need; ++i) {
                x[i] = tree[id].invReversed[i];
                y[i] = tree[sibling].product[
                    (int)tree[sibling].product.size() - 1 - i
                ];
            }

            Poly z = multiply(x, y);
            z.resize(need);
            tree[child].invReversed = move(z);
        };

        makeInverse(a, b);
        makeInverse(b, a);
        prepare(a);
        prepare(b);
    }

    Poly remainder(const Poly &f, int id) const {
        const Poly &g = tree[id].product;
        int degree = (int)g.size() - 1;
        if ((int)f.size() <= degree) return f;

        int qlen = (int)f.size() - degree;

        Poly rf(qlen), inv(qlen);
        for (int i = 0; i < qlen; ++i) {
            rf[i] = f[(int)f.size() - 1 - i];
            inv[i] = tree[id].invReversed[i];
        }

        Poly q = multiply(rf, inv);
        q.resize(qlen);
        reverse(q.begin(), q.end());

        // Only coefficients below degree are needed.
        Poly lowG(g.begin(), g.begin() + degree);
        Poly t = multiply(q, lowG);

        Poly result(degree);
        for (int i = 0; i < degree; ++i) {
            int x = i < (int)f.size() ? f[i] : 0;
            int y = i < (int)t.size() ? t[i] : 0;
            result[i] = submod(x, y);
        }
        trim(result);
        return result;
    }

    void evaluateRec(int id, const Poly &f, Poly &answer) const {
        if (tree[id].left == -1) {
            for (int j = tree[id].l; j < tree[id].r; ++j) {
                int x = j == 0 ? 0 : MOD - j;
                int value = 0;
                for (int i = (int)f.size() - 1; i >= 0; --i) {
                    value = ((long long)value * x + f[i]) % MOD;
                }
                answer[j] = value;
            }
            return;
        }

        int a = tree[id].left;
        int b = tree[id].right;
        Poly fa = remainder(f, a);
        Poly fb = remainder(f, b);
        evaluateRec(a, fa, answer);
        evaluateRec(b, fb, answer);
    }

public:
    explicit ConsecutiveEvaluator(int count) : count(count) {
        tree.reserve(4 * (count / LEAF + 2));
        root = build(0, count);

        if (tree[root].left != -1) {
            int need = (count + 1) / 2;
            Poly reversed(need);
            const Poly &p = tree[root].product;
            for (int i = 0; i < need; ++i) {
                reversed[i] = p[(int)p.size() - 1 - i];
            }
            tree[root].invReversed = inversePoly(reversed, need);
            prepare(root);
        }
    }

    Poly evaluate(const Poly &f) const {
        Poly result(count);
        evaluateRec(root, f, result);
        return result;
    }
};

/*
 * The polynomial sequence
 *
 * g_0(t) = g_1(t) = 1,
 * g_m(t) = m*g_{m-1}(t) + (m-1)*(t-1)*g_{m-2}(t).
 *
 * Compute a pair of consecutive polynomials with a product of 2x2
 * polynomial matrices.
 */
struct Matrix {
    array<Poly, 4> p; // row-major
};

Poly transitionEntry(const Poly &a, const Poly &b, int i) {
    Poly c(max(a.size(), b.empty() ? size_t(0) : b.size() + 1));

    for (int j = 0; j < (int)a.size(); ++j) {
        c[j] = (long long)a[j] * i % MOD;
    }
    for (int j = 0; j < (int)b.size(); ++j) {
        int v = (long long)b[j] * (i - 1) % MOD;
        c[j] = submod(c[j], v);
        c[j + 1] = addmod(c[j + 1], v);
    }
    trim(c);
    return c;
}

Matrix matrixMultiply(Matrix a, Matrix b) {
    int maxA = 0, maxB = 0;
    for (const auto &p : a.p) maxA = max(maxA, (int)p.size());
    for (const auto &p : b.p) maxB = max(maxB, (int)p.size());

    int n = 1;
    while (n < maxA + maxB - 1) n <<= 1;

    array<int, 4> outputSize{};
    auto termSize = [](const Poly &x, const Poly &y) -> int {
        if (x.empty() || y.empty()) return 0;
        return (int)x.size() + (int)y.size() - 1;
    };

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            outputSize[i * 2 + j] = max(
                termSize(a.p[i * 2], b.p[j]),
                termSize(a.p[i * 2 + 1], b.p[2 + j])
            );
        }
    }

    auto transform = [&](Poly &p) {
        if (p.empty()) {
            p.assign(n, 0);
        } else if (p.size() == 1) {
            p.assign(n, p[0]);
        } else {
            p.resize(n);
            NTT::forward(p);
        }
    };

    for (auto &p : a.p) transform(p);
    for (auto &p : b.p) transform(p);

    for (int k = 0; k < n; ++k) {
        int a00 = a.p[0][k], a01 = a.p[1][k];
        int a10 = a.p[2][k], a11 = a.p[3][k];
        int b00 = b.p[0][k], b01 = b.p[1][k];
        int b10 = b.p[2][k], b11 = b.p[3][k];

        a.p[0][k] = ((long long)a00 * b00 + (long long)a01 * b10) % MOD;
        a.p[1][k] = ((long long)a00 * b01 + (long long)a01 * b11) % MOD;
        a.p[2][k] = ((long long)a10 * b00 + (long long)a11 * b10) % MOD;
        a.p[3][k] = ((long long)a10 * b01 + (long long)a11 * b11) % MOD;
    }

    for (int i = 0; i < 4; ++i) {
        NTT::inverse(a.p[i]);
        a.p[i].resize(outputSize[i]);
        trim(a.p[i]);
    }
    return a;
}

Matrix matrixProduct(int l, int r) {
    if (r - l + 1 <= 32) {
        Matrix a;
        a.p[0] = {1};
        a.p[3] = {1};

        for (int i = l; i <= r; ++i) {
            Poly c0 = transitionEntry(a.p[0], a.p[2], i);
            Poly c1 = transitionEntry(a.p[1], a.p[3], i);
            a.p[2] = move(a.p[0]);
            a.p[3] = move(a.p[1]);
            a.p[0] = move(c0);
            a.p[1] = move(c1);
        }
        return a;
    }

    int mid = (l + r) >> 1;
    Matrix left = matrixProduct(l, mid);
    Matrix right = matrixProduct(mid + 1, r);
    return matrixMultiply(move(right), move(left));
}

vector<int> fact, invfact;

struct Arm {
    Poly previous; // A_{m-1}(x)
    Poly difference; // (A_{m-1}(x) - A_m(x)) / (1-x)
};

class ArmCalculator {
    vector<unique_ptr<Arm>> cache;

    /*
     * Given g_m(0), g_m(-1), ..., transform them into A_m(x).
     *
     * Let d = floor(m/2), f(j) = g_m(-j)/m!.
     * If sum f(j)y^j = E(y)/(1-y)^(d+1), then
     *
     * A_m(x) = (1-x)^d E(-x/(1-x)).
     *
     * Both transformations are factorial convolutions.
     */
    Poly transformValues(Poly values, int m) {
        int d = m / 2;
        values.resize(d + 1);
        for (int &x : values) x = (long long)x * invfact[m] % MOD;

        Poly binomial(d + 1);
        for (int i = 0; i <= d; ++i) {
            int x = (long long)fact[d + 1] * invfact[i] % MOD
                    * invfact[d + 1 - i] % MOD;
            binomial[i] = (i & 1) && x ? MOD - x : x;
        }

        Poly e = multiply(values, binomial);
        e.resize(d + 1);

        Poly a(d + 1), b(d + 1);
        for (int i = 0; i <= d; ++i) {
            a[i] = (long long)e[i] * fact[d - i] % MOD;
            b[i] = invfact[i];
        }

        Poly result = multiply(a, b);
        result.resize(d + 1);
        for (int i = 0; i <= d; ++i) {
            result[i] = (long long)result[i] * invfact[d - i] % MOD;
            if ((i & 1) && result[i]) result[i] = MOD - result[i];
        }
        trim(result);
        return result;
    }

public:
    explicit ArmCalculator(int n) : cache(n + 1) {}

    const Arm &get(int m) {
        if (cache[m]) return *cache[m];

        auto answer = make_unique<Arm>();

        if (m == 1) {
            answer->previous = {1};
            cache[m] = move(answer);
            return *cache[m];
        }

        Matrix product = matrixProduct(2, m);
        Poly gm = addPoly(product.p[0], product.p[1]);
        Poly gp = addPoly(product.p[2], product.p[3]);

        int d = m / 2;
        ConsecutiveEvaluator evaluator(d + 1);

        Poly vm = evaluator.evaluate(gm);
        Poly vp = evaluator.evaluate(gp);

        Poly current = transformValues(move(vm), m);
        Poly previous = transformValues(move(vp), m - 1);

        int size = max(current.size(), previous.size());
        Poly difference(size);
        int prefix = 0;
        for (int i = 0; i < size; ++i) {
            int x = i < (int)previous.size() ? previous[i] : 0;
            int y = i < (int)current.size() ? current[i] : 0;
            prefix = addmod(prefix, submod(x, y));
            difference[i] = prefix;
        }
        trim(difference);

        answer->previous = move(previous);
        answer->difference = move(difference);
        cache[m] = move(answer);
        return *cache[m];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    string s;
    cin >> n >> s;

    fact.resize(n + 2);
    invfact.resize(n + 2);
    fact[0] = 1;
    for (int i = 1; i <= n + 1; ++i) {
        fact[i] = (long long)fact[i - 1] * i % MOD;
    }
    invfact[n + 1] = modpow(fact[n + 1], MOD - 2);
    for (int i = n + 1; i >= 1; --i) {
        invfact[i - 1] = (long long)invfact[i] * i % MOD;
    }

    vector<pair<char, int>> runs;
    for (char c : s) {
        if (runs.empty() || runs.back().first != c) {
            runs.emplace_back(c, 1);
        } else {
            ++runs.back().second;
        }
    }

    ArmCalculator calculator(n);

    vector<Poly> pool;
    priority_queue<pair<int, int>,
                   vector<pair<int, int>>,
                   greater<pair<int, int>>> heap;

    int blocks = 0;
    int scalar = 1;

    auto addFactor = [&](Poly p) {
        trim(p);
        if (p.size() == 1) {
            scalar = (long long)scalar * p[0] % MOD;
            return;
        }
        int id = (int)pool.size();
        heap.emplace((int)p.size(), id);
        pool.push_back(move(p));
    };

    int i = 0;
    while (i < (int)runs.size()) {
        ++blocks;

        // An outward-facing prefix L-run or suffix R-run.
        if (runs[i].first == 'L' || i + 1 == (int)runs.size()) {
            const Arm &a = calculator.get(runs[i].second);
            addFactor(a.previous);
            ++i;
            continue;
        }

        // An inward-facing block R^a L^b.
        int aLength = runs[i].second;
        int bLength = runs[i + 1].second;
        const Arm &a = calculator.get(aLength);
        const Arm &b = calculator.get(bLength);

        Poly p = multiply(a.previous, b.previous);
        Poly q = multiply(a.difference, b.difference);

        // Remove the obligatory factor x:
        // C_{a,b}(x)/x = P_a P_b - (1-x) E_a E_b.
        if (p.size() < q.size() + 1) p.resize(q.size() + 1);
        for (int j = 0; j < (int)q.size(); ++j) {
            p[j] = submod(p[j], q[j]);
            p[j + 1] = addmod(p[j + 1], q[j]);
        }
        trim(p);
        addFactor(move(p));
        i += 2;
    }

    while (heap.size() > 1) {
        int a = heap.top().second;
        heap.pop();
        int b = heap.top().second;
        heap.pop();

        Poly c = multiply(pool[a], pool[b]);
        Poly().swap(pool[a]);
        Poly().swap(pool[b]);

        int id = (int)pool.size();
        heap.emplace((int)c.size(), id);
        pool.push_back(move(c));
    }

    Poly answer{1};
    if (!heap.empty()) {
        answer = move(pool[heap.top().second]);
    }

    int multiplier = (long long)scalar * fact[n] % MOD;

    for (int k = 1; k <= n; ++k) {
        int index = k - blocks;
        int value = 0;
        if (index >= 0 && index < (int)answer.size()) {
            value = (long long)answer[index] * multiplier % MOD;
        }
        if (k > 1) cout << ' ';
        cout << value;
    }
    cout << '\n';

    return 0;
}
