#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct Tree {
    int N;
    vector<vector<pair<int,ll>>> ch;   // (child, weight) left-to-right
    vector<ll> inw;                    // in-edge weight per original vertex
    int root = -1;
    // postorder-indexed
    vector<int> lft, sz;
    vector<ll> win;
};

static void buildPost(Tree& T) {
    int N = T.N;
    vector<int> post(N, -1);
    vector<int> ord; ord.reserve(N);
    T.lft.assign(N, 0);
    T.sz.assign(N, 0);
    T.win.assign(N, 0);
    vector<int> it(N, 0), st;
    st.reserve(N);
    st.push_back(T.root);
    while (!st.empty()) {
        int u = st.back();
        if (it[u] < (int)T.ch[u].size()) {
            int v = T.ch[u][it[u]].first;
            ++it[u];
            st.push_back(v);
        } else {
            int idx = (int)ord.size();
            ord.push_back(u);
            post[u] = idx;
            int ls = idx, s = 1;
            for (auto& e : T.ch[u]) {
                int c = e.first;
                ls = min(ls, T.lft[post[c]]);
                s += T.sz[post[c]];
            }
            T.lft[idx] = ls;
            T.sz[idx] = s;
            T.win[idx] = T.inw[u];
            st.pop_back();
        }
    }
    T.root = post[T.root];
}

static Tree readTree() {
    Tree T;
    int N;
    if (scanf("%d", &N) != 1) exit(1);
    T.N = N;
    T.ch.assign(N, {});
    T.inw.assign(N, 0);
    vector<char> hasPar(N, 0);
    for (int i = 0; i < N; i++) {
        int k;
        if (scanf("%d", &k) != 1) exit(1);
        T.ch[i].reserve(k);
        for (int j = 0; j < k; j++) {
            int v; ll w;
            if (scanf("%d %lld", &v, &w) != 2) exit(1);
            --v;
            T.ch[i].push_back({v, w});
            hasPar[v] = 1;
            T.inw[v] = w;
        }
    }
    for (int i = 0; i < N; i++) if (!hasPar[i]) { T.root = i; break; }
    buildPost(T);
    return T;
}

int main() {
    ll c1, c2, c3;
    if (scanf("%lld %lld %lld", &c1, &c2, &c3) != 3) return 0;

    Tree A = readTree();
    Tree B = readTree();
    const int n = A.N, m = B.N;
    const int rootA = A.root, rootB = B.root;

    // base cost: contract every non-root node of A, insert every non-root node of B
    ll base = 0;
    for (int i = 0; i < n; i++) if (i != rootA) base += c2 * A.win[i];
    for (int j = 0; j < m; j++) if (j != rootB) base += c1 * B.win[j];

    // keyroots: for each distinct leftmost-leaf value, the node with maximal postorder index
    auto keyroots = [](const Tree& T) {
        vector<int> best(T.N, -1), kr;
        for (int i = 0; i < T.N; i++) best[T.lft[i]] = i;
        for (int i = 0; i < T.N; i++) if (best[T.lft[i]] == i) kr.push_back(i);
        return kr;
    };
    vector<int> krA = keyroots(A), krB = keyroots(B);

    const size_t PM = (size_t)m + 1;             // padded stride so row pointers stay in-bounds
    vector<ll> P((size_t)(n + 1) * PM, 0);       // max savings matching subtree(i) <-> subtree(j)
    vector<ll> fd((size_t)(n + 1) * (m + 1));    // scratch forest DP table

    vector<int> colJump(m + 2);
    vector<char> specCol(m + 2);

    for (int j1 : krB) {
        const int l2 = B.lft[j1];
        const int W = j1 - l2 + 2;               // relative cols 0..W-1, col 0 = empty
        for (int b = 1; b < W; b++) {
            const int j = l2 + b - 1;
            colJump[b] = B.lft[j] - l2;
            specCol[b] = (B.lft[j] == l2);
        }
        const bool forbidIns = (j1 == rootB);    // last column j == rootB cannot be inserted
        const int blast = forbidIns ? W - 1 : W; // normal columns are [1, blast), tail is b=W-1

        for (int i1 : krA) {
            const int l1 = A.lft[i1];
            const int H = i1 - l1 + 2;           // relative rows 0..H-1, row 0 = empty
            ll* tab = fd.data();
            memset(tab, 0, sizeof(ll) * W);      // row 0 = all zeros
            for (int a = 1; a < H; a++) {
                const int i = l1 + a - 1;
                const bool forbidDel = (i == rootA);  // contracting the root of A is forbidden
                ll* cur  = tab + (size_t)a * W;
                ll* prev = cur - W;
                ll* jrow = cur - (size_t)A.sz[i] * W;        // row (lft[i]-1)
                ll* Prow = &P[(size_t)(i + 1) * PM + l2 - 1]; // Prow[b] = P[i][l2+b-1]
                const bool specRow = (A.lft[i] == l1);
                const ll wi = A.win[i];
                cur[0] = 0;
                ll run = 0;

                if (!forbidDel) {
                    if (specRow) {
                        for (int b = 1; b < blast; b++) {
                            ll v;
                            if (specCol[b]) {
                                const ll wj = B.win[l2 + b - 1];
                                const ll sav = c2*wi + c1*wj - c3*(wi > wj ? wi - wj : wj - wi);
                                v = prev[b-1] + sav;
                                if (prev[b] > v) v = prev[b];
                                if (run > v) v = run;
                                Prow[b] = v;
                            } else {
                                v = prev[b];
                                const ll u = jrow[colJump[b]] + Prow[b];
                                if (u > v) v = u;
                                if (run > v) v = run;
                            }
                            cur[b] = v; run = v;
                        }
                    } else {
                        for (int b = 1; b < blast; b++) {
                            ll v = prev[b];
                            const ll u = jrow[colJump[b]] + Prow[b];
                            if (u > v) v = u;
                            if (run > v) v = run;
                            cur[b] = v; run = v;
                        }
                    }
                    if (forbidIns) {   // tail column: no insert transition, no run
                        const int b = W - 1;
                        ll v;
                        if (specRow && specCol[b]) {
                            const ll wj = B.win[l2 + b - 1];
                            const ll sav = c2*wi + c1*wj - c3*(wi > wj ? wi - wj : wj - wi);
                            v = prev[b-1] + sav;
                            if (prev[b] > v) v = prev[b];
                            Prow[b] = v;
                        } else {
                            v = prev[b];
                            const ll u = jrow[colJump[b]] + Prow[b];
                            if (u > v) v = u;
                        }
                        cur[b] = v;
                    }
                } else {
                    // last row: "delete i" forbidden, and this row's P is not meaningful for jumps;
                    // it only needs to produce the root cell value (and special-cell P values).
                    if (specRow) {
                        for (int b = 1; b < blast; b++) {
                            ll v;
                            if (specCol[b]) {
                                const ll wj = B.win[l2 + b - 1];
                                const ll sav = c2*wi + c1*wj - c3*(wi > wj ? wi - wj : wj - wi);
                                v = prev[b-1] + sav;
                                if (run > v) v = run;
                                Prow[b] = v;
                            } else {
                                v = jrow[colJump[b]] + Prow[b];
                                if (run > v) v = run;
                            }
                            cur[b] = v; run = v;
                        }
                    } else {
                        for (int b = 1; b < blast; b++) {
                            ll v = jrow[colJump[b]] + Prow[b];
                            if (run > v) v = run;
                            cur[b] = v; run = v;
                        }
                    }
                    if (forbidIns) {
                        const int b = W - 1;
                        ll v;
                        if (specRow && specCol[b]) {
                            const ll wj = B.win[l2 + b - 1];
                            const ll sav = c2*wi + c1*wj - c3*(wi > wj ? wi - wj : wj - wi);
                            v = prev[b-1] + sav;
                            Prow[b] = v;
                        } else {
                            v = jrow[colJump[b]] + Prow[b];
                        }
                        cur[b] = v;
                    }
                }
            }
        }
    }

    ll best = P[(size_t)(rootA + 1) * PM + rootB];
    printf("%lld\n", base - best);
    return 0;
}
