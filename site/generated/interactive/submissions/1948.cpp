// Deleting Numbers - interactive solution.
//
// Observations
// ------------
// Let D be the union of the multiples of every number we have used in a "B"
// operation (the "would-be deleted" set; x is never removed from the actual
// set, so the actual set is always ([1,n] \ D) + {x if x in D}).
//
// For a query with parameter a the judge answers
//        |mult(a) \ D|  +  [x in D] * [a divides x].
// We can compute |mult(a) \ D| locally, hence every query reveals exactly one
// bit: "x is already flagged (x in D) and a divides x".
//
// * a = 1 is the only query whose bit does not need a | x, so "A 1" reveals
//   whether x in D.
// * Before x in D no query carries any information at all.
//
// Strategy
// --------
// 1) B p for every prime p <= sqrt(n): this deletes all composites and all
//    small primes, so afterwards the only possible values are
//    {1} union {primes > sqrt(n)} union {x}.
//    Bits returned by these B's are already meaningful for every prime that
//    was processed *after* the one that flagged x, which we exploit later.
// 2) If x is flagged (x composite or a small prime), determine the
//    sqrt(n)-smooth part s of x exactly (small primes are tested in
//    increasing order; the recorded bits give the rest for free; exponents by
//    testing p^2, p^3, ...).  Then x = s or x = s*q with q prime > sqrt(n) and
//    q <= n/s, and every remaining candidate is located by one "A q" query.
// 3) If x is not flagged, x is 1 or a prime > sqrt(n).  Testing membership of
//    x in a group G is done by deleting all of G ("B q") and then "A 1".
//    A DP over the number of remaining primes gives the split that minimises
//    the worst case (cost 2g when the group contains x, g+1+f(m-g) otherwise).
//    If x is inside the group it is found with |G|-1 divisibility queries.
//
// The worst case (x = 1 or a prime > n/2) needs pi(n)+... queries, which is
// provably within a few operations of the optimum: with x = 1 every prime must
// be removed by its own B operation.

#include <bits/stdc++.h>
using namespace std;

static int n;
static int sq;
static vector<char> deletedFlag;  // D indicator, 1-indexed
static int deletedCnt;
static vector<int> allPrimes;

static inline void outFlush() { fflush(stdout); }

static int readRet() {
    int v;
    if (scanf("%d", &v) != 1) exit(0);
    if (v < 0) exit(0);
    return v;
}

// |mult(a) \ D| for the current D.
static int baseCount(int a) {
    int cnt = 0;
    for (int m = a; m <= n; m += a)
        if (!deletedFlag[m]) ++cnt;
    return cnt;
}

// Information bit of an "A a" query: [x in D] * [a | x].
static int askA(int a) {
    int base = baseCount(a);
    printf("A %d\n", a);
    outFlush();
    return readRet() - base;
}

// Information bit of a "B a" query (bit evaluated before the deletion), then
// perform the deletion.
static int askB(int a) {
    int base = baseCount(a);
    printf("B %d\n", a);
    outFlush();
    int c = readRet();
    for (int m = a; m <= n; m += a)
        if (!deletedFlag[m]) { deletedFlag[m] = 1; ++deletedCnt; }
    return c - base;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    if (n == 1) {
        printf("C 1\n");
        outFlush();
        return 0;
    }

    // Sieve of Eratosthenes.
    vector<char> isComp(n + 1, 0);
    for (int i = 2; i <= n; ++i) {
        if (!isComp[i]) {
            allPrimes.push_back(i);
            if (1LL * i * i <= n)
                for (long long j = 1LL * i * i; j <= n; j += i) isComp[(int)j] = 1;
        }
    }

    sq = (int)sqrt((double)n);
    while (1LL * (sq + 1) * (sq + 1) <= n) ++sq;
    while (1LL * sq * sq > n) --sq;

    vector<int> sp;  // primes p <= sq
    for (int p : allPrimes)
        if (p <= sq) sp.push_back(p);
    int K = (int)sp.size();

    deletedFlag.assign(n + 1, 0);
    deletedCnt = 0;

    // ---- Step 1: remove multiples of every prime <= sqrt(n) ----------------
    vector<int> bits1(K, 0);
    for (int j = 0; j < K; ++j) bits1[j] = askB(sp[j]);

    // ---- Is x already flagged? --------------------------------------------
    bool flagged = false;
    int k1 = -1;  // first index whose recorded bit was 1
    for (int j = 0; j < K; ++j) {
        if (bits1[j] == 1) { flagged = true; k1 = j; break; }
    }
    if (!flagged && K > 0) {
        if (askA(1) == 1) flagged = true;
    }

    long long x = -1;

    if (!flagged) {
        // ================= case II: x == 1 or a prime > sq ==================
        vector<int> R;  // primes sq < p <= n
        for (int p : allPrimes)
            if (p > sq) R.push_back(p);
        int m = (int)R.size();

        // f[m] / bestg[m]: optimal worst-case cost and first group size for
        // the "delete a group, then A 1" game with m candidate primes left.
        vector<int> bestg(m + 1, 1), f(m + 1, 0);
        for (int mm = 1; mm <= m; ++mm) {
            int best = INT_MAX, bgg = 1;
            for (int g = 1; g <= mm; ++g) {
                int c1 = 2 * g;                 // x inside the group
                int c2 = g + 1 + f[mm - g];     // x outside the group
                int c = c1 > c2 ? c1 : c2;
                if (c < best) { best = c; bgg = g; }
            }
            f[mm] = best;
            bestg[mm] = bgg;
        }

        int remStart = 0;
        bool done = false;
        while (m > 0) {
            int g = bestg[m];
            for (int i = remStart; i < remStart + g; ++i) askB(R[i]);
            if (askA(1) == 1) {
                // x is one of R[remStart .. remStart+g-1]; find it (skip last).
                int found = -1;
                for (int i = remStart; i < remStart + g - 1; ++i) {
                    if (askA(R[i]) == 1) { found = R[i]; break; }
                }
                if (found < 0) found = R[remStart + g - 1];
                x = found;
                done = true;
                break;
            }
            remStart += g;
            m -= g;
        }
        if (!done) x = 1;
    } else {
        // ============ case I: x is composite or a small prime ===============
        // Locate the smallest prime factor of x.
        int sIdx = -1;
        if (k1 >= 0) {
            // Some prime p_{k1} divides x, so the smallest one has index <= k1.
            for (int j = 0; j < k1; ++j)
                if (askA(sp[j]) == 1) { sIdx = j; break; }
            if (sIdx < 0) sIdx = k1;
        } else {
            for (int j = 0; j < K; ++j)
                if (askA(sp[j]) == 1) { sIdx = j; break; }
        }

        // Exact sqrt(n)-smooth part s of x.
        long long s = 1;
        for (int j = 0; j < K; ++j) {
            int div;
            if (j < sIdx) div = 0;          // tested and found negative
            else if (j == sIdx) div = 1;    // the smallest prime factor
            else div = bits1[j];            // recorded after x was flagged
            if (!div) continue;
            long long p = sp[j];
            long long pe = p;
            while (pe * p <= n) {
                if (askA((int)(pe * p)) == 1) pe *= p;
                else break;
            }
            s *= pe;
        }

        // Cofactor t is 1 or a prime q with sq < q <= n/s; at most one of the
        // candidates can divide x, so probe them (all are needed when t == 1).
        long long qmax = n / s;
        long long t = 1;
        for (int p : allPrimes) {
            if (p <= sq) continue;
            if ((long long)p > qmax) break;
            if (askA(p) == 1) { t = p; break; }
        }
        x = s * t;
    }

    printf("C %lld\n", x);
    outFlush();
    return 0;
}
