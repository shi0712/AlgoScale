#include <bits/stdc++.h>
using namespace std;

static int n;
static long long g_queries = 0;

static int ask(int i, int j) {
    printf("? %d %d\n", i, j);
    fflush(stdout);
    int x;
    if (scanf("%d", &x) != 1) exit(0);
    ++g_queries;
    return x;
}

static void finish(int cnt, const vector<int>& p) {
    printf("!\n");
    printf("%d\n", cnt);
    for (int i = 0; i < n; ++i) {
        if (i) putchar(' ');
        printf("%d", p[i]);
    }
    putchar('\n');
    fflush(stdout);
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    if (n == 1) {
        finish(1, vector<int>{0});
        return 0;
    }
    if (n == 2) {
        finish(2, vector<int>{0, 1});
        return 0;
    }

    int h = n & (-n);                 // H = {0,1,...,h-1}

    // ---------- Phase 1: n-1 queries in column 0 ----------
    vector<int> R(n, 0);
    for (int i = 1; i < n; ++i) R[i] = ask(i, 0);

    // Reconstruct all permutations consistent with these replies.
    // Candidate x means we guess b_0 = x; then q_i = R_i xor x (i>=1),
    // q_0 = missing value, and we require q_x = 0.
    vector<int> seen(n, 0);
    int stamp = 0;
    vector<int> candX;
    vector<int> candK;                // K = implied A_0 = q_0 xor x
    for (int x = 0; x < n; ++x) {
        ++stamp;
        bool ok = true;
        for (int i = 1; i < n; ++i) {
            int v = R[i] ^ x;
            if (v >= n || seen[v] == stamp) { ok = false; break; }
            seen[v] = stamp;
        }
        if (!ok) continue;
        int missing = -1;
        for (int v = 0; v < n; ++v)
            if (seen[v] != stamp) { missing = v; break; }
        int qx = (x == 0) ? missing : (R[x] ^ x);
        if (qx != 0) continue;
        candX.push_back(x);
        candK.push_back(missing ^ x);
    }

    bool sameK = !candK.empty();
    int K0 = candK.empty() ? 0 : candK[0];
    for (int k : candK)
        if (k != K0) { sameK = false; break; }

    // If all candidates share the same implied A_0, we know A completely.
    // Check whether they also form a single equivalence class; if so, stop.
    if (sameK && !candX.empty()) {
        int bx = candX[0];
        vector<int> qbase(n);
        {
            ++stamp;
            for (int i = 1; i < n; ++i) {
                int v = R[i] ^ bx;
                qbase[i] = v;
                seen[v] = stamp;
            }
            int missing = -1;
            for (int v = 0; v < n; ++v)
                if (seen[v] != stamp) { missing = v; break; }
            qbase[0] = missing;
        }
        vector<int> G;
        for (int c = 0; c < h; ++c) {
            bool ok = true;
            for (int i = 0; i < n; ++i)
                if (qbase[i ^ c] != (qbase[i] ^ c)) { ok = false; break; }
            if (ok) G.push_back(c);
        }
        bool oneClass = true;
        int firstCanon = -1;
        for (int x : candX) {
            int d = x ^ bx;
            int best = INT_MAX;
            for (int g : G) best = min(best, d ^ g);
            if (firstCanon < 0) firstCanon = best;
            else if (best != firstCanon) { oneClass = false; break; }
        }
        if (oneClass) {
            finish((int)G.size(), qbase);
            return 0;
        }
    }

    // ---------- Determine A completely ----------
    vector<int> A(n, 0);
    for (int i = 1; i < n; ++i) A[i] = R[i];
    if (sameK) {
        A[0] = K0;
    } else if ((n & (n - 1)) == 0) {
        // n is a power of two: A is a permutation of [0,n)
        vector<char> used(n, 0);
        for (int i = 1; i < n; ++i) used[R[i]] = 1;
        int missing = -1;
        for (int v = 0; v < n; ++v)
            if (!used[v]) { missing = v; break; }
        A[0] = missing;
    } else {
        A[0] = ask(0, 0);
    }

    int M = 1;
    while (M <= n) M <<= 1;

    vector<int> invA(M, -1);
    for (int i = 0; i < n; ++i) invA[A[i]] = i;

    // ---------- Class group G ----------
    vector<int> G;
    for (int c = 0; c < h; ++c) {
        bool ok = true;
        for (int i = 0; i < n; ++i)
            if (A[i ^ c] != (A[i] ^ c)) { ok = false; break; }
        if (ok) G.push_back(c);
    }
    int answerCount = (int)G.size();

    // ---------- Fixed points / non-fixed positions ----------
    vector<int> F;
    vector<char> isF(n, 0);
    for (int i = 0; i < n; ++i)
        if (A[i] == i) { F.push_back(i); isF[i] = 1; }
    vector<int> E;                     // positions where A differs from identity
    for (int i = 0; i < n; ++i)
        if (!isF[i]) E.push_back(i);

    // Find b_0: a fixed point s with A xor s a permutation of [0,n).
    int b0 = -1;
    {
        vector<char> seen2(n, 0);
        for (int s : F) {
            fill(seen2.begin(), seen2.end(), 0);
            bool ok = true;
            for (int i = 0; i < n; ++i) {
                int v = A[i] ^ s;
                if (v >= n || seen2[v]) { ok = false; break; }
                seen2[v] = 1;
            }
            if (ok) { b0 = s; break; }
        }
    }
    // C = b0 xor H ; valid candidate translations.
    vector<int> Fv;
    for (int s : F)
        if ((s ^ b0) < h) Fv.push_back(s);

    // Canonical representative of G-cosets inside H.
    vector<int> canon(h, INT_MAX);
    for (int x = 0; x < h; ++x) {
        int best = INT_MAX;
        for (int g : G) best = min(best, x ^ g);
        canon[x] = best;
    }

    int base = Fv[0];
    vector<int> reps;
    {
        vector<int> repsMap(h, -1);
        for (int s : Fv) {
            int id = canon[s ^ base];
            if (repsMap[id] < 0) {
                repsMap[id] = s;
                reps.push_back(s);
            }
        }
    }

    // ---------- Resolve which class is the hidden one ----------
    vector<char> usedJ(n, 0);
    vector<int> valcnt(M, 0);
    vector<int> hitcnt(M, 0);
    bool forceScan = false;

    while ((int)reps.size() > 1) {
        int m = (int)reps.size();
        int chosen = -1;
        long long sc = (long long)n * m;
        bool targeted = (!forceScan && sc > 1000000LL);

        if (targeted) {
            vector<int> touched;
            int bestcnt = 0;
            for (int sp : reps) {
                for (int y : E) {
                    int j = sp ^ y;
                    if (j >= 1 && j < n && !usedJ[j]) {
                        if (hitcnt[j] == 0) touched.push_back(j);
                        ++hitcnt[j];
                        if (hitcnt[j] > bestcnt) { bestcnt = hitcnt[j]; chosen = j; }
                    }
                }
            }
            for (int j : touched) hitcnt[j] = 0;
            if (chosen < 0) forceScan = true;
        }

        if (!targeted || forceScan) {
            chosen = -1;
            int bestmx = INT_MAX;
            for (int j = 1; j < n; ++j) {
                if (usedJ[j]) continue;
                vector<int> touched;
                int mx = 0;
                for (int sp : reps) {
                    int v = A[0] ^ sp ^ invA[j ^ sp];
                    if (valcnt[v] == 0) touched.push_back(v);
                    ++valcnt[v];
                    if (valcnt[v] > mx) mx = valcnt[v];
                }
                for (int v : touched) valcnt[v] = 0;
                if (mx < bestmx) {
                    bestmx = mx;
                    chosen = j;
                    if (mx == 1) break;
                }
            }
            forceScan = false;
        }

        if (chosen < 0) break;
        usedJ[chosen] = 1;
        int ans = ask(0, chosen);
        vector<int> nr;
        for (int sp : reps)
            if ((A[0] ^ sp ^ invA[chosen ^ sp]) == ans) nr.push_back(sp);
        if ((int)nr.size() == m) forceScan = true;
        reps.swap(nr);
    }

    int s = reps[0];
    vector<int> result(n);
    for (int i = 0; i < n; ++i) result[i] = A[i] ^ s;
    finish(answerCount, result);
    return 0;
}
