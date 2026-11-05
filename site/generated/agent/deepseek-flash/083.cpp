#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<int> fa_, dep_;
static vector<int> tin_, tout_, byTin_;
static vector<array<int, 2>> ch_;
static vector<char> alive_;
static vector<vector<int>> up_;
static int LOGN_;

// Euler tour (iterative) so that subtree(a) == [tin[a], tout[a]] in preorder
static void buildEuler() {
    int timer = 0;
    vector<int> stk;
    stk.reserve(n);
    stk.push_back(1);
    vector<int> it(n + 1, 0);
    tin_.assign(n + 1, 0);
    tout_.assign(n + 1, 0);
    byTin_.assign(n, 0);
    tin_[1] = timer;
    byTin_[timer] = 1;
    timer++;
    while (!stk.empty()) {
        int v = stk.back();
        if (it[v] < 2) {
            int c = ch_[v][it[v]++];
            if (c != -1) {
                tin_[c] = timer;
                byTin_[timer] = c;
                timer++;
                stk.push_back(c);
            }
        } else {
            tout_[v] = timer - 1;
            stk.pop_back();
        }
    }
}

static void buildJump() {
    LOGN_ = 1;
    while ((1 << LOGN_) <= n) LOGN_++;
    up_.assign(LOGN_, vector<int>(n + 1, 1));
    for (int i = 1; i <= n; i++) up_[0][i] = (i == 1 ? 1 : fa_[i]);
    for (int k = 1; k < LOGN_; k++)
        for (int i = 1; i <= n; i++) up_[k][i] = up_[k - 1][up_[k - 1][i]];
}

static int lcaUp(int u, int v) {
    if (dep_[u] < dep_[v]) swap(u, v);
    int diff = dep_[u] - dep_[v];
    for (int k = 0; diff; k++, diff >>= 1)
        if (diff & 1) u = up_[k][u];
    if (u == v) return u;
    for (int k = LOGN_ - 1; k >= 0; k--)
        if (up_[k][u] != up_[k][v]) { u = up_[k][u]; v = up_[k][v]; }
    return up_[0][u];
}

static inline int ceilLog2(long long t) {
    int lb = 0;
    while ((1LL << lb) < t) lb++;
    return lb;
}

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    const int SLACK = 2;
    while (T--) {
        if (scanf("%d", &n) != 1) return 0;
        fa_.assign(n + 1, 0);
        dep_.assign(n + 1, 0);
        ch_.assign(n + 1, array<int, 2>{-1, -1});
        for (int i = 2; i <= n; i++) {
            if (scanf("%d", &fa_[i]) != 1) return 0;
            int p = fa_[i];
            if (ch_[p][0] == -1) ch_[p][0] = i;
            else ch_[p][1] = i;
        }
        dep_[1] = 0;
        for (int i = 2; i <= n; i++) dep_[i] = dep_[fa_[i]] + 1;
        int H = 0;
        for (int i = 1; i <= n; i++) H = max(H, dep_[i]);

        vector<int> lvl(H + 1, 0);
        for (int i = 1; i <= n; i++) lvl[dep_[i]]++;

        alive_.assign(n + 1, 1);
        int aliveCnt = n;
        int lo = 0, hi = H;

        bool eulerBuilt = false;

        while (aliveCnt > 1) {
            if (lo < hi) {
                // ---- Phase A : the live set is exactly all vertices with depth in [lo,hi]
                long long tot = 0;
                for (int d = lo; d <= hi; d++) tot += lvl[d];
                long long pre = 0, bestVal = LLONG_MAX;
                int bestK = lo;
                for (int k = lo; k < hi; k++) {
                    pre += lvl[k];
                    long long v = max(pre, tot - pre);
                    if (v < bestVal) { bestVal = v; bestK = k; }
                }
                printf("? 1 %d\n", bestK);
                fflush(stdout);
                int o;
                if (scanf("%d", &o) != 1) return 0;
                if (o == 1) hi = bestK; else lo = bestK + 1;
                aliveCnt = 0;
                for (int i = 1; i <= n; i++) {
                    if (alive_[i] && (dep_[i] < lo || dep_[i] > hi)) alive_[i] = 0;
                    aliveCnt += alive_[i];
                }
            } else {
                // ---- Phase B : all live vertices are at depth lo
                int d = lo;
                if (!eulerBuilt) { buildEuler(); buildJump(); eulerBuilt = true; }
                // C[a] = number of live vertices inside subtree(a)
                vector<int> C(n + 1, 0);
                for (int i = n; i >= 1; i--) {
                    C[i] += alive_[i] ? 1 : 0;
                    if (i > 1) C[fa_[i]] += C[i];
                }
                int tot = C[1];
                // Hh[v] = number of branching levels on the deepest live path below v:
                // the cost of identifying X by pure descent from v.
                vector<int> Hh(n + 1, 0);
                for (int i = n; i >= 1; i--) {
                    int c0 = ch_[i][0], c1 = ch_[i][1];
                    int a0 = (c0 >= 0) ? C[c0] : 0, a1 = (c1 >= 0) ? C[c1] : 0;
                    if (a0 > 0 && a1 > 0) Hh[i] = 1 + max(Hh[c0], Hh[c1]);
                    else if (a0 > 0) Hh[i] = Hh[c0];
                    else if (a1 > 0) Hh[i] = Hh[c1];
                    else Hh[i] = 0;
                }
                // live vertices in preorder; mn/mx = rank range of the live set of each subtree
                vector<int> ord;
                ord.reserve(tot);
                for (int t = 0; t < n; t++)
                    if (alive_[byTin_[t]]) ord.push_back(byTin_[t]);
                vector<int> mn(n + 1, INT_MAX), mx(n + 1, -1);
                for (int i = 0; i < tot; i++) mn[ord[i]] = mx[ord[i]] = i;
                for (int i = n; i >= 2; i--) {
                    int p = fa_[i];
                    if (mn[i] < mn[p]) mn[p] = mn[i];
                    if (mx[i] > mx[p]) mx[p] = mx[i];
                }
                int bestA = -1;
                long long bestScore = LLONG_MAX, bestBal = LLONG_MAX;
                for (int a = 1; a <= n; a++) {
                    if (dep_[a] > d) continue;
                    int cnt = C[a];
                    if (cnt == 0 || cnt == tot) continue;
                    int l = mn[a], r = mx[a];
                    // identify X inside subtree(a): cost ~ min(descent height, log2(cnt))
                    int estY = min(Hh[lcaUp(ord[l], ord[r])], ceilLog2(cnt) + SLACK);
                    // identify X outside subtree(a): the live set is a prefix + a suffix
                    int oL = (l > 0) ? ord[0] : ord[r + 1];
                    int oR = (r + 1 < tot) ? ord[tot - 1] : ord[l - 1];
                    int estN = min(Hh[lcaUp(oL, oR)], ceilLog2(tot - cnt) + SLACK);
                    long long sc = max(estY, estN);
                    long long bal = max(cnt, tot - cnt);
                    if (sc < bestScore || (sc == bestScore && bal < bestBal)) {
                        bestScore = sc;
                        bestBal = bal;
                        bestA = a;
                    }
                }
                int rad = d - dep_[bestA];
                printf("? %d %d\n", bestA, rad);
                fflush(stdout);
                int o;
                if (scanf("%d", &o) != 1) return 0;
                aliveCnt = 0;
                for (int i = 1; i <= n; i++) {
                    if (!alive_[i]) continue;
                    bool inside = (tin_[bestA] <= tin_[i] && tin_[i] <= tout_[bestA]);
                    if (inside != (o == 1)) alive_[i] = 0;
                    aliveCnt += alive_[i];
                }
            }
        }
        int ans = -1;
        for (int i = 1; i <= n; i++) if (alive_[i]) { ans = i; break; }
        printf("! %d\n", ans);
        fflush(stdout);
    }
    return 0;
}
