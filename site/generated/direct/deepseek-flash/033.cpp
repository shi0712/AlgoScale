#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int modpow(long long a, long long e) {
    long long r = 1;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> a(m), b(m);
    for (int i = 0; i < m; ++i) cin >> a[i];
    for (int i = 0; i < m; ++i) cin >> b[i];

    vector<int> fact(n + 1), invfact(n + 1), inv(n + 1);

    fact[0] = 1;
    for (int i = 1; i <= n; ++i) {
        fact[i] = 1LL * fact[i - 1] * i % MOD;
    }

    invfact[n] = modpow(fact[n], MOD - 2);
    for (int i = n; i >= 1; --i) {
        invfact[i - 1] = 1LL * invfact[i] * i % MOD;
    }

    inv[1] = 1;
    for (int i = 2; i <= n; ++i) {
        inv[i] = MOD - 1LL * (MOD / i) * inv[MOD % i] % MOD;
    }

    auto C = [&](int N, int K) -> int {
        if (K < 0 || K > N) return 0;
        return 1LL * fact[N] * invfact[K] % MOD * invfact[N - K] % MOD;
    };

    vector<int> poly(1, 1);

    for (int i = 0; i < m; ++i) {
        if (b[i] == 0) continue;

        int ai = a[i], bi = b[i];
        vector<int> f(ai + 1, 0);

        for (int j = bi; j <= ai; ++j) {
            int cb = C(ai, j);
            int shift = ai - j;

            for (int l = 0; l <= j; ++l) {
                int term = 1LL * cb * C(j, l) % MOD;
                if (l & 1) {
                    term = (term == 0 ? 0 : MOD - term);
                }

                int idx = shift + l;
                f[idx] += term;
                if (f[idx] >= MOD) f[idx] -= MOD;
            }
        }

        vector<int> np(poly.size() + f.size() - 1, 0);

        for (int u = 0; u < (int)poly.size(); ++u) {
            if (poly[u] == 0) continue;
            for (int v = 0; v < (int)f.size(); ++v) {
                if (f[v] == 0) continue;
                np[u + v] = (np[u + v] + 1LL * poly[u] * f[v]) % MOD;
            }
        }

        poly.swap(np);
    }

    int expectedMax = 0;

    for (int k = 1; k < (int)poly.size(); ++k) {
        int sub = 1LL * poly[k] * inv[k] % MOD;
        expectedMax -= sub;
        if (expectedMax < 0) expectedMax += MOD;
    }

    vector<int> harmonic(n + 1, 0);
    for (int i = 1; i <= n; ++i) {
        harmonic[i] = harmonic[i - 1] + inv[i];
        if (harmonic[i] >= MOD) harmonic[i] -= MOD;
    }

    long long answer = 0;
    int totalTarget = 0;

    for (int i = 0; i < m; ++i) {
        totalTarget += b[i];

        int expectedTi = 0;
        if (b[i] > 0) {
            expectedTi = harmonic[a[i]] - harmonic[a[i] - b[i]];
            if (expectedTi < 0) expectedTi += MOD;
        }

        int diff = expectedMax - expectedTi;
        if (diff < 0) diff += MOD;

        answer += 1LL * (a[i] - b[i]) * diff % MOD;
        answer %= MOD;
    }

    answer += totalTarget;
    answer %= MOD;

    cout << answer << '\n';
    return 0;
}
