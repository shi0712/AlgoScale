#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

static inline int mul(int a, int b) {
    return int((long long)a * b % MOD);
}

static int power(int a, int e) {
    int r = 1;
    for (; e; e >>= 1, a = mul(a, a)) {
        if (e & 1) r = mul(r, a);
    }
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    constexpr int LIMIT = 6000;
    vector<int> count(LIMIT + 1);
    int maximum = 1;
    for (int i = 0, a; i < n; ++i) {
        cin >> a;
        ++count[a];
        maximum = max(maximum, a);
    }

    vector<int> spf(maximum + 1);
    for (int p = 2; p <= maximum; ++p) {
        if (spf[p]) continue;
        for (int v = p; v <= maximum; v += p) {
            if (!spf[v]) spf[v] = p;
        }
    }

    // Only small primes that actually occur need a mask bit.
    vector<int> small, index(maximum + 1, -1);
    for (int p = 2; p * p <= maximum; ++p) {
        if (spf[p] != p) continue;
        bool used = false;
        for (int v = p; v <= maximum; v += p) used |= count[v] != 0;
        if (used) {
            index[p] = (int)small.size();
            small.push_back(p);
        }
    }

    // Group 0 contains values with no large prime factor. Every other
    // group is identified by its unique prime greater than sqrt(maximum).
    vector<vector<pair<int, int>>> terms(maximum + 1);
    vector<int> dimension(maximum + 1);
    for (int v = 1; v <= maximum; ++v) {
        if (!count[v]) continue;
        int x = v, mask = 0, large = 0;
        while (x > 1) {
            int p = spf[x];
            if (index[p] >= 0) mask |= 1 << index[p];
            else large = p;
            do { x /= p; } while (x % p == 0);
        }
        terms[large].emplace_back(mask, power(v + 1, count[v]));
        dimension[large] = max(dimension[large], (int)bit_width((unsigned)mask));
    }

    vector<int> order;
    for (int p = 0; p <= maximum; ++p) {
        if (!terms[p].empty()) order.push_back(p);
    }
    sort(order.begin(), order.end(), [&](int a, int b) {
        return dimension[a] < dimension[b];
    });

    vector<int> dp(1, 1);
    int denominator = 1;
    for (int p : order) {
        int size = 1 << dimension[p];
        while ((int)dp.size() < size) {
            int old = (int)dp.size();
            dp.resize(old * 2);
            copy_n(dp.begin(), old, dp.begin() + old);
        }

        vector<int> f(size, 1);
        for (auto [mask, factor] : terms[p]) f[mask] = mul(f[mask], factor);

        // Multiplicative subset zeta transform:
        // f[M] = product of (1+v)^count[v] over masks contained in M.
        for (int half = 1; half < size; half <<= 1) {
            for (int base = 0; base < size; base += half * 2) {
                int* low = f.data() + base;
                int* high = low + half;
                for (int j = 0; j < half; ++j) high[j] = mul(high[j], low[j]);
            }
        }

        if (p == 0) {
            for (int mask = 0; mask < size; ++mask) dp[mask] = mul(dp[mask], f[mask]);
        } else {
            // Averaging over this large prime gives (1+(p-1)*f[M])/p.
            denominator = mul(denominator, p);
            for (int mask = 0; mask < size; ++mask) {
                int factor = mul(p - 1, f[mask]) + 1;
                if (factor == MOD) factor = 0;
                dp[mask] = mul(dp[mask], factor);
            }
        }
    }

    // A small prime p is absent with weight 1/p and present with
    // weight (p-1)/p. Accumulate the numerators, then divide only once.
    for (int k = (int)small.size() - 1; k >= 0; --k) {
        int half = 1 << k;
        int p = small[k];
        denominator = mul(denominator, p);
        for (int mask = 0; mask < half; ++mask) {
            int value = dp[mask] + mul(p - 1, dp[mask + half]);
            if (value >= MOD) value -= MOD;
            dp[mask] = value;
        }
    }

    cout << mul(dp[0], power(denominator, MOD - 2)) << '\n';
    return 0;
}
