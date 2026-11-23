#include <bits/stdc++.h>
using namespace std;
// moves: 0=R 1=P 2=S ; x beats y iff x==(y+1)%3
struct Hyp { int type, a, b, c; }; // generic deterministic strategy
static vector<int> P; // player history
static int aiMove(const Hyp& h, const vector<int>& p, int t) {
    // compute AI move at round t given player moves p[0..t-1]
    switch (h.type) {
    case 0: return h.a;                                  // constant
    case 1: return (h.a + h.b * t) % 3;                  // cycle
    case 2: return t == 0 ? h.a : (p[t - 1] + h.b) % 3;  // react to player's previous
    case 3: {                                            // outcome-based shift of own move
        int a = h.a;
        for (int i = 1; i <= t; i++) {
            int prev = a, pp = p[i - 1];
            int res = (prev == (pp + 1) % 3) ? 0 : (prev == pp ? 1 : 2); // ai win/tie/loss
            int k = res == 0 ? h.b : (res == 1 ? h.c : h.c / 3);
            (void)k;
            int sh = (res == 0) ? (h.b) : (res == 1 ? (h.c % 3) : (h.c / 3));
            a = (prev + sh) % 3;
        }
        return a;
    }
    case 4: { // beat player's most frequent move (ties -> earliest of R,P,S), first move a
        if (t == 0) return h.a;
        int cnt[3] = {0, 0, 0};
        for (int i = 0; i < t; i++) cnt[p[i]]++;
        int m = 0;
        for (int i = 1; i < 3; i++) if (cnt[i] > cnt[m]) m = i;
        return (m + h.b) % 3;
    }
    case 5: { // react to player's last move with offset depending on two-back? : beat last-last
        if (t < 2) return t == 0 ? h.a : h.c;
        return (p[t - 2] + h.b) % 3;
    }
    case 6: { // periodic sequence, period a, pattern code b (base 3)
        int i = t % h.a, c = h.b;
        while (i--) c /= 3;
        return c % 3;
    }
    case 7: { // arbitrary function of player's previous move: first a, map code b
        if (t == 0) return h.a;
        int c = h.b, i = p[t - 1];
        while (i--) c /= 3;
        return c % 3;
    }
    }
    return 0;
}
static vector<Hyp> H;
static long long nodeBudget;
// returns {sum cost, max cost}
static vector<double> Wt;
static pair<double,int> solve(const vector<int>& S, vector<int>& p, int streak, int depth, int& bestMove) {
    int n = S.size();
    bestMove = 0;
    if (n == 0) return {0, 0};
    if (streak >= 10) return {0, 0};
    double wsum = 0; for (int i : S) wsum += Wt[i];
    if (n == 1) { bestMove = (aiMove(H[S[0]], p, p.size()) + 1) % 3; return {Wt[S[0]] * (10 - streak), 10 - streak}; }
    if (depth == 0 || --nodeBudget < 0) {
        int est = 10 - streak + (int)ceil(log2((double)n)) + 1;
        return {wsum * est, est};
    }
    int t = p.size();
    vector<int> pred(n);
    for (int i = 0; i < n; i++) pred[i] = aiMove(H[S[i]], p, t);
    pair<double,int> best = {1e18, INT_MAX};
    for (int x = 0; x < 3; x++) {
        vector<int> W, L;
        for (int i = 0; i < n; i++) (x == (pred[i] + 1) % 3 ? W : L).push_back(S[i]);
        if (W.empty()) continue; // pure loss with no info: never useful (heuristic)
        p.push_back(x);
        int dm;
        auto cw = solve(W, p, streak + 1, depth - 1, dm);
        pair<double,int> cl = {0, 0};
        if (!L.empty()) cl = solve(L, p, 0, depth - 1, dm);
        p.pop_back();
        pair<double,int> c = {wsum + cw.first + cl.first, 1 + max(cw.second, cl.second)};
        if (c.first < best.first - 1e-9 || (c.first < best.first + 1e-9 && c.second < best.second)) { best = c; bestMove = x; }
    }
    if (best.first >= 1e17) { // all moves lose for everyone? impossible, but guard
        return {wsum * 100, 100};
    }
    return best;
}
int main() {
    // tier 1: the six strategies we expect (weight 1) plus close variants (small weight)
    vector<Hyp> core = {{0,0,0,0},{0,1,0,0},{0,2,0,0},{1,0,1,0},{2,0,0,0},{2,0,1,0}};
    vector<Hyp> var1;
    for (int s = 0; s < 3; s++) for (int d = 1; d < 3; d++) var1.push_back({1, s, d, 0});
    for (int f = 0; f < 3; f++) for (int k = 0; k < 3; k++) var1.push_back({2, f, k, 0});
    vector<Hyp> broad;
    for (int f = 0; f < 3; f++) for (int b = 0; b < 3; b++) for (int c = 0; c < 9; c++) broad.push_back({3, f, b, c});
    for (int f = 0; f < 3; f++) for (int k = 0; k < 3; k++) broad.push_back({4, f, k, 0});
    for (int f = 0; f < 3; f++) for (int g = 0; g < 3; g++) for (int k = 0; k < 3; k++) broad.push_back({5, f, k, g});
    for (int L = 2, pw = 9; L <= 6; L++, pw *= 3) for (int c = 0; c < pw; c++) broad.push_back({6, L, c, 0});
    for (int f = 0; f < 3; f++) for (int c = 0; c < 27; c++) broad.push_back({7, f, c, 0});
    // behavioral fingerprints for dedup
    vector<vector<int>> seqs;
    {
        mt19937 g(777);
        for (int k = 0; k < 40; k++) { vector<int> q(30); for (auto& v : q) v = g() % 3; seqs.push_back(q); }
    }
    auto fp = [&](const Hyp& h) {
        string r;
        for (auto& q : seqs) for (int t = 0; t < 30; t++) r += char('0' + aiMove(h, q, t));
        return r;
    };
    set<string> seen;
    vector<int> S1, S2;
    auto add = [&](const Hyp& h, double w, vector<int>* S) {
        string f = fp(h);
        if (!seen.insert(f).second) return;
        H.push_back(h); Wt.push_back(w);
        if (S) S->push_back(H.size() - 1);
    };
    for (auto& h : core) add(h, 1.0, &S1);
    for (auto& h : var1) add(h, 0.05, &S1);
    for (auto& h : broad) add(h, 1.0, &S2);
    const char* M = "RPS";
    int streak = 0;
    while (streak < 10) {
        if (S1.empty() && S2.empty()) {
            // nothing fits the whole history: restart modelling from this round on
            P.clear();
            for (int i = 0; i < (int)H.size(); i++) S2.push_back(i);
        }
        int t = P.size();
        int x;
        vector<int>* cur = !S1.empty() ? &S1 : &S2;
        {
            // collapse behaviorally-identical classes is implicit
            nodeBudget = 400000;
            int depth = cur->size() <= 30 ? 14 : 4;
            solve(*cur, P, streak, depth, x);
        }
        printf("%c\n", M[x]);
        fflush(stdout);
        char buf[64];
        if (scanf("%63s", buf) != 1) return 0;
        string r = buf;
        bool win;
        if (r == "player") win = true;
        else if (r == "ai") win = false;
        else return 0;
        auto filt = [&](vector<int>& S) {
            vector<int> N;
            for (int i : S) {
                int a = aiMove(H[i], P, t);
                if ((x == (a + 1) % 3) == win) N.push_back(i);
            }
            S = N;
        };
        filt(S1); filt(S2);
        P.push_back(x);
        streak = win ? streak + 1 : 0;
    }
    return 0;
}
