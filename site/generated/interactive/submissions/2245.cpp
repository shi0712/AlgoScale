#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Interval {
    int type; // 0: both positive, 1: both negative, 2: one of each
    ll l, r;
};

struct Candidate {
    int type;
    ll p;
};

static ll N, M, V;
static const ll SMALL_LIMIT = 128;

static ll norm(ll x) {
    x %= N;
    if (x < 0) x += N;
    return x;
}

static ll cycleDist(ll a, ll b) {
    ll d = llabs(a - b);
    return min(d, N - d);
}

static pair<ll, ll> candidatePositions(int type, ll p) {
    ll q = V - p;
    if (type == 0) return {p, q};
    if (type == 1) return {norm(-p), norm(-q)};
    return {p, norm(-q)};
}

static ll responseFor(int type, ll p, ll x) {
    auto [a, b] = candidatePositions(type, p);
    return cycleDist(x, a) + cycleDist(x, b);
}

static ll intervalLength(const Interval &iv) {
    return iv.r - iv.l + 1;
}

static ll candidateCount(const vector<Interval> &state) {
    ll total = 0;
    for (const auto &iv : state) total += intervalLength(iv);
    return total;
}

static vector<Interval> normalizeIntervals(vector<Interval> v) {
    sort(v.begin(), v.end(), [](const Interval &a, const Interval &b) {
        if (a.type != b.type) return a.type < b.type;
        if (a.l != b.l) return a.l < b.l;
        return a.r < b.r;
    });
    vector<Interval> out;
    for (const auto &iv : v) {
        if (iv.l > iv.r) continue;
        if (!out.empty() && out.back().type == iv.type &&
            iv.l <= out.back().r + 1) {
            out.back().r = max(out.back().r, iv.r);
        } else {
            out.push_back(iv);
        }
    }
    return out;
}

static vector<Interval> initialState() {
    vector<Interval> state;

    ll half = (V - 1) / 2;

    ll ppL = max(0LL, V - M);
    ll ppR = min({M, V, half});
    if (ppL <= ppR) state.push_back({0, ppL, ppR});

    ll nnL = max(1LL, V - M);
    ll nnR = min({M, V - 1, half});
    if (nnL <= nnR) state.push_back({1, nnL, nnR});

    ll pnL = max(0LL, V - M);
    ll pnR = min(M, V - 1);
    if (pnL <= pnR) state.push_back({2, pnL, pnR});

    return normalizeIntervals(state);
}

static void queryToSideRadius(ll x, int &side, ll &k) {
    if (x <= M) {
        side = 0;
        k = x;
    } else {
        side = 1;
        k = N - x;
    }
}

static vector<pair<ll, ll>> partitionInterval(const Interval &iv, ll x) {
    int side;
    ll k;
    queryToSideRadius(x, side, k);

    vector<ll> cuts = {iv.l, iv.r + 1};
    auto addCut = [&](ll c) {
        if (c > iv.l && c < iv.r + 1) cuts.push_back(c);
    };

    ll critical[] = {k, V - k, M - k, V - M + k};
    for (ll c : critical) {
        addCut(c);
        addCut(c + 1);
    }

    sort(cuts.begin(), cuts.end());
    cuts.erase(unique(cuts.begin(), cuts.end()), cuts.end());

    vector<pair<ll, ll>> segments;
    for (int i = 0; i + 1 < (int)cuts.size(); ++i) {
        ll a = cuts[i];
        ll b = cuts[i + 1] - 1;
        if (a <= b) segments.push_back({a, b});
    }
    return segments;
}

static vector<Interval> refineState(const vector<Interval> &state, ll x, ll response) {
    vector<Interval> result;

    for (const auto &iv : state) {
        auto segments = partitionInterval(iv, x);

        auto addInterval = [&](ll a, ll b) {
            if (a <= b) result.push_back({iv.type, a, b});
        };

        auto solve = [&](auto &&self, ll a, ll b) -> void {
            if (a > b) return;

            if (b - a <= 2) {
                for (ll p = a; p <= b; ++p) {
                    if (responseFor(iv.type, p, x) == response) {
                        addInterval(p, p);
                    }
                }
                return;
            }

            ll fa = responseFor(iv.type, a, x);
            ll fb = responseFor(iv.type, b, x);
            ll len = b - a;
            ll mid = (a + b) / 2;
            ll diff = fb - fa;

            bool linear = (diff % len == 0);
            ll slope = 0;
            if (linear) {
                slope = diff / len;
                if (slope < -2 || slope > 2) linear = false;
            }
            if (linear &&
                responseFor(iv.type, mid, x) != fa + slope * (mid - a)) {
                linear = false;
            }

            if (!linear) {
                self(self, a, mid);
                self(self, mid + 1, b);
                return;
            }

            if (slope == 0) {
                if (fa == response) addInterval(a, b);
                return;
            }

            ll numerator = response - fa;
            if (numerator % slope != 0) return;
            ll p = a + numerator / slope;
            if (p >= a && p <= b && responseFor(iv.type, p, x) == response) {
                addInterval(p, p);
            }
        };

        for (auto [a, b] : segments) solve(solve, a, b);
    }

    return normalizeIntervals(result);
}

static vector<Candidate> materialize(const vector<Interval> &state) {
    vector<Candidate> candidates;
    for (const auto &iv : state) {
        for (ll p = iv.l; p <= iv.r; ++p) {
            candidates.push_back({iv.type, p});
        }
    }
    return candidates;
}

static bool queryIsUseful(const vector<Interval> &state, ll x) {
    bool haveValue = false;
    ll commonValue = 0;

    for (const auto &iv : state) {
        for (auto [a, b] : partitionInterval(iv, x)) {
            ll fa = responseFor(iv.type, a, x);
            ll fb = responseFor(iv.type, b, x);
            if (fa != fb) return true;
            if (!haveValue) {
                haveValue = true;
                commonValue = fa;
            } else if (commonValue != fa) {
                return true;
            }
        }
    }
    return false;
}

static ll upperBucketScore(const vector<Interval> &state, ll x) {
    map<ll, ll> constantBuckets;
    ll linearPieces = 0;

    for (const auto &iv : state) {
        for (auto [a, b] : partitionInterval(iv, x)) {
            ll fa = responseFor(iv.type, a, x);
            ll fb = responseFor(iv.type, b, x);
            if (a == b || fa == fb) {
                constantBuckets[fa] += b - a + 1;
            } else {
                ++linearPieces;
            }
        }
    }

    ll largestConstant = 0;
    for (const auto &[value, count] : constantBuckets) {
        largestConstant = max(largestConstant, count);
    }
    return linearPieces + largestConstant;
}

static ll upperBucketScore(const Interval &iv, ll x) {
    vector<Interval> single = {iv};
    return upperBucketScore(single, x);
}

static void addTargetQueries(ll target, set<ll> &queries) {
    ll radii[] = {
        target,
        V - target,
        M - target,
        target - V + M
    };

    for (ll k : radii) {
        if (k < 0 || k > M) continue;
        queries.insert(k);
        queries.insert(norm(-k));
    }
}

static ll askRaw(ll x) {
    cout << "? " << x + 1 << '\n' << flush;
    ll response;
    if (!(cin >> response) || response < 0) exit(0);
    return response;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N)) return 0;
    M = (N - 1) / 2;

    set<ll> queried;
    vector<Interval> state;

    auto answer = [&]() {
        Candidate found{-1, 0};
        for (const auto &iv : state) {
            for (ll p = iv.l; p <= iv.r; ++p) {
                found = {iv.type, p};
            }
        }
        auto [a, b] = candidatePositions(found.type, found.p);
        cout << "! " << a + 1 << " " << b + 1 << '\n' << flush;
        exit(0);
    };

    auto ask = [&](ll x) {
        ll response = askRaw(x);
        queried.insert(x);
        state = refineState(state, x, response);
    };

    V = askRaw(0);
    queried.insert(0);
    state = initialState();

    if (candidateCount(state) == 1) answer();

    const ll k0 = (V + 1) / 2;
    bool secondDone = false;
    bool thirdConsidered = false;

    auto smallStep = [&]() {
        vector<Candidate> candidates = materialize(state);
        int sz = (int)candidates.size();

        set<ll> options;
        auto addOption = [&](ll x) {
            x = norm(x);
            if (!queried.count(x)) options.insert(x);
        };

        for (const auto &cand : candidates) {
            auto [a, b] = candidatePositions(cand.type, cand.p);
            for (ll e : {a, b}) {
                addOption(e);
                addOption(e + 1);
                addOption(e - 1);
                addOption(e + M);
                addOption(e + M + 1);
            }
            addTargetQueries(cand.p, options);
            addTargetQueries(V - cand.p, options);
        }

        for (const auto &iv : state) {
            addTargetQueries((iv.l + iv.r) / 2, options);
        }
        addTargetQueries(k0, options);
        addOption(M);
        addOption(M + 1);

        if (N <= 400) {
            for (ll x = 0; x < N; ++x) addOption(x);
        }

        ll bestX = -1;
        int bestMaximum = INT_MAX;
        ll bestSquares = LLONG_MAX;

        for (ll x : options) {
            map<ll, int> buckets;
            for (const auto &cand : candidates) {
                ++buckets[responseFor(cand.type, cand.p, x)];
            }

            int maximum = 0;
            ll squares = 0;
            for (const auto &[value, count] : buckets) {
                maximum = max(maximum, count);
                squares += 1LL * count * count;
            }

            if (maximum == sz) continue;
            if (maximum < bestMaximum ||
                (maximum == bestMaximum && squares < bestSquares)) {
                bestMaximum = maximum;
                bestSquares = squares;
                bestX = x;
            }
        }

        if (bestX == -1) {
            for (ll x = 0; x < N; ++x) {
                if (queried.count(x)) continue;
                if (queryIsUseful(state, x)) {
                    bestX = x;
                    break;
                }
            }
        }

        ask(bestX);
    };

    while (candidateCount(state) > 1) {
        ll total = candidateCount(state);

        if (total <= SMALL_LIMIT) {
            smallStep();
            continue;
        }

        if (!secondDone) {
            secondDone = true;
            ask(k0);
            continue;
        }

        if (!thirdConsidered) {
            thirdConsidered = true;
            ll x = norm(-k0);
            if (!queried.count(x) && queryIsUseful(state, x)) {
                ask(x);
                continue;
            }
        }

        Interval target = state[0];
        for (const auto &iv : state) {
            if (intervalLength(iv) > intervalLength(target)) target = iv;
        }

        ll mid = (target.l + target.r) / 2;
        set<ll> options;
        for (ll t = mid - 2; t <= mid + 2; ++t) addTargetQueries(t, options);
        addTargetQueries(target.l, options);
        addTargetQueries(target.l + 1, options);
        addTargetQueries(target.r - 1, options);
        addTargetQueries(target.r, options);
        addTargetQueries(k0, options);

        for (const auto &iv : state) {
            addTargetQueries((iv.l + iv.r) / 2, options);
        }

        ll bestX = -1;
        ll bestTargetScore = LLONG_MAX;
        ll bestGlobalScore = LLONG_MAX;

        ll anyX = -1;
        ll anyScore = LLONG_MAX;

        for (ll x : options) {
            if (queried.count(x)) continue;

            ll globalScore = upperBucketScore(state, x);
            if (globalScore < anyScore) {
                anyScore = globalScore;
                anyX = x;
            }

            ll targetScore = upperBucketScore(target, x);
            if (targetScore >= intervalLength(target)) continue;

            if (targetScore < bestTargetScore ||
                (targetScore == bestTargetScore &&
                 globalScore < bestGlobalScore)) {
                bestTargetScore = targetScore;
                bestGlobalScore = globalScore;
                bestX = x;
            }
        }

        if (bestX == -1 && anyX != -1 && anyScore < total) bestX = anyX;

        if (bestX == -1) {
            for (ll x : options) {
                if (!queried.count(x) && queryIsUseful(state, x)) {
                    bestX = x;
                    break;
                }
            }
        }

        ask(bestX);
    }

    answer();
}
