#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

template <int PRECISION>
struct BigFloatT {
    static constexpr int LIMBS = PRECISION;
    static constexpr int CAPACITY = 2 * PRECISION + 2;
    array<uint64_t, CAPACITY> mantissa{};
    int size = 0;
    int exponent = 0;
    int sign = 0;

    BigFloatT() = default;
    BigFloatT(long long value) {
        if (value == 0) return;
        sign = value < 0 ? -1 : 1;
        unsigned long long x = value < 0 ? -(unsigned long long)value : value;
        mantissa[size++] = x;
    }

    template <int OTHER_PRECISION>
    BigFloatT(const BigFloatT<OTHER_PRECISION>& other) {
        sign = other.sign;
        exponent = other.exponent;
        int removed = max(0, other.size - LIMBS);
        size = other.size - removed;
        copy(other.mantissa.begin() + removed, other.mantissa.begin() + other.size, mantissa.begin());
        exponent += 64 * removed;
        normalize();
    }

    void normalize() {
        while (size > 0 && mantissa[size - 1] == 0) --size;
        if (size == 0) {
            exponent = 0;
            sign = 0;
            return;
        }
        if (size > LIMBS) {
            int removed = size - LIMBS;
            memmove(mantissa.data(), mantissa.data() + removed, LIMBS * sizeof(uint64_t));
            size = LIMBS;
            exponent += 64 * removed;
        }
        int leadingZeros = 0;
        while (leadingZeros < size && mantissa[leadingZeros] == 0) ++leadingZeros;
        if (leadingZeros) {
            memmove(mantissa.data(), mantissa.data() + leadingZeros, (size - leadingZeros) * sizeof(uint64_t));
            size -= leadingZeros;
            exponent += 64 * leadingZeros;
        }
    }

    static int compareAligned(const BigFloatT& a, int shiftA, const BigFloatT& b, int shiftB) {
        int sizeA = a.size + shiftA, sizeB = b.size + shiftB;
        if (sizeA != sizeB) return sizeA < sizeB ? -1 : 1;
        for (int i = sizeA - 1; i >= 0; --i) {
            uint64_t x = i >= shiftA && i < shiftA + a.size ? a.mantissa[i - shiftA] : 0;
            uint64_t y = i >= shiftB && i < shiftB + b.size ? b.mantissa[i - shiftB] : 0;
            if (x != y) return x < y ? -1 : 1;
        }
        return 0;
    }

    friend BigFloatT operator+(const BigFloatT& a, const BigFloatT& b) {
        if (a.sign == 0) return b;
        if (b.sign == 0) return a;
        int commonExponent = min(a.exponent, b.exponent);
        int shiftA = (a.exponent - commonExponent) / 64;
        int shiftB = (b.exponent - commonExponent) / 64;
        if (shiftA > LIMBS + 1) return a;
        if (shiftB > LIMBS + 1) return b;

        BigFloatT result;
        result.exponent = commonExponent;
        int resultSize = max(a.size + shiftA, b.size + shiftB);
        if (a.sign == b.sign) {
            result.sign = a.sign;
            unsigned __int128 carry = 0;
            for (int i = 0; i < resultSize; ++i) {
                unsigned __int128 sum = carry;
                if (i >= shiftA && i < shiftA + a.size) sum += a.mantissa[i - shiftA];
                if (i >= shiftB && i < shiftB + b.size) sum += b.mantissa[i - shiftB];
                result.mantissa[i] = sum;
                carry = sum >> 64;
            }
            result.size = resultSize;
            if (carry) result.mantissa[result.size++] = carry;
        } else {
            int comparison = compareAligned(a, shiftA, b, shiftB);
            if (comparison == 0) return BigFloatT();
            const BigFloatT& larger = comparison > 0 ? a : b;
            const BigFloatT& smaller = comparison > 0 ? b : a;
            int shiftLarger = comparison > 0 ? shiftA : shiftB;
            int shiftSmaller = comparison > 0 ? shiftB : shiftA;
            result.sign = larger.sign;
            result.size = resultSize;
            uint64_t borrow = 0;
            for (int i = 0; i < resultSize; ++i) {
                uint64_t x = i >= shiftLarger && i < shiftLarger + larger.size ? larger.mantissa[i - shiftLarger] : 0;
                uint64_t y = i >= shiftSmaller && i < shiftSmaller + smaller.size ? smaller.mantissa[i - shiftSmaller] : 0;
                result.mantissa[i] = x - y - borrow;
                borrow = borrow ? x <= y : x < y;
            }
        }
        result.normalize();
        return result;
    }

    friend BigFloatT operator-(const BigFloatT& a, const BigFloatT& b) {
        BigFloatT negative = b;
        negative.sign = -negative.sign;
        return a + negative;
    }

    friend BigFloatT operator*(const BigFloatT& a, const BigFloatT& b) {
        if (a.sign == 0 || b.sign == 0) return BigFloatT();
        BigFloatT result;
        result.sign = a.sign * b.sign;
        result.exponent = a.exponent + b.exponent;
        result.size = a.size + b.size;
        for (int i = 0; i < a.size; ++i) {
            unsigned __int128 carry = 0;
            for (int j = 0; j < b.size; ++j) {
                unsigned __int128 current = result.mantissa[i + j] + carry +
                                   (unsigned __int128)a.mantissa[i] * b.mantissa[j];
                result.mantissa[i + j] = current;
                carry = current >> 64;
            }
            result.mantissa[i + b.size] = carry;
        }
        result.normalize();
        return result;
    }

    BigFloatT& operator/=(long long divisor) {
        if (sign == 0) return *this;
        unsigned long long d = divisor < 0 ? -(unsigned long long)divisor : divisor;
        if (divisor < 0) sign = -sign;
        memmove(mantissa.data() + LIMBS, mantissa.data(), size * sizeof(uint64_t));
        fill(mantissa.begin(), mantissa.begin() + LIMBS, 0);
        size += LIMBS;
        exponent -= 64 * LIMBS;
        unsigned __int128 remainder = 0;
        for (int i = size - 1; i >= 0; --i) {
            unsigned __int128 current = (remainder << 64) | mantissa[i];
            mantissa[i] = current / d;
            remainder = current % d;
        }
        normalize();
        return *this;
    }

    long double toLongDouble() const {
        if (sign == 0) return 0;
        long double value = 0;
        for (int i = size - 1; i >= 0; --i)
            value = value * 18446744073709551616.0L + mantissa[i];
        return sign * ldexp(value, exponent);
    }
};

using BigFloat = BigFloatT<14>;
using WideFloat = BigFloatT<23>;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    long long m;
    cin >> n >> m;

    vector<vector<BigFloat>> choose(n + 2, vector<BigFloat>(n + 2));
    for (int i = 0; i <= n + 1; ++i) {
        choose[i][0] = 1;
        for (int j = 1; j <= i; ++j)
            choose[i][j] = choose[i - 1][j - 1] + choose[i - 1][j];
    }

    vector<BigFloat> bernoulli(n + 1);
    vector<WideFloat> work(n + 1);
    for (int i = 0; i <= n; ++i) {
        work[i] = WideFloat(1);
        work[i] /= i + 1;
        for (int j = i; j >= 1; --j)
            work[j - 1] = WideFloat(j) * (work[j - 1] - work[j]);
        bernoulli[i] = work[0];
    }

    vector<vector<BigFloat>> faulhaber(n + 1, vector<BigFloat>(n + 1));
    for (int p = 0; p <= n; ++p) {
        for (int t = 0; t <= p; ++t) {
            faulhaber[p][t] = choose[p + 1][t] * bernoulli[t];
            faulhaber[p][t] /= p + 1;
        }
    }

    vector<BigFloat> powerSums(n + 1);
    auto getPowerSums = [&](long long k) {
        if (k <= 64) {
            fill(powerSums.begin(), powerSums.end(), BigFloat());
            powerSums[0] = k + 1;
            for (long long x = 1; x <= k; ++x) {
                BigFloat power = 1;
                for (int p = 1; p <= n; ++p) {
                    power = power * BigFloat(x);
                    powerSums[p] = powerSums[p] + power;
                }
            }
            return;
        }

        vector<BigFloat> powers(n + 2);
        powers[0] = 1;
        for (int p = 1; p <= n + 1; ++p)
            powers[p] = powers[p - 1] * BigFloat(k);
        for (int p = 0; p <= n; ++p) {
            BigFloat sum;
            for (int t = 0; t <= p; ++t)
                if (faulhaber[p][t].sign != 0)
                    sum = sum + faulhaber[p][t] * powers[p + 1 - t];
            powerSums[p] = sum;
        }
        powerSums[0] = k + 1;
    };

    vector<BigFloat> basePolynomial(n + 1);
    basePolynomial[0] = 1;
    for (int t = 0; t < n; ++t) {
        BigFloat ratio(1);
        ratio /= m - t;
        for (int p = t + 1; p >= 1; --p)
            basePolynomial[p] = basePolynomial[p] - ratio * basePolynomial[p - 1];
    }

    vector<BigFloat> normalizedF(n + 1);
    for (int j = 1; j <= n; ++j) {
        long long k = (m - n) / j;
        if (j == 1) {
            normalizedF[j] = BigFloat(m + 1);
            normalizedF[j] /= n + 1;
            continue;
        }
        if (k == 0) {
            normalizedF[j] = 1;
            continue;
        }

        getPowerSums(k);
        BigFloat sum;
        BigFloat scale = 1;
        for (int p = 0; p <= n; ++p) {
            sum = sum + basePolynomial[p] * scale * powerSums[p];
            scale = scale * BigFloat(j);
        }
        normalizedF[j] = sum;
    }

    cout << fixed << setprecision(12);
    for (int i = 1; i <= n; ++i) {
        BigFloat answer;
        if (i == n) {
            answer = BigFloat(n) * normalizedF[1];
        } else {
            for (int j = max(2, n - i + 1); j <= n; ++j) {
                BigFloat term = choose[n][j] * choose[j - 2][n - i - 1] * normalizedF[j];
                if ((j - n + i + 1) & 1)
                    answer = answer - term;
                else
                    answer = answer + term;
            }
        }
        cout << answer.toLongDouble() << '\n';
    }
    return 0;
}
