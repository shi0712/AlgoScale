#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using u128 = __uint128_t;

constexpr long long MOD = 998244353;

u64 mod_pow_u64(u64 a, u64 d, u64 mod) {
    a %= mod;
    u64 res = 1 % mod;
    while (d) {
        if (d & 1) res = (u128)res * a % mod;
        a = (u128)a * a % mod;
        d >>= 1;
    }
    return res;
}

long long mod_pow_mod(long long a, long long d) {
    a %= MOD;
    long long res = 1;
    while (d > 0) {
        if (d & 1) res = res * a % MOD;
        a = a * a % MOD;
        d >>= 1;
    }
    return res;
}

bool isPrime(u64 n) {
    if (n < 2) return false;

    static const u64 small[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (u64 p : small) {
        if (n % p == 0) return n == p;
    }

    u64 d = n - 1;
    int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    static const u64 bases[] = {
        2ULL, 325ULL, 9375ULL, 28178ULL,
        450775ULL, 9780504ULL, 1795265022ULL
    };

    for (u64 a : bases) {
        if (a % n == 0) continue;

        u64 x = mod_pow_u64(a % n, d, n);
        if (x == 1 || x == n - 1) continue;

        bool passed = false;
        for (int r = 1; r < s; ++r) {
            x = (u128)x * x % n;
            if (x == n - 1) {
                passed = true;
                break;
            }
        }

        if (!passed) return false;
    }

    return true;
}

u64 gcd_u64(u64 a, u64 b) {
    while (b) {
        u64 t = a % b;
        a = b;
        b = t;
    }
    return a;
}

mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());

u64 pollard(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    while (true) {
        u64 c = rng() % (n - 1) + 1;
        u64 x = rng() % n;
        u64 y = x;
        u64 d = 1;

        auto f = [&](u64 v) -> u64 {
            return ((u128)v * v + c) % n;
        };

        while (d == 1) {
            x = f(x);
            y = f(f(y));

            u64 diff = (x > y ? x - y : y - x);
            d = gcd_u64(diff, n);
        }

        if (d != n) return d;
    }
}

void factor_rec(u64 n, vector<u64>& fac) {
    if (n == 1) return;

    if (isPrime(n)) {
        fac.push_back(n);
        return;
    }

    u64 d = pollard(n);
    factor_rec(d, fac);
    factor_rec(n / d, fac);
}

struct Info {
    u64 p;
    int e;
    int h;
    vector<long long> coef;
    vector<u64> powers;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const int LIM = 10000;
    vector<bool> composite(LIM + 1, false);
    vector<int> primes;

    for (int i = 2; i <= LIM; ++i) {
        if (!composite[i]) {
            primes.push_back(i);
            if ((long long)i * i <= LIM) {
                for (int j = i * i; j <= LIM; j += i) {
                    composite[j] = true;
                }
            }
        }
    }

    int T;
    cin >> T;

    while (T--) {
        u64 n, m;
        cin >> n >> m;

        vector<u64> fac;
        u64 rem = n;

        for (int p : primes) {
            if ((u64)p * p > rem) break;

            if (rem % p == 0) {
                while (rem % p == 0) {
                    fac.push_back((u64)p);
                    rem /= p;
                }
            }
        }

        if (rem > 1) {
            factor_rec(rem, fac);
        }

        sort(fac.begin(), fac.end());

        vector<pair<u64, int>> pf;
        for (u64 x : fac) {
            if (pf.empty() || pf.back().first != x) {
                pf.push_back({x, 1});
            } else {
                ++pf.back().second;
            }
        }

        vector<Info> infos;
        infos.reserve(pf.size());

        for (auto [p, e] : pf) {
            int h = (e + 1) / 2;

            long long pm = p % MOD;
            long long pe1 = mod_pow_mod(pm, e - 1);

            long long phi = pe1 * ((pm - 1 + MOD) % MOD) % MOD;

            vector<long long> coef;
            coef.reserve(h + 1);

            coef.push_back(phi);

            long long two_phi = 2 * phi % MOD;
            for (int a = 1; a < h; ++a) {
                coef.push_back(two_phi);
            }

            long long ch;
            if (e % 2 == 0) {
                long long val = (2 * pm % MOD - 1 + MOD) % MOD;
                ch = pe1 * val % MOD;
            } else {
                ch = pe1 * pm % MOD;
            }
            coef.push_back(ch);

            vector<u64> powers(h + 1);
            powers[0] = 1;
            for (int a = 1; a <= h; ++a) {
                powers[a] = powers[a - 1] * p;
            }

            infos.push_back({p, e, h, move(coef), move(powers)});
        }

        long long ans = 0;

        auto dfs = [&](auto&& self, int idx, u64 d, long long w) -> void {
            if (idx == (int)infos.size()) {
                long long cnt = (m / d + 1ULL) % MOD;
                ans = (ans + w * cnt) % MOD;
                return;
            }

            const Info& info = infos[idx];

            for (int a = 0; a <= info.h; ++a) {
                u64 nd = d * info.powers[a];
                long long nw = w * info.coef[a] % MOD;
                self(self, idx + 1, nd, nw);
            }
        };

        dfs(dfs, 0, 1, 1);

        cout << ans % MOD << '\n';
    }

    return 0;
}
