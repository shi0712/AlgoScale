#include <bits/stdc++.h>
using namespace std;
using u64 = uint64_t;
using u128 = __uint128_t;
constexpr long long MOD = 998244353;

u64 mulmod(u64 a, u64 b, u64 mod) { return (u128)a * b % mod; }
u64 powmod(u64 a, u64 e, u64 mod) {
    u64 r = 1;
    while (e) { if (e & 1) r = mulmod(r, a, mod); a = mulmod(a, a, mod); e >>= 1; }
    return r;
}
bool is_prime(u64 n) {
    if (n < 2) return false;
    for (u64 p : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL}) {
        if (n % p == 0) return n == p;
    }
    u64 d = n - 1; int s = __builtin_ctzll(d); d >>= s;
    // These bases are deterministic well beyond the input limit of 10^12.
    for (u64 a : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL}) {
        u64 x = powmod(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool witness = true;
        for (int i = 1; i < s; ++i) {
            x = mulmod(x, x, n);
            if (x == n - 1) { witness = false; break; }
        }
        if (witness) return false;
    }
    return true;
}
mt19937_64 rng(712367821);
u64 pollard(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;
    while (true) {
        u64 y = rng() % (n - 1) + 1, c = rng() % (n - 1) + 1;
        auto f = [&](u64 x) { return (mulmod(x, x, n) + c) % n; };
        u64 g = 1, r = 1, x = 0, ys = 0;
        while (g == 1) {
            x = y;
            for (u64 i = 0; i < r; ++i) y = f(y);
            for (u64 k = 0; k < r && g == 1; k += 128) {
                ys = y;
                u64 q = 1;
                for (u64 i = 0; i < min<u64>(128, r - k); ++i) {
                    y = f(y);
                    u64 diff = x > y ? x - y : y - x;
                    q = mulmod(q, diff, n);
                }
                g = gcd(q, n);
            }
            r <<= 1;
        }
        if (g == n) {
            do { ys = f(ys); g = gcd(x > ys ? x - ys : ys - x, n); } while (g == 1);
        }
        if (g != n) return g;
    }
}
void factor(u64 n, vector<u64>& fac) {
    if (n == 1) return;
    if (is_prime(n)) { fac.push_back(n); return; }
    u64 d = pollard(n);
    factor(d, fac); factor(n / d, fac);
}

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int T; cin >> T;
    while (T--) {
        u64 n, m; cin >> n >> m;
        vector<u64> fac;
        factor(n, fac);
        sort(fac.begin(), fac.end());
        vector<vector<pair<u64, long long>>> choices;
        for (size_t i = 0; i < fac.size();) {
            size_t j = i; u64 p = fac[i], pe = 1;
            while (j < fac.size() && fac[j] == p) { pe *= p; ++j; }
            int e = j - i, h = (e + 1) / 2;
            long long phi = (pe - pe / p) % MOD;
            vector<pair<u64, long long>> opt{{1, phi}};
            u64 power = 1;
            for (int t = 1; t <= h; ++t) {
                power *= p;
                long long coeff;
                if (t < h) coeff = 2 * phi % MOD;
                else coeff = (pe % MOD + (long long)(e - 2 * t + 1) * phi) % MOD;
                opt.emplace_back(power, coeff);
            }
            choices.push_back(move(opt));
            i = j;
        }
        long long answer = 0;
        auto dfs = [&](auto&& self, int idx, u64 divisor, long long weight) -> void {
            if (idx == (int)choices.size()) {
                answer = (answer + weight * ((m / divisor + 1) % MOD)) % MOD;
                return;
            }
            for (auto [d, w] : choices[idx]) self(self, idx + 1, divisor * d, weight * w % MOD);
        };
        dfs(dfs, 0, 1, 1);
        cout << answer << '\n';
    }
}
