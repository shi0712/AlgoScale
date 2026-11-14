#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int MAX_A = 6000;

class FastInput {
    static constexpr int SIZE = 1 << 16;
    char buffer[SIZE];
    int pos = 0, len = 0;

    char getChar() {
        if (pos == len) {
            len = (int)fread(buffer, 1, SIZE, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buffer[pos++];
    }

public:
    int nextInt() {
        char c;
        do {
            c = getChar();
        } while (c <= ' ' && c != 0);

        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return x;
    }
};

int modPow(int a, int e) {
    int result = 1;
    while (e > 0) {
        if (e & 1) result = (long long)result * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return result;
}

// After the transform:
// f[mask] = product of original f[submask] over submask ⊆ mask.
//
// Processing higher bits first and skipping multiplication by 1
// substantially reduces the number of modular multiplications.
void multiplicativeZeta(vector<int>& f, int bits) {
    const int size = (int)f.size();

    for (int bit = bits - 1; bit >= 0; --bit) {
        int half = 1 << bit;
        int block = half << 1;

        for (int start = 0; start < size; start += block) {
            int* low = f.data() + start;
            int* high = low + half;

            for (int j = 0; j < half; ++j) {
                int value = low[j];
                if (value != 1) {
                    high[j] = (long long)high[j] * value % MOD;
                }
            }
        }
    }
}

// Add a new bit on which the represented function does not depend.
void expand(vector<int>& f, int& bits, int targetBits) {
    while (bits < targetBits) {
        int oldSize = (int)f.size();
        f.resize(oldSize * 2);
        copy_n(f.data(), oldSize, f.data() + oldSize);
        ++bits;
    }
}

int main() {
    FastInput input;

    int n = input.nextInt();
    vector<int> count(MAX_A + 1, 0);
    for (int i = 0; i < n; ++i) {
        ++count[input.nextInt()];
    }

    // Linear sieve.
    vector<int> spf(MAX_A + 1, 0);
    vector<int> primes;

    for (int x = 2; x <= MAX_A; ++x) {
        if (spf[x] == 0) {
            spf[x] = x;
            primes.push_back(x);
        }
        for (int p : primes) {
            if (p > spf[x] || p * x > MAX_A) break;
            spf[p * x] = p;
        }
    }

    // There are 21 primes p with p^2 <= 6000.
    vector<int> smallPrimes;
    vector<int> smallIndex(MAX_A + 1, -1);

    for (int p : primes) {
        if (p * p <= MAX_A) {
            smallIndex[p] = (int)smallPrimes.size();
            smallPrimes.push_back(p);
        }
    }

    const int K = (int)smallPrimes.size();
    const int FULL_SIZE = 1 << K;

    // smooth[mask] initially stores factors belonging exactly to mask.
    vector<int> smooth(FULL_SIZE, 1);

    // For a large prime p, store (small-prime mask, (1+a)^count[a]).
    vector<vector<pair<int, int>>> groups(MAX_A + 1);
    vector<int> dimension(MAX_A + 1, 0);

    for (int a = 1; a <= MAX_A; ++a) {
        if (count[a] == 0) continue;

        int x = a;
        int mask = 0;
        int largePrime = 0;

        while (x > 1) {
            int p = spf[x];

            if (smallIndex[p] != -1) {
                mask |= 1 << smallIndex[p];
            } else {
                largePrime = p;
            }

            do {
                x /= p;
            } while (x % p == 0);
        }

        int weight = modPow(a + 1, count[a]);

        if (largePrime == 0) {
            smooth[mask] = (long long)smooth[mask] * weight % MOD;
        } else {
            groups[largePrime].push_back({mask, weight});
            int d = mask == 0 ? 0 : 32 - __builtin_clz((unsigned)mask);
            dimension[largePrime] = max(dimension[largePrime], d);
        }
    }

    // Denominator in the prime-expansion identity.
    int denominator = 1;
    for (int p : smallPrimes) {
        denominator = (long long)denominator * p % MOD;
    }

    vector<int> largePrimes;
    for (int p : primes) {
        if (!groups[p].empty()) {
            largePrimes.push_back(p);
            denominator = (long long)denominator * p % MOD;
        }
    }

    sort(largePrimes.begin(), largePrimes.end(), [&](int x, int y) {
        return dimension[x] < dimension[y];
    });

    // accumulated[mask] is the product of processed large-prime factors.
    vector<int> accumulated(1, 1);
    vector<int> temporary;
    int currentBits = 0;

    for (int p : largePrimes) {
        int d = dimension[p];
        expand(accumulated, currentBits, d);

        int size = 1 << d;
        temporary.assign(size, 1);

        for (auto [mask, weight] : groups[p]) {
            temporary[mask] =
                (long long)temporary[mask] * weight % MOD;
        }

        multiplicativeZeta(temporary, d);

        for (int mask = 0; mask < size; ++mask) {
            int factor = (long long)(p - 1) * temporary[mask] % MOD;
            if (++factor == MOD) factor = 0;

            accumulated[mask] =
                (long long)accumulated[mask] * factor % MOD;
        }
    }

    expand(accumulated, currentBits, K);
    multiplicativeZeta(smooth, K);

    for (int mask = 0; mask < FULL_SIZE; ++mask) {
        smooth[mask] =
            (long long)smooth[mask] * accumulated[mask] % MOD;
    }

    // Calculate sum_mask smooth[mask] * product_{j in mask}(p_j - 1).
    for (int bit = K - 1; bit >= 0; --bit) {
        int half = 1 << bit;
        int weight = smallPrimes[bit] - 1;

        for (int mask = 0; mask < half; ++mask) {
            smooth[mask] =
                (smooth[mask] +
                 (long long)weight * smooth[mask + half]) % MOD;
        }
    }

    int answer =
        (long long)smooth[0] * modPow(denominator, MOD - 2) % MOD;

    printf("%d\n", answer);
    return 0;
}
