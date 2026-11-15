#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll INF = (ll)4e18;
const ll BIG = (ll)1e13;

int n, m;
vector<int> to_, cap_;
vector<ll> cost_;
vector<vector<int>> g;

int addEdge(int u, int v, int c, ll w) {
    int idx = to_.size();
    to_.push_back(v); cap_.push_back(c); cost_.push_back(w); g[u].push_back(idx);
    to_.push_back(u); cap_.push_back(0); cost_.push_back(-w); g[v].push_back(idx + 1);
    return idx;
}

int main() {
    scanf("%d %d", &n, &m);
    vector<vector<int>> a(n, vector<int>(m));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) scanf("%d", &a[i][j]);

    vector<int> owner(n, -1), p2(n, -1);
    vector<ll> M1(n, 0), W(n, 1);
    vector<int> c(m, 0);
    for (int i = 0; i < n; i++) {
        vector<pair<int,int>> v(m);
        for (int j = 0; j < m; j++) v[j] = {a[i][j], j};
        sort(v.begin(), v.end());
        vector<int> sorted_vals(m);
        for (int j = 0; j < m; j++) sorted_vals[j] = v[j].first;
        // unique positive values
        vector<pair<int,int>> uniq;
        for (int j = 0; j < m;) {
            int k = j;
            while (k < m && v[k].first == v[j].first) k++;
            if (k - j == 1 && v[j].first > 0) uniq.push_back(v[j]);
            j = k;
        }
        if (!uniq.empty()) {
            owner[i] = uniq.back().second;
            M1[i] = uniq.back().first;
            if (uniq.size() >= 2) p2[i] = uniq[uniq.size() - 2].second;
            c[owner[i]]++;
        }
        ll w = M1[i] + 1;
        while (binary_search(sorted_vals.begin(), sorted_vals.end(), (int)w)) w++;
        W[i] = w;
    }
    int maxc = 0;
    for (int j = 0; j < m; j++) maxc = max(maxc, c[j]);

    vector<ll> ws(W.begin(), W.end());
    sort(ws.begin(), ws.end());
    vector<ll> pre(n + 1, 0);
    for (int i = 0; i < n; i++) pre[i + 1] = pre[i] + ws[i];

    int US = m, NONE = m + 1, SRC = m + 2, SNK = m + 3;
    int base = m + 4;
    int V = base + n;

    ll best = INF;
    vector<ll> bestB(n, 0);

    for (int T = 0; T <= maxc; T++) {
        if (pre[T] >= best) break;
        to_.clear(); cap_.clear(); cost_.clear();
        g.assign(V, {});
        vector<int> rE(n, -1), sE(n, -1);
        vector<int> forced;
        for (int i = 0; i < n; i++) {
            if (owner[i] >= 0) {
                int ci = base + i;
                addEdge(owner[i], ci, 1, 0);
                int tgt = (p2[i] >= 0) ? p2[i] : NONE;
                rE[i] = addEdge(ci, tgt, 1, M1[i]);
                sE[i] = addEdge(ci, US, 1, W[i]);
            } else {
                sE[i] = addEdge(NONE, US, 1, W[i]);
            }
        }
        for (int j = 0; j < m; j++) {
            if (c[j] > 0) {
                int e = c[j] - T;
                if (e > 0) {
                    forced.push_back(addEdge(SRC, j, e, -BIG));
                    if (c[j] - e > 0) addEdge(SRC, j, c[j] - e, 0);
                } else {
                    addEdge(SRC, j, c[j], 0);
                }
            }
            if (c[j] < T) addEdge(j, SNK, T - c[j], 0);
        }
        addEdge(SRC, NONE, n, 0);
        addEdge(NONE, SNK, n, 0);
        if (T > 0) forced.push_back(addEdge(US, SNK, T, -BIG));

        // initial potentials via SPFA
        vector<ll> h(V, INF);
        {
            vector<char> inq(V, 0);
            deque<int> q;
            h[SRC] = 0; q.push_back(SRC); inq[SRC] = 1;
            while (!q.empty()) {
                int u = q.front(); q.pop_front(); inq[u] = 0;
                for (int e : g[u]) {
                    if (cap_[e] <= 0) continue;
                    int v = to_[e];
                    if (h[u] + cost_[e] < h[v]) {
                        h[v] = h[u] + cost_[e];
                        if (!inq[v]) { inq[v] = 1; q.push_back(v); }
                    }
                }
            }
            for (int v = 0; v < V; v++) if (h[v] >= INF / 2) h[v] = 0;
        }

        vector<ll> dist(V);
        vector<int> pe(V);
        while (true) {
            fill(dist.begin(), dist.end(), INF);
            fill(pe.begin(), pe.end(), -1);
            priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<pair<ll,int>>> pq;
            dist[SRC] = 0;
            pq.push({0, SRC});
            while (!pq.empty()) {
                auto [d, u] = pq.top(); pq.pop();
                if (d > dist[u]) continue;
                if (u == SNK) break;
                for (int e : g[u]) {
                    if (cap_[e] <= 0) continue;
                    int v = to_[e];
                    ll nd = d + cost_[e] + h[u] - h[v];
                    if (nd < dist[v]) {
                        dist[v] = nd;
                        pe[v] = e;
                        pq.push({nd, v});
                    }
                }
            }
            if (dist[SNK] >= INF) break;
            ll ds = dist[SNK];
            for (int v = 0; v < V; v++) h[v] += min(dist[v], ds);
            if (h[SNK] - h[SRC] >= 0) break;
            int f = INT_MAX;
            for (int v = SNK; v != SRC; v = to_[pe[v] ^ 1]) f = min(f, cap_[pe[v]]);
            for (int v = SNK; v != SRC; v = to_[pe[v] ^ 1]) {
                cap_[pe[v]] -= f;
                cap_[pe[v] ^ 1] += f;
            }
        }
        bool ok = true;
        for (int e : forced) if (cap_[e] != 0) { ok = false; break; }
        if (!ok) continue;
        ll total = 0;
        vector<ll> b(n, 0);
        for (int i = 0; i < n; i++) {
            if (sE[i] >= 0 && cap_[sE[i] ^ 1] > 0) { b[i] = W[i]; total += W[i]; }
            else if (rE[i] >= 0 && cap_[rE[i] ^ 1] > 0) { b[i] = M1[i]; total += M1[i]; }
        }
        if (total < best) { best = total; bestB = b; }
    }
    for (int i = 0; i < n; i++) printf("%lld%c", bestB[i], i + 1 == n ? '\n' : ' ');
    return 0;
}
