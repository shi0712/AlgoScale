#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
static constexpr int64 NEG = -(1LL << 60);

struct Group {
    int t, count;
    vector<int64> reward;
};

/*
Let W bound the final point numbers. If more than W cards have the
same final point number a, a balanced selection can lose two of them:
if they are all on one side, the other side has at least a cards.
Among prefix sums of any a such cards, two agree modulo a. Their
intervening sum is q*a with 1 <= q <= W. Swap those cards with q
of the equal-a cards. There are now equal-a cards on both sides,
so remove one from each side.

For one original type, distinguish normal and doubled cards. Thus
counts above 2*W can always decrease by two without increasing the
doubling budget. With budget k < W, counts above W+k have more than
W normal cards and can also decrease by two. Conversely, two normal
cards can always be added on opposite sides. Consequently a small
residual count j may receive the best prefix value of any count
at least j with the same parity; all extra cards form neutral pairs.
*/
vector<Group> make_groups(const vector<vector<int64>>& suffix, int cap) {
    vector<Group> groups;
    for (int t = 1; t < (int)suffix.size(); ++t) {
        int count = (int)suffix[t].size() - 1;
        if (!count) continue;
        int c = min(count, cap);
        groups.push_back({t, c, vector<int64>(suffix[t].begin(), suffix[t].begin() + c + 1)});
    }
    return groups;
}

// Maximize value, breaking ties by using fewer doublings.
pair<int64, int> unrestricted(const vector<Group>& groups) {
    int total = 0;
    for (const auto& g : groups) total += g.t * g.count;
    const int size = 2 * total + 1;
    vector<int64> dp(size, NEG), active(size), next(size), result(size);
    vector<int> cost(size), ac(size), nc(size), rc(size);
    dp[0] = 0;
    int processed = 0, dp_limit = 0;
    for (const auto& g : groups) {
        int future = total - processed - g.t * g.count;
        int out_limit = 2 * future;
        fill(result.begin(), result.begin() + out_limit + 1, NEG);
        copy(dp.begin(), dp.begin() + dp_limit + 1, active.begin());
        copy(cost.begin(), cost.begin() + dp_limit + 1, ac.begin());
        int limit = dp_limit;
        for (int j = 0; j <= g.count; ++j) {
            int merge_limit = min(limit, out_limit);
            for (int d = 0; d <= merge_limit; ++d) {
                if (active[d] == NEG) continue;
                int64 value = active[d] + g.reward[j];
                if (value > result[d] || (value == result[d] && ac[d] < rc[d])) {
                    result[d] = value;
                    rc[d] = ac[d];
                }
            }
            if (j == g.count) break;
            int next_limit = min(2 * (processed + (j + 1) * g.t),
                                 2 * (future + (g.count - j - 1) * g.t));
            fill(next.begin(), next.begin() + next_limit + 1, NEG);
            // Reverse transitions keep every destination independent.
            for (int d = 0; d <= next_limit; ++d) {
                int64 best = NEG;
                int b = 0;
                auto relax = [&](int source, int extra) {
                    if (source > limit || active[source] == NEG) return;
                    int64 value = active[source];
                    int used = ac[source] + extra;
                    if (value > best || (value == best && used < b)) {
                        best = value;
                        b = used;
                    }
                };
                relax(abs(d - g.t), 0);
                relax(d + g.t, 0);
                relax(abs(d - 2 * g.t), 1);
                relax(d + 2 * g.t, 1);
                next[d] = best;
                nc[d] = b;
            }
            active.swap(next);
            ac.swap(nc);
            limit = next_limit;
        }
        dp.swap(result);
        cost.swap(rc);
        dp_limit = min(2 * (processed + g.t * g.count), out_limit);
        processed += g.t * g.count;
    }
    return {dp[0], cost[0]};
}

int64 restricted(const vector<Group>& groups, int budget) {
    int total = 0;
    for (const auto& g : groups) total += g.t * g.count;
    // With b doublings, at most b * max(t) additional points are available.
    int max_t = groups.empty() ? 0 : groups.back().t;
    int stride = budget + 1;
    int max_difference = total + min(total, budget * max_t);
    size_t size = (size_t)(max_difference + 1) * stride;
    vector<int64> dp(size, NEG), active(size), next(size), result(size);
    dp[0] = 0;
    int processed = 0, dp_limit = 0;
    for (const auto& g : groups) {
        int future = total - processed - g.t * g.count;
        int out_limit = min(2 * future, future + budget * max_t);
        fill(result.begin(), result.begin() + (size_t)(out_limit + 1) * stride, NEG);
        copy(dp.begin(), dp.begin() + (size_t)(dp_limit + 1) * stride, active.begin());
        int limit = dp_limit;
        for (int j = 0; j <= g.count; ++j) {
            int merge_limit = min(limit, out_limit);
            for (int d = 0; d <= merge_limit; ++d) {
                size_t base = (size_t)d * stride;
                for (int b = 0; b <= budget; ++b) {
                    int64 value = active[base + b];
                    if (value != NEG) result[base + b] = max(result[base + b], value + g.reward[j]);
                }
            }
            if (j == g.count) break;
            int past_points = processed + (j + 1) * g.t;
            int future_points = future + (g.count - j - 1) * g.t;
            int next_limit = min({2 * past_points, past_points + budget * max_t,
                                  2 * future_points, future_points + budget * max_t});
            for (int d = 0; d <= next_limit; ++d) {
                int s1 = abs(d - g.t), s2 = d + g.t;
                int s3 = abs(d - 2 * g.t), s4 = d + 2 * g.t;
                int64* dest = next.data() + (size_t)d * stride;
                const int64* p1 = s1 <= limit ? active.data() + (size_t)s1 * stride : nullptr;
                const int64* p2 = s2 <= limit ? active.data() + (size_t)s2 * stride : nullptr;
                const int64* p3 = s3 <= limit ? active.data() + (size_t)s3 * stride : nullptr;
                const int64* p4 = s4 <= limit ? active.data() + (size_t)s4 * stride : nullptr;
                for (int b = 0; b <= budget; ++b) {
                    int64 best = NEG;
                    if (p1) best = max(best, p1[b]);
                    if (p2) best = max(best, p2[b]);
                    if (b) {
                        if (p3) best = max(best, p3[b - 1]);
                        if (p4) best = max(best, p4[b - 1]);
                    }
                    dest[b] = best;
                }
            }
            active.swap(next);
            limit = next_limit;
        }
        dp.swap(result);
        dp_limit = min({2 * (processed + g.t * g.count),
                        processed + g.t * g.count + budget * max_t, out_limit});
        processed += g.t * g.count;
    }
    return *max_element(dp.begin(), dp.begin() + stride);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, k;
    cin >> n >> k;
    vector<vector<int64>> values(14), suffix(14);
    int max_t = 0;
    for (int i = 0; i < n; ++i) {
        int64 v;
        int t;
        cin >> v >> t;
        values[t].push_back(v);
        max_t = max(max_t, t);
    }
    for (int t = 1; t <= 13; ++t) {
        auto& a = values[t];
        sort(a.rbegin(), a.rend());
        int c = (int)a.size();
        suffix[t].resize(c + 1);
        for (int j = 0; j < c; ++j) suffix[t][j + 1] = suffix[t][j] + a[j];
        // Best prefix with at least j cards and the same parity as j.
        for (int j = c - 2; j >= 0; --j) suffix[t][j] = max(suffix[t][j], suffix[t][j + 2]);
    }
    if (k == 0) {
        // Only one final point number per original type: cap max_t suffices.
        cout << restricted(make_groups(suffix, max_t), 0) << '\n';
        return 0;
    }
    auto [answer, needed] = unrestricted(make_groups(suffix, 4 * max_t));
    if (needed <= k) {
        cout << answer << '\n';
    } else {
        int cap = 2 * max_t + min(k, 2 * max_t);
        cout << restricted(make_groups(suffix, cap), k) << '\n';
    }
    return 0;
}
