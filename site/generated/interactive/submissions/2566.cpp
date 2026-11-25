#include <bits/stdc++.h>
using namespace std;

int n;

int compute_vis(int A, int C, const vector<int>& U_masks, const vector<int>& C_masks) {
    int mask = U_masks[A] | C_masks[C];
    return n - __builtin_popcount(mask);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    cin >> n;
    int init_vis;
    cin >> init_vis;

    for (int i = 1; i <= n; ++i) {
        int k = n - i + 1;
        int C_len = n - i;
        int A_size = 1 << k;
        int C_size = 1 << C_len;

        vector<int> U_masks(A_size, 0);
        for (int A = 0; A < A_size; ++A) {
            if (A == 0) {
                U_masks[A] = 0;
            } else {
                int max_j = 31 - __builtin_clz(A);
                int mask = 0;
                for (int c = i; c <= i + max_j; ++c) {
                    mask |= (1 << c);
                }
                U_masks[A] = mask;
            }
        }

        vector<int> C_masks(C_size, 0);
        for (int C = 0; C < C_size; ++C) {
            int mask = 0;
            for (int j = 0; j < C_len; ++j) {
                if (C & (1 << j)) {
                    int cell = i + 1 + j;
                    mask |= (1 << cell);
                }
            }
            C_masks[C] = mask;
        }

        vector<pair<int,int>> states;
        states.reserve(A_size * C_size);
        for (int A = 0; A < A_size; ++A) {
            for (int C = 0; C < C_size; ++C) {
                states.push_back({A, C});
            }
        }

        while (true) {
            bool all_zero = true;
            for (auto& st : states) {
                if (st.first != 0) {
                    all_zero = false;
                    break;
                }
            }
            if (all_zero) break;

            int best_r = -1;
            int best_max_group = INT_MAX;
            vector<int> counts(11);
            for (int r = 0; r < k; ++r) {
                fill(counts.begin(), counts.end(), 0);
                for (auto& st : states) {
                    int A = st.first;
                    int C = st.second;
                    int A2 = A ^ (1 << r);
                    int vis = compute_vis(A2, C, U_masks, C_masks);
                    counts[vis]++;
                }
                int max_group = 0;
                for (int v = 0; v <= n; ++v) {
                    if (counts[v] > max_group) max_group = counts[v];
                }
                if (max_group < best_max_group) {
                    best_max_group = max_group;
                    best_r = r;
                }
            }

            int l = i;
            int r_end = i + best_r;
            cout << l << " " << r_end << "\n";
            cout.flush();

            int obs;
            if (!(cin >> obs)) return 0;
            if (obs == -1) return 0;
            if (obs == n) return 0;

            vector<pair<int,int>> new_states;
            new_states.reserve(states.size());
            for (auto& st : states) {
                int A = st.first;
                int C = st.second;
                int A2 = A ^ (1 << best_r);
                int vis = compute_vis(A2, C, U_masks, C_masks);
                if (vis == obs) {
                    new_states.push_back({A2, C});
                }
            }
            states = move(new_states);
        }
    }
    return 0;
}
