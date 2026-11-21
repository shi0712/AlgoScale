#include <bits/stdc++.h>
using namespace std;

using ll = long long;
constexpr ll MAX_SCORE = 1000000000LL;
constexpr ll INF = 1LL << 55;
using Scores = array<ll, 7>;

ll ask(int a, int b, int c) {
    cout << "? " << a << ' ' << b << ' ' << c << endl;
    ll reply;
    if (!(cin >> reply) || reply < 0) exit(0);
    return reply;
}

ll half_floor(ll x) {
    return x >= 0 ? x / 2 : (x - 1) / 2;
}

struct Bootstrap {
    struct Region {
        array<ll, 35> low, high;
    };

    int k;
    vector<int> labels;
    vector<array<int, 3>> triples;
    array<int, 128> by_mask{};
    vector<ll> replies;
    vector<int> queried;
    vector<array<int, 7>> orders;

    explicit Bootstrap(vector<int> indices)
        : k(int(indices.size())), labels(move(indices)) {
        for (int i = 0; i < k; ++i)
            for (int j = i + 1; j < k; ++j)
                for (int h = j + 1; h < k; ++h) {
                    by_mask[(1 << i) | (1 << j) | (1 << h)] = int(triples.size());
                    triples.push_back({i, j, h});
                }
        replies.assign(triples.size(), -1);
        array<int, 7> p{};
        iota(p.begin(), p.end(), 0);
        do {
            orders.push_back(p);
        } while (next_permutation(p.begin(), p.begin() + k));
    }

    int id(int a, int b, int c) const {
        return by_mask[(1 << a) | (1 << b) | (1 << c)];
    }

    void remember(int q, ll answer) {
        queried.push_back(q);
        replies[q] = answer;
    }

    ll query(int q) {
        auto [a, b, c] = triples[q];
        ll answer = ask(labels[a], labels[b], labels[c]);
        remember(q, answer);
        return answer;
    }

    ll value_of(const Scores& s, int q) const {
        auto [a, b, c] = triples[q];
        return min({s[a], s[b], s[c]}) + max({s[a], s[b], s[c]});
    }

    int choose_from_points(const vector<Scores>& points,
                           const vector<int>& available) const {
        int m = int(points.size());
        if (m <= 13) {
            vector<vector<int>> splits(available.size());
            for (int j = 0; j < int(available.size()); ++j) {
                map<ll, int> groups;
                for (int i = 0; i < m; ++i)
                    groups[value_of(points[i], available[j])] |= 1 << i;
                if (groups.size() > 1)
                    for (auto [v, mask] : groups)
                        splits[j].push_back(mask);
            }

            struct State {
                int depth, total, query;
            };
            vector<State> dp(1 << m, {-1, 0, -1});
            auto solve = [&](auto&& self, int mask) -> State {
                if ((mask & (mask - 1)) == 0) return {0, 0, -1};
                State& result = dp[mask];
                if (result.depth != -1) return result;
                result = {100, INT_MAX, -1};
                for (int j = 0; j < int(splits.size()); ++j) {
                    if (splits[j].empty()) continue;
                    int depth = 0, total = __builtin_popcount(unsigned(mask));
                    bool useful = true;
                    for (int group : splits[j]) {
                        int part = mask & group;
                        if (part == mask) {
                            useful = false;
                            break;
                        }
                        if (!part) continue;
                        State sub = self(self, part);
                        depth = max(depth, sub.depth + 1);
                        total += sub.total;
                    }
                    if (useful && tie(depth, total) < tie(result.depth, result.total))
                        result = {depth, total, available[j]};
                }
                return result;
            };
            return solve(solve, (1 << m) - 1).query;
        }

        pair<ll, int> best{LLONG_MAX, INT_MAX};
        int chosen = -1;
        for (int q : available) {
            map<ll, int> groups;
            for (const auto& s : points) ++groups[value_of(s, q)];
            if (groups.size() <= 1) continue;
            ll squares = 0;
            int largest = 0;
            for (auto [v, count] : groups) {
                squares += 1LL * count * count;
                largest = max(largest, count);
            }
            if (make_pair(squares, largest) < best) {
                best = {squares, largest};
                chosen = q;
            }
        }
        return chosen;
    }

    Scores solve() {
        while (true) {
            vector<int> available;
            for (int q = 0; q < int(triples.size()); ++q)
                if (replies[q] < 0) available.push_back(q);

            Scores global_low, global_high;
            global_low.fill(INF);
            global_high.fill(-INF);
            vector<Region> regions;
            vector<Scores> points;
            vector<array<int, 7>> surviving;
            int nodes = 2 * k;

            for (const auto& order : orders) {
                int rank[7];
                for (int i = 0; i < k; ++i) rank[order[i]] = i;

                auto ends = [&](int q) -> pair<int, int> {
                    auto [a, b, c] = triples[q];
                    if (rank[a] > rank[b]) swap(a, b);
                    if (rank[b] > rank[c]) swap(b, c);
                    if (rank[a] > rank[b]) swap(a, b);
                    return {a, c};
                };

                // Node 2*i represents c[i]; node 2*i+1 represents -c[i].
                // d[u][v] bounds the difference node[v] - node[u].
                ll d[14][14];
                for (int u = 0; u < nodes; ++u) {
                    fill(d[u], d[u] + nodes, INF);
                    d[u][u] = 0;
                }
                for (int i = 0; i < k; ++i) {
                    d[2 * i][2 * i + 1] = 0;
                    d[2 * i + 1][2 * i] = 2 * MAX_SCORE;
                }
                for (int i = 1; i < k; ++i) {
                    int a = 2 * order[i - 1], b = 2 * order[i];
                    d[b][a] = d[a ^ 1][b ^ 1] = 0;
                }
                for (int q : queried) {
                    auto [a, b] = ends(q);
                    a *= 2;
                    b *= 2;
                    ll r = replies[q];
                    d[a ^ 1][b] = min(d[a ^ 1][b], r);
                    d[b ^ 1][a] = min(d[b ^ 1][a], r);
                    d[a][b ^ 1] = min(d[a][b ^ 1], -r);
                    d[b][a ^ 1] = min(d[b][a ^ 1], -r);
                }

                bool feasible = true;
                for (int h = 0; h < nodes && feasible; ++h) {
                    for (int u = 0; u < nodes; ++u) {
                        if (d[u][h] == INF) continue;
                        for (int v = 0; v < nodes; ++v)
                            if (d[h][v] != INF)
                                d[u][v] = min(d[u][v], d[u][h] + d[h][v]);
                        if (d[u][u] < 0) {
                            feasible = false;
                            break;
                        }
                    }
                }
                if (!feasible) continue;

                // Tight closure for integer octagonal constraints.
                ll unary[14];
                for (int u = 0; u < nodes; ++u)
                    unary[u] = half_floor(d[u][u ^ 1]);
                for (int i = 0; i < k; ++i)
                    if (unary[2 * i] + unary[2 * i + 1] < 0)
                        feasible = false;
                if (!feasible) continue;
                surviving.push_back(order);

                Scores point{};
                bool fixed = true;
                for (int i = 0; i < k; ++i) {
                    ll lo = -unary[2 * i], hi = unary[2 * i + 1];
                    global_low[i] = min(global_low[i], lo);
                    global_high[i] = max(global_high[i], hi);
                    point[i] = lo;
                    fixed &= (lo == hi);
                }
                if (fixed) {
                    points.push_back(point);
                    continue;
                }

                Region region{};
                for (int q : available) {
                    auto [a, b] = ends(q);
                    int u = 2 * a + 1, v = 2 * b;
                    region.low[q] = -min(d[v][u], unary[v] + unary[u ^ 1]);
                    region.high[q] = min(d[u][v], unary[u] + unary[v ^ 1]);
                }
                regions.push_back(region);
            }
            orders.swap(surviving);

            bool done = true;
            for (int i = 0; i < k; ++i) done &= (global_low[i] == global_high[i]);
            if (done) return global_low;

            sort(points.begin(), points.end());
            points.erase(unique(points.begin(), points.end()), points.end());

            int chosen = -1;
            if (regions.empty()) {
                chosen = choose_from_points(points, available);
            } else {
                // Prefer queries that separate the remaining possibilities.
                vector<pair<ll, long double>> events;
                events.reserve(2 * (regions.size() + points.size()));
                long double best = -numeric_limits<long double>::infinity();
                for (int q : available) {
                    events.clear();
                    auto add = [&](ll lo, ll hi) {
                        long double weight = 1.0L / (hi - lo + 1);
                        events.emplace_back(lo, weight);
                        events.emplace_back(hi + 1, -weight);
                    };
                    for (const auto& r : regions) add(r.low[q], r.high[q]);
                    for (const auto& s : points) {
                        ll v = value_of(s, q);
                        add(v, v);
                    }
                    sort(events.begin(), events.end());
                    if (events.front().first + 1 == events.back().first) continue;
                    long double density = 0, utility = 0;
                    ll previous = events.front().first;
                    for (auto [at, change] : events) {
                        if (at > previous && density > 0)
                            utility -= (at - previous) * density * logl(density);
                        density += change;
                        previous = at;
                    }
                    if (utility > best) {
                        best = utility;
                        chosen = q;
                    }
                }
            }
            query(chosen);
        }
    }
};

struct Known {
    vector<ll> score;
    set<pair<ll, int>> sorted;
    pair<ll, int> left{}, right{};
    ll gap = INF;
    int zeros = 0, maximums = 0;

    explicit Known(int n) : score(n + 1, -1) {}

    void add(int i, ll value) {
        score[i] = value;
        zeros += value == 0;
        maximums += value == MAX_SCORE;
        pair<ll, int> item{value, i};
        auto after = sorted.lower_bound(item);
        auto consider = [&](pair<ll, int> a, pair<ll, int> b) {
            ll g = b.first - a.first;
            if (g < gap) {
                gap = g;
                left = a;
                right = b;
            }
        };
        if (after != sorted.end()) consider(item, *after);
        if (after != sorted.begin()) consider(*prev(after), item);
        sorted.emplace_hint(after, item);
    }

    ll read_score(int i) const {
        auto [a, x] = left;
        auto [b, y] = right;
        ll s = ask(x, y, i);
        if (s < a + b) return s - b;
        if (s > a + b) return s - a;
        if (a == b) return a;

        // The first reply puts c[i] in [a,b]. The second pair
        // has its entire interval on one side of [a,b].
        auto [lo, lo_id] = *sorted.begin();
        if (lo < a) return ask(lo_id, x, i) - lo;
        auto [hi, hi_id] = *sorted.rbegin();
        return ask(y, hi_id, i) - hi;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 5) return 0;

    vector<int> index(n);
    iota(index.begin(), index.end(), 1);
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    shuffle(index.begin(), index.end(), rng);

    Known known(n);
    ll first = ask(index[0], index[1], index[2]);
    int at;
    if (first == 0 || first == 2 * MAX_SCORE) {
        for (int j = 0; j < 3; ++j) known.add(index[j], first / 2);
        at = 3;
    } else {
        ll second = ask(index[0], index[3], index[4]);
        int k = (first == second ? min(n, 7) : 5);
        Bootstrap base(vector<int>(index.begin(), index.begin() + k));
        base.remember(base.id(0, 1, 2), first);
        base.remember(base.id(0, 3, 4), second);

        if (first == second && k == 7) {
            // The seven lines of the Fano plane handle a common reply
            // without spending extra queries on the uniform instance.
            for (auto t : {array<int, 3>{0, 5, 6}, {1, 3, 5}, {1, 4, 6},
                           {2, 3, 6}, {2, 4, 5}})
                if (base.query(base.id(t[0], t[1], t[2])) != first) break;
        } else if (first == second && k == 6) {
            vector<int> plan{base.id(0, 1, 5), base.id(2, 3, 5)};
            for (int q = 0; q < int(base.triples.size()); ++q) {
                if (base.triples[q][2] == 5 || base.replies[q] >= 0) continue;
                if (q != base.id(2, 3, 4) && q != base.id(0, 1, 4))
                    plan.push_back(q);
            }
            for (int q : plan)
                if (base.query(q) != first) break;
        }

        Scores start = base.solve();
        for (int j = 0; j < k; ++j) known.add(index[j], start[j]);
        at = k;
    }

    int cooldown = 0, failed_batches = 0;
    while (at < n) {
        int size = int(known.sorted.size());
        int popular = max(known.zeros, known.maximums);
        bool batch = n - at >= 2 && known.gap == 0 && cooldown == 0 &&
                     5 * popular >= 4 * size;

        if (!batch) {
            int i = index[at++];
            known.add(i, known.read_score(i));
            if (cooldown > 0) --cooldown;
            continue;
        }

        bool success = false;
        if (n - at >= 3) {
            int a = index[at++], b = index[at++], c = index[at++];
            ll sum = ask(a, b, c);
            if (sum == 0 || sum == 2 * MAX_SCORE) {
                known.add(a, sum / 2);
                known.add(b, sum / 2);
                known.add(c, sum / 2);
                success = true;
            } else {
                ll x = known.read_score(a);
                known.add(a, x);
                ll y = known.read_score(b);
                known.add(b, y);
                ll z;
                if (sum < x + y) z = sum - max(x, y);
                else if (sum > x + y) z = sum - min(x, y);
                else if (x == y) z = x;
                else z = known.read_score(c);
                known.add(c, z);
            }
        } else {
            int a = index[at++], b = index[at++];
            bool low = known.zeros >= known.maximums;
            auto [edge, anchor] = low ? *known.sorted.begin() : *known.sorted.rbegin();
            ll extreme = ask(anchor, a, b) - edge;
            if (extreme == edge) {
                known.add(a, edge);
                known.add(b, edge);
                success = true;
            } else {
                ll x = known.read_score(a);
                known.add(a, x);
                known.add(b, x != extreme ? extreme : known.read_score(b));
            }
        }
        if (!success) cooldown = 4 << min(failed_batches++, 8);
    }

    cout << '!';
    for (int i = 1; i <= n; ++i) cout << ' ' << known.score[i];
    cout << endl;
    return 0;
}
