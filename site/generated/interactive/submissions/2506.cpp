#include <bits/stdc++.h>
using namespace std;

struct Entry {
    int key, l, k;
    bool operator>(const Entry& other) const {
        if (key != other.key) return key > other.key;
        if (l != other.l) return l < other.l;
        return k > other.k;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<vector<int>> g(n + 1, vector<int>(n + 1));
    vector<int> px(n * n + 1), py(n * n + 1);
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= n; ++j) {
            cin >> g[i][j];
            px[g[i][j]] = i;
            py[g[i][j]] = j;
        }
    }

    const int K = 2 * n - 2;
    const int W = K + 1;
    auto at = [W](int l, int k) { return l * W + k; };

    vector<int> best((K + 1) * (K + 1), 0);
    auto segmentBound = [K, &best](int s, int k) -> int& {
        return best[s * (K + 1) + k];
    };
    vector<int> dp((n + 1) * (n + 1), INT_MAX);
    auto cell = [n, &dp](int i, int j) -> int& {
        return dp[i * (n + 1) + j];
    };
    for (int s = 1; s <= K; ++s) {
        for (int k = s; k <= K; ++k) {
            int b = INT_MAX;
            for (int i = max(2, k + 2 - n); i <= min(n, k + 1); ++i) {
                int j = k + 2 - i;
                int value;
                if (k == s) {
                    value = g[i][j];
                } else {
                    int previous = min(cell(i - 1, j), cell(i, j - 1));
                    value = max(g[i][j], previous);
                }
                cell(i, j) = value;
                b = min(b, value);
            }
            segmentBound(s, k) = b;
        }
    }

    vector<int> initialMax(n + 1, 0);
    for (int l = 1; l <= n; ++l) initialMax[l] = max(initialMax[l - 1], g[1][l]);

    vector<int> lowerBound((n + 1) * W, 0), exact((n + 1) * W, 0);
    priority_queue<Entry, vector<Entry>, greater<Entry>> heap;
    for (int l = 1; l <= n; ++l) {
        exact[at(l, 0)] = initialMax[l];
        heap.push({initialMax[l], l, 0});
        for (int k = 1; k <= K; ++k) {
            int b = segmentBound(max(1, k - l + 1), k);
            if (k < l) b = max(b, initialMax[l - k]);
            lowerBound[at(l, k)] = b;
            heap.push({b, l, k});
        }
    }

    vector<int> result;
    result.reserve(m);
    while ((int)result.size() < m) {
        Entry e = heap.top();
        heap.pop();
        int id = at(e.l, e.k);
        if (exact[id] != 0) {
            if (e.key == exact[id]) result.push_back(exact[id]);
            continue;
        }
        if (e.key != lowerBound[id]) {
            heap.push({lowerBound[id], e.l, e.k});
            continue;
        }

        cout << "? " << e.l << ' ' << e.k + 1 << '\n' << flush;
        int value;
        if (!(cin >> value) || value < 0) return 0;
        exact[id] = value;
        heap.push({value, e.l, e.k});

        int x = px[value], y = py[value];
        int first, last;
        if (x == 1) {
            first = 1;
            last = e.l - y + 1;
        } else {
            first = x + y - 1;
            last = first + e.l - 1;
        }
        first = max(first, 1);
        last = min(last, K + 1);
        for (int t = first; t <= last; ++t) {
            int other = at(e.l, t - 1);
            lowerBound[other] = max(lowerBound[other], value);
        }
    }

    cout << '!';
    for (int x : result) cout << ' ' << x;
    cout << '\n' << flush;
    return 0;
}
