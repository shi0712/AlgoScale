#include <bits/stdc++.h>
using namespace std;

using u64 = uint64_t;
using u128 = __uint128_t;
using i64 = int64_t;

static constexpr i64 MOD = 998244353;

static u64 mul_mod(u64 a, u64 b, u64 mod) {
    return static_cast<u128>(a) * b % mod;
}

static u64 pow_mod(u64 a, u64 e, u64 mod) {
    u64 result = 1;
    while (e != 0) {
        if (e & 1) result = mul_mod(result, a, mod);
        a = mul_mod(a, a, mod);
        e >>= 1;
    }
    return result;
}

static bool is_prime(u64 n) {
    if (n < 2) return false;
    for (u64 p : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL,
                  23ULL, 29ULL, 31ULL, 37ULL}) {
        if (n % p == 0) return n == p;
    }

    u64 d = n - 1;
    int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    // This base set is deterministic for every unsigned 64-bit integer.
    for (u64 a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL,
                  9780504ULL, 1795265022ULL}) {
        if (a % n == 0) continue;
        u64 x = pow_mod(a % n, d, n);
        if (x == 1 || x == n - 1) continue;
        bool composite = true;
        for (int r = 1; r < s; ++r) {
            x = mul_mod(x, x, n);
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

static mt19937_64 rng(0x9e3779b97f4a7c15ULL);

static u64 pollard_rho(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    while (true) {
        const u64 c = uniform_int_distribution<u64>(1, n - 1)(rng);
        u64 x = uniform_int_distribution<u64>(0, n - 1)(rng);
        u64 y = x;
        u64 divisor = 1;

        auto step = [&](u64 value) -> u64 {
            return (mul_mod(value, value, n) + c) % n;
        };

        while (divisor == 1) {
            x = step(x);
            y = step(step(y));
            const u64 difference = x > y ? x - y : y - x;
            divisor = gcd(difference, n);
        }
        if (divisor != n) return divisor;
    }
}

static void factorize(u64 n, vector<u64>& factors) {
    if (n == 1) return;
    if (is_prime(n)) {
        factors.push_back(n);
        return;
    }
    const u64 divisor = pollard_rho(n);
    factorize(divisor, factors);
    factorize(n / divisor, factors);
}

struct PrimePowerContribution {
    u64 prime;
    vector<i64> weight;
};

static i64 solve(u64 n, u64 m) {
    vector<u64> factors;
    factorize(n, factors);
    sort(factors.begin(), factors.end());

    vector<PrimePowerContribution> groups;
    i64 answer = 1;  // Empty product for n = 1.

    for (size_t i = 0; i < factors.size();) {
        const u64 prime = factors[i];
        u64 prime_power = 1;
        int exponent = 0;
        while (i < factors.size() && factors[i] == prime) {
            prime_power *= prime;
            ++exponent;
            ++i;
        }

        const i64 phi = static_cast<i64>((prime_power - prime_power / prime) % MOD);
        const i64 power_mod = static_cast<i64>(prime_power % MOD);

        // f(n, 0) contributes product_p (p^e + e * phi(p^e)).
        answer = answer * ((power_mod + exponent * phi) % MOD) % MOD;

        const int highest = (exponent + 1) / 2;
        vector<i64> weight(highest + 1, 2 * phi % MOD);
        weight[0] = phi;
        weight[highest] = (exponent & 1)
            ? power_mod
            : (power_mod + phi) % MOD;
        groups.push_back({prime, move(weight)});
    }

    auto enumerate = [&](auto&& self, size_t index, u64 divisor, i64 weight) -> void {
        if (index == groups.size()) {
            answer = (answer + weight * static_cast<i64>((m / divisor) % MOD)) % MOD;
            return;
        }

        const auto& group = groups[index];
        for (size_t exponent = 0; exponent < group.weight.size(); ++exponent) {
            self(self, index + 1, divisor,
                 weight * group.weight[exponent] % MOD);
            if (exponent + 1 == group.weight.size() || divisor > m / group.prime) {
                break;
            }
            divisor *= group.prime;
        }
    };

    if (m > 0) enumerate(enumerate, 0, 1, 1);
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int test_count;
    cin >> test_count;
    while (test_count--) {
        u64 n, m;
        cin >> n >> m;
        cout << solve(n, m) << '\n';
    }
    return 0;
}
