#include <bits/stdc++.h>
using namespace std;
using ll = long long;

static const int MOD = 998244353;

long long modpow(long long a, long long e) {
    long long r = 1;
    a %= MOD;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

/*
 Short derivation
 ----------------
 For a state that is "sorted prefix + sorted suffix", let A be the set of values
 in the prefix, |A| = k, and a_1 < ... < a_k its sorted elements.
 Define O_j = a_j - j (number of values below a_j that are not in A).
 The expected remaining time E satisfies, with q = n/(n-1) and x = q-1 = 1/(n-1),

     E = q + (q-1) * (sum over the non-terminal proper transitions).

 This makes E a universal polynomial in q, so g := E/q = E/(1+x) is a universal
 polynomial in x with E = q*g holding for every n.  In terms of the partition
 formed by the positive O_j it turns out that

     g = 1 + x * ( sum_{t>=1} g(drop t longest rows) + sum_{c>=1} g(remove c columns) ).

 Expanding that recurrence is the same as counting monotone (right/down) lattice
 paths inside the Young diagram of the partition, with weight x per move.
 Decomposing a path by its endpoint (i,j) gives

     g(partition) = sum over cells (i,j) of A(i,j),

 where A(i,j) is the total x-weight of paths from (0,0) to (i,j):
     A(0,0) = 1,
     A(i,j) = x * ( sum_{j'<j} A(i,j') + sum_{i'<i} A(i',j) ).
 Storing row prefix sums of A makes summing over a whole diagram O(#rows).

 Finally the answer is  1 + x * sum_{k=1}^{n-1} g(state after k values),
 evaluated at x = 1/(n-1) modulo MOD.
*/

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    while (T--) {
        int n;
        if (scanf("%d", &n) != 1) return 0;
        vector<int> p(n);
        for (int i = 0; i < n; i++) scanf("%d", &p[i]);

        bool sorted = true;
        for (int i = 0; i < n; i++)
            if (p[i] != i + 1) { sorted = false; break; }
        if (sorted) { puts("0"); continue; }

        // x = 1/(n-1).  (q = n/(n-1) = 1 + x.)
        int x = (int)modpow(n - 1, MOD - 2);

        // pref[i*n + j] = sum_{j'=0..j} A(i,j)
        vector<int> pref((size_t)n * n);
        vector<int> colsum(n, 0); // colsum[j] = sum_{i'<i} A(i',j)
        for (int i = 0; i < n; i++) {
            ll rowsum = 0;
            size_t base = (size_t)i * n;
            for (int j = 0; j < n; j++) {
                int a;
                if (i == 0 && j == 0) {
                    a = 1;
                } else {
                    ll s = 0;
                    if (j > 0) s += rowsum;   // sum_{j'<j} A(i,j')
                    if (i > 0) s += colsum[j]; // sum_{i'<i} A(i',j)
                    s %= MOD;
                    a = (int)((ll)x * s % MOD);
                }
                rowsum += a;
                if (rowsum >= MOD) rowsum -= MOD;
                pref[base + j] = (int)rowsum;
                int c = colsum[j] + a;
                if (c >= MOD) c -= MOD;
                colsum[j] = c;
            }
        }

        vector<int> seen; seen.reserve(n);
        vector<int> parts; parts.reserve(n);
        ll total = 0; // sum of g over prefixes of size 1..n-1

        for (int k = 1; k <= n; k++) {
            int v = p[k - 1];
            seen.insert(lower_bound(seen.begin(), seen.end(), v), v);
            if (k == n) break;

            // positive O_j, already nondecreasing in j
            parts.clear();
            for (int j = 1; j <= k; j++) {
                int O = seen[j - 1] - j;
                if (O > 0) parts.push_back(O);
            }
            // partition rows sorted descending = parts reversed
            int g = 0, row = 0;
            for (int idx = (int)parts.size() - 1; idx >= 0; idx--, row++) {
                int len = parts[idx];
                g += pref[(size_t)row * n + (len - 1)];
                if (g >= MOD) g -= MOD;
            }
            total += g;
            if (total >= MOD) total -= MOD;
        }

        ll ans = (1 + (ll)x * total) % MOD;
        printf("%lld\n", ans);
    }
    return 0;
}
