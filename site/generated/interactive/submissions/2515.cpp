#include <bits/stdc++.h>
using namespace std;

int n;
vector<vector<signed char>> cache;

int query(int l, int r) {
    if (l >= r) return 0;
    if (cache[l][r] != -1) return cache[l][r];
    cout << "? " << l << " " << r << "\n";
    cout.flush();
    int x;
    cin >> x;
    if (x == -1) exit(0);
    cache[l][r] = (signed char)x;
    return x;
}

bool less_pos(int i, int j) {
    if (i == j) return false;
    if (i < j) {
        int x = query(i, j) ^ query(i + 1, j) ^ query(i, j - 1) ^ query(i + 1, j - 1);
        return x == 0;
    } else {
        int x = query(j, i) ^ query(j + 1, i) ^ query(j, i - 1) ^ query(j + 1, i - 1);
        return x == 1;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;
    if (n == 1) {
        cout << "! 1\n";
        cout.flush();
        return 0;
    }

    cache.assign(n + 2, vector<signed char>(n + 2, -1));

    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(), less_pos);

    vector<int> ans(n + 1);
    for (int i = 0; i < n; ++i) {
        ans[order[i]] = i + 1;
    }

    cout << "!";
    for (int i = 1; i <= n; ++i) {
        cout << " " << ans[i];
    }
    cout << "\n";
    cout.flush();
    return 0;
}
