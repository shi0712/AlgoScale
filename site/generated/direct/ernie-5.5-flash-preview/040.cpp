#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using ull = unsigned long long;
using u128 = __uint128_t;

const ll MOD = 998244353;

ll mulmod(ll a, ll b, ll mod) {
    return (ll)((u128)a * b % mod);
}

ll powmod(ll a, ll e, ll mod) {
    a %= mod;
    ll r = 1 % mod;
    while (e) {
        if (e & 1) r = mulmod(r, a, mod);
        a = mulmod(a, a, mod);
        e >>= 1;
    }
    return r;
}

bool isPrime(ll n) {
    if (n < 2) return false;
    static const ll small[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (ll p : small) {
        if (n % p == 0) return n == p;
    }

    ll d = n - 1;
    int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    static const ll bases[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
    for (ll a : bases) {
        if (a % n == 0) continue;
        ll x = powmod(a, d, n);
        if (x == 1 || x == n - 1) continue;

        bool composite = true;
        for (int r = 1; r < s; ++r) {
            x = mulmod(x, x, n);
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

ll pollard(ll n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    static mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());

    while (true) {
        ll c = rng() % (n - 1) + 1;
        ll x = rng() % (n - 2) + 2;
        ll y = x;
        ll d = 1;

        auto f = [&](ll v) -> ll {
            return (mulmod(v, v, n) + c) % n;
        };

        while (d == 1) {
            x = f(x);
            y = f(f(y));
            ll diff = x > y ? x - y : y - x;
            d = std::gcd(diff, n);
            if (d == n) break;
        }

        if (d != n && d > 1) return d;
    }
}

void factorBig(ll n, vector<ll>& fac) {
    if (n == 1) return;
    if (isPrime(n)) {
        fac.push_back(n);
        return;
    }
    ll d = pollard(n);
    factorBig(d, fac);
    factorBig(n / d, fac);
}

struct Info {
    ll p;
    int e;
    int L;
    vector<ll> c;
};

vector<int> smallPrimes;

void initSmallPrimes() {
    const int N = 1000;
    vector<bool> is(N + 1, true);
    is[0] = is[1] = false;
    for (int i = 2; i * i <= N; ++i) {
        if (is[i]) {
            for (int j = i * i; j <= N; j += i) is[j] = false;
        }
    }
    for (int i = 2; i <= N; ++i) {
        if (is[i]) smallPrimes.push_back(i);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    initSmallPrimes();

    int T;
    cin >> T;

    while (T--) {
        ll n, m;
        cin >> n >> m;

        vector<ll> fac;
        ll x = n;

        for (int p : smallPrimes) {
            if ((ll)p * p > x) break;
            while (x % p == 0) {
                fac.push_back(p);
                x /= p;
            }
        }

        if (x > 1) factorBig(x, fac);
        sort(fac.begin(), fac.end());

        vector<Info> infos;
        ll F0 = 1;

        for (int i = 0; i < (int)fac.size();) {
            ll p = fac[i];
            int e = 0;
            while (i < (int)fac.size() && fac[i] == p) {
                ++e;
                ++i;
            }

            int L = (e + 1) / 2;

            ll pm1 = (p - 1) % MOD;
            ll pe = powmod(p, e - 1, MOD);

            ll B = pe * pm1 % MOD;
            ll Z = pe * ((e * pm1 + p) % MOD) % MOD;

            vector<ll> c(L + 1);
            c[0] = B;
            for (int a = 1; a < L; ++a) {
                c[a] = (2 * B) % MOD;
            }

            ll cL = (Z - ((2LL * L - 1) % MOD) * B) % MOD;
            if (cL < 0) cL += MOD;
            c[L] = cL;

            infos.push_back({p, e, L, std::move(c)});
            F0 = F0 * Z % MOD;
        }

        sort(infos.begin(), infos.end(), [](const Info& a, const Info& b) {
            return a.p > b.p;
        });

        ll sum = 0;
        int sz = (int)infos.size();

        auto dfs = [&](auto&& self, int idx, ll d, ll coeff) -> void {
            if (d > m) return;

            if (idx == sz) {
                sum = (sum + coeff * ((m / d) % MOD)) % MOD;
                return;
            }

            const Info& info = infos[idx];
            ll pw = 1;

            for (int a = 0; a <= info.L; ++a) {
                if (a > 0) {
                    if (pw > m / d) break;
                }

                ll nd = d * pw;
                ll nc = coeff * info.c[a] % MOD;
                self(self, idx + 1, nd, nc);

                if (a < info.L) pw *= info.p;
            }
        };

        dfs(dfs, 0, 1, 1);

        ll ans = (F0 + sum) % MOD;
        cout << ans << '\n';
    }

    return 0;
}
