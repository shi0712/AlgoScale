#include <bits/stdc++.h>
using namespace std;

using u64 = uint64_t;
using u128 = __uint128_t;

constexpr u64 MOD = 998244353;

static u64 rngState = chrono::steady_clock::now().time_since_epoch().count();

u64 random64() {
    u64 z = (rngState += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

u64 mulMod(u64 a, u64 b, u64 mod) {
    return static_cast<u64>(static_cast<u128>(a) * b % mod);
}

u64 powMod(u64 a, u64 e, u64 mod) {
    u64 result = 1;
    while (e) {
        if (e & 1) result = mulMod(result, a, mod);
        a = mulMod(a, a, mod);
        e >>= 1;
    }
    return result;
}

// These bases are deterministic for every n <= 10^12.
bool isPrime(u64 n) {
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

    for (u64 a : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL}) {
        u64 x = powMod(a, d, n);
        if (x == 1 || x == n - 1) continue;

        bool passed = false;
        for (int r = 1; r < s; ++r) {
            x = mulMod(x, x, n);
            if (x == n - 1) {
                passed = true;
                break;
            }
        }
        if (!passed) return false;
    }
    return true;
}

u64 pollardRho(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    while (true) {
        u64 y = random64() % (n - 1) + 1;
        u64 c = random64() % (n - 1) + 1;

        auto next = [&](u64 x) -> u64 {
            return (mulMod(x, x, n) + c) % n;
        };

        u64 g = 1, r = 1;
        u64 x = 0, ys = 0;

        while (g == 1) {
            x = y;
            for (u64 i = 0; i < r; ++i) y = next(y);

            for (u64 k = 0; k < r && g == 1; k += 128) {
                ys = y;
                u64 q = 1;
                u64 len = min<u64>(128, r - k);

                for (u64 i = 0; i < len; ++i) {
                    y = next(y);
                    u64 diff = (x > y ? x - y : y - x);
                    q = mulMod(q, diff, n);
                }
                g = gcd(q, n);
            }
            r <<= 1;
        }

        if (g == n) {
            do {
                ys = next(ys);
                u64 diff = (x > ys ? x - ys : ys - x);
                g = gcd(diff, n);
            } while (g == 1);
        }

        if (g != n) return g;
    }
}

void factor(u64 n, vector<u64>& result) {
    if (n == 1) return;
    if (isPrime(n)) {
        result.push_back(n);
        return;
    }

    u64 d = pollardRho(n);
    factor(d, result);
    factor(n / d, result);
}

struct Option {
    u64 divisor;
    u64 coefficient;
};

struct Data {
    vector<vector<Option>> options;
    u64 zeroValue = 1; // f(n, 0)
};

Data buildData(u64 n) {
    vector<u64> factors;

    static constexpr u64 smallPrimes[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37
    };

    u64 remaining = n;
    for (u64 p : smallPrimes) {
        while (remaining % p == 0) {
            factors.push_back(p);
            remaining /= p;
        }
    }
    factor(remaining, factors);
    sort(factors.begin(), factors.end());

    Data data;

    for (size_t i = 0; i < factors.size();) {
        size_t j = i;
        u64 p = factors[i];
        while (j < factors.size() && factors[j] == p) ++j;

        int e = static_cast<int>(j - i);
        u64 q = 1;
        for (int t = 0; t < e; ++t) q *= p;

        u64 phi = q - q / p;

        data.zeroValue =
            data.zeroValue * ((q + static_cast<u64>(e) * phi) % MOD) % MOD;

        vector<Option> choices;
        choices.push_back({1, phi % MOD});

        u64 divisor = 1;
        for (int t = 1; t <= (e + 1) / 2; ++t) {
            divisor *= p;

            u64 coefficient;
            if (2 * t < e) {
                coefficient = 2 * phi;
            } else if (2 * t == e) {
                coefficient = phi + q;
            } else {
                coefficient = q;
            }

            choices.push_back({divisor, coefficient % MOD});
        }

        data.options.push_back(move(choices));
        i = j;
    }

    return data;
}

u64 solve(const Data& data, u64 m) {
    u64 answer = data.zeroValue;
    if (m == 0) return answer;

    auto dfs = [&](auto&& self, size_t index, u64 divisor,
                   u64 coefficient) -> void {
        if (index == data.options.size()) {
            u64 term = coefficient * ((m / divisor) % MOD) % MOD;
            answer += term;
            if (answer >= MOD) answer -= MOD;
            return;
        }

        for (const Option& option : data.options[index]) {
            // The options are in increasing divisor order.
            if (option.divisor > m / divisor) break;

            self(self, index + 1,
                 divisor * option.divisor,
                 coefficient * option.coefficient % MOD);
        }
    };

    dfs(dfs, 0, 1, 1);
    return answer;
}

struct CustomHash {
    static u64 splitmix64(u64 x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }

    size_t operator()(u64 x) const {
        static const u64 seed =
            chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(x + seed);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    unordered_map<u64, Data, CustomHash> cache;
    cache.reserve(2 * T);
    cache.max_load_factor(0.7f);

    while (T--) {
        u64 n, m;
        cin >> n >> m;

        auto it = cache.find(n);
        if (it == cache.end()) {
            it = cache.emplace(n, buildData(n)).first;
        }

        cout << solve(it->second, m) << '\n';
    }

    return 0;
}
