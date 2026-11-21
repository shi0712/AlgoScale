#include <bits/stdc++.h>
using namespace std;

bool query(int i, int j) {
    cout << "? " << i << " " << j << '\n';
    cout.flush();

    char c;
    if (!(cin >> c)) exit(0);
    if (c == '<') return true;
    if (c == '>') return false;

    exit(0);
    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> top1(n + 1), top2(n + 1), top3(n + 1);
    vector<int> top(3, 0);
    int sz = 0;

    // Adaptive insertion strategy:
    // favor = 0: compare with the current smallest first (saves a query when new is smallest)
    // favor = 1: compare with the current second smallest first (saves a query when new is third)
    int favor = 0;
    int counter = 0;
    const int K = 8;

    for (int i = n; i >= 1; --i) {
        int x = i;

        if (sz == 0) {
            top[0] = x;
            sz = 1;
        } else if (sz == 1) {
            if (query(x, top[0])) {
                top[1] = top[0];
                top[0] = x;
            } else {
                top[1] = x;
            }
            sz = 2;
        } else if (sz == 2) {
            if (favor == 0) {
                if (query(x, top[0])) {
                    top[2] = top[1];
                    top[1] = top[0];
                    top[0] = x;
                } else {
                    if (query(x, top[1])) {
                        top[2] = top[1];
                        top[1] = x;
                    } else {
                        top[2] = x;
                    }
                }
            } else {
                if (query(x, top[1])) {
                    if (query(x, top[0])) {
                        top[2] = top[1];
                        top[1] = top[0];
                        top[0] = x;
                    } else {
                        top[2] = top[1];
                        top[1] = x;
                    }
                } else {
                    top[2] = x;
                }
            }
            sz = 3;
        } else { // sz == 3
            // For a valid shuffle, position i is always among the three smallest
            // values of suffix i..n, so x is guaranteed to be smaller than top[2].
            if (favor == 0) {
                if (query(x, top[0])) {
                    top[2] = top[1];
                    top[1] = top[0];
                    top[0] = x;
                } else {
                    if (query(x, top[1])) {
                        top[2] = top[1];
                        top[1] = x;
                    } else {
                        top[2] = x;
                    }
                }
            } else {
                if (query(x, top[1])) {
                    if (query(x, top[0])) {
                        top[2] = top[1];
                        top[1] = top[0];
                        top[0] = x;
                    } else {
                        top[2] = top[1];
                        top[1] = x;
                    }
                } else {
                    top[2] = x;
                }
            }
        }

        top1[i] = top[0];
        top2[i] = (sz >= 2 ? top[1] : 0);
        top3[i] = (sz >= 3 ? top[2] : 0);

        int rank = 0;
        if (top[0] == x) rank = 1;
        else if (sz >= 2 && top[1] == x) rank = 2;
        else if (sz >= 3 && top[2] == x) rank = 3;

        if (favor == 0) {
            if (rank == 3) ++counter;
            else counter = 0;
            if (counter >= K) {
                favor = 1;
                counter = 0;
            }
        } else {
            if (rank == 1) ++counter;
            else counter = 0;
            if (counter >= K) {
                favor = 0;
                counter = 0;
            }
        }
    }

    vector<int> ans(n + 1);
    set<int> remaining;
    for (int v = 1; v <= n; ++v) remaining.insert(v);

    for (int i = 1; i <= n; ++i) {
        int rank;
        if (top1[i] == i) rank = 1;
        else if (top2[i] == i) rank = 2;
        else rank = 3;

        auto it = remaining.begin();
        advance(it, rank - 1);
        ans[i] = *it;
        remaining.erase(it);
    }

    cout << "! ";
    for (int i = 1; i <= n; ++i) {
        if (i > 1) cout << ' ';
        cout << ans[i];
    }
    cout << '\n';
    cout.flush();

    return 0;
}
