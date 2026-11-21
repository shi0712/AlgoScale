#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

static int64 mod(int64 x, int64 m) {
    x %= m;
    return x < 0 ? x + m : x;
}

static int64 floor_half(int64 x) {
    return x >= 0 ? x / 2 : (x - 1) / 2;
}

static int log2_floor(int64 x) {
    return 63 - __builtin_clzll(static_cast<unsigned long long>(x));
}

class Solver {
    int64 n, m;
    int64 k, flip, top, width, h, block;
    int64 first_s, choices_s;

    int64 ask(int64 x) {
        cout << "? " << mod(x, 2 * n) / 2 + 1 << '\n' << flush;
        int64 answer;
        if (!(cin >> answer) || answer < 0) exit(0);
        return answer;
    }

    int64 read_at(int64 x) {
        int64 z = ask(x);
        return flip ? n - z : z;
    }

    void report(int64 c, int64 s) {
        c += flip * n;
        cout << "! " << mod((c - s) / 2, n) + 1
             << ' ' << mod((c + s) / 2, n) + 1 << '\n' << flush;
    }

    // With doubled coordinates, the transformed answer is the distance
    // to the transformed midpoint, clipped to [s, n-s].
    // After the first query, either s=k or the midpoint is +k or -k.
    int64 flat_count(int64 t) const {
        if (t >= k) return 0;
        return clamp(floor_half(top - t) + 1, 0LL, top + 1);
    }

    int64 gap_count(int64 t) const {
        return clamp(floor_half(abs(t) - first_s) + 1, 0LL, choices_s);
    }

    int64 internal_nodes(int64 q) const {
        return q <= 1 ? 0 : (q - 2) / block + 1;
    }

    pair<int64, int64> flat_cost(int64 q) const {
        if (q <= 1) return {0, 0};
        int64 nodes = internal_nodes(q);
        int64 d = log2_floor(nodes);
        int64 p = 1LL << d;
        int64 depth_sum = d * (nodes + 1) - 2 * p + 2;
        int64 unused = 1 + block * nodes - q;
        int64 cost = (block + 1) * nodes + block * depth_sum
                     - unused * (d + 1);
        return {d + 1, cost};
    }

    int64 choose_second_query() const {
        vector<int64> cuts{1, m + 1};

        // Add the change point of a monotone predicate.
        auto cut_at = [&](auto pred) {
            bool initial = pred(1);
            if (pred(m) == initial) return;
            int64 l = 2, r = m;
            while (l < r) {
                int64 mid = (l + r) / 2;
                if (pred(mid) == initial) l = mid + 1;
                else r = mid;
            }
            cuts.push_back(l);
        };

        for (bool other : {false, true}) {
            auto offset = [&](int64 y) {
                int64 t = 2 * y - k;
                return other ? width - t : t;
            };

            cut_at([&](int64 y) { return offset(y) >= k; });
            cut_at([&](int64 y) { return flat_count(offset(y)) >= 1; });
            cut_at([&](int64 y) { return flat_count(offset(y)) >= top + 1; });

            for (int64 p = 1; block * (p - 1) + 2 <= top + 1; p *= 2) {
                int64 q = block * (p - 1) + 2;
                cut_at([&](int64 y) { return flat_count(offset(y)) >= q; });
            }

            if (choices_s >= 2) {
                for (int64 v : {first_s + 2, top}) {
                    cut_at([&](int64 y) { return offset(y) >= v; });
                    cut_at([&](int64 y) { return offset(y) > -v; });
                }
            }
        }

        sort(cuts.begin(), cuts.end());
        cuts.erase(unique(cuts.begin(), cuts.end()), cuts.end());

        array<int64, 3> best{LLONG_MAX, LLONG_MAX, LLONG_MAX};
        int64 best_y = 1;
        auto evaluate = [&](int64 y) {
            int64 t = 2 * y - k;
            int64 worst = 0, sum = 0, unresolved = 0;
            for (int64 u : {t, width - t}) {
                int64 q = flat_count(u), g = gap_count(u);
                auto [depth, cost] = flat_cost(q);
                worst = max(worst, depth);
                sum += cost;
                if (q > 1) unresolved += q;
                if (g > 1) {
                    worst = max(worst, 1LL);
                    sum += g;
                    unresolved += g;
                }
            }
            array<int64, 3> score{worst, sum, unresolved};
            if (score < best) {
                best = score;
                best_y = y;
            }
        };

        // Between cuts the costs are affine except for the rounding in
        // internal_nodes(). Shifting by block gives a constant cost change,
        // so it suffices to inspect the first and last blocks.
        auto examine = [&](int64 l, int64 r) {
            evaluate(l);
            evaluate(r);
            for (bool other : {false, true}) {
                auto nodes_at = [&](int64 y) {
                    int64 t = 2 * y - k;
                    return internal_nodes(flat_count(other ? width - t : t));
                };
                int64 init = nodes_at(l);
                if (nodes_at(r) == init) continue;
                int64 a = l + 1, b = r;
                while (a < b) {
                    int64 mid = (a + b) / 2;
                    if (nodes_at(mid) == init) a = mid + 1;
                    else b = mid;
                }
                evaluate(a - 1);
                evaluate(a);
            }
        };

        for (size_t i = 1; i < cuts.size(); ++i) {
            int64 l = cuts[i - 1], r = cuts[i] - 1;
            examine(l, min(r, l + block - 1));
            if (r - l + 1 > block) examine(max(l, r - block + 1), r);
        }
        return 2 * best_y;
    }

    void solve_gap(int64 midpoint, int64 count) {
        int64 s = count == 1 ? first_s : read_at(midpoint - mod(midpoint, 2));
        report(midpoint, s);
    }

    void solve_flat(int64 lo, int64 q) {
        // Each query returns up to h individual midpoints between two
        // plateau responses. Pack the internal nodes breadth first.
        while (q > 1) {
            int64 nodes = internal_nodes(q);
            int64 left;
            if (nodes == 1) {
                left = (q == h + 2 ? 1 : 0);
            } else {
                int64 half = 1LL << (log2_floor(nodes) - 1);
                int64 last_level = nodes - (2 * half - 1);
                int64 left_nodes = half - 1 + min(last_level, half);
                int64 unused = 1 + block * nodes - q;
                left = 1 + block * left_nodes - unused;
            }

            int64 x = lo + 2 * (left - 1) + n - k;
            x += mod(x, 2);
            int64 z = read_at(x);
            if (z == n - k) {
                q = left;
            } else if (z == k) {
                lo += 2 * (left + h);
                q -= left + h;
            } else {
                report(x - z, k);
                return;
            }
        }
        report(lo, k);
    }

public:
    void run() {
        if (!(cin >> n) || n < 3) return;
        m = n / 2;
        int64 r = ask(0);
        flip = r > m;
        k = flip ? n - r : r;
        top = k - flip;
        width = n - 2 * k;
        h = m - k;
        block = h + 1;
        first_s = top % 2 ? 1 : 2;
        choices_s = (top + 1) / 2;

        int64 interior = k - 1 + flip;
        if (interior + 2 * choices_s == 1) {
            report(0, k);
            return;
        }

        int64 x = choose_second_query();
        int64 t = x - k, u = width - t;
        int64 z = read_at(x);
        int64 right = flat_count(t), left = flat_count(u);

        if (z == k && right > 0) {
            solve_flat(top - 2 * (right - 1), right);
        } else if (z == n - k && left > 0) {
            solve_flat(-top, left);
        } else if (z == abs(t) && gap_count(t) > 0) {
            solve_gap(k, gap_count(t));
        } else if (z == n - abs(u) && gap_count(u) > 0) {
            solve_gap(-k, gap_count(u));
        } else if (z <= k) {
            report(k, z);
        } else if (z >= n - k) {
            report(-k, n - z);
        } else {
            report(x - z, k);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver().run();
    return 0;
}
