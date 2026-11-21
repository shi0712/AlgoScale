#include <bits/stdc++.h>
using namespace std;

int n;
vector<vector<int>> adj;
vector<int> par0, sub0;
vector<int> szk;      // known subtree size under real root, -1 if unprobed
vector<int> valprobe; // value just before the probe toggle
vector<char> toggled;
long long S_prev = 0;
int probes = 0;

long long querySumAll() {
    cout << "? 1 " << n;
    for (int i = 1; i <= n; ++i) cout << ' ' << i;
    cout << '\n' << flush;
    long long x;
    if (!(cin >> x)) exit(0);
    return x;
}

long long queryG(int u) {
    cout << "? 1 1 " << u << '\n' << flush;
    long long x;
    if (!(cin >> x)) exit(0);
    return x;
}

void toggleNode(int u) {
    cout << "? 2 " << u << '\n' << flush;
}

void probe(int u) {
    toggleNode(u);
    long long S = querySumAll();
    long long delta = S - S_prev;
    int val = (delta < 0 ? 1 : -1);
    long long sz = llabs(delta) / 2;
    szk[u] = (int)sz;
    valprobe[u] = val;
    toggled[u] = 1;
    S_prev = S;
    ++probes;
}

int compSize(int u, int w) {
    // size of the component containing w after removing edge (u,w)
    if (par0[w] == u) return sub0[w];
    // then par0[u] == w
    return n - sub0[u];
}

void dfs0(int u, int p) {
    par0[u] = p;
    sub0[u] = 1;
    for (int v : adj[u]) if (v != p) {
        dfs0(v, u);
        sub0[u] += sub0[v];
    }
}

pair<int, vector<int>> farthest(int src) {
    vector<int> d(n + 1, -1), p(n + 1, 0);
    queue<int> q;
    d[src] = 0;
    q.push(src);
    int far = src;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (d[u] > d[far]) far = u;
        for (int v : adj[u]) if (d[v] == -1) {
            d[v] = d[u] + 1;
            p[v] = u;
            q.push(v);
        }
    }
    return {far, p};
}

vector<char> inC;
vector<int> C;

int findCentroid() {
    int m = (int)C.size();
    if (m <= 2) return C[0];
    vector<int> par(n + 1, 0), sz(n + 1, 0), ord;
    ord.reserve(m);
    vector<int> st;
    st.push_back(C[0]);
    par[C[0]] = -1;
    while (!st.empty()) {
        int u = st.back(); st.pop_back();
        ord.push_back(u);
        for (int w : adj[u]) {
            if (!inC[w] || w == par[u]) continue;
            par[w] = u;
            st.push_back(w);
        }
    }
    for (int i = (int)ord.size() - 1; i >= 0; --i) {
        int u = ord[i];
        sz[u] = 1;
        for (int w : adj[u]) if (inC[w] && par[w] == u) sz[u] += sz[w];
    }
    int cent = ord[0];
    for (int u : ord) {
        int mx = m - sz[u];
        for (int w : adj[u]) if (inC[w] && par[w] == u) mx = max(mx, sz[w]);
        if (mx * 2 <= m) { cent = u; break; }
    }
    return cent;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    adj.assign(n + 1, {});
    for (int i = 0; i < n - 1; ++i) {
        int a, b;
        cin >> a >> b;
        adj[a].push_back(b);
        adj[b].push_back(a);
    }

    par0.assign(n + 1, 0);
    sub0.assign(n + 1, 0);
    dfs0(1, 0);

    szk.assign(n + 1, -1);
    valprobe.assign(n + 1, 0);
    toggled.assign(n + 1, 0);

    S_prev = querySumAll();

    inC.assign(n + 1, 1);
    C.resize(n);
    for (int i = 1; i <= n; ++i) C[i - 1] = i;

    int root = -1;
    while (true) {
        if ((int)C.size() == 1) {
            int u = C[0];
            if (szk[u] == -1) probe(u);
            root = u;
            break;
        }
        int v = findCentroid();
        if (szk[v] == -1) probe(v);
        int s = szk[v];
        if (s == n) { root = v; break; }
        int t = n - s;

        vector<int> cand;
        for (int w : adj[v]) if (inC[w] && compSize(v, w) == t) cand.push_back(w);

        int parent = -1;
        // check already probed candidates first
        for (int w : cand) {
            if (szk[w] != -1 && szk[w] > s) { parent = w; break; }
        }
        if (parent == -1) {
            int left = -1;
            for (int w : cand) {
                if (szk[w] != -1) continue; // already probed child
                if (left == -1) { left = w; continue; }
                probe(w);
                if (szk[w] == n) { root = w; break; }
                if (szk[w] > s) { parent = w; break; }
            }
            if (root != -1) break;
            if (parent == -1) parent = left;
        }
        if (parent == -1) {
            // Fallback: this should never happen. Try all candidates.
            for (int w : cand) {
                if (szk[w] == -1) probe(w);
                if (szk[w] == n) { root = w; break; }
                if (szk[w] > s) { parent = w; break; }
            }
            if (root != -1) break;
        }

        // new candidate component = component of (current C - v) containing parent
        vector<int> nC;
        vector<int> st;
        st.push_back(parent);
        vector<int> seen(n + 1, 0);
        seen[parent] = 1;
        while (!st.empty()) {
            int u = st.back(); st.pop_back();
            nC.push_back(u);
            for (int w : adj[u]) if (w != v && inC[w] && !seen[w]) {
                seen[w] = 1;
                st.push_back(w);
            }
        }
        // update inC
        for (int u : C) inC[u] = 0;
        for (int u : nC) inC[u] = 1;
        C.swap(nC);
    }

    // Recover all current values. Root is known.
    vector<int> par(n + 1, 0), order;
    order.reserve(n);
    vector<char> vis(n + 1, 0);
    queue<int> q;
    q.push(root);
    vis[root] = 1;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (int v : adj[u]) if (!vis[v]) {
            vis[v] = 1;
            par[v] = u;
            q.push(v);
        }
    }

    vector<int> val(n + 1, 0);
    vector<long long> g(n + 1, 0);
    for (int u : order) {
        if (u == root) {
            val[u] = -valprobe[u];
            g[u] = val[u];
        } else {
            if (toggled[u]) {
                val[u] = -valprobe[u];
                g[u] = g[par[u]] + val[u];
            } else {
                long long gu = queryG(u);
                g[u] = gu;
                val[u] = (int)(gu - g[par[u]]);
            }
        }
    }

    cout << "!";
    for (int i = 1; i <= n; ++i) cout << ' ' << val[i];
    cout << '\n' << flush;
    return 0;
}
