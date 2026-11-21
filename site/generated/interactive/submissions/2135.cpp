#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    if (!(cin >> n)) return 0;
    
    vector<int> d(n + 1, 0);
    set<pair<int,int>> edges;
    
    for (int i = 1; i <= n; ++i) {
        fill(d.begin() + 1, d.end(), 0);
        d[i] = 1;
        cout << "?";
        for (int j = 1; j <= n; ++j) {
            cout << " " << d[j];
        }
        cout << "\n";
        cout.flush();
        
        string s;
        // read until we have at least n characters (handles spaced output)
        while ((int)s.size() < n) {
            string token;
            if (!(cin >> token)) return 0; // EOF or error
            if (token == "-1") return 0;   // judge error
            s += token;
        }
        if ((int)s.size() > n) s.resize(n);
        for (int j = 1; j <= n; ++j) {
            if (j == i) continue;
            if (s[j-1] == '1') {
                int a = i, b = j;
                if (a > b) swap(a, b);
                edges.emplace(a, b);
            }
        }
    }
    
    cout << "!\n";
    for (auto &e : edges) {
        cout << e.first << " " << e.second << "\n";
    }
    cout.flush();
    return 0;
}
