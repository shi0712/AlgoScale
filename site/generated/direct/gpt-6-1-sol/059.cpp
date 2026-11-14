#include <bits/stdc++.h>
using namespace std;

/*
    A floating-point number with a 1536-bit significand.
    Only addition, subtraction, and multiplication/division by small integers
    are needed.
*/
class HighPrecision {
public:
    static constexpr int PREC = 24;
    using u64 = uint64_t;
    using u128 = __uint128_t;

    // value = sign * (sum digit[i] * 2^(64*i)) * 2^(64*exponent)
    u64 digit[PREC]{};
    int length = 0;
    int exponent = 0;
    int sign = 0;

    HighPrecision() = default;

    explicit HighPrecision(u64 x) {
        if (x != 0) {
            digit[0] = x;
            length = 1;
            sign = 1;
        }
    }

    void normalize() {
        while (length && digit[length - 1] == 0) --length;
        if (length == 0) {
            exponent = 0;
            sign = 0;
            return;
        }

        int removed = 0;
        while (removed < length && digit[removed] == 0) ++removed;
        if (removed) {
            memmove(digit, digit + removed,
                    (length - removed) * sizeof(u64));
            length -= removed;
            exponent += removed;
        }
    }

    void multiply(u64 x) {
        if (!sign || x == 1) return;
        if (x == 0) {
            length = 0;
            exponent = 0;
            sign = 0;
            return;
        }

        u64 carry = 0;
        for (int i = 0; i < length; ++i) {
            u128 z = (u128)digit[i] * x + carry;
            digit[i] = (u64)z;
            carry = (u64)(z >> 64);
        }

        if (carry) {
            if (length < PREC) {
                digit[length++] = carry;
            } else {
                memmove(digit, digit + 1, (PREC - 1) * sizeof(u64));
                digit[PREC - 1] = carry;
                ++exponent;
            }
        }
        normalize();
    }

    void divide(u64 x) {
        if (!sign || x == 1) return;

        u64 remainder = 0;
        for (int i = length - 1; i >= 0; --i) {
            u128 z = ((u128)remainder << 64) | digit[i];
            digit[i] = (u64)(z / x);
            remainder = (u64)(z % x);
        }

        while (length && digit[length - 1] == 0) --length;

        // Generate fractional words when necessary.
        while (remainder && length < PREC) {
            memmove(digit + 1, digit, length * sizeof(u64));
            u128 z = (u128)remainder << 64;
            digit[0] = (u64)(z / x);
            remainder = (u64)(z % x);
            ++length;
            --exponent;
        }
        normalize();
    }

    static int compareAbsolute(const HighPrecision& a,
                               const HighPrecision& b) {
        if (!a.sign) return b.sign ? -1 : 0;
        if (!b.sign) return 1;

        int ah = a.exponent + a.length;
        int bh = b.exponent + b.length;
        if (ah != bh) return ah < bh ? -1 : 1;

        int low = min(a.exponent, b.exponent);
        for (int p = ah - 1; p >= low; --p) {
            int ai = p - a.exponent;
            int bi = p - b.exponent;
            u64 av = (unsigned)ai < (unsigned)a.length ? a.digit[ai] : 0;
            u64 bv = (unsigned)bi < (unsigned)b.length ? b.digit[bi] : 0;
            if (av != bv) return av < bv ? -1 : 1;
        }
        return 0;
    }

    static HighPrecision addAbsolute(const HighPrecision& a,
                                     const HighPrecision& b) {
        if (!a.sign) {
            HighPrecision result = b;
            if (result.sign) result.sign = 1;
            return result;
        }
        if (!b.sign) {
            HighPrecision result = a;
            result.sign = 1;
            return result;
        }

        HighPrecision result;
        int high = max(a.exponent + a.length, b.exponent + b.length);
        int low = max(min(a.exponent, b.exponent), high - PREC);
        result.exponent = low;
        result.length = high - low;
        result.sign = 1;

        u64 carry = 0;
        for (int i = 0; i < result.length; ++i) {
            int ai = low + i - a.exponent;
            int bi = low + i - b.exponent;
            u64 av = (unsigned)ai < (unsigned)a.length ? a.digit[ai] : 0;
            u64 bv = (unsigned)bi < (unsigned)b.length ? b.digit[bi] : 0;
            u128 z = (u128)av + bv + carry;
            result.digit[i] = (u64)z;
            carry = (u64)(z >> 64);
        }

        if (carry) {
            if (result.length < PREC) {
                result.digit[result.length++] = carry;
            } else {
                memmove(result.digit, result.digit + 1,
                        (PREC - 1) * sizeof(u64));
                result.digit[PREC - 1] = carry;
                ++result.exponent;
            }
        }
        result.normalize();
        return result;
    }

    // Requires |a| >= |b|. Returns |a| - |b|.
    static HighPrecision subtractAbsolute(const HighPrecision& a,
                                          const HighPrecision& b) {
        if (!b.sign) {
            HighPrecision result = a;
            if (result.sign) result.sign = 1;
            return result;
        }

        HighPrecision result;
        int high = max(a.exponent + a.length, b.exponent + b.length);
        int low = max(min(a.exponent, b.exponent), high - PREC);
        result.exponent = low;
        result.length = high - low;
        result.sign = 1;

        u64 borrow = 0;
        for (int i = 0; i < result.length; ++i) {
            int ai = low + i - a.exponent;
            int bi = low + i - b.exponent;
            u64 av = (unsigned)ai < (unsigned)a.length ? a.digit[ai] : 0;
            u64 bv = (unsigned)bi < (unsigned)b.length ? b.digit[bi] : 0;

            u64 first = av - bv;
            u64 nextBorrow = (av < bv);
            result.digit[i] = first - borrow;
            nextBorrow |= (first < borrow);
            borrow = nextBorrow;
        }

        result.normalize();
        return result;
    }

    friend HighPrecision operator+(const HighPrecision& a,
                                   const HighPrecision& b) {
        if (!a.sign) return b;
        if (!b.sign) return a;

        if (a.sign == b.sign) {
            HighPrecision result = addAbsolute(a, b);
            result.sign = a.sign;
            return result;
        }

        int comparison = compareAbsolute(a, b);
        if (comparison == 0) return HighPrecision();

        if (comparison > 0) {
            HighPrecision result = subtractAbsolute(a, b);
            if (result.sign) result.sign = a.sign;
            return result;
        } else {
            HighPrecision result = subtractAbsolute(b, a);
            if (result.sign) result.sign = b.sign;
            return result;
        }
    }

    friend HighPrecision operator-(const HighPrecision& a,
                                   const HighPrecision& b) {
        HighPrecision negativeB = b;
        negativeB.sign = -negativeB.sign;
        return a + negativeB;
    }

    static long double ratio(const HighPrecision& a,
                             const HighPrecision& b) {
        if (!a.sign) return 0;

        long double am = (long double)a.digit[a.length - 1];
        long double bm = (long double)b.digit[b.length - 1];

        if (a.length > 1)
            am += ldexpl((long double)a.digit[a.length - 2], -64);
        if (b.length > 1)
            bm += ldexpl((long double)b.digit[b.length - 2], -64);

        int power = 64 * ((a.exponent + a.length - 1)
                        - (b.exponent + b.length - 1));

        long double answer = ldexpl(am / bm, power);
        return a.sign == b.sign ? answer : -answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    long long m;
    cin >> n >> m;

    using HP = HighPrecision;

    /*
        Only binomial coefficients C(x,n) for x <= n(n+1)-1
        are needed for polynomial interpolation.
    */
    int limit = (int)min(m, 1LL * n * (n + 1) - 1);
    vector<HP> binomial(limit + 1);

    binomial[n] = HP(1);
    for (int x = n + 1; x <= limit; ++x) {
        binomial[x] = binomial[x - 1];
        binomial[x].multiply(x);
        binomial[x].divide(x - n);
    }

    HP denominator(1);
    for (int i = 1; i <= n; ++i) {
        denominator.multiply(m - n + i);
        denominator.divide(i);
    }

    /*
        sums[j] = sum_{t >= 1} C(m - j*t, n),
        where terms with m-j*t < n are zero.
    */
    vector<HP> sums(n + 1);
    vector<HP> values(n + 1), differences(n + 1);

    for (int j = 1; j <= n; ++j) {
        long long L = m / j;
        int r = (int)(m % j);

        if (L <= n + 1) {
            HP result;
            for (int t = 0; t < L; ++t) {
                result = result + binomial[r + j * t];
            }
            sums[j] = result;
            continue;
        }

        // f(t) = C(r + j*t, n), a polynomial of degree n.
        for (int t = 0; t <= n; ++t)
            values[t] = binomial[r + j * t];

        // differences[k] = Delta^k f(0).
        for (int k = 0; k <= n; ++k) {
            differences[k] = values[0];
            for (int t = 0; t < n - k; ++t) {
                // These forward differences are nonnegative.
                values[t] = HP::subtractAbsolute(values[t + 1], values[t]);
            }
        }

        /*
            sum_{t=0}^{L-1} f(t)
              = sum_{k=0}^n differences[k] * C(L,k+1).

            Evaluate in nested form, using only small-integer operations.
        */
        HP result = differences[n];
        for (int k = n - 1; k >= 0; --k) {
            result.multiply(L - k - 1);
            result.divide(k + 2);
            result = result + differences[k];
        }
        result.multiply(L);
        sums[j] = result;
    }

    vector<long double> choose(n + 1, 1);
    for (int q = 1; q <= n; ++q)
        choose[q] = choose[q - 1] * (n - q + 1) / q;

    cout << fixed << setprecision(12);

    long double prefixExpectation = 0;

    // q = n-k+1, where k is the ascending order-statistic index.
    for (int q = n; q >= 1; --q) {
        HP result = sums[n];

        /*
            result =
              sum_{j=q}^n (-1)^(j-q)
              C(n,j) C(j-1,q-1) / C(n,q) * sums[j].
        */
        for (int j = n - 1; j >= q; --j) {
            result.multiply(1ULL * (n - j) * j);
            result.divide(1ULL * (j + 1) * (j - q + 1));
            result = sums[j] - result;
        }

        long double expectedExtra =
            HP::ratio(result, denominator) * choose[q];

        // Remove a possible negligible negative roundoff residue.
        expectedExtra = max(0.0L, expectedExtra);

        prefixExpectation += 1.0L + expectedExtra;
        cout << prefixExpectation << '\n';
    }

    return 0;
}
