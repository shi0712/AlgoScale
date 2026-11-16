#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

using namespace std;
using u64 = unsigned long long;
using u128 = __uint128_t;

static constexpr u64 MOD = 998244353;

u64 mul_mod(u64 a, u64 b, u64 mod) {
    return static_cast<u128>(a) * b % mod;
}

u64 pow_mod(u64 a, u64 e, u64 mod) {
    u64 result = 1;
    while (e) {
        if (e & 1) result = mul_mod(result, a, mod);
        a = mul_mod(a, a, mod);
        e >>= 1;
    }
    return result;
}

bool is_prime(u64 n) {
    if (n < 2) return false;
    for (u64 p : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL}) {
        if (n % p == 0) return n == p;
    }

    u64 d = n - 1;
    int s = __builtin_ctzll(d);
    d >>= s;
    for (u64 a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}) {
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

u64 pollard_rho(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    static mt19937_64 rng(random_device{}());
    uniform_int_distribution<u64> distribution(2, n - 2);
    while (true) {
        u64 x = distribution(rng);
        u64 y = x;
        u64 c = distribution(rng);
        u64 d = 1;
        auto advance = [&](u64 v) {
            return (mul_mod(v, v, n) + c) % n;
        };
        while (d == 1) {
            x = advance(x);
            y = advance(advance(y));
            d = gcd(x > y ? x - y : y - x, n);
        }
        if (d != n) return d;
    }
}

void factor(u64 n, vector<u64>& factors) {
    if (n == 1) return;
    if (is_prime(n)) {
        factors.push_back(n);
        return;
    }
    u64 d = pollard_rho(n);
    factor(d, factors);
    factor(n / d, factors);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        u64 n, m;
        cin >> n >> m;

        vector<u64> factors;
        factor(n, factors);
        sort(factors.begin(), factors.end());
        vector<pair<u64, int>> primes;
        for (u64 p : factors) {
            if (!primes.empty() && primes.back().first == p) {
                ++primes.back().second;
            } else {
                primes.push_back({p, 1});
            }
        }

        u64 phi_n = n;
        for (auto [p, e] : primes) phi_n = phi_n / p * (p - 1);

        vector<pair<u64, u64>> divisors = {{1, phi_n}};
        for (auto [p, e] : primes) {
            u64 prime_power = 1;
            for (int i = 0; i < e; ++i) prime_power *= p;
            u64 local_phi = prime_power / p * (p - 1);

            int old_size = static_cast<int>(divisors.size());
            for (int i = 0; i < old_size; ++i) {
                auto [d, weight] = divisors[i];
                u64 base = weight / local_phi;
                u64 previous = local_phi;
                u64 power = p;
                for (int j = 1; j <= e; ++j) {
                    u64 current = 2 * j >= e
                        ? prime_power + static_cast<u64>(e) * local_phi
                        : static_cast<u64>(2 * j + 1) * local_phi;
                    divisors.push_back({d * power, base * (current - previous)});
                    previous = current;
                    power *= p;
                }
            }
        }

        u64 answer = 0;
        for (auto [d, weight] : divisors) {
            u64 multiples = m / d + 1;
            answer = (answer + static_cast<u128>(weight % MOD) * (multiples % MOD)) % MOD;
        }
        cout << answer << '\n';
    }
    return 0;
}
