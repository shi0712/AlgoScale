#include <bits/stdc++.h>
using namespace std;

static vector<int> computeRanks(int root, const vector<vector<int>>& adj) {
    int n = (int)adj.size();

    vector<int> parent(n, -1), order;
    order.reserve(n);

    vector<int> st;
    st.push_back(root);
    parent[root] = -2;

    while (!st.empty()) {
        int u = st.back();
        st.pop_back();
        order.push_back(u);

        for (int v : adj[u]) {
            if (parent[v] == -1) {
                parent[v] = u;
                st.push_back(v);
            }
        }
    }

    vector<int> sz(n, 1);
    for (int i = n - 1; i >= 0; --i) {
        int u = order[i];
        for (int v : adj[u]) {
            if (parent[v] == u) {
                sz[u] += sz[v];
            }
        }
    }

    vector<int> nodes(n);
    iota(nodes.begin(), nodes.end(), 0);
    sort(nodes.begin(), nodes.end(), [&](int a, int b) {
        return sz[a] < sz[b];
    });

    struct TypeInfo {
        vector<int> elems; // sorted decreasing by record order
    };

    vector<TypeInfo> infos;
    vector<int> rankOfType;
    vector<int> orderIDs;
    map<vector<int>, int> mp;

    infos.push_back(TypeInfo{ {} });
    rankOfType.push_back(0);
    orderIDs.push_back(0);
    mp[vector<int>{}] = 0;

    auto cmpRecord = [&](const vector<int>& A, const vector<int>& B) -> int {
        int p = min(A.size(), B.size());
        for (int i = 0; i < p; ++i) {
            int ra = rankOfType[A[i]];
            int rb = rankOfType[B[i]];
            if (ra != rb) {
                return ra > rb ? 1 : -1;
            }
        }
        if (A.size() == B.size()) return 0;
        return A.size() > B.size() ? 1 : -1;
    };

    vector<int> nodeType(n, -1);
    vector<vector<int>> Tvec(n);

    for (int u : nodes) {
        vector<int> M;
        for (int v : adj[u]) {
            if (parent[v] == u) {
                M.insert(M.end(), Tvec[v].begin(), Tvec[v].end());
            }
        }

        for (int v : adj[u]) {
            if (parent[v] == u) {
                vector<int>().swap(Tvec[v]);
            }
        }

        sort(M.begin(), M.end(), [&](int a, int b) {
            return rankOfType[a] > rankOfType[b];
        });

        vector<int> key = M;
        sort(key.begin(), key.end());

        int typ;
        auto it = mp.find(key);
        if (it != mp.end()) {
            typ = it->second;
        } else {
            typ = (int)infos.size();
            infos.push_back(TypeInfo{ M });
            mp[key] = typ;

            int lo = 0, hi = (int)orderIDs.size();
            while (lo < hi) {
                int mid = (lo + hi) >> 1;
                int c = cmpRecord(M, infos[orderIDs[mid]].elems);
                if (c <= 0) hi = mid;
                else lo = mid + 1;
            }

            orderIDs.insert(orderIDs.begin() + lo, typ);
            rankOfType.push_back(0);

            for (int i = lo; i < (int)orderIDs.size(); ++i) {
                rankOfType[orderIDs[i]] = i;
            }
        }

        nodeType[u] = typ;

        Tvec[u] = M;
        Tvec[u].push_back(typ);
    }

    vector<int> cnt(infos.size(), 0);
    for (int u = 0; u < n; ++u) {
        cnt[rankOfType[nodeType[u]]]++;
    }

    vector<int> suffixGreater(infos.size() + 1, 0);
    for (int i = (int)infos.size() - 1; i >= 0; --i) {
        suffixGreater[i] = suffixGreater[i + 1] + cnt[i];
    }

    vector<int> f(n);
    for (int u = 0; u < n; ++u) {
        int rk = rankOfType[nodeType[u]];
        f[u] = 1 + suffixGreater[rk + 1];
    }

    return f;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int c;
    cin >> c;

    int n;
    cin >> n;

    vector<vector<int>> adj(n);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    vector<int> par(n, -1), dep(n, 0);
    {
        vector<int> st;
        st.push_back(0);
        par[0] = -2;

        while (!st.empty()) {
            int u = st.back();
            st.pop_back();

            for (int v : adj[u]) {
                if (par[v] == -1) {
                    par[v] = u;
                    dep[v] = dep[u] + 1;
                    st.push_back(v);
                }
            }
        }
    }

    auto getPath = [&](int s, int t) {
        vector<int> L, R;

        int a = s, b = t;

        while (dep[a] > dep[b]) {
            L.push_back(a);
            a = par[a];
        }

        while (dep[b] > dep[a]) {
            R.push_back(b);
            b = par[b];
        }

        while (a != b) {
            L.push_back(a);
            a = par[a];

            R.push_back(b);
            b = par[b];
        }

        L.push_back(a);
        reverse(R.begin(), R.end());
        L.insert(L.end(), R.begin(), R.end());
        return L;
    };

    int m;
    cin >> m;

    unordered_map<int, vector<int>> cache;
    cache.reserve(m * 2 + 10);

    while (m--) {
        int s, t, ox, oy, r;
        cin >> s >> t >> ox >> oy >> r;
        --s;
        --t;

        vector<int> path = getPath(s, t);

        vector<int> X, Y;
        if (ox == 0) X.push_back(s);
        else X = path;

        if (oy == 0) Y.push_back(t);
        else Y = path;

        long long ans = 0;

        for (int x : X) {
            auto it = cache.find(x);
            if (it == cache.end()) {
                vector<int> f = computeRanks(x, adj);
                it = cache.emplace(x, move(f)).first;
            }

            const vector<int>& f = it->second;

            for (int y : Y) {
                if (f[y] <= r) {
                    ++ans;
                }
            }
        }

        cout << ans << '\n';
    }

    return 0;
}
