#include <bits/stdc++.h>
using namespace std;

struct Tile {
    int end;
    int len;
    bool needL, needR;
    int next;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        string s;
        int K;
        cin >> s >> K;

        const int n = (int)s.size();
        vector<int> head(n, -1);
        vector<Tile> tiles;
        tiles.reserve(3 * n);

        auto addTile = [&](int l, int len, char target) {
            int r = l + len - 1;
            bool needL = (l > 0 && s[l - 1] == target);
            bool needR = (r + 1 < n && s[r + 1] == target);

            tiles.push_back({r, len, needL, needR, head[l]});
            head[l] = (int)tiles.size() - 1;
        };

        // Length 3: xxy, xyy, xyx.
        for (int i = 0; i + 2 < n; ++i) {
            char a = s[i], b = s[i + 1], c = s[i + 2];

            if (a == b && b != c)
                addTile(i, 3, a);                 // xxy
            if (a != b && b == c)
                addTile(i, 3, b);                 // xyy
            if (a == c && a != b)
                addTile(i, 3, a);                 // xyx
        }

        // Length 4: xxyx, xyxx.
        for (int i = 0; i + 3 < n; ++i) {
            char x = s[i];

            if (s[i + 1] == x && s[i + 2] != x && s[i + 3] == x)
                addTile(i, 4, x);                 // xxyx

            if (s[i + 1] != x && s[i + 2] == x && s[i + 3] == x)
                addTile(i, 4, x);                 // xyxx
        }

        // Length 5: xxyxx.
        for (int i = 0; i + 4 < n; ++i) {
            char x = s[i];
            if (s[i + 1] == x && s[i + 2] != x &&
                s[i + 3] == x && s[i + 4] == x) {
                addTile(i, 5, x);
            }
        }

        const long long NEG = -(1LL << 60);
        vector<long long> dp(n + 1), open0(n + 1), open1(n + 1);
        long long H[5];

        for (int p = 0; p <= 4; ++p) {
            fill(dp.begin(), dp.end(), NEG);
            fill(open0.begin(), open0.end(), NEG);
            fill(open1.begin(), open1.end(), NEG);

            dp[0] = 0;

            for (int i = 0; i < n; ++i) {
                // Leave character i unused.
                dp[i + 1] = max(dp[i + 1], dp[i]);

                for (int id = head[i]; id != -1; id = tiles[id].next) {
                    const Tile &t = tiles[id];

                    long long base = NEG;

                    // Start a new chain.
                    if (!t.needL)
                        base = max(base, dp[i]);

                    // Previous tile has needR = 0: always compatible.
                    base = max(base, open0[i]);

                    // Previous tile has needR = 1: compatible only if
                    // the current tile does not need the left neighbor.
                    if (!t.needL)
                        base = max(base, open1[i]);

                    if (base == NEG)
                        continue;

                    long long value = base + t.len - p;
                    int e = t.end + 1;

                    if (t.needR) {
                        open1[e] = max(open1[e], value);
                    } else {
                        open0[e] = max(open0[e], value);
                        dp[e] = max(dp[e], value);
                    }
                }
            }

            H[p] = dp[n];
        }

        // p = 5 gives H(5) + 5K = 5K.
        long long answer = 5LL * K;
        for (int p = 0; p <= 4; ++p)
            answer = min(answer, H[p] + 1LL * p * K);

        cout << answer << '\n';
    }

    return 0;
}
