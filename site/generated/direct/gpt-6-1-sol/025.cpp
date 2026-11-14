#include <bits/stdc++.h>
using namespace std;

class ModArithmetic {
public:
    uint32_t mod;
    uint64_t reciprocal;

    explicit ModArithmetic(uint32_t p)
        : mod(p), reciprocal(uint64_t(-1) / p + 1) {}

    inline uint32_t add(uint32_t a, uint32_t b) const {
        uint32_t s = a + b;
        return s >= mod ? s - mod : s;
    }

    inline uint32_t sub(uint32_t a, uint32_t b) const {
        return a >= b ? a - b : a + mod - b;
    }

    inline uint32_t mul(uint32_t a, uint32_t b) const {
        uint64_t z = uint64_t(a) * b;
        uint64_t q = (uint64_t)((__uint128_t(z) * reciprocal) >> 64);
        uint64_t y = q * mod;
        return uint32_t(z - y + (z < y ? mod : 0));
    }

    uint32_t power(uint32_t a, uint32_t e) const {
        uint32_t result = 1;
        while (e) {
            if (e & 1) result = mul(result, a);
            a = mul(a, a);
            e >>= 1;
        }
        return result;
    }

    uint32_t inverse(uint32_t a) const {
        return power(a, mod - 2);
    }

    inline uint32_t fromSigned(int x) const {
        return x >= 0 ? uint32_t(x) : mod - uint32_t(-x);
    }
};

class FastOutput {
    static constexpr size_t CAPACITY = 1 << 16;
    char buffer[CAPACITY];
    size_t used = 0;

public:
    ~FastOutput() {
        flush();
    }

    void flush() {
        if (used) {
            fwrite(buffer, 1, used, stdout);
            used = 0;
        }
    }

    void put(char c) {
        if (used == CAPACITY) flush();
        buffer[used++] = c;
    }

    void number(uint32_t x, char separator) {
        char digits[12];
        int length = 0;
        do {
            digits[length++] = char('0' + x % 10);
            x /= 10;
        } while (x);
        while (length) put(digits[--length]);
        put(separator);
    }
};

int main() {
    int n;
    uint32_t p;
    if (scanf("%d%u", &n, &p) != 2) return 0;

    ModArithmetic mod(p);

    const int N = n * (n + 1) / 2;
    const int m = n + 1;
    const int maxDegree = N + n;
    const int width = N + 1;

    vector<uint32_t> inverse(maxDegree + 2);
    inverse[1] = 1;
    for (int i = 2; i < (int)inverse.size(); ++i) {
        inverse[i] = mod.sub(
            0, mod.mul(p / uint32_t(i), inverse[p % uint32_t(i)])
        );
    }

    vector<uint32_t> factorial(N + 1), inverseFactorial(N + 1);
    factorial[0] = 1;
    for (int i = 1; i <= N; ++i) {
        factorial[i] = mod.mul(factorial[i - 1], i);
    }
    inverseFactorial[N] = mod.inverse(factorial[N]);
    for (int i = N; i >= 1; --i) {
        inverseFactorial[i - 1] = mod.mul(inverseFactorial[i], i);
    }

    vector<uint32_t> binomial(width), inverseBinomial(width);
    for (int j = 0; j <= N; ++j) {
        binomial[j] = mod.mul(
            factorial[N],
            mod.mul(inverseFactorial[j], inverseFactorial[N - j])
        );
        inverseBinomial[j] = mod.mul(
            inverseFactorial[N],
            mod.mul(factorial[j], factorial[N - j])
        );
    }

    vector<int> A(m), C(m), D(m), triangular(m);
    for (int k = 0; k <= n; ++k) {
        A[k] = (n - k) * (n - k + 1) / 2;
        C[k] = (k + 1) * (2 * n + 2 - k) / 2;
        D[k] = (n - k + 1) * (n - k + 2) / 2;
        triangular[k] = k * (k + 1) / 2;
    }

    /*
       basis[s][j] is the coefficient of x^j in H_0
       for the s-th fundamental solution.
    */
    vector<uint32_t> basis(size_t(m) * width, 0);

    /*
       V[k][s] is H_k(1) for fundamental solution s.
    */
    vector<uint32_t> V(size_t(m) * m, 0);
    vector<uint32_t> previous(size_t(m) * m, 0);
    vector<uint32_t> current(size_t(m) * m, 0);

    int active = 0;

    for (int degree = 0; degree <= maxDegree; ++degree) {
        while (active < n && triangular[active + 1] <= degree) {
            ++active;
        }

        fill(current.begin(), current.end(), 0);

        for (int k = n; k >= 0; --k) {
            if (degree > N + k) continue;

            uint32_t* cur = current.data() + size_t(k) * m;
            const uint32_t* prev = previous.data() + size_t(k) * m;

            if (degree == A[k]) {
                cur[n - k] = 1;
            } else {
                int denominator = degree - A[k];
                uint32_t invDenominator =
                    denominator > 0
                        ? inverse[denominator]
                        : mod.sub(0, inverse[-denominator]);

                uint32_t coefficient = mod.fromSigned(
                    N + k - A[k] - degree + 1
                );

                const uint32_t* lower =
                    k > 0 ? previous.data() + size_t(k - 1) * m : nullptr;
                const uint32_t* upper =
                    k < n ? current.data() + size_t(k + 1) * m : nullptr;

                for (int s = 0; s <= active; ++s) {
                    uint32_t value = mod.mul(coefficient, prev[s]);
                    if (lower) {
                        value = mod.add(value, mod.mul(D[k], lower[s]));
                    }
                    if (upper) {
                        value = mod.add(value, mod.mul(C[k], upper[s]));
                    }
                    cur[s] = mod.mul(value, invDenominator);
                }
            }

            uint32_t* evaluation = V.data() + size_t(k) * m;
            for (int s = 0; s <= active; ++s) {
                evaluation[s] = mod.add(evaluation[s], cur[s]);
            }
        }

        if (degree <= N) {
            for (int s = 0; s <= active; ++s) {
                basis[size_t(s) * width + degree] = current[s];
            }
        }

        previous.swap(current);
    }

    /*
       Reflection x -> 1/x acts on the solution space by

           M = V^{-1} diag(1,-1,1,-1,...) V.

       Compute M with one Gauss-Jordan elimination.
    */
    const int augmentedWidth = 2 * m;
    vector<uint32_t> augmented(size_t(m) * augmentedWidth);

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            uint32_t value = V[size_t(i) * m + j];
            augmented[size_t(i) * augmentedWidth + j] = value;
            augmented[size_t(i) * augmentedWidth + m + j] =
                (i & 1) ? mod.sub(0, value) : value;
        }
    }

    for (int column = 0; column < m; ++column) {
        int pivot = column;
        while (!augmented[size_t(pivot) * augmentedWidth + column]) {
            ++pivot;
        }

        if (pivot != column) {
            for (int j = column; j < augmentedWidth; ++j) {
                swap(
                    augmented[size_t(pivot) * augmentedWidth + j],
                    augmented[size_t(column) * augmentedWidth + j]
                );
            }
        }

        uint32_t* pivotRow =
            augmented.data() + size_t(column) * augmentedWidth;
        uint32_t invPivot = mod.inverse(pivotRow[column]);

        for (int j = column; j < augmentedWidth; ++j) {
            pivotRow[j] = mod.mul(pivotRow[j], invPivot);
        }

        for (int i = 0; i < m; ++i) {
            if (i == column) continue;
            uint32_t* row = augmented.data() + size_t(i) * augmentedWidth;
            uint32_t factor = row[column];
            if (!factor) continue;

            row[column] = 0;
            for (int j = column + 1; j < augmentedWidth; ++j) {
                row[j] = mod.sub(row[j], mod.mul(factor, pivotRow[j]));
            }
        }
    }

    vector<uint32_t> M(size_t(m) * m);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            M[size_t(i) * m + j] =
                augmented[size_t(i) * augmentedWidth + m + j];
        }
    }

    /*
       Column r of T gives a solution vanishing to order
       triangular[r] at t=0 and triangular[n-r] at t=1.
    */
    vector<uint32_t> T(size_t(m) * m, 0);
    for (int i = 0; i < m; ++i) T[size_t(i) * m + i] = 1;

    for (int r = n; r >= 0; --r) {
        for (int row = 0; row < n - r; ++row) {
            int q = n - row;
            uint32_t factor = mod.mul(
                M[size_t(row) * m + r],
                mod.inverse(M[size_t(row) * m + q])
            );

            if (!factor) continue;

            for (int i = row; i < m; ++i) {
                M[size_t(i) * m + r] = mod.sub(
                    M[size_t(i) * m + r],
                    mod.mul(factor, M[size_t(i) * m + q])
                );
            }
            for (int i = q; i < m; ++i) {
                T[size_t(i) * m + r] = mod.sub(
                    T[size_t(i) * m + r],
                    mod.mul(factor, T[size_t(i) * m + q])
                );
            }
        }
    }

    /*
       Express (1+x)^N in the fundamental basis, then in the
       endpoint-adapted basis. This normalizes the probabilities.
    */
    vector<uint32_t> constantCoordinates(m), scale(m);

    for (int r = 0; r <= n; ++r) {
        int degree = triangular[r];
        uint32_t value = binomial[degree];

        for (int s = 0; s < r; ++s) {
            value = mod.sub(
                value,
                mod.mul(
                    constantCoordinates[s],
                    basis[size_t(s) * width + degree]
                )
            );
        }

        constantCoordinates[r] = mod.mul(
            value,
            mod.inverse(basis[size_t(r) * width + degree])
        );
    }

    for (int r = 0; r <= n; ++r) {
        uint32_t value = constantCoordinates[r];
        for (int q = 0; q < r; ++q) {
            value = mod.sub(
                value,
                mod.mul(T[size_t(r) * m + q], scale[q])
            );
        }
        scale[r] = value;
    }

    /*
       Total number of valid arrangements:
       N! / n! * product_{1 <= i < j <= n} (j-i)/(j+i).
    */
    uint32_t total = mod.mul(factorial[N], inverseFactorial[n]);
    for (int i = 1; i <= n; ++i) {
        for (int j = i + 1; j <= n; ++j) {
            total = mod.mul(total, j - i);
            total = mod.mul(total, inverse[i + j]);
        }
    }

    /*
       Convert to counts of arrangements having exactly r singletons
       in the first j positions. Increasing r permits in-place work.
    */
    for (int r = 0; r <= n; ++r) {
        uint32_t* destination = basis.data() + size_t(r) * width;

        for (int s = r + 1; s <= n; ++s) {
            uint32_t coefficient = T[size_t(s) * m + r];
            if (!coefficient) continue;

            const uint32_t* source = basis.data() + size_t(s) * width;
            for (int j = 0; j <= N; ++j) {
                destination[j] = mod.add(
                    destination[j], mod.mul(coefficient, source[j])
                );
            }
        }

        uint32_t multiplier = mod.mul(scale[r], total);
        for (int j = 0; j <= N; ++j) {
            destination[j] = mod.mul(
                mod.mul(destination[j], multiplier),
                inverseBinomial[j]
            );
        }
    }

    // Suffix sums: singleton i lies among the first j positions.
    for (int i = n - 1; i >= 1; --i) {
        uint32_t* row = basis.data() + size_t(i) * width;
        const uint32_t* next = basis.data() + size_t(i + 1) * width;
        for (int j = 0; j <= N; ++j) {
            row[j] = mod.add(row[j], next[j]);
        }
    }

    FastOutput output;
    for (int i = 1; i <= n; ++i) {
        const uint32_t* row = basis.data() + size_t(i) * width;
        for (int j = 1; j <= N; ++j) {
            output.number(
                mod.sub(row[j], row[j - 1]),
                j == N ? '\n' : ' '
            );
        }
    }

    return 0;
}
