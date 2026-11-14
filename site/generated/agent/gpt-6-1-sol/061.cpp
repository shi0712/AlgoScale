#include <bits/stdc++.h>
using namespace std;

constexpr int MOD = 998244353;
constexpr int MAX_N = 4000;

int mod_pow(int a, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r = (long long)r * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<int> fact(MAX_N + 1), invfact(MAX_N + 1), pow2(MAX_N + 1);
    fact[0] = pow2[0] = 1;
    for (int i = 1; i <= MAX_N; ++i) {
        fact[i] = (long long)fact[i - 1] * i % MOD;
        pow2[i] = 2LL * pow2[i - 1] % MOD;
    }
    invfact[MAX_N] = mod_pow(fact[MAX_N], MOD - 2);
    for (int i = MAX_N; i; --i)
        invfact[i - 1] = (long long)invfact[i] * i % MOD;

    int T;
    cin >> T;
    while (T--) {
        int n, m;
        cin >> n >> m;
        const int N = 2 * m;
        vector<int> sizes(n);
        for (int &a : sizes) cin >> a;
        if (n == 1) {
            cout << 0 << '\n';
            continue;
        }

        // Conditional on k distinct ancestors, their set is a uniform
        // k-subset. mono[k] is the probability they share an initial set.
        vector<int> mono(m + 1);
        for (int a : sizes) {
            for (int k = 2; k <= min(a, m); ++k) {
                int choose = (long long)fact[a] * invfact[k] % MOD
                           * invfact[a - k] % MOD;
                mono[k] += choose;
                if (mono[k] >= MOD) mono[k] -= MOD;
            }
        }
        for (int k = 2; k <= m; ++k) {
            int inverse_choose = (long long)invfact[N] * fact[k] % MOD
                                * fact[N - k] % MOD;
            mono[k] = (long long)mono[k] * inverse_choose % MOD;
        }

        // Backward ancestry starts with N vertices and becomes exactly m
        // after one round. incoming[k] excludes self-loop visits to k.
        vector<int> incoming(m + 1);
        incoming[m] = 1;
        int answer = 1;  // The initial configuration is not monochromatic.
        for (int k = m; k >= 2; --k) {
            // If j of the matching's pairs have both endpoints among the
            // k ancestors, the next ancestor count is k-j. Its probability:
            // k! (N-k)! m! 2^(k-2j) /
            // ((k-2j)! j! (m-k+j)! N!).
            int common = (long long)fact[k] * fact[N - k] % MOD
                       * fact[m] % MOD * invfact[N] % MOD;
            int self = (long long)common * pow2[k] % MOD
                     * invfact[k] % MOD * invfact[m - k] % MOD;
            int leave = (1LL + MOD - self) % MOD;
            int visits = (long long)incoming[k] * mod_pow(leave, MOD - 2) % MOD;
            answer = (answer + (long long)visits * (1LL + MOD - mono[k])) % MOD;

            int weighted_common = (long long)visits * common % MOD;
            for (int j = 1; j <= k / 2; ++j) {
                int contribution = (long long)weighted_common * pow2[k - 2 * j] % MOD
                                 * invfact[k - 2 * j] % MOD
                                 * invfact[j] % MOD * invfact[m - k + j] % MOD;
                incoming[k - j] += contribution;
                if (incoming[k - j] >= MOD) incoming[k - j] -= MOD;
            }
        }
        cout << answer << '\n';
    }
}
