#include <bits/stdc++.h>
using namespace std;

static int advance_dial(int p) {
    cout << "? " << p + 1 << endl;
    int r;
    if (!(cin >> r) || (r != 0 && r != 1)) exit(0);
    return r;
}

static void done() {
    cout << "!" << endl;
}

// Exact finite-horizon planning for the smallest board, modulo rotation.
static void solve_three() {
    constexpr int M = 512, H = 36;
    int next_state[9][3], answer[9][3];
    for (int s = 0; s < 9; ++s) {
        for (int p = 0; p < 3; ++p) {
            int a[3] = {0, s / 3, s % 3};
            a[p] = (a[p] + 1) % 3;
            answer[s][p] = 0;
            for (int j = 0; j < 3; ++j)
                if (j != p && a[j] == a[p]) answer[s][p] = 1;
            next_state[s][p] = 3 * ((a[1] - a[0] + 3) % 3)
                                  + ((a[2] - a[0] + 3) % 3);
        }
    }

    int trans[M][3][2] = {};
    for (int mask = 1; mask < M; ++mask)
        for (int s = 0; s < 9; ++s)
            if (mask >> s & 1)
                for (int p = 0; p < 3; ++p)
                    trans[mask][p][answer[s][p]] |= 1 << next_state[s][p];

    constexpr double INF = 1e9;
    double value[H + 1][M];
    int action[H + 1][M] = {};
    for (auto &row : value) {
        fill(begin(row), end(row), INF);
        row[0] = row[1] = 0;
    }
    for (int h = 1; h <= H; ++h) {
        for (int mask = 2; mask < M; ++mask) {
            for (int p = 0; p < 3; ++p) {
                int a = trans[mask][p][0], b = trans[mask][p][1];
                if (value[h - 1][a] == INF || value[h - 1][b] == INF)
                    continue;
                double v = 1 + (popcount((unsigned)a) * value[h - 1][a]
                              + popcount((unsigned)b) * value[h - 1][b])
                              / popcount((unsigned)mask);
                if (v < value[h][mask] - 1e-10) {
                    value[h][mask] = v;
                    action[h][mask] = p;
                }
            }
        }
    }
    int mask = M - 2, h = H;
    while (mask != 1) {
        while (h > 1 && value[h - 1][mask] <= value[h][mask] + 1e-10) --h;
        int p = action[h][mask];
        mask = trans[mask][p][advance_dial(p)];
        --h;
    }
    done();
}

constexpr int MAXN = 100;
using Bits = bitset<MAXN>;

// Number of assignments of k labeled dials to m allowed values,
// using each of r specified values at least once.
static double ways[MAXN + 1][MAXN + 1][MAXN + 1];

struct State {
    int x = -1;
    Bits required, allowed;
    int r = 0, m = 0;
    double weight = 0, optional_hit = 0;
};

class Solver {
    int n;
    vector<int> pos, count_at;
    vector<State> states;
    int focus = -1, offset = 0, consecutive = 0, left;
    int protected_hole = -1;

    int add(int x, int d) const {
        int y = x + d;
        return y < n ? y : y - n;
    }

    int dist(int x, int y) const {
        return y >= x ? y - x : y - x + n;
    }

    bool normalize(State &s) const {
        s.r = (int)s.required.count();
        if (s.r > left) return false;
        if (s.r == left) s.allowed = s.required;
        s.m = (int)s.allowed.count();
        if (left && !s.m) return false;
        if (s.m == 1) {
            s.required = s.allowed;
            s.r = 1;
        }
        s.weight = ways[left][s.m][s.r];
        s.optional_hit = s.m > s.r
            ? 1.0 - ways[left][s.m - 1][s.r] / s.weight : 0.0;
        return true;
    }

    // Only the focus dial has an unknown moving coordinate. For each
    // hypothesis about it, all untouched dials have simple support bounds.
    int query(int p) {
        int y = -1;
        if (p == focus) {
            offset = add(offset, 1);
            ++consecutive;
        } else {
            --count_at[pos[p]];
            y = pos[p] = add(pos[p], 1);
            ++count_at[y];
            consecutive = 0;
        }

        int reply = advance_dial(p);
        int keep = 0;
        for (State s : states) {
            int z = y;
            bool already_matched;
            if (p == focus) {
                z = add(s.x, offset);
                already_matched = count_at[z] != 0;
            } else {
                already_matched = count_at[z] > 1 ||
                    (focus != -1 && add(s.x, offset) == z);
            }

            if (already_matched) {
                if (reply) states[keep++] = s;
            } else {
                if (reply) {
                    if (!s.allowed[z]) continue;
                    s.required.set(z);
                } else {
                    if (s.required[z]) continue;
                    s.allowed.reset(z);
                }
                if (normalize(s)) states[keep++] = s;
            }
        }
        states.resize(keep);
        return reply;
    }

    static double hit(const State &s, int y) {
        if (s.required[y]) return 1.0;
        return s.allowed[y] ? s.optional_hit : 0.0;
    }

    int best_target(const vector<double> &mass) const {
        double total = accumulate(mass.begin(), mass.end(), 0.0);
        double cost = 0;
        for (int x = 1; x < n; ++x) cost += (n - x) * mass[x];
        int best = 0;
        double best_cost = cost;
        for (int t = 1; t < n; ++t) {
            cost += total - n * mass[t];
            if (cost < best_cost - 1e-8 ||
                (abs(cost - best_cost) <= 1e-8 && mass[t] > mass[best] + 1e-8)) {
                best = t;
                best_cost = cost;
            }
        }
        return best;
    }

    struct Probe {
        int dial = -1, length = 0;
        double score = 0;
    };

    void nearest_probes(vector<int> &who, vector<int> &length) const {
        vector<int> at(n, -1);
        for (int p = 0; p < n; ++p)
            if (pos[p] >= 0) at[pos[p]] = p;
        int last = -1;
        for (int y = n - 1; y >= 0; --y) {
            if (at[y] != -1) {
                last = at[y];
                break;
            }
        }
        for (int y = 0; y < n; ++y) {
            if (at[y] != -1) last = at[y];
            who[y] = last;
            length[y] = dist(pos[last], y);
        }
    }

    void walk(const Probe &p) {
        for (int i = 0; i < p.length && states.size() > 1; ++i) query(p.dial);
    }

    // Guaranteed disambiguation, including rotationally periodic support.
    void resolve() {
        while (states.size() > 1 && consecutive < n) query(focus);
        if (states.size() == 1) return;

        vector<int> who(n), length(n);
        nearest_probes(who, length);
        double sum = 0;
        for (const State &s : states) sum += s.weight;
        int p = who[0];
        double best = -1;
        for (int y = 0; y < n; ++y) {
            if (!length[y]) continue;
            double empty = 0;
            for (const State &s : states)
                if (add(s.x, offset) != y)
                    empty += s.weight * (1 - hit(s, y));
            double score = empty / sum / length[y];
            if (score > best) {
                best = score;
                p = who[y];
            }
        }

        int d = 0;
        for (; d < n && states.size() > 1;) {
            ++d;
            if (query(p) == 0) break;
        }
        if (states.size() == 1) return;

        // If p stopped at a new value, that value was added to the
        // scanned support. Otherwise its starting value was unique:
        // leaving it now deletes that value from the scanned support.
        if (d == n) query(p);
        for (int i = 0; i < n && states.size() > 1; ++i) query(focus);
    }

    void locate() {
        int used = 0, setup_count = 0;
        vector<int> who(n), length(n);
        vector<double> w, mass(n);
        vector<int> current;

        while (states.size() > 1 && used < n + 12) {
            int k = (int)states.size();
            w.resize(k);
            current.resize(k);
            double sum = 0;
            for (const State &s : states) sum += s.weight;
            for (int i = 0; i < k; ++i) {
                w[i] = states[i].weight / sum;
                current[i] = add(states[i].x, offset);
            }
            fill(mass.begin(), mass.end(), 0.0);
            for (int y = 0; y < n; ++y) mass[y] = count_at[y];
            for (int i = 0; i < k; ++i) {
                const State &s = states[i];
                mass[current[i]] += w[i];
                if (!left) continue;
                double scale = w[i] * left / s.weight;
                double must = s.r ? scale * ways[left - 1][s.m][s.r - 1] : 0;
                double may = scale * ways[left - 1][s.m][s.r];
                for (size_t y = s.allowed._Find_first(); y < (size_t)n;
                     y = s.allowed._Find_next(y))
                    mass[y] += s.required[y] ? must : may;
            }
            int target = best_target(mass);
            if (protected_hole != -1 && left >= n / 4)
                target = add(protected_hole, 1);

            double forward_score = 0;
            for (int d = 1; d <= n - min(n, consecutive); ++d) {
                double mean = 0, second = 0, past = 0;
                for (int i = 0; i < k; ++i) {
                    int y = add(current[i], d);
                    double a = count_at[y] ? 1 : hit(states[i], y);
                    mean += w[i] * a;
                    second += w[i] * a * a;
                    if (dist(current[i], target) < d) past += w[i];
                }
                forward_score = max(forward_score,
                    (second - mean * mean) / (d * (1 + 1.6 * past)));
            }

            nearest_probes(who, length);
            Probe best;
            for (int y = 0; y < n; ++y) {
                int d = length[y], p = who[y];
                if (!d || d > n + 12 - used) continue;
                double mean = 0, second = 0;
                for (int i = 0; i < k; ++i) {
                    double a = current[i] == y ? 1 : hit(states[i], y);
                    mean += w[i] * a;
                    second += w[i] * a * a;
                }
                double cost = 0.85 + 0.75 * (d - 1);
                if (d > dist(pos[p], target)) cost *= 2.0;
                double score = (second - mean * mean) / cost;
                if (score > best.score) best = {p, d, score};
            }

            if (best.score > max(1e-12, 1.25 * forward_score)) {
                walk(best);
                used += best.length;
            } else if (forward_score > 1e-12) {
                query(focus);
                ++used;
            } else {
                // Try a short change to the known support before paying
                // for another complete scan.
                Probe setup;
                if (setup_count < 2) {
                    for (int origin = 0; origin < n; ++origin) {
                        if (!count_at[origin]) continue;
                        int p = who[origin];
                        for (int d = 1; d <= min(3, n - 1); ++d) {
                            int to = add(origin, d);
                            if (count_at[origin] > 1 && count_at[to]) continue;
                            for (int t = 1; t <= min(n, 10); ++t) {
                                double mean = 0, second = 0;
                                for (int i = 0; i < k; ++i) {
                                    int y = add(current[i], t);
                                    double a = count_at[y] + (y == to) - (y == origin)
                                             ? 1 : hit(states[i], y);
                                    mean += w[i] * a;
                                    second += w[i] * a * a;
                                }
                                double cost = t + d * (d <= dist(origin, target) ? 0.8 : 1.7);
                                double score = (second - mean * mean) / cost;
                                if (score > setup.score) setup = {p, d, score};
                            }
                        }
                    }
                }
                if (setup.score <= 1e-12) break;
                walk(setup);
                used += setup.length;
                ++setup_count;
            }
        }

        if (states.size() > 1) resolve();
    }

public:
    explicit Solver(int size)
        : n(size), pos(n, -1), count_at(n), left(n - 1) {}

    void run() {
        for (int m = 0; m <= n; ++m) ways[0][m][0] = 1;
        for (int k = 1; k <= n; ++k) {
            for (int m = 1; m <= n; ++m) {
                for (int r = 0; r <= min(k, m); ++r) {
                    ways[k][m][r] = (m - r) * ways[k - 1][m][r];
                    if (r) ways[k][m][r] += r * ways[k - 1][m][r - 1];
                }
            }
        }

        pos[0] = 0;
        count_at[0] = 1;
        State initial;
        for (int y = 0; y < n; ++y) initial.allowed.set(y);
        normalize(initial);
        states.push_back(initial);

        while (states[0].allowed != states[0].required) query(0);

        if (states[0].m == n - 1) {
            for (int y = 0; y < n; ++y)
                if (!states[0].allowed[y]) protected_hole = y;
            int t = add(protected_hole, 1);
            int d = dist(pos[0], t);
            if (d <= 2) {
                while (pos[0] != t) query(0);
            } else if (pos[0] == protected_hole) {
                query(0);
            }
        }

        for (int p = 1; p < n; ++p) {
            State base = states[0];
            if (base.m == 1) {
                int y = (int)base.allowed._Find_first();
                for (int j = p; j < n; ++j) {
                    pos[j] = y;
                    ++count_at[y];
                }
                break;
            }

            focus = p;
            offset = consecutive = 0;
            --left;
            states.clear();
            for (int x = 0; x < n; ++x) {
                if (!base.allowed[x]) continue;
                State s = base;
                s.x = x;
                s.required.reset(x);
                if (normalize(s)) states.push_back(s);
            }

            locate();
            pos[p] = add(states[0].x, offset);
            ++count_at[pos[p]];
            focus = -1;

            if (protected_hole != -1 && left >= n / 4 &&
                states[0].m > 1) {
                for (int j = 0; j <= p; ++j)
                    if (pos[j] == protected_hole) query(j);
            }
        }

        vector<double> mass(count_at.begin(), count_at.end());
        int target = best_target(mass);
        for (int p = 0; p < n; ++p)
            for (int d = dist(pos[p], target); d > 0; --d)
                advance_dial(p);
        done();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    if (n == 3) solve_three();
    else Solver(n).run();
    return 0;
}
