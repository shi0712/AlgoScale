#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;

    auto query = [&](int pos) -> char {
        cout << "? " << pos << endl;
        string s;
        if (!(cin >> s)) exit(0);
        if (s == "E") exit(0);
        return s[0];
    };

    auto reset = [&]() {
        cout << "R" << endl;
    };

    // If memory can hold the whole array, just query all positions once.
    if (k >= n) {
        reset();
        int distinct = 0;
        for (int i = 1; i <= n; ++i) {
            if (query(i) == 'N') ++distinct;
        }
        cout << "! " << distinct << endl;
        return 0;
    }

    int s = (k >= 2 ? k / 2 : 1);   // block size
    int m = n / s;                  // number of blocks

    vector<vector<int>> reps(m);
    vector<int> d(m, 0);

    // First pass: find representatives of distinct colors in each block.
    for (int i = 0; i < m; ++i) {
        reset();
        int start = i * s + 1;
        int end = (i + 1) * s;
        for (int pos = start; pos <= end; ++pos) {
            if (query(pos) == 'N') {
                ++d[i];
                reps[i].push_back(pos);
            }
        }
    }

    long long total = 0;

    // For each block, count colors not seen in any previous block.
    for (int i = 0; i < m; ++i) {
        vector<int> unresolved = reps[i];

        for (int j = 0; j < i && !unresolved.empty(); ++j) {
            reset();

            // Load block j into memory.
            int start = j * s + 1;
            int end = (j + 1) * s;
            for (int pos = start; pos <= end; ++pos) {
                query(pos);
            }

            // Check unresolved representatives against block j.
            vector<int> next;
            next.reserve(unresolved.size());
            for (int rep : unresolved) {
                if (query(rep) == 'N') {
                    next.push_back(rep);
                }
                // 'Y' means this color already appeared in a previous block.
            }
            unresolved.swap(next);
        }

        total += unresolved.size();
    }

    cout << "! " << total << endl;
    return 0;
}
