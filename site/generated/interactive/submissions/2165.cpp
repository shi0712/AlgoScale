#include <bits/stdc++.h>
using namespace std;

int n;

int ask1(const vector<int>& nodes) {
    cout << "? 1 " << (int)nodes.size();
    for (int x : nodes) cout << ' ' << x;
    cout << endl;

    int resp;
    if (!(cin >> resp)) exit(0);
    return resp;
}

void ask2(int u) {
    cout << "? 2 " << u << endl;
}

vector<int> ask_group(const vector<int>& nodes, int offset) {
    int s = (int)nodes.size();
    vector<int> q;
    int W = 0;

    for (int j = 0; j < s; ++j) {
        int w = 1 << j;
        W += w;
        for (int t = 0; t < w; ++t) q.push_back(nodes[j]);
    }

    int Y = ask1(q);
    int R = Y - offset * W;
    int val = (R + W) / 2;

    vector<int> b(s);
    for (int j = 0; j < s; ++j) {
        b[j] = (val & (1 << j)) ? 1 : -1;
    }
    return b;
}

void finish(const vector<int>& ans) {
    cout << "!";
    for (int i = 1; i <= n; ++i) cout << ' ' << ans[i];
    cout << endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
    }

    if (n == 2) {
        int T = ask1({1});
        vector<int> ans(3, 0);

        if (T == 2) {
            ans[1] = 1;
            ans[2] = 1;
        } else if (T == -2) {
            ans[1] = -1;
            ans[2] = -1;
        } else if (T == 0) {
            int b = ask1({2});
            ans[2] = b;
            ans[1] = -b;
        } else {
            ans[1] = T;
            int f2 = ask1({2});
            ans[2] = f2 - T;
        }

        finish(ans);
        return 0;
    }

    int g = 0;
    while ((1 << (g + 1)) - 1 <= n) ++g;

    vector<int> leaves;
    for (int i = 2; i <= n; ++i) leaves.push_back(i);
    int m = n - 1;

    int T = ask1({1});
    vector<int> ans(n + 1, 0);

    auto decode_all = [&](int offset) {
        for (int start = 0; start < m; start += g) {
            int s = min(g, m - start);
            vector<int> nodes(leaves.begin() + start, leaves.begin() + start + s);
            vector<int> b = ask_group(nodes, offset);
            for (int j = 0; j < s; ++j) ans[nodes[j]] = b[j];
        }
    };

    if (T % 2 != 0) {
        // Root is node 1.
        ans[1] = T;
        decode_all(T);
    } else if (T == 2 || T == -2) {
        // Root is a leaf, and f(1) = +/-2.
        // Toggle center: then f(1) becomes 0 and every leaf f(i) equals its value.
        ask2(1);
        ans[1] = (T == 2 ? -1 : 1);
        decode_all(0);
    } else {
        // T == 0. Root is a leaf, and initially leaf f(i) equals its value.
        decode_all(0);

        bool all_same = true;
        for (int i = 3; i <= n; ++i) {
            if (ans[i] != ans[2]) {
                all_same = false;
                break;
            }
        }

        if (all_same) {
            ans[1] = -ans[2];
            finish(ans);
            return 0;
        }

        // Toggle center. Then f(1) = 2 * value(root leaf), which gives the final center value.
        ask2(1);
        int T2 = ask1({1});
        ans[1] = T2 / 2;
    }

    finish(ans);
    return 0;
}
