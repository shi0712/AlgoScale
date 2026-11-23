#include <bits/stdc++.h>
using namespace std;

struct Entry {
    int lower, upper, l, t;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;

    vector<vector<int>> a(n, vector<int>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            cin >> a[i][j];

    const int S = 2 * n;
    const int INF = 1e9;

    vector<int> diagonalMin(S + 1, INF), diagonalMax(S + 1, 0);
    for (int s = 2; s <= 2 * n - 1; ++s) {
        int rlo = max(2, s + 1 - n);
        int rhi = min(n, s);
        for (int r = rlo; r <= rhi; ++r) {
            int c = s + 1 - r;
            int value = a[r - 1][c - 1];
            diagonalMin[s] = min(diagonalMin[s], value);
            diagonalMax[s] = max(diagonalMax[s], value);
        }
    }

    vector<int> prefixMax(n + 1, 0);
    for (int k = 1; k <= n; ++k)
        prefixMax[k] = max(prefixMax[k - 1], a[0][k - 1]);

    int levels = 1;
    while ((1 << levels) <= S) ++levels;

    vector<int> lg(S + 1, 0);
    for (int i = 2; i <= S; ++i) lg[i] = lg[i / 2] + 1;

    vector<vector<int>> sparseMin(levels, vector<int>(S + 1));
    vector<vector<int>> sparseMax(levels, vector<int>(S + 1));
    sparseMin[0] = diagonalMin;
    sparseMax[0] = diagonalMax;

    for (int k = 1; k < levels; ++k) {
        int len = 1 << k;
        int half = len >> 1;
        for (int i = 0; i + len - 1 <= S; ++i) {
            sparseMin[k][i] = max(sparseMin[k - 1][i],
                                  sparseMin[k - 1][i + half]);
            sparseMax[k][i] = max(sparseMax[k - 1][i],
                                  sparseMax[k - 1][i + half]);
        }
    }

    auto rangeMax = [&](const vector<vector<int>>& st, int l, int r) {
        int k = lg[r - l + 1];
        int len = 1 << k;
        return max(st[k][l], st[k][r - len + 1]);
    };

    priority_queue<int> candidates;
    auto addKnownValue = [&](int value) {
        if ((int)candidates.size() < m) {
            candidates.push(value);
        } else if (value < candidates.top()) {
            candidates.pop();
            candidates.push(value);
        }
    };

    vector<Entry> entries;
    entries.reserve(n * (2 * n - 1));

    for (int l = 1; l <= n; ++l) {
        for (int t = 1; t <= 2 * n - 1; ++t) {
            int lower, upper;

            if (t == 1) {
                lower = upper = prefixMax[l];
            } else {
                int remainingInitial = l - t + 1;
                int mandatory = remainingInitial > 0
                              ? prefixMax[remainingInitial] : 0;
                int lo = max(2, t - l + 1);

                lower = max({l, mandatory,
                             rangeMax(sparseMin, lo, t)});
                upper = max(mandatory, rangeMax(sparseMax, lo, t));
            }

            if (lower == upper) {
                addKnownValue(lower);
            } else {
                entries.push_back({lower, upper, l, t});
            }
        }
    }

    sort(entries.begin(), entries.end(), [](const Entry& x, const Entry& y) {
        if (x.lower != y.lower) return x.lower < y.lower;
        if (x.upper != y.upper) return x.upper < y.upper;
        if (x.l != y.l) return x.l < y.l;
        return x.t < y.t;
    });

    size_t next = 0;
    while (true) {
        if ((int)candidates.size() == m &&
            (next == entries.size() ||
             entries[next].lower >= candidates.top())) {
            break;
        }
        if (next == entries.size()) break;

        Entry e = entries[next++];
        cout << "? " << e.l << ' ' << e.t << endl;

        int value;
        if (!(cin >> value) || value < 0) return 0;
        addKnownValue(value);
    }

    vector<int> answer;
    answer.reserve(m);
    while (!candidates.empty()) {
        answer.push_back(candidates.top());
        candidates.pop();
    }
    sort(answer.begin(), answer.end());

    cout << '!';
    for (int value : answer) cout << ' ' << value;
    cout << endl;
    return 0;
}
