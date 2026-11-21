#include <bits/stdc++.h>
using namespace std;

constexpr double PHI = 1.6180339887498948482;
constexpr double EPS = 1e-8;
constexpr int TABLE_LIMIT = 768;
constexpr int EXACT_LIMIT = 20;

struct Candidate {
    int roll;
    int state; // Bit 0: last answer was a lie. Bit 1: last two had the same honesty.
};

struct Split {
    int l, r;
};

int ask(int l, int r) {
    cout << "? " << l << ' ' << r << endl;
    int hands;
    if (!(cin >> hands) || hands == -1) exit(0);
    int len = r - l + 1;
    if (hands != len && hands != len - 1) exit(0);
    return hands == len - 1;
}

bool nominate(int roll) {
    cout << "! " << roll << endl;
    int reply;
    if (!(cin >> reply) || (reply != 0 && reply != 1)) exit(0);
    return reply == 1;
}

struct Prefix {
    vector<int> c, d;
    int a = 0, f = 0, c0 = 0, d0 = 0;

    explicit Prefix(const vector<Candidate>& v)
        : c(v.size() + 1), d(v.size() + 1) {
        for (int i = 0; i < (int)v.size(); ++i) {
            int s = v[i].state;
            c[i + 1] = c[i];
            d[i + 1] = d[i];
            if (s & 2) {
                ++f;
                d0 += s & 1;
                d[i + 1] += (s & 1) ? -1 : 1;
            } else {
                ++a;
                c0 += !(s & 1);
                c[i + 1] += (s & 1) ? 1 : -1;
            }
        }
    }

    // For answer 0, C flexible candidates repeat their honesty, and
    // D forced candidates can give the answer.
    pair<int, int> counts(int l, int r) const {
        return {c0 + c[r + 1] - c[l], d0 + d[r + 1] - d[l]};
    }
};

class Strategy {
    int bound, stride;
    vector<unsigned char> relaxed;

    struct Proof {
        int bad = -1, good = 100;
    };
    unordered_map<uint64_t, Proof> memo;
    int totalWork = 2200000;
    int turnWork = 0;

    int depth(int a, int f) const {
        return relaxed[a * stride + f];
    }

    void buildRelaxation() {
        // In the relaxation, the query can put each candidate inside or
        // outside independently. prev[f] is the maximum feasible a.
        relaxed.assign(stride * stride, 100);
        vector<int> prev(stride, -1);
        for (int f = 0; f <= 2; ++f) {
            prev[f] = 2 - f;
            for (int a = 0; a <= prev[f]; ++a)
                relaxed[a * stride + f] = 0;
        }

        for (int t = 1; ; ++t) {
            vector<int> next = prev;
            for (int x = 0; x <= bound; ++x) {
                int hi = min(prev[x], bound - x);
                for (int y = x; y <= hi && prev[y] >= x; ++y) {
                    int a = x + y;
                    int f = min(bound - a, prev[x] + prev[y] - a);
                    next[f] = max(next[f], a);
                }
            }
            for (int f = bound - 1; f >= 0; --f)
                next[f] = max(next[f], next[f + 1]);

            bool done = true;
            for (int f = 0; f <= bound; ++f) {
                for (int a = prev[f] + 1; a <= next[f]; ++a)
                    relaxed[a * stride + f] = t;
                if (next[f] != bound - f) done = false;
            }
            if (done) break;
            prev.swap(next);
        }
    }

    static uint64_t canonical(uint64_t code, int m) {
        uint64_t reverseCode = 0;
        uint64_t temp = code;
        for (int i = 0; i < m; ++i) {
            reverseCode = (reverseCode << 2) | (temp & 3);
            temp >>= 2;
        }

        int shift = 2 * (m - 1);
        uint64_t sentinel = 1ULL << (2 * m);
        uint64_t flip = (sentinel - 1) & 0x5555555555555555ULL;
        uint64_t best = sentinel;
        for (int i = 0; i < m; ++i) {
            best = min(best, min(code, code ^ flip));
            best = min(best, min(reverseCode, reverseCode ^ flip));
            code = (code >> 2) | ((code & 3) << shift);
            reverseCode = (reverseCode >> 2) | ((reverseCode & 3) << shift);
        }
        return best | sentinel;
    }

    struct Option {
        int l, r, d0, d1;
        double w0, w1;
    };

    static auto ranking(const Option& o) {
        return tuple(max(o.d0, o.d1), o.d0 + o.d1, max(o.w0, o.w1));
    }

    // Return 1 for a proved strategy, 0 for impossible, -1 on search cutoff.
    int search(uint64_t code, int m, int t, Split* root) {
        if (m <= 2) return 1;
        if (t <= 0) return 0;

        int states[EXACT_LIMIT];
        int c[EXACT_LIMIT + 1] = {}, d[EXACT_LIMIT + 1] = {};
        int a = 0, f = 0, c0 = 0, d0 = 0;
        for (int i = 0; i < m; ++i) {
            int s = states[i] = (code >> (2 * i)) & 3;
            c[i + 1] = c[i];
            d[i + 1] = d[i];
            if (s & 2) {
                ++f;
                d0 += s & 1;
                d[i + 1] += (s & 1) ? -1 : 1;
            } else {
                ++a;
                c0 += !(s & 1);
                c[i + 1] += (s & 1) ? 1 : -1;
            }
        }
        if (depth(a, f) > t) return 0;

        uint64_t key = canonical(code, m);
        auto it = memo.find(key);
        if (it != memo.end()) {
            if (it->second.bad >= t) return 0;
            if (!root && it->second.good <= t) return 1;
        }
        if (m > 5) {
            int cost = m * (m + 1) / 2;
            if (cost > turnWork || cost > totalWork) return -1;
            turnWork -= cost;
            totalWork -= cost;
        }

        Option options[EXACT_LIMIT * (EXACT_LIMIT + 1) / 2];
        int count = 0;
        for (int l = 0; l < m; ++l) {
            // A proper suffix is equivalent to asking its complementary prefix.
            for (int r = l; r < m - (l > 0); ++r) {
                int x = c0 + c[r + 1] - c[l];
                int y = d0 + d[r + 1] - d[l];
                int a0 = a - x + y, f0 = x;
                int a1 = x + f - y, f1 = a - x;
                int b0 = depth(a0, f0), b1 = depth(a1, f1);
                if (max(b0, b1) > t - 1) continue;
                options[count++] = {l, r, b0, b1,
                                    PHI * a0 + f0, PHI * a1 + f1};
            }
        }
        sort(options, options + count, [](const Option& u, const Option& v) {
            return ranking(u) < ranking(v);
        });

        for (int k = 0; k < count; ++k) {
            const auto& o = options[k];
            int first = pair(o.d1, o.w1) > pair(o.d0, o.w0);
            bool success = true;
            for (int b = 0; b < 2; ++b) {
                int answer = first ^ b;
                if ((answer ? o.d1 : o.d0) == 0) continue;
                uint64_t child = 0;
                int size = 0;
                for (int i = 0; i < m; ++i) {
                    int lie = (o.l <= i && i <= o.r) ^ answer;
                    bool repeat = (states[i] & 1) == lie;
                    if (repeat && (states[i] & 2)) continue;
                    child |= uint64_t(lie | (repeat ? 2 : 0)) << (2 * size++);
                }
                int result = search(child, size, t - 1, nullptr);
                if (result < 0) return -1;
                if (result == 0) {
                    success = false;
                    break;
                }
            }
            if (success) {
                auto& proof = memo[key];
                proof.good = min(proof.good, t);
                if (root) *root = {o.l, o.r};
                return 1;
            }
        }
        auto& proof = memo[key];
        proof.bad = max(proof.bad, t);
        return 0;
    }

    static Split weighted(const Prefix& p) {
        int m = p.a + p.f;
        double target = PHI * (PHI * p.a + p.f) / 2
                        - (PHI * (p.a + p.d0) - (PHI - 1) * p.c0);
        set<pair<double, int>> seen;
        seen.emplace(0, 0);
        double best = numeric_limits<double>::infinity();
        Split choice{0, 0};
        for (int j = 1; j <= m; ++j) {
            double w = PHI * p.d[j] - (PHI - 1) * p.c[j];
            double want = w - target;
            auto it = seen.lower_bound({want, -1});
            auto consider = [&](auto pos) {
                double error = abs(pos->first - want);
                if (error < best - EPS) {
                    best = error;
                    choice = {pos->second, j - 1};
                }
            };
            if (it != seen.end()) consider(it);
            if (it != seen.begin()) consider(prev(it));
            seen.emplace(w, j);
            if (best < EPS) break;
        }
        return choice;
    }

public:
    explicit Strategy(int n) : bound(min(n, TABLE_LIMIT)), stride(bound + 1) {
        buildRelaxation();
    }

    Split choose(const vector<Candidate>& v) {
        int m = (int)v.size();
        Prefix p(v);
        if (m <= EXACT_LIMIT) {
            uint64_t code = 0;
            for (int i = 0; i < m; ++i)
                code |= uint64_t(v[i].state) << (2 * i);
            turnWork = 550000;
            for (int t = depth(p.a, p.f); t <= 32; ++t) {
                Split answer;
                int result = search(code, m, t, &answer);
                if (result == 1) return answer;
                if (result < 0) break;
            }
        }

        if (m > bound || m <= 4) return weighted(p);

        // Balance future feasible response histories, using the relaxed
        // endgame to decide which small imbalances are worth accepting.
        double current = PHI * p.a + p.f;
        double ceiling = PHI * current / 2 + 1.15;
        tuple<int, int, double> best{100, 200, 1e100};
        Split answer{-1, -1};
        for (int l = 0; l < m; ++l) {
            for (int r = l; r < m - (l > 0); ++r) {
                auto [x, y] = p.counts(l, r);
                int a0 = p.a - x + y, f0 = x;
                int a1 = x + p.f - y, f1 = p.a - x;
                double worst = max(PHI * a0 + f0, PHI * a1 + f1);
                if (worst > ceiling + EPS || worst >= current - EPS) continue;
                int t0 = depth(a0, f0), t1 = depth(a1, f1);
                auto score = tuple(max(t0, t1), t0 + t1, worst);
                if (score < best) {
                    best = score;
                    answer = {l, r};
                }
            }
        }
        return answer.l >= 0 ? answer : weighted(p);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 3) return 0;

    int first = ask(1, n);
    vector<Candidate> candidates;
    candidates.reserve(n);
    for (int x = 1; x <= n; ++x)
        candidates.push_back({x, first ^ 1});

    Strategy strategy(n);
    while (candidates.size() > 2) {
        if (candidates.size() == 3) {
            int forced[3], cnt = 0;
            for (int i = 0; i < 3; ++i)
                if (candidates[i].state & 2) forced[cnt++] = i;
            if (cnt >= 2) {
                int i = forced[0], j = forced[1];
                if (!nominate(candidates[3 - i - j].roll)) {
                    auto a = candidates[i], b = candidates[j];
                    int r = (a.state == b.state) ? a.roll : b.roll;
                    int answer = ask(a.roll, r);
                    nominate((answer == (a.state & 1)) ? a.roll : b.roll);
                }
                cout << "#" << endl;
                return 0;
            }
        }

        auto [i, j] = strategy.choose(candidates);
        int l = candidates[i].roll, r = candidates[j].roll;
        int answer = ask(l, r);
        int kept = 0;
        for (auto [roll, state] : candidates) {
            int lie = (l <= roll && roll <= r) ^ answer;
            bool repeat = (state & 1) == lie;
            if (repeat && (state & 2)) continue;
            candidates[kept++] = {roll, lie | (repeat ? 2 : 0)};
        }
        candidates.resize(kept);
    }

    for (const auto& c : candidates)
        if (nominate(c.roll)) break;
    cout << "#" << endl;
    return 0;
}
