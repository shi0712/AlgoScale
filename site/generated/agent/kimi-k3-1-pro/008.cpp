#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

using Values = array<int32_t, 6>;

static inline int halving_cost(int x, int mu, int &q) {
    int twice_mu = 2 * mu;
    if (x <= twice_mu) {
        q = 0;
        return x;
    }
    q = int(bit_width(uint32_t(x)) - bit_width(uint32_t(twice_mu)));
    if ((x >> q) > twice_mu) ++q;
    if (q > 0 && (x >> (q - 1)) <= twice_mu) --q;
    return (x >> q) + mu * q;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, P, Q;
    cin >> n >> P >> Q;

    vector<Values> roots(n);
    int max_h = 0;
    int64_t initial_sum = 0;
    for (int i = 0; i < n; ++i) {
        int h;
        cin >> h;
        max_h = max(max_h, h);
        initial_sum += h;
        roots[i][0] = h;
        for (int p = 1; p < 6; ++p) {
            roots[i][p] = int(std::sqrt(double(roots[i][p - 1])));
            while ((roots[i][p] + 1LL) * (roots[i][p] + 1LL) <= roots[i][p - 1]) ++roots[i][p];
            while (roots[i][p] * 1LL * roots[i][p] > roots[i][p - 1]) --roots[i][p];
        }
    }

    P = min<int64_t>(P, 5LL * n);
    vector<Values> best(n);
    vector<uint8_t> zero_q(n);
    vector<int32_t> counts(max_h + 1);
    array<int32_t, 1415> small_cost{};
    array<uint8_t, 1415> small_q{};

    auto evaluate = [&](int mu) -> pair<int64_t, int64_t> {
        for (int x = 0; x <= 1414; ++x) {
            int q;
            small_cost[x] = halving_cost(x, mu, q);
            small_q[x] = uint8_t(q);
        }

        fill(counts.begin(), counts.end(), 0);
        for (int i = 0; i < n; ++i) {
            int h = roots[i][0];
            int q;
            best[i][0] = h - halving_cost(h, mu, q);
            zero_q[i] = uint8_t(q);
            for (int p = 1; p < 6; ++p) {
                best[i][p] = h - small_cost[roots[i][p]];
                ++counts[best[i][p] - best[i][p - 1]];
            }
        }

        int lambda = max_h + 1;
        int64_t ties_needed = 0;
        if (P > 0) {
            int64_t greater = 0;
            for (int gain = max_h; gain >= 0; --gain) {
                if (greater + counts[gain] >= P) {
                    lambda = gain;
                    ties_needed = P - greater;
                    break;
                }
                greater += counts[gain];
            }
        }

        int64_t dual_value = 0;
        int64_t used_q = 0;
        for (int i = 0; i < n; ++i) {
            int p = 0;
            while (p < 5 && best[i][p + 1] - best[i][p] > lambda) ++p;
            while (p < 5 && ties_needed > 0 && best[i][p + 1] - best[i][p] == lambda) {
                ++p;
                --ties_needed;
            }
            dual_value += best[i][p];
            used_q += p == 0 ? zero_q[i] : small_q[roots[i][p]];
        }
        return {dual_value, used_q};
    };

    int64_t best_saving = numeric_limits<int64_t>::max();
    auto consider = [&](int mu) {
        auto [value, used_q] = evaluate(mu);
        best_saving = min(best_saving, value + int64_t(mu) * Q);
        return used_q;
    };

    int low = 0;
    int high = (max_h + 1) / 2;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (consider(mid) <= Q) high = mid;
        else low = mid + 1;
    }
    consider(low);
    if (low > 0) consider(low - 1);

    cout << initial_sum - best_saving << '\n';
    return 0;
}
