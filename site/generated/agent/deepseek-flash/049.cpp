#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long u64;
typedef __int128 i128;

static const u64 MOD = 998244353ULL;

// ---------------------------------------------------------------
//  Reduction
//  ---------
//  k(a+b) == ab (mod n)  <=>  (a-k)(b-k) == k^2 (mod n), and
//  a -> a-k is a bijection of Z_n, hence
//      f(n,k) = #{(x,y) mod n : x*y == k^2 (mod n)} = G(gcd(k^2,n)),
//  where for a divisor d of n, G(d) is that count for any c with
//  gcd(c,n)=d.
//
//  For a prime power p^e || n, with v = min(v_p(c), e), the number
//  of solutions of xy == c (mod p^e) is
//      L(v) = (v+1) p^(e-1) (p-1)              for 0 <= v < e,
//      L(e) = p^(e-1) (p + e(p-1)),
//  and G is multiplicative over the prime powers, so G(d) depends
//  only on min(v_p(d),e_p) - as used above.
//
//  Mobius inversion on the divisor lattice (chain per prime) gives
//  A(p^v) = G(p^v) - G(p^(v-1)), i.e.
//      A(p^v) = p^(e-1)(p-1)  for v < e,     A(p^e) = p^e ,
//  and multiplicatively A(c) for c | n.  Then
//      sum_{k=0}^{m} G(gcd(k^2,n))
//        = sum_{c|n} A(c) * #{0<=k<=m : c | k^2}
//        = sum_{c|n} A(c) * (m / s(c) + 1),
//  with s(c) = prod_{p^v || c} p^ceil(v/2)  (c | k^2  <=>  s(c) | k).
// ---------------------------------------------------------------

// modular helpers for mod 998244353
static inline u64 mmul(u64 a, u64 b) { return (u64)((i128)a * b % MOD); }
static u64 mpow(u64 a, u64 d) {
    u64 r = 1;
    a %= MOD;
    while (d) {
        if (d & 1) r = mmul(r, a);
        a = mmul(a, a);
        d >>= 1;
    }
    return r;
}

// ---------------- 64-bit primality / factorisation ----------------
static inline u64 mulmod64(u64 a, u64 b, u64 m) { return (u64)((i128)a * b % m); }
static u64 powmod64(u64 a, u64 d, u64 m) {
    u64 r = 1 % m;
    a %= m;
    while (d) {
        if (d & 1) r = mulmod64(r, a, m);
        a = mulmod64(a, a, m);
        d >>= 1;
    }
    return r;
}

static bool isPrime(u64 n) {
    if (n < 2) return false;
    static const u64 small[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (u64 p : small)
        if (n % p == 0) return n == p;
    u64 d = n - 1;
    int s = 0;
    while (!(d & 1)) { d >>= 1; ++s; }
    for (u64 a : small) {
        u64 x = powmod64(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool composite = true;
        for (int i = 1; i < s; ++i) {
            x = mulmod64(x, x, n);
            if (x == n - 1) { composite = false; break; }
        }
        if (composite) return false;
    }
    return true;
}

// Brent's variant of Pollard's rho
static u64 pollard(u64 n) {
    if ((n & 1) == 0) return 2;
    static mt19937_64 rng(0x9e3779b97f4a7c15ULL);
    while (true) {
        u64 c = rng() % (n - 1) + 1;
        u64 y = rng() % (n - 1) + 1;
        const u64 batch = 128;
        u64 g = 1, r = 1, q = 1, x = 0, ys = 0;
        while (g == 1) {
            x = y;
            for (u64 i = 0; i < r; ++i) y = (mulmod64(y, y, n) + c) % n;
            for (u64 k = 0; k < r && g == 1; k += batch) {
                ys = y;
                u64 lim = min(batch, r - k);
                for (u64 i = 0; i < lim; ++i) {
                    y = (mulmod64(y, y, n) + c) % n;
                    u64 diff = x > y ? x - y : y - x;
                    q = mulmod64(q, diff, n);
                }
                g = std::gcd(q, n);
            }
            r <<= 1;
        }
        if (g == n) {  // rare: the batched product vanished, back up
            do {
                ys = (mulmod64(ys, ys, n) + c) % n;
                u64 diff = x > ys ? x - ys : ys - x;
                g = std::gcd(diff, n);
            } while (g == 1);
        }
        if (g != n) return g;
    }
}

static void factorRec(u64 n, vector<u64>& fac) {
    if (n == 1) return;
    if (isPrime(n)) { fac.push_back(n); return; }
    u64 d = pollard(n);
    factorRec(d, fac);
    factorRec(n / d, fac);
}

struct PrimePow { u64 p; int e; };

static u64 solve(u64 n, u64 m) {
    if (n == 1) return (m + 1) % MOD;

    vector<u64> fac;
    fac.reserve(32);
    factorRec(n, fac);
    sort(fac.begin(), fac.end());

    vector<PrimePow> pp;
    for (size_t i = 0; i < fac.size();) {
        size_t j = i;
        while (j < fac.size() && fac[j] == fac[i]) ++j;
        pp.push_back({fac[i], (int)(j - i)});
        i = j;
    }

    const int k = (int)pp.size();
    // A[i][v] = A(p^v) mod MOD ; S[i][v] = p^ceil(v/2)  (exact)
    vector<vector<u64>> A(k), S(k);
    for (int i = 0; i < k; ++i) {
        const u64 p = pp[i].p;
        const int e = pp[i].e;
        A[i].resize(e + 1);
        S[i].resize(e + 1);
        u64 base = mmul(mpow(p, (u64)(e - 1)), (p - 1) % MOD);  // p^(e-1)(p-1)
        for (int v = 0; v <= e; ++v)
            A[i][v] = (v < e) ? base : mpow(p, (u64)e);
        for (int v = 0; v <= e; ++v) {
            int h = (v + 1) / 2;  // ceil(v/2)
            u64 s = 1;
            for (int t = 0; t < h; ++t) s *= p;
            S[i][v] = s;
        }
    }

    u64 ans = 0;
    // DFS over all divisors c | n, accumulating A(c) and s(c)
    function<void(int, u64, u64)> dfs = [&](int pos, u64 a, u64 s) {
        if (pos == k) {
            ans = (ans + mmul(a, (m / s + 1) % MOD)) % MOD;
            return;
        }
        for (int v = 0; v <= pp[pos].e; ++v)
            dfs(pos + 1, mmul(a, A[pos][v]), s * S[pos][v]);
    };
    dfs(0, 1, 1);
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    string out;
    out.reserve((size_t)T * 12);
    while (T--) {
        u64 n, m;
        cin >> n >> m;
        out += to_string(solve(n, m));
        out += '\n';
    }
    fputs(out.c_str(), stdout);
    return 0;
}
