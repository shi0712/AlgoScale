#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll INF = 4'000'000'000'000'000'000LL;

struct TreeInfo {
    int N;
    int rootOriginal;
    int rootPo;
    int timer;

    vector<int> po;        // original vertex -> postorder index
    vector<int> L;         // postorder index -> leftmost postorder index of subtree
    vector<int> weight;    // postorder index -> incoming edge weight
    vector<int> keyroots;  // postorder indices, increasing
};

TreeInfo readTree() {
    int N;
    cin >> N;

    vector<vector<pair<int, int>>> children(N + 1);
    vector<int> parent(N + 1, -1);
    vector<int> idxInParent(N + 1, -1);
    vector<int> wIn(N + 1, 0);
    vector<int> hasParent(N + 1, 0);

    for (int i = 1; i <= N; ++i) {
        int k;
        cin >> k;
        children[i].reserve(k);
        for (int t = 0; t < k; ++t) {
            int v, w;
            cin >> v >> w;
            children[i].push_back({v, w});
            parent[v] = i;
            idxInParent[v] = t;
            wIn[v] = w;
            hasParent[v] = 1;
        }
    }

    int root = -1;
    for (int i = 1; i <= N; ++i) {
        if (!hasParent[i]) {
            root = i;
            break;
        }
    }

    TreeInfo T;
    T.N = N;
    T.rootOriginal = root;
    T.timer = 0;
    T.po.assign(N + 1, 0);
    T.L.assign(N + 1, 0);
    T.weight.assign(N + 1, 0);

    function<int(int)> dfs = [&](int v) -> int {
        int first = -1;
        for (auto [u, w] : children[v]) {
            int lc = dfs(u);
            if (first == -1) first = lc;
        }

        int p = ++T.timer;
        T.po[v] = p;
        T.weight[p] = wIn[v];

        if (first == -1) T.L[p] = p;
        else T.L[p] = first;

        return T.L[p];
    };

    dfs(root);
    T.rootPo = T.po[root];

    for (int v = 1; v <= N; ++v) {
        bool isKey = (parent[v] == -1) || (idxInParent[v] > 0);
        if (isKey) T.keyroots.push_back(T.po[v]);
    }

    sort(T.keyroots.begin(), T.keyroots.end());
    return T;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ll c1, c2, c3;
    cin >> c1 >> c2 >> c3;

    TreeInfo A = readTree(); // source
    TreeInfo B = readTree(); // target

    int n = A.N;
    int m = B.N;

    const int rootA = A.rootPo;
    const int rootB = B.rootPo;

    vector<ll> del(n + 1, 0), ins(m + 1, 0);

    for (int i = 1; i <= n; ++i) {
        if (i == rootA) del[i] = INF;
        else del[i] = c2 * (ll)A.weight[i];
    }

    for (int j = 1; j <= m; ++j) {
        if (j == rootB) ins[j] = INF;
        else ins[j] = c1 * (ll)B.weight[j];
    }

    vector<vector<ll>> td(n + 1, vector<ll>(m + 1, INF));
    vector<vector<ll>> fd(n + 1, vector<ll>(m + 1, INF));

    auto relabelCost = [&](int i, int j) -> ll {
        bool ri = (i == rootA);
        bool rj = (j == rootB);

        if (ri && rj) return 0;
        if (ri || rj) return INF;

        ll diff = (ll)A.weight[i] - (ll)B.weight[j];
        if (diff < 0) diff = -diff;
        return diff * c3;
    };

    for (int krA : A.keyroots) {
        int startA = A.L[krA];

        for (int krB : B.keyroots) {
            int startB = B.L[krB];

            int r0 = startA - 1;
            int c0 = startB - 1;

            fd[r0][c0] = 0;

            for (int r = startA; r <= krA; ++r) {
                ll val = fd[r - 1][c0] + del[r];
                if (val > INF) val = INF;
                fd[r][c0] = val;
            }

            for (int c = startB; c <= krB; ++c) {
                ll val = fd[r0][c - 1] + ins[c];
                if (val > INF) val = INF;
                fd[r0][c] = val;
            }

            for (int r = startA; r <= krA; ++r) {
                int lr = A.L[r];
                bool rLeftmost = (lr == startA);

                ll delr = del[r];

                ll* fd_r = fd[r].data();
                ll* fd_rm1 = fd[r - 1].data();
                ll* td_r = td[r].data();
                ll* fd_prev = fd[lr - 1].data();

                for (int c = startB; c <= krB; ++c) {
                    int lc = B.L[c];

                    ll v1 = fd_rm1[c] + delr;
                    if (v1 > INF) v1 = INF;

                    ll v2 = fd_r[c - 1] + ins[c];
                    if (v2 > INF) v2 = INF;

                    ll best = min(v1, v2);

                    if (rLeftmost && lc == startB) {
                        ll rc;
                        bool ri = (r == rootA);
                        bool rj = (c == rootB);

                        if (ri && rj) rc = 0;
                        else if (ri || rj) rc = INF;
                        else {
                            ll diff = (ll)A.weight[r] - (ll)B.weight[c];
                            if (diff < 0) diff = -diff;
                            rc = diff * c3;
                        }

                        ll v3 = fd_rm1[c - 1] + rc;
                        if (v3 > INF) v3 = INF;
                        if (v3 < best) best = v3;

                        td_r[c] = best;
                    } else {
                        ll v3 = fd_prev[lc - 1] + td_r[c];
                        if (v3 > INF) v3 = INF;
                        if (v3 < best) best = v3;
                    }

                    fd_r[c] = best;
                }
            }
        }
    }

    cout << td[rootA][rootB] << '\n';
    return 0;
}
