#include <bits/stdc++.h>
using namespace std;

int n, k;

void reset_memory() {
    cout << "R\n" << flush;
}

bool ask(int c) {
    cout << "? " << c << '\n' << flush;
    string answer;
    if (!(cin >> answer) || answer == "E") exit(0);
    return answer == "Y";
}

vector<int> direct_compare(const vector<int>& a, const vector<int>& b) {
    int best_p = 1, best_q = 1;
    long long best_cost = LLONG_MAX;
    for (int p = 1; p <= (int)a.size(); ++p) {
        int a_part = ((int)a.size() + p - 1) / p;
        for (int q = 1; q <= (int)b.size(); ++q) {
            int b_part = ((int)b.size() + q - 1) / q;
            if (a_part + b_part > k) continue;
            long long cost = 1LL * q * a.size() + 1LL * p * b.size();
            if (cost < best_cost) {
                best_cost = cost;
                best_p = p;
                best_q = q;
            }
        }
    }

    vector<char> matched(b.size(), false);
    for (int p = 0; p < best_p; ++p) {
        int al = p * a.size() / best_p;
        int ar = (p + 1) * a.size() / best_p;
        for (int q = 0; q < best_q; ++q) {
            int bl = q * b.size() / best_q;
            int br = (q + 1) * b.size() / best_q;
            reset_memory();
            for (int i = al; i < ar; ++i) ask(a[i]);
            for (int j = bl; j < br; ++j) {
                if (ask(b[j])) matched[j] = true;
            }
        }
    }

    vector<int> remaining;
    for (int j = 0; j < (int)b.size(); ++j) {
        if (!matched[j]) remaining.push_back(b[j]);
    }
    return remaining;
}

vector<int> compare_sets(const vector<int>& a, const vector<int>& b) {
    if (b.empty()) return {};
    if (a.empty()) return b;

    if ((int)a.size() + (int)b.size() > k) {
        long long aba_cost = 3LL * a.size() + b.size();
        long long grouped_cost = LLONG_MAX;
        for (int p = 1; p <= (int)a.size(); ++p) {
            int a_part = ((int)a.size() + p - 1) / p;
            for (int q = 1; q <= (int)b.size(); ++q) {
                int b_part = ((int)b.size() + q - 1) / q;
                if (a_part + b_part <= k) {
                    long long cost = 1LL * q * (long long)a.size() +
                                     1LL * p * (long long)b.size();
                    grouped_cost = min(grouped_cost, cost);
                }
            }
        }
        if (grouped_cost < aba_cost) return direct_compare(a, b);
    }

    reset_memory();
    for (int x : a) ask(x);

    vector<char> matched(b.size(), false);
    for (int j = 0; j < (int)b.size(); ++j) {
        matched[j] = ask(b[j]);
    }

    if ((int)a.size() + (int)b.size() <= k) {
        vector<int> remaining;
        for (int j = 0; j < (int)b.size(); ++j) {
            if (!matched[j]) remaining.push_back(b[j]);
        }
        return remaining;
    }

    vector<char> reverse_match(a.size(), false);
    for (int i = 0; i < (int)a.size(); ++i) {
        reverse_match[i] = ask(a[i]);
    }

    vector<int> unmatched;
    for (int j = 0; j < (int)b.size(); ++j) {
        if (!matched[j]) unmatched.push_back(b[j]);
    }

    int reverse_count = count(reverse_match.begin(), reverse_match.end(), true);
    long long scan_cost = 1LL * reverse_count *
                          (unmatched.size() + (unmatched.size() + k - 2) / (k - 1));
    long long grouped_cost = LLONG_MAX;
    for (int p = 1; p <= (int)a.size(); ++p) {
        int a_part = ((int)a.size() + p - 1) / p;
        for (int q = 1; q <= (int)unmatched.size(); ++q) {
            int b_part = ((int)unmatched.size() + q - 1) / q;
            if (a_part + b_part <= k) {
                long long cost = 1LL * q * (long long)a.size() +
                                 1LL * p * (long long)unmatched.size();
                grouped_cost = min(grouped_cost, cost);
            }
        }
    }
    if (grouped_cost < scan_cost) return direct_compare(a, unmatched);

    for (int i = 0; i < (int)a.size() && !unmatched.empty(); ++i) {
        if (!reverse_match[i]) continue;

        vector<int> still_unmatched;
        bool found = false;
        for (int start = 0; start < (int)unmatched.size(); start += k - 1) {
            int finish = min((int)unmatched.size(), start + k - 1);
            reset_memory();
            ask(a[i]);
            int j = start;
            for (; j < finish; ++j) {
                if (ask(unmatched[j])) {
                    found = true;
                    ++j;
                    break;
                }
                still_unmatched.push_back(unmatched[j]);
            }
            if (found) {
                still_unmatched.insert(still_unmatched.end(),
                                       unmatched.begin() + j, unmatched.end());
                break;
            }
        }
        unmatched.swap(still_unmatched);
    }

    return unmatched;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> k)) return 0;

    if (k == 1) {
        vector<int> representatives;
        for (int i = 1; i <= n; ++i) {
            bool is_new = true;
            for (int r : representatives) {
                reset_memory();
                ask(r);
                if (ask(i)) {
                    is_new = false;
                    break;
                }
            }
            if (is_new) representatives.push_back(i);
        }
        cout << "! " << representatives.size() << '\n' << flush;
        return 0;
    }

    vector<vector<int>> blocks;
    for (int first = 1; first <= n; first += k) {
        reset_memory();
        vector<int> unique;
        for (int i = first; i < first + k; ++i) {
            if (!ask(i)) unique.push_back(i);
        }
        blocks.push_back(move(unique));
    }

    for (int current = 1; current < (int)blocks.size(); ++current) {
        vector<int> candidates = move(blocks[current]);
        vector<int> order(current);
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int x, int y) {
            return blocks[x].size() < blocks[y].size();
        });

        int p = 0;
        while (p < current && !candidates.empty()) {
            vector<int> loaded;
            while (p < current &&
                   loaded.size() + blocks[order[p]].size() + candidates.size() <= k) {
                const vector<int>& add = blocks[order[p]];
                loaded.insert(loaded.end(), add.begin(), add.end());
                ++p;
            }
            if (loaded.empty()) {
                loaded = blocks[order[p]];
                ++p;
            }
            candidates = compare_sets(loaded, candidates);
        }
        blocks[current] = move(candidates);
    }

    int diversity = 0;
    for (const vector<int>& block : blocks) diversity += block.size();
    cout << "! " << diversity << '\n' << flush;
    return 0;
}
