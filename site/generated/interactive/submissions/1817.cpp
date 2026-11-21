#include <bits/stdc++.h>
using namespace std;

int n;
vector<vector<int>> adj;

long long ask1(const vector<int>& ep) {
    cout << "? 1 " << ep.size();
    for (int v : ep) cout << ' ' << v;
    cout << '\n';
    cout.flush();

    long long res;
    cin >> res;
    return res;
}

void askToggle(int u) {
    cout << "? 2 " << u << '\n';
    cout.flush();
}

vector<vector<int>> getComps(const vector<int>& C, int rem) {
    vector<char> inC(n + 1, 0), vis(n + 1, 0);
    for (int v : C) inC[v] = 1;

    vector<vector<int>> res;

    for (int s : C) {
        if (s == rem || vis[s]) continue;

        vector<int> comp;
        queue<int> q;
        q.push(s);
        vis[s] = 1;

        while (!q.empty()) {
            int v = q.front();
            q.pop();
            comp.push_back(v);

            for (int to : adj[v]) {
                if (to == rem || !inC[to] || vis[to]) continue;
                vis[to] = 1;
                q.push(to);
            }
        }

        res.push_back(comp);
    }

    return res;
}

int chooseCentroid(const vector<int>& C) {
    int bestU = C[0];
    int bestMax = n + 1;

    for (int u : C) {
        auto comps = getComps(C, u);
        int mx = 0;
        for (auto &comp : comps) {
            mx = max(mx, (int)comp.size());
        }

        if (mx < bestMax) {
            bestMax = mx;
            bestU = u;
        }
    }

    return bestU;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    adj.assign(n + 1, {});
    for (int i = 0; i < n - 1; i++) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    // Find the root.
    vector<int> C;
    for (int i = 1; i <= n; i++) C.push_back(i);

    int root = -1;

    while (true) {
        if ((int)C.size() == 1) {
            root = C[0];
            break;
        }

        int u = chooseCentroid(C);
        auto comps = getComps(C, u);
        int d = (int)comps.size();

        vector<int> reps(d);
        for (int i = 0; i < d; i++) reps[i] = comps[i][0];

        struct QueryInfo {
            vector<int> ep;
            bool all;
            int bit;
            int cnt;
        };

        vector<QueryInfo> queries;
        queries.push_back({reps, true, -1, d});

        int B = 0;
        while ((1 << B) < d) B++;

        for (int b = 0; b < B; b++) {
            vector<int> ep;
            for (int i = 0; i < d; i++) {
                if ((i >> b) & 1) ep.push_back(reps[i]);
            }

            if (!ep.empty()) {
                queries.push_back({ep, false, b, (int)ep.size()});
            }
        }

        vector<long long> before(queries.size());

        for (int i = 0; i < (int)queries.size(); i++) {
            before[i] = ask1(queries[i].ep);
        }

        askToggle(u);

        vector<long long> values(queries.size());

        for (int i = 0; i < (int)queries.size(); i++) {
            long long after = ask1(queries[i].ep);
            values[i] = llabs(after - before[i]) / 2;
        }

        int allValue = -1;
        vector<int> bitValue(B, 0);

        for (int i = 0; i < (int)queries.size(); i++) {
            if (queries[i].all) {
                allValue = (int)values[i];
            } else {
                bitValue[queries[i].bit] =
                    queries[i].cnt - (int)values[i];
            }
        }

        if (allValue == d) {
            root = u;
            break;
        }

        int idx = 0;
        for (int b = 0; b < B; b++) {
            if (bitValue[b]) idx |= (1 << b);
        }

        C = comps[idx];
    }

    // Root the tree.
    vector<int> parent(n + 1, 0);
    vector<vector<int>> children(n + 1);
    vector<int> order;
    order.reserve(n);

    queue<int> q;
    q.push(root);
    parent[root] = 0;

    while (!q.empty()) {
        int v = q.front();
        q.pop();
        order.push_back(v);

        for (int to : adj[v]) {
            if (to == parent[v]) continue;
            parent[to] = v;
            children[v].push_back(to);
            q.push(to);
        }
    }

    vector<int> height(n + 1, 1), deepChild(n + 1, 0);

    for (int i = n - 1; i >= 0; i--) {
        int v = order[i];
        height[v] = 1;
        deepChild[v] = 0;

        for (int to : children[v]) {
            if (height[to] + 1 > height[v]) {
                height[v] = height[to] + 1;
                deepChild[v] = to;
            }
        }
    }

    vector<int> value(n + 1, 0);
    vector<long long> pathSum(n + 1, 0);
    vector<char> decoded(n + 1, 0);

    long long rootValue = ask1({root});
    value[root] = (int)rootValue;
    pathSum[root] = rootValue;
    decoded[root] = true;

    while (true) {
        vector<int> ready;

        for (int v = 1; v <= n; v++) {
            if (!decoded[v] && parent[v] != 0 && decoded[parent[v]]) {
                ready.push_back(v);
            }
        }

        if (ready.empty()) break;

        sort(ready.begin(), ready.end(), [&](int a, int b) {
            if (height[a] != height[b]) return height[a] > height[b];
            return a < b;
        });

        int sumHeight = 0;
        for (int v : ready) {
            sumHeight += min(height[v], 10);
            if (sumHeight >= 10) {
                sumHeight = 10;
                break;
            }
        }

        int maxM = min(10, sumHeight);
        int chosenM = 0;
        vector<pair<int, int>> chosen;

        for (int m = maxM; m >= 1; m--) {
            int rem = m;
            vector<pair<int, int>> cand;

            for (int v : ready) {
                if (rem == 0) break;

                int len = min(height[v], rem);
                if (len > 0) {
                    cand.push_back({v, len});
                    rem -= len;
                }
            }

            if (rem != 0) continue;

            vector<int> lens;
            for (auto &p : cand) lens.push_back(p.second);
            sort(lens.begin(), lens.end(), greater<int>());

            long long cost = 0;
            int idx = m - 1;

            for (int len : lens) {
                cost += (1LL << idx);
                idx -= len;
            }

            if (cost <= n) {
                chosenM = m;
                chosen = cand;

                sort(chosen.begin(), chosen.end(), [](auto &a, auto &b) {
                    return a.second > b.second;
                });

                break;
            }
        }

        int mTotal = chosenM;

        struct SelectedChain {
            vector<int> nodes;
            long long from;
        };

        vector<SelectedChain> selectedChains;
        vector<int> endpoints;
        vector<int> exponent(n + 1, -1);

        long long knownPart = 0;
        int powerIndex = mTotal - 1;

        for (auto [top, len] : chosen) {
            SelectedChain sc;
            sc.nodes.clear();

            int cur = top;
            for (int j = 0; j < len; j++) {
                sc.nodes.push_back(cur);
                cur = deepChild[cur];
            }

            long long topWeight = (1LL << powerIndex);
            long long from = pathSum[parent[top]];

            sc.from = from;

            for (int j = 0; j < len; j++) {
                int exp = powerIndex - j;
                long long A = (1LL << exp);
                long long nextA =
                    (j + 1 < len ? (1LL << (powerIndex - (j + 1))) : 0);

                long long cnt = A - nextA;

                for (long long rep = 0; rep < cnt; rep++) {
                    endpoints.push_back(sc.nodes[j]);
                }

                exponent[sc.nodes[j]] = exp;
            }

            knownPart += from * topWeight;
            powerIndex -= len;

            selectedChains.push_back(sc);
        }

        long long response = ask1(endpoints);
        long long weightedValue = response - knownPart;

        long long totalWeight = (1LL << mTotal) - 1;
        long long negativeWeight = (totalWeight - weightedValue) / 2;

        for (auto &sc : selectedChains) {
            for (int v : sc.nodes) {
                int e = exponent[v];
                if ((negativeWeight >> e) & 1LL) {
                    value[v] = -1;
                } else {
                    value[v] = 1;
                }
            }
        }

        for (auto &sc : selectedChains) {
            long long curSum = sc.from;

            for (int v : sc.nodes) {
                curSum += value[v];
                pathSum[v] = curSum;
                decoded[v] = true;
            }
        }
    }

    cout << "!";
    for (int i = 1; i <= n; i++) {
        cout << ' ' << value[i];
    }
    cout << '\n';
    cout.flush();

    return 0;
}
