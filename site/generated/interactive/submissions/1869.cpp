#include <bits/stdc++.h>
using namespace std;

using Int = long long;
using U64 = uint64_t;
using U128 = __uint128_t;
using I128 = __int128_t;

static int n;
static mt19937_64 rng(0x726a028ef2d3407bULL);

struct Query {
    int l, r;
};

static Int ask(Query q) {
    cout << "? " << q.l << ' ' << q.r << endl;
    Int answer;
    if (!(cin >> answer) || answer < 0) exit(0);
    return answer;
}

static Int tri(Int x) {
    return x * (x - 1) / 2;
}

static U64 isqrt(U128 x) {
    U64 y = static_cast<U64>(sqrtl(static_cast<long double>(x)));
    while (U128(y + 1) * (y + 1) <= x) ++y;
    while (U128(y) * y > x) --y;
    return y;
}

static U64 mulmod(U64 a, U64 b, U64 m) {
    return U128(a) * b % m;
}

static U64 powmod(U64 a, U64 b, U64 m) {
    U64 r = 1;
    for (; b; b >>= 1, a = mulmod(a, a, m))
        if (b & 1) r = mulmod(r, a, m);
    return r;
}

static bool prime(U64 x) {
    if (x < 2) return false;
    for (U64 p : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37})
        if (x % p == 0) return x == p;
    int s = __builtin_ctzll(x - 1);
    U64 d = (x - 1) >> s;
    for (U64 a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL,
                  9780504ULL, 1795265022ULL}) {
        if (a % x == 0) continue;
        U64 y = powmod(a % x, d, x);
        if (y == 1 || y == x - 1) continue;
        bool ok = false;
        for (int j = 1; j < s; ++j) {
            y = mulmod(y, y, x);
            if (y == x - 1) {
                ok = true;
                break;
            }
        }
        if (!ok) return false;
    }
    return true;
}

static U64 rho(U64 m) {
    for (U64 p : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37})
        if (m % p == 0) return p;
    for (;;) {
        U64 y = 1 + rng() % (m - 1), c = 1 + rng() % (m - 1);
        auto f = [&](U64 x) { return (mulmod(x, x, m) + c) % m; };
        U64 g = 1, x = 0, ys = 0;
        for (U64 r = 1; g == 1 && r <= (1ULL << 20); r <<= 1) {
            x = y;
            for (U64 j = 0; j < r; ++j) y = f(y);
            for (U64 k = 0; k < r && g == 1; k += 128) {
                ys = y;
                U64 q = 1;
                for (U64 j = 0; j < min<U64>(128, r - k); ++j) {
                    y = f(y);
                    q = mulmod(q, x > y ? x - y : y - x, m);
                }
                g = gcd(q, m);
            }
        }
        if (g == m) {
            do {
                ys = f(ys);
                g = gcd(x > ys ? x - ys : ys - x, m);
            } while (g == 1);
        }
        if (g > 1 && g < m) return g;
    }
}

static void factor(U64 x, map<U64, int>& out) {
    if (x == 1) return;
    if (prime(x)) {
        ++out[x];
        return;
    }
    U64 p = rho(x);
    factor(p, out);
    factor(x / p, out);
}

struct Gaussian {
    Int x, y;
    Gaussian operator*(Gaussian b) const {
        return {x * b.x - y * b.y, x * b.y + y * b.x};
    }
};

static Gaussian splitPrime(U64 p) {
    U64 t = 2;
    while (powmod(t, (p - 1) / 2, p) != p - 1) ++t;
    U64 root = powmod(t, (p - 1) / 4, p);
    for (U64 r : {root, p - root}) {
        U64 a = p, b = r;
        while (U128(b) * b > p) {
            U64 v = a % b;
            a = b;
            b = v;
        }
        U64 rem = p - b * b;
        U64 y = isqrt(rem);
        if (y * y == rem) return {Int(b), Int(y)};
    }
    exit(0);
}

struct Range {
    int a, b, lo, hi;
};

struct State {
    int i, j, k;
};

static vector<Range> initialRanges(Int total) {
    // 4 * total + 1 = (a + b - 1)^2 + (a - b)^2.
    map<U64, int> factors;
    factor(4 * total + 1, factors);
    vector<Gaussian> reps{{1, 0}};
    Int scale = 1;

    for (auto [p, e] : factors) {
        if (p % 4 == 3) {
            for (int j = 0; j < e / 2; ++j) scale *= p;
        } else {
            Gaussian v = splitPrime(p), w{v.x, -v.y};
            vector<Gaussian> vp(e + 1, {1, 0}), wp = vp, next;
            for (int j = 1; j <= e; ++j) {
                vp[j] = vp[j - 1] * v;
                wp[j] = wp[j - 1] * w;
            }
            next.reserve(reps.size() * (e + 1));
            for (int j = 0; j <= e; ++j) {
                Gaussian g = vp[j] * wp[e - j];
                for (Gaussian z : reps) next.push_back(z * g);
            }
            reps.swap(next);
        }
    }

    set<pair<int, int>> lengths;
    for (Gaussian z : reps) {
        Int x = abs(z.x) * scale, y = abs(z.y) * scale;
        if (x < y) swap(x, y);
        if (x + 1 > n) continue;
        Int a = (x + 1 + y) / 2, b = (x + 1 - y) / 2;
        if (b >= 2 && tri(a) + tri(b) == total) {
            lengths.emplace(a, b);
            lengths.emplace(b, a);
        }
    }
    vector<Range> result;
    for (auto [a, b] : lengths) result.push_back({a, b, 1, n - a - b + 1});
    return result;
}

static int overlap(int l, int r, int a, int b) {
    return max(0, min(r, b) - max(l, a));
}

static Int value(const State& s, Query q) {
    int a = overlap(q.l, q.r + 1, s.i, s.j);
    int b = overlap(q.l, q.r + 1, s.j, s.k + 1);
    return tri(a) + tri(b);
}

// On each piece, twice the reply is A*t*t + B*t + C, t = start - lo.
struct Piece {
    int lo, len, A;
    Int B, C, mn, mx;

    Int eval(Int t) const {
        return (A * t + B) * t + C;
    }

    int roots(Int target, int* out = nullptr) const {
        if (target < mn || target > mx) return 0;
        if (A == 0) return len;
        I128 disc = I128(B) * B + I128(4 * A) * (target - C);
        if (disc < 0) return 0;
        Int d = isqrt(disc);
        if (I128(d) * d != disc) return 0;
        int cnt = 0;
        auto addRoot = [&](Int v) {
            if (v < 0 || v % (2 * A)) return;
            v /= 2 * A;
            if (v >= len) return;
            if (out) out[cnt] = int(v);
            ++cnt;
        };
        addRoot(-B - d);
        if (d) addRoot(-B + d);
        return cnt;
    }
};

template<class Func>
static void forPieces(const Range& s, Query q, Func use) {
    int cut[10], nc = 0;
    cut[nc++] = s.lo;
    cut[nc++] = s.hi + 1;
    for (int x : {q.l, q.r + 1}) {
        for (int d : {0, s.a, s.a + s.b}) {
            int c = x - d;
            if (s.lo < c && c <= s.hi) cut[nc++] = c;
        }
    }
    sort(cut, cut + nc);
    nc = int(unique(cut, cut + nc) - cut);
    for (int h = 0; h + 1 < nc; ++h) {
        int i = cut[h], len = cut[h + 1] - i;
        int j = i + s.a, k = j + s.b;
        int u = overlap(q.l, q.r + 1, i, j);
        int v = overlap(q.l, q.r + 1, j, k);
        int du = 0, dv = 0;
        if (len > 1) {
            du = overlap(q.l, q.r + 1, i + 1, j + 1) - u;
            dv = overlap(q.l, q.r + 1, j + 1, k + 1) - v;
        }
        Piece p{i, len, du * du + dv * dv,
                (2LL * u - 1) * du + (2LL * v - 1) * dv,
                2 * (tri(u) + tri(v)), 0, 0};
        p.mn = p.mx = p.C;
        if (p.A) {
            Int t = clamp(-p.B / (2 * p.A), 0LL, Int(len - 1));
            p.mn = min(p.eval(t), p.eval(min(t + 1, Int(len - 1))));
            p.mx = max(p.C, p.eval(len - 1));
        }
        use(p);
    }
}

static vector<Range> restrictRanges(const vector<Range>& ranges, Query q, Int ans) {
    vector<Range> result;
    for (const Range& s : ranges) {
        auto add = [&](int l, int r) {
            if (!result.empty()) {
                Range& last = result.back();
                if (last.a == s.a && last.b == s.b && last.hi + 1 == l) {
                    last.hi = r;
                    return;
                }
            }
            result.push_back({s.a, s.b, l, r});
        };
        forPieces(s, q, [&](const Piece& p) {
            if (p.mn == p.mx) {
                if (p.mn == 2 * ans) add(p.lo, p.lo + p.len - 1);
            } else {
                int sol[2];
                int m = p.roots(2 * ans, sol);
                for (int j = 0; j < m; ++j) add(p.lo + sol[j], p.lo + sol[j]);
            }
        });
    }
    return result;
}

static void normalize(vector<int>& a) {
    sort(a.begin(), a.end());
    a.erase(unique(a.begin(), a.end()), a.end());
}

struct Pool {
    vector<Query> list;
    unordered_set<U64> used;

    void add(int l, int r) {
        if (l < 1 || r > n || l >= r) return;
        if (used.insert((U64(l) << 32) | U64(r)).second)
            list.push_back({l, r});
    }

    void cuts(int x) {
        add(1, x);
        add(x + 1, n);
        add(x, n);
    }
};

static void addLandmarks(Pool& pool, vector<int> marks, int budget) {
    marks.push_back(1);
    marks.push_back(n);
    normalize(marks);

    vector<int> middle;
    for (int j = 1; j < int(marks.size()); ++j)
        if (marks[j] - marks[j - 1] > 1)
            middle.push_back((marks[j] + marks[j - 1]) / 2);

    vector<int> coord = marks;
    for (int x : marks) {
        if (x > 1) coord.push_back(x - 1);
        if (x < n) coord.push_back(x + 1);
    }
    coord.insert(coord.end(), middle.begin(), middle.end());
    normalize(coord);

    vector<Query> cuts, inside;
    for (int x : coord) {
        cuts.push_back({1, x});
        cuts.push_back({x, n});
    }
    for (int j = 0; j < int(coord.size()); ++j)
        for (int h = j + 1; h < int(coord.size()); ++h)
            if (coord[j] != 1 && coord[h] != n)
                inside.push_back({coord[j], coord[h]});

    if (budget != INT_MAX) {
        shuffle(cuts.begin(), cuts.end(), rng);
        shuffle(inside.begin(), inside.end(), rng);
    }
    for (Query q : cuts) {
        if (int(pool.list.size()) >= budget / 2) break;
        pool.add(q.l, q.r);
    }
    for (Query q : inside) {
        if (int(pool.list.size()) >= budget) break;
        pool.add(q.l, q.r);
    }
    for (Query q : cuts) {
        if (int(pool.list.size()) >= budget) break;
        pool.add(q.l, q.r);
    }
}

static int offset(const Range& s, int which) {
    if (which == 0) return 0;
    if (which == 1) return s.a;
    return s.a + s.b - 1;
}

static Int countLE(const vector<Range>& ranges, int x, int which) {
    Int count = 0;
    for (const Range& s : ranges)
        count += max(0, min(s.hi, x - offset(s, which)) - s.lo + 1);
    return count;
}

static int quantile(const vector<Range>& ranges, Int rank, int which) {
    int l = 1, r = n;
    while (l < r) {
        int m = (l + r) / 2;
        if (countLE(ranges, m, which) >= rank) r = m;
        else l = m + 1;
    }
    return l;
}

static Query chooseImplicit(const vector<Range>& ranges, Int count) {
    Pool pool;
    int median = quantile(ranges, (count + 1) / 2, 0) + 1;
    pool.cuts(median);
    int l = 2, r = n - 1;
    while (l < r) {
        int m = (l + r) / 2;
        if (count - countLE(ranges, m - 1, 0) <= countLE(ranges, m, 2)) r = m;
        else l = m + 1;
    }
    for (int x = l - 2; x <= l + 2; ++x) pool.cuts(x);

    vector<int> marks;
    for (int which = 0; which < 3; ++which)
        for (int j = 0; j <= 8; ++j)
            marks.push_back(quantile(ranges, 1 + (count - 1) * j / 8, which));
    for (int x : marks) pool.cuts(x);
    addLandmarks(pool, marks, clamp(100000 / (int(ranges.size()) + 1), 100, 640));

    Query best = {1, median};
    double bestScore = numeric_limits<double>::infinity();
    vector<Piece> variable;
    vector<pair<Int, Int>> constants, merged;
    variable.reserve(7 * ranges.size());
    constants.reserve(7 * ranges.size());

    for (Query q : pool.list) {
        constants.clear();
        variable.clear();
        Int varSize = 0, varCap = 0;
        for (const Range& s : ranges) {
            forPieces(s, q, [&](const Piece& p) {
                if (p.mn == p.mx) {
                    constants.emplace_back(p.mn, p.len);
                } else {
                    variable.push_back(p);
                    varSize += p.len;
                    varCap += 2;
                }
            });
        }
        sort(constants.begin(), constants.end());
        merged.clear();
        for (auto [v, f] : constants) {
            if (!merged.empty() && merged.back().first == v) merged.back().second += f;
            else merged.emplace_back(v, f);
        }
        Int maxConst = 0;
        double lower = 0;
        for (auto [v, f] : merged) {
            maxConst = max(maxConst, f);
            if (f > 1) lower += double(f) * log2(double(f));
        }
        Int worstBound = maxConst + varCap;
        if (worstBound * 5 > count * 4) continue;
        if (lower / count > bestScore) continue;

        bool refine = merged.size() <= 8 || merged.size() * variable.size() <= 1200;
        double sum = lower;
        if (refine) {
            for (auto [v, f] : merged) {
                Int exact = f;
                for (const Piece& p : variable) exact += p.roots(v);
                if (exact != f) sum += double(f) * log2(double(exact) / f);
            }
        }

        if (varSize) {
            int samples = int(min<Int>(varSize, ranges.size() <= 24 ? 36 : 16));
            Int before = 0;
            size_t at = 0;
            double extra = 0;
            for (int j = 0; j < samples; ++j) {
                Int index = varSize * (2 * j + 1) / (2 * samples);
                while (before + variable[at].len <= index) before += variable[at++].len;
                Int v = variable[at].eval(index - before);
                auto it = lower_bound(merged.begin(), merged.end(), pair<Int, Int>{v, 0});
                Int f = (it != merged.end() && it->first == v) ? it->second : 0;
                for (const Piece& p : variable) f += p.roots(v);
                if (f > 1) extra += log2(double(f));
            }
            sum += extra * (double(varSize) / samples);
        }
        double score = sum / count + 0.018 * log2(double(max<Int>(1, worstBound)));
        if (score + 1e-10 < bestScore) {
            bestScore = score;
            best = q;
        }
    }
    return best;
}

static Query separatingEdge(const State& a, const State& b) {
    auto edge = [](const State& s, int p) {
        return (s.i <= p && p < s.j - 1) || (s.j <= p && p < s.k);
    };
    for (int p : {a.i, a.j - 1, a.j, a.k, b.i, b.j - 1, b.j, b.k})
        if (p < n && edge(a, p) != edge(b, p)) return {p, p + 1};
    exit(0);
}

struct Partition {
    Query q;
    vector<int> groups;
    double score;
};

static Query exactSmall(int k, vector<Partition>& parts) {
    sort(parts.begin(), parts.end(), [](const Partition& a, const Partition& b) {
        return a.score < b.score;
    });
    int full = (1 << k) - 1;
    vector<int> cost(full + 1, -1), height(full + 1), choice(full + 1, -1);

    auto solve = [&](auto&& self, int mask) -> int {
        if (!(mask & (mask - 1))) return 0;
        if (cost[mask] >= 0) return cost[mask];
        int sz = __builtin_popcount(unsigned(mask));
        int best = INT_MAX, bestH = INT_MAX;
        for (int t = 0; t < int(parts.size()); ++t) {
            const auto& groups = parts[t].groups;
            int bound = sz;
            for (int g : groups) {
                int sub = g & mask;
                if (sub == mask) {
                    bound = INT_MAX;
                    break;
                }
                if (sub & (sub - 1)) bound += __builtin_popcount(unsigned(sub));
            }
            if (bound == INT_MAX || bound > best) continue;
            int sum = sz, h = 1;
            for (int g : groups) {
                int sub = g & mask;
                if (sub & (sub - 1)) {
                    sum += self(self, sub);
                    h = max(h, 1 + height[sub]);
                    if (sum > best) break;
                }
            }
            if (sum < best || (sum == best && h < bestH)) {
                best = sum;
                bestH = h;
                choice[mask] = t;
                if (best == sz) break;
            }
        }
        height[mask] = bestH;
        return cost[mask] = best;
    };

    solve(solve, full);
    return parts[choice[full]].q;
}

static Query chooseExplicit(const vector<State>& states, Query complement) {
    int k = int(states.size());
    if (k == 2) return separatingEdge(states[0], states[1]);

    Pool pool;
    pool.add(complement.l, complement.r);
    Query fallback = separatingEdge(states[0], states[1]);
    pool.add(fallback.l, fallback.r);

    if (n <= 26) {
        for (int l = 1; l < n; ++l)
            for (int r = l + 1; r <= n; ++r)
                pool.add(l, r);
    } else if (k <= 10) {
        vector<int> marks;
        for (const State& s : states) {
            for (int p : {s.i, s.j - 1, s.j, s.k}) {
                pool.add(p, p + 1);
                marks.push_back(p);
            }
        }
        addLandmarks(pool, marks, INT_MAX);
    } else {
        vector<int> marks;
        vector<int> v(k);
        for (int which = 0; which < 3; ++which) {
            for (int j = 0; j < k; ++j)
                v[j] = which == 0 ? states[j].i : which == 1 ? states[j].j : states[j].k;
            sort(v.begin(), v.end());
            if (which == 0) pool.cuts(v[(k - 1) / 2] + 1);
            for (int j = 0; j <= 12; ++j)
                marks.push_back(v[Int(k - 1) * j / 12]);
        }
        for (int x : marks) pool.cuts(x);
        addLandmarks(pool, marks, clamp(2400000 / k, 96, 950));
    }

    vector<double> penalty(k + 1);
    for (int j = 2; j <= k; ++j) penalty[j] = j * log2(double(j));
    vector<Int> answers(k);
    vector<pair<Int, int>> smallAnswers(k);
    vector<Partition> parts;
    unordered_set<U64> seen;
    Query best = fallback, balanced = fallback;
    double bestScore = numeric_limits<double>::infinity();
    double balancedScore = bestScore;
    int bestWorst = k;

    for (Query q : pool.list) {
        double score = 0;
        int worst = 0;
        if (k <= 10) {
            for (int j = 0; j < k; ++j) smallAnswers[j] = {value(states[j], q), j};
            sort(smallAnswers.begin(), smallAnswers.end());
            vector<int> groups;
            for (int j = 0; j < k;) {
                int end = j + 1;
                int mask = 1 << smallAnswers[j].second;
                while (end < k && smallAnswers[end].first == smallAnswers[j].first)
                    mask |= 1 << smallAnswers[end++].second;
                groups.push_back(mask);
                worst = max(worst, end - j);
                score += penalty[end - j];
                j = end;
            }
            if (worst == 1) return q;
            if (worst == k) continue;
            sort(groups.begin(), groups.end());
            U64 signature = 1;
            // All masks together have only k set bits; label the group of each state.
            signature = 0;
            for (int j = 0; j < int(groups.size()); ++j)
                for (int m = groups[j]; m; m &= m - 1)
                    signature |= U64(j) << (4 * __builtin_ctz(unsigned(m)));
            if (seen.insert(signature).second) parts.push_back({q, move(groups), score});
        } else {
            for (int j = 0; j < k; ++j) answers[j] = value(states[j], q);
            sort(answers.begin(), answers.end());
            for (int j = 0; j < k;) {
                int end = j + 1;
                while (end < k && answers[end] == answers[j]) ++end;
                worst = max(worst, end - j);
                score += penalty[end - j];
                j = end;
            }
            if (worst == 1) return q;
            if (worst == k) continue;
            if (score < bestScore || (score == bestScore && worst < bestWorst)) {
                bestScore = score;
                bestWorst = worst;
                best = q;
            }
            if (Int(worst) * 5 <= Int(k) * 4 && score < balancedScore) {
                balancedScore = score;
                balanced = q;
            }
        }
    }
    if (k <= 10) return exactSmall(k, parts);
    if (Int(bestWorst) * 5 > Int(k) * 4 && isfinite(balancedScore)) return balanced;
    return best;
}

static void report(const State& s) {
    cout << "! " << s.i << ' ' << s.j << ' ' << s.k << endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n) || n < 4) return 0;
    vector<State> states;
    Query complement{0, 0};
    Int total = -1;

    if (n <= 24) {
        for (int i = 1; i + 3 <= n; ++i)
            for (int j = i + 2; j < n; ++j)
                for (int k = j + 1; k <= n; ++k)
                    states.push_back({i, j, k});
    } else {
        total = ask({1, n});
        vector<Range> ranges = initialRanges(total);
        for (;;) {
            Int count = 0;
            for (const Range& s : ranges) count += s.hi - s.lo + 1;
            if (count == 0) return 0;
            if (count <= max<Int>(1200, 24LL * Int(ranges.size()))) {
                states.reserve(size_t(count));
                for (const Range& s : ranges)
                    for (int i = s.lo; i <= s.hi; ++i)
                        states.push_back({i, i + s.a, i + s.a + s.b - 1});
                break;
            }
            Query q = chooseImplicit(ranges, count);
            Int v = ask(q);
            complement = {0, 0};
            if (0 < v && v < total) {
                if (q.l == 1) complement = {q.r + 1, n};
                else if (q.r == n) complement = {1, q.l - 1};
            }
            ranges = restrictRanges(ranges, q, v);
        }
    }

    while (states.size() > 1) {
        Query q = chooseExplicit(states, complement);
        Int v = ask(q);
        states.erase(remove_if(states.begin(), states.end(), [&](const State& s) {
            return value(s, q) != v;
        }), states.end());

        if (q.l == 1 && q.r == n) total = v;
        complement = {0, 0};
        if (0 < v && v < total) {
            if (q.l == 1) complement = {q.r + 1, n};
            else if (q.r == n) complement = {1, q.l - 1};
        }
    }
    if (!states.empty()) report(states.front());
    return 0;
}
