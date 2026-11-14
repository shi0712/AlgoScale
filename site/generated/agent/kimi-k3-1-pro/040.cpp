#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using u128 = __uint128_t;

static constexpr u64 MOD = 998244353;

u64 mulMod(u64 a, u64 b, u64 mod) {
    return (u128)a * b % mod;
}

u64 powerMod(u64 a, u64 e, u64 mod) {
    u64 result = 1;
    while (e) {
        if (e & 1) result = mulMod(result, a, mod);
        a = mulMod(a, a, mod);
        e >>= 1;
    }
    return result;
}

bool isPrime(u64 n) {
    if (n < 2) return false;
    for (u64 p : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL}) {
        if (n % p == 0) return n == p;
    }
    u64 d = n - 1;
    int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }
    for (u64 a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}) {
        if (a % n == 0) continue;
        u64 x = powerMod(a % n, d, n);
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

u64 pollardRho(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;
    static mt19937_64 rng(0x9e3779b97f4a7c15ULL);
    uniform_int_distribution<u64> dist(2, n - 2);
    while (true) {
        u64 c = dist(rng);
        u64 y = dist(rng);
        u64 x = 0, ys = 0, q = 1, g = 1;
        u64 r = 1;
        auto step = [&](u64 v) {
            u64 z = mulMod(v, v, n) + c;
            return z >= n ? z - n : z;
        };
        while (g == 1) {
            x = y;
            for (u64 i = 0; i < r; ++i) y = step(y);
            for (u64 k = 0; k < r && g == 1; k += 128) {
                ys = y;
                u64 limit = min<u64>(128, r - k);
                q = 1;
                for (u64 i = 0; i < limit; ++i) {
                    y = step(y);
                    u64 difference = x > y ? x - y : y - x;
                    q = mulMod(q, difference, n);
                }
                g = gcd(q, n);
            }
            r <<= 1;
        }
        if (g == n) {
            do {
                ys = step(ys);
                u64 difference = x > ys ? x - ys : ys - x;
                g = gcd(difference, n);
            } while (g == 1);
        }
        if (g != n) return g;
    }
}

void factorRecursive(u64 n, vector<u64>& factors) {
    if (n == 1) return;
    if (isPrime(n)) {
        factors.push_back(n);
        return;
    }
    u64 d = pollardRho(n);
    factorRecursive(d, factors);
    factorRecursive(n / d, factors);
}

vector<pair<u64, int>> factorize(u64 n) {
    vector<u64> factors;
    factorRecursive(n, factors);
    sort(factors.begin(), factors.end());
    vector<pair<u64, int>> result;
    for (u64 p : factors) {
        if (!result.empty() && result.back().first == p) ++result.back().second;
        else result.push_back({p, 1});
    }
    return result;
}

u64 solve(u64 m, const vector<pair<u64, int>>& factors) {
    u64 answer = 1;
    vector<vector<u64>> transforms;
    vector<int> limits;
    for (auto [p, e] : factors) {
        u64 pe = 1;
        for (int i = 0; i < e; ++i) pe *= p;
        u64 a = pe - pe / p;
        u64 b = (pe + (u64)e * a) % MOD;
        answer = answer * b % MOD;
        int c = (e + 1) / 2;
        vector<u64> local(c + 1);
        local[0] = a % MOD;
        for (int t = 1; t < c; ++t) local[t] = 2 * local[0] % MOD;
        local[c] = (b + MOD - (2 * c - 1) % MOD * local[0] % MOD) % MOD;
        transforms.push_back(move(local));
        limits.push_back(c);
    }
    int count = factors.size();
    function<void(int, u64, u64)> enumerate = [&](int index, u64 divisor, u64 h) {
        if (index == count) {
            answer = (answer + (m / divisor) % MOD * h) % MOD;
            return;
        }
        u64 d = divisor;
        for (int t = 0; t <= limits[index]; ++t) {
            enumerate(index + 1, d, h * transforms[index][t] % MOD);
            d *= factors[index].first;
        }
    };
    enumerate(0, 1, 1);
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    cin >> T;
    vector<pair<u64, u64>> queries(T);
    for (auto& [n, m] : queries) cin >> n >> m;
    unordered_map<u64, vector<pair<u64, int>>> cache;
    cache.reserve(T * 2 + 1);
    for (auto [n, m] : queries) {
        auto it = cache.find(n);
        if (it == cache.end()) it = cache.emplace(n, factorize(n)).first;
        cout << solve(m, it->second) << '\n';
    }
    return 0;
}
