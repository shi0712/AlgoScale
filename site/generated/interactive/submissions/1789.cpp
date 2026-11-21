#include <bits/stdc++.h>
using namespace std;

static int N, K;
static bool wantMax;

int ask(int a, int b) {
    cout << "? " << a << " " << b << '\n';
    cout.flush();

    int res;
    if (!(cin >> res)) exit(0);
    if (res == -1) exit(0);
    return res;
}

int combineVal(int a, int b) {
    if (a == 0) return b;
    if (b == 0) return a;

    int winner = ask(a, b);
    if (wantMax) return winner;
    return (winner == a ? b : a);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N >> K)) return 0;

    if (K == N) {
        cout << "!";
        for (int i = 1; i <= N; ++i) cout << ' ' << i;
        cout << '\n';
        cout.flush();
        return 0;
    }

    int M = N - K;
    bool doBottom = (M < K);
    wantMax = !doBottom;

    int need = doBottom ? M : K;

    int S = 1;
    while (S < N) S <<= 1;

    vector<int> tree(2 * S, 0);
    vector<int> pos(N + 1, 0);

    for (int i = 1; i <= N; ++i) {
        int p = S + i - 1;
        tree[p] = i;
        pos[i] = p;
    }

    for (int i = S - 1; i >= 1; --i) {
        tree[i] = combineVal(tree[i << 1], tree[i << 1 | 1]);
    }

    vector<int> chosen;
    chosen.reserve(need);

    for (int step = 0; step < need; ++step) {
        int x = tree[1];
        chosen.push_back(x);

        if (step + 1 == need) break;

        int p = pos[x];
        tree[p] = 0;
        p >>= 1;

        while (p >= 1) {
            tree[p] = combineVal(tree[p << 1], tree[p << 1 | 1]);
            p >>= 1;
        }
    }

    if (!doBottom) {
        cout << "!";
        for (int x : chosen) cout << ' ' << x;
        cout << '\n';
        cout.flush();
    } else {
        vector<char> excluded(N + 1, false);
        for (int x : chosen) excluded[x] = true;

        cout << "!";
        int printed = 0;
        for (int i = 1; i <= N && printed < K; ++i) {
            if (!excluded[i]) {
                cout << ' ' << i;
                ++printed;
            }
        }
        cout << '\n';
        cout.flush();
    }

    return 0;
}
