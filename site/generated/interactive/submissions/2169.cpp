#include <bits/stdc++.h>
using namespace std;

struct Comp {
    vector<int> coins;
    vector<int> A, B;
    bool oriented = false;
    int bad_count = -1;
};

int N, M;
vector<Comp> comps;
vector<char> isBad;
int found = 0;
int known_good = -1;
int next_idx = 0;
int unprocessed = 1000;

int query(int x, int y) {
    cout << "? " << x << " " << y << "\n";
    cout.flush();
    int r;
    if (!(cin >> r)) exit(0);
    if (r == -1) exit(0);
    return r;
}

void add_bad(const vector<int>& v) {
    for (int x : v) {
        if (!isBad[x]) {
            isBad[x] = 1;
            ++found;
        }
    }
}

void process_component(int idx) {
    Comp& c = comps[idx];
    unprocessed -= (int)c.coins.size();

    c.A.clear();
    c.B.clear();

    int first = c.coins[0];
    c.A.push_back(first);

    for (int i = 1; i < (int)c.coins.size(); ++i) {
        int r = query(first, c.coins[i]);
        if (r == 0) c.A.push_back(c.coins[i]);
        else c.B.push_back(c.coins[i]);
    }

    if (c.B.empty()) {
        if (c.coins.size() > 10) {
            c.oriented = true;
            c.bad_count = 0;
            if (known_good == -1) known_good = first;
        } else {
            c.oriented = false;
        }
    } else {
        c.oriented = false;
    }
}

void get_known_good() {
    if (known_good != -1) return;

    for (int i = 0; i < next_idx; ++i) {
        if (comps[i].oriented && comps[i].bad_count == 0 && comps[i].coins.size() > 10) {
            known_good = comps[i].coins[0];
            return;
        }
    }

    while (known_good == -1 && next_idx < 91) {
        process_component(next_idx);
        ++next_idx;
    }

    // Should never be needed for valid instances.
    if (known_good == -1) known_good = 1;
}

void orient_component(int idx) {
    if (comps[idx].oriented) return;
    if (known_good == -1) get_known_good();

    Comp& c = comps[idx];

    if (c.B.empty()) {
        int r = query(known_good, c.coins[0]);
        if (r == 0) {
            c.oriented = true;
            c.bad_count = 0;
        } else {
            c.oriented = true;
            c.bad_count = (int)c.coins.size();
            add_bad(c.coins);
        }
    } else {
        int r = query(known_good, c.A[0]);
        if (r == 0) {
            c.oriented = true;
            c.bad_count = (int)c.B.size();
            add_bad(c.B);
        } else {
            c.oriented = true;
            c.bad_count = (int)c.A.size();
            add_bad(c.A);
        }
    }
}

void apply_forced(int idx, int cnt) {
    if (comps[idx].oriented) return;

    Comp& c = comps[idx];
    int a = (int)c.A.size();
    int b = (int)c.B.size();

    if (cnt == 0) {
        c.oriented = true;
        c.bad_count = 0;
        return;
    }

    if (a == b) {
        if (known_good == -1) get_known_good();
        orient_component(idx);
        return;
    }

    if (a == cnt) {
        c.oriented = true;
        c.bad_count = cnt;
        add_bad(c.A);
    } else if (b == cnt) {
        c.oriented = true;
        c.bad_count = cnt;
        add_bad(c.B);
    } else {
        if (known_good != -1) orient_component(idx);
        else {
            c.oriented = true;
            c.bad_count = cnt;
        }
    }
}

bool force_any() {
    vector<int> un;
    for (int i = 0; i < next_idx; ++i) {
        if (!comps[i].oriented) un.push_back(i);
    }
    if (un.empty()) return false;

    int m = (int)un.size();
    vector<vector<int>> vals(m);

    for (int k = 0; k < m; ++k) {
        int a = (int)comps[un[k]].A.size();
        int b = (int)comps[un[k]].B.size();
        vector<int> v;

        if (a == b) {
            if (a <= 10) v.push_back(a);
        } else {
            if (a <= 10) v.push_back(a);
            if (b <= 10) v.push_back(b);
        }

        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
        if (v.empty()) v.push_back(0);
        vals[k] = v;
    }

    int ymax = min(10, unprocessed);

    for (int k = 0; k < m; ++k) {
        int idx = un[k];

        bitset<11> dp;
        dp[0] = 1;

        for (int l = 0; l < m; ++l) {
            if (l == k) continue;
            bitset<11> ndp;
            for (int val : vals[l]) {
                for (int s = 0; s + val <= 10; ++s) {
                    if (dp[s]) ndp[s + val] = 1;
                }
            }
            dp = ndp;
        }

        int feasible = 0;
        int forced_val = -1;

        for (int val : vals[k]) {
            int low = max(0, 10 - found - val - ymax);
            int high = min(10, 10 - found - val);
            if (low > high) continue;

            bool ok = false;
            for (int s = low; s <= high; ++s) {
                if (dp[s]) {
                    ok = true;
                    break;
                }
            }

            if (ok) {
                ++feasible;
                forced_val = val;
            }
        }

        if (feasible == 1) {
            apply_forced(idx, forced_val);
            return true;
        }
    }

    return false;
}

int choose_heuristic() {
    vector<int> un;
    for (int i = 0; i < next_idx; ++i) {
        if (!comps[i].oriented) un.push_back(i);
    }
    if (un.empty()) return -1;

    int r = 10 - found;
    if (r <= 0) return -1;

    int best = -1;
    int best_score = -1;

    for (int idx : un) {
        int a = (int)comps[idx].A.size();
        int b = (int)comps[idx].B.size();
        int mx = max(a, b);

        if (mx * 2 >= r) {
            int score = mx * 100 + (mx - min(a, b));
            if (score > best_score) {
                best_score = score;
                best = idx;
            }
        }
    }

    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N >> M;

    isBad.assign(N + 1, 0);

    int coin = 1;
    for (int i = 0; i < 90; ++i) {
        Comp c;
        for (int j = 0; j < 11; ++j) c.coins.push_back(coin++);
        comps.push_back(c);
    }

    Comp last;
    for (int j = 0; j < 10; ++j) last.coins.push_back(coin++);
    comps.push_back(last);

    unprocessed = N;

    while (found < 10) {
        if (next_idx < 91) {
            process_component(next_idx);
            ++next_idx;
        }

        bool acted = true;
        while (found < 10 && acted) {
            acted = false;

            if (force_any()) {
                acted = true;
                continue;
            }

            if (known_good != -1) {
                int best = choose_heuristic();
                if (best != -1) {
                    orient_component(best);
                    acted = true;
                    continue;
                }
            }
        }

        if (found < 10 && next_idx == 91) {
            bool progress = true;
            while (found < 10 && progress) {
                progress = false;

                if (force_any()) {
                    progress = true;
                    continue;
                }

                if (known_good != -1) {
                    for (int i = 0; i < next_idx; ++i) {
                        if (!comps[i].oriented) {
                            orient_component(i);
                            progress = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    // Safety fallback; should not be needed for valid instances.
    if (found < 10) {
        if (known_good == -1) get_known_good();
        for (int i = 1; i <= N && found < 10; ++i) {
            if (!isBad[i]) {
                int r = query(known_good, i);
                if (r == 1) {
                    isBad[i] = 1;
                    ++found;
                }
            }
        }
    }

    cout << "!";
    for (int i = 1; i <= N; ++i) {
        if (isBad[i]) cout << " " << i;
    }
    cout << "\n";
    cout.flush();

    return 0;
}
