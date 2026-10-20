#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using u128 = __uint128_t;

constexpr u64 MOD = 998244353;

u64 mul_mod(u64 a, u64 b, u64 mod) {
    return (u128)a * b % mod;
}

u64 pow_mod(u64 a, u64 d, u64 mod) {
    u64 res = 1 % mod;
    while (d) {
        if (d & 1) res = mul_mod(res, a, mod);
        a = mul_mod(a, a, mod);
        d >>= 1;
    }
    return res;
}

bool is_prime(u64 n) {
    if (n < 2) return false;

    static int small[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (int p : small) {
        if (n % p == 0) return n == (u64)p;
    }

    u64 d = n - 1;
    int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    static u64 bases[] = {
        2ULL, 325ULL, 9375ULL, 28178ULL,
        450775ULL, 9780504ULL, 1795265022ULL
    };

    for (u64 a : bases) {
        if (a % n == 0) continue;

        u64 x = pow_mod(a % n, d, n);
        if (x == 1 || x == n - 1) continue;

        bool witness = true;
        for (int r = 1; r < s; ++r) {
            x = mul_mod(x, x, n);
            if (x == n - 1) {
                witness = false;
                break;
            }
        }

        if (witness) return false;
    }

    return true;
}

mt19937_64 rng64(chrono::steady_clock::now().time_since_epoch().count());

u64 pollard(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    while (true) {
        u64 y = rng64() % (n - 1) + 1;
        u64 c = rng64() % (n - 1) + 1;

        const u64 block = 128;
        u64 g = 1, r = 1, q = 1;
        u64 x = 0, ys = 0;

        auto f = [&](u64 v) -> u64 {
            u64 z = mul_mod(v, v, n);
            z += c;
            if (z >= n) z -= n;
            return z;
        };

        while (g == 1) {
            x = y;
            for (u64 i = 0; i < r; ++i) y = f(y);

            u64 k = 0;
            while (k < r && g == 1) {
                ys = y;
                u64 lim = min(block, r - k);

                for (u64 i = 0; i < lim; ++i) {
                    y = f(y);
                    u64 diff = (x > y ? x - y : y - x);
                    q = mul_mod(q, diff, n);
                }

                g = std::gcd(q, n);
                k += lim;
            }

            r <<= 1;
        }

        if (g == n) {
            g = 1;
            do {
                ys = f(ys);
                u64 diff = (x > ys ? x - ys : ys - x);
                g = std::gcd(diff, n);
            } while (g == 1);
        }

        if (g != n && g != 1) return g;
    }
}

void factor_rec(u64 n, vector<u64>& fac) {
    if (n == 1) return;

    if (is_prime(n)) {
        fac.push_back(n);
        return;
    }

    u64 d = pollard(n);
    factor_rec(d, fac);
    factor_rec(n / d, fac);
}

struct Factor {
    u64 p;
    int a;
};

unordered_map<u64, vector<Factor>> cache_fac;

vector<Factor> get_factors(u64 n) {
    if (n == 1) return {};

    auto it = cache_fac.find(n);
    if (it != cache_fac.end()) return it->second;

    vector<u64> fac;

    static int small[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    u64 tmp = n;

    for (int p : small) {
        while (tmp % p == 0) {
            fac.push_back((u64)p);
            tmp /= p;
        }
    }

    if (tmp > 1) factor_rec(tmp, fac);

    sort(fac.begin(), fac.end());

    vector<Factor> res;
    for (size_t i = 0; i < fac.size();) {
        u64 p = fac[i];
        int cnt = 0;

        while (i < fac.size() && fac[i] == p) {
            ++cnt;
            ++i;
        }

        res.push_back({p, cnt});
    }

    cache_fac[n] = res;
    return res;
}

class Solver {
    static constexpr int MAXF = 20;
    static constexpr int MAXE = 70;

    int K;
    u64 p[MAXF];
    int expo[MAXF];

    u64 eModArr[MAXF][MAXE];
    u64 qArr[MAXF][MAXE];
    u64 phiArr[MAXF][MAXE];

    u64 m, ans;

    void build(int i) {
        u64 P = p[i];
        int a = expo[i];
        u64 pmod = P % MOD;

        eModArr[i][0] = 1;
        for (int b = 1; b <= a; ++b) {
            eModArr[i][b] = eModArr[i][b - 1] * pmod % MOD;
        }

        qArr[i][0] = 1;
        for (int b = 1; b <= a; ++b) {
            qArr[i][b] = qArr[i][b - 1];
            if (b & 1) qArr[i][b] *= P;
        }

        u64 phiC[MAXE];
        phiC[0] = 1;

        u64 pPow = 1;
        u64 pm1 = (pmod + MOD - 1) % MOD;

        for (int c = 1; c <= a; ++c) {
            phiC[c] = pPow * pm1 % MOD;
            pPow = pPow * pmod % MOD;
        }

        for (int b = 0; b <= a; ++b) {
            phiArr[i][b] = phiC[a - b];
        }
    }

    void dfs(int idx, u64 eMod, u64 phiMod, u64 qVal) {
        if (idx == K) {
            u64 cnt = m / qVal + 1;
            u64 term = eMod * phiMod % MOD;
            term = term * (cnt % MOD) % MOD;

            ans += term;
            if (ans >= MOD) ans -= MOD;
            return;
        }

        for (int b = 0; b <= expo[idx]; ++b) {
            dfs(
                idx + 1,
                eMod * eModArr[idx][b] % MOD,
                phiMod * phiArr[idx][b] % MOD,
                qVal * qArr[idx][b]
            );
        }
    }

public:
    u64 calculate(const vector<Factor>& fs, u64 mm) {
        K = (int)fs.size();
        m = mm;
        ans = 0;

        for (int i = 0; i < K; ++i) {
            p[i] = fs[i].p;
            expo[i] = fs[i].a;
            build(i);
        }

        dfs(0, 1, 1, 1);
        return ans;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    Solver solver;

    while (T--) {
        u64 n, m;
        cin >> n >> m;

        auto fs = get_factors(n);
        cout << solver.calculate(fs, m) << '\n';
    }

    return 0;
}
