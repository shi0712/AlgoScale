#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K;
    if (!(cin >> N >> K)) return 0;

    if (K == N) {
        cout << '!';
        for (int i = 1; i <= N; ++i) cout << ' ' << i;
        cout << endl;
        return 0;
    }

    const bool findBottom = (N - K < K);
    const int needed = findBottom ? N - K : K;

    int size = 1;
    while (size < N) size <<= 1;

    vector<int> tree(2 * size, 0);
    vector<int> leafPosition(N + 1);

    for (int i = 1; i <= N; ++i) {
        tree[size + i - 1] = i;
        leafPosition[i] = size + i - 1;
    }

    auto ask = [&](int a, int b) -> int {
        cout << "? " << a << ' ' << b << endl;
        int winner;
        if (!(cin >> winner) || winner == -1) exit(0);
        if (winner != a && winner != b) exit(0);
        return winner;
    };

    auto combine = [&](int a, int b) -> int {
        if (a == 0) return b;
        if (b == 0) return a;

        int winner = ask(a, b);
        if (!findBottom) return winner;
        return winner == a ? b : a;
    };

    for (int v = size - 1; v >= 1; --v) {
        tree[v] = combine(tree[v << 1], tree[v << 1 | 1]);
    }

    vector<int> selected;
    selected.reserve(needed);

    for (int count = 0; count < needed - 1; ++count) {
        int current = tree[1];
        selected.push_back(current);

        int position = leafPosition[current];
        tree[position] = 0;

        for (position >>= 1; position >= 1; position >>= 1) {
            tree[position] = combine(tree[position << 1],
                                     tree[position << 1 | 1]);
            if (position == 1) break;
        }
    }

    selected.push_back(tree[1]);

    vector<int> answer;
    if (!findBottom) {
        answer = selected;
    } else {
        vector<char> removed(N + 1, false);
        for (int x : selected) removed[x] = true;
        for (int x = 1; x <= N; ++x) {
            if (!removed[x]) answer.push_back(x);
        }
    }

    cout << '!';
    for (int x : answer) cout << ' ' << x;
    cout << endl;

    return 0;
}
