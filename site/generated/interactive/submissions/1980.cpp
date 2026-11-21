// Library of Magic - interactive solution
//
// Query "xor l r" returns the XOR of the stolen numbers inside [l,r]
// (non-stolen types cancel because both copies remain).  Let S = a^b^c.
//
//  * S != 0:  binary-search the value axis.  For a node [L,R] known to
//    contain all three numbers let X be their xor.  Asking the left half:
//       0   -> left half empty          (xor 0  <=> empty, a pair can't xor to 0)
//       X   -> right half empty
//       else-> both halves non-empty; one holds one number, the other two.
//    A single point query on the left xor decides which side is the pair.
//    The pair with known xor P inside [lo,hi] is found by binary searching
//    over the *candidate* pairs {p, p^P}, costing log2(#candidates) queries
//    (0 when the interval pins the pair down uniquely).
//
//  * S == 0:  then c = a^b and msb(c)=h satisfies a < 2^h <= b < c.  Scanning
//    f(2^k) downwards finds h (the largest power of two inside [a,c)) together
//    with a plateau value; a binary search in (2^h, 2^(h+1)] locates c.
#include <cstdio>
#include <cstdlib>

typedef long long ll;
typedef unsigned long long ull;

static ll ASK(ll l, ll r) {
    printf("xor %lld %lld\n", l, r);
    fflush(stdout);
    ll v;
    if (scanf("%lld", &v) != 1) exit(0);
    if (v == -1) exit(0);
    return v;
}

static inline int msbll(ll v) { return 63 - __builtin_clzll((ull)v); }

// F(X,Y) = #{ p : 0<=p<=X, 0<=(p^P)<=Y, bit h of p == 0 }
static ll Fcnt(ll X, ll Y, ll P, int h) {
    if (X < 0 || Y < 0) return 0;
    ll dp[2][2] = {{0, 0}, {0, 0}};
    dp[1][1] = 1;
    for (int i = 62; i >= 0; --i) {
        ll nd[2][2] = {{0, 0}, {0, 0}};
        int xb = (int)(((ull)X >> i) & 1);
        int yb = (int)(((ull)Y >> i) & 1);
        int pb = (int)(((ull)P >> i) & 1);
        for (int tx = 0; tx < 2; ++tx)
            for (int ty = 0; ty < 2; ++ty) {
                ll cur = dp[tx][ty];
                if (!cur) continue;
                for (int b = 0; b < 2; ++b) {
                    if (i == h && b) continue;          // bit h of p must be 0
                    int qb = b ^ pb;
                    if (tx && b > xb) continue;
                    if (ty && qb > yb) continue;
                    int ntx = (tx && b == xb) ? 1 : 0;
                    int nty = (ty && qb == yb) ? 1 : 0;
                    nd[ntx][nty] += cur;
                }
            }
        dp[0][0] = nd[0][0]; dp[0][1] = nd[0][1];
        dp[1][0] = nd[1][0]; dp[1][1] = nd[1][1];
    }
    return dp[0][0] + dp[0][1] + dp[1][0] + dp[1][1];
}

// # candidates p with lo<=p<=m, lo<=(p^P)<=hi, bit h of p == 0   (P > 0)
static ll count_cand_pref(ll lo, ll hi, ll m, ll P, int h) {
    if (lo > hi || m < lo || P == 0) return 0;
    return Fcnt(m, hi, P, h) - Fcnt(lo - 1, hi, P, h)
         - Fcnt(m, lo - 1, P, h) + Fcnt(lo - 1, lo - 1, P, h);
}

// number of candidate pairs p,p^P both inside [lo,hi]
static ll count_cand(ll lo, ll hi, ll P, int h) {
    return count_cand_pref(lo, hi, hi, P, h);
}

// smallest p in [lo,hi] with (#candidates having value <= p) >= k+1
static ll kth_cand(ll lo, ll hi, ll P, int h, ll k) {
    ll a = lo, b = hi;
    while (a < b) {
        ll m = a + (b - a) / 2;
        if (count_cand_pref(lo, hi, m, P, h) >= k + 1) b = m;
        else a = m + 1;
    }
    return a;
}

// find the unique pair with xor P inside [lo,hi]
static void resolve_pair(ll lo, ll hi, ll P, ll &e2, ll &e3) {
    int h = msbll(P);
    while (true) {
        ll cnt = count_cand(lo, hi, P, h);
        if (cnt == 1) {
            ll p = kth_cand(lo, hi, P, h, 0);
            e2 = p;
            e3 = p ^ P;
            return;
        }
        if (cnt == 0) {                       // defensive; cannot happen
            while (lo < hi) {
                ll m = lo + (hi - lo) / 2;
                ll q = ASK(lo, m);
                if (q == 0) { lo = m + 1; continue; }
                if (q == P) { hi = m; continue; }
                e2 = q; e3 = P ^ q; return;
            }
            e2 = lo; e3 = lo ^ P;
            return;
        }
        ll m = kth_cand(lo, hi, P, h, cnt / 2);
        ll q = ASK(lo, m);
        if (q == 0) lo = m + 1;
        else if (q == P) hi = m;
        else { e2 = q; e3 = P ^ q; return; }
    }
}

int main() {
    ll N;
    if (scanf("%lld", &N) != 1) return 0;
    ll S = ASK(1, N);
    if (S != 0) {
        ll L = 1, R = N, X = S;
        ll e1 = 0, e2 = 0, e3 = 0;
        while (true) {
            ll mid = L + (R - L) / 2;
            ll q = ASK(L, mid);
            if (q == 0) { L = mid + 1; continue; }   // left half empty
            if (q == X) { R = mid; continue; }        // right half empty
            ll Y = X ^ q;
            int hq = msbll(q), hY = msbll(Y);
            ll cLeft = count_cand(L, mid, q, hq);     // left holds the pair (xor q)
            ll cRight = count_cand(mid + 1, R, Y, hY);
            bool H1 = (L <= q && q <= mid) && cRight > 0;    // left singleton = q
            bool H2 = (mid + 1 <= Y && Y <= R) && cLeft > 0; // right singleton = Y
            ll pL, pR, px;
            if (H1 && !H2) { e1 = q; pL = mid + 1; pR = R; px = Y; }
            else if (H2 && !H1) { e1 = Y; pL = L; pR = mid; px = q; }
            else {
                bool leftSingle;
                if (q > N) leftSingle = false;        // q cannot be a stolen value
                else leftSingle = (ASK(q, q) == q);
                if (leftSingle) { e1 = q; pL = mid + 1; pR = R; px = Y; }
                else { e1 = Y; pL = L; pR = mid; px = q; }
            }
            resolve_pair(pL, pR, px, e2, e3);
            break;
        }
        printf("ans %lld %lld %lld\n", e1, e2, e3);
        fflush(stdout);
    } else {
        int K = 0;
        while ((1LL << (K + 1)) <= N) ++K;
        ll v = 0; int h = -1;
        for (int k = K; k >= 0; --k) {
            ll q = ASK(1, 1LL << k);
            if (q != 0) { v = q; h = k; break; }
        }
        if (h < 0) h = 0;                             // impossible for valid input
        ll twoH = 1LL << h;
        ll hi0 = (h + 1 <= K) ? (1LL << (h + 1)) : N; // a position known to have prefix-xor 0
        ll lo = twoH, hi = hi0;
        ll a = 0, b = 0, c = 0;
        bool done = false;
        while (hi - lo > 1) {
            ll m = lo + (hi - lo) / 2;
            ll q = ASK(1, m);
            if (q == 0) hi = m;                       // first zero position == c
            else if (q != v) { a = v; c = q; b = v ^ q; done = true; break; }
            else lo = m;
        }
        if (!done) {
            ll m0 = hi;
            if (m0 != v) { a = v; c = m0; b = a ^ c; }
            else { b = twoH; c = v; a = twoH ^ v; }
        }
        printf("ans %lld %lld %lld\n", a, b, c);
        fflush(stdout);
    }
    return 0;
}
