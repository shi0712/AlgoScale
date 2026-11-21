#include <bits/stdc++.h>
using namespace std;

using U64 = uint64_t;

struct Oracle {
    const vector<int>* ranks;
    bool reverseOrder;
    int queries = 0;

    bool better(int a, int b) {
        ++queries;
        if (ranks != nullptr) {
            return ((*ranks)[a] > (*ranks)[b]) != reverseOrder;
        }
        cout << "? " << a << ' ' << b << '\n' << flush;
        int w;
        if (!(cin >> w) || (w != a && w != b)) exit(0);
        return (w == a) != reverseOrder;
    }
};

struct Relations {
    Oracle& oracle;
    vector<int> label;
    array<U64, 64> below{};

    Relations(Oracle& o, vector<int> ids) : oracle(o), label(move(ids)) {}

    void link(int a, int b) {
        U64 bit = U64(1) << a;
        U64 add = below[b] | (U64(1) << b);
        below[a] |= add;
        for (int i = 0; i < (int)label.size(); ++i)
            if (below[i] & bit) below[i] |= add;
    }

    bool better(int a, int b) {
        if (below[a] & (U64(1) << b)) return true;
        if (below[b] & (U64(1) << a)) return false;
        bool ans = oracle.better(label[a], label[b]);
        if (ans) link(a, b);
        else link(b, a);
        return ans;
    }

    void forget(int a) {
        U64 mask = ~(U64(1) << a);
        for (int i = 0; i < (int)label.size(); ++i) below[i] &= mask;
        below[a] = 0;
    }
};

struct SmallCut {
    struct Entry {
        uint16_t ways, cost;
        uint8_t worst, a, b;
    };
    static inline unordered_map<U64, Entry> memo[7][7];

    static U64 link(U64 state, int n, int a, int b) {
        U64 mask = (U64(1) << n) - 1;
        U64 add = ((state >> (b * n)) & mask) | (U64(1) << b);
        for (int i = 0; i < n; ++i)
            if (i == a || ((state >> (i * n + a)) & 1))
                state |= add << (i * n);
        return state;
    }

    static unsigned fixedTop(U64 state, int n, int k) {
        U64 mask = (U64(1) << n) - 1;
        unsigned top = 0;
        for (int i = 0; i < n; ++i)
            if (popcount((state >> (i * n)) & mask) >= n - k)
                top |= 1u << i;
        return top;
    }

    static int orders(U64 state, int n) {
        unsigned pred[6]{};
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if ((state >> (i * n + j)) & 1) pred[j] |= 1u << i;
        int dp[64]{};
        dp[0] = 1;
        unsigned end = (1u << n) - 1;
        for (unsigned s = 0; s < end; ++s) {
            if (!dp[s]) continue;
            unsigned left = end ^ s;
            for (unsigned bits = left; bits; bits &= bits - 1) {
                int i = countr_zero(bits);
                if (!(pred[i] & left)) dp[s | (1u << i)] += dp[s];
            }
        }
        return dp[end];
    }

    static Entry strategy(U64 state, int n, int k) {
        auto& table = memo[n][k];
        auto it = table.find(state);
        if (it != table.end()) return it->second;

        Entry res{0, uint16_t(65535), uint8_t(255), 255, 255};
        if (popcount(fixedTop(state, n, k)) == k) {
            res = {uint16_t(orders(state, n)), 0, 0, 255, 255};
        } else {
            for (int a = 0; a < n; ++a) {
                for (int b = a + 1; b < n; ++b) {
                    if (((state >> (a * n + b)) & 1) ||
                        ((state >> (b * n + a)) & 1)) continue;
                    Entry x = strategy(link(state, n, a, b), n, k);
                    Entry y = strategy(link(state, n, b, a), n, k);
                    int ways = x.ways + y.ways;
                    int cost = ways + x.cost + y.cost;
                    int worst = 1 + max(x.worst, y.worst);
                    if (cost < res.cost || (cost == res.cost && worst < res.worst))
                        res = {uint16_t(ways), uint16_t(cost), uint8_t(worst),
                               uint8_t(a), uint8_t(b)};
                }
            }
        }
        table.emplace(state, res);
        return res;
    }

    template<class Compare>
    static unsigned run(U64 state, int n, int k, Compare cmp) {
        for (;;) {
            Entry e = strategy(state, n, k);
            if (e.a == 255) return fixedTop(state, n, k);
            if (cmp(e.a, e.b)) state = link(state, n, e.a, e.b);
            else state = link(state, n, e.b, e.a);
        }
    }
};

// If cut is true, partition at t. Otherwise put the exact rank t at t.
static void selectRange(vector<int>& v, int l, int r, int t, bool cut,
                        Relations& rel, mt19937_64& rng) {
    while (r - l > 1) {
        int m = r - l;
        if (cut && (t == l || t == r)) return;
        if (cut && m <= 6) {
            U64 state = 0;
            for (int i = 0; i < m; ++i)
                for (int j = 0; j < m; ++j)
                    if (rel.below[v[l + i]] & (U64(1) << v[l + j]))
                        state |= U64(1) << (i * m + j);
            unsigned top = SmallCut::run(state, m, t - l, [&](int i, int j) {
                return rel.better(v[l + i], v[l + j]);
            });
            int tmp[6], a = 0, b = m - 1;
            for (int i = 0; i < m; ++i) {
                if ((top >> i) & 1) tmp[a++] = v[l + i];
                else tmp[b--] = v[l + i];
            }
            copy(tmp, tmp + m, v.begin() + l);
            return;
        }

        bool first = cut ? t == l + 1 : t == l;
        bool last = cut ? t == r - 1 : t == r - 1;
        if (first || last) {
            // Only undominated candidates can be the requested extreme.
            U64 mask = 0, dominated = 0;
            for (int i = l; i < r; ++i) {
                mask |= U64(1) << v[i];
                dominated |= rel.below[v[i]];
            }
            int best = -1;
            for (int i = l; i < r; ++i) {
                int x = v[i];
                if (first ? ((dominated >> x) & 1) : (rel.below[x] & mask))
                    continue;
                if (best == -1 ||
                    (first ? rel.better(x, v[best]) : rel.better(v[best], x)))
                    best = i;
            }
            swap(v[best], v[first ? l : r - 1]);
            return;
        }

        int s = m >= 49 ? 7 : m >= 24 ? 5 : m >= 9 ? 3 : 1;
        for (int i = 0; i < s; ++i) {
            int j = l + i + int(rng() % (m - i));
            swap(v[l + i], v[j]);
        }
        double q = (t - l + (cut ? 0.5 : 1.0)) / (m + 1.0);
        int p = clamp(int((s + 1) * q + 0.5) - 1, 0, s - 1);
        if (s > 1) selectRange(v, l, l + s, l + p, false, rel, rng);
        int pivot = v[l + p];
        int tmp[64], a = 0, b = m - 1;
        for (int i = 0; i < p; ++i) tmp[a++] = v[l + i];
        for (int i = p + 1; i < s; ++i) tmp[b--] = v[l + i];
        for (int i = l + s; i < r; ++i) {
            if (rel.better(v[i], pivot)) tmp[a++] = v[i];
            else tmp[b--] = v[i];
        }
        tmp[a] = pivot;
        copy(tmp, tmp + m, v.begin() + l);
        int at = l + a;
        if (t == at || (cut && t == at + 1)) return;
        if (t < at) r = at;
        else l = at + 1;
    }
}

struct Tournament {
    Oracle& oracle;
    const vector<int>& labels;
    vector<int> child, next, parent, size;

    Tournament(Oracle& o, const vector<int>& ids)
        : oracle(o), labels(ids), child(ids.size() + 1), next(ids.size() + 1),
          parent(ids.size() + 1), size(ids.size() + 1, 1) {}

    int join(int a, int b) {
        if (!oracle.better(a, b)) swap(a, b);
        parent[b] = a;
        next[b] = child[a];
        child[a] = b;
        size[a] += size[b];
        return a;
    }

    int build(int l, int n) {
        if (n == 1) return labels[l];
        int p = int(bit_floor(unsigned(n)));
        int left = min(p, n - p / 2);
        int a = build(l, left);
        int b = build(l + left, n - left);
        return join(a, b);
    }

    vector<int> groups(int b) {
        int n = (int)labels.size();
        vector<int> roots;
        roots.reserve(b);
        for (int i = 0, l = 0; i < b; ++i) {
            int s = n / b + (i < n % b);
            roots.push_back(build(l, s));
            l += s;
        }
        return roots;
    }

    int combine(const vector<int>& roots) {
        if (roots.size() == 1) return roots[0];
        using Pair = pair<int, int>;
        priority_queue<Pair, vector<Pair>, greater<Pair>> pq;
        for (int x : roots) pq.emplace(size[x], x);
        while (pq.size() > 1) {
            int a = pq.top().second;
            pq.pop();
            int b = pq.top().second;
            pq.pop();
            int x = join(a, b);
            pq.emplace(size[x], x);
        }
        return pq.top().second;
    }

    int afterRemoving(int x) {
        vector<int> roots;
        for (int y = child[x]; y; y = next[y]) roots.push_back(y);
        return combine(roots);
    }
};

static vector<int> tournamentSelect(const vector<int>& ids, int k, Oracle& o) {
    Tournament t(o, ids);
    int x = t.build(0, (int)ids.size());
    vector<int> ans;
    for (int i = 0; i < k; ++i) {
        ans.push_back(x);
        if (i + 1 < k) x = t.afterRemoving(x);
    }
    return ans;
}

static vector<int> sieveSelect(const vector<int>& ids, int k, int b,
                               Oracle& o, mt19937_64& rng) {
    Tournament t(o, ids);
    Relations roots(o, t.groups(b));
    vector<int> order(b);
    iota(order.begin(), order.end(), 0);
    selectRange(order, 0, b, k, true, roots, rng);

    vector<int> initial;
    for (int i = 0; i < k; ++i) initial.push_back(roots.label[order[i]]);
    Relations keep(o, move(initial));
    for (int i = 0; i < k; ++i)
        for (int j = 0; j < k; ++j)
            if ((roots.below[order[i]] >> order[j]) & 1)
                keep.below[i] |= U64(1) << j;

    vector<int> slot(ids.size() + 1, -1);
    priority_queue<pair<int, int>> frontier;
    auto enqueue = [&](int x) {
        for (int y = t.child[x]; y; y = t.next[y])
            frontier.emplace(t.size[y], y);
    };

    int p = int(bit_ceil(unsigned(k)));
    vector<int> tree(2 * p, -1);
    auto weaker = [&](int a, int b) {
        if (a < 0 || b < 0) return max(a, b);
        return keep.better(a, b) ? b : a;
    };
    for (int i = 0; i < k; ++i) {
        int x = keep.label[i];
        slot[x] = i;
        tree[p + i] = i;
        enqueue(x);
    }
    for (int i = p - 1; i > 0; --i)
        tree[i] = weaker(tree[i * 2], tree[i * 2 + 1]);

    while (!frontier.empty()) {
        int x = frontier.top().second;
        frontier.pop();
        int ps = slot[t.parent[x]];
        int ws = tree[1];
        if (ps < 0 || ps == ws) continue;
        int w = keep.label[ws];
        if (!o.better(x, w)) continue;

        slot[w] = -1;
        keep.forget(ws);
        keep.label[ws] = x;
        slot[x] = ws;
        keep.link(ps, ws);
        for (int i = (p + ws) / 2; i > 0; i /= 2)
            tree[i] = weaker(tree[2 * i], tree[2 * i + 1]);
        enqueue(x);
    }
    return keep.label;
}

static vector<int> forestSelect(const vector<int>& ids, int k, int b, Oracle& o) {
    Tournament t(o, ids);
    vector<int> live = t.groups(b), ans;
    for (size_t i = 0; i < live.size(); ++i)
        for (int x = t.child[live[i]]; x; x = t.next[x]) live.push_back(x);

    int n = (int)ids.size();
    vector<int> present(n + 1), above(n + 1), depth(n + 1);
    vector<double> mean1(n + 1, 1.0), mean2(n + 1, 1.0);
    int epoch = 0;
    for (;;) {
        int m = (int)live.size();
        if (k == 0) return ans;
        if (k == m) {
            ans.insert(ans.end(), live.begin(), live.end());
            return ans;
        }

        ++epoch;
        for (int x : live) {
            present[x] = epoch;
            t.size[x] = 1;
        }
        for (int i = m - 1; i >= 0; --i) {
            int x = live[i], p = t.parent[x];
            if (present[p] == epoch) t.size[p] += t.size[x];
            else t.parent[x] = 0;
        }

        int len = 0;
        int complement = m - k;
        for (int x : live) {
            int p = t.parent[x];
            depth[x] = p ? depth[p] + 1 : 0;
            if (t.size[x] > complement) {
                ans.push_back(x);
                --k;
            } else if (depth[x] <= m - complement - 1) {
                live[len++] = x;
            }
        }
        if (len != m) {
            live.resize(len);
            continue;
        }

        if (m <= 6) {
            U64 state = 0;
            for (int j = 0; j < m; ++j) {
                for (int p = t.parent[live[j]]; p; p = t.parent[p]) {
                    int i = int(find(live.begin(), live.end(), p) - live.begin());
                    state |= U64(1) << (i * m + j);
                }
            }
            unsigned pick = SmallCut::run(state, m, k, [&](int i, int j) {
                return o.better(live[i], live[j]);
            });
            for (int i = 0; i < m; ++i)
                if ((pick >> i) & 1) ans.push_back(live[i]);
            return ans;
        }

        if (k == 1 || k == m - 1) {
            int extreme = 0;
            for (int x : live) {
                if (k == 1 ? t.parent[x] != 0 : t.size[x] != 1) continue;
                if (!extreme || (k == 1 ? o.better(x, extreme) : o.better(extreme, x)))
                    extreme = x;
            }
            if (k == 1) ans.push_back(extreme);
            else for (int x : live) if (x != extreme) ans.push_back(x);
            return ans;
        }

        if (k == 2) {
            vector<int> roots;
            for (int x : live) t.child[x] = 0;
            for (int x : live) {
                int p = t.parent[x];
                if (p == 0) roots.push_back(x);
                else {
                    t.next[x] = t.child[p];
                    t.child[p] = x;
                }
            }
            int x = t.combine(roots);
            ans.push_back(x);
            ans.push_back(t.afterRemoving(x));
            return ans;
        }

        int pivot = 0;
        double best = numeric_limits<double>::infinity();
        for (int x : live) {
            int p = t.parent[x];
            double s = t.size[x];
            mean1[x] = mean1[p] * (s / (s + 1));
            mean2[x] = mean2[p] * (s / (s + 2));
            double mu = (m + 1.0) * (1.0 - mean1[x]);
            double bias = abs(mu - (k + 0.5));
            if (bias >= best) continue;
            double var = (m + 1.0) * (m + 2.0) * (1.0 - 2.0 * mean1[x] + mean2[x])
                         - mu * (mu + 1.0);
            double score = bias + 0.35 * sqrt(max(0.0, var));
            if (score < best) {
                best = score;
                pivot = x;
            }
        }

        int g = 0;
        for (int x = t.parent[pivot]; x; x = t.parent[x]) {
            above[x] = epoch;
            ++g;
        }
        for (int x : live) {
            if (x == pivot || above[x] == epoch) continue;
            int p = t.parent[x];
            if ((p == 0 || above[p] == epoch) && o.better(x, pivot)) {
                above[x] = epoch;
                ++g;
            }
        }

        len = 0;
        if (g >= k) {
            for (int x : live)
                if (above[x] == epoch) live[len++] = x;
        } else {
            for (int x : live) {
                if (x == pivot || above[x] == epoch) ans.push_back(x);
                else live[len++] = x;
            }
            k -= g + 1;
        }
        live.resize(len);
    }
}

struct Mode {
    int type;
    int groups;
};

static vector<int> solve(const vector<int>& ids, int k, Mode mode,
                         Oracle& o, mt19937_64& rng) {
    if (mode.type == 0) return tournamentSelect(ids, k, o);
    if (mode.type == 1) return sieveSelect(ids, k, mode.groups, o, rng);
    if (mode.type == 2) return forestSelect(ids, k, mode.groups, o);
    Relations rel(o, ids);
    vector<int> v(ids.size());
    iota(v.begin(), v.end(), 0);
    selectRange(v, 0, (int)v.size(), k, true, rel, rng);
    v.resize(k);
    for (int& x : v) x = rel.label[x];
    return v;
}

// Compare strategies on generated rankings only. No calibration calls use the judge.
static Mode chooseMode(int n, int k, mt19937_64& rng) {
    if (k == 1) return {0, 1};
    if (n <= 6) return {3, 0};
    if (k == 2 && n > 32) return {0, 1};

    vector<Mode> modes{{0, 1}, {2, 1}};
    int forestGroups = n <= 128 ? (n + 1) / 2 : max(1, k / 2);
    if (forestGroups > 1) modes.push_back({2, forestGroups});
    int previous = -1;
    for (int b : {k, (3 * k + 1) / 2, 2 * k}) {
        b = min(n, min(64, b));
        if (b != previous) modes.push_back({1, b});
        previous = b;
    }
    if (n <= 64) modes.push_back({3, 0});

    int trials = n <= 128 ? 36 : (n <= 2048 ? 22 : 14);
    vector<int> ids(n), ranks(n + 1);
    iota(ids.begin(), ids.end(), 1);
    iota(ranks.begin(), ranks.end(), 0);
    vector<int> sums(modes.size());
    for (int t = 0; t < trials; ++t) {
        shuffle(ranks.begin() + 1, ranks.end(), rng);
        U64 seed = rng();
        for (int i = 0; i < (int)modes.size(); ++i) {
            mt19937_64 local(seed);
            Oracle fake{&ranks, false};
            solve(ids, k, modes[i], fake, local);
            sums[i] += fake.queries;
        }
    }
    return modes[min_element(sums.begin(), sums.end()) - sums.begin()];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k) || n < 1 || k < 1 || k > n) return 0;

    vector<int> ans;
    if (k == n) {
        ans.resize(n);
        iota(ans.begin(), ans.end(), 1);
    } else {
        bool reverseOrder = k > n - k;
        int need = reverseOrder ? n - k : k;
        mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
        Mode mode = chooseMode(n, need, rng);
        vector<int> ids(n);
        iota(ids.begin(), ids.end(), 1);
        shuffle(ids.begin(), ids.end(), rng);
        Oracle real{nullptr, reverseOrder};
        ans = solve(ids, need, mode, real, rng);
        if (reverseOrder) {
            vector<char> excluded(n + 1);
            for (int x : ans) excluded[x] = true;
            ans.clear();
            for (int x = 1; x <= n; ++x)
                if (!excluded[x]) ans.push_back(x);
        }
    }

    cout << '!';
    for (int x : ans) cout << ' ' << x;
    cout << '\n' << flush;
    return 0;
}
