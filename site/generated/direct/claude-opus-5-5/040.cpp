#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
typedef long long ll;
const ull MOD = 998244353ULL;

int main() {
    const int LIM = 1000001;
    vector<bool> comp(LIM + 1, false);
    vector<int> primes;
    for (int i = 2; i <= LIM; i++) {
        if (!comp[i]) {
            primes.push_back(i);
            for (ll j = (ll)i * i; j <= LIM; j += i) comp[j] = true;
        }
    }
    int T;
    scanf("%d", &T);
    while (T--) {
        ll n, m;
        scanf("%lld %lld", &n, &m);
        vector<pair<ll,int>> fac;
        ll r = n;
        for (int p : primes) {
            if ((ll)p * p > r) break;
            if (r % p == 0) {
                int e = 0;
                while (r % p == 0) { r /= p; e++; }
                fac.push_back({p, e});
            }
        }
        if (r > 1) fac.push_back({r, 1});

        vector<pair<ll, ull>> divs; // (divisor, G value)
        divs.push_back({1, 1});
        for (auto &pe : fac) {
            ll p = pe.first;
            int e = pe.second;
            int t = (e + 1) / 2;
            ll pe_full = 1;
            for (int i = 0; i < e; i++) pe_full *= p;
            ull pem = (ull)(pe_full % MOD);
            ull phi = (ull)((pe_full - pe_full / p) % MOD);
            vector<ull> F(t + 1);
            for (int j = 0; j <= t; j++) {
                int v = min(2 * j, e);
                if (v < e) F[j] = (ull)(v + 1) % MOD * phi % MOD;
                else F[j] = ((ull)e % MOD * phi + pem) % MOD;
            }
            vector<ull> G(t + 1);
            G[0] = 1;
            for (int j = 1; j <= t; j++) G[j] = (F[j] + MOD - F[j - 1]) % MOD;
            // F(p^0) = h with v=0 = phi; but total F(gcd) = prod F; G(1) must be F(1)
            // handle: G(p^0) = F[0]
            G[0] = F[0];
            vector<pair<ll, ull>> nd;
            nd.reserve(divs.size() * (t + 1));
            for (auto &d : divs) {
                ll pw = 1;
                for (int j = 0; j <= t; j++) {
                    nd.push_back({d.first * pw, d.second * G[j] % MOD});
                    if (j < t) pw *= p;
                }
            }
            divs.swap(nd);
        }
        ull ans = 0;
        for (auto &d : divs) {
            ull cnt = (ull)((m / d.first + 1) % MOD);
            ans = (ans + d.second * cnt) % MOD;
        }
        printf("%llu\n", ans);
    }
    return 0;
}
