#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;

    int cur_k = k;

    auto do_move = [&](int idx0, long long delta) -> int {
        cout << idx0 + 1 << ' ' << delta << '\n';
        cout.flush();

        int nk;
        if (!(cin >> nk)) {
            exit(0);
        }
        cur_k = nk;
        if (cur_k == 1) {
            exit(0);
        }
        return nk;
    };

    auto process_rotor = [&](int idx, int target,
                             vector<char>& occupied,
                             vector<char>& empty,
                             int& current_k) {
        int k_before = current_k;

        vector<pair<int,int>> cand;
        for (int a = 0; a < n; ++a) {
            if (!occupied[a]) continue;

            if (a == target) {
                cand.push_back({a, 1}); // target always has another rotor
            } else {
                cand.push_back({a, 0});
                cand.push_back({a, 1});
            }
        }

        auto predict = [&](int a, int u, int d) -> int {
            int y = (a + d) % n;
            if (u == 1) {
                if (occupied[y]) return k_before;
                else return k_before + 1;
            } else {
                // d is never 0 when this is used for a query
                if (occupied[y]) return k_before - 1;
                else return k_before;
            }
        };

        int current_d = 0;
        int last_k = current_k;

        while (true) {
            bool all_same = true;
            for (size_t i = 1; i < cand.size(); ++i) {
                if (cand[i] != cand[0]) {
                    all_same = false;
                    break;
                }
            }
            if (all_same) break;

            int best_d = -1;
            int best_score = INT_MAX;
            int best_distinct = -1;

            for (int d = 1; d < n; ++d) {
                if (d == current_d) continue;

                int freq[3] = {0, 0, 0};
                for (auto [a, u] : cand) {
                    int p = predict(a, u, d);
                    int id = p - (k_before - 1);
                    if (id < 0) id = 0;
                    if (id > 2) id = 2;
                    ++freq[id];
                }

                int mx = max({freq[0], freq[1], freq[2]});
                int distinct = (freq[0] > 0) + (freq[1] > 0) + (freq[2] > 0);

                if (mx < best_score || (mx == best_score && distinct > best_distinct)) {
                    best_score = mx;
                    best_distinct = distinct;
                    best_d = d;
                }
            }

            if (best_d == -1) break;
            if (best_score == (int)cand.size()) break;

            int delta = best_d - current_d;
            int nk = do_move(idx, delta);
            last_k = nk;
            current_d = best_d;

            vector<pair<int,int>> nxt;
            for (auto [a, u] : cand) {
                if (predict(a, u, best_d) == nk) {
                    nxt.push_back({a, u});
                }
            }

            if (nxt.empty()) break;
            cand.swap(nxt);
        }

        if (cand.empty()) {
            cand.push_back({target, 1});
        }

        int a = cand[0].first;
        int u = cand[0].second;

        int cur_y = (a + current_d) % n;
        int delta_to_target = target - cur_y;
        delta_to_target %= n;
        if (delta_to_target < 0) delta_to_target += n;
        if (delta_to_target > n / 2) delta_to_target -= n;

        int nk;
        if (delta_to_target != 0) {
            nk = do_move(idx, delta_to_target);
            last_k = nk;
        } else {
            nk = last_k;
        }

        if (a != target && u == 0) {
            occupied[a] = false;
            empty[a] = true;
        }

        current_k = nk;
    };

    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    if (k == n) {
        // Make rotor 1 move one step. This creates an empty position at 0
        // and a target group at 1.
        do_move(0, 1);

        int target = 1;

        vector<char> occupied(n, true), empty(n, false);
        occupied[0] = false;
        empty[0] = true;

        vector<int> order;
        for (int i = 1; i < n; ++i) order.push_back(i);
        shuffle(order.begin(), order.end(), rng);

        for (int idx : order) {
            process_rotor(idx, target, occupied, empty, cur_k);
        }
    } else {
        int k0 = cur_k;

        vector<int> response(n + 1);

        for (int t = 1; t <= n; ++t) {
            response[t] = do_move(0, 1);
        }

        bool any_less = false;
        bool any_greater = false;

        for (int t = 1; t < n; ++t) {
            if (response[t] < k0) any_less = true;
            if (response[t] > k0) any_greater = true;
        }

        bool unique_rotor1;
        if (any_less) unique_rotor1 = true;
        else if (any_greater) unique_rotor1 = false;
        else unique_rotor1 = false;

        vector<char> occupied(n, false), empty(n, true);
        occupied[0] = true;
        empty[0] = false;

        for (int t = 1; t < n; ++t) {
            bool is_occupied;
            if (unique_rotor1) {
                is_occupied = (response[t] == k0 - 1);
            } else {
                is_occupied = (response[t] == k0);
            }

            if (is_occupied) {
                occupied[t] = true;
                empty[t] = false;
            }
        }

        cur_k = k0;
        int target = 0;

        vector<int> order;
        for (int i = 1; i < n; ++i) order.push_back(i);
        shuffle(order.begin(), order.end(), rng);

        for (int idx : order) {
            process_rotor(idx, target, occupied, empty, cur_k);
        }
    }

    return 0;
}
