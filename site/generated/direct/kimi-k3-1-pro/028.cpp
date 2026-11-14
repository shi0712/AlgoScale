#include <bits/stdc++.h>
using namespace std;

static constexpr uint32_t MOD = 998244353;
static constexpr int A = 6000;
static constexpr int SMALL_LIMIT = 73;

static inline uint32_t mul(uint32_t a, uint32_t b) {
    return uint64_t(a) * b % MOD;
}

static uint32_t mod_pow(uint32_t a, int e) {
    uint32_t r = 1;
    while (e > 0) {
        if (e & 1) r = mul(r, a);
        a = mul(a, a);
        e >>= 1;
    }
    return r;
}

// After this transform:
// a[s] = product of the original a[m] over all m contained in s.
static void subset_product_zeta(vector<uint32_t>& a, int k) {
    const int n = int(a.size());
    for (int bit = 0; bit < k; ++bit) {
        const int b = 1 << bit;
        for (int start = 0; start < n; start += 2 * b) {
            for (int j = 0; j < b; ++j) {
                const int x = start + j;
                const int y = x + b;
                const uint32_t v = a[x];
                if (v == 1) continue;
                if (a[y] == 1) a[y] = v;
                else a[y] = mul(a[y], v);
            }
        }
    }
}

struct Group {
    int prime;
    int support = 0; // The function depends only on the lowest `support` bits.
    vector<pair<int, uint32_t>> terms; // (small-prime mask, weight)
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int> freq(A + 1, 0);
    for (int i = 0; i < n; ++i) {
        int a;
        cin >> a;
        ++freq[a];
    }

    // Smallest prime factors.
    vector<int> spf(A + 1, 0);
    for (int i = 2; i <= A; ++i) {
        if (spf[i]) continue;
        for (int j = i; j <= A; j += i) {
            if (!spf[j]) spf[j] = i;
        }
    }

    vector<int> small_primes;
    vector<int> small_index(A + 1, -1);
    for (int p = 2; p <= SMALL_LIMIT; ++p) {
        if (spf[p] == p) {
            small_index[p] = int(small_primes.size());
            small_primes.push_back(p);
        }
    }
    const int SMALL_COUNT = int(small_primes.size()); // 21

    vector<int> full_mask(A + 1, 0);
    vector<int> large_prime(A + 1, 0);

    for (int a = 1; a <= A; ++a) {
        int x = a;
        int mask = 0;
        while (x > 1) {
            const int p = spf[x];
            if (p > SMALL_LIMIT) {
                // x must now be prime, since otherwise x >= 79^2 > 6000.
                large_prime[a] = x;
                break;
            }
            mask |= 1 << small_index[p];
            while (x % p == 0) x /= p;
        }
        full_mask[a] = mask;
    }

    // Remove small primes that do not occur in the input at all.
    int used_mask = 0;
    for (int a = 1; a <= A; ++a) {
        if (freq[a]) used_mask |= full_mask[a];
    }

    vector<int> old_to_new(SMALL_COUNT, -1);
    vector<int> active_primes;
    for (int i = 0; i < SMALL_COUNT; ++i) {
        if (used_mask & (1 << i)) {
            old_to_new[i] = int(active_primes.size());
            active_primes.push_back(small_primes[i]);
        }
    }

    const int M = int(active_primes.size());
    const int STATES = 1 << M;

    auto compress_mask = [&](int mask) {
        int result = 0;
        while (mask) {
            const int b = __builtin_ctz(mask);
            mask &= mask - 1;
            result |= 1 << old_to_new[b];
        }
        return result;
    };

    vector<uint32_t> inv(A + 1, 0);
    inv[1] = 1;
    for (int i = 2; i <= A; ++i) {
        inv[i] = uint32_t(MOD - uint64_t(MOD / i) * inv[MOD % i] % MOD);
    }

    // Q_0 before its subset zeta transform.
    vector<uint32_t> base(STATES, 1);

    vector<Group> groups;
    vector<int> group_id(A + 1, -1);

    for (int a = 1; a <= A; ++a) {
        if (!freq[a]) continue;

        const uint32_t weight = mod_pow(uint32_t(a + 1), freq[a]);
        const int mask = compress_mask(full_mask[a]);
        const int p = large_prime[a];

        if (p == 0) {
            base[mask] = mul(base[mask], weight);
            continue;
        }

        if (group_id[p] == -1) {
            group_id[p] = int(groups.size());
            groups.push_back(Group{p});
        }

        Group& g = groups[group_id[p]];
        g.terms.push_back({mask, weight});
        if (mask) {
            g.support = max(g.support, 32 - __builtin_clz(mask));
        }
    }

    subset_product_zeta(base, M);

    vector<vector<int>> by_support(M + 1);
    for (int i = 0; i < int(groups.size()); ++i) {
        by_support[groups[i].support].push_back(i);
    }

    // Multiply one large-prime group into dst.
    auto multiply_group = [&](Group& g, vector<uint32_t>& dst) {
        const int size = 1 << g.support;
        vector<uint32_t> q(size, 1);

        for (auto [mask, weight] : g.terms) {
            q[mask] = mul(q[mask], weight);
        }

        subset_product_zeta(q, g.support);

        const uint32_t rp = mul(uint32_t(g.prime - 1), inv[g.prime]);
        for (int s = 0; s < size; ++s) {
            // 1 + r_p * (Q_p[s] - 1) = 1/p + r_p * Q_p[s].
            uint32_t factor = inv[g.prime] + mul(rp, q[s]);
            if (factor >= MOD) factor -= MOD;
            if (factor != 1) dst[s] = mul(dst[s], factor);
        }
    };

    // G contains the product of all large-prime group factors.
    // A group with support k is inserted while the array has 2^k entries.
    vector<uint32_t> G(1, 1);
    for (int id : by_support[0]) multiply_group(groups[id], G);

    for (int k = 1; k <= M; ++k) {
        const size_t half = G.size();
        vector<uint32_t> next(2 * half);
        copy(G.begin(), G.end(), next.begin());
        copy(G.begin(), G.end(), next.begin() + half);

        for (int id : by_support[k]) {
            multiply_group(groups[id], next);
        }
        G.swap(next);
    }

    // C[0] = product of 1/p over all active small primes.
    uint32_t initial_coefficient = 1;
    for (int p : active_primes) {
        initial_coefficient = mul(initial_coefficient, inv[p]);
    }

    vector<uint32_t> coefficient(STATES);
    coefficient[0] = initial_coefficient;

    uint32_t answer = 0;
    for (int s = 0; s < STATES; ++s) {
        if (s) {
            const int b = __builtin_ctz(s);
            coefficient[s] = mul(
                coefficient[s & (s - 1)],
                uint32_t(active_primes[b] - 1)
            );
        }

        const uint32_t z = mul(base[s], G[s]);
        const uint32_t term = mul(z, coefficient[s]);
        answer += term;
        if (answer >= MOD) answer -= MOD;
    }

    cout << answer << '\n';
    return 0;
}
