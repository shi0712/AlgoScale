#include <bits/stdc++.h>
using namespace std;

map<array<int, 3>, int> memo;

int ask(int i, int j, int k) {
    array<int, 3> key = {i, j, k};
    sort(key.begin(), key.end());

    auto it = memo.find(key);
    if (it != memo.end()) return it->second;

    cout << "? " << i << " " << j << " " << k << endl;
    int x;
    cin >> x;
    if (x == -1) exit(0);

    memo[key] = x;
    return x;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    if (n == 4) {
        vector<int> r(5), id = {1, 2, 3, 4};

        for (int i = 1; i <= 4; ++i) {
            vector<int> v;
            for (int j = 1; j <= 4; ++j) {
                if (j != i) v.push_back(j);
            }
            r[i] = ask(v[0], v[1], v[2]);
        }

        sort(id.begin(), id.end(), [&](int x, int y) {
            if (r[x] != r[y]) return r[x] < r[y];
            return x < y;
        });

        cout << "! " << id[0] << " " << id[1] << endl;
        return 0;
    }

    vector<int> v(n + 1);
    bool allEqual = true;

    for (int i = 3; i <= n; ++i) {
        v[i] = ask(1, 2, i);
        if (i > 3 && v[i] != v[3]) allEqual = false;
    }

    if (!allEqual) {
        int p = 3;
        for (int i = 4; i <= n; ++i) {
            if (v[i] > v[p]) p = i;
        }

        bool eq = true;
        int first = -1, best = -1, r = -1;

        for (int i = 2; i <= n; ++i) {
            if (i == p) continue;

            int w = ask(p, 1, i);

            if (first == -1) {
                first = best = w;
                r = i;
            } else {
                if (w != first) eq = false;
                if (w > best) {
                    best = w;
                    r = i;
                }
            }
        }

        if (eq) {
            cout << "! " << p << " " << 1 << endl;
        } else {
            cout << "! " << p << " " << r << endl;
        }
        return 0;
    }

    // All queries (1, 2, i) gave the same result.
    int W = ask(1, 3, 2);
    int r = 2;

    for (int i = 4; i <= n; ++i) {
        int cur = ask(1, 3, i);
        if (cur > W) {
            W = cur;
            r = i;
        }
    }

    if (r == 2) {
        cout << "! " << 1 << " " << 2 << endl;
        return 0;
    }

    int Q = ask(2, 3, r);

    if (Q < W) {
        cout << "! " << 1 << " " << r << endl;
    } else {
        cout << "! " << 3 << " " << r << endl;
    }

    return 0;
}
