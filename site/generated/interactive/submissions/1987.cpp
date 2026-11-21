// Duloc Network - interactive connectivity test.
//
// Query:  q(S) = |N(S) \ S|  (number of vertices outside S adjacent to S).
//
// Main tool.  For disjoint sets A,B and C = V \ (A u B),
//     q(A) + q(B) - q(A u B)
//   = |N(A) n B| + |N(B) n A| + |N(A) n N(B) n C|          ( >= 0 )
// This is positive iff some a in A and b in B are at distance <= 2 in G, i.e.
// iff A and B are joined in H = G^2.  Since H has the same connected
// components as G, we can grow the H-component of a start vertex.  The value is
// at most n, and equals n only if every vertex of B is adjacent to A, in which
// case the whole of B may be merged at once.
//
// A witness inside a candidate set is located by binary search with the test
// above (O(log n) queries per witness), giving O(n log n) queries overall.
//
// Certificates used:
//   q(S) == n - |S|  -> every vertex outside S touches S; with S H-connected
//                       (which is maintained) this proves G connected.
//   q(S) == 0, S != V -> S is closed, hence G disconnected.

#include <bits/stdc++.h>
using namespace std;

static const int MAXW = 4;  // n <= 200 -> 4*64 bits

static int n, W;

struct Bits {
    uint64_t w[MAXW];
    Bits() { w[0] = w[1] = w[2] = w[3] = 0; }
    inline bool test(int i) const { return (w[i >> 6] >> (i & 63)) & 1ULL; }
    inline void set(int i) { w[i >> 6] |= 1ULL << (i & 63); }
};

static inline Bits OR(const Bits &a, const Bits &b) {
    Bits r;
    for (int i = 0; i < W; i++) r.w[i] = a.w[i] | b.w[i];
    return r;
}
static inline Bits XOR(const Bits &a, const Bits &b) {
    Bits r;
    for (int i = 0; i < W; i++) r.w[i] = a.w[i] ^ b.w[i];
    return r;
}
static inline bool eqBits(const Bits &a, const Bits &b) {
    for (int i = 0; i < W; i++) if (a.w[i] != b.w[i]) return false;
    return true;
}
static inline Bits AND(const Bits &a, const Bits &b) {
    Bits r;
    for (int i = 0; i < W; i++) r.w[i] = a.w[i] & b.w[i];
    return r;
}
static inline bool anyBits(const Bits &a) {
    for (int i = 0; i < W; i++) if (a.w[i]) return true;
    return false;
}

struct BitsHash {
    size_t operator()(const Bits &b) const {
        uint64_t h = 1469598103934665603ULL;
        for (int i = 0; i < W; i++) { h ^= b.w[i]; h *= 1099511628211ULL; h ^= h >> 29; }
        return (size_t)h;
    }
};
struct BitsEq {
    bool operator()(const Bits &a, const Bits &b) const { return eqBits(a, b); }
};

static Bits FULL;
static unordered_map<Bits, int, BitsHash, BitsEq> cache;
static char outbuf[512];

static inline int readInt() {
    int c = getchar();
    while (c < '0' || c > '9') {
        if (c == EOF) exit(0);
        c = getchar();
    }
    int x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = getchar(); }
    return x;
}

// query q(S), with memoisation
static int doQuery(const Bits &S) {
    auto it = cache.find(S);
    if (it != cache.end()) return it->second;
    outbuf[0] = '?'; outbuf[1] = ' ';
    for (int i = 0; i < n; i++) outbuf[2 + i] = S.test(i) ? '1' : '0';
    outbuf[2 + n] = '\n';
    fwrite(outbuf, 1, (size_t)n + 3, stdout);
    fflush(stdout);
    int r = readInt();
    cache.emplace(S, r);
    return r;
}

static Bits S;                // current H-connected set
static int qS;                // q(S)
static vector<int> curList;   // V \ S
static vector<int> witness;   // witnesses found in this round
static long long curSc;       // |S|

static const int BATCH = 4;         // certificate check every BATCH witnesses
static const int ADAPT_MUL = 8;     // inner linear scan if L >= m/ADAPT_MUL
static const int ROOT_MUL = 16;     // outer linear scan if qS >= m/ROOT_MUL
static bool connectedCertified = false;

// Check the "S dominates V" certificate for S | found.  Returns true (and sets
// connectedCertified) when the graph is proven connected.
static bool checkCertificate(const vector<int> &found) {
    if (found.empty()) return false;
    Bits S2 = S;
    for (int x : found) S2.set(x);
    long long sc2 = curSc + (long long)found.size();
    int val = doQuery(S2);
    if (val == n - (int)sc2) { connectedCertified = true; return true; }
    return false;
}

// Precondition: D(S, curList[lo..hi)) > 0.  dX is that value, or -1 if only
// positivity is known.
static void collect(int lo, int hi, long long dX) {
    if (connectedCertified) return;
    int m = hi - lo;
    if (m == 1) { witness.push_back(curList[lo]); return; }

    if (dX >= 0) {
        long long low = dX - (n - m);  // lower bound on the number of witnesses
        if (low * (long long)ADAPT_MUL >= (long long)m) {
            vector<int> found;
            int lastCheck = 0;
            for (int i = lo; i < hi; i++) {
                Bits X; X.set(curList[i]);
                if ((long long)qS + doQuery(X) - doQuery(OR(S, X)) > 0) {
                    found.push_back(curList[i]);
                    if ((int)found.size() - lastCheck >= BATCH) {
                        lastCheck = (int)found.size();
                        if (checkCertificate(found)) return;
                    }
                }
            }
            if (checkCertificate(found)) return;
            for (int x : found) witness.push_back(x);
            return;
        }
    }

    int mid = (lo + hi) >> 1;
    Bits A, B;
    for (int i = lo; i < mid; i++) A.set(curList[i]);
    for (int i = mid; i < hi; i++) B.set(curList[i]);

    int qA = doQuery(A);
    long long dA = (long long)qS + qA - doQuery(OR(S, A));
    if (dA == n) {
        for (int i = lo; i < mid; i++) witness.push_back(curList[i]);
        if (connectedCertified) return;
        int qB = doQuery(B);
        long long dB = (long long)qS + qB - doQuery(OR(S, B));
        if (dB == n) { for (int i = mid; i < hi; i++) witness.push_back(curList[i]); }
        else if (dB > 0) collect(mid, hi, dB);
    } else if (dA > 0) {
        collect(lo, mid, dA);
        if (connectedCertified) return;
        int qB = doQuery(B);
        long long dB = (long long)qS + qB - doQuery(OR(S, B));
        if (dB == n) { for (int i = mid; i < hi; i++) witness.push_back(curList[i]); }
        else if (dB > 0) collect(mid, hi, dB);
    } else {
        // no witness in A, hence at least one in B
        collect(mid, hi, -1);
    }
}

int main() {
    n = readInt();
    W = (n + 63) >> 6;
    for (int i = 0; i < n; i++) FULL.set(i);
    if (n <= 1) { printf("! 1\n"); fflush(stdout); return 0; }

    {
        Bits X0; X0.set(0);
        int d0 = doQuery(X0);
        if (d0 == n - 1) { printf("! 1\n"); fflush(stdout); return 0; }
        if (d0 == 0) { printf("! 0\n"); fflush(stdout); return 0; }
        int bestV = 0, bestD = d0;
        if (d0 == 1 && n > 2) {
            const int KS = 8;
            int k = min(n, KS);
            for (int j = 1; j < k; j++) {
                int v = (int)((long long)j * (n - 1) / (k - 1));
                Bits X; X.set(v);
                int d = doQuery(X);
                if (d == n - 1) { printf("! 1\n"); fflush(stdout); return 0; }
                if (d == 0) { printf("! 0\n"); fflush(stdout); return 0; }
                if (d > bestD) { bestD = d; bestV = v; }
                else if (d == bestD && d >= 2) {
                    if (abs(v - n / 2) < abs(bestV - n / 2)) bestV = v;
                }
            }
        }
        S.set(bestV);
        qS = bestD;
    }
    curSc = 1;
    curList.reserve(n);
    witness.reserve(n);

    while (true) {
        if (curSc == n) { printf("! 1\n"); fflush(stdout); return 0; }
        if (qS == n - (int)curSc) { printf("! 1\n"); fflush(stdout); return 0; }
        if (qS == 0) { printf("! 0\n"); fflush(stdout); return 0; }

        Bits R = XOR(FULL, S);
        curList.clear();
        for (int v = 0; v < n; v++) if (R.test(v)) curList.push_back(v);
        witness.clear();
        int m = (int)curList.size();

        if ((long long)qS * ROOT_MUL >= (long long)m) {
            // Many direct neighbours: scan the remaining vertices, checking the
            // domination certificate periodically.
            vector<int> found;
            int lastCheck = 0;
            for (int i = 0; i < m; i++) {
                Bits X; X.set(curList[i]);
                if ((long long)qS + doQuery(X) - doQuery(OR(S, X)) > 0) {
                    found.push_back(curList[i]);
                    if ((int)found.size() - lastCheck >= BATCH) {
                        lastCheck = (int)found.size();
                        if (checkCertificate(found)) { printf("! 1\n"); fflush(stdout); return 0; }
                    }
                }
            }
            if (checkCertificate(found)) { printf("! 1\n"); fflush(stdout); return 0; }
            if (found.empty()) { printf("! 0\n"); fflush(stdout); return 0; }
            for (int x : found) S.set(x);
            curSc += (long long)found.size();
            qS = doQuery(S);
            continue;
        }

        if (m == 1) {
            Bits X; X.set(curList[0]);
            long long d = (long long)qS + doQuery(X) - doQuery(OR(S, X));
            if (d > 0) witness.push_back(curList[0]);
        } else {
            int mid = m >> 1;
            Bits A, B;
            for (int i = 0; i < mid; i++) A.set(curList[i]);
            for (int i = mid; i < m; i++) B.set(curList[i]);
            int qA = doQuery(A);
            long long dA = (long long)qS + qA - doQuery(OR(S, A));
            int qB = doQuery(B);
            long long dB = (long long)qS + qB - doQuery(OR(S, B));

            if (dA == 0 && dB == 0) { printf("! 0\n"); fflush(stdout); return 0; }

            if (dA == n) { for (int i = 0; i < mid; i++) witness.push_back(curList[i]); }
            else if (dA > 0) collect(0, mid, dA);
            if (connectedCertified) { printf("! 1\n"); fflush(stdout); return 0; }

            if (dB == n) { for (int i = mid; i < m; i++) witness.push_back(curList[i]); }
            else if (dB > 0) collect(mid, m, dB);
        }

        if (witness.empty()) { printf("! 0\n"); fflush(stdout); return 0; }
        for (int x : witness) S.set(x);
        curSc += (long long)witness.size();
        qS = doQuery(S);
    }
}
