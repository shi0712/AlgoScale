#include <bits/stdc++.h>
using namespace std;

// Fixed-precision decimal floating point.  Only division by integers is
// needed.  252 significant decimal digits leave ample room for cancellation.
struct Real {
    static constexpr int L = 28;
    static constexpr uint64_t BASE = 1000000000;
    array<uint32_t, L> a{}; // Little endian mantissa.
    int exponent = 0, sign = 0;

    Real(long long value = 0) {
        if (!value) return;
        sign = value < 0 ? -1 : 1;
        uint64_t v = value < 0 ? -value : value;
        int size = 0;
        while (v) {
            a[size++] = v % BASE;
            v /= BASE;
        }
        int shift = L - size;
        for (int i = size - 1; i >= 0; --i) a[i + shift] = a[i];
        fill(a.begin(), a.begin() + shift, 0);
        exponent = -shift;
    }
    void normalize() {
        int shift = 0;
        while (shift < L && !a[L - 1 - shift]) ++shift;
        if (shift == L) {
            sign = 0;
            exponent = 0;
            return;
        }
        if (shift) {
            for (int i = L - 1; i >= shift; --i) a[i] = a[i - shift];
            fill(a.begin(), a.begin() + shift, 0);
            exponent -= shift;
        }
    }
    static int compare_abs(const Real& x, const Real& y) {
        if (x.exponent != y.exponent) return x.exponent > y.exponent ? 1 : -1;
        for (int i = L - 1; i >= 0; --i)
            if (x.a[i] != y.a[i]) return x.a[i] > y.a[i] ? 1 : -1;
        return 0;
    }
    Real operator-() const {
        Real result = *this;
        result.sign = -result.sign;
        return result;
    }
    Real& operator+=(const Real& other) {
        if (!other.sign) return *this;
        if (!sign) return *this = other;
        Real x = *this, y = other;
        if (compare_abs(x, y) < 0) swap(x, y);
        int gap = x.exponent - y.exponent;
        *this = x;
        if (gap >= L) return *this;
        if (x.sign == y.sign) {
            uint64_t carry = 0;
            for (int i = 0; i < L; ++i) {
                uint64_t v = uint64_t(x.a[i]) + carry;
                if (i + gap < L) v += y.a[i + gap];
                a[i] = v % BASE;
                carry = v / BASE;
            }
            if (carry) {
                for (int i = 0; i < L - 1; ++i) a[i] = a[i + 1];
                a[L - 1] = carry;
                ++exponent;
            }
        } else {
            int64_t borrow = 0;
            for (int i = 0; i < L; ++i) {
                int64_t v = int64_t(x.a[i]) - borrow;
                if (i + gap < L) v -= y.a[i + gap];
                borrow = v < 0;
                a[i] = v + borrow * BASE;
            }
            normalize();
        }
        return *this;
    }
    Real& operator-=(const Real& y) { return *this += -y; }
    friend Real operator+(Real x, const Real& y) { return x += y; }
    friend Real operator-(Real x, const Real& y) { return x -= y; }

    friend Real operator*(const Real& x, const Real& y) {
        Real result;
        if (!x.sign || !y.sign) return result;
        array<unsigned __int128, 2 * L> product{};
        for (int i = 0; i < L; ++i)
            if (x.a[i])
                for (int j = 0; j < L; ++j)
                    product[i + j] += uint64_t(x.a[i]) * y.a[j];
        for (int i = 0; i < 2 * L - 1; ++i) {
            product[i + 1] += product[i] / BASE;
            product[i] %= BASE;
        }
        int top = 2 * L - 1;
        while (!product[top]) --top;
        int shift = top - L + 1;
        for (int i = 0; i < L; ++i) result.a[i] = product[i + shift];
        result.exponent = x.exponent + y.exponent + shift;
        result.sign = x.sign * y.sign;
        return result;
    }
    friend Real operator*(Real x, long long value) {
        if (!value || !x.sign) return Real();
        if (value < 0) x.sign = -x.sign, value = -value;
        uint64_t carry = 0;
        for (int i = 0; i < L; ++i) {
            uint64_t v = uint64_t(x.a[i]) * value + carry;
            x.a[i] = v % BASE;
            carry = v / BASE;
        }
        if (carry) {
            for (int i = 0; i < L - 1; ++i) x.a[i] = x.a[i + 1];
            x.a[L - 1] = carry;
            ++x.exponent;
        }
        return x;
    }
    friend Real operator/(Real x, long long value) {
        if (!x.sign) return x;
        if (value < 0) x.sign = -x.sign, value = -value;
        uint64_t remainder = 0;
        for (int i = L - 1; i >= 0; --i) {
            uint64_t v = remainder * BASE + x.a[i];
            x.a[i] = v / value;
            remainder = v % value;
        }
        int shift = 0;
        while (!x.a[L - 1 - shift]) ++shift;
        if (shift) {
            for (int i = L - 1; i >= shift; --i) x.a[i] = x.a[i - shift];
            for (int i = shift - 1; i >= 0; --i) {
                uint64_t v = remainder * BASE;
                x.a[i] = v / value;
                remainder = v % value;
            }
            x.exponent -= shift;
        }
        return x;
    }
    Real& operator*=(long long value) { return *this = *this * value; }
    Real& operator*=(const Real& value) { return *this = *this * value; }
    long double to_long_double() const {
        if (!sign) return 0;
        long double result = 0;
        for (int i = L - 1; i >= L - 4; --i) result = result * BASE + a[i];
        return sign * result * powl((long double)BASE, exponent + L - 4);
    }
};

/*
Put x_i = d_i-1.  These, together with the unused final gap, form
a uniform weak composition of m-n into n+1 parts.  Consequently, for
any specified r of the first n parts,

  Pr(all those parts >= t) = C(m-r*t,n) / C(m,n).

Let S[r] be the sum of this probability over t >= 1.  Inclusion-exclusion
gives the expectation of the q-th largest x_i as

  sum_{r=q}^n (-1)^(r-q) C(r-1,q-1) C(n,r) S[r].

These alternating sums require considerably more than double precision.
They are evaluated together by shifting a polynomial by -1.

For small m, compute S directly.  Otherwise use the polynomial part of
the arithmetic-progression sum of g(x)=C(x,n).  Euler-Maclaurin is exact
for a polynomial.  Taking both endpoints to have residue zero gives

 S[r] = integral_0^m g(x)/g(m) dx / r - 1/2
      + sum_{odd d} B[d+1]/(d+1) * r^d
          * (g^(d)(m)-g^(d)(0))/(d! g(m)),

up to the periodic part.  The generating function of the unnormalized
sum is z^(n+r)/((1-z)^(n+1)(1-z^r)).  Its nontrivial r-th-root poles
bound the error of this polynomial by

  2*(r/4)^(n+1) / C(m,n)                 (r >= 2).

After all transforms and prefix sums, the absolute error is at most
  n^2/2 * (3*n^2/(4*m))^n.
For m > 100000 and 2 <= n <= 300 this is below 1.8e-9.
For r=1 the polynomial is exact.  The case n=1 is handled separately.

Complexity: O(n^2 + min(m,100000)*log(n)) high-precision operations;
O(n + min(m,100000)) memory.
*/

static vector<Real> direct_sums(int n, int m) {
    vector<Real> probability(m + 1), sums(n + 1);
    probability[m] = 1;
    for (int x = m; x > n; --x)
        probability[x - 1] = probability[x] * (x - n) / x;
    for (int r = 1; r <= n; ++r)
        for (int x = m - r; x >= n; x -= r)
            sums[r] += probability[x];
    return sums;
}

static vector<Real> polynomial_sums(int n, int m) {
    // p[j] is the coefficient of y^j in g(m*y)/g(m).
    // q[j] is m^j * g^(j)(m)/(j!*g(m)).
    vector<Real> p(n + 1), q(n + 1), bernoulli(n + 2);
    p[0] = q[0] = 1;
    for (int i = 0; i < n; ++i) {
        Real a = Real(m) / (m - i);
        Real b = -Real(i) / (m - i);
        for (int j = i + 1; j >= 1; --j) {
            p[j] = p[j] * b + p[j - 1] * a;
            q[j] += q[j - 1] * a;
        }
        p[0] *= b;
    }
    Real integral = 0;
    for (int j = 0; j <= n; ++j)
        integral += p[j] / (j + 1);
    integral *= m;

    // Even Bernoulli numbers, from sum C(s+1,j)*B[j] = 0.
    bernoulli[0] = 1;
    for (int s = 2; s <= n + 1; s += 2) {
        Real total = -Real(s - 1) / 2;
        Real choose = 1;
        for (int j = 1; j < s; ++j) {
            choose = choose * (s + 2 - j) / j;
            if ((j & 1) == 0)
                total += choose * bernoulli[j];
        }
        bernoulli[s] = -total / (s + 1);
    }
    vector<Real> coefficient(n + 1);
    for (int d = 1; d <= n; d += 2)
        coefficient[d] = bernoulli[d + 1] * (q[d] - p[d]) / (d + 1);

    vector<Real> sums(n + 1);
    int last = (n & 1) ? n : n - 1;
    for (int r = 1; r <= n; ++r) {
        Real ratio = Real(r) / m;
        Real square = ratio * ratio;
        Real correction = 0;
        for (int d = last; d >= 1; d -= 2)
            correction = correction * square + coefficient[d];
        sums[r] = integral / r - Real(1) / 2 + ratio * correction;
    }
    return sums;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m;
    if (!(cin >> n >> m)) return 0;
    cout << fixed << setprecision(12);
    if (n == 1) {
        cout << (static_cast<long double>(m) + 1) / 2 << '\n';
        return 0;
    }
    vector<Real> sums = m <= 100000 ? direct_sums(n, m)
                                           : polynomial_sums(n, m);
    vector<Real> a(n);
    Real choose = 1;
    for (int r = 1; r <= n; ++r) {
        choose = choose * (n - r + 1) / r;
        a[r - 1] = choose * sums[r];
    }
    // Coefficients of P(z-1), where P(z)=sum C(n,r)*S[r]*z^(r-1).
    for (int i = n - 2; i >= 0; --i)
        for (int j = i; j < n - 1; ++j)
            a[j] -= a[j + 1];

    Real answer = 0;
    for (int i = 0; i < n; ++i) {
        answer += 1 + a[n - 1 - i];
        cout << answer.to_long_double() << '\n';
    }
}
