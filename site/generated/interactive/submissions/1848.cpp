#include <bits/stdc++.h>
using namespace std;

constexpr int MAXN = 600;
using Mask = bitset<MAXN>;

struct Link {
    int child, side, degree;
};

struct Node {
    array<Mask, 2> side;
    vector<Link> children;
    int edges = 0;
    int size = 0;
};

struct Bucket {
    int component, side, rank, size;
    bool second;
};

struct BadGuess {};

int n, m, asked = 0;
Mask all;
array<Mask, MAXN> knownEdge;
array<Node, MAXN> tree;
unordered_map<Mask, int> memo;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

int query(const Mask& s) {
    int k = (int)s.count();
    if (k <= 1) return 0;
    if (auto it = memo.find(s); it != memo.end()) return it->second;

    cout << "? " << k << '\n';
    bool first = true;
    int a = -1, b = -1;
    for (int i = 0; i < n; ++i) if (s[i]) {
        if (!first) cout << ' ';
        cout << i + 1;
        first = false;
        if (k == 2) {
            a = b;
            b = i;
        }
    }
    cout << '\n' << flush;
    int result;
    if (!(cin >> result) || result < 0) exit(0);
    ++asked;
    memo.emplace(s, result);
    if (k == 2 && result == 1) {
        knownEdge[a].set(b);
        knownEdge[b].set(a);
    }
    return result;
}

int pairQuery(int a, int b) {
    Mask s;
    s.set(a).set(b);
    return query(s);
}

void yes(const Mask& s) {
    cout << "Y " << s.count() << '\n';
    bool first = true;
    for (int i = 0; i < n; ++i) if (s[i]) {
        if (!first) cout << ' ';
        first = false;
        cout << i + 1;
    }
    cout << '\n' << flush;
}

[[noreturn]] void no(const vector<int>& cycle) {
    cout << "N " << cycle.size() << '\n';
    for (int i = 0; i < (int)cycle.size(); ++i) {
        if (i) cout << ' ';
        cout << cycle[i] + 1;
    }
    cout << '\n' << flush;
    exit(0);
}

int withVertex(Mask s, int v, int internal = 0) {
    s.set(v);
    return query(s) - internal;
}

// There are at least lower neighbors of v in the independent set s.
int witness(int v, Mask s, int lower) {
    Mask known = s & knownEdge[v];
    if (known.any()) {
        for (int i = 0; i < n; ++i) if (known[i]) return i;
    }
    vector<int> candidates;
    for (int i = 0; i < n; ++i) if (s[i]) candidates.push_back(i);
    // Removing lower-1 arbitrary candidates still leaves a neighbor.
    int l = 0, r = (int)candidates.size() - lower + 1;
    while (r - l > 1) {
        int mid = (l + r) / 2;
        Mask half;
        for (int i = l; i < mid; ++i) half.set(candidates[i]);
        int d = withVertex(half, v);
        if (d == 0) l = mid;
        else r = mid - d + 1;
    }
    return candidates[l];
}

void treePath(int root, int a, int b, vector<int>& path) {
    if (a == b) {
        path.push_back(a);
        return;
    }
    const Link *ca = nullptr, *cb = nullptr;
    for (const Link& link : tree[root].children) {
        const Node& child = tree[link.child];
        if (child.side[0][a] || child.side[1][a]) ca = &link;
        if (child.side[0][b] || child.side[1][b]) cb = &link;
    }
    if (ca && ca == cb) {
        treePath(ca->child, a, b, path);
        return;
    }
    if (ca) {
        int w = witness(root, tree[ca->child].side[ca->side], ca->degree);
        treePath(ca->child, a, w, path);
    }
    path.push_back(root);
    if (cb) {
        int w = witness(root, tree[cb->child].side[cb->side], cb->degree);
        treePath(cb->child, w, b, path);
    }
}

[[noreturn]] void oddCycle(int v, int component, int d0, int d1) {
    int a = witness(v, tree[component].side[0], d0);
    int b = witness(v, tree[component].side[1], d1);
    vector<int> cycle{v};
    treePath(component, a, b, cycle);
    no(cycle);
}

void walsh(vector<int>& a) {
    for (int len = 1; len < (int)a.size(); len *= 2) {
        for (int i = 0; i < (int)a.size(); i += 2 * len) {
            for (int j = i; j < i + len; ++j) {
                int x = a[j], y = a[j + len];
                a[j] = x + y;
                a[j + len] = x - y;
            }
        }
    }
}

// A verified shortcut for complete bipartite graphs, including stars.
bool tryComplete(const vector<int>& order) {
    int small = 1;
    while (small <= n / 2 && small * (n - small) < m) ++small;
    if (small > n / 2 || small * (n - small) != m) return false;

    int pivot = order[0];
    Mask rest = all;
    rest.reset(pivot);
    int degree = m - query(rest);
    if (degree != small && degree != n - small) return false;

    bool pivotMinor = degree > n - degree;
    Mask minority;
    vector<int> majority;
    if (pivotMinor) minority.set(pivot);
    else majority.push_back(pivot);
    vector<int> unknown(order.begin() + 1, order.end());
    int remainingMinority = small - (int)pivotMinor;
    int budget = asked + max(24, (3 * n) / 4 + 12);

    auto classify = [&](int v, bool isMinor) {
        if (isMinor) minority.set(v);
        else majority.push_back(v);
    };

    auto groupCount = [&](const Mask& s, int k, int lo, int hi) -> int {
        if (lo == hi) return lo;
        if (asked >= budget) throw BadGuess{};
        int pad = max(0, 2 * hi - k);
        if (pad > (int)majority.size()) pad = 0;
        Mask t = s;
        for (int j = 0; j < pad; ++j) t.set(majority[j]);
        int z = k + pad;
        int e = query(t);
        int left = 0, right = z / 2;
        while (left < right) {
            int mid = (left + right) / 2;
            if (mid * (z - mid) < e) left = mid + 1;
            else right = mid;
        }
        if (left * (z - left) != e) throw BadGuess{};
        int other = z - left;
        bool ok1 = lo <= left && left <= hi;
        bool ok2 = lo <= other && other <= hi;
        if (!ok1 && !ok2) throw BadGuess{};
        if (!ok2 || (ok1 && left == other)) return left;
        if (!ok1) return other;

        int neighbors = withVertex(s, pivot, query(s));
        int answer = pivotMinor ? k - neighbors : neighbors;
        if (answer != left && answer != other) throw BadGuess{};
        return answer;
    };

    try {
        int length = (int)unknown.size();
        if (remainingMinority <= max(2, length / 14)) {
            auto split = [&](auto&& self, int l, int r, int cnt) -> void {
                if (cnt == 0 || cnt == r - l) {
                    for (int i = l; i < r; ++i) classify(unknown[i], cnt != 0);
                    return;
                }
                int mid = (l + r) / 2;
                Mask s;
                for (int i = l; i < mid; ++i) s.set(unknown[i]);
                int a = groupCount(s, mid - l, max(0, cnt - (r - mid)),
                                   min(cnt, mid - l));
                self(self, l, mid, a);
                self(self, mid, r, cnt - a);
            };
            split(split, 0, length, remainingMinority);
        } else {
            vector<int> dp(length + 1), block(length + 1), bits(length + 1);
            for (int i = 1; i <= length; ++i) {
                dp[i] = dp[i - 1] + 1;
                block[i] = 1;
                for (int b = 3; b <= 8; ++b) {
                    int k = min(i, 1 + b * (1 << (b - 1)));
                    int cost = (1 << b) + 1 + dp[i - k];
                    if (cost < dp[i]) {
                        dp[i] = cost;
                        block[i] = k;
                        bits[i] = b;
                    }
                }
            }
            vector<pair<int, int>> plan;
            for (int i = length; i > 0; i -= block[i])
                plan.emplace_back(block[i], bits[i]);
            sort(plan.begin(), plan.end());

            int start = 0;
            for (auto [k, b] : plan) {
                int remain = length - start;
                if (remainingMinority == 0 || remainingMinority == remain) {
                    for (int j = 0; j < k; ++j)
                        classify(unknown[start + j], remainingMinority != 0);
                    if (remainingMinority != 0) remainingMinority -= k;
                    start += k;
                    continue;
                }
                if (k == 1) {
                    Mask s;
                    s.set(unknown[start]);
                    int z = groupCount(s, 1, 0, 1);
                    classify(unknown[start++], z);
                    remainingMinority -= z;
                    continue;
                }

                int rows = 1 << b;
                vector<Mask> sets(rows);
                vector<vector<int>> transform;
                vector<int> begin(rows + 1);
                // A column indexed by mask has no Fourier coefficients
                // outside submasks, and its top coefficient encodes a bit.
                for (int mask = 0; mask < rows; ++mask) {
                    begin[mask] = (int)transform.size();
                    int p = popcount((unsigned)mask);
                    for (int j = 0; j < max(1, p) && (int)transform.size() < k; ++j) {
                        int fixed = mask;
                        for (int t = 0; t < j + 1 && fixed; ++t)
                            fixed &= fixed - 1;
                        int pattern = (int)(rng() & (unsigned)fixed);
                        int vertex = unknown[start + transform.size()];
                        vector<int> col(rows);
                        for (int x = 0; x < rows; ++x) {
                            if ((x & fixed) == pattern &&
                                !(popcount((unsigned)(x & mask)) & 1)) {
                                col[x] = 1;
                                sets[x].set(vertex);
                            }
                        }
                        walsh(col);
                        transform.push_back(move(col));
                    }
                }
                begin[rows] = k;
                vector<int> f(rows);
                for (int x = 0; x < rows; ++x) {
                    int sz = (int)sets[x].count();
                    f[x] = groupCount(sets[x], sz,
                                      max(0, remainingMinority - (remain - sz)),
                                      min(sz, remainingMinority));
                }
                walsh(f);
                vector<int> label(k);
                int used = 0;
                for (int mask = rows - 1; mask >= 0; --mask) {
                    int unit = 1 << (b - popcount((unsigned)mask));
                    int cnt = begin[mask + 1] - begin[mask];
                    int value = f[mask] / unit;
                    if (f[mask] < 0 || f[mask] % unit || value >= (1 << cnt))
                        throw BadGuess{};
                    for (int j = 0; j < cnt; ++j) if ((value >> j) & 1) {
                        int col = begin[mask] + j;
                        label[col] = 1;
                        ++used;
                        for (int x = 0; x < rows; ++x) f[x] -= transform[col][x];
                    }
                }
                if (used > remainingMinority ||
                    remainingMinority - used > remain - k) throw BadGuess{};
                for (int j = 0; j < k; ++j)
                    classify(unknown[start + j], label[j]);
                remainingMinority -= used;
                start += k;
            }
        }

        if ((int)minority.count() != small) return false;
        if (query(all ^ minority) != 0 || query(minority) != 0) return false;
        yes(minority);
        return true;
    } catch (const BadGuess&) {
        return false;
    }
}

// One query per new vertex on the fast path. The opposite side is
// checked in batches; neither tentative colors nor checks are assumed.
bool tryGreedy(Mask a, Mask b, const vector<int>& order, int start) {
    if (a.count() < b.count()) swap(a, b);
    int batch = 8;
    Mask lastChecked = b;
    for (int l = start; l < n; ) {
        int r = min(n, l + batch);
        for (int i = l; i < r; ++i) {
            int v = order[i];
            a.set(v);
            if (query(a) != 0) {
                a.reset(v);
                b.set(v);
            }
        }
        if (b != lastChecked && query(b) != 0) return false;
        lastChecked = b;
        l = r;
        batch *= 2;
    }
    yes(a);
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n) || n < 1) return 0;
    if (n <= 2) {
        Mask s;
        s.set(0);
        yes(s);
        return 0;
    }
    if (n == 3) {
        for (int i = 0; i < 3; ++i) {
            int j = (i + 1) % 3;
            if (!pairQuery(i, j)) {
                Mask s;
                s.set(i).set(j);
                yes(s);
                return 0;
            }
        }
        no({0, 1, 2});
    }

    for (int i = 0; i < n; ++i) all.set(i);
    memo.reserve(15000);
    m = query(all);

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    shuffle(order.begin(), order.end(), rng);
    if (tryComplete(order)) return 0;

    vector<int> active;
    Mask prefix;
    int prefixEdges = 0;
    bool triedGreedy = false;

    for (int pos = 0; pos < n; ++pos) {
        int v = order[pos];
        int nextEdges;
        if (pos == n - 1) nextEdges = m;
        else nextEdges = prefixEdges + withVertex(prefix, v, prefixEdges);
        int degree = nextEdges - prefixEdges;

        Node& root = tree[v];
        root.side[0].set(v);
        root.size = 1;
        root.edges = degree;

        if (degree > 0) {
            // Contract the already processed components. Connectivity
            // bounds the number of surplus edges in the contracted graph.
            int slack = m - prefixEdges - (n - pos + (int)active.size() - 1);

            sort(active.begin(), active.end(), [&](int a, int b) {
                return tree[a].size > tree[b].size;
            });
            vector<Bucket> buckets;
            vector<int> sizes{0}, inside{0};
            for (int rank = 0; rank < (int)active.size(); ++rank) {
                int c = active[rank];
                for (int s = 0; s < 2; ++s) {
                    int sz = (int)tree[c].side[s].count();
                    if (!sz) continue;
                    buckets.push_back({c, s, rank, sz, s == 1});
                    sizes.push_back(sizes.back() + sz);
                    inside.push_back(inside.back() + (s ? tree[c].edges : 0));
                }
            }
            array<array<int, 2>, MAXN> counts{};
            auto record = [&](int i, int d) {
                const auto& b = buckets[i];
                auto& a = counts[b.component];
                a[b.side] = d;
                if (a[0] && a[1]) oddCycle(v, b.component, a[0], a[1]);
            };
            auto capacity = [&](int l, int r) {
                int components = buckets[r - 1].rank - buckets[l].rank + 1;
                return min(sizes[r] - sizes[l], components + slack);
            };
            auto askRange = [&](int l, int r) {
                Mask s;
                for (int i = l; i < r; ++i)
                    s |= tree[buckets[i].component].side[buckets[i].side];
                int e = inside[r] - inside[l];
                if (buckets[l].second) e -= tree[buckets[l].component].edges;
                return withVertex(s, v, e);
            };
            auto locate = [&](auto&& self, int l, int r, int d) -> void {
                if (!d) return;
                if (r - l == 1) {
                    record(l, d);
                    return;
                }
                if (d == sizes[r] - sizes[l]) {
                    for (int i = l; i < r; ++i) record(i, buckets[i].size);
                    return;
                }
                if (slack == 0 && d == r - l &&
                    buckets[r - 1].rank - buckets[l].rank + 1 == r - l) {
                    for (int i = l; i < r; ++i) record(i, 1);
                    return;
                }
                if (r - l == 2 && buckets[l].component == buckets[l + 1].component &&
                    d > max(buckets[l].size, buckets[l + 1].size)) {
                    oddCycle(v, buckets[l].component,
                             d - buckets[l + 1].size, d - buckets[l].size);
                }

                int target = (sizes[l] + sizes[r] + 1) / 2;
                int mid = (int)(lower_bound(sizes.begin() + l + 1,
                                             sizes.begin() + r, target) - sizes.begin());
                mid = min(mid, r - 1);
                if (mid > l + 1 &&
                    abs(2 * sizes[mid - 1] - sizes[l] - sizes[r]) <
                    abs(2 * sizes[mid] - sizes[l] - sizes[r])) --mid;
                int lo = max(0, d - capacity(mid, r));
                int hi = min(d, capacity(l, mid));
                int dl;
                if (lo == hi) {
                    dl = lo;
                } else if (sizes[mid] - sizes[l] <= sizes[r] - sizes[mid]) {
                    dl = askRange(l, mid);
                } else {
                    dl = d - askRange(mid, r);
                }
                self(self, l, mid, dl);
                self(self, mid, r, d - dl);
            };
            locate(locate, 0, (int)buckets.size(), degree);

            int keep = 0;
            for (int c : active) {
                if (counts[c][0] == 0 && counts[c][1] == 0) {
                    active[keep++] = c;
                    continue;
                }
                int side = counts[c][1] > 0;
                root.side[1] |= tree[c].side[side];
                root.side[0] |= tree[c].side[1 - side];
                root.children.push_back({c, side, counts[c][side]});
                root.edges += tree[c].edges;
                root.size += tree[c].size;
            }
            active.resize(keep);
        }
        active.push_back(v);
        prefix.set(v);
        prefixEdges = nextEdges;

        if (!triedGreedy && pos + 1 < n && pos >= 5) {
            int largest = 0;
            for (int c : active) largest = max(largest, tree[c].size);
            int remaining = n - pos - 1;
            bool dense = 2LL * m * largest >= 6LL * n * (n - 1);
            bool close = remaining >= 4 && remaining <= 18 &&
                         largest == pos + 1 && 2 * m >= 3 * n;
            if ((dense && 5 * largest >= 4 * (pos + 1) && remaining >= 4) || close) {
                triedGreedy = true;
                Mask a, b;
                for (int c : active) {
                    a |= tree[c].side[0];
                    b |= tree[c].side[1];
                }
                memo.emplace(a, 0);
                memo.emplace(b, 0);
                if (tryGreedy(a, b, order, pos + 1)) return 0;
            }
        }
    }

    Mask answer;
    for (int c : active) answer |= tree[c].side[0];
    yes(answer);
    return 0;
}
