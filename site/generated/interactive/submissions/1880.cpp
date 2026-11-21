#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using i128 = __int128_t;

constexpr ll LIMIT = 100000000000000LL;
constexpr ll SAT = LIMIT + 1;
constexpr int H = 60;

ll power2[H + 1];

// In a model, a query costs c on failure and earns g on success.
// Recharging also earns g. Balances are measured in units d / scale.
struct Model {
    ll d = 1;
    int scale = 1, g = 1, c = 1;
};

struct Table {
    vector<vector<ll>> rows{{}};
};

unique_ptr<Table> tables[129][129];

ll count_leaves(int t, ll b, int g, int c) {
    if (t < 0) return 0;
    if (b >= ll(t) * c) return power2[t];

    auto &ptr = tables[g][c];
    if (!ptr) ptr = make_unique<Table>();
    auto &rows = ptr->rows;
    while (int(rows.size()) <= t) {
        int k = int(rows.size());
        const auto &previous = rows.back();
        int end = (k - 1) * c;
        vector<ll> row(k * c);
        for (int v = 0; v < k * c; ++v) {
            ll a = v + g >= end ? power2[k - 1] : previous[v + g];
            if (v >= c) a += previous[v - c];
            row[v] = min(a, SAT);
        }
        rows.push_back(move(row));
    }
    return rows[t][b];
}

Model conservative(ll l, ll u) {
    int s = u <= 2 * l ? 64 : 16;
    return {l, s, s, int((u * s + l - 1) / l)};
}

Model optimistic(ll l, ll u) {
    ll d = l + 1;
    int s = u <= 2 * l ? 64 : 16;
    return {d, s, int((u * s + d - 1) / d), s};
}

ll credits(const Model &m, ll b) {
    return b * m.scale / m.d;
}

ll capacity(int t, const Model &m, ll b) {
    return count_leaves(t, credits(m, b), m.g, m.c);
}

ll upper_bound_leaves(int t, ll l, ll u, ll b) {
    if (l == u) return t >= 0;
    if (b <= l) {
        ll k = (2 * l - b) / l;
        t -= int(k);
        b += k * l;
    }
    return capacity(t, optimistic(l, u), b);
}

struct Plan;
using Ptr = shared_ptr<Plan>;

struct Plan {
    int h;
    ll x;  // Zero means to use the fixed model.
    Model model;
    Ptr fail, lucky;
};

const Ptr finished = make_shared<Plan>(Plan{0, 0, {}, {}, {}});

Ptr uniform_plan(int h, const Model &m) {
    return make_shared<Plan>(Plan{h, 0, m, {}, {}});
}

Ptr explicit_plan(int h, ll x, Ptr fail, Ptr lucky) {
    return make_shared<Plan>(Plan{h, x, {}, move(fail), move(lucky)});
}

Ptr certificate(int t, ll l, ll u, ll b) {
    if (l == u) return finished;
    Model m = conservative(l, u);
    if (capacity(t, m, b) >= u - l + 1) return uniform_plan(t, m);
    return {};
}

// Try a split certified by two separate, tighter models for the children.
Ptr quick_split(int t, ll l, ll u, ll b) {
    if (t <= 0) return {};
    ll a = max(l + 1, u + 1 - power2[t - 1]);
    ll z = min({u, b, l + power2[t - 1]});
    ll chosen = 0;
    while (a <= z) {
        ll x = a + (z - a) / 2;
        ll nf = x - l, ns = u - x + 1;
        ll cf = nf == 1 ? 1 : capacity(t - 1, conservative(l, x - 1), b - x);
        ll cs = ns == 1 ? 1 : capacity(t - 1, conservative(x, u), b + x);
        if (nf <= cf && ns <= cs) chosen = x;
        if (i128(nf) * cs <= i128(ns) * cf)
            a = x + 1;
        else
            z = x - 1;
    }
    if (!chosen) return {};
    return explicit_plan(t, chosen,
        certificate(t - 1, l, chosen - 1, b - chosen),
        certificate(t - 1, chosen, u, b + chosen));
}

// A failed search with budget remaining proves infeasibility. If the
// budget is exhausted, an already certified plan can still be used.
int budget;

Ptr prove(int t, ll l, ll u, ll b) {
    if (l == u) return finished;
    if (budget <= 0) return {};
    --budget;

    ll n = u - l + 1;
    if (t <= 0 || n > power2[t]) return {};
    if (b <= l) {
        Ptr p = prove(t - 1, l, u, b + l);
        return p ? explicit_plan(t, l, {}, move(p)) : Ptr{};
    }
    if (n == 2) return explicit_plan(1, u, finished, finished);

    Model m = conservative(l, u);
    if (capacity(t, m, b) >= n) return uniform_plan(t, m);
    if (upper_bound_leaves(t, l, u, b) < n) return {};

    ll a = max(l + 1, u + 1 - power2[t - 1]);
    ll z = min({u, b, l + power2[t - 1]});
    while (a <= z && budget > 0) {
        --budget;
        ll x = a + (z - a) / 2;
        ll nf = x - l, ns = u - x + 1;
        ll uf = upper_bound_leaves(t - 1, l, x - 1, b - x);
        if (uf < nf) {
            z = x - 1;
            continue;
        }
        ll us = upper_bound_leaves(t - 1, x, u, b + x);
        if (us < ns) {
            a = x + 1;
            continue;
        }

        Ptr f = certificate(t - 1, l, x - 1, b - x);
        Ptr s = certificate(t - 1, x, u, b + x);
        if (!f && (s || i128(nf) * us >= i128(ns) * uf)) {
            f = prove(t - 1, l, x - 1, b - x);
            if (!f) {
                z = x - 1;
                continue;
            }
        }
        if (!s) {
            s = prove(t - 1, x, u, b + x);
            if (!s) {
                a = x + 1;
                continue;
            }
        }
        if (!f) {
            f = prove(t - 1, l, x - 1, b - x);
            if (!f) {
                z = x - 1;
                continue;
            }
        }
        return explicit_plan(t, x, move(f), move(s));
    }

    if (budget > 0 && n <= power2[t - 1]) {
        Ptr p = prove(t - 1, l, u, b + l);
        if (p) return explicit_plan(t, l, {}, move(p));
    }
    return {};
}

Ptr improve(ll l, ll u, ll b, Ptr best) {
    ll n = u - l + 1;
    if (n == 2 && b >= u) return explicit_plan(1, u, finished, finished);

    int low = 1;
    while (low < H && upper_bound_leaves(low, l, u, b) < n) ++low;

    if (best && best->x == 0) {
        while (best->h > 1 && capacity(best->h - 1, best->model, b) >= n)
            best = uniform_plan(best->h - 1, best->model);
    }

    Model m = conservative(l, u);
    int top = best ? best->h - 1 : H;
    for (int t = low; t <= top; ++t) {
        if (capacity(t, m, b) >= n) {
            best = uniform_plan(t, m);
            break;
        }
    }

    top = best ? best->h : H;
    for (int t = low; t <= top; ++t) {
        Ptr p = quick_split(t, l, u, b);
        if (p) {
            best = move(p);
            break;
        }
    }

    budget = 65000;
    for (int t = best ? best->h - 1 : H; t >= low; ) {
        Ptr p = prove(t, l, u, b);
        if (!p) break;
        best = move(p);
        t = best->h - 1;
    }
    return best;
}

ll model_query(const Plan &p, ll l, ll u, ll b) {
    const auto &m = p.model;
    ll v = credits(m, b);
    if (v < m.c) return l;

    ll cf = count_leaves(p.h - 1, v - m.c, m.g, m.c);
    ll cs = count_leaves(p.h - 1, v + m.g, m.g, m.c);
    ll n = u - l + 1;
    ll k = (i128(n) * cf + (cf + cs) / 2) / (cf + cs);
    k = clamp(k, max(1LL, n - cs), min(n - 1, cf));
    return l + k;
}

// 1: success; 0: paid fine; -1: terminate.
int ask(ll x) {
    cout << "? " << x << endl;
    string reply;
    if (!(cin >> reply)) return -1;
    if (reply == "Lucky!") return 1;
    if (reply == "Fraudster!") return 0;
    return -1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    power2[0] = 1;
    for (int i = 1; i <= H; ++i) power2[i] = min(SAT, 2 * power2[i - 1]);

    ll l = 0, u = LIMIT, b = 1;

    // Grow the balance while the upper end is still far away. After the
    // first two successes, retain one euro even on the first failure.
    while (l < u && 4 * l < LIMIT) {
        ll x = min(u, l <= 1 ? b : b - 1);
        int r = ask(x);
        if (r < 0) return 0;
        if (r) {
            l = x;
            b += x;
        } else {
            u = x - 1;
            b -= x;
            break;
        }
    }

    Ptr plan;
    while (l < u) {
        ll x;
        if (b <= l) {
            x = l;
        } else {
            plan = improve(l, u, b, move(plan));
            if (plan) {
                x = plan->x ? plan->x : model_query(*plan, l, u, b);
            } else {
                // Guaranteed progress even without a search certificate.
                ll mid = l + (u - l + 1) / 2;
                x = b >= mid ? mid : l;
            }
        }

        int r = ask(x);
        if (r < 0) return 0;
        if (r) {
            b += x;
            l = max(l, x);
        } else {
            b -= x;
            u = x - 1;
        }
        if (plan && l < u) {
            Ptr next;
            if (plan->x == 0) next = uniform_plan(plan->h - 1, plan->model);
            else if (plan->x == x) next = r ? plan->lucky : plan->fail;
            plan = move(next);
        }
    }

    cout << "! " << l << endl;
    return 0;
}
