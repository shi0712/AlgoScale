#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int MAXN = 4000;

long long fact[MAXN + 1], invFact[MAXN + 1];
long long pow2[MAXN + 1], invPow2[MAXN + 1];

long long modPow(long long a, long long e) {
    long long result = 1;
    while (e > 0) {
        if (e & 1) result = result * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return result;
}

void precompute() {
    fact[0] = pow2[0] = invPow2[0] = 1;
    const long long inv2 = (MOD + 1) / 2;

    for (int i = 1; i <= MAXN; ++i) {
        fact[i] = fact[i - 1] * i % MOD;
        pow2[i] = pow2[i - 1] * 2 % MOD;
        invPow2[i] = invPow2[i - 1] * inv2 % MOD;
    }

    invFact[MAXN] = modPow(fact[MAXN], MOD - 2);
    for (int i = MAXN; i >= 1; --i) {
        invFact[i - 1] = invFact[i] * i % MOD;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    precompute();

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;
        int N = 2 * m;

        vector<int> a(n);
        for (int &x : a) cin >> x;

        if (n == 1) {
            cout << 0 << '\n';
            continue;
        }

        // monoWays[k] = sum_i C(a_i, k).
        vector<long long> monoWays(m + 1, 0);
        for (int x : a) {
            for (int k = 2; k <= min(x, m); ++k) {
                long long ways = fact[x] * invFact[k] % MOD
                               * invFact[x - k] % MOD;
                monoWays[k] += ways;
                if (monoWays[k] >= MOD) monoWays[k] -= MOD;
            }
        }

        // g[k] = probability that a uniform k-element subset
        // is not monochromatic in the initial configuration.
        vector<long long> g(m + 1, 0);
        for (int k = 2; k <= m; ++k) {
            long long invChoose = invFact[N] * fact[k] % MOD
                                * fact[N - k] % MOD;
            g[k] = (1 - monoWays[k] * invChoose % MOD + MOD) % MOD;
        }

        // Transition factorization:
        // P(k -> l) = A[k] * B[l] / ((2l-k)! (k-l)!).
        vector<long long> A(m + 1), B(m + 1);
        long long common = fact[m] * invFact[N] % MOD;

        for (int k = 2; k <= m; ++k) {
            A[k] = fact[k] * fact[N - k] % MOD
                 * common % MOD * invPow2[k] % MOD;
            B[k] = pow2[2 * k] * invFact[m - k] % MOD;
        }

        // Initially there are N ancestors, and after one step exactly m.
        // Since n > 1, time 0 contributes 1 to the tail-sum expectation.
        long long answer = 1;
        vector<long long> incoming(m + 1, 0);
        incoming[m] = 1;

        for (int k = m; k >= 2; --k) {
            long long stay = A[k] * B[k] % MOD * invFact[k] % MOD;
            long long leave = (1 - stay + MOD) % MOD;

            // Expected total number of visits to state k.
            long long visits = incoming[k] * modPow(leave, MOD - 2) % MOD;

            answer = (answer + visits * g[k]) % MOD;

            long long base = visits * A[k] % MOD;
            int lowest = max(2, (k + 1) / 2);

            for (int l = lowest; l < k; ++l) {
                long long contribution = base * B[l] % MOD
                                       * invFact[2 * l - k] % MOD
                                       * invFact[k - l] % MOD;
                incoming[l] += contribution;
                if (incoming[l] >= MOD) incoming[l] -= MOD;
            }
        }

        cout << answer << '\n';
    }

    return 0;
}
