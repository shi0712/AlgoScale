#include <bits/stdc++.h>
using namespace std;

int n, half;
mt19937_64 rng;

void answer(int x, int y) {
    cout << "! " << x << ' ' << y << '\n' << flush;
    int ok;
    if (!(cin >> ok)) exit(0);
    exit(0);
}

int query(int i, int j) {
    cout << "? " << i << ' ' << j << '\n' << flush;
    int res;
    if (!(cin >> res)) exit(0);
    if (res == -1) exit(0);

    // If gcd is greater than (n-1)/2, the only possible positive multiple
    // of it in [0, n-1] is itself, so one of the two values must be 0.
    if (res > half) {
        answer(i, j);
    }
    return res;
}

void handle_small(vector<int>& C, int a, int v) {
    if (C.empty()) {
        answer(a, a);
        return;
    }
    if (C.size() == 1) {
        answer(a, C[0]);
        return;
    }
    if (C.size() == 2) {
        answer(C[0], C[1]);
        return;
    }
    if (C.size() == 3 && 2LL * v > half) {
        // In this case both non-zero candidates are > half.
        // If the queried pair does not contain zero, the remaining one does.
        query(C[0], C[1]);
        answer(C[2], C[0]);
        return;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    half = (n - 1) / 2;

    rng.seed(
        (uint64_t)chrono::steady_clock::now().time_since_epoch().count() ^
        (uint64_t)n
    );

    vector<int> resp(n + 1, -1);

    uniform_int_distribution<int> distRoot(1, n);
    int a = distRoot(rng);

    vector<int> order;
    order.reserve(n - 1);
    for (int i = 1; i <= n; ++i) {
        if (i != a) order.push_back(i);
    }
    shuffle(order.begin(), order.end(), rng);

    int v = 0;
    for (int j : order) {
        int res = query(a, j);
        resp[j] = res;
        if (res > v) v = res;
    }

    vector<int> C;
    C.reserve(order.size());
    for (int j : order) {
        if (resp[j] == v) C.push_back(j);
    }

    handle_small(C, a, v);

    int steps = 0;
    while (true) {
        if (++steps > 100) {
            answer(a, a);
            return 0;
        }

        uniform_int_distribution<int> distC(0, (int)C.size() - 1);
        int b = C[distC(rng)];

        vector<int> order2;
        order2.reserve(C.size() - 1);
        for (int d : C) {
            if (d != b) order2.push_back(d);
        }
        shuffle(order2.begin(), order2.end(), rng);

        int M = 0;
        for (int d : order2) {
            int res = query(b, d);
            resp[d] = res;
            if (res > M) M = res;
        }

        vector<int> newC;
        newC.reserve(C.size());
        for (int d : C) {
            if (d == b) continue;
            if (resp[d] == M) newC.push_back(d);
        }

        a = b;
        v = M;
        C.swap(newC);

        handle_small(C, a, v);
    }

    return 0;
}
