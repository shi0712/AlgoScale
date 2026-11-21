#include <bits/stdc++.h>
using namespace std;

static const int MAXN = 1000000000;
static long long C[35][35];

static int readInt() {
    int x;
    if (!(cin >> x)) exit(0);
    return x;
}

static void sendQuery(long long x) {
    cout << "- " << x << '\n' << flush;
}

static void sendAnswer(long long n) {
    cout << "! " << n << '\n' << flush;
}

// Count numbers in [0, X] with exactly k set bits.
static long long countLE(long long X, int k) {
    if (X < 0 || k < 0) return 0;
    long long cnt = 0;
    int used = 0;
    for (int i = 29; i >= 0; --i) {
        if ((X >> i) & 1LL) {
            int need = k - used;
            if (need >= 0 && need <= i) cnt += C[i][need];
            ++used;
            if (used > k) break;
        }
    }
    if (used == k) ++cnt;
    return cnt;
}

static void genCandidatesBound(int c, long long bound, vector<int>& out) {
    if (c == 0) {
        out.push_back(0);
        return;
    }
    if (bound <= 0) return;
    long long comb = 1;
    for (int i = 0; i < c; ++i) comb = comb * (30 - i) / (i + 1);
    if (comb > 0 && comb < 30000000LL) out.reserve((size_t)comb);
    // Generate in increasing numeric order (highest set bit groups from low
    // to high), so no sorting is needed afterwards.
    function<void(int,int,long long)> rec = [&](int maxBit, int left, long long base) {
        for (int h = left - 1; h <= maxBit; ++h) {
            long long v = base | (1LL << h);
            if (v > bound) break;
            if (left == 1) {
                out.push_back((int)v);
            } else {
                rec(h - 1, left - 1, v);
            }
        }
    };
    rec(29, c, 0);
}

static void selectQuery(const vector<int>& cur, long long& bestX, int& bestMax) {
    int minM = cur.front();
    vector<long long> pool;
    pool.reserve(160);
    auto add = [&](long long v) {
        if (v >= 1 && v <= minM) pool.push_back(v);
    };
    add(1);
    add(minM);
    for (long long v = 1; v <= minM; v <<= 1) {
        add(v);
        add(v - 1);
        add((long long)minM - v);
        add((long long)minM - (v - 1));
        if (v > (minM >> 1)) break;
    }
    sort(pool.begin(), pool.end());
    pool.erase(unique(pool.begin(), pool.end()), pool.end());
    {
        auto it = find(pool.begin(), pool.end(), (long long)minM);
        if (it != pool.end()) rotate(pool.begin(), it, it + 1);
    }
    bestX = 1;
    bestMax = INT_MAX;
    int bestGroups = -1;
    int cnt[32];
    for (long long xl : pool) {
        int x = (int)xl;
        memset(cnt, 0, sizeof(cnt));
        int groups = 0, mx = 0;
        bool bad = false;
        for (int m : cur) {
            int p = __builtin_popcount((unsigned)(m - x));
            if (cnt[p]++ == 0) ++groups;
            if (cnt[p] > mx) {
                mx = cnt[p];
                if (mx > bestMax) { bad = true; break; }
            }
        }
        if (bad) continue;
        if (mx < bestMax || (mx == bestMax && groups > bestGroups)) {
            bestMax = mx; bestGroups = groups; bestX = x;
            if (mx == 1) break;
        }
    }
}

// Greedy on the high part H.  Current value is H * 2^s + junk.
static void greedyHigh(int rem, int s, long long junk0, long long sub,
                       long long Hmax) {
    vector<int> hcur;
    genCandidatesBound(rem, Hmax, hcur);
    long long junk = junk0;
    while (hcur.size() > 1) {
        long long y;
        int mx;
        selectQuery(hcur, y, mx);
        long long actual = junk + (y << s);
        sendQuery(actual);
        int p = readInt();
        if (p < 0) exit(0);
        vector<int> nxt;
        nxt.reserve(hcur.size());
        for (int h : hcur) {
            int v = (int)((long long)h - y);
            if (__builtin_popcount((unsigned)v) == p) nxt.push_back(v);
        }
        hcur.swap(nxt);
        junk = 0;
        sub += actual;
    }
    long long curVal = (long long)hcur[0] * (1LL << s) + junk;
    sendAnswer(curVal + sub);
}

// Adaptive: peel queries while there are too many possible high parts,
// then solve the remaining small candidate set by the greedy strategy.
static void solveAdaptive(int c) {
    const long long LIMIT = 25000000LL;
    int rem = c;
    int s = 0;
    long long J = 0, sub = 0;
    long long bitsAns = 0;
    while (true) {
        long long avail = (long long)MAXN - sub - J;
        long long Hmax = avail >= 0 ? avail / (1LL << s) : -1;
        long long cnt = countLE(Hmax, rem);
        if (cnt >= 1 && cnt <= LIMIT) {
            greedyHigh(rem, s, J, sub, Hmax);
            return;
        }
        // One peeling query.
        long long x = J + (1LL << s);
        sendQuery(x);
        int p = readInt();
        if (p < 0) exit(0);
        int t = p - rem + 1;
        bitsAns |= (1LL << (s + t));
        sub += x;
        if (rem == 1) {
            sendAnswer(bitsAns);
            return;
        }
        J = ((1LL << t) - 1) << s;
        s = s + t + 1;
        --rem;
    }
}

// Pure peeling fallback for small popcounts.
static long long solvePeel(int c) {
    long long ans = 0;
    int rem = c;
    int s = 0;
    long long J = 0;
    for (int it = 0; it < c; ++it) {
        long long x = J + (1LL << s);
        sendQuery(x);
        int p = readInt();
        if (p < 0) exit(0);
        int t = p - rem + 1;
        ans |= (1LL << (s + t));
        if (it + 1 == c) break;
        J = ((1LL << t) - 1) << s;
        s = s + t + 1;
        --rem;
    }
    sendAnswer(ans);
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    for (int i = 0; i <= 32; ++i) {
        C[i][0] = C[i][i] = 1;
        for (int j = 1; j < i; ++j) C[i][j] = C[i - 1][j - 1] + C[i - 1][j];
    }

    int c = readInt();
    if (c < 0) return 0;
    if (c <= 9) {
        solvePeel(c);
    } else {
        solveAdaptive(c);
    }
    return 0;
}
