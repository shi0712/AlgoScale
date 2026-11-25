#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    auto query = [&](int p) {
        cout << "? " << p << '\n' << flush;
        int response;
        if (!(cin >> response)) exit(0);
        if (response == -1) exit(0);
        return response;
    };

    int anchor = -1;
    for (int p = 1; p <= n; ++p) {
        int steps = 0;
        while (query(p) != 0) {
            ++steps;
        }
        ++steps;
        if (steps == n) {
            anchor = p;
            break;
        }
    }
    if (anchor == -1) anchor = n;
    query(anchor);

    vector<int> value(n + 1, -1);
    value[anchor] = 1 % n;
    int empty = 0;

    vector<int> probes;
    for (int p = 1; p <= n; ++p) {
        if (p != anchor) probes.push_back(p);
    }

    for (int i = 0; i + 1 < (int)probes.size(); ++i) {
        int p = probes[i];
        int steps = 0;
        while (query(p) != 0) {
            ++steps;
        }
        ++steps;

        int oldValue = (empty - steps) % n;
        if (oldValue < 0) oldValue += n;
        value[p] = empty;

        if (oldValue == value[anchor]) {
            query(anchor);
            empty = value[anchor];
            value[anchor] = (value[anchor] + 1) % n;
        } else {
            empty = oldValue;
        }
    }

    vector<int> count(n, 0);
    for (int p = 1; p <= n; ++p) {
        if (value[p] != -1) ++count[value[p]];
    }
    for (int v = 0; v < n; ++v) {
        if (v == empty) continue;
        int expected = 1 + (v == value[anchor]);
        if (count[v] < expected) {
            value[probes.back()] = v;
            break;
        }
    }

    int target = 0;
    long long bestCost = LLONG_MAX;
    for (int t = 0; t < n; ++t) {
        long long cost = 0;
        for (int p = 1; p <= n; ++p) {
            cost += (t - value[p] + n) % n;
        }
        if (cost < bestCost) {
            bestCost = cost;
            target = t;
        }
    }

    for (int p = 1; p <= n; ++p) {
        int steps = (target - value[p] + n) % n;
        while (steps--) query(p);
    }

    cout << "!\n" << flush;
    return 0;
}
