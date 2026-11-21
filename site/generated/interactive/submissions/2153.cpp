#include <bits/stdc++.h>
using namespace std;

int n;
vector<vector<int>> B;

int ask(int p) {
    cout << "? " << p << '\n';
    cout.flush();
    int r;
    if (!(cin >> r)) exit(0);
    if (r == -1) exit(0);
    return r;
}

void probe_dial(int i) {
    for (int t = 1; t <= n; ++t) {
        B[i][t] = ask(i);
    }
}

void probe_all() {
    for (int i = 1; i <= n; ++i) probe_dial(i);
}

struct Result {
    bool ok = false;
    bool all_distinct = false;
    vector<int> val;
};

Result analyze() {
    bool all_distinct = true;
    for (int i = 1; i <= n; ++i) {
        if (B[i][n] != 0) all_distinct = false;
        for (int t = 1; t < n; ++t) {
            if (B[i][t] != 1) all_distinct = false;
        }
    }
    if (all_distinct) return {false, true, {}};

    vector<char> empty(n, 0);
    int empty_cnt = 0;
    for (int t = 1; t < n; ++t) {
        if (B[1][t] == 0) {
            empty[t] = 1;
            ++empty_cnt;
        }
    }

    if (empty_cnt == 0) return {false, true, {}};

    vector<vector<int>> dom(n + 1);
    for (int i = 1; i <= n; ++i) {
        for (int s = 0; s < n; ++s) {
            if (empty[s]) continue;
            bool ok = true;
            for (int t = 1; t < n; ++t) {
                bool z = (B[i][t] == 0);
                bool e = empty[(t + s) % n];
                if (z != e) {
                    ok = false;
                    break;
                }
            }
            if (ok) dom[i].push_back(s);
        }
        if (dom[i].empty()) return {false, false, {}};
    }

    for (int i = 1; i <= n; ++i) {
        if (dom[i].size() != 1) return {false, false, {}};
    }

    vector<int> val(n + 1), cnt(n, 0);
    for (int i = 1; i <= n; ++i) {
        val[i] = dom[i][0];
        ++cnt[val[i]];
    }

    for (int i = 1; i <= n; ++i) {
        if (B[i][n] != (cnt[val[i]] >= 2)) return {false, false, {}};
        for (int t = 1; t < n; ++t) {
            if (B[i][t] != (cnt[(val[i] + t) % n] > 0)) {
                return {false, false, {}};
            }
        }
    }

    return {true, false, val};
}

void execute(const vector<int>& val) {
    vector<int> cnt(n, 0);
    for (int i = 1; i <= n; ++i) ++cnt[val[i]];

    vector<long long> f(n, 0);
    for (int i = 1; i <= n; ++i) {
        f[0] += (n - val[i]) % n;
    }

    for (int x = 1; x < n; ++x) {
        f[x] = f[x - 1] + 1LL * n * (1 - cnt[x]);
    }

    int target = 0;
    for (int x = 1; x < n; ++x) {
        if (f[x] < f[target]) target = x;
    }

    for (int i = 1; i <= n; ++i) {
        int d = (target - val[i] + n) % n;
        for (int k = 0; k < d; ++k) {
            ask(i);
        }
    }

    cout << "!\n";
    cout.flush();
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    B.assign(n + 1, vector<int>(n + 1, -1));

    // Probe dial 1 first. If it sees every other value occupied, the state is all distinct.
    probe_dial(1);

    bool b1_all_distinct = (B[1][n] == 0);
    for (int t = 1; t < n; ++t) {
        if (B[1][t] != 1) b1_all_distinct = false;
    }

    if (b1_all_distinct) {
        // Break the all-distinct symmetry with one move, then probe everything.
        ask(1);
        probe_all();
        Result res = analyze();
        if (res.ok) execute(res.val);
    } else {
        // Probe the remaining dials.
        for (int i = 2; i <= n; ++i) probe_dial(i);
        Result res = analyze();
        if (res.ok) execute(res.val);
    }

    // Fallback: break any remaining periodic symmetry by moving dial 1 to the first empty cell.
    while (true) {
        bool got_zero = false;
        for (int step = 1; step <= n; ++step) {
            int r = ask(1);
            if (r == 0) {
                got_zero = true;
                break;
            }
        }
        (void)got_zero;

        probe_all();
        Result res = analyze();

        if (res.all_distinct) {
            ask(1);
            probe_all();
            res = analyze();
        }

        if (res.ok) execute(res.val);
    }
}
