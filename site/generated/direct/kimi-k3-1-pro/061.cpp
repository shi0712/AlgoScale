#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 998244353;

long long modpow(long long a, long long e) {
    long long r = 1;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

vector<long long> fact_, invfact_, pow2_;

long long C(int n, int k) {
    if (k < 0 || k > n) return 0;
    return fact_[n] * invfact_[k] % MOD * invfact_[n - k] % MOD;
}

long long invC(int n, int k) {
    // 1 / C(n,k) = k! (n-k)! / n!
    return fact_[k] * fact_[n - k] % MOD * invfact_[n] % MOD;
}

struct TestCase {
    int n, m;
    vector<int> a;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    vector<TestCase> tests(T);
    int maxN = 0, maxM = 0;

    for (auto &tc : tests) {
        cin >> tc.n >> tc.m;
        tc.a.resize(tc.n);
        for (int &x : tc.a) cin >> x;
        maxN = max(maxN, 2 * tc.m);
        maxM = max(maxM, tc.m);
    }

    fact_.assign(maxN + 1, 1);
    for (int i = 1; i <= maxN; ++i) {
        fact_[i] = fact_[i - 1] * i % MOD;
    }

    invfact_.assign(maxN + 1, 1);
    invfact_[maxN] = modpow(fact_[maxN], MOD - 2);
    for (int i = maxN; i >= 1; --i) {
        invfact_[i - 1] = invfact_[i] * i % MOD;
    }

    pow2_.assign(maxM + 1, 1);
    for (int i = 1; i <= maxM; ++i) {
        pow2_[i] = pow2_[i - 1] * 2 % MOD;
    }

    for (const auto &tc : tests) {
        const int n = tc.n;
        const int m = tc.m;
        const int N = 2 * m;

        if (n == 1) {
            cout << 0 << '\n';
            continue;
        }

        vector<long long> invCN(m + 1);
        for (int k = 0; k <= m; ++k) {
            invCN[k] = invC(N, k);
        }

        // c[k]: expected number of generations with k ancestral lineages.
        vector<long long> c(m + 1, 0);

        for (int k = m; k >= 2; --k) {
            // Probability of ever reaching state k.
            long long reach = (k == m ? 1 : 0);

            const int hmax = min(m, 2 * k);
            for (int h = k + 1; h <= hmax; ++h) {
                const int r = h - k;       // number of mergers
                const int s = 2 * k - h;   // blocks contributing one element

                long long ways = C(m, r) * C(m - r, s) % MOD;
                ways = ways * pow2_[s] % MOD;

                const long long prob = ways * invCN[h] % MOD;
                reach = (reach + c[h] * prob) % MOD;
            }

            const long long stay =
                C(m, k) * pow2_[k] % MOD * invCN[k] % MOD;
            const long long leave = (1 - stay + MOD) % MOD;

            c[k] = reach * modpow(leave, MOD - 2) % MOD;
        }

        // sumC[k] = sum_i C(a_i, k)
        vector<long long> sumC(m + 1, 0);
        for (int a : tc.a) {
            const int lim = min(a, m);
            for (int k = 2; k <= lim; ++k) {
                sumC[k] += C(a, k);
                if (sumC[k] >= MOD) sumC[k] -= MOD;
            }
        }

        long long answer = 1;
        for (int k = 2; k <= m; ++k) {
            const long long ratio = sumC[k] * invCN[k] % MOD;
            const long long bracket = (1 - ratio + MOD) % MOD;
            answer = (answer + c[k] * bracket) % MOD;
        }

        cout << answer << '\n';
    }

    return 0;
}
