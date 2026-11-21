#include <bits/stdc++.h>
using namespace std;

constexpr int LIMIT = 1000000000;

int n;
vector<char> excluded;
unordered_map<int, int> cache;

int key(array<int, 3> t) {
    sort(t.begin(), t.end());
    return (t[0] * 1024 + t[1]) * 1024 + t[2];
}

int known(array<int, 3> t) {
    auto it = cache.find(key(t));
    return it == cache.end() ? -1 : it->second;
}

int ask(array<int, 3> t) {
    int k = key(t);
    auto it = cache.find(k);
    if (it != cache.end()) return it->second;
    cout << "? " << t[0] << ' ' << t[1] << ' ' << t[2] << endl;
    int value;
    if (!(cin >> value) || value == -1) exit(0);
    cache.emplace(k, value);
    return value;
}

[[noreturn]] void answer(int a, int b) {
    cout << "! " << a << ' ' << b << endl;
    exit(0);
}

[[noreturn]] void answer(const vector<int>& v) {
    answer(v[0], v.size() == 1 ? v[0] : v[1]);
}

void finish_if_small() {
    vector<int> live;
    for (int i = 1; i <= n; ++i) {
        if (!excluded[i]) {
            live.push_back(i);
            if (live.size() > 2) return;
        }
    }
    answer(live);
}

// This triple is already known to contain both 0 and LIMIT.
[[noreturn]] void full_triple(array<int, 3> t) {
    vector<int> live;
    for (int x : t) if (!excluded[x]) live.push_back(x);
    if (live.size() <= 2) answer(live);

    auto replacement = [&](int v, int i) {
        auto u = t;
        u[i] = v;
        return u;
    };

    int best = -1, best_count = -1;
    for (int v = 1; v <= n; ++v) {
        if (find(t.begin(), t.end(), v) != t.end()) continue;
        int count = 0;
        for (int i = 0; i < 3; ++i) {
            int r = known(replacement(v, i));
            if (r == LIMIT) answer(t[(i + 1) % 3], t[(i + 2) % 3]);
            count += (r >= 0);
        }
        if (count > best_count) {
            best_count = count;
            best = v;
        }
    }

    array<int, 3> r;
    for (int i = 0; i < 3; ++i) r[i] = known(replacement(best, i));
    for (int i = 0; i < 3; ++i) {
        if (r[i] >= 0) continue;
        int j = (i + 1) % 3, k = (i + 2) % 3;
        if (r[j] >= 0 && r[k] >= 0) answer(t[j], t[k]);
        r[i] = ask(replacement(best, i));
        if (r[i] == LIMIT) answer(t[j], t[k]);
    }
    exit(0);
}

struct Interval {
    int a = 0, b = 0, width = 0;
    // If set, witness differs from b and lies between a and b.
    int witness = 0;
    vector<int> covered;
};

void set_equal(Interval& s, array<int, 3> t) {
    for (int x : t) excluded[x] = true;
    s = {t[0], t[1], 0, t[2], {t[0], t[1], t[2]}};
}

array<int, 3> without(const vector<int>& v, int i) {
    array<int, 3> t;
    int k = 0;
    for (int j = 0; j < (int)v.size(); ++j)
        if (j != i) t[k++] = v[j];
    return t;
}

bool inspect(const vector<int>& pool, Interval& s) {
    int m = (int)pool.size();

    auto inspect_four = [&](const vector<int>& q) {
        array<int, 4> r;
        int count = 0, hi = -1;
        for (int i = 0; i < 4; ++i) {
            r[i] = known(without(q, i));
            if (r[i] >= 0) {
                ++count;
                hi = max(hi, r[i]);
            }
        }
        if (count < 3) return;
        // The full range occurs in at least two of the four triples.
        vector<int> lower;
        int witness = 0;
        for (int i = 0; i < 4; ++i) {
            if (r[i] == hi) {
                excluded[q[i]] = true;
                witness = q[i];
            } else if (r[i] >= 0) {
                lower.push_back(q[i]);
            }
        }
        if (lower.size() == 2 && !s.a)
            s = {lower[0], lower[1], hi, witness, q};
    };

    if (m == 4) {
        inspect_four(pool);
    } else {
        for (int skip = 0; skip < 5; ++skip) {
            vector<int> q;
            for (int i = 0; i < 5; ++i)
                if (i != skip) q.push_back(pool[i]);
            inspect_four(q);
        }
    }
    finish_if_small();
    if (s.a || m == 4) return s.a != 0;

    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            int a = pool[i], b = pool[j];
            array<int, 3> rest;
            int k = 0;
            for (int x : pool) if (x != a && x != b) rest[k++] = x;
            int r = known({a, b, rest[0]});
            int other = known(rest);
            if (other <= 0 || other > r ||
                known({a, b, rest[1]}) != r || known({a, b, rest[2]}) != r)
                continue;

            // If the pair's distance were less than r, the three other
            // values could occupy only two locations, separated by > r.
            s = {a, b, r, 0, pool};
            for (int side = 0; side < 2 && !s.witness; ++side) {
                for (int u = 0; u < 3; ++u) {
                    for (int v = u + 1; v < 3; ++v) {
                        int h = known({s.a, rest[u], rest[v]});
                        if (h >= 0 && h < r) s.witness = rest[u];
                    }
                }
                if (!s.witness) swap(s.a, s.b);
            }
            return true;
        }
    }
    return false;
}

// Prefer a query that brings a still-possible spanning pair closer to a certificate.
array<int, 3> next_bootstrap_query(const vector<int>& pool) {
    int best = 5;
    array<int, 3> result{};
    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            int a = pool[i], b = pool[j];
            array<int, 3> rest;
            int k = 0;
            for (int x : pool) if (x != a && x != b) rest[k++] = x;
            int r = -1, missing = 0;
            bool ok = true;
            array<int, 3> next{};
            for (int x : rest) {
                array<int, 3> t{a, b, x};
                int v = known(t);
                if (v < 0) {
                    ++missing;
                    next = t;
                } else if (r < 0) {
                    r = v;
                } else if (r != v) {
                    ok = false;
                }
            }
            int other = known(rest);
            if (other < 0) {
                if (!missing) next = rest;
                ++missing;
            } else if (other == 0 || (r >= 0 && other > r)) {
                ok = false;
            }
            if (ok && missing > 0 && missing < best) {
                best = missing;
                result = next;
            }
        }
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n) || n == -1) return 0;
    excluded.assign(n + 1, false);
    cache.reserve(2 * n + 16);

    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count()
                ^ random_device{}());
    shuffle(order.begin(), order.end(), rng);

    Interval s;
    int screen_credit = 0;
    int first = 0;
    for (; first + 2 < n; first += 3) {
        array<int, 3> t{order[first], order[first + 1], order[first + 2]};
        int r = ask(t);
        if (r == LIMIT) full_triple(t);
        if (r != 0) break;
        set_equal(s, t);
        screen_credit = 2;
    }
    finish_if_small();

    if (!s.a) {
        vector<int> pool(order.begin(), order.begin() + 4);
        for (int skip = 2; skip >= 0 && !s.a; --skip) {
            auto t = without(pool, skip);
            int r = ask(t);
            if (r == LIMIT) full_triple(t);
            if (r == 0) {
                set_equal(s, t);
                screen_credit = 2;
                break;
            }
            inspect(pool, s);
        }

        if (!s.a) {
            bool all_same = true;
            for (int i = 0; i < 4; ++i)
                all_same &= known(without(pool, i)) == known(without(pool, 0));
            if (all_same) screen_credit = 1;
            pool.push_back(order[4]);
            while (!inspect(pool, s)) {
                auto t = next_bootstrap_query(pool);
                int r = ask(t);
                if (r == LIMIT) full_triple(t);
                if (r == 0) {
                    set_equal(s, t);
                    screen_credit = 2;
                    break;
                }
            }
        }
    }

    vector<int> work;
    for (int x : order) {
        if (!excluded[x] &&
            find(s.covered.begin(), s.covered.end(), x) == s.covered.end())
            work.push_back(x);
    }

    int pos = 0, next_screen = 0;
    int pending = 0, pending_dist = 0;
    const int total = (int)work.size();

    auto check_finish = [&](int second = 0) {
        if (s.width == LIMIT) answer(s.a, s.b);
        int count = total - pos + (pending != 0) + (second != 0)
                  + !excluded[s.a] + !excluded[s.b];
        if (count > 2) return;
        vector<int> live;
        if (!excluded[s.a]) live.push_back(s.a);
        if (!excluded[s.b]) live.push_back(s.b);
        if (pending) live.push_back(pending);
        if (second) live.push_back(second);
        for (int i = pos; i < total; ++i) live.push_back(work[i]);
        answer(live);
    };

    auto finish_outside = [&](int x, int range) {
        if (excluded[s.a]) answer(x, s.b);
        if (excluded[s.b]) answer(x, s.a);
        if (s.witness) {
            int r = ask({s.a, s.witness, x});
            answer(x, r < range ? s.b : s.a);
        }
        // At least one of these witnesses differs from b.
        for (int c : s.covered) {
            if (c != s.a && c != s.b && ask({s.a, c, x}) < range)
                answer(x, s.b);
        }
        answer(x, s.a);
    };

    while (true) {
        check_finish();
        if (pos == total) finish_outside(pending, s.width + pending_dist);

        if (screen_credit > 0 && pos >= next_screen && total - pos >= 3) {
            array<int, 3> t{work[pos], work[pos + 1], work[pos + 2]};
            int r = known(t);
            if (r < 0) {
                r = ask(t);
                if (r != 0) --screen_credit;
            }
            if (r == LIMIT) full_triple(t);
            next_screen = pos + 3;
            if (r == 0) {
                pos += 3;
                screen_credit = min(4, screen_credit + 2);
                continue;
            }
        }

        int x = work[pos++];
        int r = ask({s.a, s.b, x});
        if (r == s.width) continue;

        if (s.width == 0) {
            s.witness = s.b;
            s.b = x;
            s.width = r;
            continue;
        }
        if (r == LIMIT) finish_outside(x, r);

        int d = r - s.width;
        if (!pending) {
            pending = x;
            pending_dist = d;
            continue;
        }

        check_finish(x);
        int hi = max(d, pending_dist);
        int side = ask({s.a, pending, x});
        if (side == hi || side == s.width + hi) {
            int farthest = d > pending_dist ? x : pending;
            if (d == pending_dist) excluded[farthest] = true;
            if (side == hi) {
                s.witness = s.a;
                s.a = farthest;
            } else {
                s.witness = s.b;
                s.b = farthest;
            }
            s.width += hi;
        } else {
            s.witness = s.a;
            s.a = pending;
            s.b = x;
            s.width += d + pending_dist;
        }
        pending = 0;
    }
}
