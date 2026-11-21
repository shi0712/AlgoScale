#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
using u64 = uint64_t;
using i128 = __int128_t;
using u128 = __uint128_t;

struct Query {
    int l, r;
    auto operator<=>(const Query&) const = default;
};

i64 ask(Query query) {
    cout << "? " << query.l << ' ' << query.r << '\n' << flush;
    i64 reply;
    if (!(cin >> reply) || reply < 0) exit(0);
    return reply;
}

// Both block lengths are fixed, and the first block may start anywhere in [lo, hi].
struct Run {
    int a, b;
    int lo, hi;
};

void answer(const Run& run) {
    int j = run.lo + run.a;
    cout << "! " << run.lo << ' ' << j << ' ' << j + run.b - 1 << '\n' << flush;
}

i64 triangle(i64 length) {
    return length * (length - 1) / 2;
}

u64 integer_sqrt(u64 value) {
    u64 root = sqrtl(static_cast<long double>(value));
    while (u128(root) * root > value) --root;
    while (u128(root + 1) * (root + 1) <= value) ++root;
    return root;
}

int triangle_length(i64 value) {
    if (value < 0) return 0;
    i64 length = (integer_sqrt(8 * value + 1) + 1) / 2;
    return triangle(length) == value ? length : 0;
}

int overlap(int start, int length, Query q) {
    return max(0, min(start + length - 1, q.r) - max(start, q.l) + 1);
}

i64 inversions(int i, int a, int b, Query q) {
    return triangle(overlap(i, a, q)) + triangle(overlap(i + a, b, q));
}

u64 mix(u64 x) {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

namespace sums_of_triangles {
u64 mul_mod(u64 a, u64 b, u64 mod) {
    return u128(a) * b % mod;
}

u64 power_mod(u64 base, u64 exponent, u64 mod) {
    u64 result = 1;
    for (; exponent; exponent >>= 1) {
        if (exponent & 1) result = mul_mod(result, base, mod);
        base = mul_mod(base, base, mod);
    }
    return result;
}

constexpr array<u64, 12> small_primes{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

bool is_prime(u64 n) {
    if (n < 2) return false;
    for (u64 p : small_primes) {
        if (n % p == 0) return n == p;
    }
    int twos = countr_zero(n - 1);
    u64 odd = (n - 1) >> twos;
    for (u64 base : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}) {
        if (base % n == 0) continue;
        u64 x = power_mod(base % n, odd, n);
        if (x == 1 || x == n - 1) continue;
        for (int s = 1; s < twos && x != n - 1; ++s) x = mul_mod(x, x, n);
        if (x != n - 1) return false;
    }
    return true;
}

u64 find_factor(u64 n) {
    static u64 seed = 0x123456789abcdefULL;
    auto random_below = [&](u64 limit) {
        seed += 0x9e3779b97f4a7c15ULL;
        return mix(seed) % limit;
    };
    auto distance = [](u64 a, u64 b) { return a < b ? b - a : a - b; };
    while (true) {
        u64 y = 1 + random_below(n - 1);
        u64 c = 1 + random_below(n - 1);
        auto advance = [&](u64 x) { return (mul_mod(x, x, n) + c) % n; };
        u64 factor = 1, x = 0, saved = 0;
        for (int length = 1; factor == 1 && length <= (1 << 20); length *= 2) {
            x = y;
            for (int t = 0; t < length; ++t) y = advance(y);
            for (int t = 0; t < length && factor == 1; t += 128) {
                saved = y;
                u64 product = 1;
                for (int s = 0; s < min(128, length - t); ++s) {
                    y = advance(y);
                    product = mul_mod(product, distance(x, y), n);
                }
                factor = gcd(product, n);
            }
        }
        if (factor == n) {
            do {
                saved = advance(saved);
                factor = gcd(distance(x, saved), n);
            } while (factor == 1);
        }
        if (factor > 1 && factor < n) return factor;
    }
}

void factorize(u64 n, map<u64, int>& factors) {
    if (n == 1) return;
    if (is_prime(n)) {
        ++factors[n];
        return;
    }
    u64 factor = find_factor(n);
    factorize(factor, factors);
    factorize(n / factor, factors);
}

struct Gaussian {
    i64 re, im;
};

Gaussian operator*(Gaussian a, Gaussian b) {
    return {a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
}

Gaussian prime_as_squares(u64 p) {
    u64 base = 2;
    while (power_mod(base, (p - 1) / 2, p) != p - 1) ++base;
    u64 root = power_mod(base, (p - 1) / 4, p);
    u64 x = p, y = root;
    while (u128(y) * y > p) {
        u64 next = x % y;
        x = y;
        y = next;
    }
    return {static_cast<i64>(y), static_cast<i64>(integer_sqrt(p - y * y))};
}

vector<pair<int, int>> lengths(i64 total, int limit) {
    // Writing x=a+b-1 and y=a-b gives x^2+y^2=4*total+1.
    // Allocate each split prime between its two conjugate Gaussian factors.
    u64 value = 4 * total + 1;
    map<u64, int> factors;
    for (u64 p : small_primes) {
        while (value % p == 0) {
            ++factors[p];
            value /= p;
        }
    }
    factorize(value, factors);
    vector<Gaussian> products{{1, 0}};
    i64 scale = 1;
    for (auto [p, exponent] : factors) {
        if (p % 4 == 3) {
            if (exponent % 2) return {};
            for (int t = 0; t < exponent / 2; ++t) scale *= p;
            continue;
        }
        Gaussian prime = prime_as_squares(p);
        vector<Gaussian> powers(exponent + 1, {1, 0});
        for (int t = 1; t <= exponent; ++t) powers[t] = powers[t - 1] * prime;
        vector<Gaussian> next;
        next.reserve(products.size() * (exponent + 1));
        for (int t = 0; t <= exponent; ++t) {
            Gaussian left = powers[t], right = powers[exponent - t];
            right.im = -right.im;
            Gaussian multiplier = left * right;
            for (Gaussian product : products) next.push_back(product * multiplier);
        }
        products.swap(next);
    }

    vector<pair<int, int>> result;
    for (auto [re, im] : products) {
        i64 x = abs(re) * scale, y = abs(im) * scale;
        if (x < y) swap(x, y);
        if (x + 1 > limit || x <= y) continue;
        int a = (x + y + 1) / 2, b = (x - y + 1) / 2;
        result.emplace_back(a, b);
        result.emplace_back(b, a);
    }
    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    return result;
}
} // namespace sums_of_triangles

// The reply along a contiguous run is first + delta*t + curvature*T(t).
struct Piece {
    int lo, hi;
    i64 first, delta;
    int curvature;

    i64 at(i64 t) const {
        return first + delta * t + curvature * triangle(t);
    }

    bool constant() const { return delta == 0 && curvature == 0; }
    int size() const { return hi - lo + 1; }
};

template<class Visitor>
void visit_pieces(const Run& run, Query q, Visitor visit) {
    if (run.lo == run.hi) {
        visit(Piece{run.lo, run.hi, inversions(run.lo, run.a, run.b, q), 0, 0});
        return;
    }

    // As the block slides, its overlap is affine between these cuts.
    auto cuts = [&](int length, int offset) -> array<int, 4> {
        return {q.l - length + 1 - offset, min(q.l, q.r - length + 1) + 1 - offset,
                max(q.l, q.r - length + 1) + 1 - offset, q.r + 1 - offset};
    };
    auto a_cuts = cuts(run.a, 0);
    auto b_cuts = cuts(run.b, run.a);
    int ai = 0, bi = 0;
    for (int lo = run.lo; lo <= run.hi;) {
        while (ai < 4 && a_cuts[ai] <= lo) ++ai;
        while (bi < 4 && b_cuts[bi] <= lo) ++bi;
        int hi = run.hi;
        if (ai < 4) hi = min(hi, a_cuts[ai] - 1);
        if (bi < 4) hi = min(hi, b_cuts[bi] - 1);
        int a = overlap(lo, run.a, q), b = overlap(lo + run.a, run.b, q);
        i64 first = triangle(a) + triangle(b);
        i64 delta = 0;
        int curvature = 0;
        if (hi > lo) {
            int an = overlap(lo + 1, run.a, q), bn = overlap(lo + run.a + 1, run.b, q);
            delta = triangle(an) + triangle(bn) - first;
            if (hi - lo >= 2) curvature = (an - a) * (an - a) + (bn - b) * (bn - b);
        }
        visit(Piece{lo, hi, first, delta, curvature});
        lo = hi + 1;
    }
}

template<class Visitor>
void matching_positions(const Piece& piece, i64 value, Visitor visit) {
    if (!piece.curvature) {
        if (!piece.delta) {
            if (value == piece.first) visit(piece.lo, piece.hi);
        } else {
            i64 difference = value - piece.first;
            if (difference % piece.delta == 0) {
                i64 t = difference / piece.delta;
                if (0 <= t && t < piece.size()) visit(piece.lo + t, piece.lo + t);
            }
        }
        return;
    }
    i64 c = piece.curvature;
    i64 b = 2 * piece.delta - c;
    i128 discriminant = i128(b) * b + 8 * i128(c) * (value - piece.first);
    if (discriminant < 0) return;
    u64 root = integer_sqrt(static_cast<u64>(discriminant));
    if (i128(root) * root != discriminant) return;
    auto check_root = [&](i64 numerator) {
        if (numerator % (2 * c) == 0) {
            i64 t = numerator / (2 * c);
            if (0 <= t && t < piece.size()) visit(piece.lo + t, piece.lo + t);
        }
    };
    check_root(-b + i64(root));
    if (root) check_root(-b - i64(root));
}

vector<Run> filter(const vector<Run>& runs, Query q, i64 value) {
    vector<Run> survivors;
    for (const Run& run : runs) {
        visit_pieces(run, q, [&](const Piece& piece) {
            matching_positions(piece, value, [&](int lo, int hi) {
                survivors.push_back({run.a, run.b, lo, hi});
            });
        });
    }
    return survivors;
}

void compact(vector<Run>& runs) {
    sort(runs.begin(), runs.end(), [](const Run& x, const Run& y) {
        return tie(x.a, x.b, x.lo, x.hi) < tie(y.a, y.b, y.lo, y.hi);
    });
    size_t kept = 0;
    for (Run run : runs) {
        if (kept && runs[kept - 1].a == run.a && runs[kept - 1].b == run.b &&
            run.lo <= runs[kept - 1].hi + 1) {
            runs[kept - 1].hi = max(runs[kept - 1].hi, run.hi);
        } else {
            runs[kept++] = run;
        }
    }
    runs.resize(kept);
}

i64 count_candidates(const vector<Run>& runs) {
    i64 total = 0;
    for (auto run : runs) total += run.hi - run.lo + 1;
    return total;
}

vector<Run> samples_of(const vector<Run>& runs, i64 total, int count) {
    vector<Run> points;
    size_t index = 0;
    i64 passed = 0;
    for (int t = 0; t < count; ++t) {
        i64 rank = (2 * t + 1) * total / (2 * count);
        while (rank >= passed + runs[index].hi - runs[index].lo + 1) {
            passed += runs[index].hi - runs[index].lo + 1;
            ++index;
        }
        Run run = runs[index];
        run.lo += rank - passed;
        run.hi = run.lo;
        points.push_back(run);
    }
    return points;
}

Query separating_edge(const Run& x, const Run& y) {
    for (const Run& p : {x, y}) {
        int i = p.lo, j = i + p.a, k = j + p.b - 1;
        for (int pos : {i, j - 1, j, k}) {
            Query q{pos, pos + 1};
            if (inversions(x.lo, x.a, x.b, q) != inversions(y.lo, y.a, y.b, q)) return q;
        }
    }
    abort();
}

vector<Query> propose_queries(const vector<Run>& runs, i64 total, int left, int right,
                              int suffix_limit) {
    vector<Query> queries;
    auto add = [&](int l, int r) {
        if (left <= l && l < r && r <= right &&
            (!suffix_limit || (l <= suffix_limit && r == right))) queries.push_back({l, r});
    };

    // Balance the placements that would give zero and a whole-span count.
    int lo = left, hi = min(right, suffix_limit ? suffix_limit : right);
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        i64 starts_after = 0, ends_before = 0;
        for (Run run : runs) {
            starts_after += max(0, run.hi - max(run.lo, mid) + 1);
            ends_before += max(0, min(run.hi, mid - run.a - run.b + 1) - run.lo + 1);
        }
        if (starts_after > ends_before) lo = mid + 1;
        else hi = mid;
    }
    int balanced = lo;
    for (int x = balanced - 3; x <= balanced + 3; ++x) {
        add(left, x);
        add(x, right);
    }
    if (suffix_limit) add(left, right);

    if (right - left <= 45) {
        for (int l = left; l < right; ++l) {
            if (suffix_limit) add(l, right);
            else for (int r = l + 1; r <= right; ++r) add(l, r);
        }
    } else {
        auto points = samples_of(runs, total, min<i64>(total, 32));
        vector<int> starts, middles, ends;
        for (Run run : points) {
            starts.push_back(run.lo);
            middles.push_back(run.lo + run.a);
            ends.push_back(run.lo + run.a + run.b - 1);
        }
        vector<int> landmarks{left, right, balanced};
        if (total <= 20 && !suffix_limit) {
            for (Run run : points) {
                int j = run.lo + run.a, k = j + run.b - 1;
                for (int x : {run.lo, run.lo + 1, j - 1, j, k - 1, k}) landmarks.push_back(x);
            }
            sort(landmarks.begin(), landmarks.end());
            landmarks.erase(unique(landmarks.begin(), landmarks.end()), landmarks.end());
            if (landmarks.size() <= 32) {
                int original = landmarks.size();
                for (int t = 1; t < original; ++t) {
                    if (landmarks[t] - landmarks[t - 1] > 2)
                        landmarks.push_back((landmarks[t] + landmarks[t - 1]) / 2);
                }
            }
        } else {
            for (auto* positions : {&starts, &middles, &ends}) {
                sort(positions->begin(), positions->end());
                int size = positions->size();
                for (int step : {0, 1, 2, 4, 6, 7, 8}) {
                    int x = (*positions)[step * (size - 1) / 8];
                    landmarks.push_back(x);
                    add(left, x - 1);
                    add(x + 1, right);
                }
            }
            landmarks.push_back(left + (right - left) / 4);
            landmarks.push_back(left + (right - left) / 2);
            landmarks.push_back(right - (right - left) / 4);
        }
        sort(landmarks.begin(), landmarks.end());
        landmarks.erase(unique(landmarks.begin(), landmarks.end()), landmarks.end());
        for (int x : landmarks) {
            add(left, x);
            add(x, right);
            if (suffix_limit) continue;
            add(x - 1, x);
            add(x, x + 1);
        }
        if (!suffix_limit) {
            int stride = landmarks.size() > 55 ? 2 : 1;
            for (int p = 0; p < int(landmarks.size()); p += stride) {
                for (int q = p + 1; q < int(landmarks.size()); q += stride)
                    add(landmarks[p], landmarks[q]);
            }
        }
    }
    if (!suffix_limit) {
        Run first = runs.front(), last = runs.back();
        last.lo = last.hi;
        Query edge = separating_edge(first, last);
        add(edge.l, edge.r);
    }
    sort(queries.begin(), queries.end());
    queries.erase(unique(queries.begin(), queries.end()), queries.end());
    return queries;
}

double future_cost(i64 size) {
    if (size <= 1) return 0;
    return log2(double(size));
}

double score_of_bins(const vector<pair<i64, i64>>& bins, i64 total) {
    double sum = 0;
    i64 largest = 1;
    for (auto [value, size] : bins) {
        sum += size * future_cost(size);
        largest = max(largest, size);
    }
    return sum / total + 0.06 * future_cost(largest);
}

vector<pair<i64, i64>> constant_bins(const vector<Piece>& pieces) {
    vector<pair<i64, i64>> bins;
    for (const Piece& piece : pieces) {
        if (piece.constant()) bins.emplace_back(piece.first, piece.size());
    }
    sort(bins.begin(), bins.end());
    size_t kept = 0;
    for (auto bin : bins) {
        if (kept && bins[kept - 1].first == bin.first) bins[kept - 1].second += bin.second;
        else bins[kept++] = bin;
    }
    bins.resize(kept);
    return bins;
}

double sampled_score(const vector<Piece>& pieces, vector<pair<i64, i64>> bins,
                     i64 total, int sample_count) {
    vector<Piece> varying;
    i64 changing = 0;
    for (const Piece& piece : pieces) {
        if (!piece.constant()) {
            varying.push_back(piece);
            changing += piece.size();
        }
    }
    if (!changing || changing < total / 100000) return score_of_bins(bins, total);

    auto matches = [&](i64 value) {
        i64 count = 0;
        for (const Piece& piece : varying) {
            matching_positions(piece, value, [&](int, int) { ++count; });
        }
        return count;
    };
    // Account exactly for varying placements that join a large constant bucket.
    for (auto& [value, size] : bins) {
        if (size >= max<i64>(2, total / 200)) size += matches(value);
    }
    double sum = 0;
    size_t index = 0;
    i64 passed = 0;
    int count = min<i64>(sample_count, changing);
    for (int t = 0; t < count; ++t) {
        i64 rank = (2 * t + 1) * changing / (2 * count);
        while (rank >= passed + varying[index].size()) passed += varying[index++].size();
        i64 value = varying[index].at(rank - passed);
        auto it = lower_bound(bins.begin(), bins.end(), pair<i64, i64>{value, 0});
        i64 fixed = it != bins.end() && it->first == value ? it->second : 0;
        if (fixed >= max<i64>(2, total / 200)) continue;
        sum += future_cost(fixed + matches(value));
    }
    return score_of_bins(bins, total) + (sum / count) * (double(changing) / total);
}

struct SuffixChoice {
    int cut, right, first_start;
    vector<pair<int, int>> starts;

    void add_pieces(Query q, vector<Piece>& pieces) const {
        int end_count = right - cut;
        for (auto [i, j] : starts) {
            int first = max(0, j - max(i, q.l));
            int second = cut + 2 - max(j, q.l);
            pieces.push_back({cut + 1, right, triangle(first) + triangle(second),
                              end_count >= 2 ? second : 0, end_count >= 3 ? 1 : 0});
        }
    }

    // When the prefix ends inside the first block, the suffix can hide
    // the order of the two visible lengths. Count the feasible swaps.
    i64 matching_swaps(int left) const {
        int start = max(left, first_start);
        int min_j = max(cut + 1, start + 2);
        int min_b = max(2, cut + 1 - start);
        int choices = max(0, right - min_j + 2 - min_b);
        int diagonal = max(0, (right + start + 1) / 2 - max(min_j, start + min_b) + 1);
        return triangle(choices + 1) - diagonal;
    }

    double penalty(int left, i64 others) const {
        if (!first_start) return 0;
        i64 aliases = matching_swaps(left) - matching_swaps(first_start);
        i64 restricted_total = max(others, triangle(right - cut) / 20);
        return 0.6 * aliases / max<i64>(1, restricted_total);
    }
};

Query choose_query(const vector<Run>& runs, int left, int right, const SuffixChoice* suffix = nullptr) {
    int suffix_limit = suffix ? suffix->cut : 0;
    i64 contained = count_candidates(runs);
    i64 total = contained;
    if (suffix) total += i64(suffix->starts.size()) * (right - suffix_limit);
    if (total <= 1 && suffix) return {left, right};
    if (total == 2 && !suffix_limit) {
        Run other = runs.back();
        other.lo = other.hi;
        return separating_edge(runs.front(), other);
    }

    vector<Query> queries;
    if (contained) queries = propose_queries(runs, contained, left, right, suffix_limit);
    if (suffix) {
        size_t stride = (suffix->starts.size() + 31) / 32;
        for (size_t index = 0; index < suffix->starts.size(); index += stride) {
            auto [i, j] = suffix->starts[index];
            for (int x : {i, i + 1, j - 1, j}) {
                if (left <= x && x <= suffix_limit) queries.push_back({x, right});
            }
        }
        if (suffix->first_start) queries.push_back({suffix->first_start, right});
        queries.push_back({left, right});
        sort(queries.begin(), queries.end());
        queries.erase(unique(queries.begin(), queries.end()), queries.end());
    }
    auto penalty = [&](Query q) { return suffix ? suffix->penalty(q.l, total) : 0.0; };
    if (total <= 900) {
        auto points = samples_of(runs, contained, contained);
        vector<i64> values(total);
        vector<double> costs(total + 1);
        for (int t = 2; t <= total; ++t) costs[t] = future_cost(t);
        double best_score = numeric_limits<double>::infinity();
        Query best = queries.front();
        vector<Piece> extended;
        for (Query q : queries) {
            int filled = 0;
            for (auto [a, b, i, unused] : points) values[filled++] = inversions(i, a, b, q);
            if (suffix) {
                extended.clear();
                suffix->add_pieces(q, extended);
                for (const Piece& piece : extended) {
                    for (int u = 0; u < piece.size(); ++u) values[filled++] = piece.at(u);
                }
            }
            sort(values.begin(), values.end());
            double sum = 0;
            int largest = 1;
            for (int t = 0, end; t < total; t = end) {
                for (end = t + 1; end < total && values[t] == values[end]; ++end) {}
                int size = end - t;
                sum += size * costs[size];
                largest = max(largest, size);
            }
            if (largest == 1 && !suffix) return q;
            double score = sum / total + 0.06 * costs[largest] + penalty(q);
            if (score < best_score - 1e-10) {
                best = q;
                best_score = score;
            }
        }
        return best;
    }

    // Score constant reply buckets exactly and sample quadratic pieces for the shortlist.
    vector<pair<double, Query>> options;
    vector<Piece> pieces;
    pieces.reserve(runs.size() * 7);
    auto build_pieces = [&](Query q) {
        pieces.clear();
        for (const Run& run : runs) {
            visit_pieces(run, q, [&](Piece piece) { pieces.push_back(piece); });
        }
        if (suffix) suffix->add_pieces(q, pieces);
    };
    for (Query q : queries) {
        build_pieces(q);
        auto bins = constant_bins(pieces);
        if (!suffix && bins.size() == 1 && bins.front().second == total) continue;
        options.emplace_back(score_of_bins(bins, total) + penalty(q), q);
    }
    int finalists = min<int>(runs.size() <= 64 ? 10 : 5, options.size());
    partial_sort(options.begin(), options.begin() + finalists, options.end());
    Query best = options.front().second;
    double best_score = numeric_limits<double>::infinity();
    for (int t = 0; t < finalists; ++t) {
        Query q = options[t].second;
        build_pieces(q);
        double score = sampled_score(pieces, constant_bins(pieces), total, runs.size() <= 64 ? 24 : 12)
                     + penalty(q);
        if (score < best_score - 1e-10) {
            best = q;
            best_score = score;
        }
    }
    return best;
}

// Optimize the sum of remaining query counts over a small candidate set.
class SmallTree {
    using Mask = u128;

    struct MaskHash {
        size_t operator()(Mask mask) const {
            return mix(u64(mask)) ^ rotl(mix(u64(mask >> 64)), 19);
        }
    };

    struct Option {
        Query q;
        vector<Mask> groups;
    };

    struct Decision {
        int cost, depth;
        Query query;
    };

    vector<Option> options;
    unordered_map<Mask, Decision, MaskHash> memo;

    static int size_of(Mask mask) {
        return popcount(u64(mask)) + popcount(u64(mask >> 64));
    }

    static int first_bit(Mask mask) {
        return u64(mask) ? countr_zero(u64(mask)) : 64 + countr_zero(u64(mask >> 64));
    }

    Decision best(Mask mask) {
        int size = size_of(mask);
        if (size <= 1) return {0, 0, {0, 0}};
        if (auto it = memo.find(mask); it != memo.end()) return it->second;
        Decision result{INT_MAX, INT_MAX, {0, 0}};
        for (const auto& [q, groups] : options) {
            int cost = size, depth = 0;
            for (Mask group : groups) {
                Mask next = group & mask;
                if (next == mask) {
                    cost = INT_MAX;
                    break;
                }
                Decision child = best(next);
                if (child.cost == INT_MAX) {
                    cost = INT_MAX;
                    break;
                }
                cost += child.cost;
                depth = max(depth, child.depth);
                if (cost > result.cost) break;
            }
            if (tie(cost, depth) < tie(result.cost, result.depth)) result = {cost, depth, q};
            if (result.cost == size) break;
        }
        return memo.emplace(mask, result).first->second;
    }

public:
    void solve(const vector<Run>& points, const vector<Query>& queries) {
        set<vector<Mask>> partitions;
        for (Query q : queries) {
            map<i64, Mask> groups;
            for (size_t t = 0; t < points.size(); ++t) {
                const auto& [a, b, i, unused] = points[t];
                groups[inversions(i, a, b, q)] |= Mask(1) << t;
            }
            if (groups.size() == points.size()) {
                i64 value = ask(q);
                for (const auto& [a, b, i, unused] : points) {
                    if (inversions(i, a, b, q) == value) {
                        answer({a, b, i, i});
                        return;
                    }
                }
                return;
            }
            if (groups.size() <= 1) continue;
            vector<Mask> masks;
            for (auto [value, mask] : groups) masks.push_back(mask);
            sort(masks.begin(), masks.end());
            if (partitions.insert(masks).second) options.push_back({q, move(masks)});
        }
        sort(options.begin(), options.end(), [](const Option& x, const Option& y) {
            return x.groups.size() > y.groups.size();
        });
        Mask mask = (Mask(1) << points.size()) - 1;
        while (size_of(mask) > 1) {
            Decision decision = best(mask);
            if (!decision.query.l) {
                int x = first_bit(mask);
                int y = first_bit(mask ^ (Mask(1) << x));
                decision.query = separating_edge(points[x], points[y]);
            }
            i64 value = ask(decision.query);
            for (Mask remaining = mask; remaining; remaining &= remaining - 1) {
                int t = first_bit(remaining);
                const auto& [a, b, i, unused] = points[t];
                if (inversions(i, a, b, decision.query) != value) mask ^= Mask(1) << t;
            }
        }
        if (mask) answer(points[first_bit(mask)]);
    }
};

void solve_known(vector<Run> runs) {
    compact(runs);
    while (!runs.empty()) {
        i64 total = count_candidates(runs);
        if (total == 1) {
            answer(runs.front());
            return;
        }
        int left = INT_MAX, right = 0;
        for (Run run : runs) {
            left = min(left, run.lo);
            right = max(right, run.hi + run.a + run.b - 1);
        }
        if (total <= 13 && total >= 3) {
            auto points = samples_of(runs, total, total);
            auto queries = propose_queries(runs, total, left, right, 0);
            SmallTree().solve(points, queries);
            return;
        }
        Query q = choose_query(runs, left, right);
        i64 reply = ask(q);
        runs = filter(runs, q, reply);
        compact(runs);
    }
}

void solve_small(int left, int right) {
    vector<Run> points;
    for (int i = left; i + 3 <= right; ++i) {
        for (int j = i + 2; j + 1 <= right; ++j) {
            for (int k = j + 1; k <= right; ++k) points.push_back({j - i, k - j + 1, i, i});
        }
    }
    if (points.size() == 1) {
        answer(points.front());
        return;
    }
    if (points.size() <= 120) {
        vector<Query> queries;
        for (int l = left; l < right; ++l) {
            for (int r = l + 1; r <= right; ++r) queries.push_back({l, r});
        }
        SmallTree().solve(points, queries);
        return;
    }
    vector<int> bins(triangle(right - left + 1) + 1);
    vector<double> costs(points.size() + 1);
    for (size_t t = 2; t < costs.size(); ++t) costs[t] = future_cost(t);
    Query best{left, right};
    double best_score = numeric_limits<double>::infinity();
    for (int l = left; l < right; ++l) {
        for (int r = l + 1; r <= right && l - left <= right - r; ++r) {
            Query q{l, r};
            fill(bins.begin(), bins.end(), 0);
            for (const auto& [a, b, i, unused] : points) ++bins[inversions(i, a, b, q)];
            double sum = 0;
            int largest = 1;
            for (int size : bins) {
                sum += size * costs[size];
                largest = max(largest, size);
            }
            double score = sum / points.size() + 0.06 * costs[largest];
            if (score < best_score - 1e-10) {
                best = q;
                best_score = score;
            }
        }
    }
    solve_known(filter(points, best, ask(best)));
}

void solve_large(int left, int right) {
    int percent = 73;
    while (right - left + 1 > 22) {
        int cut = left + i64(right - left) * percent / 100;
        i64 prefix = ask({left, cut});
        if (prefix == 0) {
            // The first reversed block contributes as soon as two of its entries are seen.
            left = cut;
            percent = 53;
            continue;
        }

        auto prefix_lengths = sums_of_triangles::lengths(prefix, cut - left + 1);
        vector<Run> contained;
        for (auto [a, b] : prefix_lengths) {
            if (a >= 2 && b >= 2) contained.push_back({a, b, left, cut - a - b + 1});
        }
        int seen_first = triangle_length(prefix);
        SuffixChoice choice{cut, right, seen_first >= 2 ? cut - seen_first + 1 : 0, {}};
        for (auto [a, b] : prefix_lengths) {
            if (a >= 2) choice.starts.emplace_back(cut - a - b + 1, cut - b + 1);
        }
        Query suffix_query = choose_query(contained, left, right, &choice);
        i64 suffix = ask(suffix_query);
        vector<Run> runs = filter(contained, suffix_query, suffix);

        if (suffix > 0) {
            // A prefix ending in the second block determines i and j for each visible shape.
            for (auto [a, b] : prefix_lengths) {
                if (a < 2) continue;
                int j = cut - b + 1, i = j - a;
                int visible_first = max(0, j - max(i, suffix_query.l));
                int visible_second = triangle_length(suffix - triangle(visible_first));
                if (visible_second < 2) continue;
                i64 k = i64(max(j, suffix_query.l)) + visible_second - 1;
                if (cut < k && k <= right) runs.push_back({a, int(k) - j + 1, i, i});
            }

            // A prefix ending in the first block reveals i by itself.
            if (seen_first >= 2) {
                int i = cut - seen_first + 1;
                int start = max(i, suffix_query.l);
                auto suffix_lengths = sums_of_triangles::lengths(suffix, right - start + 1);
                for (auto [a, b] : suffix_lengths) {
                    int j = start + a;
                    if (j > cut && b >= 2) runs.push_back({j - i, b, i, i});
                }
            }
        }
        solve_known(move(runs));
        return;
    }
    solve_small(left, right);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 4) return 0;
    solve_large(1, n);
    return 0;
}
