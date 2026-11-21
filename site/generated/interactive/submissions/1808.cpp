#include <bits/stdc++.h>
using namespace std;

int n;

int ask(int p) {
    cout << "? " << p << '\n';
    cout.flush();

    int r;
    cin >> r;
    if (r == -1) exit(0);
    return r;
}

vector<int> cyclePos(int p) {
    vector<int> b(n + 1);
    for (int d = 1; d <= n; ++d) {
        b[d] = ask(p);
    }
    return b;
}

void moveSteps(int p, int steps) {
    for (int i = 0; i < steps; ++i) {
        ask(p);
    }
}

int firstZero(const vector<int>& b) {
    for (int d = 1; d < n; ++d) {
        if (b[d] == 0) return d;
    }
    return -1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    vector<int> B1 = cyclePos(1);

    int holes = 0;
    for (int d = 1; d < n; ++d) {
        if (B1[d] == 0) ++holes;
    }

    vector<int> x(n + 1);

    if (holes == 0) {
        // The state is a permutation: create exactly one hole.
        ask(1);

        for (int p = 1; p <= n; ++p) {
            vector<int> b = cyclePos(p);
            int d = firstZero(b);
            x[p] = n - d;
        }
    } else if (holes == 1) {
        // Already exactly one hole. Reuse B1 for position 1.
        int d = firstZero(B1);
        x[1] = n - d;

        for (int p = 2; p <= n; ++p) {
            vector<int> b = cyclePos(p);
            d = firstZero(b);
            x[p] = n - d;
        }
    } else {
        // Reduce to exactly one hole.
        if (holes > 1 && B1[n] == 1) {
            int d = firstZero(B1);
            if (d != -1) {
                moveSteps(1, d);
                --holes;
            }
        }

        for (int p = 2; p <= n && holes > 1; ++p) {
            vector<int> b = cyclePos(p);

            if (b[n] == 1) {
                int d = firstZero(b);
                if (d != -1) {
                    moveSteps(p, d);
                    --holes;
                }
            }
        }

        // Safety fallback; should normally not be needed.
        while (holes > 1) {
            bool moved = false;

            for (int p = 1; p <= n && holes > 1; ++p) {
                vector<int> b = cyclePos(p);

                if (b[n] == 1) {
                    int d = firstZero(b);
                    if (d != -1) {
                        moveSteps(p, d);
                        --holes;
                        moved = true;
                        break;
                    }
                }
            }

            if (!moved) break;
        }

        // With exactly one hole, determine every position relative to it.
        for (int p = 1; p <= n; ++p) {
            vector<int> b = cyclePos(p);
            int d = firstZero(b);
            x[p] = n - d;
        }
    }

    int bestT = 0;
    int bestCost = INT_MAX;

    for (int t = 0; t < n; ++t) {
        int cost = 0;
        for (int p = 1; p <= n; ++p) {
            cost += (t - x[p] + n) % n;
        }

        if (cost < bestCost) {
            bestCost = cost;
            bestT = t;
        }
    }

    for (int p = 1; p <= n; ++p) {
        int steps = (bestT - x[p] + n) % n;
        moveSteps(p, steps);
    }

    cout << "!\n";
    cout.flush();

    return 0;
}
