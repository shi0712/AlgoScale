#include <bits/stdc++.h>
using namespace std;

const long long NEG_INF = -1000000000000000LL; // -1e15

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<long long> p(n + 1, 0);
    vector<long long> b(n + 1, 0);

    // Query all singletons
    for (int i = 1; i <= n; ++i) {
        cout << "? " << i << " " << i << "\n";
        cout.flush();
        long long val;
        cin >> val;
        if (val == -1) return 0;
        p[i] = max(0LL, val);
    }

    struct Block {
        int l, r;
        long long sum;
    };
    vector<Block> blocks;

    // Identify positive blocks
    int i = 1;
    while (i <= n) {
        if (p[i] > 0) {
            int l = i;
            long long sum = 0;
            while (i <= n && p[i] > 0) {
                b[i] = p[i];
                sum += p[i];
                ++i;
            }
            int r = i - 1;
            blocks.push_back({l, r, sum});
        } else {
            ++i;
        }
    }

    // Process gaps between consecutive positive blocks
    for (size_t idx = 1; idx < blocks.size(); ++idx) {
        const Block &left = blocks[idx - 1];
        const Block &right = blocks[idx];

        int l = left.l;
        int r = right.r;

        cout << "? " << l << " " << r << "\n";
        cout.flush();
        long long M;
        cin >> M;
        if (M == -1) return 0;

        long long L = left.sum;
        long long R = right.sum;
        long long maxLR = max(L, R);

        if (M > maxLR) {
            long long T = M - L - R; // total of the nonpositive gap
            int gapStart = left.r + 1;
            int gapEnd = right.l - 1;
            if (gapStart <= gapEnd) {
                b[gapStart] = T;
                // other entries in the gap remain 0
            }
        } else {
            // Gap is irrelevant; make it sufficiently negative.
            int gapStart = left.r + 1;
            int gapEnd = right.l - 1;
            if (gapStart <= gapEnd) {
                b[gapStart] = NEG_INF;
                // other entries remain 0
            }
        }
    }

    // Leading and trailing nonpositive entries are already 0.

    cout << "!";
    for (int i = 1; i <= n; ++i) {
        cout << " " << b[i];
    }
    cout << "\n";
    cout.flush();

    return 0;
}
