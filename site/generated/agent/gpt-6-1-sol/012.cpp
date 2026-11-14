#include <bits/stdc++.h>
using namespace std;
using u32 = uint32_t;
using u64 = uint64_t;

/*
Left-child/right-sibling encoding turns the given tree into an increasing
binary tree whose root 1 has no right child. Removing root 1 leaves an arbitrary
increasing binary tree. Thus, for M = n-1, we count permutations that are
preorders of increasing binary trees.

Let d(p) be the minimum number of increasing binary trees whose preorders,
concatenated, equal p. If p = A, minimum, B, then

    d(p) = d(A) + max(1, d(B)-1),       d(empty) = 0.

Indeed, the minimum starts a new tree and can absorb the first two trees of
an optimal forest for B as its two subtrees. Conversely, splitting its two
subtrees off any forest gives the matching lower bound.

Let F(z,u) = sum_p z^|p| / |p|! * u^d(p), and f(z) = [u] F(z,u).
Splitting at the minimum (with binomial choices of labels) gives

    F_z = F * ((F-1)/u - f + u*(1+f)).

Put H = 1/F = sum_k h_k(z) u^k and g_k = h_k-h_{k-1}.
Then h_0=1, h_1=-f, and

    f'   = 1 + 2*f + f*f - g_2,
    h_k' = g_k + g_{k+1} + f*g_k          (k >= 2).

All arrays below store ordinary coefficients in z; therefore differentiation
requires division by the new degree s. The answer is M! * [z^M] f.

Since h_k starts at degree k, g_k starts at degree k-1. Moreover, to obtain
[z^M]f we only need [z^s]g_k with s+k <= M+1. The resulting truncated
recurrence uses O(M^3) time (about M^3/12 modular products) and O(M^2) memory.
Columns g_k are contiguous, and dot products reduce modulo P in blocks of 16.
*/

static inline u32 dot(const u32* a, const u32* b, int len, u32 mod) {
    u64 sum = 0;
    while (len >= 16) {
        // 16*(P-1)^2 + (P-1) fits uint64_t for every allowed P.
        u64 block = 0;
        for (int j = 0; j < 16; ++j) block += (u64)a[j] * b[-j];
        sum = (sum + block) % mod;
        a += 16;
        b -= 16;
        len -= 16;
    }
    for (int j = 0; j < len; ++j) sum += (u64)a[j] * b[-j];
    return sum % mod;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    u32 mod;
    cin >> n >> mod;
    if (n <= 2) { cout << 1 << '\n'; return 0; }
    int M = n - 1;
    vector<u32> f(M + 1), inv(M + 1);
    inv[1] = 1;
    for (int i = 2; i <= M; ++i)
        inv[i] = mod - (u64)(mod / i) * inv[mod % i] % mod;
    int stride = M + 1;
    // D(k,s) = [z^s] g_k(z). Only k <= (M+2)/2 can be needed.
    vector<u32> d((size_t)(M / 2 + 4) * stride);
    auto D = [&](int k, int t) -> u32& { return d[(size_t)k * stride + t]; };
    for (int s = 1; s <= M; ++s) {
        int t = s - 1;
        u32 conv = t >= 2 ? dot(f.data() + 1, f.data() + t - 1, t - 1, mod) : 0;
        u64 val = (u64)2 * f[t] + conv + (t == 0) + mod - D(2, t);
        f[s] = val % mod * inv[s] % mod;
        if (s == M) break;
        int K = min(s, M + 1 - s);
        u32 last = f[s] ? mod - f[s] : 0;
        for (int k = 2; k <= K; ++k) {
            // [z^t](f*g_k), using the vanishing degrees of both factors.
            int len = t - k + 1;
            u32 v = len > 0 ? dot(&D(k, k - 1), f.data() + len, len, mod) : 0;
            u32 h = ((u64)v + D(k, t) + D(k + 1, t)) * inv[s] % mod;
            D(k, s) = h >= last ? h - last : h + mod - last;
            last = h;
        }
        // h_{s+1} has zero coefficient at degree s.
        if (s + 1 <= M + 1 - s)
            D(s + 1, s) = last ? mod - last : 0;
    }
    u32 ans = f[M];
    for (int i = 2; i <= M; ++i) ans = (u64)ans * i % mod;
    cout << ans << '\n';
}
