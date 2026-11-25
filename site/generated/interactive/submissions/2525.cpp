#include <bits/stdc++.h>
using namespace std;

int n;
vector<vector<int>> adj;

vector<int> get_dist(int c) {
    vector<int> dist(n + 1, -1);
    queue<int> q;
    dist[c] = 0;
    q.push(c);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : adj[u]) {
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }
    return dist;
}

pair<int, int> find_ball(const vector<int>& S, int L, int U) {
    int m = S.size();
    if (m == 1) return {S[0], 0};
    if (m == 2) return {S[0], 0};
    int c = S[0];
    for (int iter = 0; iter < 1000; ++iter) {
        auto dist = get_dist(c);
        vector<pair<int, int>> ds(m);
        for (int i = 0; i < m; ++i) {
            ds[i] = {dist[S[i]], S[i]};
        }
        nth_element(ds.begin(), ds.begin() + L - 1, ds.end());
        int d = ds[L - 1].first;
        int cnt = 0;
        for (auto& p : ds) {
            if (p.first <= d) cnt++;
        }
        if (cnt <= U) {
            return {c, d};
        } else {
            c = ds[L - 1].second;
        }
    }
    auto dist = get_dist(c);
    vector<pair<int, int>> ds(m);
    for (int i = 0; i < m; ++i) ds[i] = {dist[S[i]], S[i]};
    nth_element(ds.begin(), ds.begin() + L - 1, ds.end());
    return {c, ds[L - 1].first};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    cin >> n;
    adj.assign(n + 1, {});
    for (int i = 2; i <= n; ++i) {
        int p;
        cin >> p;
        adj[i].push_back(p);
        adj[p].push_back(i);
    }
    vector<int> S(n);
    iota(S.begin(), S.end(), 1);
    while (S.size() > 1) {
        int m = S.size();
        int L = (m + 2) / 3;
        int U = (2 * m) / 3;
        auto [u, k] = find_ball(S, L, U);
        cout << "? " << u << " " << k << "\n";
        cout.flush();
        int resp;
        cin >> resp;
        if (resp == -1) return 0;
        auto dist = get_dist(u);
        vector<int> newS;
        newS.reserve(m);
        if (resp == 1) {
            for (int x : S) if (dist[x] <= k) newS.push_back(x);
        } else {
            for (int x : S) if (dist[x] > k) newS.push_back(x);
        }
        S.swap(newS);
    }
    cout << "! " << S[0] << "\n";
    cout.flush();
    return 0;
}
