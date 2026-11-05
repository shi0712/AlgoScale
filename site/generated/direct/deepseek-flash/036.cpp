#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;

struct Mat {
    int a, b, c, d;
};

static inline Mat identityMat() {
    return {1, 0, 0, 1};
}

static inline Mat mulMat(const Mat& x, const Mat& y) {
    Mat z;
    z.a = (1LL * x.a * y.a + 1LL * x.b * y.c) % MOD;
    z.b = (1LL * x.a * y.b + 1LL * x.b * y.d) % MOD;
    z.c = (1LL * x.c * y.a + 1LL * x.d * y.c) % MOD;
    z.d = (1LL * x.c * y.b + 1LL * x.d * y.d) % MOD;
    return z;
}

static inline Mat makeLightMat(int S, int A) {
    // [[S, 0],
    //  [A, S]]
    return {S, 0, A, S};
}

struct SegTree {
    int n, sz;
    vector<Mat> tr;

    void init(const vector<Mat>& arr, int m) {
        n = m;
        sz = 1;
        while (sz < n) sz <<= 1;
        tr.assign(sz << 1, identityMat());

        for (int i = 1; i <= m; ++i) {
            tr[sz + i - 1] = arr[i];
        }
        for (int i = sz - 1; i >= 1; --i) {
            tr[i] = mulMat(tr[i << 1], tr[i << 1 | 1]);
        }
    }

    void update(int pos, const Mat& val) {
        int p = sz + pos - 1;
        tr[p] = val;
        for (p >>= 1; p; p >>= 1) {
            tr[p] = mulMat(tr[p << 1], tr[p << 1 | 1]);
        }
    }

    Mat query(int l, int r) const {
        if (l > r) return identityMat();
        int L = sz + l - 1;
        int R = sz + r - 1;

        Mat left = identityMat();
        Mat right = identityMat();

        while (L <= R) {
            if (L & 1) left = mulMat(left, tr[L++]);
            if (!(R & 1)) right = mulMat(tr[R--], right);
            L >>= 1;
            R >>= 1;
        }
        return mulMat(left, right);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    if (n == 1) {
        return 0;
    }

    vector<vector<int>> g(n + 1);
    for (int i = 1; i < n; ++i) {
        int u, v;
        cin >> u >> v;
        g[u].push_back(v);
        g[v].push_back(u);
    }

    vector<int> b(n - 1);
    vector<char> inB(n + 1, false);
    for (int i = 0; i < n - 1; ++i) {
        cin >> b[i];
        inB[b[i]] = true;
    }

    int root = 1;
    while (root <= n && inB[root]) ++root;
    // root is the unique vertex that never appears in b

    // Root the tree at root.
    vector<int> parent(n + 1, 0);
    vector<int> order;
    order.reserve(n);

    vector<int> stk;
    stk.reserve(n);
    stk.push_back(root);
    parent[root] = 0;

    while (!stk.empty()) {
        int v = stk.back();
        stk.pop_back();
        order.push_back(v);

        for (int u : g[v]) {
            if (u == parent[v]) continue;
            parent[u] = v;
            stk.push_back(u);
        }
    }

    // Subtree sizes and heavy child.
    vector<int> sz(n + 1, 0), heavy(n + 1, 0);

    for (int i = n - 1; i >= 0; --i) {
        int v = order[i];
        sz[v] = 1;

        int bestSize = 0;
        for (int u : g[v]) {
            if (parent[u] == v) {
                sz[v] += sz[u];
                if (sz[u] > bestSize) {
                    bestSize = sz[u];
                    heavy[v] = u;
                }
            }
        }
    }

    // Heavy-light decomposition.
    vector<int> head(n + 1), dfn(n + 1), rev(n + 1), tailPos(n + 1);
    vector<pair<int, int>> hldStack;
    hldStack.push_back({root, root});

    int timer = 0;

    while (!hldStack.empty()) {
        auto [v, h] = hldStack.back();
        hldStack.pop_back();

        int cur = v;
        int last = 0;

        while (cur) {
            head[cur] = h;
            dfn[cur] = ++timer;
            rev[timer] = cur;
            last = cur;

            for (int u : g[cur]) {
                if (parent[u] == cur && u != heavy[cur]) {
                    hldStack.push_back({u, u});
                }
            }

            cur = heavy[cur];
        }

        tailPos[h] = dfn[last];
    }

    vector<char> selected(n + 1, false);

    // Initial DP when every vertex is unselected.
    // For a node v:
    //   S[v] = 2 * product S[child], A[v] = product S[child]
    vector<int> S(n + 1, 0), A(n + 1, 0);

    for (int i = n - 1; i >= 0; --i) {
        int v = order[i];

        long long prod = 1;
        for (int u : g[v]) {
            if (parent[u] == v) {
                prod = prod * S[u] % MOD;
            }
        }

        S[v] = 2LL * prod % MOD;
        A[v] = prod;
    }

    // Build light-child segment tree.
    int lightCnt = 0;
    vector<int> lightPos(n + 1, 0), L(n + 1), R(n + 1);
    vector<Mat> lightArr(1, identityMat());

    for (int v = 1; v <= n; ++v) {
        L[v] = lightCnt + 1;

        for (int u : g[v]) {
            if (parent[u] == v && u != heavy[v]) {
                lightPos[u] = ++lightCnt;
                lightArr.push_back(makeLightMat(S[u], A[u]));
            }
        }

        R[v] = lightCnt;
    }

    SegTree lightSeg;
    lightSeg.init(lightArr, lightCnt);

    // Build initial heavy-path segment tree.
    vector<Mat> heavyArr(n + 1, identityMat());

    for (int v = 1; v <= n; ++v) {
        int P0 = 1, Q0 = 0;

        if (L[v] <= R[v]) {
            Mat m = lightSeg.query(L[v], R[v]);
            P0 = m.a;
            Q0 = m.c;
        }

        Mat hMat;
        if (!selected[v]) {
            hMat = {2LL * P0 % MOD, 0, P0, 0};
        } else {
            hMat = {
                (P0 + 2LL * Q0) % MOD,
                2LL * P0 % MOD,
                Q0,
                P0
            };
        }

        heavyArr[dfn[v]] = hMat;
    }

    SegTree heavySeg;
    heavySeg.init(heavyArr, n);

    // Powers of inverse of 2.
    vector<int> invPow2(n + 1);
    invPow2[0] = 1;
    const int inv2 = (MOD + 1) / 2;

    for (int i = 1; i <= n; ++i) {
        invPow2[i] = 1LL * invPow2[i - 1] * inv2 % MOD;
    }

    int unselectedEdges = n - 1;

    auto recalcH = [&](int v) {
        int P0 = 1, Q0 = 0;

        if (L[v] <= R[v]) {
            Mat m = lightSeg.query(L[v], R[v]);
            P0 = m.a;
            Q0 = m.c;
        }

        Mat hMat;

        if (!selected[v]) {
            hMat = {2LL * P0 % MOD, 0, P0, 0};
        } else {
            hMat = {
                (P0 + 2LL * Q0) % MOD,
                2LL * P0 % MOD,
                Q0,
                P0
            };
        }

        heavySeg.update(dfn[v], hMat);
    };

    for (int x : b) {
        // Remove edges whose both endpoints were unselected.
        for (int u : g[x]) {
            if (!selected[u]) {
                --unselectedEdges;
            }
        }

        selected[x] = true;
        recalcH(x);

        int cur = x;
        int tauPar = 0;

        while (true) {
            int h = head[cur];

            Mat prod = heavySeg.query(dfn[h], tailPos[h]);

            int newS = prod.a; // first column of prod
            int newA = prod.c;

            S[h] = newS;
            A[h] = newA;

            if (h == root) {
                tauPar = newA;
                break;
            }

            int p = parent[h];

            lightSeg.update(lightPos[h], makeLightMat(newS, newA));
            recalcH(p);

            cur = p;
        }

        int ans = 1LL * tauPar * invPow2[unselectedEdges] % MOD;
        cout << ans << '\n';
    }

    return 0;
}
