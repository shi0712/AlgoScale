#include <bits/stdc++.h>
using namespace std;

int n;
int vis;
bool finished = false;

void flipInterval(int i, int j) {
    if (finished) return;

    cout << i << ' ' << j << '\n';
    cout.flush();

    int res;
    if (!(cin >> res)) {
        exit(0);
    }
    if (res == -1) {
        exit(0);
    }

    vis = res;
    if (vis == n) {
        finished = true;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;
    cin >> vis;

    if (vis == n) return 0;

    for (int p = 1; p <= n && !finished; ++p) {
        int k = n - p + 1;
        int initialVis = vis;
        int bestVis = vis;
        int bestS = 0;
        int curS = 0;
        bool skip = false;

        int limit = 1 << k;

        for (int s = 1; s < limit && !finished; ++s) {
            int gray = s ^ (s >> 1);
            int prev = (s - 1) ^ ((s - 1) >> 1);
            int diff = gray ^ prev;
            int b = __builtin_ctz(diff);

            flipInterval(p, p + b);
            if (finished) break;

            curS = gray;

            if (s == 1) {
                if (vis < initialVis) {
                    // Cell p was visible; restore the singleton barrier.
                    flipInterval(p, p);
                    if (finished) break;
                    curS = 0;
                    skip = true;
                    break;
                } else if (vis > initialVis) {
                    // The singleton barrier was the only active interval covering p.
                    // Now it is off, so all intervals starting at p are inactive.
                    skip = true;
                    break;
                }
            }

            if (vis > bestVis) {
                bestVis = vis;
                bestS = gray;
            }
        }

        if (finished) break;

        if (!skip) {
            int delta = curS ^ bestS;

            for (int b = 0; b < k && !finished; ++b) {
                if ((delta >> b) & 1) {
                    flipInterval(p, p + b);
                }
            }
        }
    }

    return 0;
}
