#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

const int MOD = 998244353;
const int MAX_N = 4000;

int addmod(int a, int b) {
    int s = a + b;
    return s >= MOD ? s - MOD : s;
}

int submod(int a, int b) {
    int s = a - b;
    return s < 0 ? s + MOD : s;
}

int mulmod(int a, int b) {
    return int(int64(a) * b % MOD);
}

int power(int a, int e) {
    int result = 1;
    while (e > 0) {
        if (e & 1) result = mulmod(result, a);
        a = mulmod(a, a);
        e >>= 1;
    }
    return result;
}

int fact[MAX_N + 1];
int invfact[MAX_N + 1];
int dfact[MAX_N + 1];

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    fact[0] = 1;
    for (int i = 1; i <= MAX_N; ++i) fact[i] = mulmod(fact[i - 1], i);
    invfact[MAX_N] = power(fact[MAX_N], MOD - 2);
    for (int i = MAX_N; i > 0; --i) invfact[i - 1] = mulmod(invfact[i], i);

    dfact[0] = dfact[1] = 1;
    for (int i = 2; i <= MAX_N; ++i) dfact[i] = mulmod(dfact[i - 2], i);

    int T;
    cin >> T;
    while (T--) {
        int n, m;
        cin >> n >> m;
        int N = 2 * m;
        vector<int> a(n);
        for (int &x : a) cin >> x;

        int inv_total_matchings = power(dfact[N - 1], MOD - 2);
        auto pair_probability = [&](int r, int j) {
            int remaining = N - 2 * r + 2 * j;
            if (remaining < 0) return 0;

            int result = fact[r];
            result = mulmod(result, invfact[2 * j]);
            result = mulmod(result, invfact[r - 2 * j]);
            if (j > 0) result = mulmod(result, dfact[2 * j - 1]);
            result = mulmod(result, fact[N - r]);
            result = mulmod(result, invfact[remaining]);
            if (remaining > 0) result = mulmod(result, dfact[remaining - 1]);
            return mulmod(result, inv_total_matchings);
        };

        vector<int> visits(N + 1, 0);
        visits[N] = 1;
        for (int r = N - 1; r >= 2; --r) {
            int entries = 0;
            for (int previous = r + 1; previous <= min(2 * r, N); ++previous) {
                int term = mulmod(visits[previous], pair_probability(previous, previous - r));
                entries = addmod(entries, term);
            }
            int leaves = submod(1, pair_probability(r, 0));
            visits[r] = mulmod(entries, power(leaves, MOD - 2));
        }

        int inv_fact_N = invfact[N];
        int answer = 0;
        for (int r = 2; r <= N; ++r) {
            int fixed_probability_sum = 0;
            int remaining_factor = fact[N - r];
            for (int size : a) {
                if (size < r) continue;
                int ratio = fact[size];
                ratio = mulmod(ratio, invfact[size - r]);
                ratio = mulmod(ratio, remaining_factor);
                ratio = mulmod(ratio, inv_fact_N);
                fixed_probability_sum = addmod(fixed_probability_sum, ratio);
            }
            int term = mulmod(visits[r], submod(1, fixed_probability_sum));
            answer = addmod(answer, term);
        }
        cout << answer << '\n';
    }
    return 0;
}
