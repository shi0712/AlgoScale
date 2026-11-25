#include <bits/stdc++.h>
using namespace std;

bool query(int idx) {
    cout << "? " << idx << "\n" << flush;
    string s;
    if (!(cin >> s)) exit(0);
    if (s == "E" || s == "-1") exit(0);
    return s[0] == 'Y';
}

void reset_memory() {
    cout << "R\n" << flush;
}

void answer(int d) {
    cout << "! " << d << "\n" << flush;
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;

    if (n == 1) {
        answer(1);
    }

    if (k >= n) {
        reset_memory();
        int d = 0;
        for (int i = 1; i <= n; ++i) {
            if (!query(i)) ++d;
        }
        answer(d);
    }

    if (k == 1) {
        vector<int> reps;
        for (int i = 1; i <= n; ++i) {
            bool found = false;
            for (int r : reps) {
                reset_memory();
                query(r);
                if (query(i)) {
                    found = true;
                    break;
                }
            }
            if (!found) reps.push_back(i);
        }
        answer((int)reps.size());
    }

    int c = k / 2;
    int m = (n + c - 1) / c;
    vector<vector<int>> reps(m);

    for (int i = 0; i < m; ++i) {
        int start = i * c;
        int end = min(n, start + c);
        reset_memory();
        for (int idx = start; idx < end; ++idx) {
            if (!query(idx + 1)) {
                reps[i].push_back(idx + 1);
            }
        }
    }

    int d = 0;
    for (int i = 0; i < m; ++i) {
        vector<bool> seen(reps[i].size(), false);
        for (int j = 0; j < i; ++j) {
            if (reps[j].empty() || reps[i].empty()) continue;

            bool all_seen = true;
            for (bool b : seen) {
                if (!b) {
                    all_seen = false;
                    break;
                }
            }
            if (all_seen) break;

            reset_memory();
            for (int x : reps[j]) query(x);
            for (int t = 0; t < (int)reps[i].size(); ++t) {
                if (query(reps[i][t])) seen[t] = true;
            }
        }
        for (bool b : seen) {
            if (!b) ++d;
        }
    }

    answer(d);
    return 0;
}
