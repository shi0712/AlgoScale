#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    auto cmp = [&](int i, int j) -> int {
        cout << "? " << i << " " << j << '\n' << flush;
        char c;
        if (!(cin >> c)) exit(0);
        if (c == '-') exit(0);
        if (c == '<') return -1;
        if (c == '>') return 1;
        exit(0);
    };

    vector<int> ans(n + 1, 0);

    if (n == 1) {
        cout << "! 1\n" << flush;
        return 0;
    }

    if (n == 2) {
        int c = cmp(1, 2);
        if (c < 0) {
            ans[1] = 1;
            ans[2] = 2;
        } else {
            ans[1] = 2;
            ans[2] = 1;
        }
        cout << "! " << ans[1] << ' ' << ans[2] << '\n' << flush;
        return 0;
    }

    int p0 = n - 2;
    int p1 = n - 1;
    int p2 = n;

    int high, low;
    if (cmp(p0, p1) > 0) {
        high = p0;
        low = p1;
    } else {
        high = p1;
        low = p0;
    }

    vector<int> ord(3);
    if (cmp(high, p2) > 0) {
        if (cmp(low, p2) > 0) {
            ord[0] = high;
            ord[1] = low;
            ord[2] = p2;
        } else {
            ord[0] = high;
            ord[1] = p2;
            ord[2] = low;
        }
    } else {
        ord[0] = p2;
        ord[1] = high;
        ord[2] = low;
    }

    ans[ord[0]] = n;

    int a = ord[1];
    int b = ord[2];

    for (int v = n - 1; v >= 3; --v) {
        int x = v - 2;

        if (cmp(x, a) > 0) {
            ans[x] = v;
        } else {
            ans[a] = v;

            if (cmp(x, b) > 0) {
                a = x;
            } else {
                a = b;
                b = x;
            }
        }
    }

    ans[a] = 2;
    ans[b] = 1;

    cout << "!";
    for (int i = 1; i <= n; ++i) {
        cout << ' ' << ans[i];
    }
    cout << '\n' << flush;

    return 0;
}
