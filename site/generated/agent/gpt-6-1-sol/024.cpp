#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int mod_pow(int a, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r = (long long)r * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return r;
}

int add_mod(int a, int b) {
    int s = a + b;
    return s >= MOD ? s - MOD : s;
}

int sub_mod(int a, int b) {
    int s = a - b;
    return s < 0 ? s + MOD : s;
}

void ntt(vector<int>& a, bool inverse = false) {
    int n = (int)a.size();
    static vector<int> roots{0, 1}, rev;
    if ((int)roots.size() < n) {
        int k = __builtin_ctz((unsigned)roots.size());
        roots.resize(n);
        while ((1 << k) < n) {
            int z = mod_pow(3, (MOD - 1) >> (k + 1));
            for (int i = 1 << (k - 1); i < (1 << k); ++i) {
                roots[2 * i] = roots[i];
                roots[2 * i + 1] = (long long)roots[i] * z % MOD;
            }
            ++k;
        }
    }
    if ((int)rev.size() != n) {
        rev.resize(n);
        int lg = __builtin_ctz((unsigned)n);
        for (int i = 0; i < n; ++i)
            rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (lg - 1));
    }
    if (inverse) reverse(a.begin() + 1, a.end());
    for (int i = 0; i < n; ++i)
        if (i < rev[i]) swap(a[i], a[rev[i]]);
    for (int k = 1; k < n; k <<= 1) {
        for (int i = 0; i < n; i += 2 * k) {
            for (int j = 0; j < k; ++j) {
                int u = a[i + j];
                int v = (long long)a[i + j + k] * roots[k + j] % MOD;
                a[i + j] = add_mod(u, v);
                a[i + j + k] = sub_mod(u, v);
            }
        }
    }
    if (inverse) {
        int inv_n = MOD - (MOD - 1) / n;
        for (int& x : a) x = (long long)x * inv_n % MOD;
    }
}

// Returns exp(integral(w[1] + w[2] x + ...)) modulo x^N.
// N is a power of two, and inv[i] = 1/i.
vector<int> exponential(const vector<int>& w, const vector<int>& inv) {
    int N = (int)w.size();
    vector<int> g{1}, h{1};
    for (int k = 1; k < N; k <<= 1) {
        int len = 2 * k;
        vector<int> G(len), H(len), R(len), T(len);
        copy(g.begin(), g.end(), G.begin());
        copy(h.begin(), h.end(), H.begin());
        for (int i = 0; i + 1 < len; ++i) T[i] = w[i + 1];
        ntt(G);
        ntt(H);
        ntt(T);
        for (int i = 0; i < len; ++i)
            R[i] = (long long)G[i] * T[i] % MOD;
        ntt(R, true);

        // g agrees with exp(f) below degree k. Thus f'g-g' starts
        // at degree k-1. Cyclic convolution gives its next k terms
        // exactly: all wrapped terms have degree at most k-3.
        for (int i = 0; i < k; ++i) R[i] = R[k - 1 + i];
        fill(R.begin() + k, R.end(), 0);
        ntt(R);
        for (int i = 0; i < len; ++i)
            R[i] = (long long)R[i] * H[i] % MOD;
        ntt(R, true);

        // e = f-log(g) has no terms below degree k.
        vector<int> e(k);
        for (int i = 0; i < k; ++i)
            e[i] = R[i] = (long long)R[i] * inv[k + i] % MOD;
        fill(R.begin() + k, R.end(), 0);
        ntt(R);
        for (int i = 0; i < len; ++i)
            T[i] = (long long)G[i] * R[i] % MOD;
        ntt(T, true);
        g.resize(len);
        for (int i = 0; i < k; ++i) g[k + i] = T[i];

        if (len == N) break;

        // Update h = 1/g to the same precision for the next step.
        for (int i = 0; i < len; ++i)
            T[i] = (long long)G[i] * H[i] % MOD;
        ntt(T, true);
        for (int i = 0; i < k; ++i)
            T[i] = add_mod(T[k + i], e[i]);
        fill(T.begin() + k, T.end(), 0);
        ntt(T);
        for (int i = 0; i < len; ++i)
            T[i] = (long long)T[i] * H[i] % MOD;
        ntt(T, true);
        h.resize(len);
        for (int i = 0; i < k; ++i)
            h[k + i] = T[i] ? MOD - T[i] : 0;
    }
    return g;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<int> freq(n + 2);
    int m = 0, present = 0;
    for (int i = 0; i < n; ++i) {
        int a;
        cin >> a;
        m += a;
        if (a) {
            ++present;
            ++freq[a + 1];
        }
    }

    int half = m / 2, N = 1;
    while (N <= half) N <<= 1;
    vector<int> inv(N), w(N, present);
    if (N > 1) inv[1] = 1;
    for (int i = 2; i < N; ++i)
        inv[i] = MOD - (long long)(MOD / i) * inv[MOD % i] % MOD;

    // P(x) = product (1-x^(a_i+1))/(1-x), over present values.
    // [x^(j-1)] P'(x)/P(x) = present - sum_{d|j} d*freq[d].
    for (int d = 2; d < N && d < (int)freq.size(); ++d) {
        if (!freq[d]) continue;
        int delta = (long long)d * freq[d] % MOD;
        for (int j = d; j < N; j += d) w[j] = sub_mod(w[j], delta);
    }
    vector<int> c = exponential(w, inv);

    // Valid words have no strictly decreasing subsequence of length 3.
    // RSK therefore has shape (m-k,k). The number of standard tableaux
    // is C(m,k)-C(m,k-1); the number with the prescribed content is
    // c[k]-c[k-1], by the two-row Jacobi-Trudi identity.
    int answer = 0, choose = 1, previous_choose = 0, previous_c = 0;
    for (int k = 0; k <= half; ++k) {
        answer = (answer + (long long)sub_mod(choose, previous_choose)
                  * sub_mod(c[k], previous_c)) % MOD;
        previous_choose = choose;
        previous_c = c[k];
        if (k < half)
            choose = (long long)choose * (m - k) % MOD * inv[k + 1] % MOD;
    }
    cout << answer << '\n';
}
