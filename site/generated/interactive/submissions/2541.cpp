#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    cin >> n;
    vector<pair<int,int>> edges;
    for (int v = 1; v <= n-2; ++v) {
        cout << "? ";
        for (int i = 1; i <= n; ++i) {
            if (i == v) cout << "1 ";
            else cout << "0 ";
        }
        cout << endl;
        string s;
        cin >> s;
        for (int i = 1; i <= n; ++i) {
            if (s[i-1] == '1' && i > v) {
                edges.emplace_back(v, i);
            }
        }
    }
    if ((int)edges.size() == n-2) {
        edges.emplace_back(n-1, n);
    }
    cout << "!" << endl;
    for (auto [a,b] : edges) {
        cout << a << " " << b << endl;
    }
    return 0;
}
