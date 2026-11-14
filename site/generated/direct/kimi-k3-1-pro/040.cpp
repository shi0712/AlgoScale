#include <bits/stdc++.h>
using namespace std;

using u64 = uint64_t;
using u128 = unsigned __int128;

static constexpr u64 MOD = 998244353;

static u64 mulMod(u64 a, u64 b, u64 mod) {
    return (u64)((u128)a * b % mod);
}

static u64 powMod(u64 a, u64 e, u64 mod) {
    u64 result = 1;
    while (e > 0) {
        if (e & 1) result = mulMod(result, a, mod);
        a = mulMod(a, a, mod);
        e >>= 1;
    }
    return result;
}

static bool isPrime(u64 n) {
    if (n < 2) return false;

    static constexpr u64 smallPrimes[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37
    };
    for (u64 p : smallPrimes) {
        if (n % p == 0) return n == p;
    }

    u64 d = n - 1;
    int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    // Deterministic for all unsigned 64-bit integers.
    static constexpr u64 bases[] = {
        2, 325, 9375, 28178, 450775, 9780504, 1795265022
    };

    for (u64 a : bases) {
        if (a % n == 0) continue;

        u64 x = powMod(a, d, n);
        if (x == 1 || x == n - 1) continue;

        bool composite = true;
        for (int r = 1; r < s; ++r) {
            x = mulMod(x, x, n);
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

static u64 pollardRho(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    static mt19937_64 rng(0x9e3779b97f4a7c15ULL);

    while (true) {
        u64 c = rng() % (n - 1) + 1;
        u64 x = rng() % n;
        u64 y = x;
        u64 g = 1;

        auto advance = [&](u64 v) {
            return (u64)(((u128)v * v + c) % n);
        };

        while (g == 1) {
            x = advance(x);
            y = advance(advance(y));
            u64 diff = x > y ? x - y : y - x;
            g = std::gcd(diff, n);
        }

        if (g != n) return g;
    }
}

static void factorRecursive(u64 n, vector<u64>& primes) {
    if (n == 1) return;
    if (isPrime(n)) {
        primes.push_back(n);
        return;
    }

    u64 d = pollardRho(n);
    factorRecursive(d, primes);
    factorRecursive(n / d, primes);
}

static vector<pair<u64, int>> factorize(u64 n) {
    vector<u64> primes;
    factorRecursive(n, primes);
    sort(primes.begin(), primes.end());

    vector<pair<u64, int>> factors;
    for (u64 p : primes) {
        if (!factors.empty() && factors.back().first == p) {
            ++factors.back().second;
        } else {
            factors.push_back({p, 1});
        }
    }
    return factors;
}

struct Option {
    u64 rho;
    u64 weight;
};

class CaseSolver {
public:
    CaseSolver(u64 n, u64 m) : n(n), m(m) {}

    u64 solve() {
        auto factors = factorize(n);

        u64 answerAtZero = 1;

        for (auto [p, e] : factors) {
            vector<u64> power(e + 1);
            power[0] = 1;
            for (int i = 1; i <= e; ++i) {
                power[i] = power[i - 1] * p;
            }

            vector<Option> local;
            u64 localWeightSum = 0;

            for (int s = 0; s <= e; ++s) {
                // Local divisor is p^s.
                int remaining = e - s;
                u64 phi;
                if (remaining == 0) {
                    phi = 1;
                } else {
                    phi = power[remaining] - power[remaining - 1];
                }

                // p^s * phi(p^(e-s))
                u64 weight = power[s] * phi;

                // p^ceil(s/2), or 1 when s=0.
                u64 rho = (s == 0 ? 1 : power[(s + 1) / 2]);

                local.push_back({rho, weight});
                localWeightSum += weight;
            }

            options.push_back(std::move(local));
            answerAtZero = (u64)((u128)answerAtZero *
                                 (localWeightSum % MOD) % MOD);
        }

        if (m == 0) return answerAtZero;

        enumerate(0, 1, 1);
        return answer;
    }

private:
    u64 n, m;
    u64 answer = 0;
    vector<vector<Option>> options;

    void enumerate(size_t index, u64 rho, u64 weight) {
        if (index == options.size()) {
            u64 multiplier = m / rho + 1;
            u64 term = (u64)((u128)(weight % MOD) *
                             (multiplier % MOD) % MOD);

            answer += term;
            if (answer >= MOD) answer -= MOD;
            return;
        }

        for (const Option& option : options[index]) {
            enumerate(index + 1,
                      rho * option.rho,
                      weight * option.weight);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        u64 n, m;
        cin >> n >> m;
        cout << CaseSolver(n, m).solve() << '\n';
    }

    return 0;
}
