// Poetic Tournament: report the set of K strongest applicants among N using
// pairwise comparisons (interactive).
//
// Idea
// ----
// 1. Run a balanced knockout tournament (N-1 comparisons); the champion is the
//    strongest applicant.  For every element y keep the list ch[y] of the
//    elements that lost *directly* to y (in whatever tournament they met).
//
// 2. Invariant: right after extracting the t-th strongest element, the still
//    unextracted elements are exactly the union of the "pending subtrees"
//    rooted at the elements of one batch L, and every element of L is the
//    maximum of its own pending subtree.  Therefore the next strongest element
//    is max(L).
//
// 3. max(L) is found with a small knockout over L (|L|-1 comparisons).  Because
//    the losers are recorded in the ch[] lists, the leftovers become exactly
//    the batch for the following step:
//        L' = (elements that lost to max(L) inside the mini tournament)
//             + (main-tournament losers of max(L)).
//    The batches stay tiny (a handful of elements), so the whole extraction of
//    the remaining K-1 elements costs only a few hundred comparisons.
//
// 4. If K > N-K it is cheaper to extract the N-K weakest applicants with a
//    min-tournament and print the complement.
#include <bits/stdc++.h>
using namespace std;

static int N, K;
static bool findMax;              // true: extract strongest, false: weakest

static int askRaw(int a, int b) {
    printf("? %d %d\n", a, b);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);   // judge closed the stream
    return r;
}

// The element of {a,b} that advances in the current mode (stronger / weaker).
static inline int betterOf(int a, int b) {
    int w = askRaw(a, b);                // w = stronger of the two
    return findMax ? w : (a ^ b ^ w);
}

int main() {
    if (scanf("%d %d", &N, &K) != 2) return 0;

    if (K >= N) {                        // the answer is everything
        string out = "!";
        for (int i = 1; i <= N; i++) {
            out += ' ';
            out += to_string(i);
        }
        out += '\n';
        fwrite(out.data(), 1, out.size(), stdout);
        fflush(stdout);
        return 0;
    }

    findMax = (K <= N - K);
    const int want = findMax ? K : (N - K);   // how many elements to extract

    vector<vector<int>> ch(N + 1);

    // ---------- main tournament (N-1 comparisons) ----------
    vector<int> cur(N), nxt;
    iota(cur.begin(), cur.end(), 1);
    while (cur.size() > 1) {
        size_t m = cur.size();
        nxt.clear();
        nxt.reserve((m + 1) / 2);
        for (size_t i = 0; i + 1 < m; i += 2) {
            int adv = betterOf(cur[i], cur[i + 1]);
            int los = cur[i] ^ cur[i + 1] ^ adv;
            ch[adv].push_back(los);
            nxt.push_back(adv);
        }
        if (m & 1) nxt.push_back(cur.back());   // bye
        cur.swap(nxt);
    }

    int champ = cur[0];
    vector<int> ans;
    ans.reserve(want);
    ans.push_back(champ);

    // ---------- extract the remaining want-1 elements ----------
    vector<int> batch, nxtBatch;
    batch.swap(ch[champ]);                  // pending subtree maxima

    while ((int)ans.size() < want && !batch.empty()) {
        // maximum of the batch, via a knockout tournament
        while (batch.size() > 1) {
            size_t m = batch.size();
            nxtBatch.clear();
            nxtBatch.reserve((m + 1) / 2);
            for (size_t i = 0; i + 1 < m; i += 2) {
                int adv = betterOf(batch[i], batch[i + 1]);
                int los = batch[i] ^ batch[i + 1] ^ adv;
                ch[adv].push_back(los);
                nxtBatch.push_back(adv);
            }
            if (m & 1) nxtBatch.push_back(batch.back());   // bye
            batch.swap(nxtBatch);
        }
        int y = batch[0];
        ans.push_back(y);
        if ((int)ans.size() == want) break;
        batch.swap(ch[y]);                  // leftovers of y become the new batch
    }

    // ---------- output ----------
    string out = "!";
    if (findMax) {
        for (int x : ans) {
            out += ' ';
            out += to_string(x);
        }
    } else {
        vector<char> gone(N + 1, 0);
        for (int x : ans) gone[x] = 1;
        for (int i = 1; i <= N; i++)
            if (!gone[i]) {
                out += ' ';
                out += to_string(i);
            }
    }
    out += '\n';
    fwrite(out.data(), 1, out.size(), stdout);
    fflush(stdout);
    return 0;
}
