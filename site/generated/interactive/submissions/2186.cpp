#include <bits/stdc++.h>
using namespace std;

int n;
vector<vector<int>> adj;
vector<long long> init_f;

long long ask1(const vector<int>& nodes) {
    int k = (int)nodes.size();
    cout << "? 1 " << k;
    for (int u : nodes) cout << ' ' << u;
    cout << '\n';
    cout.flush();

    long long res;
    if (!(cin >> res)) exit(0);
    return res;
}

void toggle(int u) {
    cout << "? 2 " << u << '\n';
    cout.flush();
}

struct Component {
    vector<int> nodes;
    int rep;
};

int centroid(const vector<int>& A, const vector<char>& inA) {
    int N = (int)A.size();
    int start = A[0];

    vector<int> parent(n + 1, 0), order;
    vector<char> vis(n + 1, 0);
    stack<int> st;

    st.push(start);
    vis[start] = 1;
    parent[start] = -1;

    while (!st.empty()) {
        int v = st.top();
        st.pop();
        order.push_back(v);

        for (int to : adj[v]) {
            if (inA[to] && !vis[to]) {
                vis[to] = 1;
                parent[to] = v;
                st.push(to);
            }
        }
    }

    vector<int> sub(n + 1, 0);
    for (int i = (int)order.size() - 1; i >= 0; --i) {
        int v = order[i];
        sub[v] = 1;
        for (int to : adj[v]) {
            if (inA[to] && parent[to] == v) {
                sub[v] += sub[to];
            }
        }
    }

    int best = start;
    for (int v : A) {
        int maxpart = N - sub[v];
        for (int to : adj[v]) {
            if (inA[to] && parent[to] == v) {
                maxpart = max(maxpart, sub[to]);
            }
        }
        if (maxpart <= N / 2) {
            best = v;
            break;
        }
    }
    return best;
}

vector<Component> getComps(const vector<int>& A, int t, const vector<char>& inA) {
    vector<Component> comps;
    vector<char> vis(n + 1, 0);
    vis[t] = 1;

    for (int to : adj[t]) {
        if (!inA[to] || vis[to]) continue;

        vector<int> nodes;
        stack<int> st;
        st.push(to);
        vis[to] = 1;

        while (!st.empty()) {
            int v = st.top();
            st.pop();
            nodes.push_back(v);

            for (int to2 : adj[v]) {
                if (inA[to2] && !vis[to2]) {
                    vis[to2] = 1;
                    st.push(to2);
                }
            }
        }

        comps.push_back({nodes, to});
    }

    return comps;
}

int identifyBinary(int t, const vector<Component>& comps, bool alreadyToggled) {
    int d = (int)comps.size();

    int q = 0;
    while ((1 << q) - 1 < d) ++q;

    vector<vector<int>> nodes(q);
    vector<long long> F0(q, 0);
    vector<int> K(q, 0);

    for (int b = 0; b < q; ++b) {
        nodes[b].push_back(t);
        F0[b] = init_f[t];
        K[b] = 1;
    }

    for (int j = 0; j < d; ++j) {
        int code = j + 1;
        for (int b = 0; b < q; ++b) {
            if (code & (1 << b)) {
                nodes[b].push_back(comps[j].rep);
                F0[b] += init_f[comps[j].rep];
                ++K[b];
            }
        }
    }

    if (!alreadyToggled) toggle(t);

    int pattern = 0;
    for (int b = 0; b < q; ++b) {
        if (K[b] <= 1) continue;

        long long F1 = ask1(nodes[b]);
        long long D = F1 - F0[b];
        long long W = (D >= 0 ? D : -D) / 2;

        if (W == K[b] - 1) {
            pattern |= (1 << b);
        }
    }

    if (pattern == 0) return -1;
    if (pattern - 1 < d) return pattern - 1;

    // Safety fallback; should not be needed for valid judge responses.
    for (int j = 0; j < d; ++j) {
        vector<int> nodes2 = {t, comps[j].rep};
        long long F0 = init_f[t] + init_f[comps[j].rep];
        long long F1 = ask1(nodes2);
        long long D = F1 - F0;
        long long W = (D >= 0 ? D : -D) / 2;

        if (W == 1) return j;
    }

    return -1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    adj.assign(n + 1, {});
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    init_f.assign(n + 1, 0);
    for (int v = 1; v <= n; ++v) {
        vector<int> nodes = {v};
        init_f[v] = ask1(nodes);
    }

    vector<int> A;
    A.reserve(n);
    for (int v = 1; v <= n; ++v) A.push_back(v);

    vector<char> inA(n + 1, 0);
    vector<int> flipped;
    flipped.reserve(20);

    while (A.size() > 1) {
        fill(inA.begin(), inA.end(), 0);
        for (int v : A) inA[v] = 1;

        int t = centroid(A, inA);
        vector<Component> comps = getComps(A, t, inA);
        int d = (int)comps.size();

        int compIdx = -1;
        bool weighted = (1LL * d * (d + 1) / 2 <= n - 1);

        if (weighted) {
            vector<int> nodes;
            nodes.reserve(1 + d * (d + 1) / 2);
            nodes.push_back(t);

            long long F0 = init_f[t];
            int K = 1;

            for (int j = 0; j < d; ++j) {
                int w = j + 1;
                int rep = comps[j].rep;

                for (int r = 0; r < w; ++r) nodes.push_back(rep);
                F0 += 1LL * w * init_f[rep];
                K += w;
            }

            toggle(t);

            long long F1 = ask1(nodes);
            long long D = F1 - F0;
            long long W = (D >= 0 ? D : -D) / 2;

            if (W == K) {
                compIdx = -1;
            } else {
                int target = K - (int)W;
                for (int j = 0; j < d; ++j) {
                    if (j + 1 == target) {
                        compIdx = j;
                        break;
                    }
                }

                if (compIdx == -1) {
                    compIdx = identifyBinary(t, comps, true);
                }
            }
        } else {
            compIdx = identifyBinary(t, comps, false);
        }

        flipped.push_back(t);

        if (compIdx == -1) {
            A.assign(1, t);
            break;
        } else {
            A = comps[compIdx].nodes;
        }
    }

    int root = A.empty() ? 1 : A[0];

    vector<int> parent(n + 1, 0), order;
    vector<char> vis(n + 1, 0);
    stack<int> st;

    st.push(root);
    vis[root] = 1;
    parent[root] = -1;

    while (!st.empty()) {
        int v = st.top();
        st.pop();
        order.push_back(v);

        for (int to : adj[v]) {
            if (!vis[to]) {
                vis[to] = 1;
                parent[to] = v;
                st.push(to);
            }
        }
    }

    vector<int> ans(n + 1, 0);
    ans[root] = (init_f[root] > 0 ? 1 : -1);

    for (int v : order) {
        if (v == root) continue;
        long long diff = init_f[v] - init_f[parent[v]];
        ans[v] = (diff > 0 ? 1 : -1);
    }

    for (int u : flipped) {
        ans[u] = -ans[u];
    }

    cout << '!';
    for (int v = 1; v <= n; ++v) {
        cout << ' ' << ans[v];
    }
    cout << '\n';
    cout.flush();

    return 0;
}
