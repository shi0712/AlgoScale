#include <bits/stdc++.h>
using namespace std;

int n, totalBuckets;

struct Query {
    int l, r, k;
};

int ask(Query q) {
    cout << "? " << q.l << ' ' << q.r << ' ' << q.k << endl;
    int answer;
    if (!(cin >> answer) || answer < 1) exit(0);
    return answer;
}

void finish(int position) {
    cout << "! " << position << endl;
}

// For k=2 each bucket is a pair, except for 1 and, when n is even, n.
// At a cut, A counts first occurrences and B counts last occurrences.
// A+B minus the cut index is the number of singletons to its left.
struct Interval {
    int l, r, a0, b0, a1, b1;

    int length() const { return r - l + 1; }
    int singles() const { return a1 - a0 + b1 - b0 - length(); }
    int opens() const { return a1 - a0 - singles(); }
    int height() const { return a0 - b0; }
};

Query queryN(int cut) {
    if (cut >= 2) return {1, cut, n};
    return {cut + 1, n, n};
}

bool nIsLeft(int cut) {
    return (ask(queryN(cut)) == 2) == (cut >= 2);
}

// Exact decision trees for small remaining instances.
constexpr int WORDS = 7;
using Mask = array<uint64_t, WORDS>;

struct MaskHash {
    size_t operator()(const Mask& mask) const {
        uint64_t h = 0x9e3779b97f4a7c15ULL;
        for (uint64_t x : mask) {
            x ^= x >> 30;
            x *= 0xbf58476d1ce4e5b9ULL;
            x ^= x >> 27;
            h ^= x + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        }
        return h;
    }
};

void setBit(Mask& mask, int index) {
    mask[index / 64] |= uint64_t(1) << (index % 64);
}

struct Hypothesis {
    int target;
    double weight;
    vector<int> answers;
};

class SmallTree {
    struct Action {
        Query query;
        vector<pair<int, Mask>> groups;
        double order;
    };
    struct Decision {
        double cost;
        int action;
    };
    struct Limit {};

    vector<Action> actions;
    vector<pair<int, Mask>> targets;
    unordered_map<Mask, Decision, MaskHash> memo;
    array<array<double, 256>, WORDS * 8> weights{};
    int budget = 800000;

    int determined(const Mask& state) const {
        for (const auto& [position, mask] : targets) {
            bool subset = true;
            for (int j = 0; j < WORDS; ++j)
                if (state[j] & ~mask[j]) {
                    subset = false;
                    break;
                }
            if (subset) return position;
        }
        return 0;
    }

    double weight(const Mask& state) const {
        double result = 0;
        for (int j = 0; j < WORDS; ++j) {
            uint64_t word = state[j];
            for (int b = 0; word; ++b, word >>= 8)
                result += weights[8 * j + b][word & 255];
        }
        return result;
    }

    double solve(const Mask& state) {
        if (determined(state)) return 0;
        auto it = memo.find(state);
        if (it != memo.end()) return it->second.cost;

        double w = weight(state);
        double best = numeric_limits<double>::infinity();
        int bestAction = -1;
        for (int i = 0; i < (int)actions.size(); ++i) {
            if (--budget < 0) throw Limit{};
            array<Mask, 12> children;
            int count = 0, unresolved = 0;
            double lower = w;
            for (const auto& [response, group] : actions[i].groups) {
                Mask child;
                uint64_t nonempty = 0;
                for (int j = 0; j < WORDS; ++j) {
                    child[j] = state[j] & group[j];
                    nonempty |= child[j];
                }
                if (!nonempty) continue;
                ++count;
                if (child == state) break;
                if (!determined(child)) {
                    children[unresolved++] = child;
                    lower += weight(child);
                }
            }
            if (count <= 1 || lower >= best) continue;

            double cost = w;
            for (int j = 0; j < unresolved && cost < best; ++j)
                cost += solve(children[j]);
            if (cost < best) {
                best = cost;
                bestAction = i;
                if (best == w) break;
            }
        }
        if (bestAction == -1) throw Limit{};
        memo.emplace(state, Decision{best, bestAction});
        return best;
    }

public:
    int run(const vector<Query>& queries, const vector<Hypothesis>& hypotheses) {
        int size = (int)hypotheses.size();
        if (!size || size > 64 * WORDS) return 0;

        Mask state{};
        map<int, Mask> byTarget;
        for (int i = 0; i < size; ++i) {
            setBit(state, i);
            setBit(byTarget[hypotheses[i].target], i);
        }
        targets.assign(byTarget.begin(), byTarget.end());

        for (int b = 0; b < WORDS * 8; ++b) {
            for (unsigned v = 1; v < 256; ++v) {
                unsigned bit = countr_zero(v);
                int index = 8 * b + bit;
                weights[b][v] = weights[b][v & (v - 1)] +
                                (index < size ? hypotheses[index].weight : 0);
            }
        }

        set<vector<Mask>> seen;
        for (int q = 0; q < (int)queries.size(); ++q) {
            map<int, Mask> groups;
            for (int i = 0; i < size; ++i)
                setBit(groups[hypotheses[i].answers[q]], i);
            if (groups.size() <= 1) continue;
            vector<Mask> signature;
            double order = 0;
            for (const auto& [response, mask] : groups) {
                signature.push_back(mask);
                double w = weight(mask);
                order += w * w;
            }
            sort(signature.begin(), signature.end());
            if (!seen.insert(signature).second) continue;
            actions.push_back({queries[q], {groups.begin(), groups.end()}, order});
        }
        stable_sort(actions.begin(), actions.end(),
                    [](const Action& x, const Action& y) { return x.order < y.order; });

        memo.reserve(16384);
        try {
            solve(state);
        } catch (const Limit&) {
            return 0;
        }

        while (true) {
            int position = determined(state);
            if (position) return position;
            const Action& action = actions[memo.at(state).action];
            int response = ask(action.query);
            bool found = false;
            for (const auto& [value, mask] : action.groups) {
                if (value != response) continue;
                uint64_t nonempty = 0;
                for (int j = 0; j < WORDS; ++j) {
                    state[j] &= mask[j];
                    nonempty |= state[j];
                }
                found = nonempty != 0;
                break;
            }
            if (!found) exit(0);
        }
    }
};

int tinyInstance() {
    vector<Query> queries;
    for (int l = 1; l <= n; ++l)
        for (int r = l + 1; r <= n; ++r)
            for (int k = 2; k <= n; ++k)
                queries.push_back({l, r, k});

    vector<Hypothesis> hypotheses;
    vector<int> p(n);
    iota(p.begin(), p.end(), 1);
    do {
        Hypothesis h{int(find(p.begin(), p.end(), 1) - p.begin()) + 1, 1, {}};
        for (Query q : queries) {
            unsigned values = 0;
            for (int i = q.l - 1; i < q.r; ++i) values |= 1U << (p[i] / q.k);
            h.answers.push_back(popcount(values));
        }
        hypotheses.push_back(move(h));
    } while (next_permutation(p.begin(), p.end()));

    return SmallTree().run(queries, hypotheses);
}

int smallInterval(const Interval& in) {
    const int m = in.length(), s = in.singles(), f = in.opens();
    const bool trackPairs = m <= (s == 1 ? 6 : 5);
    vector<Query> queries;
    vector<pair<int, int>> description;
    auto add = [&](Query q, int kind, int x) {
        queries.push_back(q);
        description.emplace_back(kind, x);
    };
    for (int a = 1; a < m; ++a) {
        int cut = in.l + a - 1;
        add({1, cut, 2}, 0, a);
        add({cut + 1, n, 2}, 1, a);
        if (s == 2) add(queryN(cut), 2, a);
    }
    if (trackPairs) {
        for (int l = in.l; l < in.r; ++l)
            for (int r = l + 1; r <= in.r; ++r)
                add({l, r, 2}, 3, 0);
    }

    vector<Hypothesis> hypotheses;
    int type[8], partner[8];
    fill(begin(partner), end(partner), -1);

    auto generate = [&](auto&& self, int at, int opens, int closes, bool one,
                        bool last, int height, unsigned pending, double w) -> void {
        if (at == m) {
            int a[9]{}, b[9]{}, p1 = -1, pn = -1;
            for (int j = 0; j < m; ++j) {
                a[j + 1] = a[j] + (type[j] != 1);
                b[j + 1] = b[j] + (type[j] != 0);
                if (type[j] == 2) p1 = j;
                if (type[j] == 3) pn = j;
            }

            Hypothesis h{in.l + p1, w, {}};
            for (int i = 0; i < (int)queries.size(); ++i) {
                auto [kind, x] = description[i];
                Query q = queries[i];
                int answer;
                if (kind == 0) answer = in.a0 + a[x];
                else if (kind == 1) answer = totalBuckets - in.b0 - b[x];
                else if (kind == 2)
                    answer = 1 + ((pn < x) == (in.l + x - 1 >= 2));
                else {
                    answer = q.r - q.l + 1;
                    for (int j = q.l - in.l; j <= q.r - in.l; ++j)
                        answer -= partner[j] >= q.l - in.l;
                }
                h.answers.push_back(answer);
            }
            hypotheses.push_back(move(h));
            return;
        }

        if (opens) {
            type[at] = 0;
            self(self, at + 1, opens - 1, closes, one, last, height + 1,
                 pending | (1U << at), w);
        }
        if (closes && height) {
            type[at] = 1;
            int external = trackPairs ? height - popcount(pending) : height;
            if (external)
                self(self, at + 1, opens, closes - 1, one, last, height - 1,
                     pending, w * external);
            if (trackPairs) {
                for (unsigned mask = pending; mask; mask &= mask - 1) {
                    int j = countr_zero(mask);
                    partner[at] = j;
                    self(self, at + 1, opens, closes - 1, one, last, height - 1,
                         pending ^ (1U << j), w);
                }
                partner[at] = -1;
            }
        }
        if (one) {
            type[at] = 2;
            self(self, at + 1, opens, closes, false, last, height, pending, w);
        }
        if (last) {
            type[at] = 3;
            self(self, at + 1, opens, closes, one, false, height, pending, w);
        }
    };
    generate(generate, 0, f, m - s - f, true, s == 2, in.height(), 0, 1);
    return SmallTree().run(queries, hypotheses);
}

// Choose a cut and which side to query first. If that response determines
// the singleton count at the cut, the complementary query is unnecessary.
constexpr int LIM = 96;
constexpr int MAXN = 1024;
vector<vector<long double>> chooseNumber;
long double halfPower[MAXN + 1];
double dp[3][LIM + 1][LIM + 1];

void initCombinations() {
    chooseNumber.resize(n + 1);
    for (int i = 0; i <= n; ++i) {
        chooseNumber[i].resize(i + 1, 1);
        for (int j = 1; j < i; ++j)
            chooseNumber[i][j] = chooseNumber[i - 1][j - 1] + chooseNumber[i - 1][j];
    }
    halfPower[0] = 1;
    for (int i = 1; i <= n; ++i) halfPower[i] = halfPower[i - 1] * 0.5L;
}

struct Split {
    int t, f, e;
};

template<class Callback>
void possibleSplits(int m, int f, int s, int a, int height, Callback callback) {
    int e = m - s - f;
    for (int t = 0; t <= s && t <= a; ++t) {
        int low = max(0, a - t - e);
        if (height >= 0) low = max(low, (a - t - height + 1) / 2);
        int high = min(f, a - t);
        for (int i = low; i <= high; ++i)
            callback(Split{t, i, a - t - i});
    }
}

// The matching weight for a block, with a common factorial factor removed.
// u is the number of pairs whose two ends are in the block.
long double matchingWeight(int height, int opens, int closes) {
    long double ways = 0;
    int high = min(opens, closes);
    for (int u = max(0, closes - height); u <= high; ++u)
        ways += chooseNumber[height][closes - u] *
                chooseNumber[opens][u] * halfPower[u];
    return ways;
}

pair<long double, int> evaluate(int m, int f, int s, int a, int height,
                                bool estimateFuture) {
    int b = m - a, e = m - s - f;
    long double groups[2][MAXN + 1];
    int counts[2][MAXN + 1];
    for (int k = 0; k < 2; ++k) {
        fill_n(groups[k], a + 1, 0.0L);
        fill_n(counts[k], a + 1, 0);
    }
    long double total = 0, future = 0;
    possibleSplits(m, f, s, a, height, [&](Split x) {
        long double w = chooseNumber[s][x.t] * chooseNumber[f][x.f];
        if (height == -1) {
            w *= chooseNumber[e][x.e];
        } else {
            w *= matchingWeight(height, x.f, x.e) *
                 matchingWeight(height + x.f - x.e, f - x.f, e - x.e);
        }
        int u = x.f + x.t, v = x.e + x.t;
        groups[0][u] += w;
        groups[1][v] += w;
        ++counts[0][u];
        ++counts[1][v];
        total += w;

        if (estimateFuture) {
            double cost;
            if (x.t == 0) cost = dp[s][b][f - x.f];
            else if (x.t == s) cost = dp[s][a][x.f];
            else cost = 1 + 0.5 * (dp[1][a][x.f] + dp[1][b][f - x.f]);
            future += w * cost;
        }
    });

    long double cost[2];
    for (int k = 0; k < 2; ++k) {
        long double extra = 0;
        int different = 0;
        for (int x = 0; x <= a; ++x) {
            different += counts[k][x] > 0;
            if (counts[k][x] > 1) extra += groups[k][x];
        }
        cost[k] = (different > 1) + (extra + future) / total;
    }
    int dir = cost[1] < cost[0] ? 1 : 0;
    return {cost[dir], dir};
}

void initDP() {
    for (int m = 2; m <= min(n, LIM); ++m) {
        for (int s = 1; s <= 2; ++s) {
            for (int f = 0; f <= m - s; ++f) {
                long double best = 1e9;
                for (int a = 1; a < m; ++a)
                    best = min(best, evaluate(m, f, s, a, -1, true).first);
                dp[s][m][f] = (double)best;
            }
        }
    }
}

pair<int, int> selectCut(const Interval& in) {
    int m = in.length(), f = in.opens(), s = in.singles();
    bool predict = m <= 2 * LIM;
    int low = predict ? max({1, m - LIM, m / 3}) : m / 2;
    int high = predict ? min({m - 1, LIM, m - m / 3}) : m / 2;
    int bestA = m / 2, bestDir = 0;
    long double bestCost = 1e9;
    for (int a = low; a <= high; ++a) {
        auto [cost, dir] = evaluate(m, f, s, a, in.height(), predict);
        if (cost < bestCost - 1e-12L ||
            (abs(cost - bestCost) <= 1e-12L && abs(m - 2 * a) < abs(m - 2 * bestA))) {
            bestCost = cost;
            bestA = a;
            bestDir = dir;
        }
    }
    return {bestA, bestDir};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n) || n < 3) return 0;
    totalBuckets = n / 2 + 1;

    if (n <= 5) {
        int position = tinyInstance();
        if (position) {
            finish(position);
            return 0;
        }
    }

    initCombinations();
    initDP();

    Interval in{1, n, 0, 0, totalBuckets, totalBuckets};
    while (in.l < in.r) {
        int m = in.length(), s = in.singles(), f = in.opens();
        if (m <= (s == 1 ? 8 : 7)) {
            int position = smallInterval(in);
            if (position) {
                finish(position);
                return 0;
            }
        }

        auto [a, dir] = selectCut(in);
        int cut = in.l + a - 1;
        vector<Split> candidates;
        possibleSplits(m, f, s, a, in.height(),
                       [&](Split x) { candidates.push_back(x); });

        auto value = [](Split x, int d) { return (d == 0 ? x.f : x.e) + x.t; };
        auto filter = [&](int d) {
            int expected = value(candidates.front(), d);
            if (all_of(candidates.begin(), candidates.end(),
                       [&](Split x) { return value(x, d) == expected; }))
                return;

            int obtained = d == 0 ? ask({1, cut, 2}) - in.a0
                                  : totalBuckets - ask({cut + 1, n, 2}) - in.b0;
            erase_if(candidates, [&](Split x) { return value(x, d) != obtained; });
            if (candidates.empty()) exit(0);
        };

        filter(dir);
        if (candidates.size() > 1) filter(dir ^ 1);
        Split x = candidates.front();

        bool left;
        if (x.t == 0) left = false;
        else if (x.t == s) left = true;
        else left = !nIsLeft(cut);

        int ac = in.a0 + x.f + x.t;
        int bc = in.b0 + x.e + x.t;
        if (left) in = {in.l, cut, in.a0, in.b0, ac, bc};
        else in = {cut + 1, in.r, ac, bc, in.a1, in.b1};
    }

    finish(in.l);
    return 0;
}
