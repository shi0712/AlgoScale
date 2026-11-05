#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Edge {
    int to, rev, cap;
    ll cost;
};

struct Field {
    bool hasCur = false;
    int p = 0;
    bool hasSecond = false;
    int q = 0;
    ll r = 0;
    ll C = 0;
};

struct MinCostResult {
    int flow;
    ll cost;
};

int n, m;
vector<Field> fields;

const ll INF = (ll)4e18;
const ll BIG = (ll)1e12;

void addEdge(vector<vector<Edge>>& g, int u, int v, int cap, ll cost) {
    Edge a{v, (int)g[v].size(), cap, cost};
    Edge b{u, (int)g[u].size(), 0, -cost};
    g[u].push_back(a);
    g[v].push_back(b);
}

MinCostResult minCostFlow(vector<vector<Edge>>& g, int s, int t, int need) {
    int N = (int)g.size();

    vector<ll> pot(N, 0);
    queue<int> q;
    vector<char> inq(N, true);

    for (int i = 0; i < N; ++i) q.push(i);

    while (!q.empty()) {
        int v = q.front();
        q.pop();
        inq[v] = false;

        for (const Edge& e : g[v]) {
            if (e.cap > 0 && pot[e.to] > pot[v] + e.cost) {
                pot[e.to] = pot[v] + e.cost;
                if (!inq[e.to]) {
                    inq[e.to] = true;
                    q.push(e.to);
                }
            }
        }
    }

    int flow = 0;
    ll cost = 0;

    vector<ll> dist(N);
    vector<int> pv(N), pe(N);

    while (flow < need) {
        fill(dist.begin(), dist.end(), INF);
        fill(pv.begin(), pv.end(), -1);
        fill(pe.begin(), pe.end(), -1);

        priority_queue<pair<ll, int>, vector<pair<ll, int>>, greater<pair<ll, int>>> pq;

        dist[s] = 0;
        pq.push({0, s});

        while (!pq.empty()) {
            auto [d, v] = pq.top();
            pq.pop();

            if (d != dist[v]) continue;

            for (int i = 0; i < (int)g[v].size(); ++i) {
                Edge& e = g[v][i];
                if (e.cap == 0) continue;

                ll nd = d + e.cost + pot[v] - pot[e.to];
                if (nd < dist[e.to]) {
                    dist[e.to] = nd;
                    pv[e.to] = v;
                    pe[e.to] = i;
                    pq.push({nd, e.to});
                }
            }
        }

        if (dist[t] == INF) break;

        for (int i = 0; i < N; ++i) {
            if (dist[i] < INF) pot[i] += dist[i];
        }

        int add = need - flow;
        for (int v = t; v != s; v = pv[v]) {
            add = min(add, g[pv[v]][pe[v]].cap);
        }

        for (int v = t; v != s; v = pv[v]) {
            Edge& e = g[pv[v]][pe[v]];
            cost += (ll)add * e.cost;
            e.cap -= add;
            g[v][e.rev].cap += add;
        }

        flow += add;
    }

    return {flow, cost};
}

struct Option {
    int edgeIndex;
    ll b;
};

ll solveT(int T, vector<ll>* answer) {
    int S = 0;
    int fieldBase = 1;
    int oppBase = fieldBase + n;
    int us = oppBase + m;
    int sink = us + 1;
    int V = sink + 1;

    vector<vector<Edge>> g(V);

    for (int i = 0; i < n; ++i) {
        addEdge(g, S, fieldBase + i, 1, 0);
    }

    vector<vector<Option>> options(n);
    vector<int> usEdge(n, -1);

    for (int i = 0; i < n; ++i) {
        int fid = fieldBase + i;
        Field& f = fields[i];

        auto addOption = [&](int to, ll cost, ll b) {
            int idx = (int)g[fid].size();
            addEdge(g, fid, to, 1, cost);
            options[i].push_back({idx, b});
            return idx;
        };

        if (f.hasCur) {
            addOption(oppBase + f.p, 0, 0);

            if (f.hasSecond) {
                addOption(oppBase + f.q, f.r, f.r);
            } else {
                addOption(sink, f.r, f.r);
            }
        } else {
            addOption(sink, 0, 0);
        }

        usEdge[i] = addOption(us, f.C - BIG, f.C);
    }

    for (int j = 0; j < m; ++j) {
        addEdge(g, oppBase + j, sink, T, 0);
    }
    addEdge(g, us, sink, T, 0);

    MinCostResult res = minCostFlow(g, S, sink, n);

    if (res.flow < n) return INF;

    int usUsed = 0;
    for (int i = 0; i < n; ++i) {
        int fid = fieldBase + i;
        if (g[fid][usEdge[i]].cap == 0) ++usUsed;
    }

    if (usUsed != T) return INF;

    ll realCost = res.cost + BIG * (ll)usUsed;

    if (answer != nullptr) {
        answer->assign(n, 0);

        for (int i = 0; i < n; ++i) {
            int fid = fieldBase + i;
            for (const Option& op : options[i]) {
                if (g[fid][op.edgeIndex].cap == 0) {
                    (*answer)[i] = op.b;
                    break;
                }
            }
        }
    }

    return realCost;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> m;
    fields.resize(n);

    for (int i = 0; i < n; ++i) {
        vector<ll> a(m);
        map<ll, int> freq;

        for (int j = 0; j < m; ++j) {
            cin >> a[j];
            if (a[j] > 0) ++freq[a[j]];
        }

        vector<pair<ll, int>> uniq;

        for (int j = 0; j < m; ++j) {
            if (a[j] > 0 && freq[a[j]] == 1) {
                uniq.push_back({a[j], j});
            }
        }

        sort(uniq.begin(), uniq.end(), greater<pair<ll, int>>());

        Field f;

        if (!uniq.empty()) {
            f.hasCur = true;
            f.p = uniq[0].second;
            f.r = uniq[0].first;

            if (uniq.size() >= 2) {
                f.hasSecond = true;
                f.q = uniq[1].second;
            }
        }

        set<ll> used;
        for (int j = 0; j < m; ++j) {
            if (a[j] > 0) used.insert(a[j]);
        }

        ll x = f.r + 1;
        while (used.count(x)) ++x;
        f.C = x;

        fields[i] = f;
    }

    vector<ll> cache(n + 1, -1);

    auto getCost = [&](int T) -> ll {
        if (cache[T] != -1) return cache[T];
        return cache[T] = solveT(T, nullptr);
    };

    int lo = 0, hi = n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (getCost(mid) < INF / 2) hi = mid;
        else lo = mid + 1;
    }

    int left = lo;
    int right = n;

    while (right - left > 5) {
        int m1 = left + (right - left) / 3;
        int m2 = right - (right - left) / 3;

        ll c1 = getCost(m1);
        ll c2 = getCost(m2);

        if (c1 <= c2) right = m2;
        else left = m1;
    }

    int bestT = left;
    ll bestCost = getCost(left);

    for (int T = left + 1; T <= right; ++T) {
        ll cur = getCost(T);

        if (cur < bestCost) {
            bestCost = cur;
            bestT = T;
        }
    }

    vector<ll> answer;
    solveT(bestT, &answer);

    for (int i = 0; i < n; ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';

    return 0;
}
