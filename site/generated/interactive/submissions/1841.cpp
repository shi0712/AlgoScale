#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using Int = __int128_t;
using UInt = __uint128_t;

struct Line {
    ll m, b;
    ll operator()(ll t) const { return m * t + b; }
};

struct Piece {
    int index;
    ll l, r;
    Line low, high;
};

struct Board {
    ll x, y, m;

    pair<ll, ll> center() const {
        ll d = (5 * m - 1) / 2;
        return {x + d, y + d};
    }

    bool red(ll a, ll b) const {
        a -= x;
        b -= y;
        return a >= 0 && b >= 0 && a < 5 * m && b < 5 * m
            && ((a / m + b / m) % 2 == 0);
    }
};

ll N, start[2];

bool ask(ll x, ll y) {
    cout << "? " << x << ' ' << y << endl;
    string s;
    if (!(cin >> s)) exit(0);
    if (s == "true") return true;
    if (s == "false") return false;
    exit(0);
}

[[noreturn]] void answer(ll x, ll y) {
    cout << "! " << x << ' ' << y << endl;
    exit(0);
}

ll floor_div(ll a, ll b) {
    ll q = a / b;
    return q - (a % b < 0);
}

// Restrict t to the integers on which a(t) <= b(t).
bool restrict_le(ll &l, ll &r, Line a, Line b) {
    ll m = a.m - b.m, d = b.b - a.b;
    if (m > 0) r = min(r, floor_div(d, m));
    else if (m < 0) l = max(l, -floor_div(d, -m));
    else if (d < 0) return false;
    return l <= r;
}

Line plus_one(Line a) {
    ++a.b;
    return a;
}

// A bound of the form v - k*M, where M = 2*t+1.
Line bound(ll v, int k) {
    return {-2LL * k, v - k};
}

template<class F>
void clip(const Piece &p, Line low, Line high, F emit) {
    // Each choice identifies which of the old and new bounds is active.
    for (int i = 0; i < 2; ++i) {
        ll l1 = p.l, r1 = p.r;
        if (i == 0) {
            if (!restrict_le(l1, r1, low, p.low)) continue;
        } else {
            if (!restrict_le(l1, r1, plus_one(p.low), low)) continue;
        }
        Line a = i ? low : p.low;
        for (int j = 0; j < 2; ++j) {
            ll l = l1, r = r1;
            if (j == 0) {
                if (!restrict_le(l, r, p.high, high)) continue;
            } else {
                if (!restrict_le(l, r, plus_one(high), p.high)) continue;
            }
            Line b = j ? high : p.high;
            if (restrict_le(l, r, a, b)) emit(Piece{p.index, l, r, a, b});
        }
    }
}

template<class F>
void filter_piece(const Piece &p, ll q, bool yes, F emit) {
    int first = p.index & 1, last = 4 - first;
    if (yes) {
        for (int k = first; k <= last; k += 2)
            clip(p, bound(q + 1, k + 1), bound(q, k), emit);
    } else {
        clip(p, bound(q + 1, first), {0, N}, emit);
        for (int k = first + 1; k < last; k += 2)
            clip(p, bound(q + 1, k + 1), bound(q, k), emit);
        clip(p, {0, 1}, bound(q, last + 1), emit);
    }
}

auto shape(const Piece &p) {
    return tuple(p.index, p.low.m, p.low.b, p.high.m, p.high.b);
}

void normalize(vector<Piece> &v) {
    sort(v.begin(), v.end(), [](const Piece &a, const Piece &b) {
        if (shape(a) != shape(b)) return shape(a) < shape(b);
        return a.l < b.l;
    });
    size_t w = 0;
    for (auto p : v) {
        if (w && shape(v[w - 1]) == shape(p) && p.l <= v[w - 1].r + 1)
            v[w - 1].r = max(v[w - 1].r, p.r);
        else
            v[w++] = p;
    }
    v.resize(w);
}

Int mass(const Piece &a, const Piece &b, ll l, ll r) {
    if (l > r) return 0;
    Int n = r - l + 1;
    Int sa = a.high.m - a.low.m, sb = b.high.m - b.low.m;
    Int wa = a.high(l) - a.low(l) + 1, wb = b.high(l) - b.low(l) + 1;
    return n * wa * wb
        + n * (n - 1) / 2 * (sa * wb + sb * wa)
        + n * (n - 1) * (2 * n - 1) / 6 * sa * sb;
}

class Model {
public:
    struct Match {
        int x, y;
        ll l, r;
        Int prefix;
    };

    vector<Piece> v[2];
    vector<Match> matches;
    Int total;
    ll tlo, thi, dlo[4], dhi[4], clo[2], chi[2];

    Model() {
        for (int a = 0; a < 2; ++a) {
            for (int i = 0; i < 5; ++i) {
                Piece p{i, 1, (N / 5 - 1) / 2, {0, 1}, bound(N + 1, 5)};
                clip(p, bound(start[a] + 1, i + 1), bound(start[a], i),
                     [&](Piece s) { v[a].push_back(s); });
            }
        }
        refresh();
    }

    void prune(int axis) {
        vector<pair<ll, ll>> ranges[2];
        for (const auto &p : v[axis ^ 1])
            ranges[p.index & 1].emplace_back(p.l, p.r);
        for (auto &r : ranges) {
            sort(r.begin(), r.end());
            size_t w = 0;
            for (auto s : r) {
                if (w && s.first <= r[w - 1].second + 1)
                    r[w - 1].second = max(r[w - 1].second, s.second);
                else
                    r[w++] = s;
            }
            r.resize(w);
        }
        vector<Piece> next;
        for (auto p : v[axis]) {
            const auto &r = ranges[p.index & 1];
            auto it = lower_bound(r.begin(), r.end(), p.l,
                [](const pair<ll, ll> &s, ll t) { return s.second < t; });
            for (; it != r.end() && it->first <= p.r; ++it) {
                Piece s = p;
                s.l = max(p.l, it->first);
                s.r = min(p.r, it->second);
                next.push_back(s);
            }
        }
        normalize(next);
        v[axis].swap(next);
    }

    void refresh() {
        prune(0);
        prune(1);
        matches.clear();
        total = 0;
        tlo = LLONG_MAX;
        thi = 0;
        fill(dlo, dlo + 4, LLONG_MAX);
        fill(dhi, dhi + 4, 0);
        fill(clo, clo + 2, LLONG_MAX);
        fill(chi, chi + 2, 0);

        for (int i = 0; i < (int)v[0].size(); ++i) {
            const Piece &p = v[0][i];
            for (int j = 0; j < (int)v[1].size(); ++j) {
                const Piece &q = v[1][j];
                if ((p.index ^ q.index) & 1) continue;
                ll l = max(p.l, q.l), r = min(p.r, q.r);
                if (l > r) continue;
                total += mass(p, q, l, r);
                matches.push_back({i, j, l, r, total});
                tlo = min(tlo, l);
                thi = max(thi, r);
                for (int a = 0; a < 2; ++a) {
                    const Piece &s = a == 0 ? p : q;
                    for (ll t : {l, r}) {
                        ll m = 2 * t + 1, lo = s.low(t), hi = s.high(t);
                        int d = 2 * a;
                        dlo[d] = min(dlo[d], start[a] + 1 - hi - s.index * m);
                        dhi[d] = max(dhi[d], start[a] + 1 - lo - s.index * m);
                        dlo[d + 1] = min(dlo[d + 1], lo + (s.index + 1) * m - start[a]);
                        dhi[d + 1] = max(dhi[d + 1], hi + (s.index + 1) * m - start[a]);
                        clo[a] = min(clo[a], lo + 5 * t + 2);
                        chi[a] = max(chi[a], hi + 5 * t + 2);
                    }
                }
            }
        }
        if (total == 0) exit(0);
    }

    Int yes_mass(int axis, ll coord) const {
        if (coord < 1 || coord > N) return 0;
        if (coord == start[axis]) return total;
        vector<vector<Piece>> cut(v[axis].size());
        for (size_t i = 0; i < cut.size(); ++i)
            filter_piece(v[axis][i], coord, true,
                         [&](Piece p) { cut[i].push_back(p); });
        Int ret = 0;
        for (const auto &m : matches) {
            const auto &parts = cut[axis == 0 ? m.x : m.y];
            const Piece &other = axis == 0 ? v[1][m.y] : v[0][m.x];
            for (const Piece &p : parts)
                ret += mass(p, other, max(p.l, m.l), min(p.r, m.r));
        }
        return ret;
    }

    bool probe(int axis, ll coord, Int yes) {
        if (yes == 0) return false;
        if (yes == total) return true;
        bool res = ask(axis == 0 ? coord : start[0], axis == 1 ? coord : start[1]);
        vector<Piece> next;
        for (const auto &p : v[axis])
            filter_piece(p, coord, res, [&](Piece q) { next.push_back(q); });
        v[axis].swap(next);
        refresh();
        return res;
    }

    vector<Board> enumerate() const {
        vector<Board> boards;
        for (const auto &m : matches) {
            const Piece &p = v[0][m.x], &q = v[1][m.y];
            for (ll t = m.l; t <= m.r; ++t)
                for (ll x = p.low(t); x <= p.high(t); ++x)
                    for (ll y = q.low(t); y <= q.high(t); ++y)
                        boards.push_back({x, y, 2 * t + 1});
        }
        return boards;
    }

    Board sample(mt19937_64 &rng) const {
        UInt bits = UInt(rng()) << 64;
        bits |= rng();
        Int rank = bits % UInt(total);
        auto it = upper_bound(matches.begin(), matches.end(), rank,
            [](Int r, const Match &m) { return r < m.prefix; });
        if (it != matches.begin()) rank -= prev(it)->prefix;
        const Piece &p = v[0][it->x], &q = v[1][it->y];
        ll l = it->l, r = it->r;
        while (l < r) {
            ll mid = (l + r) / 2;
            if (mass(p, q, it->l, mid) > rank) r = mid;
            else l = mid + 1;
        }
        ll x = p.low(l), y = q.low(l);
        x += rng() % (p.high(l) - x + 1);
        y += rng() % (q.high(l) - y + 1);
        return {x, y, 2 * l + 1};
    }
};

pair<ll, ll> small_query(const vector<Board> &b, const vector<ll> &xs,
                         const vector<ll> &ys) {
    int n = b.size(), full = (1 << n) - 1;
    vector<pair<ll, ll>> point(1 << n, {0, 0});
    vector<int> masks;
    for (ll x : xs) if (x <= N) {
        for (ll y : ys) if (y <= N) {
            int mask = 0;
            for (int j = 0; j < n; ++j)
                if (b[j].red(x, y)) mask |= 1 << j;
            if (!(mask & 1)) mask ^= full;
            if (mask != full && point[mask].first == 0) {
                point[mask] = {x, y};
                masks.push_back(mask);
            }
        }
    }

    vector<int> same(n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (b[i].center() == b[j].center()) same[i] |= 1 << j;

    vector<pair<int, int>> dp(1 << n, {0, 0});
    vector<int> move(1 << n);
    for (int s = 1; s <= full; ++s) {
        if ((s & same[__builtin_ctz((unsigned)s)]) == s) continue;
        dp[s] = {n + 1, INT_MAX};
        for (int mask : masks) {
            int a = s & mask, c = s ^ a;
            if (!a || !c) continue;
            pair<int, int> value = {
                1 + max(dp[a].first, dp[c].first),
                __builtin_popcount((unsigned)s) + dp[a].second + dp[c].second
            };
            if (value < dp[s]) {
                dp[s] = value;
                move[s] = mask;
            }
        }
    }
    return point[move[full]];
}

[[noreturn]] void finish(vector<Board> boards) {
    while (true) {
        auto c = boards.front().center();
        if (all_of(boards.begin(), boards.end(),
                   [&](const Board &b) { return b.center() == c; }))
            answer(c.first, c.second);

        vector<ll> xs{1, N + 1}, ys{1, N + 1};
        for (const auto &b : boards)
            for (int k = 0; k <= 5; ++k) {
                xs.push_back(b.x + k * b.m);
                ys.push_back(b.y + k * b.m);
            }
        sort(xs.begin(), xs.end());
        sort(ys.begin(), ys.end());
        xs.erase(unique(xs.begin(), xs.end()), xs.end());
        ys.erase(unique(ys.begin(), ys.end()), ys.end());

        pair<ll, ll> q;
        int n = boards.size();
        if (n <= 10) {
            q = small_query(boards, xs, ys);
        } else {
            int h = xs.size(), w = ys.size();
            vector<int> diff(h * w);
            for (const auto &b : boards) {
                int xi[6], yi[6];
                for (int k = 0; k <= 5; ++k) {
                    xi[k] = lower_bound(xs.begin(), xs.end(), b.x + k * b.m) - xs.begin();
                    yi[k] = lower_bound(ys.begin(), ys.end(), b.y + k * b.m) - ys.begin();
                }
                for (int i = 0; i < 5; ++i)
                    for (int j = i & 1; j < 5; j += 2) {
                        ++diff[xi[i] * w + yi[j]];
                        --diff[xi[i + 1] * w + yi[j]];
                        --diff[xi[i] * w + yi[j + 1]];
                        ++diff[xi[i + 1] * w + yi[j + 1]];
                    }
            }
            int best = -1;
            for (int i = 0; i + 1 < h; ++i) {
                int row = 0;
                for (int j = 0; j + 1 < w; ++j) {
                    int &f = diff[i * w + j];
                    row += f;
                    f = row + (i ? diff[(i - 1) * w + j] : 0);
                    int balance = min(f, n - f);
                    if (balance > best) {
                        best = balance;
                        q = {xs[i], ys[j]};
                    }
                }
                if (2 * best == n || best == n / 2) break;
            }
        }

        bool yes = ask(q.first, q.second);
        erase_if(boards, [&](const Board &b) { return b.red(q.first, q.second) != yes; });
        if (boards.empty()) exit(0);
    }
}

struct Query {
    int axis;
    ll pos;
    Int yes;
    Int balance(Int total) const { return min(yes, total - yes); }
};

Query improve(const Model &model, Query best, mt19937_64 &rng) {
    if (best.balance(model.total) * 100 >= model.total * 48) return best;
    if (model.matches.size() > 2500) return best;

    constexpr int S = 72;
    vector<pair<ll, int>> events[2];
    for (int i = 0; i < S; ++i) {
        Board b = model.sample(rng);
        for (int a = 0; a < 2; ++a) {
            ll p = a == 0 ? b.x : b.y;
            int parity = ((start[a] - p) / b.m) & 1;
            for (int k = parity; k < 5; k += 2) {
                events[a].emplace_back(p + k * b.m, 1);
                events[a].emplace_back(p + (k + 1) * b.m, -1);
            }
        }
    }

    vector<tuple<int, int, ll>> trials;
    for (int a = 0; a < 2; ++a) {
        auto &e = events[a];
        sort(e.begin(), e.end());
        int count = 0;
        for (size_t i = 0, j; i < e.size(); i = j) {
            ll p = e[i].first;
            for (j = i; j < e.size() && e[j].first == p; ++j) count += e[j].second;
            if (j < e.size() && count > 0 && count < S)
                trials.emplace_back(abs(2 * count - S), a, (p + e[j].first - 1) / 2);
        }
    }
    sort(trials.begin(), trials.end());
    int attempts = 0;
    for (auto [score, a, p] : trials) {
        if (a == best.axis && p == best.pos) continue;
        if (attempts++ == 22) break;
        Query q{a, p, model.yes_mass(a, p)};
        Int split = q.balance(model.total);
        if (split * 100 >= model.total * 42 &&
            split * 100 > best.balance(model.total) * 103)
            best = q;
        if (best.balance(model.total) * 1000 >= model.total * 497) break;
    }
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N >> start[0] >> start[1])) return 0;

    Model model;
    constexpr int ENUM_LIMIT = 220;

    auto check_finish = [&]() {
        if (model.clo[0] == model.chi[0] && model.clo[1] == model.chi[1])
            answer(model.clo[0], model.clo[1]);
        if (model.total <= ENUM_LIMIT) finish(model.enumerate());
    };

    // Get a constant-factor interval for M without walking cell by cell,
    // or spending logarithmically many probes locating the scale.
    int round = 0;
    while (true) {
        check_finish();
        ll lo = 2 * model.tlo + 1, hi = 2 * model.thi + 1;
        if (hi <= 14 * lo) break;

        ll d = (ll)sqrtl(2.5L * lo * hi);
        if (round == 0 && lo == 3) d = 3;
        else if (round == 1 && lo <= 5) d = min(d, 27LL);
        ++round;

        Query q[4];
        for (int i = 0; i < 4; ++i) {
            int a = i / 2;
            ll p = start[a] + ((i & 1) ? d : -d);
            q[i] = {a, p, model.yes_mass(a, p)};
        }

        int first = 0;
        for (int i = 1; i < 4; ++i) {
            auto rank = [&](int j) {
                return tuple(q[j].yes == model.total, q[j ^ 1].yes == 0,
                             q[j].yes);
            };
            if (rank(i) > rank(first)) first = i;
        }
        if (model.probe(q[first].axis, q[first].pos, q[first].yes)) continue;
        check_finish();
        if (2 * model.thi + 1 <= 14 * (2 * model.tlo + 1)) break;
        Query other = q[first ^ 1];
        model.probe(other.axis, other.pos, model.yes_mass(other.axis, other.pos));
    }

    mt19937_64 rng(0x53a9f1ce28e0d144ULL);
    int first_edge = min_element(model.dhi, model.dhi + 4) - model.dhi;
    for (int i = 0; i < 4; ++i)
        if (model.dhi[i] - model.dlo[i] < model.dhi[first_edge] - model.dlo[first_edge])
            first_edge = i;

    while (true) {
        check_finish();
        ll lo = 2 * model.tlo + 1;
        int dir = first_edge;
        ll offset = 0;

        if (model.tlo != model.thi && model.dlo[dir] == model.dhi[dir]) {
            // Once one edge is known, the opposite distance determines M.
            ll tm = min((model.tlo + model.thi) / 2, lo - 1);
            offset = (2 * tm + 1) + 1 - model.dlo[dir];
            dir ^= 1;
        } else {
            if (model.tlo == model.thi) {
                dir = -1;
                for (int i = 0; i < 4; ++i) {
                    ll spread = model.dhi[i] - model.dlo[i];
                    if (spread && (dir == -1 || spread < model.dhi[dir] - model.dlo[dir]))
                        dir = i;
                }
                if (dir < 0) finish(model.enumerate());
            }
            // Before d + M, leaving the starting square cannot reach
            // another red square on the same line.
            offset = min((model.dlo[dir] + model.dhi[dir]) / 2,
                         model.dlo[dir] + lo - 1);
        }

        int axis = dir / 2;
        ll pos = start[axis] + ((dir & 1) ? offset : -offset);
        Query q = improve(model, {axis, pos, model.yes_mass(axis, pos)}, rng);
        model.probe(q.axis, q.pos, q.yes);
    }
}
