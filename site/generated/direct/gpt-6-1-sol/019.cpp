#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using uint64 = unsigned long long;
using uint128 = __uint128_t;

/*
    Reduction of a signed integer whose absolute value is below 3 * 10^18.

    Adding a sufficiently large multiple of mod makes it nonnegative.
    The reciprocal then replaces division in the innermost loop.
*/
class FastModulo {
    uint64 mod, reciprocal, offset;

public:
    explicit FastModulo(uint64 m) : mod(m) {
        reciprocal = (uint64)((uint128(1) << 64) / mod);
        offset = (4000000000000000000ULL / mod) * mod;
    }

    inline int reduce(int64 value) const {
        uint64 x = uint64(value) + offset;
        uint64 quotient = uint64((uint128(x) * reciprocal) >> 64);
        uint64 remainder = x - quotient * mod;
        if (remainder >= mod) remainder -= mod;
        return int(remainder);
    }
};

int64 modPow(int64 a, int64 e, int64 mod) {
    int64 result = 1;
    while (e > 0) {
        if (e & 1) result = result * a % mod;
        a = a * a % mod;
        e >>= 1;
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;

    /*
        C_r[a][b] = r! * h(r, b, a).

        Thus a is the number of descents, and b is the number of
        excedances. Two padded rows and one padded column allow all
        out-of-range predecessors in the recurrence to be read as zero.
    */
    const int stride = N + 3;
    const size_t size = size_t(stride) * stride;

    vector<int> previous(size, 0);
    vector<int> current(size, 0);
    vector<int> next(size, 0);

    auto index = [stride](int a, int b) -> size_t {
        return size_t(a + 2) * stride + (b + 1);
    };

    previous[index(0, 0)] = 1; // C_0
    current[index(0, 0)] = 1;  // C_1

    FastModulo reducer(M);
    int64 factorial = 1;

    for (int r = 1; r < N; ++r) {
        factorial = factorial * (r + 1) % M;
        next[index(0, 0)] = int(factorial);

        const int64 rr = int64(r) * r;

        for (int a = 1; a <= r; ++a) {
            const int* c0 = current.data() + index(a, 0);
            const int* c1 = current.data() + index(a - 1, 0);

            const int* p0 = previous.data() + index(a, 0);
            const int* p1 = previous.data() + index(a - 1, 0);
            const int* p2 = previous.data() + index(a - 2, 0);

            int* out = next.data() + index(a, 0);

            const int64 step = r + 2 - a;

            // Coefficients at b = 1.
            int64 c00 = r + 1 + 2LL * a;
            int64 c10 = r + 3 - 2LL * a;
            int64 c01 = r + int64(r) * a;
            int64 c11 = int64(r) * (r + 1 - a);

            const int64 d0 = -rr * (a + 1);
            const int64 d1 = -rr * (r - 2LL * a);
            const int64 d2 = rr * (r + 1 - a);

            for (int b = 1; b <= r; ++b) {
                int64 value =
                    c00 * c0[b] +
                    c10 * c1[b] +
                    c01 * c0[b - 1] +
                    c11 * c1[b - 1] +
                    d0 * p0[b - 1] +
                    d1 * p1[b - 1] +
                    d2 * p2[b - 1];

                out[b] = reducer.reduce(value);

                c00 += a;
                c10 += step;
                c01 -= a;
                c11 -= step;
            }
        }

        previous.swap(current);
        current.swap(next);
    }

    const int64 inverseFactorial = modPow(factorial, M - 2, M);

    // Output rows are excedances, while columns are descents.
    string output;
    output.reserve(size_t(N) * N * 11);

    for (int excedances = 0; excedances < N; ++excedances) {
        for (int descents = 0; descents < N; ++descents) {
            int answer = int(
                int64(current[index(descents, excedances)]) *
                inverseFactorial % M
            );

            output += to_string(answer);
            output += (descents + 1 == N ? '\n' : ' ');
        }
    }

    cout << output;
    return 0;
}
