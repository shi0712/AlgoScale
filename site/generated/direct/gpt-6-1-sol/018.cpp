#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int G = 3;

using Poly = vector<int>;

int addmod(int a, int b) {
    a += b;
    if (a >= MOD) a -= MOD;
    return a;
}

int submod(int a, int b) {
    a -= b;
    if (a < 0) a += MOD;
    return a;
}

int modpow(int a, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r = (long long)r * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return r;
}

namespace FFT {
    vector<int> roots{0, 1};
    vector<vector<int>> reversals(24);
    int inversePower[24];
    bool initialized = false;

    void initialize() {
        if (initialized) return;
        initialized = true;
        inversePower[0] = 1;
        const int inv2 = (MOD + 1) / 2;
        for (int i = 1; i < 24; ++i)
            inversePower[i] = (long long)inversePower[i - 1] * inv2 % MOD;
    }

    void ensureRoots(int n) {
        if ((int)roots.size() >= n) return;
        int k = __builtin_ctz((unsigned)roots.size());
        roots.resize(n);
        while ((1 << k) < n) {
            int z = modpow(G, (MOD - 1) >> (k + 1));
            for (int i = 1 << (k - 1); i < (1 << k); ++i) {
                roots[2 * i] = roots[i];
                roots[2 * i + 1] = (long long)roots[i] * z % MOD;
            }
            ++k;
        }
    }

    void ntt(Poly &a, bool inverse) {
        initialize();
        int n = (int)a.size();
        if (n == 1) return;

        int lg = __builtin_ctz((unsigned)n);
        auto &rev = reversals[lg];
        if (rev.empty()) {
            rev.resize(n);
            for (int i = 1; i < n; ++i)
                rev[i] = (rev[i >> 1] >> 1) | ((i & 1) ? (n >> 1) : 0);
        }

        ensureRoots(n);

        for (int i = 0; i < n; ++i)
            if (i < rev[i]) swap(a[i], a[rev[i]]);

        for (int len = 1; len < n; len <<= 1) {
            for (int i = 0; i < n; i += len << 1) {
                for (int j = 0; j < len; ++j) {
                    int u = a[i + j];
                    int v = (long long)a[i + j + len] * roots[len + j] % MOD;
                    a[i + j] = addmod(u, v);
                    a[i + j + len] = submod(u, v);
                }
            }
        }

        if (inverse) {
            reverse(a.begin() + 1, a.end());
            int invN = inversePower[lg];
            for (int &x : a) x = (long long)x * invN % MOD;
        }
    }

    Poly multiply(const Poly &a, const Poly &b) {
        if (a.empty() || b.empty()) return {};
        int need = (int)a.size() + (int)b.size() - 1;

        if (min(a.size(), b.size()) <= 16 ||
            (long long)a.size() * b.size() <= 1024) {
            Poly c(need);
            for (int i = 0; i < (int)a.size(); ++i) {
                if (!a[i]) continue;
                for (int j = 0; j < (int)b.size(); ++j) {
                    c[i + j] = (c[i + j] + (long long)a[i] * b[j]) % MOD;
                }
            }
            return c;
        }

        int n = 1;
        while (n < need) n <<= 1;

        Poly fa(a.begin(), a.end());
        fa.resize(n);
        ntt(fa, false);

        if (&a == &b) {
            for (int &x : fa) x = (long long)x * x % MOD;
        } else {
            Poly fb(b.begin(), b.end());
            fb.resize(n);
            ntt(fb, false);
            for (int i = 0; i < n; ++i)
                fa[i] = (long long)fa[i] * fb[i] % MOD;
        }

        ntt(fa, true);
        fa.resize(need);
        return fa;
    }

    Poly inverse(const Poly &a, int need) {
        Poly b(1, modpow(a[0], MOD - 2));
        while ((int)b.size() < need) {
            int k = min(need, (int)b.size() * 2);
            Poly cut(min(k, (int)a.size()));
            copy(a.begin(), a.begin() + cut.size(), cut.begin());

            Poly t = multiply(cut, b);
            t.resize(k);
            for (int &x : t)
                if (x) x = MOD - x;
            t[0] = addmod(t[0], 2);

            b = multiply(b, t);
            b.resize(k);
        }
        return b;
    }
}

void trim(Poly &a) {
    while (!a.empty() && a.back() == 0) a.pop_back();
}

/*
   A_n(u) is defined by

       sum_{m >= 0} answer(n,m) u^m = A_n(u)/(1-u)^(n+1).

   Recurrence:
       A_0 = 1,
       A_n = u(1-u) A'_{n-1} + n*u*A_{n-1}
             - u(1-u) sum_{i+j=n-2} A_i A_j.

   CDQ computes the quadratic convolution online.
*/
class NumeratorPrecomputation {
public:
    int size;
    vector<Poly> a;

    explicit NumeratorPrecomputation(int maximumN) {
        size = 1;
        while (size <= maximumN) size <<= 1;
        a.resize(size);
        for (int i = 0; i < size; ++i) a[i].resize(i + 1);
        solve(0, size);
    }

private:
    void addSmallProduct(int i, int j, int target, int factor) {
        const Poly &x = a[i];
        const Poly &y = a[j];
        Poly &out = a[target];

        for (int q = 0; q <= j; ++q) {
            if (!y[q]) continue;
            int v = y[q];
            if (factor == 2) v = addmod(v, v);

            for (int p = 0; p <= i; ++p) {
                if (!x[p]) continue;
                int z = (long long)x[p] * v % MOD;
                int k = p + q + 1;
                out[k] = submod(out[k], z);
                out[k + 1] = addmod(out[k + 1], z);
            }
        }
    }

    void addContributions(int l, int mid, int r) {
        int len = r - l;

        if (len <= 8) {
            if (l == 0) {
                for (int i = l; i < mid; ++i) {
                    for (int j = i; j < mid; ++j) {
                        int target = i + j + 2;
                        if (target < mid || target >= r) continue;
                        addSmallProduct(i, j, target, i == j ? 1 : 2);
                    }
                }
            } else {
                for (int i = l; i < mid; ++i) {
                    int low = max(0, mid - i - 2);
                    int high = r - i - 3;
                    for (int j = low; j <= high; ++j)
                        addSmallProduct(i, j, i + j + 2, 2);
                }
            }
            return;
        }

        // Kronecker substitution: (row, column) -> row * width + column.
        // Any relevant product has degree less than width.
        int width = r - 1;
        int rows = mid - l;

        Poly x((rows - 1) * width + mid);
        for (int i = l; i < mid; ++i) {
            int offset = (i - l) * width;
            copy(a[i].begin(), a[i].end(), x.begin() + offset);
        }

        Poly product;
        int factor;
        if (l == 0) {
            product = FFT::multiply(x, x);
            factor = 1;
        } else {
            int highestJ = len - 3;
            Poly y(highestJ * width + highestJ + 1);
            for (int j = 0; j <= highestJ; ++j) {
                int offset = j * width;
                copy(a[j].begin(), a[j].end(), y.begin() + offset);
            }
            product = FFT::multiply(x, y);
            factor = 2;
        }

        for (int n = mid; n < r; ++n) {
            int row = n - l - 2;
            if (row < 0) continue;
            int offset = row * width;

            for (int k = 0; k <= n - 2; ++k) {
                int index = offset + k;
                if (index >= (int)product.size()) break;
                int v = product[index];
                if (factor == 2) v = addmod(v, v);
                a[n][k + 1] = submod(a[n][k + 1], v);
                a[n][k + 2] = addmod(a[n][k + 2], v);
            }
        }
    }

    void solve(int l, int r) {
        if (r - l == 1) {
            if (l == 0) {
                a[0][0] = 1;
                return;
            }

            int n = l;
            for (int k = 1; k <= n; ++k) {
                long long v = (long long)(n - k + 1) * a[n - 1][k - 1];
                if (k < n) v += (long long)k * a[n - 1][k];
                a[n][k] = addmod(a[n][k], v % MOD);
            }
            return;
        }

        int mid = (l + r) / 2;
        solve(l, mid);
        addContributions(l, mid, r);
        solve(mid, r);
    }
};

/*
   Convert sum c_k (x)_k to ordinary monomial coefficients.
   Products of consecutive linear factors are shared by all conversions.
*/
class FallingFactorialConverter {
    int size;
    vector<Poly> product;

    void build(int v, int l, int r) {
        if (r - l == 1) {
            product[v] = {l == 0 ? 0 : MOD - l, 1};
            return;
        }
        int mid = (l + r) / 2;
        build(v * 2, l, mid);
        build(v * 2 + 1, mid, r);
        product[v] = FFT::multiply(product[v * 2], product[v * 2 + 1]);
    }

    Poly convert(int v, int l, int r, const Poly &c) const {
        if (l >= (int)c.size()) return {};
        if (r - l == 1) {
            if (c[l] == 0) return {};
            return {c[l]};
        }

        int mid = (l + r) / 2;
        Poly left = convert(v * 2, l, mid, c);
        Poly right = convert(v * 2 + 1, mid, r, c);
        if (right.empty()) return left;

        Poly result = FFT::multiply(product[v * 2], right);
        if (result.size() < left.size()) result.resize(left.size());
        for (int i = 0; i < (int)left.size(); ++i)
            result[i] = addmod(result[i], left[i]);

        trim(result);
        return result;
    }

public:
    explicit FallingFactorialConverter(int s) : size(s), product(4 * s) {
        build(1, 0, size);
    }

    Poly convert(const Poly &c) const {
        return convert(1, 0, size, c);
    }
};

int horner(const Poly &f, int x) {
    int result = 0;
    for (int i = (int)f.size() - 1; i >= 0; --i)
        result = ((long long)result * x + f[i]) % MOD;
    return result;
}

/*
   Fast multipoint evaluation.

   If P is a node's product polynomial of degree s and r is the remainder,
   store
       b = reverse_{s-1}(r) / reverse_s(P) mod x^s.
   Passing to a child only needs a middle product with the sibling.
*/
class MultipointEvaluation {
    static constexpr int LEAF = 16;

    const Poly &points;
    int count;
    vector<Poly> product;
    Poly result;

    void build(int v, int l, int r) {
        if (r - l <= LEAF) {
            Poly p{1};
            for (int i = l; i < r; ++i) {
                Poly q(p.size() + 1);
                int neg = points[i] ? MOD - points[i] : 0;
                for (int j = 0; j < (int)p.size(); ++j) {
                    q[j] = (q[j] + (long long)p[j] * neg) % MOD;
                    q[j + 1] = addmod(q[j + 1], p[j]);
                }
                p.swap(q);
            }
            product[v] = move(p);
            return;
        }

        int mid = (l + r) / 2;
        build(v * 2, l, mid);
        build(v * 2 + 1, mid, r);
        product[v] = FFT::multiply(product[v * 2], product[v * 2 + 1]);
    }

    void descend(int v, int l, int r, Poly b) {
        int s = r - l;
        if (s <= LEAF) {
            Poly q = product[v];
            reverse(q.begin(), q.end());
            Poly rem = FFT::multiply(b, q);
            rem.resize(s);
            reverse(rem.begin(), rem.end());

            for (int i = l; i < r; ++i)
                result[i] = horner(rem, points[i]);
            return;
        }

        int mid = (l + r) / 2;
        int leftSize = mid - l;
        int rightSize = r - mid;

        Poly q = product[v * 2 + 1];
        reverse(q.begin(), q.end());
        Poly t = FFT::multiply(b, q);
        Poly left(leftSize);
        for (int i = 0; i < leftSize; ++i)
            left[i] = t[rightSize + i];

        q = product[v * 2];
        reverse(q.begin(), q.end());
        t = FFT::multiply(b, q);
        Poly right(rightSize);
        for (int i = 0; i < rightSize; ++i)
            right[i] = t[leftSize + i];

        descend(v * 2, l, mid, move(left));
        descend(v * 2 + 1, mid, r, move(right));
    }

public:
    MultipointEvaluation(const Poly &f, const Poly &x)
        : points(x), count((int)x.size()), product(4 * x.size()),
          result(x.size()) {
        if (f.size() <= 33 || count <= LEAF) {
            for (int i = 0; i < count; ++i)
                result[i] = horner(f, points[i]);
            return;
        }

        build(1, 0, count);

        int length = max((int)f.size(), count);
        Poly reversedF(length);
        for (int i = 0; i < (int)f.size(); ++i)
            reversedF[length - 1 - i] = f[i];

        Poly reversedP = product[1];
        reverse(reversedP.begin(), reversedP.end());

        Poly inv = FFT::inverse(reversedP, length);
        Poly t = FFT::multiply(reversedF, inv);

        Poly root(count);
        int start = length - count;
        for (int i = 0; i < count; ++i)
            root[i] = t[start + i];

        descend(1, 0, count, move(root));
    }

    const Poly &values() const {
        return result;
    }
};

class FastInput {
    static constexpr int BUFFER = 1 << 20;
    char data[BUFFER];
    int pos = 0, len = 0;

    char get() {
        if (pos == len) {
            len = fread(data, 1, BUFFER, stdin);
            pos = 0;
            if (!len) return 0;
        }
        return data[pos++];
    }

public:
    int readInt() {
        char c;
        do {
            c = get();
        } while (c <= ' ' && c);
        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + c - '0';
            c = get();
        }
        return x;
    }
};

struct Query {
    int m;
    int id;
};

int main() {
    FastInput input;
    int T = input.readInt();

    vector<vector<Query>> queries(1001);
    vector<int> answer(T);
    int maximumN = 0;

    for (int id = 0; id < T; ++id) {
        int n = input.readInt();
        int m = input.readInt();
        if (m >= MOD) m -= MOD;

        if (n == 1) {
            answer[id] = m;
        } else if (n == 2) {
            answer[id] = (long long)m * submod(m, 1) % MOD;
        } else if (n == 3) {
            int t = submod(m, 1);
            answer[id] = (long long)m * t % MOD * t % MOD;
        } else {
            queries[n].push_back({m, id});
            maximumN = max(maximumN, n);
        }
    }

    if (maximumN >= 4) {
        NumeratorPrecomputation precomputation(maximumN);
        FallingFactorialConverter converter(precomputation.size);

        int limit = 2 * precomputation.size + 5;
        Poly inverseInteger(limit), factorial(limit), inverseFactorial(limit);
        inverseInteger[1] = 1;
        for (int i = 2; i < limit; ++i)
            inverseInteger[i] =
                MOD - (long long)(MOD / i) * inverseInteger[MOD % i] % MOD;

        factorial[0] = 1;
        for (int i = 1; i < limit; ++i)
            factorial[i] = (long long)factorial[i - 1] * i % MOD;

        inverseFactorial[limit - 1] = modpow(factorial[limit - 1], MOD - 2);
        for (int i = limit - 1; i >= 1; --i)
            inverseFactorial[i - 1] =
                (long long)inverseFactorial[i] * i % MOD;

        for (int n = 4; n <= maximumN; ++n) {
            if (queries[n].empty()) continue;

            // Obtain answer(n,0), ..., answer(n,n) from A_n.
            Poly kernel(n + 1);
            kernel[0] = 1;
            for (int t = 1; t <= n; ++t)
                kernel[t] = (long long)kernel[t - 1] * (n + t) % MOD
                            * inverseInteger[t] % MOD;

            Poly values = FFT::multiply(precomputation.a[n], kernel);
            values.resize(n + 1);

            // Newton coefficients: c_k = Delta^k f(0) / k!.
            Poly x(n + 1), y(n + 1);
            for (int k = 0; k <= n; ++k) {
                x[k] = (long long)values[k] * inverseFactorial[k] % MOD;
                y[k] = inverseFactorial[k];
                if (k & 1) y[k] = MOD - y[k];
            }

            Poly c = FFT::multiply(x, y);
            c.resize(n + 1);
            Poly polynomial = converter.convert(c);

            int batchSize = 1;
            while (batchSize < n + 1) batchSize <<= 1;

            auto &group = queries[n];
            for (int start = 0; start < (int)group.size(); start += batchSize) {
                int length = min(batchSize, (int)group.size() - start);
                Poly points(length);
                for (int i = 0; i < length; ++i)
                    points[i] = group[start + i].m;

                MultipointEvaluation evaluation(polynomial, points);
                const Poly &evaluated = evaluation.values();
                for (int i = 0; i < length; ++i)
                    answer[group[start + i].id] = evaluated[i];
            }
        }
    }

    string output;
    output.reserve((size_t)T * 11);
    for (int x : answer) {
        output += to_string(x);
        output.push_back('\n');
    }
    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}
