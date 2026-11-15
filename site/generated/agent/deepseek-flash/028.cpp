#include <bits/stdc++.h>
using namespace std;

static constexpr uint32_t MOD = 998244353;
static constexpr int MAXV = 6000;
static constexpr int B = 73;   // small primes are <= 73; primes > 73 are isolated

static inline uint32_t mulmod(uint32_t a, uint32_t b) {
    return (uint32_t)((uint64_t)a * b % MOD);
}

static uint32_t powmod(uint64_t a, long long e) {
    uint32_t r = 1;
    a %= MOD;
    while (e > 0) {
        if (e & 1) r = mulmod(r, (uint32_t)a);
        a = (uint64_t)a * a % MOD;
        e >>= 1;
    }
    return r;
}

// ---------------------------------------------------------------------------
// Fast parallel bit extraction (software pext) for at most 21-bit numbers.
// A mask m defines which bits of x are kept; PEXT7[m][v] keeps bits of the
// 7-bit value v according to the 7-bit mask m, packing them to the low bits.
// ---------------------------------------------------------------------------
static uint8_t PEXT7[128][128];

static void initPext7() {
    for (int m = 0; m < 128; ++m) {
        int pos[7], pc = 0;
        for (int i = 0; i < 7; ++i) if ((m >> i) & 1) pos[pc++] = i;
        for (int v = 0; v < 128; ++v) {
            int out = 0, b = 0;
            for (int i = 0; i < pc; ++i, ++b)
                if ((v >> pos[i]) & 1) out |= (1 << b);
            PEXT7[m][v] = (uint8_t)out;
        }
    }
}

struct PextTable {
    uint32_t t[3][128];
    PextTable() { memset(t, 0, sizeof(t)); }
    explicit PextTable(uint32_t m) {
        int shift = 0;
        for (int j = 0; j < 3; ++j) {
            int mj = (int)((m >> (7 * j)) & 127);
            for (int v = 0; v < 128; ++v)
                t[j][v] = (uint32_t)PEXT7[mj][v] << shift;
            shift += __builtin_popcount((unsigned)mj);
        }
    }
    inline uint32_t operator()(uint32_t x) const {
        return t[0][x & 127u] | t[1][(x >> 7) & 127u] | t[2][(x >> 14) & 127u];
    }
};

// For infrequent calls.
static uint32_t compressBits(uint32_t x, uint32_t m) {
    uint32_t r = 0;
    int b = 0;
    while (m) {
        int i = __builtin_ctz(m);
        m &= m - 1;
        if ((x >> i) & 1u) r |= (1u << b);
        ++b;
    }
    return r;
}

int main() {
    initPext7();

    // Smallest prime factor sieve.
    static int spf[MAXV + 1];
    for (int i = 2; i <= MAXV; ++i) {
        if (!spf[i]) {
            spf[i] = i;
            if ((long long)i * i <= MAXV)
                for (long long j = (long long)i * i; j <= MAXV; j += i)
                    if (!spf[j]) spf[j] = i;
        }
    }

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    static int cnt[MAXV + 1];
    for (int i = 0; i < n; ++i) {
        int x;
        cin >> x;
        ++cnt[x];
    }

    // Factorizations of values that occur.
    static vector<pair<int,int>> fac[MAXV + 1];
    static bool seenPrime[MAXV + 1];
    int maxSeen = 1;

    for (int v = 2; v <= MAXV; ++v) if (cnt[v]) {
        maxSeen = max(maxSeen, v);
        int x = v;
        while (x > 1) {
            int p = spf[x], e = 0;
            do { x /= p; ++e; } while (x % p == 0);
            fac[v].push_back({p, e});
            seenPrime[p] = true;
        }
    }

    // Bits for appearing small primes (<= 73).
    int bitOf[MAXV + 1];
    fill(begin(bitOf), end(bitOf), -1);
    vector<int> smallPrimes;
    for (int p = 2; p <= B; ++p)
        if (seenPrime[p]) {
            bitOf[p] = (int)smallPrimes.size();
            smallPrimes.push_back(p);
        }
    int k = (int)smallPrimes.size();
    int N = 1 << k;
    uint32_t ALL = (uint32_t)(N - 1);

    // Product of all distinct appearing primes, and its inverse.
    uint32_t prodQ = 1;
    for (int p = 2; p <= MAXV; ++p)
        if (seenPrime[p]) prodQ = mulmod(prodQ, (uint32_t)p);
    uint32_t invProdQ = powmod(prodQ, MOD - 2);

    auto smallMaskOf = [&](int v) -> uint32_t {
        uint32_t m = 0;
        int x = v;
        while (x > 1) {
            int p = spf[x];
            do { x /= p; } while (x % p == 0);
            // For a cofactor b <= 75 all prime factors are <= 73 and appear.
            m |= (1u << bitOf[p]);
        }
        return m;
    };

    // H0[m] will become, after subset zeta, the product over allowed indices
    // with no large prime factor.
    vector<uint32_t> F0(N, 1);
    if (cnt[1]) F0[0] = powmod(2, cnt[1]);

    struct Group {
        int p;
        uint32_t constp;
        int bcnt[76];
        Group(int pp) : p(pp), constp(1) { memset(bcnt, 0, sizeof(bcnt)); }
    };
    static int gid[MAXV + 1];
    fill(begin(gid), end(gid), -1);
    vector<Group> groups;
    groups.reserve(800);

    for (int v = 2; v <= MAXV; ++v) if (cnt[v]) {
        int large = 0;
        uint32_t mask = 0;
        for (auto &pe : fac[v]) {
            int p = pe.first;
            if (p > B) large = p;
            else mask |= (1u << bitOf[p]);
        }
        if (!large) {
            F0[(int)mask] = mulmod(F0[(int)mask], powmod((uint32_t)v + 1, cnt[v]));
        } else {
            int id = gid[large];
            if (id == -1) {
                id = (int)groups.size();
                gid[large] = id;
                groups.emplace_back(large);
            }
            Group &g = groups[id];
            int b = v / large; // b <= 75
            if (b == 1) {
                g.constp = mulmod(g.constp, powmod((uint32_t)large + 1, cnt[v]));
            } else {
                g.bcnt[b] += cnt[v];
            }
        }
    }

    // Multiply singleton masks by (q-1); subset zeta then gives F0(A)*W(A).
    for (int i = 0; i < k; ++i) {
        uint32_t q = (uint32_t)smallPrimes[i];
        F0[1 << i] = mulmod(F0[1 << i], q - 1);
    }

    // Multiplicative subset zeta.
    for (int i = 0; i < k; ++i) {
        int step = 1 << i;
        for (int base = 0; base < N; base += step << 1) {
            uint32_t *hi = F0.data() + base + step;
            uint32_t *lo = F0.data() + base;
            for (int j = 0; j < step; ++j)
                hi[j] = mulmod(hi[j], lo[j]);
        }
    }

    struct Factor {
        uint32_t S;
        vector<uint32_t> val;
    };
    vector<Factor> factors;
    factors.reserve(groups.size());
    vector<int> factorIdx(N, -1);
    uint32_t constFactor = 1;

    for (Group &g : groups) {
        int p = g.p;
        uint32_t S = 0;
        for (int b = 2; b <= 75; ++b)
            if (g.bcnt[b]) S |= smallMaskOf(b);

        uint32_t Cp = mulmod((uint32_t)(p - 1), g.constp);

        if (S == 0) {
            uint32_t f = (uint32_t)(1 + (uint64_t)Cp % MOD);
            if (f >= MOD) f -= MOD;
            constFactor = mulmod(constFactor, f);
            continue;
        }

        int s = __builtin_popcount(S);
        int sz = 1 << s;
        vector<uint32_t> H(sz, 1);
        for (int b = 2; b <= 75; ++b) if (g.bcnt[b]) {
            uint32_t maskb = smallMaskOf(b);
            int idx = (int)compressBits(maskb, S);
            H[idx] = mulmod(H[idx], powmod((uint32_t)((int64_t)p * b + 1), g.bcnt[b]));
        }
        for (int i = 0; i < s; ++i) {
            int step = 1 << i;
            for (int base = 0; base < sz; base += step << 1) {
                uint32_t *hi = H.data() + base + step;
                uint32_t *lo = H.data() + base;
                for (int j = 0; j < step; ++j)
                    hi[j] = mulmod(hi[j], lo[j]);
            }
        }
        for (int i = 0; i < sz; ++i) {
            uint32_t v = (uint32_t)(1 + (uint64_t)Cp * H[i] % MOD);
            if (v >= MOD) v -= MOD;
            H[i] = v;
        }

        int id = factorIdx[S];
        if (id == -1) {
            factorIdx[S] = (int)factors.size();
            factors.push_back(Factor{S, std::move(H)});
        } else {
            vector<uint32_t> &old = factors[id].val;
            for (int i = 0; i < sz; ++i) old[i] = mulmod(old[i], H[i]);
        }
    }

    // Multiply all factors into one product, incrementally extending the
    // currently used set of bits.  Greedy order keeps the intermediate domain
    // as small as possible.
    int D = (int)factors.size();
    uint32_t sumF = 0;
    if (D == 0) {
        for (int a = 0; a < N; ++a) {
            sumF += F0[a];
            if (sumF >= MOD) sumF -= MOD;
        }
    } else {
        vector<char> used(D, 0);
        vector<int> order;
        order.reserve(D);
        uint32_t Ccur = 0;
        for (int step = 0; step < D; ++step) {
            int best = -1, bestUnion = INT_MAX;
            for (int j = 0; j < D; ++j) if (!used[j]) {
                int u = __builtin_popcount(factors[j].S | Ccur);
                if (u < bestUnion) { bestUnion = u; best = j; }
            }
            used[best] = 1;
            order.push_back(best);
            Ccur |= factors[best].S;
        }

        vector<uint32_t> buf1(N ? N : 1), buf2(N ? N : 1);
        uint32_t *prod = buf1.data();
        uint32_t *tmp = buf2.data();
        int csize = 1;
        prod[0] = 1;
        uint32_t C = 0;

        for (int oi = 0; oi < D; ++oi) {
            const Factor &fac = factors[order[oi]];
            uint32_t S = fac.S;
            uint32_t newbits = S & ~C;
            if (newbits == 0) {
                uint32_t posS = compressBits(S, C);
                PextTable ts(posS);
                const uint32_t *fv = fac.val.data();
                int sz = 1 << __builtin_popcount(C);
                for (int x = 0; x < sz; ++x)
                    prod[x] = mulmod(prod[x], fv[ts((uint32_t)x)]);
            } else {
                uint32_t Cn = C | S;
                int c2 = __builtin_popcount(Cn);
                int sz = 1 << c2;
                uint32_t posC = compressBits(C, Cn);
                uint32_t posS = compressBits(S, Cn);
                PextTable tc(posC), ts(posS);
                const uint32_t *fv = fac.val.data();
                for (int x = 0; x < sz; ++x)
                    tmp[x] = mulmod(prod[tc((uint32_t)x)], fv[ts((uint32_t)x)]);
                swap(prod, tmp);
                csize = sz;
                C = Cn;
            }
            (void)csize;
        }

        if (C == ALL) {
            for (int a = 0; a < N; ++a)
                sumF = (sumF + mulmod(F0[a], prod[a])) % MOD;
        } else {
            PextTable tc(C);
            for (int a = 0; a < N; ++a)
                sumF = (sumF + mulmod(F0[a], prod[tc((uint32_t)a)])) % MOD;
        }
    }

    uint32_t ans = mulmod(sumF, constFactor);
    ans = mulmod(ans, invProdQ);
    cout << ans << '\n';
    return 0;
}
