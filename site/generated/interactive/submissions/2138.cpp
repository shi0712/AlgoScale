#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;

    // state of each candidate
    vector<char> alive(n + 1, 1);
    vector<char> run_len(n + 1, 0);   // 0,1,2
    vector<char> last_d(n + 1, 0);    // meaningful only if run_len>0

    int total_alive = n;

    auto get_group = [&](int idx) -> int {
        if (!alive[idx]) return -1;
        if (run_len[idx] == 0) return 0;
        if (run_len[idx] == 1) {
            return (last_d[idx] == 0) ? 1 : 2;
        } else { // run_len == 2
            return (last_d[idx] == 0) ? 3 : 4;
        }
    };

    while (total_alive > 2) {
        // count candidates per group
        int sz[5] = {0,0,0,0,0};
        for (int i = 1; i <= n; ++i) if (alive[i]) {
            int g = get_group(i);
            ++sz[g];
        }

        // compute half = ceil(sz/2) and need = floor(sz/2)
        int half[5];
        int need[5];
        for (int g = 0; g < 5; ++g) {
            half[g] = (sz[g] + 1) / 2;
            need[g] = sz[g] - half[g]; // = floor(sz/2)
        }

        // try to find a prefix interval [1, p] that respects half
        int pref_cnt[5] = {0,0,0,0,0};
        int first_exceed = -1; // position where some group exceeds half
        int first_pos_need[5];
        for (int g = 0; g < 5; ++g) first_pos_need[g] = -1;

        for (int i = 1; i <= n; ++i) {
            if (alive[i]) {
                int g = get_group(i);
                ++pref_cnt[g];
                if (first_exceed == -1 && pref_cnt[g] > half[g]) {
                    first_exceed = i; // i is the first position that exceeds
                }
                if (need[g] > 0 && first_pos_need[g] == -1 && pref_cnt[g] >= need[g]) {
                    first_pos_need[g] = i;
                }
            }
        }

        int l, r;
        if (first_exceed != -1 && first_exceed > 1) {
            // use prefix [1, first_exceed-1]
            l = 1;
            r = first_exceed - 1;
        } else {
            // use suffix
            for (int g = 0; g < 5; ++g) {
                if (need[g] == 0) first_pos_need[g] = 0;
                else if (first_pos_need[g] == -1) first_pos_need[g] = n; // should not happen
            }
            int p = 0;
            for (int g = 0; g < 5; ++g) p = max(p, first_pos_need[g]);
            l = p + 1;
            r = n;
            if (l > r) { // fallback, should not happen
                l = 1; r = n;
            }
        }

        // make the query
        cout << "? " << l << " " << r << "\n" << flush;
        int y;
        if (!(cin >> y)) return 0;
        if (y == -1) return 0;
        int len = r - l + 1;
        int b = (y == len) ? 1 : 0; // observed bit

        // update all candidates
        for (int i = 1; i <= n; ++i) if (alive[i]) {
            bool inside = (l <= i && i <= r);
            int dnew = inside ? b : 1 - b;
            if (run_len[i] == 0) {
                last_d[i] = dnew;
                run_len[i] = 1;
            } else {
                if (dnew == last_d[i]) {
                    if (run_len[i] == 2) {
                        alive[i] = 0;
                        --total_alive;
                    } else {
                        run_len[i] = 2; // stay with same last_d
                    }
                } else {
                    last_d[i] = dnew;
                    run_len[i] = 1;
                }
            }
        }
    }

    // output remaining candidates (1 or 2)
    vector<int> ans;
    for (int i = 1; i <= n; ++i) if (alive[i]) ans.push_back(i);
    for (size_t i = 0; i < ans.size(); ++i) {
        cout << "! " << ans[i] << "\n" << flush;
        int resp;
        if (!(cin >> resp)) return 0;
        if (resp == -1) return 0;
        if (resp == 1) break; // correct answer, we may stop early
    }
    cout << "#\n" << flush;
    return 0;
}
