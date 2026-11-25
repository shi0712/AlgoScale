#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int N, M;
    cin >> N >> M;
    vector<vector<int>> groups;
    int idx = 1;
    for (int i = 0; i < 89; ++i) {
        vector<int> g;
        for (int j = 0; j < 11; ++j) g.push_back(idx++);
        groups.push_back(g);
    }
    vector<int> g;
    while (idx <= N) g.push_back(idx++);
    if (!g.empty()) groups.push_back(g);
    int G = groups.size();
    vector<vector<int>> rel(G);
    vector<int> a(G), b(G);
    for (int i = 0; i < G; ++i) {
        int sz = groups[i].size();
        rel[i].resize(sz, 0);
        for (int j = 0; j < sz - 1; ++j) {
            cout << "? " << groups[i][j] << " " << groups[i][j+1] << "\n";
            cout.flush();
            int resp;
            if (!(cin >> resp) || resp == -1) return 0;
            rel[i][j+1] = rel[i][j] ^ resp;
        }
        int ones = 0;
        for (int x : rel[i]) if (x == 1) ++ones;
        a[i] = ones;
        b[i] = sz - ones;
    }
    vector<vector<bool>> dp(G + 1, vector<bool>(M + 1, false));
    vector<vector<int>> choice(G + 1, vector<int>(M + 1, -1));
    dp[0][0] = true;
    for (int i = 0; i < G; ++i) {
        for (int j = 0; j <= M; ++j) {
            if (!dp[i][j]) continue;
            if (j + a[i] <= M) {
                dp[i+1][j + a[i]] = true;
                choice[i+1][j + a[i]] = 0;
            }
            if (j + b[i] <= M) {
                dp[i+1][j + b[i]] = true;
                choice[i+1][j + b[i]] = 1;
            }
        }
    }
    vector<int> ans;
    int cur = M;
    for (int i = G; i >= 1; --i) {
        int c = choice[i][cur];
        if (c == 0) {
            for (int j = 0; j < (int)groups[i-1].size(); ++j)
                if (rel[i-1][j] == 1) ans.push_back(groups[i-1][j]);
            cur -= a[i-1];
        } else {
            for (int j = 0; j < (int)groups[i-1].size(); ++j)
                if (rel[i-1][j] == 0) ans.push_back(groups[i-1][j]);
            cur -= b[i-1];
        }
    }
    cout << "!";
    for (int x : ans) cout << " " << x;
    cout << "\n";
    cout.flush();
    return 0;
}
