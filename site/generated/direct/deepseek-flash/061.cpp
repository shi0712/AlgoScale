#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int MAXN = 4005;

long long fact[MAXN], invfact[MAXN], pow2v[MAXN];

long long modpow(long long a, long long e) {
    long long r = 1;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

long long C(int n, int k) {
    if (k < 0 || k > n) return 0;
    return fact[n] * invfact[k] % MOD * invfact[n - k] % MOD;
}

// inverse of C(n,k) modulo MOD
long long invC(int n, int k) {
    if (k < 0 || k > n) return 0;
    return fact[k] * fact[n - k] % MOD * invfact[n] % MOD;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    fact[0] = 1;
    for (int i = 1; i < MAXN; ++i) {
        fact[i] = fact[i - 1] * i % MOD;
    }

    invfact[MAXN - 1] = modpow(fact[MAXN - 1], MOD - 2);
    for (int i = MAXN - 2; i >= 0; --i) {
        invfact[i] = invfact[i + 1] * (i + 1) % MOD;
    }

    pow2v[0] = 1;
    for (int i = 1; i < MAXN; ++i) {
        pow2v[i] = pow2v[i - 1] * 2 % MOD;
    }

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        vector<int> a(n);
        for (int &x : a) cin >> x;

        int N = 2 * m;

        if (n == 1) {
            cout << 0 << '\n';
            continue;
        }

        // sumC[s] = sum_i C(a_i, s)
        vector<int> sumC(N + 1, 0);

        for (int x : a) {
            int up = min(x, N);
            for (int s = 2; s <= up; ++s) {
                sumC[s] += (int)C(x, s);
                if (sumC[s] >= MOD) sumC[s] -= MOD;
            }
        }

        // g[s] = probability that a uniformly chosen s-subset
        // does NOT lie completely inside one initial set.
        vector<int> g(N + 1, 0);

        for (int s = 2; s <= N; ++s) {
            long long same = 1LL * sumC[s] * invC(N, s) % MOD;
            g[s] = (1 - same + MOD) % MOD;
        }

        // v[s] = expected number of visits to ancestor-count state s, for s >= 2.
        vector<int> v(N + 1, 0);
        v[N] = 1;

        for (int s = N - 1; s >= 2; --s) {
            long long numerator = 0;

            if (s <= m) {
                long long chooseMS = C(m, s);
                int maxR = min(N, 2 * s);

                // Transition from r ancestors to s distinct parents.
                for (int r = s + 1; r <= maxR; ++r) {
                    long long term = chooseMS * C(s, r - s) % MOD;
                    term = term * pow2v[2 * s - r] % MOD;
                    term = term * invC(N, r) % MOD;
                    term = term * v[r] % MOD;

                    numerator += term;
                    if (numerator >= MOD) numerator -= MOD;
                }
            }

            long long self = 0;
            if (s <= m) {
                // P(s -> s): no pair contains two chosen lineages.
                self = 1LL * C(m, s) * pow2v[s] % MOD * invC(N, s) % MOD;
            }

            long long den = (1 - self + MOD) % MOD;
            v[s] = numerator * modpow(den, MOD - 2) % MOD;
        }

        long long ans = 0;

        for (int s = 2; s <= N; ++s) {
            ans += 1LL * v[s] * g[s] % MOD;
            if (ans >= MOD) ans -= MOD;
        }

        cout << ans % MOD << '\n';
    }

    return 0;
}
