#include <bits/stdc++.h>
using namespace std;

// ---------------------------------------------------------------------------
// Coffee Varieties (hard version)
//
// The friend's memory is a queue S holding at most k values (positions count,
// so duplicate values occupy several slots).  Query c: answer Y iff a_c in S,
// then push a_c (pop front while |S| > k).  Reset R clears S (free, at most
// 30000 resets).  Only tastings are charged.
//
// Core idea
// ---------
// A "run" is a reset followed by queries of some indices in increasing order.
// Then every value sitting in the window when j is queried belongs to an index
// smaller than j, so an answer Y *proves* that j is not the first occurrence of
// its value.
//
// Certificate lemma: if every pair (f,j), f<j, of still unresolved indices has
// been compared, then every unresolved index is a first occurrence.
// Indeed, if a_j reappeared at some i<j, the first occurrence f of that value
// would be unresolved itself (a first occurrence can never be proven non-first)
// and the comparison of (f,j) would have answered Y.
//
// Hence it is enough to run the covering scheme on the current set U of
// unresolved indices ("compressed" blocks): the window then stays full of
// useful elements, a large win whenever many indices are already resolved.
// When a level finishes and |U| has at least halved, the plan is rebuilt on the
// smaller set (progress is guaranteed, so at most log2(n) rebuilds happen).
//
// Covering scheme over s = |U| indices
// -----------------------------------
// U (in increasing index order) is split into blocks of B consecutive elements
// (B a power of two, B <= k/2; chosen per plan by a cost estimate, in practice
// B = k/2, or B = 1 for k = 2).  If a run visits block Q right after block P,
// at most B-1 elements of Q were queried before the current one, so the
// window still contains all queried elements of P: every pair P x Q is
// compared (intra-block pairs are compared as well).  The window holds
// c = k/B blocks, so a block of a run is really compared with the previous
// c-1 blocks (with c blocks when B = 1, since then a block is one element).
//
// Covering all pairs of blocks with "levels":
//   * block distance t <= m/2: for every offset r in 0..t-1 run
//     r, r+t, r+2t, ...  (over the t offsets every block appears exactly once,
//     so a level costs s queries and distance-t pairs become consecutive).
//   * distance t > m/2: isolated pairs, one 2-block run (cost 2B).
// A level at distance t also covers distances 2t, ..., c'*t, so those distances
// need no own level; the level set is built greedily for that.
//
// Runs are skipped as soon as their later blocks have nothing left to learn,
// and indices proven non-first are never queried again.
//
// Special cases
//   k >= n : one increasing pass over everything (n queries, optimal).
//   k == 1 : one query compares only with the immediately preceding one, so an
//            answer identifies the exact pair and proves the larger index
//            non-first.  All still needed pairs are covered by greedy walks; a
//            pair stops being needed as soon as its larger index is proven
//            non-first, so the cost is (#needed pairs + #walks).
// ---------------------------------------------------------------------------

static int n, k;

static bool ask(int idx) {
    cout << "? " << idx << '\n';
    cout.flush();
    string s;
    if (!(cin >> s)) exit(0);
    if (s == "Y") return true;
    if (s == "N") return false;
    exit(0);  // "E" or a closed stream: terminate
}

static void answer(int d) {
    cout << "! " << d << '\n';
    cout.flush();
}

// ------------------------------- k == 1 ------------------------------------

namespace k1 {

static int N;
static vector<vector<char>> need;   // need[a][b], a<b: untested and b unresolved
static vector<int> degv;            // number of still needed pairs at a vertex
static vector<char> determined;
static long long pairsLeft;

static void mark_determined(int j) {
    if (determined[j]) return;
    determined[j] = 1;
    for (int i = 1; i < j; i++)
        if (need[i][j]) {
            need[i][j] = 0;
            pairsLeft--;
            degv[i]--;
            degv[j]--;
        }
}

static bool test_pair(int prev, int cur) {
    int a = min(prev, cur), b = max(prev, cur);
    bool y = ask(cur);
    if (need[a][b]) {
        need[a][b] = 0;
        pairsLeft--;
        degv[a]--;
        degv[b]--;
    }
    if (y) mark_determined(b);
    return y;
}

static void solve() {
    N = n;
    need.assign(N + 1, vector<char>(N + 1, 0));
    determined.assign(N + 1, 0);
    degv.assign(N + 1, 0);
    for (int i = 1; i <= N; i++)
        for (int j = i + 1; j <= N; j++) need[i][j] = 1;
    pairsLeft = (long long)N * (N - 1) / 2;
    for (int v = 1; v <= N; v++) degv[v] = N - 1;

    // 1) one increasing path 1,2,...,n: cheapest way to catch adjacent repeats
    if (pairsLeft > 0) {
        cout << "R\n";
        cout.flush();
        ask(1);
        int prev = 1;
        for (int v = 2; v <= N && pairsLeft > 0; v++) {
            test_pair(prev, v);
            prev = v;
        }
    }

    // 2) greedy walks over the pairs that are still needed
    while (pairsLeft > 0) {
        int start = -1;
        for (int v = 1; v <= N; v++)
            if (degv[v] > 0 && (degv[v] & 1)) { start = v; break; }
        if (start < 0)
            for (int v = 1; v <= N; v++)
                if (degv[v] > 0) { start = v; break; }
        if (start < 0) break;

        cout << "R\n";
        cout.flush();
        ask(start);
        int prev = start;
        while (pairsLeft > 0) {
            int best = -1, bestdeg = INT_MAX;
            for (int x = 1; x <= N; x++) {
                if (x == prev) continue;
                int a = min(prev, x), b = max(prev, x);
                if (!need[a][b]) continue;
                if (degv[x] < bestdeg) { bestdeg = degv[x]; best = x; }
            }
            if (best < 0) break;
            test_pair(prev, best);
            prev = best;
        }
    }

    int d = 0;
    for (int j = 1; j <= N; j++)
        if (!determined[j]) d++;
    answer(d);
}

}  // namespace k1

// ---------------------------- general case ---------------------------------

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> k)) return 0;

    if (n == 1) { answer(1); return 0; }

    if (k >= n) {
        int d = 0;
        for (int i = 1; i <= n; i++)
            if (!ask(i)) d++;
        answer(d);
        return 0;
    }

    if (k == 1) { k1::solve(); return 0; }

    vector<char> det(n + 1, 0);        // proven not a first occurrence
    int undetermined = n;

    // A plan for s unresolved indices:
    //   * block size B (a power of two dividing n, B <= k/2), m = ceil(s/B) blocks
    //     of consecutive unresolved indices,
    //   * the window holds c = k/B blocks; a run therefore compares a block with
    //     the previous min(c, ...) blocks of the run: distances 1..c-1 are
    //     fully compared (c fully, when B = 1 since then blocks are single
    //     elements).  So a level at distance t also covers distances
    //     2t, ..., c'*t and those distances need no own level.
    //   * the level set is built greedily from small to large distance.
    struct Plan {
        int B = 1;
        vector<int> levels;
        long long cost = 0;
    };

    auto build_plan = [&](long long s2) -> Plan {
        Plan best;
        best.B = 1;
        best.cost = LLONG_MAX;
        for (int cand = 1; cand <= k / 2 || cand == 1; cand *= 2) {
            if (cand > n) break;
            long long m2 = (s2 + cand - 1) / cand;
            if (m2 < 2) continue;
            int cp = (cand == 1) ? (int)(k / cand) : (int)(k / cand) - 1;
            vector<char> inS(m2 + 1, 0);
            vector<int> ls;
            for (int d = 1; d <= (int)m2 - 1; d++) {
                bool covered = false;
                for (int p = 2; p <= cp && p <= d; p++)
                    if (d % p == 0 && inS[d / p]) { covered = true; break; }
                if (!covered) { inS[d] = 1; ls.push_back(d); }
            }
            long long cost = 0;
            for (int d : ls) {
                if (d <= m2 / 2) {
                    cost += s2;                 // every block is visited once
                } else {
                    for (long long i = 0; i + d < m2; i++) {
                        cost += min(s2, (i + 1) * cand) - i * cand;
                        cost += min(s2, (i + d + 1) * cand) - (i + d) * cand;
                    }
                }
            }
            if (cost < best.cost) { best.B = cand; best.cost = cost; best.levels = std::move(ls); }
        }
        return best;
    };

    while (true) {
        // ---- current unresolved index set, in increasing order ----
        vector<int> U;
        U.reserve(undetermined);
        for (int i = 1; i <= n; i++)
            if (!det[i]) U.push_back(i);
        int s = (int)U.size();
        if (s <= 1) break;

        if (s <= k + 1) {
            // a single run compares every pair of the remaining indices
            cout << "R\n";
            cout.flush();
            for (int idx : U) {
                if (det[idx]) continue;
                if (ask(idx)) { det[idx] = 1; undetermined--; }
            }
            break;
        }

        Plan plan = build_plan(s);
        const int B = plan.B;
        const int m = (s + B - 1) / B;
        if (plan.levels.empty()) {          // defensive fallback: plain pass
            cout << "R\n";
            cout.flush();
            for (int idx : U) {
                if (det[idx]) continue;
                if (ask(idx)) { det[idx] = 1; undetermined--; }
            }
            break;
        }

        vector<int> cnt(m, 0);             // unresolved elements per block
        for (int t = 0; t < m; t++)
            cnt[t] = min(s, (t + 1) * B) - t * B;

        // ---- covering plan: runs grouped in levels ----
        vector<vector<int>> runs;
        vector<size_t> levelEnd;
        for (int st : plan.levels) {
            if (st <= m / 2) {
                for (int r = 0; r < st; r++) {
                    vector<int> run;
                    for (int b = r; b < m; b += st) run.push_back(b);
                    if (run.size() >= 2) runs.push_back(std::move(run));
                }
            } else {
                for (int i = 0; i + st < m; i++) runs.push_back({i, i + st});
            }
            levelEnd.push_back(runs.size());
        }

        bool restart = false;
        size_t executed = 0;
        for (size_t lv = 0; lv < levelEnd.size() && !restart; lv++) {
            while (executed < levelEnd[lv]) {
                const vector<int>& run = runs[executed++];
                bool useful = false;
                for (size_t t = 1; t < run.size(); t++)
                    if (cnt[run[t]]) { useful = true; break; }
                if (!useful) continue;

                cout << "R\n";
                cout.flush();
                for (size_t t = 0; t < run.size(); t++) {
                    int b = run[t];
                    int lo = b * B, hi = min(s, (b + 1) * B);
                    for (int u = lo; u < hi; u++) {
                        int idx = U[u];
                        if (det[idx]) continue;
                        if (ask(idx)) { det[idx] = 1; cnt[b]--; undetermined--; }
                    }
                    bool more = false;
                    for (size_t v = t + 1; v < run.size(); v++)
                        if (cnt[run[v]]) { more = true; break; }
                    if (!more) break;
                }
            }
            if ((long long)undetermined * 2 < s) restart = true;
        }
        if (restart) continue;             // rebuild blocks on the smaller set
        break;                             // covering finished
    }

    answer(undetermined);
    return 0;
}
