#include <bits/stdc++.h>
using namespace std;

// ---------------------------------------------------------------------------
// Interactive solution for "Automatized Mineral Classification".
//
// The belt behaves as a FIFO queue: "+ id" appends the mineral at the back,
// "-" removes the oldest one at the front, and after every operation the
// judge reports the number of distinct classes currently on the belt.
//
// Phase 1 (2N operations)
//   Push 1..N in order.  A push that increases the distinct count introduces
//   the first occurrence ("leader") of a new class, so the leaders form a set
//   of representatives, one per class.  Then pop everything.  When the i-th
//   pushed mineral is popped, the count drops exactly when it was the last
//   occurrence of its class; in particular a leader whose pop does not drop
//   the count is the leader of a class that also contains some non-leader.
//   Hence we learn, for free, which leaders actually need non-leaders.
//
// Phase 2
//   Assign each non-leader to its class with a divide & conquer:
//   presence(S, Y) pushes the elements of S and then the leaders of Y; each
//   leader of Y increments the count iff its class does not occur in S, so
//   one pass of cost 2*(|S|+|Y|) reveals exactly which classes of Y occur in
//   S.  Splitting S in halves recursively localises every class.
//   While S is being pushed we also obtain, for free, its first occurrences
//   (one element per class, hence pairwise distinct classes).  When those are
//   known a "layer" step may beat the balanced split: match that transversal
//   to its classes and recurse only on the remaining duplicates.
//
//   For a node with s elements and p candidate classes, if p is tiny and s is
//   not, directly scanning the candidates (keeping the probe element on the
//   belt) is cheaper than the recursion, so that base case is used there.
// ---------------------------------------------------------------------------

// ------------------------------- I/O layer -------------------------------
#ifdef SIMULATE
int  simReadN();
int  simPush(int id);
int  simPop();
void simAnswer(const vector<vector<int>> &classes);
static inline int  ioReadN() { return simReadN(); }
static inline int  ioPush(int id) { return simPush(id); }
static inline int  ioPop() { return simPop(); }
static inline void ioAnswer(const vector<vector<int>> &c) { simAnswer(c); }
#else
static inline int ioReadN() {
    int n;
    if (scanf("%d", &n) != 1) exit(0);
    return n;
}
static inline int ioPush(int id) {
    printf("+ %d\n", id);
    fflush(stdout);
    int c;
    if (scanf("%d", &c) != 1) exit(0);
    return c;
}
static inline int ioPop() {
    printf("-\n");
    fflush(stdout);
    int c;
    if (scanf("%d", &c) != 1) exit(0);
    return c;
}
static inline void ioAnswer(const vector<vector<int>> &classes) {
    size_t total = 0;
    for (const auto &v : classes) total += v.size();
    string out;
    out.reserve(8 * total + 16);
    out += "! " + to_string(classes.size()) + "\n";
    for (const auto &v : classes) {
        out += to_string(v.size());
        for (int x : v) { out += ' '; out += to_string(x); }
        out += '\n';
    }
    fwrite(out.data(), 1, out.size(), stdout);
    fflush(stdout);
}
#endif

// ---------------------------- belt bookkeeping ---------------------------
static int curCount = 0;   // number of distinct classes currently on the belt

static inline int pushElem(int id) {
    int c = ioPush(id);
    curCount = c;
    return c;
}
static inline int popElem() {
    int c = ioPop();
    curCount = c;
    return c;
}

// ------------------------------- solution --------------------------------
static int N;
static vector<int> assignment_;   // assignment_[x] = leader of x's class

// Ask which classes represented in Y occur among the elements of S.
// Precondition: S and Y are disjoint, elements of Y represent pairwise
// distinct classes, and every class occurring in S is represented in Y.
// Cost: 2*(|S| + |Y|) belt operations.
// If firstOut is not null, it receives the elements of S that are the first
// occurrence of their class inside S (one per class of presence(S)); this
// information comes for free while S is being pushed.
static vector<int> presence(const vector<int> &S, const vector<int> &Y,
                            vector<int> *firstOut = nullptr) {
    vector<int> res;
    int before = 0;
    for (int x : S) {
        int prev = before;
        int c = pushElem(x);
        if (firstOut && c > prev) firstOut->push_back(x);
        before = c;
    }
    for (int y : Y) {
        int c = pushElem(y);
        if (c == before) res.push_back(y);
        before = c;
    }
    int tot = (int)S.size() + (int)Y.size();
    for (int i = 0; i < tot; i++) popElem();
    return res;
}

static mt19937 rng(712367821u);

// Assign every element of S by testing the candidates of Y one after another.
// A single probe element stays on the belt while candidates are cycled through
// the other belt position; a candidate whose class equals the probe's class
// does not increase the distinct count.  Cost about |S|*(|Y|+3).
static void scanAssign(const vector<int> &S, const vector<int> &Y) {
    vector<int> order = Y;
    shuffle(order.begin(), order.end(), rng);
    for (int x : S) {
        int before = pushElem(x);
        int pushed = 1;
        int match = -1;
        for (int idx = 0; idx < (int)order.size(); idx++) {
            int y = order[idx];
            int c = pushElem(y);
            pushed++;
            if (c == before) {
                match = y;
                if (idx > 0)
                    rotate(order.begin(), order.begin() + idx,
                           order.begin() + idx + 1);   // move-to-front
                break;
            }
            before = c;
        }
        for (int i = 0; i < pushed; i++) popElem();
        assignment_[x] = match;
    }
}

// Match the pairwise-distinct elements of F to their classes in Y.  It is
// guaranteed that |F| = |Y| and that classes(F) = classes(Y).
static void matchAssign(const vector<int> &F, const vector<int> &Y) {
    if (F.size() == 1) {
        assignment_[F[0]] = Y[0];
        return;
    }
    if (F.size() == 2) {
        // one cheap query decides the whole 2-element permutation
        vector<int> one{Y[0]};
        vector<int> first{F[0]};
        bool same = !presence(first, one).empty();
        assignment_[F[0]] = same ? Y[0] : Y[1];
        assignment_[F[1]] = same ? Y[1] : Y[0];
        return;
    }
    int half = (int)F.size() / 2;
    vector<int> F1(F.begin(), F.begin() + half);
    vector<int> F2(F.begin() + half, F.end());
    vector<int> Y1 = presence(F1, Y);
    vector<char> in1(N + 1, 0);
    for (int y : Y1) in1[y] = 1;
    vector<int> Y2;
    Y2.reserve(Y.size() - Y1.size());
    for (int y : Y)
        if (!in1[y]) Y2.push_back(y);
    matchAssign(F1, Y1);
    matchAssign(F2, Y2);
}

// Direct candidate scanning is used only for very few candidates.
static const int SCAN_PMAX = 6;    // at most this many candidate classes
static const int SCAN_SMIN = 8;    // and at least this many elements

// Upper estimate of the divide & conquer cost for s elements and p classes.
static double estimateRec(int s, int p) {
    double est = 0;
    long long q = s, P = p;
    while (q > 1) {
        est += 2.0 * ((double)s + 2.0 * (double)P);
        q = (q + 1) / 2;
        P = min((long long)s, P * 2);
    }
    return est;
}

static double matchCost(int p) {
    return p <= 1 ? 0.0 : 3.0 * (double)p * log2((double)p);
}

static void solve(vector<int> S, vector<int> Y, bool hasFirst = false,
                  vector<int> first = vector<int>()) {
    if (S.empty()) return;
    if ((int)Y.size() == 1) {
        for (int x : S) assignment_[x] = Y[0];
        return;
    }
    if ((int)S.size() == 1) {
        assignment_[S[0]] = Y[0];
        return;
    }
    const int s = (int)S.size();
    const int p = (int)Y.size();
    if (p <= SCAN_PMAX && s >= SCAN_SMIN) {
        scanAssign(S, Y);
        return;
    }

    // "Layer" step: peel off the first occurrences of S (one element per
    // class, so pairwise distinct), match them to their classes and recurse on
    // the remaining duplicates.  The first occurrences are already known for
    // free, which makes this cheaper than a balanced split in some cases.
    if (hasFirst) {
        const int m = s - p;   // number of elements in S \ first
        double layerEst = matchCost(p) + 2.0 * ((double)m + (double)p) +
                          estimateRec(m, min(p, m));
        if (layerEst < estimateRec(s, p)) {
            matchAssign(first, Y);
            vector<int> mid;   // S \ first, order preserved
            {
                vector<char> isFirst(N + 1, 0);
                for (int x : first) isFirst[x] = 1;
                mid.reserve(m);
                for (int x : S)
                    if (!isFirst[x]) mid.push_back(x);
            }
            if (mid.empty()) return;   // every element was already matched
            vector<int> midFirst;
            vector<int> midClasses = presence(mid, Y, &midFirst);
            solve(std::move(mid), std::move(midClasses), true,
                  std::move(midFirst));
            return;
        }
    }

    const int half = s / 2;
    vector<int> S1(S.begin(), S.begin() + half);
    vector<int> S2(S.begin() + half, S.end());
    vector<int> first1;
    vector<int> p1 = presence(S1, Y, &first1);

    vector<char> in1(N + 1, 0);
    for (int y : p1) in1[y] = 1;

    vector<int> p2;
    bool hasFirst2 = false;
    vector<int> first2;
    if ((int)S2.size() == p - (int)p1.size()) {
        // Every class of Y not occurring in S1 has to occur in S2 and S2 has
        // exactly that many elements, so presence(S2) = Y \ presence(S1).
        p2.reserve(p - p1.size());
        for (int y : Y)
            if (!in1[y]) p2.push_back(y);
    } else {
        // presence(S2) = (Y \ p1) union (p1 intersect presence(S2)); the
        // second part is obtained by pushing S2 followed by the leaders of p1,
        // which is never more expensive than pushing all of Y.
        vector<int> span;
        if (!p1.empty()) {
            span = presence(S2, p1, &first2);
            hasFirst2 = true;
        }
        vector<char> inspan(N + 1, 0);
        for (int y : span) inspan[y] = 1;
        p2.reserve(Y.size());
        for (int y : Y)
            if (!in1[y] || inspan[y]) p2.push_back(y);
    }

    solve(std::move(S1), std::move(p1), true, std::move(first1));
    solve(std::move(S2), std::move(p2), hasFirst2, std::move(first2));
}

int mainSolution() {
    N = ioReadN();
    assignment_.assign(N + 1, 0);

    // ---- Phase 1: leaders and "leader of a class with duplicates" ----
    vector<char> isLeader(N + 1, 0), isLast(N + 1, 0);
    int prev = 0;
    for (int i = 1; i <= N; i++) {
        int c = pushElem(i);
        isLeader[i] = (c > prev);
        prev = c;
    }
    for (int i = 1; i <= N; i++) {
        int c = popElem();
        isLast[i] = (c < prev);
        prev = c;
    }

    vector<int> candidates;   // leaders of classes having at least one duplicate
    vector<int> nonLeaders;
    for (int i = 1; i <= N; i++) {
        if (isLeader[i]) {
            if (!isLast[i]) candidates.push_back(i);
        } else {
            nonLeaders.push_back(i);
        }
    }

    // ---- Phase 2: assign non-leaders ----
    if (!nonLeaders.empty()) {
        if ((int)candidates.size() == 1) {
            for (int x : nonLeaders) assignment_[x] = candidates[0];
        } else {
            solve(nonLeaders, candidates);
        }
    }

    // ---- output the partition ----
    vector<int> idxOf(N + 1, -1);
    vector<vector<int>> classes;
    for (int i = 1; i <= N; i++) {
        if (isLeader[i]) {
            idxOf[i] = (int)classes.size();
            classes.push_back({i});
        }
    }
    for (int x : nonLeaders) classes[idxOf[assignment_[x]]].push_back(x);
    ioAnswer(classes);
    return 0;
}

#ifndef SIMULATE
int main() { return mainSolution(); }
#endif
