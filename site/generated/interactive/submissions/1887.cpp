#include <bits/stdc++.h>
using namespace std;

constexpr int MOD = 1000000007;

int addmod(int a, int b) {
    int c = a + b;
    return c >= MOD ? c - MOD : c;
}

int submod(int a, int b) {
    int c = a - b;
    return c < 0 ? c + MOD : c;
}

int mulmod(int a, int b) {
    return int(1LL * a * b % MOD);
}

int modint(long long x) {
    x %= MOD;
    return int(x < 0 ? x + MOD : x);
}

int inverse(int x) {
    int ans = 1;
    for (int e = MOD - 2; e; e >>= 1, x = mulmod(x, x))
        if (e & 1) ans = mulmod(ans, x);
    return ans;
}

struct Point {
    int x, y;
};

struct Query {
    int p, q;
    int key() const { return p * 1001 + q; }
};

Query fraction(int p, int q) {
    int g = gcd(p, q);
    return {p / g, q / g};
}

struct Term {
    Query query;
    int weight;
};

unordered_map<int, int> answers;

int readResidue() {
    string s;
    if (!(cin >> s) || s.empty() || s[0] == '-') exit(0);
    int a = 0;
    for (char c : s) {
        if (c < '0' || c > '9') exit(0);
        a = (10LL * a + c - '0') % MOD;
    }
    return a;
}

int ask(Query q) {
    auto it = answers.find(q.key());
    if (it != answers.end()) return it->second;
    cout << "? " << q.p << ' ' << q.q << endl;
    int r = readResidue();
    int s = readResidue();
    if (s == 0) exit(0);
    int value = mulmod(r, inverse(s));
    answers.emplace(q.key(), value);
    return value;
}

void finish(int twiceArea) {
    if (twiceArea & 1) cout << "! " << twiceArea << " 2" << endl;
    else cout << "! " << twiceArea / 2 << " 1" << endl;
}

int cross(Point a, Point b) {
    return a.x * b.y - a.y * b.x;
}

int orient(Point a, Point b, Point c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

// The points are sorted lexicographically. Keep every boundary point.
vector<int> boundary(const vector<Point>& p) {
    int n = int(p.size());
    vector<int> lower, upper;
    for (int i = 0; i < n; ++i) {
        while (lower.size() >= 2 &&
               orient(p[lower[lower.size() - 2]], p[lower.back()], p[i]) < 0)
            lower.pop_back();
        lower.push_back(i);
    }
    for (int i = n - 1; i >= 0; --i) {
        while (upper.size() >= 2 &&
               orient(p[upper[upper.size() - 2]], p[upper.back()], p[i]) < 0)
            upper.pop_back();
        upper.push_back(i);
    }
    lower.pop_back();
    upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());
    return lower;
}

vector<Term> makeQuadrature(const vector<int>& x, const vector<int>& count) {
    int m = int(x.size());
    vector<Term> result;
    auto pairTerm = [&](int i, int a, int b) {
        return Term{fraction(x[i] * a + x[i + 1] * b, a + b), a + b};
    };

    // At an internal x with several vertices, the two one-sided
    // lengths are kept separate. Elsewhere the adjacent weights merge.
    for (int l = 0; l + 1 < m;) {
        int r = l + 1;
        while (r + 1 < m && count[r] == 1) ++r;
        bool leftZero = l == 0 && count[l] == 1;
        bool rightZero = r == m - 1 && count[r] == 1;
        int start = l + leftZero, stop = r - rightZero;
        int k = max(0, stop - start + 1);

        vector<int> w(k);
        for (int j = 0; j < k; ++j) {
            int i = start + j;
            w[j] = (i > l ? x[i] - x[i - 1] : 0)
                 + (i < r ? x[i + 1] - x[i] : 0);
        }
        vector<int> dp(k + 1, 1000000), nxt(k);
        vector<vector<Term>> use(k);
        dp[k] = 0;
        for (int j = k - 1; j >= 0; --j) {
            int i = start + j;
            auto relax = [&](int len, vector<Term> terms) {
                int v = int(terms.size()) + dp[j + len];
                if (v < dp[j]) {
                    dp[j] = v;
                    nxt[j] = j + len;
                    use[j] = move(terms);
                }
            };

            if ((l < i && i < r) || i == 0 || i == m - 1) {
                relax(1, {{{x[i], 1}, w[j]}});
            } else if (leftZero && i == l + 1) {
                relax(1, {{fraction(x[l] + x[i], 2), 2 * w[j]}});
            } else if (rightZero && i == r - 1) {
                relax(1, {{fraction(x[i] + x[r], 2), 2 * w[j]}});
            }

            if (j + 1 < k) {
                int a = w[j], b = w[j + 1];
                Term t = pairTerm(i, a, b);
                if (t.query.q <= 1000) {
                    relax(2, {t});
                } else {
                    relax(2, {
                        {fraction(2 * x[i] + x[i + 1], 3), 2 * a - b},
                        {fraction(x[i] + 2 * x[i + 1], 3), 2 * b - a}
                    });
                }
            }

            if (j + 2 < k && dp[j + 3] + 2 < dp[j]) {
                int a = w[j], b = w[j + 1], c = w[j + 2];
                auto split = [&](int s) {
                    Term t1 = pairTerm(i, a, s);
                    Term t2 = pairTerm(i + 1, b - s, c);
                    if (t1.query.q > 1000 || t2.query.q > 1000) return false;
                    relax(3, {t1, t2});
                    return true;
                };
                if (!split(x[i + 1] - x[i])) {
                    for (int s = 1; s < b; ++s)
                        if (split(s)) break;
                }
            }
        }

        if (dp[0] <= r - l) {
            for (int j = 0; j < k; j = nxt[j])
                result.insert(result.end(), use[j].begin(), use[j].end());
        } else {
            for (int i = l; i < r; ++i)
                result.push_back({fraction(x[i] + x[i + 1], 2),
                                  2 * (x[i + 1] - x[i])});
        }
        l = r;
    }
    return result;
}

struct Edge {
    int a, b, sign;
};

struct Candidate {
    int area;
    vector<Edge> delta;
};

struct Enumerator {
    const vector<Point>& pts;
    const vector<int>& hull;
    vector<int> inside;
    vector<Edge> edges;
    vector<Candidate> candidates;
    int hullArea, fullMask = 0;
    int attempts = 0;
    bool aborted = false;

    Enumerator(const vector<Point>& pts, const vector<int>& hull, int area)
        : pts(pts), hull(hull), hullArea(area) {
        vector<bool> on(pts.size());
        for (int i : hull) on[i] = true;
        for (int i = 0; i < int(pts.size()); ++i)
            if (!on[i]) inside.push_back(i);
    }

    bool compatible(int a, int b, int c, int d) const {
        if (a == c || a == d || b == c || b == d) {
            if (b == c || b == d) swap(a, b);
            if (a == d) swap(c, d);
            Point p = pts[a], q = pts[b], r = pts[d];
            return orient(p, q, r) != 0 ||
                (q.x - p.x) * (r.x - p.x) + (q.y - p.y) * (r.y - p.y) < 0;
        }
        Point p = pts[a], q = pts[b], r = pts[c], s = pts[d];
        if (max(min(p.x, q.x), min(r.x, s.x)) >
                min(max(p.x, q.x), max(r.x, s.x)) ||
            max(min(p.y, q.y), min(r.y, s.y)) >
                min(max(p.y, q.y), max(r.y, s.y))) return true;
        int u = orient(p, q, r), v = orient(p, q, s);
        if ((u > 0 && v > 0) || (u < 0 && v < 0)) return true;
        u = orient(r, s, p);
        v = orient(r, s, q);
        return (u > 0 && v > 0) || (u < 0 && v < 0);
    }

    bool canAdd(int a, int b) {
        if (++attempts > 1400000) {
            aborted = true;
            return false;
        }
        for (const Edge& e : edges)
            if (e.sign > 0 && !compatible(a, b, e.a, e.b)) return false;
        return true;
    }

    // Hull vertices must be visited in boundary order. Enumerate the
    // disjoint interior chains inserted between successive hull vertices.
    void dfs(int g, int tail, int mask, int area) {
        if (aborted) return;
        int b = int(hull.size());
        if (tail < 0 && mask == fullMask) {
            if (candidates.size() == 24000) aborted = true;
            else candidates.push_back({area, edges});
            return;
        }

        auto extend = [&](int a, int v, int gap, int newTail, int newMask, int base) {
            if (!canAdd(a, v)) return;
            edges.push_back({a, v, 1});
            dfs(gap, newTail, newMask, base + cross(pts[a], pts[v]));
            edges.pop_back();
        };

        if (tail >= 0) {
            extend(tail, hull[(g + 1) % b], g + 1, -1, mask, area);
            for (int bits = fullMask ^ mask; bits && !aborted; bits &= bits - 1) {
                int j = countr_zero(unsigned(bits));
                extend(tail, inside[j], g, inside[j], mask | (1 << j), area);
            }
        } else {
            for (int i = g; i < b && !aborted; ++i) {
                int a = hull[i], z = hull[(i + 1) % b];
                edges.push_back({a, z, -1});
                for (int bits = fullMask ^ mask; bits && !aborted; bits &= bits - 1) {
                    int j = countr_zero(unsigned(bits));
                    extend(a, inside[j], i, inside[j], mask | (1 << j),
                           area - cross(pts[a], pts[z]));
                }
                edges.pop_back();
            }
        }
    }

    bool run() {
        int k = int(inside.size()), n = int(pts.size());
        if (k > 18) return false;
        long long estimate = 1;
        for (int j = 0; j < k && estimate <= 220000; ++j)
            estimate *= n - 1 - j;
        if (n > 25 && estimate > 220000) return false;
        fullMask = (1 << k) - 1;
        dfs(0, -1, 0, hullArea);
        return !aborted && !candidates.empty();
    }
};

struct Sample {
    Query query;
    int index, mx;
    bool knot;
};

struct SliceTable {
    vector<Sample> samples;
    vector<int> hullValue, data, areas, base;
    int columns = 0;

    const int* row(int t) const {
        return data.data() + size_t(t) * columns;
    }

    SliceTable(const vector<Point>& pts, const vector<int>& xs,
               const vector<int>& hull, const vector<Candidate>& candidates,
               const vector<Term>& plan) {
        columns = int(candidates.size());
        int m = int(xs.size());
        int limit = min(5200, max(int(plan.size()), 6500000 / columns));
        int inv[1001] = {};
        inv[1] = 1;
        for (int i = 2; i <= 1000; ++i)
            inv[i] = MOD - mulmod(MOD / i, inv[MOD % i]);

        unordered_map<int, int> ids;
        auto addSample = [&](Query q) {
            auto it = ids.find(q.key());
            if (it != ids.end()) return it->second;
            int i = int(lower_bound(xs.begin(), xs.end(), q.p,
                           [&](int a, int p) { return a * q.q < p; }) - xs.begin());
            bool knot = (i < m && xs[i] * q.q == q.p);
            int id = int(samples.size());
            samples.push_back({q, knot ? i : i - 1, mulmod(q.p, inv[q.q]), knot});
            ids.emplace(q.key(), id);
            return id;
        };
        for (const Term& t : plan) base.push_back(addSample(t.query));
        sort(base.begin(), base.end());
        base.erase(unique(base.begin(), base.end()), base.end());

        mt19937 rng(245193);
        vector<int> order(m - 1);
        iota(order.begin(), order.end(), 0);
        shuffle(order.begin(), order.end(), rng);
        for (int round = 0; round < 4 && int(samples.size()) < limit; ++round) {
            if (round == 1) {
                for (int i = 0; i < m && int(samples.size()) < limit; ++i)
                    addSample({xs[i], 1});
            }
            for (int i : order) {
                if (int(samples.size()) >= limit) break;
                int q = round == 0 ? 997 : 800 + int(rng() % 201);
                int width = (xs[i + 1] - xs[i]) * q;
                int p = xs[i] * q + 1 + int(rng() % (width - 1));
                addSample(fraction(p, q));
            }
        }

        vector<int> xIndex(1001);
        for (int i = 0; i < m; ++i) xIndex[xs[i]] = i;
        vector<int> a(m), b(m), v(m);
        auto makeProfile = [&](const vector<Edge>& edges) {
            fill(a.begin(), a.end(), 0);
            fill(b.begin(), b.end(), 0);
            fill(v.begin(), v.end(), 0);
            for (const Edge& e : edges) {
                Point p = pts[e.a], q = pts[e.b];
                if (p.x == q.x) {
                    int i = xIndex[p.x];
                    v[i] = addmod(v[i], modint(e.sign * abs(p.y - q.y)));
                } else {
                    int sign = -e.sign;
                    if (p.x > q.x) {
                        swap(p, q);
                        sign = -sign;
                    }
                    int slope = mulmod(modint(sign * (q.y - p.y)), inv[q.x - p.x]);
                    int intercept = submod(modint(sign * p.y), mulmod(slope, p.x));
                    int i = xIndex[p.x], j = xIndex[q.x];
                    a[i] = addmod(a[i], slope);
                    b[i] = addmod(b[i], intercept);
                    a[j] = submod(a[j], slope);
                    b[j] = submod(b[j], intercept);
                }
            }
            for (int i = 1; i < m; ++i) {
                a[i] = addmod(a[i], a[i - 1]);
                b[i] = addmod(b[i], b[i - 1]);
            }
        };
        auto evaluate = [&](const Sample& s) {
            int i = s.index;
            int value = addmod(mulmod(a[i], s.mx), b[i]);
            if (!s.knot) return value;
            // At a knot, the closed slice is the union of the two
            // one-sided slices. Vertical edges belong to just one side.
            if (i) value = addmod(value, addmod(mulmod(a[i - 1], s.mx), b[i - 1]));
            return mulmod(addmod(value, v[i]), inv[2]);
        };

        vector<Edge> hullEdges;
        for (int i = 0; i < int(hull.size()); ++i)
            hullEdges.push_back({hull[i], hull[(i + 1) % hull.size()], 1});
        makeProfile(hullEdges);
        for (const Sample& s : samples) hullValue.push_back(evaluate(s));
        data.resize(size_t(samples.size()) * columns);
        areas.resize(columns);
        for (int c = 0; c < columns; ++c) {
            areas[c] = candidates[c].area;
            makeProfile(candidates[c].delta);
            for (int j = 0; j < int(samples.size()); ++j)
                data[size_t(j) * columns + c] = evaluate(samples[j]);
        }
    }
};

int exactSmall(const SliceTable& table, const vector<int>& alive) {
    int n = int(alive.size()), full = (1 << n) - 1;
    map<vector<int>, int> uniqueTests;
    for (int j = 0; j < int(table.samples.size()); ++j) {
        const int* row = table.row(j);
        vector<int> groups;
        int used = 0;
        for (int i = 0; i < n; ++i) {
            if (used & (1 << i)) continue;
            int g = 1 << i;
            for (int k = i + 1; k < n; ++k)
                if (row[alive[k]] == row[alive[i]]) g |= 1 << k;
            groups.push_back(g);
            used |= g;
        }
        if (groups.size() > 1) uniqueTests.emplace(move(groups), j);
    }

    vector<int> same(n);
    for (int i = 0; i < n; ++i)
        for (int k = 0; k < n; ++k)
            if (table.areas[alive[i]] == table.areas[alive[k]]) same[i] |= 1 << k;
    vector<int> depth(full + 1), sum(full + 1);
    int choice = -1;
    for (int m = 1; m <= full; ++m) {
        if ((m & ~same[countr_zero(unsigned(m))]) == 0) continue;
        pair<int, int> best{1000, 1000000};
        for (const auto& [groups, j] : uniqueTests) {
            int d = 0, s = popcount(unsigned(m));
            bool split = true;
            for (int mask : groups) {
                int g = m & mask;
                if (g == m) {
                    split = false;
                    break;
                }
                d = max(d, depth[g]);
                s += sum[g];
            }
            if (split && pair<int, int>{d + 1, s} < best) {
                best = {d + 1, s};
                if (m == full) choice = j;
            }
        }
        tie(depth[m], sum[m]) = best;
    }
    return choice;
}

int solveCandidates(const SliceTable& table) {
    vector<int> alive(table.columns), base = table.base;
    iota(alive.begin(), alive.end(), 0);
    int remaining = int(base.size());
    int tests = int(table.samples.size());

    struct Bin {
        int stamp = 0, value = 0, count = 0, area = 0;
    };
    unsigned capacity = 1;
    while (capacity < 4u * table.columns) capacity <<= 1;
    vector<Bin> bins(capacity);
    vector<unsigned> touched;
    int stamp = 0;
    long long work = 0;

    auto nonconstant = [&](int j) {
        const int* r = table.row(j);
        int first = r[alive.front()];
        for (int c : alive)
            if (r[c] != first) return true;
        return false;
    };

    while (!alive.empty()) {
        int firstArea = table.areas[alive.front()];
        bool pure = true;
        for (int c : alive) if (table.areas[c] != firstArea) {
            pure = false;
            break;
        }
        if (pure) return firstArea;

        base.erase(remove_if(base.begin(), base.end(),
                             [&](int j) { return !nonconstant(j); }), base.end());
        int chosen = -1;
        if (alive.size() <= 11) {
            chosen = exactSmall(table, alive);
        } else if (work + 1LL * alive.size() * tests <= 48000000) {
            using Score = tuple<int, long long, int, int, int>;
            vector<Score> scores;
            vector<char> isBase(tests);
            for (int j : base) isBase[j] = true;
            work += 1LL * alive.size() * tests;

            for (int j = 0; j < tests; ++j) {
                ++stamp;
                touched.clear();
                const int* r = table.row(j);
                for (int c : alive) {
                    unsigned value = r[c];
                    unsigned h = value;
                    h ^= h >> 16;
                    h *= 0x7feb352du;
                    h ^= h >> 15;
                    h &= capacity - 1;
                    while (bins[h].stamp == stamp && bins[h].value != int(value))
                        h = (h + 1) & (capacity - 1);
                    Bin& b = bins[h];
                    if (b.stamp != stamp) {
                        b = {stamp, int(value), 1, table.areas[c]};
                        touched.push_back(h);
                    } else {
                        ++b.count;
                        if (b.area != table.areas[c]) b.area = -1;
                    }
                }
                if (touched.size() <= 1) continue;
                int unresolved = 0, worst = 0;
                long long squares = 0;
                for (unsigned h : touched) {
                    if (bins[h].area >= 0) continue;
                    int cnt = bins[h].count;
                    unresolved += cnt;
                    squares += 1LL * cnt * cnt;
                    worst = max(worst, cnt);
                }
                scores.emplace_back(unresolved, squares, worst, !isBase[j], j);
                if (unresolved == 0) break;
            }
            sort(scores.begin(), scores.end());

            int checked = 0;
            for (auto [unresolved, squares, worst, other, j] : scores) {
                if (unresolved == 0 || worst <= remaining ||
                    int(base.size()) < remaining || !other) {
                    chosen = j;
                    break;
                }

                // An alternative test can replace a quadrature test
                // if every large unresolved branch makes one redundant.
                if (remaining <= 1 || checked++ >= 10 || work > 48000000) continue;
                const int* r = table.row(j);
                vector<int> group = alive;
                sort(group.begin(), group.end(), [&](int a, int b) { return r[a] < r[b]; });
                bool safe = true;
                for (int lo = 0, hi; lo < int(group.size()) && safe; lo = hi) {
                    hi = lo + 1;
                    while (hi < int(group.size()) && r[group[lo]] == r[group[hi]]) ++hi;
                    if (hi - lo <= remaining) continue;
                    bool mixed = false;
                    for (int k = lo + 1; k < hi; ++k)
                        if (table.areas[group[k]] != table.areas[group[lo]]) {
                            mixed = true;
                            break;
                        }
                    if (!mixed) continue;
                    bool redundant = false;
                    for (int q : base) {
                        const int* test = table.row(q);
                        int val = test[group[lo]], k = lo + 1;
                        while (k < hi && test[group[k]] == val) ++k;
                        work += k - lo;
                        if (k == hi) {
                            redundant = true;
                            break;
                        }
                    }
                    if (!redundant) safe = false;
                }
                if (safe) {
                    chosen = j;
                    break;
                }
            }
        }

        if (chosen == -1 && !base.empty()) chosen = base.front();
        if (chosen == -1) return -1;
        int response = submod(ask(table.samples[chosen].query), table.hullValue[chosen]);
        const int* predicted = table.row(chosen);
        alive.erase(remove_if(alive.begin(), alive.end(),
                              [&](int c) { return predicted[c] != response; }),
                    alive.end());
        --remaining;
    }
    return -1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 3) return 0;
    vector<Point> pts(n);
    for (Point& p : pts)
        if (!(cin >> p.x >> p.y) || p.x < 0 || p.y < 0) return 0;
    sort(pts.begin(), pts.end(), [](Point a, Point b) {
        return tie(a.x, a.y) < tie(b.x, b.y);
    });

    vector<int> hull = boundary(pts);
    int hullArea = 0;
    for (int i = 0; i < int(hull.size()); ++i)
        hullArea += cross(pts[hull[i]], pts[hull[(i + 1) % hull.size()]]);
    if (hullArea == 0 || int(hull.size()) == n) {
        finish(hullArea);
        return 0;
    }

    vector<int> xs, count;
    for (Point p : pts) {
        if (xs.empty() || xs.back() != p.x) {
            xs.push_back(p.x);
            count.push_back(0);
        }
        ++count.back();
    }

    vector<Term> plan = makeQuadrature(xs, count);
    Enumerator enumerator(pts, hull, hullArea);
    if (enumerator.run()) {
        const auto& candidates = enumerator.candidates;
        if (all_of(candidates.begin(), candidates.end(), [&](const Candidate& c) {
                return c.area == candidates.front().area;
            })) {
            finish(candidates.front().area);
            return 0;
        }
        SliceTable table(pts, xs, hull, candidates, plan);
        int answer = solveCandidates(table);
        if (answer >= 0) {
            finish(answer);
            return 0;
        }
    }

    // The integral of each affine slab gives the quadrature weights
    // for twice the area. All slice denominators divide a product of
    // the query denominator and an lcm of integer edge widths, so the
    // calculation is valid modulo MOD. Twice the area is below MOD.
    int twiceArea = 0;
    for (const Term& t : plan)
        twiceArea = addmod(twiceArea, mulmod(modint(t.weight), ask(t.query)));
    finish(twiceArea);
    return 0;
}
