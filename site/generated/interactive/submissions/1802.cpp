#include <bits/stdc++.h>
using namespace std;
using ull = unsigned long long;

int n;
vector<vector<bool>> adj;
vector<pair<int, int>> edges;
int edge_count;
mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());

void add_edge(int u, int v) {
    if (u == v) return;
    if (!adj[u][v]) {
        adj[u][v] = adj[v][u] = true;
        edges.push_back({u + 1, v + 1});
        edge_count++;
    }
}

void process_set(vector<int>& H) {
    int m = H.size();
    if (m == 0) return;
    if (m == 1) {
        H.clear();
        return;
    }
    if (m == 2) {
        int v0 = H[0], v1 = H[1];
        cout << "?";
        for (int i = 0; i < n; ++i) cout << ' ' << (i == v0 ? 1 : 0);
        cout << '\n';
        cout.flush();
        string s;
        cin >> s;
        if (s == "-1") exit(0);
        if (s[v1] == '1') {
            add_edge(v0, v1);
        }
        H.clear();
        return;
    }

    vector<int> nextH;
    int L = 0;
    vector<ull> codes;
    bool code_found = false;

    if (m <= 25) {
        L = m;
        codes.resize(m);
        for (int i = 0; i < m; ++i) codes[i] = 1ULL << i;
        code_found = true;
    } else {
        int L_start = max(20, (int)(2.0 * log2(m)) + 20);
        for (int Ltry = L_start; Ltry <= 60; ++Ltry) {
            for (int attempt = 0; attempt < 10; ++attempt) {
                vector<ull> cand(m);
                for (int i = 0; i < m; ++i) {
                    ull c = 0;
                    for (int b = 0; b < Ltry; ++b) {
                        if ((rng() & 3) == 0) c |= (1ULL << b);
                    }
                    cand[i] = c;
                }
                unordered_map<ull, pair<int, int>> mp;
                mp.reserve(m * m / 2 + 10);
                bool ok = true;
                for (int i = 0; i < m && ok; ++i) {
                    ull u = cand[i];
                    if (mp.count(u)) { ok = false; break; }
                    mp[u] = {i, i};
                    for (int j = i + 1; j < m; ++j) {
                        u = cand[i] | cand[j];
                        if (mp.count(u)) { ok = false; break; }
                        mp[u] = {i, j};
                    }
                }
                if (ok) {
                    codes = cand;
                    L = Ltry;
                    code_found = true;
                    break;
                }
            }
            if (code_found) break;
        }
    }

    if (!code_found) {
        for (int v : H) {
            cout << "?";
            for (int i = 0; i < n; ++i) cout << ' ' << (i == v ? 1 : 0);
            cout << '\n';
            cout.flush();
            string s;
            cin >> s;
            if (s == "-1") exit(0);
            for (int u = 0; u < n; ++u) {
                if (s[u] == '1') add_edge(v, u);
            }
        }
        H.clear();
        return;
    }

    unordered_map<ull, pair<int, int>> mp;
    mp.reserve(m * m / 2 + 10);
    for (int i = 0; i < m; ++i) {
        ull u = codes[i];
        mp[u] = {i, i};
        for (int j = i + 1; j < m; ++j) {
            u = codes[i] | codes[j];
            mp[u] = {i, j};
        }
    }

    vector<ull> z(m, 0);
    for (int b = 0; b < L; ++b) {
        vector<int> d(n, 0);
        for (int idx = 0; idx < m; ++idx) {
            if ((codes[idx] >> b) & 1) d[H[idx]] = 1;
        }
        cout << "?";
        for (int i = 0; i < n; ++i) cout << ' ' << d[i];
        cout << '\n';
        cout.flush();
        string s;
        cin >> s;
        if (s == "-1") exit(0);
        for (int idx = 0; idx < m; ++idx) {
            int v = H[idx];
            if (s[v] == '1') z[idx] |= (1ULL << b);
        }
    }

    for (int idx = 0; idx < m; ++idx) {
        int v = H[idx];
        ull zv = z[idx];
        if (zv == 0) continue;
        auto it = mp.find(zv);
        if (it != mp.end()) {
            int a = it->second.first;
            int b = it->second.second;
            add_edge(v, H[a]);
            if (a != b) add_edge(v, H[b]);
        } else {
            nextH.push_back(v);
        }
    }
    H = nextH;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cin >> n;
    adj.assign(n, vector<bool>(n, false));
    edge_count = 0;
    edges.clear();

    vector<int> H(n);
    iota(H.begin(), H.end(), 0);
    while (!H.empty()) {
        process_set(H);
    }

    bool valid = (edge_count == n - 1);
    if (valid) {
        vector<bool> vis(n, false);
        queue<int> q;
        q.push(0);
        vis[0] = true;
        int cnt = 0;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            cnt++;
            for (int v = 0; v < n; ++v) {
                if (adj[u][v] && !vis[v]) {
                    vis[v] = true;
                    q.push(v);
                }
            }
        }
        if (cnt != n) valid = false;
    }

    if (!valid) {
        edges.clear();
        adj.assign(n, vector<bool>(n, false));
        edge_count = 0;
        for (int v = 0; v < n; ++v) {
            cout << "?";
            for (int i = 0; i < n; ++i) cout << ' ' << (i == v ? 1 : 0);
            cout << '\n';
            cout.flush();
            string s;
            cin >> s;
            if (s == "-1") exit(0);
            for (int u = 0; u < n; ++u) {
                if (s[u] == '1') add_edge(v, u);
            }
        }
    }

    cout << "!" << '\n';
    for (auto [u, v] : edges) {
        cout << u << ' ' << v << '\n';
    }
    cout.flush();
    return 0;
}
