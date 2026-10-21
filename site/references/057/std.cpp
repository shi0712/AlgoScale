#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

using DP = vector<uint16_t>;
int N, B;
vector<array<int, 4>> dista;

// f[t]: cost at the next hospital, with cost at this hospital <= t.
void add_item(DP& f, int a, int b) {
    const int INF = B + 1;
    for (int t = B; t >= a; --t)
        f[t] = uint16_t(min({INF, int(f[t]) + b, int(f[t - a])}));
    for (int t = min(B, a - 1); t >= 0; --t)
        f[t] = uint16_t(min(INF, int(f[t]) + b));
}

bool check_cycle(const array<DP, 4>& f, const array<int, 4>& cap) {
    // The composition of the four budget maps is nondecreasing.
    int t = cap[0];
    while (t >= 0) {
        int x = t;
        for (int e = 0; e < 4 && x >= 0; ++e)
            x = cap[(e + 1) % 4] - int(f[e][x]);
        if (x < 0) return false;
        if (x >= t) return true;
        t = x;
    }
    return false;
}

bool solve() {
    if (B == 0) return false;
    long long lower_bound = 0;
    for (const auto& d : dista) {
        int z = *min_element(d.begin(), d.end());
        if (z > B) return false;
        lower_bound += z;
    }
    if (lower_bound > 4LL * B) return false;

    vector<int> order_u(N), order_v(N), rank_u(N), rank_v(N);
    iota(order_u.begin(), order_u.end(), 0);
    iota(order_v.begin(), order_v.end(), 0);
    auto cmp = [&](int h, int i, int j) {
        if (dista[i][h] != dista[j][h])
            return dista[i][h] < dista[j][h];
        return i < j;
    };
    sort(order_u.begin(), order_u.end(),
         [&](int i, int j) { return cmp(0, i, j); });
    sort(order_v.begin(), order_v.end(),
         [&](int i, int j) { return cmp(1, i, j); });
    for (int i = 0; i < N; ++i) {
        rank_u[order_u[i]] = i;
        rank_v[order_v[i]] = i;
    }

    const int K = max(1, int(sqrt(N)));
    for (int left = 0; left < N; left += K) {
        int right = min(N, left + K), len = right - left;
        vector<DP> suffix_cd(len), suffix_da(len);
        DP cd(B + 1, 0), da(B + 1, 0);

        // Only save suffix states at pivots belonging to this block.
        for (int j = N - 1; j >= 0; --j) {
            int p = order_v[j], r = rank_u[p];
            if (left <= r && r < right) {
                suffix_cd[r - left] = cd;
                suffix_da[r - left] = da;
            } else if (r < left) {
                add_item(da, dista[p][3], dista[p][0]);
            } else {
                add_item(cd, dista[p][2], dista[p][3]);
            }
        }

        DP ab(B + 1, 0), bc(B + 1, 0);
        for (int j = 0; j < N; ++j) {
            int p = order_v[j], r = rank_u[p];
            if (r < left) {
                add_item(ab, dista[p][0], dista[p][1]);
            } else if (r >= right) {
                add_item(bc, dista[p][1], dista[p][2]);
            } else {
                array<DP, 4> f = {
                    ab, bc, suffix_cd[r - left], suffix_da[r - left]
                };

                // Outside-block points are already included. Add the rest.
                for (int k = left; k < right; ++k) {
                    int q = order_u[k];
                    if (q == p) continue;
                    int e;
                    if (rank_u[q] < r)
                        e = rank_v[q] < j ? 0 : 3;
                    else
                        e = rank_v[q] < j ? 1 : 2;
                    add_item(f[e], dista[q][e], dista[q][(e + 1) % 4]);
                }

                // The pivot itself can be assigned to any of the hospitals.
                for (int h = 0; h < 4; ++h) {
                    if (dista[p][h] > B) continue;
                    array<int, 4> cap = {B, B, B, B};
                    cap[h] -= dista[p][h];
                    if (check_cycle(f, cap)) return true;
                }
            }
        }
    }
    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int L, T;
    cin >> L >> N >> T;
    B = T / 2;
    const int D = L - 1;
    dista.resize(N);
    for (auto& d : dista) {
        int x, y;
        cin >> x >> y;
        --x; --y;
        d = {x + y, x + D - y, 2 * D - x - y, D - x + y};
    }
    cout << (solve() ? "Yes\n" : "No\n");
}
