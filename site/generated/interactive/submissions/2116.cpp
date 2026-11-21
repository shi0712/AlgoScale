#include <bits/stdc++.h>
using namespace std;

int n;
vector<vector<int>> qCache;   // -1 = unknown, otherwise 0/1
vector<vector<int>> comp;     // -1 = unknown, 0 = p[i] < p[j], 1 = p[i] > p[j]

int ask(int l, int r) {
    if (qCache[l][r] != -1) return qCache[l][r];
    cout << "? " << l << ' ' << r << endl;
    cout.flush();
    int ans;
    if (!(cin >> ans)) exit(0);
    if (ans == -1) exit(0);
    qCache[l][r] = ans;
    return ans;
}

// returns true iff p[i] > p[j]
bool isGreater(int i, int j) {
    if (i == j) return false;
    if (i > j) return !isGreater(j, i);          // use symmetry
    if (comp[i][j] != -1) return comp[i][j];
    bool res;
    if (i + 1 == j) {                            // adjacent: single query
        res = (ask(i, j) == 1);
    } else {
        int a = ask(i, j);
        int b = ask(i + 1, j);
        int c = ask(i, j - 1);
        int d = ask(i + 1, j - 1);
        int inv = (a ^ b) ^ (c ^ d);
        res = (inv == 1);
    }
    comp[i][j] = res ? 1 : 0;
    comp[j][i] = res ? 0 : 1;
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;

    qCache.assign(n + 2, vector<int>(n + 2, -1));
    comp.assign(n + 2, vector<int>(n + 2, -1));

    vector<int> pos(n);
    iota(pos.begin(), pos.end(), 1);          // positions 1 … n

    sort(pos.begin(), pos.end(),
         [&](int a, int b) { return !isGreater(a, b); }); // a < b ?

    vector<int> ans(n + 1);
    for (int i = 0; i < n; ++i) ans[pos[i]] = i + 1;

    cout << "!";
    for (int i = 1; i <= n; ++i) cout << ' ' << ans[i];
    cout << endl;
    cout.flush();
    return 0;
}
