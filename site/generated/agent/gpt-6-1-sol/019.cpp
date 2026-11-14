#include <bits/stdc++.h>
using namespace std;
using u32 = uint32_t;
using u64 = uint64_t;

static u32 mod;

static u32 power(u32 a, u32 e) {
    u32 r = 1;
    while (e) {
        if (e & 1) r = (u64)r * a % mod;
        a = (u64)a * a % mod;
        e >>= 1;
    }
    return r;
}

// At most sixteen products below 10^18 can be added before reducing.
static vector<u32> convolution(const vector<u32>& a,
                               const vector<u32>& b, int limit) {
    int size = min(limit, (int)a.size() + (int)b.size() - 1);
    vector<u32> c(size);
    for (int i = 0; i < size; ++i) {
        int lo = max(0, i - (int)b.size() + 1);
        int hi = min(i, (int)a.size() - 1);
        u64 sum = 0;
        while (lo <= hi) {
            int end = min(hi + 1, lo + 16);
            for (; lo < end; ++lo) sum += (u64)a[lo] * b[i - lo];
            sum %= mod;
        }
        c[i] = sum;
    }
    return c;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n >> mod)) return 0;
    const int stride = n + 1;
    const int cells = stride * stride;
    vector<u32> choose(cells), stirling(cells), euler(cells);
    choose[0] = stirling[0] = euler[0] = 1;
    for (int d = 1; d <= n; ++d) {
        int row = d * stride, prev = row - stride;
        choose[row] = choose[row + d] = 1;
        for (int k = 1; k < d; ++k) {
            u32 x = choose[prev + k - 1] + choose[prev + k];
            if (x >= mod) x -= mod;
            choose[row + k] = x;
        }
        for (int q = 1; q <= d; ++q) {
            u32 sub = (u64)(d - 1) * stirling[prev + q] % mod;
            u32 x = stirling[prev + q - 1] + mod - sub;
            if (x >= mod) x -= mod;
            stirling[row + q] = x;
        }
        for (int k = 0; k < d; ++k) {
            u64 x = (u64)(k + 1) * euler[prev + k];
            if (k) x += (u64)(d - k) * euler[prev + k - 1];
            euler[row + k] = x % mod;
        }
    }
    vector<u32> invfact(n + 1), weight(n + 1);
    u32 fact = 1;
    for (int i = 1; i <= n; ++i) fact = (u64)fact * i % mod;
    invfact[n] = power(fact, mod - 2);
    for (int i = n; i; --i) invfact[i - 1] = (u64)invfact[i] * i % mod;
    for (int d = 0; d <= n; ++d)
        weight[d] = (u64)choose[n * stride + d] * invfact[d] % mod;

    /* Write E_q(x) for the ordinary Eulerian polynomial and c(d,q)
       for signed Stirling numbers of the first kind. The joint polynomial is

       (1-y)^n + sum_{q=1}^n F_q(x) G_q(y),
       F_q(x) = E_q(x) sum_{d=q}^n C(n,d)c(d,q)(1-x)^(d-q)/d!,
       G_q(y) = y E_q(y)(1-y)^(n-q).

       Only coefficients of degrees below n are needed. Store F transposed
       so that the final coefficient table is an ordinary matrix product.
    */
    vector<u32> f(n * n), g(n * n);
    for (int q = 1; q <= n; ++q) {
        int degree = n - q;
        vector<u32> raw(degree + 1), b(degree + 1), p(degree + 1);
        for (int r = 0; r <= degree; ++r)
            raw[r] = (u64)weight[q + r] * stirling[(q + r) * stride + q] % mod;
        // Expand the polynomial in powers of (1-x) into powers of x.
        for (int j = 0; j <= degree; ++j) {
            u64 sum = 0;
            int r = j;
            while (r <= degree) {
                int end = min(degree + 1, r + 16);
                for (; r < end; ++r)
                    sum += (u64)raw[r] * choose[r * stride + j];
                sum %= mod;
            }
            b[j] = (j & 1) && sum ? mod - sum : sum;
            u32 value = choose[degree * stride + j];
            p[j] = (j & 1) && value ? mod - value : value;
        }
        vector<u32> eq(q);
        copy_n(euler.begin() + q * stride, q, eq.begin());
        vector<u32> fq = convolution(eq, b, n);
        vector<u32> gq = convolution(eq, p, n - 1);
        for (int i = 0; i < (int)fq.size(); ++i) f[i * n + q - 1] = fq[i];
        for (int j = 0; j < (int)gq.size(); ++j) g[(q - 1) * n + j + 1] = gq[j];
    }

    vector<u64> sums(n);
    string output;
    output.reserve((size_t)n * n * 11);
    for (int i = 0; i < n; ++i) {
        fill(sums.begin(), sums.end(), 0);
        const u32* fi = f.data() + i * n;
        for (int block = 0; block < n; block += 16) {
            int end = min(n, block + 16);
            for (int q = block; q < end; ++q) {
                u64 value = fi[q];
                const u32* grow = g.data() + q * n;
                for (int j = 1; j < n; ++j) sums[j] += value * grow[j];
            }
            for (int j = 1; j < n; ++j) sums[j] %= mod;
        }
        for (int j = 0; j < n; ++j) {
            u32 value = sums[j];
            if (i == 0) {
                u32 term = choose[n * stride + j];
                if ((j & 1) && term) term = mod - term;
                value += term;
                if (value >= mod) value -= mod;
            }
            output += to_string(value);
            output += j + 1 == n ? '\n' : ' ';
        }
    }
    cout << output;
}
