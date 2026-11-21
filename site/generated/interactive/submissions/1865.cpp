#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
using i128 = __int128_t;

constexpr int N = 100000;
constexpr i64 Q = 1LL * N * N + 1;
constexpr i64 SCALE = 10000000;
constexpr i64 DEN = 4LL * SCALE * SCALE;

struct Reading {
    i128 lo, hi;
};

struct Disk {
    int x, y, r;
};

array<bool, N> asked{};

Reading probe(int b) {
    cout << "query 0 " << b << ' ' << N << ' ' << b + 1 << '\n' << flush;

    string s;
    if (!(cin >> s)) exit(0);

    i64 whole = 0, frac = 0, place = SCALE;
    bool negative = s[0] == '-';
    bool dot = false;
    for (int i = (s[0] == '-' || s[0] == '+'); i < (int)s.size(); ++i) {
        char c = s[i];
        if (c == '.' && !dot) {
            dot = true;
        } else if (c >= '0' && c <= '9') {
            if (!dot) {
                whole = whole * 10 + c - '0';
                if (whole > N + 1) exit(0);
            } else {
                place /= 10;
                frac += (c - '0') * place;
            }
        } else {
            exit(0);
        }
    }

    i64 m = whole * SCALE + frac;
    if (negative) m = -m;
    if (m < -10) exit(0);

    // The squared chord length is 4*S/Q. Convert the error
    // interval into exact integer bounds on S.
    i128 a = max<i64>(0, m - 10);
    i128 z = m + 10;
    asked[b] = true;
    return {(a * a * Q + DEN - 1) / DEN, z * z * Q / DEN};
}

void answer(const Disk& d) {
    cout << "answer " << d.x << ' ' << d.y << ' ' << d.r << '\n' << flush;
}

i128 raw(const Disk& d, int b) {
    i64 z = 1LL * N * (d.y - b) - d.x;
    return (i128)d.r * d.r * Q - (i128)z * z;
}

i64 isqrt(i128 n) {
    i64 s = (i64)sqrtl((long double)n);
    while ((i128)s * s > n) --s;
    while ((i128)(s + 1) * (s + 1) <= n) ++s;
    return s;
}

vector<Disk> enumerate(int b, const Reading& v, int below, int above) {
    vector<Disk> ds;
    for (int r = 100; r <= N / 2; ++r) {
        i128 rq = (i128)r * r * Q;
        if (rq < v.lo) continue;

        i128 low = max<i128>(0, rq - v.hi);
        i64 l = isqrt(low);
        if ((i128)l * l < low) ++l;
        i64 h = isqrt(rq - v.lo);

        auto add = [&](i64 z) {
            // z = N*(y-b)-x, with 0 < x < N.
            i64 dy = (z > 0 ? z + N - 1 : z) / N;
            int x = int(dy * N - z);
            int y = b + int(dy);
            if (x < r || x > N - r || y < r || y > N - r) return;

            // The chord is positive exactly when y-r <= b <= y+r-1.
            if (below >= y - r || above <= y + r - 1) return;
            ds.push_back({x, y, r});
        };

        for (i64 z = l; z <= h; ++z) {
            add(z);
            if (z != 0) add(-z);
        }
    }
    return ds;
}

bool same_yr(const vector<Disk>& ds) {
    for (const Disk& d : ds) {
        if (d.y != ds.front().y || d.r != ds.front().r) return false;
    }
    return true;
}

void tangent_answer(int y, int r) {
    // Here the length decreases strictly with x. Consecutive
    // lattice x coordinates give lengths separated by > 2e-6.
    int b = y + r - 1;
    Reading v = probe(b);
    int l = r, h = N - r;
    while (l < h) {
        int m = (l + h) / 2;
        if (raw({m, y, r}, b) > v.hi) l = m + 1;
        else h = m;
    }
    answer({l, y, r});
}

void filter(vector<Disk>& ds, int b, const Reading& v) {
    erase_if(ds, [&](const Disk& d) {
        i128 s = max<i128>(0, raw(d, b));
        return s < v.lo || s > v.hi;
    });
}

int choose_probe(const vector<Disk>& ds, int b, bool must_hit) {
    int minr = N, commonL = 0, commonR = N - 1;
    int unionL = N - 1, unionR = 0;
    for (const Disk& d : ds) {
        int l = d.y - d.r, h = d.y + d.r - 1;
        minr = min(minr, d.r);
        commonL = max(commonL, l);
        commonR = min(commonR, h);
        unionL = min(unionL, l);
        unionR = max(unionR, h);
    }

    // With this shift bound, a miss implies that the unit shift
    // in the opposite direction from b hits every survivor.
    int lo = must_hit ? commonL : max(unionL, b - 2 * minr + 1);
    int hi = must_hit ? commonR : min(unionR, b + 2 * minr - 1);

    vector<int> options;
    vector<bool> added(N);
    auto add = [&](int p) {
        if (p >= lo && p <= hi && !asked[p] && !added[p]) {
            added[p] = true;
            options.push_back(p);
        }
    };

    add(commonL);
    add(commonR);
    add(b - 1);
    add(b + 1);
    for (int k = 0; k <= 32; ++k) add(lo + (hi - lo) * k / 32);
    for (int k : {1, 2, 3, 4, 6, 8, 12, 15}) {
        add(b - minr * k / 8);
        add(b + minr * k / 8);
    }
    for (int k = 0; k <= 12; ++k) {
        const Disk& d = ds[(ds.size() - 1) * k / 12];
        add(d.y - d.r);
        add(d.y + d.r - 1);
        add(d.y - d.r + 1);
        add(d.y + d.r - 2);
    }
    if (1LL * (hi - lo + 1) * (i64)ds.size() <= 250000) {
        for (int p = lo; p <= hi; ++p) add(p);
    }

    // Prefer a probe whose response resolves as many candidates
    // immediately as possible. Only this ranking uses floating point.
    vector<double> lengths;
    lengths.reserve(ds.size());
    constexpr double close = 2.0001e-6;
    pair<int, int> best{INT_MAX, INT_MAX};
    int pick = -1;
    for (int p : options) {
        lengths.clear();
        int zeros = 0, zy = 0, zr = 0;
        bool mixed = false;

        for (const Disk& d : ds) {
            if (p < d.y - d.r || p > d.y + d.r - 1) {
                if (zeros == 0) {
                    zy = d.y;
                    zr = d.r;
                } else if (zy != d.y || zr != d.r) {
                    mixed = true;
                }
                ++zeros;
            } else {
                lengths.push_back(2.0 * sqrt((double)raw(d, p) / (double)Q));
            }
        }

        sort(lengths.begin(), lengths.end());
        int ambiguous = zeros > 1 ? zeros : 0;
        int worst = zeros, first = 0;
        int count = (int)lengths.size();
        for (int j = 0; j < count; ++j) {
            if ((j > 0 && lengths[j] - lengths[j - 1] <= close) ||
                (j + 1 < count && lengths[j + 1] - lengths[j] <= close)) {
                ++ambiguous;
            }
            while (lengths[j] - lengths[first] > close) ++first;
            worst = max(worst, j - first + 1);
        }

        pair<int, int> score{4 * ambiguous + (mixed ? zeros : 0), worst};
        if (score < best) {
            best = score;
            pick = p;
        }
        if (ambiguous == 0) break;
    }
    return pick;
}

void recover(int b, vector<Disk> ds) {
    if (ds.size() > 1 && !same_yr(ds)) {
        int p = choose_probe(ds, b, false);
        Reading v = probe(p);
        filter(ds, p, v);

        if (v.lo == 0 && ds.size() > 1 && !same_yr(ds)) {
            p = choose_probe(ds, b, true);
            filter(ds, p, probe(p));
        }
    }

    // Two distinct positive chords determine the integer r and y
    // even at the given precision. A near-end chord resolves x.
    if (ds.size() == 1) answer(ds.front());
    else if (!ds.empty()) tangent_answer(ds.front().y, ds.front().r);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // At r=100 the 499 bands cover all center rows except 99900.
    // Split the largest unprobed intervals first to find larger disks early.
    priority_queue<tuple<int, int, int>> gaps;
    gaps.emplace(499, 0, 498);
    set<int> misses{-N, 2 * N};

    while (!gaps.empty()) {
        auto [size, l, h] = gaps.top();
        gaps.pop();
        int m = (l + h) / 2;
        int b = 199 + 200 * m;

        Reading v = probe(b);
        if (v.lo > 0) {
            auto it = misses.lower_bound(b);
            recover(b, enumerate(b, v, *prev(it), *it));
            return 0;
        }

        misses.insert(b);
        if (l < m) gaps.emplace(m - l, l, m - 1);
        if (m < h) gaps.emplace(h - m, m + 1, h);
    }

    // Missing the topmost probed line requires y >= 99800+r
    // in the only remaining band, while y <= 100000-r.
    tangent_answer(99900, 100);
    return 0;
}
