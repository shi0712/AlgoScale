#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using u128 = __uint128_t;

static constexpr u64 MOD = 998244353ULL;

// ---------- modular arithmetic (mod MOD) ----------
static inline u64 addmod(u64 a, u64 b) { a += b; if (a >= MOD) a -= MOD; return a; }
static inline u64 submod(u64 a, u64 b) { a += MOD - b; if (a >= MOD) a -= MOD; return a; }
static inline u64 mulmod(u64 a, u64 b) { return (a * b) % MOD; }

static u64 powmod(u64 a, u64 e) {
    u64 r = 1;
    while (e) {
        if (e & 1) r = mulmod(r, a);
        a = mulmod(a, a);
        e >>= 1;
    }
    return r;
}

// ---------- Miller-Rabin + Pollard-Rho for n <= 1e12 ----------
static inline u64 mul_mod_n(u64 a, u64 b, u64 mod) {
    return (u64)((u128)a * b % mod);
}

static u64 pow_mod_n(u64 a, u64 e, u64 mod) {
    u64 r = 1 % mod;
    a %= mod;
    while (e) {
        if (e & 1) r = mul_mod_n(r, a, mod);
        a = mul_mod_n(a, a, mod);
        e >>= 1;
    }
    return r;
}

static bool is_prime(u64 n) {
    if (n < 2) return false;
    static const u64 small[] = {2,3,5,7,11,13,17,19,23,29,31,37};
    for (u64 p : small) {
        if (n % p == 0) return n == p;
    }
    u64 d = n - 1;
    int s = 0;
    while ((d & 1) == 0) { d >>= 1; ++s; }
    for (u64 a : small) {
        if (a >= n) continue;
        u64 x = pow_mod_n(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool comp = true;
        for (int r = 1; r < s; ++r) {
            x = mul_mod_n(x, x, n);
            if (x == n - 1) { comp = false; break; }
        }
        if (comp) return false;
    }
    return true;
}

static u64 rng_state = 0x9e3779b97f4a7c15ULL;
static inline u64 splitmix64() {
    u64 z = (rng_state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static u64 gcd_u64(u64 a, u64 b) {
    while (b) { u64 t = a % b; a = b; b = t; }
    return a;
}

static u64 pollard_rho(u64 n) {
    if ((n & 1) == 0) return 2;
    if (n % 3 == 0) return 3;
    const u64 m = 128;
    while (true) {
        u64 c = splitmix64() % (n - 1) + 1;
        u64 y = splitmix64() % n;
        u64 g = 1, r = 1, q = 1;
        u64 x = 0, ys = 0;
        auto f = [&](u64 v) -> u64 {
            return (mul_mod_n(v, v, n) + c) % n;
        };
        while (g == 1) {
            x = y;
            for (u64 i = 0; i < r; ++i) y = f(y);
            for (u64 k = 0; k < r && g == 1; k += m) {
                ys = y;
                u64 lim = min(m, r - k);
                for (u64 i = 0; i < lim; ++i) {
                    y = f(y);
                    u64 diff = (x > y) ? (x - y) : (y - x);
                    q = mul_mod_n(q, diff, n);
                }
                g = gcd_u64(q, n);
            }
            r <<= 1;
        }
        if (g == n) {
            do {
                ys = f(ys);
                u64 diff = (x > ys) ? (x - ys) : (ys - x);
                g = gcd_u64(diff, n);
            } while (g == 1);
        }
        if (g != n) return g;
    }
}

static void factor_rec(u64 n, vector<u64>& fac) {
    if (n == 1) return;
    if (is_prime(n)) { fac.push_back(n); return; }
    u64 d = pollard_rho(n);
    factor_rec(d, fac);
    factor_rec(n / d, fac);
}

// ---------- main logic ----------
struct PrimePow {
    u64 p;
    int e;
    int c;
    vector<u64> pows;   // p^0 .. p^c
    vector<u64> gs;     // G(p^a), a=0..c
};

static u64 answer_for(const vector<PrimePow>& pps, u64 m) {
    // Iterate over all divisors q of D, with coefficient G(q).
    vector<pair<u64,u64>> cur;
    cur.reserve(2);
    cur.push_back({1ULL, 1ULL});
    for (const auto& pp : pps) {
        vector<pair<u64,u64>> nxt;
        nxt.reserve(cur.size() * (pp.c + 1));
        for (auto [q, g] : cur) {
            for (int a = 0; a <= pp.c; ++a) {
                nxt.push_back({q * pp.pows[a], mulmod(g, pp.gs[a])});
            }
        }
        cur.swap(nxt);
    }
    u64 ans = 0;
    for (auto [q, g] : cur) {
        u64 term = (m / q + 1) % MOD;
        ans += mulmod(g, term);
        if (ans >= MOD) ans -= MOD;
    }
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    vector<u64> ns(T), ms(T);
    for (int i = 0; i < T; ++i) cin >> ns[i] >> ms[i];

    vector<int> ord(T);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) { return ns[a] < ns[b]; });

    vector<u64> ans(T, 0);
    int i = 0;
    while (i < T) {
        int j = i;
        u64 n = ns[ord[i]];
        while (j < T && ns[ord[j]] == n) ++j;

        vector<PrimePow> pps;
        if (n == 1) {
            // no prime factors: D = 1, G(1) = 1
        } else {
            vector<u64> fac;
            factor_rec(n, fac);
            sort(fac.begin(), fac.end());
            int k = 0;
            while (k < (int)fac.size()) {
                int l = k;
                while (l < (int)fac.size() && fac[l] == fac[k]) ++l;
                u64 p = fac[k];
                int e = l - k;
                int c = (e + 1) / 2; // ceil(e/2)

                u64 pmod = p % MOD;
                u64 pe1 = powmod(pmod, (u64)(e - 1)); // p^(e-1) mod MOD
                u64 A = mulmod(submod(pmod, 1), pe1); // (p-1) p^(e-1)
                u64 H = addmod(mulmod(A, (u64)(e + 1)), pe1); // f(p^e)

                PrimePow pp;
                pp.p = p;
                pp.e = e;
                pp.c = c;
                pp.pows.resize(c + 1);
                pp.gs.resize(c + 1);
                pp.pows[0] = 1;
                for (int a = 1; a <= c; ++a) pp.pows[a] = pp.pows[a - 1] * p;
                pp.gs[0] = A; // G(p^0) = F(p^0) = (p-1)p^(e-1)
                for (int a = 1; a < c; ++a) pp.gs[a] = addmod(A, A); // 2A
                // last: G(p^c) = H - A*(2c-1)
                u64 coef = (u64)(2 * c - 1) % MOD;
                pp.gs[c] = submod(H, mulmod(A, coef));
                pps.push_back(std::move(pp));
                k = l;
            }
        }

        for (int t = i; t < j; ++t) {
            int idx = ord[t];
            ans[idx] = answer_for(pps, ms[idx]);
        }
        i = j;
    }

    for (int t = 0; t < T; ++t) cout << ans[t] << '\n';
    return 0;
}
