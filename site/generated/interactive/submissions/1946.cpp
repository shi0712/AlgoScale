#include <bits/stdc++.h>
using namespace std;

static int n;
static char ask(int i, int j) {
    cout << "? " << i << ' ' << j << '\n';
    cout.flush();
    char c;
    if (!(cin >> c)) exit(0);
    if (c != '<' && c != '>') exit(0);
    return c;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (n <= 1) {
        cout << "! 1\n";
        cout.flush();
        return 0;
    }

    vector<int> p1(n + 1, 0), p2(n + 1, 0);

    // top1/top2 are the positions of the smallest and second smallest
    // values in the already processed suffix.
    int top1 = n, top2 = 0;
    p1[n] = n;

    // Hysteresis predictor: target e=0 with strategy A, or e=2 with
    // strategy B.  The difference of observed counts only changes when
    // e is 0 or 2, so alternating 0/2 patterns do not cause thrashing.
    int balance = 0;
    const int HYST = 6;
    bool useB = false;

    for (int i = n - 1; i >= 1; --i) {
        int xpos = i;
        if (i == n - 1) {
            // Only one element to the right: compare with it.
            char r = ask(xpos, top1);
            int oldTop1 = top1;
            if (r == '<') {
                // x is the new minimum
                top1 = i;
                top2 = oldTop1;
            } else {
                top1 = oldTop1;
                top2 = i;
            }
            p1[i] = top1;
            p2[i] = top2;
            continue;
        }

        if (!useB) {
            // Strategy A: compare with the smallest first.
            char r = ask(xpos, top1);
            if (r == '<') {
                // x is the new smallest (e_i = 0).
                int oldTop1 = top1;
                top1 = i;
                top2 = oldTop1;
                --balance;
            } else {
                // x is either the second smallest or the third (new) one.
                char r2 = ask(xpos, top2);
                if (r2 == '<') {
                    // x is the second smallest (e_i = 1).
                    top2 = i;
                } else {
                    // x is the third smallest (e_i = 2); top two unchanged.
                    ++balance;
                    if (balance > HYST) useB = true;
                }
            }
        } else {
            // Strategy B: compare with the second smallest first.
            char r = ask(xpos, top2);
            if (r == '>') {
                // x is larger than both; it is the third (new) one.
                ++balance;
            } else {
                // x is not larger than the second smallest; compare with the smallest.
                char r2 = ask(xpos, top1);
                if (r2 == '<') {
                    // x is the new smallest.
                    int oldTop1 = top1;
                    top1 = i;
                    top2 = oldTop1;
                    --balance;
                    if (balance < -HYST) useB = false;
                } else {
                    // x is the second smallest.
                    top2 = i;
                }
            }
        }
        p1[i] = top1;
        p2[i] = top2;
    }

    // Reconstruct from left to right.  At step i, the two smallest values
    // of suffix i are exactly the two smallest not-yet-used values.
    vector<int> ans(n + 1, 0);
    int a = 1, b = 2; // sorted two smallest unused values

    for (int i = 1; i <= n - 1; ++i) {
        int val;
        if (i == p1[i]) {
            val = a;
            // remove a, insert i+2 if it exists
            a = b;
            b = (i <= n - 2) ? (i + 2) : 0;
        } else if (i == p2[i]) {
            val = b;
            // remove b, insert i+2
            b = (i <= n - 2) ? (i + 2) : 0;
        } else {
            // i is not one of the two smallest positions, so pi_i is the
            // third smallest value of suffix i, namely i+2.
            val = i + 2;
        }
        ans[i] = val;
    }
    ans[n] = a; // exactly one value remains

    cout << "!";
    for (int i = 1; i <= n; ++i) cout << ' ' << ans[i];
    cout << '\n';
    cout.flush();
    return 0;
}
