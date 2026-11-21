#include <bits/stdc++.h>
using namespace std;

using Mask = uint64_t;

int n, k;
Mask all;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

Mask bit(int x) { return Mask(1) << x; }
int first(Mask x) { return countr_zero(x); }
int cnt(Mask x) { return popcount(x); }

void move_wheel(int i, int d) {
    if (d > n / 2) d -= n;
    if (d < -n / 2) d += n;
    cout << i + 1 << ' ' << d << '\n' << flush;
    if (!(cin >> k) || k <= 1 || k > n) exit(0);
}

Mask rotate_mask(Mask s, int d) {
    if (!d) return s;
    return ((s << d) & all) | (s >> (n - d));
}

bool periodic(Mask s) {
    for (int d = 1; d <= n / 2; ++d)
        if (n % d == 0 && rotate_mask(s, d) == s) return true;
    return false;
}

void solve_three() {
    int kind[9], go[6][9];
    int valid[4]{};
    for (int s = 0; s < 9; ++s) {
        int x = s % 3, y = s / 3;
        kind[s] = cnt(1 | bit(x) | bit(y));
        valid[kind[s]] |= 1 << s;
        for (int d = 1; d <= 2; ++d) {
            go[d - 1][s] = (x + 3 - d) % 3 + 3 * ((y + 3 - d) % 3);
            go[d + 1][s] = (x + d) % 3 + 3 * y;
            go[d + 3][s] = x + 3 * ((y + d) % 3);
        }
    }

    int to[512][6][4]{};
    vector<int> by_size[10];
    for (int c = 2; c <= 3; ++c)
        for (int m = valid[c]; m; m = (m - 1) & valid[c]) {
            by_size[popcount((unsigned)m)].push_back(m);
            for (int a = 0; a < 6; ++a)
                for (int s = 0; s < 9; ++s)
                    if (m >> s & 1) {
                        int t = go[a][s];
                        to[m][a][kind[t]] |= 1 << t;
                    }
        }
    int dp[512], choice[512]{};
    fill(dp, dp + 512, 1000000);
    dp[0] = 0;
    for (int size = 1; size <= 9; ++size) {
        bool changed;
        do {
            changed = false;
            for (int m : by_size[size])
                for (int a = 0; a < 6; ++a) {
                    int value = size + dp[to[m][a][2]] + dp[to[m][a][3]];
                    if (value < dp[m]) {
                        dp[m] = value;
                        choice[m] = a;
                        changed = true;
                    }
                }
        } while (changed);
    }
    int m = valid[k];
    while (true) {
        int a = choice[m];
        move_wheel(a / 2, a % 2 + 1);
        m = to[m][a][k];
    }
}

double ways[2][51][51];

// Decide where to place a wheel whose original position is known only
// to belong to the surviving singleton / nonsingleton hypotheses.
struct WheelSearch {
    int anchor;
    Mask hit[50], goal[50];
    double wu, wd;

    WheelSearch(Mask occupied, int a, double u, double d)
        : anchor(a), wu(u), wd(d) {
        for (int j = 0; j < n; ++j) {
            hit[j] = rotate_mask(occupied, (n - j) % n);
            goal[j] = bit((a + n - j) % n);
        }
    }

    double mass(Mask u, Mask d) const { return cnt(u) * wu + cnt(d) * wd; }

    int exact(Mask u, Mask d) const {
        int x[11], h = 0;
        bool unique[11];
        for (Mask b = u; b; b &= b - 1) {
            x[h] = first(b);
            unique[h++] = true;
        }
        for (Mask b = d; b; b &= b - 1) {
            x[h] = first(b);
            unique[h++] = false;
        }

        int full = (1 << h) - 1;
        int parts[50][3]{}, aligned[50]{};
        for (int j = 1; j < n; ++j) {
            for (int b = 0; b < h; ++b) {
                bool occupied = (hit[j] & bit(x[b])) != 0;
                int r = 1 + !occupied - unique[b];
                parts[j][r] |= 1 << b;
                if (goal[j] & bit(x[b])) aligned[j] |= 1 << b;
            }
        }

        double weight[2048], memo[2048];
        bool single[2048]{};
        Mask positions[2048];
        weight[0] = 0;
        positions[0] = 0;
        fill(memo, memo + full + 1, -1.0);
        for (int s = 1; s <= full; ++s) {
            int b = countr_zero((unsigned)s), t = s & (s - 1);
            weight[s] = weight[t] + (unique[b] ? wu : wd);
            positions[s] = positions[t] | bit(x[b]);
            single[s] = has_single_bit(positions[s]);
        }

        int best_action = 1;
        auto dfs = [&](auto&& self, int s) -> double {
            if (memo[s] >= 0) return memo[s];
            double best = numeric_limits<double>::infinity();
            for (int j = 1; j < n; ++j) {
                int a = s & parts[j][0], b = s & parts[j][1], c = s & parts[j][2];
                if (a == s || b == s || c == s) continue;
                double value = weight[s];
                for (int t : {a, b, c}) {
                    if (!t) continue;
                    if (single[t]) {
                        if (!(t & aligned[j])) value += weight[t];
                    } else {
                        value += self(self, t);
                    }
                }
                if (value < best) {
                    best = value;
                    if (s == full) best_action = j;
                }
            }
            return memo[s] = best;
        };
        dfs(dfs, full);
        return best_action;
    }

    int choose(Mask u, Mask d) const {
        if (cnt(u) + cnt(d) <= 11) return exact(u, d);
        double total = mass(u, d);
        double best = -1;
        int answer = 1;
        for (int j = 1; j < n; ++j) {
            Mask a = u & hit[j], b = u & ~hit[j];
            Mask c = d & hit[j], e = d & ~hit[j];
            if ((!a) + (!(b | c)) + (!e) >= 2) continue;
            double score = 0;
            auto add = [&](Mask x, double w) {
                if (!x) return;
                double p = w / total;
                score -= p * log2(p);
                if (x == goal[j]) score += 0.85 * p;
            };
            add(a, mass(a, 0));
            add(b | c, mass(b, c));
            add(e, mass(0, e));
            if (score > best) {
                best = score;
                answer = j;
            }
        }
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> k) || k <= 1 || n < 3 || n > 50 || k > n) return 0;
    all = bit(n) - 1;
    if (n == 3) {
        solve_three();
        return 0;
    }

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    shuffle(order.begin(), order.end(), rng);
    int scout = order[0];
    int known_scout_size = -1;

    // With few holes, a positional probe can rule out very few positions.
    // A short merge phase can create holes cheaply in this regime.
    if (n >= 18 && 5 * k >= 4 * n && (k < n || n >= 44)) {
        int stop = (3 * n + 3) / 4;
        for (int t = 0; t < n / 2 && k > stop; ++t) {
            scout = order[t];
            int before = k;
            move_wheel(scout, uniform_int_distribution<int>(1, n - 1)(rng));
            known_scout_size = (k < before ? k : k > before ? k - 1 : -1);
        }
    }

    // Use the scout's current symbol as the coordinate origin. Sweeping
    // only the scout measures membership in the set of the other wheels.
    int start_k = k, scout_pos = 0;
    int seen[50]{};
    seen[0] = k;
    vector<int> sweep(n - 1);
    iota(sweep.begin(), sweep.end(), 1);
    shuffle(sweep.begin(), sweep.end(), rng);
    Mask rest = 0;
    for (int step = 0;; ++step) {
        int feasible = 0;
        bool complete = false;
        for (int sz = start_k - 1; sz <= start_k; ++sz) {
            if (sz < 1 || sz >= n ||
                (known_scout_size != -1 && sz != known_scout_size)) continue;
            Mask in = 0, out = 0;
            bool ok = true;
            for (int x = 0; x < n; ++x) {
                if (!seen[x]) continue;
                if (seen[x] == sz) in |= bit(x);
                else if (seen[x] == sz + 1) out |= bit(x);
                else ok = false;
            }
            if (!ok || cnt(in) > sz || cnt(out) > n - sz) continue;
            ++feasible;
            if (cnt(in) == sz || cnt(out) == n - sz) {
                rest = cnt(in) == sz ? in : all ^ out;
                complete = true;
            }
        }
        if (feasible == 1 && complete) break;
        int x = sweep[step];
        move_wheel(scout, x - scout_pos);
        scout_pos = x;
        seen[x] = k;
    }

    int anchor;
    if (rest & bit(scout_pos)) {
        anchor = scout_pos;
    } else {
        anchor = (rest & 1) ? 0 : first(rest);
        if (!periodic(rest)) {
            move_wheel(scout, anchor - scout_pos);
            scout_pos = anchor;
        }
    }
    Mask occupied = rest | bit(scout_pos);
    int locked = 0;

    for (int f = 0; f <= 1; ++f) {
        ways[f][0][0] = 1;
        for (int m = 1; m <= n; ++m)
            for (int r = 0; r <= n; ++r)
                ways[f][m][r] = (r + f) * ways[f][m - 1][r]
                             + (r ? r * ways[f][m - 1][r - 1] : 0);
    }

    order.erase(find(order.begin(), order.end(), scout));
    shuffle(order.begin(), order.end(), rng);
    for (int t = 0; t < (int)order.size(); ++t) {
        if (scout_pos != anchor && k == 2)
            move_wheel(scout, anchor - scout_pos);

        // A periodic mask makes some positions indistinguishable. Moving
        // the scout breaks the period. A scout outside the anchor always
        // occupies a symbol that no other wheel occupies.
        if (periodic(occupied)) {
            if (scout_pos != anchor) {
                move_wheel(scout, anchor - scout_pos);
                occupied ^= bit(scout_pos);
                scout_pos = anchor;
            } else if (occupied == all) {
                int next = (anchor + 1) % n;
                move_wheel(scout, next - anchor);
                occupied ^= bit(anchor);
                anchor = scout_pos = next;
            } else {
                int next = first(all ^ occupied);
                int old_k = k;
                move_wheel(scout, next - anchor);
                if (k == old_k) {
                    occupied ^= bit(anchor);
                    anchor = first(occupied);
                }
                scout_pos = next;
                occupied |= bit(next);
            }
        }

        int base = k, offset = 0;
        int remaining = (int)order.size() - t;
        bool beacon = scout_pos != anchor;
        int optional = !beacon || locked != 0;
        Mask possible = occupied & ~(beacon ? bit(scout_pos) : Mask(0));
        Mask required = possible & ~(optional ? bit(anchor) : Mask(0));
        int r = cnt(required);
        double wu = r ? ways[optional][remaining - 1][r - 1] : 0;
        double wd = ways[optional][remaining - 1][r];
        Mask u = wu > 0 ? required : 0;
        Mask d = wd > 0 ? possible : 0;
        double norm = max(wu, wd);
        WheelSearch search(occupied, anchor, wu / norm, wd / norm);

        while (!has_single_bit(u | d)) {
            int next = search.choose(u, d);
            move_wheel(order[t], next - offset);
            offset = next;
            Mask hit = search.hit[next];
            if (k < base) {
                u &= hit;
                d = 0;
            } else if (k > base) {
                u = 0;
                d &= ~hit;
            } else {
                u &= ~hit;
                d &= hit;
            }
        }

        int x = first(u | d);
        int goal = (anchor + n - x) % n;
        if (goal != offset) move_wheel(order[t], goal - offset);
        if (x != anchor && k < base) occupied ^= bit(x);
        ++locked;
    }

    if (scout_pos != anchor) move_wheel(scout, anchor - scout_pos);
    return 0;
}
