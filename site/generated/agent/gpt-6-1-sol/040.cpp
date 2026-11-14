#include <bits/stdc++.h>
using namespace std;

using u64 = uint64_t;
using u128 = __uint128_t;
constexpr u64 MOD = 998244353;

u64 mul_mod(u64 a, u64 b, u64 mod) {
    return (u128)a * b % mod;
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
    for (u64 p : {2, 3, 5, 7, 11}) {
        if (n % p == 0) return n == p;
    }
    u64 d = n - 1;
    int s = 0;
    while (!(d & 1)) d >>= 1, ++s;
    // These bases are deterministic for n < 2,152,302,898,747.
    for (u64 a : {2, 3, 5, 7, 11}) {
        u64 x = pow_mod(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool passed = false;
        for (int r = 1; r < s; ++r) {
            x = mul_mod(x, x, n);
            if (x == n - 1) {
                passed = true;
                break;
            }
        }
        if (!passed) return false;
    }
    return true;
}

mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());

u64 pollard_rho(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;
    while (true) {
        u64 y = rng() % (n - 1) + 1;
        u64 c = rng() % (n - 1) + 1;
        auto next = [&](u64 x) { return (mul_mod(x, x, n) + c) % n; };
        u64 g = 1, r = 1, x = 0, ys = 0;
        while (g == 1) {
            x = y;
            for (u64 i = 0; i < r; ++i) y = next(y);
            for (u64 k = 0; k < r && g == 1; k += 128) {
                ys = y;
                u64 product = 1;
                for (u64 i = 0; i < min<u64>(128, r - k); ++i) {
                    y = next(y);
                    u64 diff = x > y ? x - y : y - x;
                    product = mul_mod(product, diff, n);
                }
                g = gcd(product, n);
            }
            r <<= 1;
        }
        if (g == n) {
            do {
                ys = next(ys);
                g = gcd(x > ys ? x - ys : ys - x, n);
            } while (g == 1);
        }
        if (g != n) return g;
    }
}

void factor(u64 n, vector<u64>& factors) {
    if (n == 1) return;
    if (is_prime(n)) {
        factors.push_back(n);
        return;
    }
    u64 root = sqrtl(n);
    while ((root + 1) * (root + 1) <= n) ++root;
    while (root * root > n) --root;
    if (root * root == n) {
        factor(root, factors);
        factor(root, factors);
        return;
    }
    u64 d = pollard_rho(n);
    factor(d, factors);
    factor(n / d, factors);
}

struct Dimension {
    vector<pair<u64, u64>> terms; // (divisibility requirement, coefficient)
};

u64 solve(u64 n, u64 m) {
    vector<u64> factors;
    u64 remaining = n;
    for (u64 p : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37,
                  41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}) {
        while (remaining % p == 0) {
            factors.push_back(p);
            remaining /= p;
        }
    }
    factor(remaining, factors);
    sort(factors.begin(), factors.end());

    vector<Dimension> dimensions;
    u64 answer = 1;
    for (size_t i = 0; i < factors.size();) {
        size_t j = i;
        u64 p = factors[i], pe = 1;
        while (j < factors.size() && factors[j] == p) pe *= p, ++j;
        int e = int(j - i);
        u64 phi = pe / p * (p - 1);

        // k=0 contributes product_p (p^e + e*phi(p^e)).
        answer = answer * ((pe % MOD + e * (phi % MOD)) % MOD) % MOD;

        Dimension dim;
        dim.terms.push_back({1, phi % MOD});
        u64 power = 1;
        for (int r = 1; r <= (e + 1) / 2; ++r) {
            power *= p;
            u64 coefficient;
            if (2 * r < e) coefficient = 2 * phi;
            else if (2 * r == e) coefficient = phi + pe;
            else coefficient = pe;
            dim.terms.push_back({power, coefficient % MOD});
        }
        dimensions.push_back(move(dim));
        i = j;
    }

    // For d|n, fixing gcd(x,n)=d gives d*phi(n/d) solutions
    // exactly when d|k^2. This requires prod p^ceil(v_p(d)/2) | k.
    // The local coefficients above group equal requirements together.
    auto dfs = [&](auto&& self, size_t pos, u64 requirement, u64 coefficient) -> void {
        if (pos == dimensions.size()) {
            answer = (answer + coefficient * ((m / requirement) % MOD)) % MOD;
            return;
        }
        u64 limit = m / requirement;
        for (auto [power, weight] : dimensions[pos].terms) {
            if (power > limit) break;
            self(self, pos + 1, requirement * power, coefficient * weight % MOD);
        }
    };
    if (m) dfs(dfs, 0, 1, 1);
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    cin >> T;
    while (T--) {
        u64 n, m;
        cin >> n >> m;
        cout << solve(n, m) << '\n';
    }
}
