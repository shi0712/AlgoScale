#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
using i128 = __int128_t;
constexpr int N = 100000;
constexpr int tilt = 101;
constexpr int tiltInverse = 9901;
static_assert(tilt * tiltInverse % N == 1);
constexpr double eps = 1.001e-6;

struct Disk {
    int x, y, r;
};

struct Probe {
    int x, y, X, Y;
    i64 dx, dy, norm;
    double invLength;
    bool full;

    Probe(int x, int y, int X, int Y) : x(x), y(y), X(X), Y(Y),
        dx(X - x), dy(Y - y), norm(dx * dx + dy * dy),
        invLength(1.0 / sqrt(double(norm))),
        full((x == 0 || x == N || y == 0 || y == N) &&
             (X == 0 || X == N || Y == 0 || Y == N)) {}

    Probe transposed() const { return Probe(y, x, Y, X); }

    double length(const Disk& d) const {
        i64 wx = d.x - x, wy = d.y - y;
        i64 cross = dx * wy - dy * wx;
        i128 disc = i128(d.r) * d.r * norm - i128(cross) * cross;
        if (disc <= 0) return 0;
        double half = sqrt(double(disc));
        if (full) return 2 * half * invLength;
        double along = dx * wx + dy * wy;
        return max(0.0, min(double(norm), along + half) - max(0.0, along - half)) * invLength;
    }

    double ask() const {
        cout << "query " << x << ' ' << y << ' ' << X << ' ' << Y << endl;
        double v;
        if (!(cin >> v) || !(v >= -eps)) exit(0);
        return v;
    }
};

// The 500 intercepts start at 99, end at 99800, and have gaps at
// most 200. At every permissible x their strips cover [100, 99900].
vector<int> searchOrder() {
    priority_queue<pair<int, int>> gaps;
    gaps.emplace(501, -1);
    vector<int> order;
    while (!gaps.empty()) {
        auto [width, left] = gaps.top();
        gaps.pop();
        if (width < 2) break;
        int mid = width == 501 ? 250 : left + width / 2;
        int b = mid * 200 + (mid < 250 ? 99 : 0);
        if (mid != 0 && mid != 499) order.push_back(b);
        gaps.emplace(mid - left, left);
        gaps.emplace(left + width - mid, mid);
    }
    // Test the outer strips early to cover disks tangent to a side.
    order.insert(order.begin() + 7, {99, 99800});
    return order;
}

vector<Disk> candidates(const Probe& line, double v, int lower, int upper,
                        const vector<Probe>& otherMisses) {
    int b = line.y;
    long double norm = line.norm;
    long double lo = max(0.0, v - eps) / 2, hi = (v + eps) / 2;
    vector<Disk> result;
    // A disk hitting b fits between the nearest misses (or the box walls).
    int maxR = min(N / 2, (upper - lower + (lower == 0 ? tilt : 0)) / 2 + 1);
    for (int r = max(100, int(floor(lo))); r <= maxR; ++r) {
        long double rr = i64(r) * r;
        if (rr < lo * lo) continue;
        i64 first = max(1LL, i64(sqrtl(max(0.L, rr - hi * hi) * norm)) - 1);
        i64 last = i64(sqrtl(max(0.L, rr - lo * lo) * norm)) + 1;
        for (i64 a = first; a <= last; ++a) {
            // The signed cross product is N*(y-b)-dy*x. All search
            // slopes have dy congruent to +tilt or -tilt modulo N.
            for (i64 cross : {a, -a}) {
                i64 residue = (line.dy == N - tilt) ? cross : -cross;
                int x = int(residue * tiltInverse % N);
                if (x < 0) x += N;
                if (x < r || x > N - r) continue;
                int y = int(b + (cross + line.dy * x) / N);
                if (y < r || y > N - r) continue;
                Disk d{x, y, r};
                if (abs(line.length(d) - v) > eps) continue;
                auto hits = [&](int other) {
                    i64 c = cross + i64(b - other) * N;
                    return i128(c) * c < i128(r) * r * line.norm;
                };
                if (lower > 0 && hits(lower)) continue;
                if (upper < N && hits(upper)) continue;
                if (any_of(otherMisses.begin(), otherMisses.end(),
                           [&](const Probe& p) { return p.length(d) > eps; })) continue;
                result.push_back(d);
            }
        }
    }
    return result;
}

// A unit segment that gives length one on one disk and zero on the other.
Probe separate(Disk a, Disk b) {
    for (auto [dx, dy] : {pair{1,0}, {-1,0}, {0,1}, {0,-1}}) {
        int ea = dx * a.x + dy * a.y + a.r;
        int eb = dx * b.x + dy * b.y + b.r;
        if (ea == eb) continue;
        Disk d = ea > eb ? a : b;
        int x = d.x + dx * d.r, y = d.y + dy * d.r;
        return Probe(x, y, x - dx, y - dy);
    }
    abort();
}

Probe chooseProbe(const vector<Disk>& ds, const Probe& base, double chord, mt19937& rng) {
    int b = base.y;
    if (ds.size() == 2) return separate(ds[0], ds[1]);
    Probe best = separate(ds.front(), ds.back());
    pair<int, i64> bestScore{int(ds.size()) + 1, 0};
    vector<double> values(ds.size());
    // Readings within 2*eps can share a reported value. Minimize the
    // largest surviving group, breaking ties by the number of close pairs.
    auto consider = [&](const Probe& p) {
        for (size_t i = 0; i < ds.size(); ++i) values[i] = p.length(ds[i]);
        sort(values.begin(), values.end());
        int width = 1, left = 0;
        i64 pairs = 0;
        for (int i = 1; i < int(values.size()); ++i) {
            while (values[i] - values[left] > 2 * eps) ++left;
            width = max(width, i - left + 1);
            pairs += i - left;
            if (width > bestScore.first) return false;
        }
        pair<int, i64> score{width, pairs};
        if (score < bestScore) {
            bestScore = score;
            best = p;
        }
        return width == 1;
    };

    // A parallel vertical translation anywhere in [-down, up] still
    // cuts all candidates. Sample rotations on that scale as well.
    double down = N, up = N;
    double scaleY = sqrt(double(base.norm)) / N;
    for (Disk d : ds) {
        double offset = (d.y - b) - double(d.x) * base.dy / N;
        down = min(down, d.r * scaleY - offset);
        up = min(up, d.r * scaleY + offset);
    }
    down = max(0.0, down);
    up = max(0.0, up);
    auto slanted = [&](double a, double z) {
        int y = clamp(int(lround(b + a)), 0, N);
        int Y = clamp(int(lround(base.Y + z)), 0, N);
        return consider(Probe(0, y, N, Y));
    };
    if (slanted(-0.71 * down, 0.89 * up) ||
        slanted(0.83 * up, -0.93 * down)) return best;

    // Cropping one end lets us change the slope by a fraction of 1/N
    // while keeping lattice endpoints. This helps with very short chords.
    auto trimmed = [&](int l, int r) {
        int y = b + lround(double(l) * base.dy / N);
        int Y = base.Y - lround(double(r) * base.dy / N);
        return consider(Probe(l, y, N - r, Y));
    };
    for (int trim : {157, 1901, 12379, 17, 990, 9901, 31711}) {
        if (trimmed(trim, 0) || trimmed(0, trim)) return best;
    }
    for (int k = 0; k < 144; ++k) {
        auto uniform = [&]() { return generate_canonical<double, 32>(rng); };
        double a, z;
        switch (k % 4) {
            case 0:
                a = -down + uniform() * (down + up);
                z = -down + uniform() * (down + up);
                break;
            case 1: {
                double scale = pow(max(2.0, chord * 0.45), uniform());
                a = (2 * uniform() - 1) * scale;
                z = (2 * uniform() - 1) * scale;
                break;
            }
            case 2: {
                int l = int(pow(40001.0, uniform()));
                int r = int(pow(40001.0, uniform()));
                if (trimmed(l, r)) return best;
                continue;
            }
            default: {
                Disk d = ds[rng() % ds.size()];
                a = (d.y - b) - double(d.x) * base.dy / N + (uniform() - 0.5) * d.r;
                z = a + (2 * uniform() - 1) * d.r;
                break;
            }
        }
        if (slanted(a, z)) return best;
    }
    if (bestScore.first >= int(ds.size())) return separate(ds.front(), ds.back());
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    set<int> misses{0, N};
    mt19937 rng(548271);
    struct SearchLine { Probe base; bool swapped; };
    vector<SearchLine> lines;
    auto add = [&](int b, int rise, bool swapped = false) {
        lines.push_back({Probe(0, b, N, b + rise), swapped});
    };
    add(32, N - tilt);
    vector<int> order = searchOrder();
    for (size_t i = 0; i < order.size(); ++i) {
        add(order[i], tilt);
        if (i == 0) add(N - 32, tilt - N);
        if (i == 14) {
            add(99, tilt, true);
            add(99800, tilt, true);
        }
    }

    vector<Probe> extraMisses, allMisses;
    for (const auto& [base, swapped] : lines) {
        Probe line = swapped ? base.transposed() : base;
        bool regular = !swapped && base.dy == tilt;
        double v = line.ask();
        // Every positive search chord is at least 2/sqrt(line.norm).
        if (v <= eps) {
            allMisses.push_back(line);
            if (regular) misses.insert(base.y);
            else extraMisses.push_back(line);
            continue;
        }

        int low = 0, high = N;
        vector<Probe> constraints = regular ? extraMisses : allMisses;
        if (regular) {
            auto it = misses.upper_bound(base.y);
            low = *prev(it);
            high = *it;
        }
        if (swapped) {
            for (Probe& p : constraints) p = p.transposed();
        }
        vector<Disk> ds = candidates(base, v, low, high, constraints);
        while (ds.size() > 1) {
            Probe p = chooseProbe(ds, base, v, rng);
            double observed = (swapped ? p.transposed() : p).ask();
            erase_if(ds, [&](Disk d) { return abs(p.length(d) - observed) > eps; });
        }
        if (!ds.empty()) {
            auto [x, y, r] = ds.front();
            if (swapped) swap(x, y);
            cout << "answer " << x << ' ' << y << ' ' << r << endl;
        }
        return 0;
    }
}
