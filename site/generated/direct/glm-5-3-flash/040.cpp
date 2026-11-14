#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
typedef unsigned __int128 u128;

static const u64 MOD = 998244353;

static inline u64 mulmod(u64 a, u64 b, u64 m){ return (u64)((u128)a * b % m); }

static inline u64 powmod(u64 a, u64 e, u64 m){
    u64 r = 1 % m; a %= m;
    while(e){
        if(e & 1) r = mulmod(r, a, m);
        a = mulmod(a, a, m);
        e >>= 1;
    }
    return r;
}

static bool isPrime(u64 n){
    if(n < 2) return false;
    for(u64 p : {2ull,3ull,5ull,7ull,11ull,13ull,17ull,19ull,23ull,29ull,31ull,37ull})
        if(n % p == 0) return n == p;
    u64 d = n - 1; int s = 0;
    while(!(d & 1)){ d >>= 1; ++s; }
    for(u64 a : {2ull,3ull,5ull,7ull,11ull,13ull,17ull}){
        u64 x = powmod(a, d, n);
        if(x == 1 || x == n - 1) continue;
        bool comp = true;
        for(int i = 1; i < s; ++i){
            x = mulmod(x, x, n);
            if(x == n - 1){ comp = false; break; }
        }
        if(comp) return false;
    }
    return true;
}

static u64 pollard(u64 n){
    if((n & 1) == 0) return 2;
    static mt19937_64 rng(88172645463325252ULL);
    while(true){
        u64 c = rng() % (n - 1) + 1;
        auto f = [&](u64 v){ return (mulmod(v, v, n) + c) % n; };
        u64 x = rng() % n, y = x, d = 1;
        do{
            x = f(x);
            y = f(f(y));
            u64 diff = x > y ? x - y : y - x;
            d = __gcd(diff, n);
        } while(d == 1);
        if(d != n) return d;
    }
}

int main(){
    const int LIM = 10000;
    vector<int> primes;
    {
        vector<bool> comp(LIM + 1, false);
        for(int i = 2; i <= LIM; ++i){
            if(!comp[i]){
                primes.push_back(i);
                for(int j = 2 * i; j <= LIM; j += i) comp[j] = true;
            }
        }
    }

    int T;
    if(scanf("%d", &T) != 1) return 0;
    while(T--){
        u64 n, m;
        scanf("%llu %llu", &n, &m);

        vector<pair<u64,int>> fac;
        u64 r = n;
        for(int pr : primes){
            u64 p = (u64)pr;
            if(p * p > r) break;
            if(r % p == 0){
                int e = 0;
                while(r % p == 0){ r /= p; ++e; }
                fac.push_back({p, e});
            }
        }
        if(r > 1){
            if(isPrime(r)){
                fac.push_back({r, 1});
            } else {
                u64 s = (u64)sqrtl((long double)r);
                while(s > 0 && s * s > r) --s;
                while((s + 1) * (s + 1) <= r) ++s;
                if(s * s == r){
                    fac.push_back({s, 2});
                } else {
                    u64 p = pollard(r);
                    u64 q = r / p;
                    if(p > q) swap(p, q);
                    fac.push_back({p, 1});
                    fac.push_back({q, 1});
                }
            }
        }

        // divs: (divisor d of n with nonzero Mobius-transform, that transform mod MOD)
        vector<pair<u64,u64>> divs;
        divs.push_back({1ULL, 1ULL});
        u64 Hn = 1; // H(n) = prod C_p, value at k = 0
        for(auto &fe : fac){
            u64 p = fe.first;
            int e = fe.second;
            u64 pe = 1, pem = 1;
            for(int i = 0; i < e; ++i) pe *= p;
            for(int i = 0; i + 1 < e; ++i) pem *= p;
            u64 A = (pem % MOD) * ((p - 1) % MOD) % MOD;               // G_p(0)
            u64 C = ((u64)(e % MOD) * ((pe - pem) % MOD) + pe % MOD) % MOD; // G_p(e)
            Hn = Hn * C % MOD;

            int amax = (e + 1) / 2;
            size_t sz = divs.size();
            for(int a = 1; a <= amax; ++a){
                u64 w;
                if(2 * a < e) w = 2 * A % MOD;
                else if(2 * a == e) w = (2 * pe % MOD + MOD - pem % MOD) % MOD;
                else w = pe % MOD; // 2a == e+1
                u64 pa = 1;
                for(int i = 0; i < a; ++i) pa *= p;
                for(size_t i = 0; i < sz; ++i)
                    divs.push_back({divs[i].first * pa, divs[i].second * w % MOD});
            }
            for(size_t i = 0; i < sz; ++i)
                divs[i].second = divs[i].second * A % MOD; // a = 0 contributes G_p(0)
        }

        u64 ans = Hn;
        for(auto &dw : divs){
            u64 d = dw.first, w = dw.second;
            u64 cnt = m / d;
            if(cnt) ans = (ans + (u64)((u128)w * cnt % MOD)) % MOD;
        }
        printf("%llu\n", ans);
    }
    return 0;
}
