#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 2) {
        cout << "! 1 2" << endl;
        return 0;
    }

    vector<int> rv(n - 1), ru(n - 1);
    vector<char> alive(n + 1, 1);

    for (int i = 0; i < n - 1; ++i) {
        cout << "? 0" << endl;
        int v, u;
        if (!(cin >> v >> u)) return 0;
        if (v == 0) break; // should not happen before one vertex remains
        rv[i] = v;
        ru[i] = u;
        alive[v] = 0;
    }

    int last = 0;
    for (int i = 1; i <= n; ++i) {
        if (alive[i]) {
            last = i;
            break;
        }
    }

    vector<int> path;
    path.push_back(last);

    for (int i = n - 2; i >= 0; --i) {
        int v = rv[i];
        int u = ru[i];

        if (u == 0) {
            path.insert(path.begin(), v);
            continue;
        }

        int pos = -1;
        for (int p = 0; p <= (int)path.size(); ++p) {
            bool bad = false;
            if (p > 0 && path[p - 1] == u) bad = true;
            if (p < (int)path.size() && path[p] == u) bad = true;
            if (!bad) {
                pos = p;
                break;
            }
        }

        if (pos == -1) pos = 0; // fallback, should not be needed
        path.insert(path.begin() + pos, v);
    }

    cout << "!";
    for (int x : path) cout << " " << x;
    cout << endl;

    return 0;
}
