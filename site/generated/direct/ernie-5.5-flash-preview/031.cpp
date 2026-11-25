#include <bits/stdc++.h>
using namespace std;

const int MOD = 998244353;

struct Matrix {
    int a, b, c, d; // [[a,b],[c,d]]
};

Matrix mul(const Matrix &A, const Matrix &B) {
    return {
        (int)((1LL * A.a * B.a + 1LL * A.b * B.c) % MOD),
        (int)((1LL * A.a * B.b + 1LL * A.b * B.d) % MOD),
        (int)((1LL * A.c * B.a + 1LL * A.d * B.c) % MOD),
        (int)((1LL * A.c * B.b + 1LL * A.d * B.d) % MOD)
    };
}

struct LightNode {
    int l, r;
    int val0, valA, valB;
};

int n, root;
vector<vector<int>> adj;
vector<int> parent, depth, heavy, top, pos, path_end, vertexAtPos;
vector<int> p; // 0 = unprocessed, 1 = processed
vector<vector<int>> lightChildren;
vector<int> lightPos; // position in parent's light list, -1 if not light
vector<int> lightRoot, lightLen;
vector<LightNode> lightPool;

vector<Matrix> seg;
vector<Matrix> Tmat;

void combineLight(int idx, int left, int right) {
    auto &L = lightPool[left];
    auto &R = lightPool[right];
    lightPool[idx].val0 = (int)(1LL * L.val0 * R.val0 % MOD);
    lightPool[idx].valA = (int)(1LL * L.valA * R.valA % MOD);
    lightPool[idx].valB = (int)((1LL * L.valB * R.valA + 1LL * L.valA * R.valB) % MOD);
}

int buildLightRec(int v, int l, int r) {
    int idx = lightPool.size();
    lightPool.push_back({-1, -1, 1, 1, 0});
    if (l == r) {
        // initially all vertices are unprocessed, state A = 1
        lightPool[idx].val0 = 1;
        lightPool[idx].valA = 2;
        lightPool[idx].valB = 1;
    } else {
        int mid = (l + r) >> 1;
        int left = buildLightRec(v, l, mid);
        int right = buildLightRec(v, mid + 1, r);
        lightPool[idx].l = left;
        lightPool[idx].r = right;
        combineLight(idx, left, right);
    }
    return idx;
}

void updateLightRec(int idx, int l, int r, int pos, int s, int a, int b) {
    if (l == r) {
        lightPool[idx].val0 = s;
        lightPool[idx].valA = a;
        lightPool[idx].valB = b;
        return;
    }
    int mid = (l + r) >> 1;
    if (pos <= mid)
        updateLightRec(lightPool[idx].l, l, mid, pos, s, a, b);
    else
        updateLightRec(lightPool[idx].r, mid + 1, r, pos, s, a, b);
    combineLight(idx, lightPool[idx].l, lightPool[idx].r);
}

Matrix compute_T(int v) {
    if (p[v] == 0) {
        int L0 = (lightRoot[v] == -1 ? 1 : lightPool[lightRoot[v]].val0);
        if (heavy[v] != -1) {
            return {L0, (int)(2LL * L0 % MOD), 0, 0};
        } else {
            return {L0, 0, 0, 0};
        }
    } else {
        int A = (lightRoot[v] == -1 ? 1 : lightPool[lightRoot[v]].valA);
        int B = (lightRoot[v] == -1 ? 0 : lightPool[lightRoot[v]].valB);
        if (heavy[v] == -1) {
            return {A, 0, B, A};
        } else {
            if (p[heavy[v]] == 0) {
                return {
                    (int)(2LL * A % MOD),
                    0,
                    (int)((2LL * B + A) % MOD),
                    0
                };
            } else {
                return {
                    A,
                    (int)(2LL * A % MOD),
                    B,
                    (int)((2LL * B + A) % MOD)
                };
            }
        }
    }
}

void buildSeg(int node, int l, int r) {
    if (l == r) {
        seg[node] = Tmat[vertexAtPos[l]];
        return;
    }
    int mid = (l + r) >> 1;
    buildSeg(node * 2, l, mid);
    buildSeg(node * 2 + 1, mid + 1, r);
    seg[node] = mul(seg[node * 2], seg[node * 2 + 1]);
}

void updateSeg(int node, int l, int r, int idx, Matrix val) {
    if (l == r) {
        seg[node] = val;
        return;
    }
    int mid = (l + r) >> 1;
    if (idx <= mid)
        updateSeg(node * 2, l, mid, idx, val);
    else
        updateSeg(node * 2 + 1, mid + 1, r, idx, val);
    seg[node] = mul(seg[node * 2], seg[node * 2 + 1]);
}

Matrix querySeg(int node, int l, int r, int ql, int qr) {
    if (ql <= l && r <= qr) return seg[node];
    int mid = (l + r) >> 1;
    if (qr <= mid) return querySeg(node * 2, l, mid, ql, qr);
    if (ql > mid) return querySeg(node * 2 + 1, mid + 1, r, ql, qr);
    Matrix left = querySeg(node * 2, l, mid, ql, qr);
    Matrix right = querySeg(node * 2 + 1, mid + 1, r, ql, qr);
    return mul(left, right);
}

void updateT(int v) {
    Matrix T = compute_T(v);
    Tmat[v] = T;
    updateSeg(1, 0, n - 1, pos[v], T);
}

int flip(int x) {
    p[x] = 1;
    updateT(x);

    int par = parent[x];
    if (par != 0 && heavy[par] == x) {
        // x is the heavy child of par, so par's transition depends on p[x]
        updateT(par);
    }

    int cur = top[x];
    Matrix P = querySeg(1, 0, n - 1, pos[cur], path_end[cur]);
    int U = P.a;
    int V = P.c;

    while (cur != root) {
        int par = parent[cur];

        int s, a, b;
        if (p[cur] == 0) {
            s = U % MOD;
            a = (int)(2LL * U % MOD);
            b = U % MOD;
        } else {
            s = (int)((U + 2LL * V) % MOD);
            a = s;
            b = V % MOD;
        }

        int idx = lightPos[cur];
        int len = lightLen[par];
        updateLightRec(lightRoot[par], 0, len - 1, idx, s, a, b);
        updateT(par);

        cur = top[par];
        P = querySeg(1, 0, n - 1, pos[cur], path_end[cur]);
        U = P.a;
        V = P.c;
    }

    return U; // root is unprocessed, its state is (A, 0)
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;
    if (n == 1) {
        return 0;
    }

    adj.assign(n + 1, {});
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    vector<int> b(n - 1);
    vector<char> used(n + 1, 0);
    for (int i = 0; i < n - 1; ++i) {
        cin >> b[i];
        used[b[i]] = 1;
    }

    root = 1;
    while (root <= n && used[root]) ++root;

    parent.assign(n + 1, 0);
    depth.assign(n + 1, 0);
    heavy.assign(n + 1, -1);
    top.assign(n + 1, 0);
    pos.assign(n + 1, 0);
    path_end.assign(n + 1, 0);
    vertexAtPos.assign(n, 0);
    p.assign(n + 1, 0);

    // First DFS: parent, depth, order
    vector<int> order;
    order.reserve(n);
    order.push_back(root);
    parent[root] = 0;
    depth[root] = 0;
    for (size_t i = 0; i < order.size(); ++i) {
        int u = order[i];
        for (int v : adj[u]) {
            if (v != parent[u]) {
                parent[v] = u;
                depth[v] = depth[u] + 1;
                order.push_back(v);
            }
        }
    }

    // Compute sizes and heavy child
    vector<int> sz(n + 1, 0);
    for (int i = n - 1; i >= 0; --i) {
        int u = order[i];
        sz[u] = 1;
        int maxsz = 0;
        for (int v : adj[u]) {
            if (v != parent[u]) {
                sz[u] += sz[v];
                if (sz[v] > maxsz) {
                    maxsz = sz[v];
                    heavy[u] = v;
                }
            }
        }
    }

    // Heavy-light decomposition (iterative, heavy child first)
    int timer = 0;
    stack<pair<int, int>> st;
    st.push({root, root});
    while (!st.empty()) {
        auto [u, tp] = st.top();
        st.pop();

        top[u] = tp;
        pos[u] = timer;
        vertexAtPos[timer] = u;
        ++timer;

        vector<int> lights;
        for (int v : adj[u]) {
            if (v != parent[u] && v != heavy[u]) lights.push_back(v);
        }
        for (int i = (int)lights.size() - 1; i >= 0; --i) {
            st.push({lights[i], lights[i]});
        }
        if (heavy[u] != -1) {
            st.push({heavy[u], tp});
        }
    }

    // Compute path_end for each heavy path
    for (int v = 1; v <= n; ++v) {
        if (top[v] == v) {
            int btm = v;
            while (heavy[btm] != -1) btm = heavy[btm];
            path_end[v] = pos[btm];
        }
    }

    // Build light children lists
    lightChildren.assign(n + 1, {});
    lightPos.assign(n + 1, -1);
    for (int u = 1; u <= n; ++u) {
        for (int v : adj[u]) {
            if (v != parent[u] && v != heavy[u]) {
                lightPos[v] = lightChildren[u].size();
                lightChildren[u].push_back(v);
            }
        }
    }

    // Build light segment trees
    lightRoot.assign(n + 1, -1);
    lightLen.assign(n + 1, 0);
    lightPool.clear();
    lightPool.reserve(4 * n + 5);
    for (int v = 1; v <= n; ++v) {
        if (!lightChildren[v].empty()) {
            lightLen[v] = lightChildren[v].size();
            lightRoot[v] = buildLightRec(v, 0, lightLen[v] - 1);
        }
    }

    // Initialize T matrices
    Tmat.assign(n + 1, {});
    for (int v = 1; v <= n; ++v) {
        Tmat[v] = compute_T(v);
    }

    // Build heavy-path segment tree
    seg.assign(4 * n, {});
    buildSeg(1, 0, n - 1);

    // Process operations
    for (int x : b) {
        int ans = flip(x);
        cout << ans << '\n';
    }

    return 0;
}
