#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int A = 6000;
static constexpr int SMALL_LIMIT = 73;

int mod_pow(int a, int e) {
    long long result = 1;
    long long base = a;
    while (e > 0) {
        if (e & 1) result = result * base % MOD;
        base = base * base % MOD;
        e >>= 1;
    }
    return static_cast<int>(result);
}

inline int mul(int a, int b) {
    return static_cast<int>(1LL * a * b % MOD);
}

void subset_product_transform(vector<int>& values, int bits) {
    const int size = 1 << bits;
    for (int i = 0; i < bits; ++i) {
        const int bit = 1 << i;
        for (int start = 0; start < size; start += 2 * bit) {
            for (int j = 0; j < bit; ++j) {
                const int at = start + bit + j;
                values[at] = mul(values[at], values[at - bit]);
            }
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<int> count(A + 1, 0);
    for (int i = 0; i < n; ++i) {
        int a;
        cin >> a;
        ++count[a];
    }

    vector<int> spf(A + 1, 0);
    vector<int> primes;
    for (int i = 2; i <= A; ++i) {
        if (spf[i] == 0) {
            spf[i] = i;
            primes.push_back(i);
        }
        for (int p : primes) {
            if (p > spf[i] || i * p > A) break;
            spf[i * p] = p;
        }
    }

    vector<int> small_primes;
    vector<int> prime_index(A + 1, -1);
    for (int p : primes) {
        if (p > SMALL_LIMIT) break;
        prime_index[p] = static_cast<int>(small_primes.size());
        small_primes.push_back(p);
    }
    const int small_count = static_cast<int>(small_primes.size());
    const int full_size = 1 << small_count;

    vector<int> mask(A + 1, 0);
    vector<bool> has_large(A + 1, false);
    for (int x = 2; x <= A; ++x) {
        const int p = spf[x];
        const int previous = x / p;
        mask[x] = mask[previous];
        has_large[x] = has_large[previous];
        if (p <= SMALL_LIMIT) {
            mask[x] |= 1 << prime_index[p];
        } else {
            has_large[x] = true;
        }
    }

    vector<int> value_factor(A + 1, 1);
    for (int x = 1; x <= A; ++x) {
        if (count[x] > 0) {
            value_factor[x] = mod_pow(x + 1, count[x]);
        }
    }

    vector<vector<int>> large_groups(small_count + 1);
    for (int p : primes) {
        if (p <= SMALL_LIMIT) continue;
        const int maximum_cofactor = A / p;
        const int bits = static_cast<int>(upper_bound(
            small_primes.begin(), small_primes.end(), maximum_cofactor
        ) - small_primes.begin());
        large_groups[bits].push_back(p);
    }

    vector<vector<int>> group_product(small_count + 1);
    for (int bits = 0; bits <= small_count; ++bits) {
        group_product[bits].assign(1 << bits, 1);
    }

    vector<int> work(full_size, 1);
    for (int bits = 0; bits <= small_count; ++bits) {
        const int size = 1 << bits;
        int unused_product = 1;
        for (int p : large_groups[bits]) {
            fill(work.begin(), work.begin() + size, 1);
            const int maximum_cofactor = A / p;
            bool used = false;
            for (int cofactor = 1; cofactor <= maximum_cofactor; ++cofactor) {
                const int x = p * cofactor;
                if (count[x] == 0) continue;
                used = true;
                const int at = mask[cofactor];
                work[at] = mul(work[at], value_factor[x]);
            }
            if (!used) {
                unused_product = mul(unused_product, p);
                continue;
            }
            subset_product_transform(work, bits);
            for (int s = 0; s < size; ++s) {
                const int factor = (1 + 1LL * (p - 1) * work[s]) % MOD;
                group_product[bits][s] = mul(group_product[bits][s], factor);
            }
        }
        for (int s = 0; s < size; ++s) {
            group_product[bits][s] = mul(group_product[bits][s], unused_product);
        }
    }

    vector<int> base(full_size, 1);
    for (int x = 1; x <= A; ++x) {
        if (count[x] > 0 && !has_large[x]) {
            const int at = mask[x];
            base[at] = mul(base[at], value_factor[x]);
        }
    }
    subset_product_transform(base, small_count);

    vector<int> large_product(full_size, 1);
    large_product[0] = group_product[0][0];
    int current_size = 1;
    for (int bits = 1; bits <= small_count; ++bits) {
        const int half = current_size;
        for (int s = 0; s < half; ++s) {
            const int previous = large_product[s];
            large_product[s] = mul(previous, group_product[bits][s]);
            large_product[s + half] = mul(previous, group_product[bits][s + half]);
        }
        current_size *= 2;
    }

    vector<int> weight(full_size, 1);
    int sum = 0;
    for (int s = 0; s < full_size; ++s) {
        if (s != 0) {
            const int bit = __builtin_ctz(static_cast<unsigned int>(s));
            weight[s] = mul(weight[s ^ (1 << bit)], small_primes[bit] - 1);
        }
        const int term = mul(mul(weight[s], base[s]), large_product[s]);
        sum += term;
        if (sum >= MOD) sum -= MOD;
    }

    int prime_product = 1;
    for (int p : primes) {
        prime_product = mul(prime_product, p);
    }
    const int answer = mul(sum, mod_pow(prime_product, MOD - 2));
    cout << answer << '\n';
    return 0;
}
