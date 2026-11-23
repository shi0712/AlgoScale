#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K;
    if (!(cin >> N >> K)) return 0;

    bool chooseWeakest = (K > N - K);
    int M = chooseWeakest ? N - K : K;

    if (M == 0) {
        cout << '!';
        for (int i = 1; i <= N; ++i) cout << ' ' << i;
        cout << '\n' << flush;
        return 0;
    }

    auto compare = [&](int a, int b) -> int {
        cout << "? " << a << ' ' << b << '\n' << flush;
        int winner;
        if (!(cin >> winner)) exit(0);
        if (winner != a && winner != b) exit(0);
        return chooseWeakest ? (a + b - winner) : winner;
    };

    auto advance = [&](int a, int b) -> int {
        if (a == 0) return b;
        if (b == 0) return a;
        return compare(a, b);
    };

    int base = 1;
    while (base < N) base <<= 1;
    vector<int> tree(2 * base, 0);

    for (int i = 1; i <= N; ++i) tree[base + i - 1] = i;
    for (int i = base - 1; i >= 1; --i)
        tree[i] = advance(tree[2 * i], tree[2 * i + 1]);

    vector<int> selected;
    selected.reserve(M);

    for (int i = 0; i < M; ++i) {
        int v = tree[1];
        selected.push_back(v);

        if (i + 1 == M) break;

        int p = base + v - 1;
        tree[p] = 0;
        for (p >>= 1; p; p >>= 1)
            tree[p] = advance(tree[2 * p], tree[2 * p + 1]);
    }

    cout << '!';
    if (!chooseWeakest) {
        for (int v : selected) cout << ' ' << v;
    } else {
        vector<bool> excluded(N + 1, false);
        for (int v : selected) excluded[v] = true;
        for (int i = 1; i <= N; ++i)
            if (!excluded[i]) cout << ' ' << i;
    }
    cout << '\n' << flush;
    return 0;
}
