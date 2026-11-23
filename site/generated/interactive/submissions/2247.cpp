#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using i128 = __int128_t;

static i128 gcd128(i128 a, i128 b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        i128 r = a % b;
        a = b;
        b = r;
    }
    return a;
}

struct Frac {
    ll n = 0, d = 1;

    Frac() = default;
    Frac(ll x) : n(x), d(1) {}

    static Frac make(i128 nn, i128 dd) {
        Frac r;
        if (dd < 0) {
            nn = -nn;
            dd = -dd;
        }
        if (nn == 0) {
            r.n = 0;
            r.d = 1;
            return r;
        }
        i128 g = gcd128(nn, dd);
        nn /= g;
        dd /= g;
        r.n = (ll)nn;
        r.d = (ll)dd;
        return r;
    }

    bool zero() const { return n == 0; }

    Frac operator+(const Frac& o) const {
        return make((i128)n * o.d + (i128)o.n * d, (i128)d * o.d);
    }
    Frac operator-(const Frac& o) const {
        return make((i128)n * o.d - (i128)o.n * d, (i128)d * o.d);
    }
    Frac operator*(const Frac& o) const {
        return make((i128)n * o.n, (i128)d * o.d);
    }
    Frac operator/(const Frac& o) const {
        return make((i128)n * o.d, (i128)d * o.n);
    }
    bool operator<(const Frac& o) const {
        return (i128)n * o.d < (i128)o.n * d;
    }
};

struct WeakOrder {
    vector<int> g;
    int k;
};

struct BaseSolver {
    int n;
    vector<WeakOrder> orders;
    vector<array<int, 3>> qs;
    vector<ll> ans;

    explicit BaseSolver(int n_) : n(n_) {
        int total = 1;
        for (int i = 0; i < n; i++) total *= n;

        for (int code = 0; code < total; code++) {
            int x = code;
            vector<int> g(n);
            int mx = -1;
            for (int i = 0; i < n; i++) {
                g[i] = x % n;
                x /= n;
                mx = max(mx, g[i]);
            }
            vector<char> seen(mx + 1, 0);
            for (int v : g) seen[v] = 1;
            bool ok = true;
            for (char v : seen) ok &= v;
            if (ok) orders.push_back({g, mx + 1});
        }
    }

    void add(array<int, 3> q, ll a) {
        qs.push_back(q);
        ans.push_back(a);
    }

    pair<bool, vector<ll>> solve(bool finalStage = false) const {
        set<vector<ll>> candidates;
        bool consistentUnderdetermined = false;
        int m = (int)qs.size();

        for (const auto& ord : orders) {
            int k = ord.k;
            vector<vector<Frac>> a(m, vector<Frac>(k + 1));

            for (int r = 0; r < m; r++) {
                int lo = min({ord.g[qs[r][0]], ord.g[qs[r][1]], ord.g[qs[r][2]]});
                int hi = max({ord.g[qs[r][0]], ord.g[qs[r][1]], ord.g[qs[r][2]]});
                a[r][lo] = a[r][lo] + Frac(1);
                a[r][hi] = a[r][hi] + Frac(1);
                a[r][k] = Frac(ans[r]);
            }

            vector<int> where(k, -1);
            int row = 0;

            for (int col = 0; col < k; col++) {
                int sel = -1;
                for (int i = row; i < m; i++) {
                    if (!a[i][col].zero()) {
                        sel = i;
                        break;
                    }
                }
                if (sel == -1) continue;

                swap(a[sel], a[row]);
                Frac piv = a[row][col];
                for (int j = 0; j <= k; j++) a[row][j] = a[row][j] / piv;

                for (int i = 0; i < m; i++) {
                    if (i == row || a[i][col].zero()) continue;
                    Frac factor = a[i][col];
                    for (int j = 0; j <= k; j++) {
                        a[i][j] = a[i][j] - factor * a[row][j];
                    }
                }

                where[col] = row;
                row++;
            }

            bool inconsistent = false;
            for (int i = 0; i < m; i++) {
                bool allZero = true;
                for (int j = 0; j < k; j++) {
                    if (!a[i][j].zero()) {
                        allZero = false;
                        break;
                    }
                }
                if (allZero && !a[i][k].zero()) {
                    inconsistent = true;
                    break;
                }
            }
            if (inconsistent) continue;

            if (row < k) {
                consistentUnderdetermined = true;
                continue;
            }

            vector<Frac> x(k);
            for (int i = 0; i < k; i++) x[i] = a[where[i]][k];

            bool ok = true;
            vector<ll> groupVal(k);
            for (int i = 0; i < k; i++) {
                if (i > 0 && !(x[i - 1] < x[i])) ok = false;
                if (x[i].d != 1 || x[i].n < 0 || x[i].n > 1000000000LL) ok = false;
                groupVal[i] = x[i].n;
            }
            if (!ok) continue;

            vector<ll> val(n);
            for (int i = 0; i < n; i++) val[i] = groupVal[ord.g[i]];

            for (int r = 0; r < m; r++) {
                ll p = val[qs[r][0]];
                ll q = val[qs[r][1]];
                ll s = val[qs[r][2]];
                if (min({p, q, s}) + max({p, q, s}) != ans[r]) {
                    ok = false;
                    break;
                }
            }
            if (ok) candidates.insert(val);
        }

        if (candidates.size() == 1 && (!consistentUnderdetermined || finalStage)) {
            return {true, *candidates.begin()};
        }
        return {false, {}};
    }
};

static ll ask(int i, int j, int k) {
    cout << "? " << i + 1 << ' ' << j + 1 << ' ' << k + 1 << '\n';
    cout.flush();
    ll response;
    if (!(cin >> response)) exit(0);
    if (response == -1) exit(0);
    return response;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<array<int, 3>> all5;
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            for (int k = j + 1; k < 5; k++) {
                all5.push_back({i, j, k});
            }
        }
    }

    vector<array<int, 3>> preferred = {
        {0, 1, 2}, {0, 1, 3}, {0, 1, 4}, {1, 2, 3}, {2, 3, 4}
    };
    set<array<int, 3>> used(preferred.begin(), preferred.end());
    for (auto q : all5) {
        if (!used.count(q)) preferred.push_back(q);
    }

    BaseSolver solver(5);
    vector<ll> baseValues;
    bool solved = false;

    for (auto q : preferred) {
        ll response = ask(q[0], q[1], q[2]);
        solver.add(q, response);
        auto result = solver.solve(false);
        if (result.first) {
            baseValues = result.second;
            solved = true;
            break;
        }
    }

    if (!solved) {
        auto result = solver.solve(true);
        if (result.first) {
            baseValues = result.second;
            solved = true;
        }
    }

    if (!solved && n >= 6) {
        BaseSolver solver6(6);
        set<array<int, 3>> asked;
        for (int i = 0; i < (int)solver.qs.size(); i++) {
            solver6.add(solver.qs[i], solver.ans[i]);
            asked.insert(solver.qs[i]);
        }

        for (int i = 0; i < 6 && !solved; i++) {
            for (int j = i + 1; j < 6 && !solved; j++) {
                for (int k = j + 1; k < 6 && !solved; k++) {
                    array<int, 3> q = {i, j, k};
                    if (asked.count(q)) continue;
                    ll response = ask(i, j, k);
                    solver6.add(q, response);
                    auto result = solver6.solve(false);
                    if (result.first) {
                        baseValues = result.second;
                        solved = true;
                    }
                }
            }
        }

        if (!solved) {
            auto result = solver6.solve(true);
            if (result.first) {
                baseValues = result.second;
                solved = true;
            }
        }
    }

    if (!solved) return 0;

    int baseSize = (int)baseValues.size();
    vector<ll> c(n, 0);
    vector<pair<ll, int>> ord;
    for (int i = 0; i < baseSize; i++) {
        c[i] = baseValues[i];
        ord.push_back({c[i], i});
    }
    sort(ord.begin(), ord.end());

    for (int i = baseSize; i < n; i++) {
        int r = 0;
        ll bestGap = ord[1].first - ord[0].first;
        for (int p = 1; p + 1 < (int)ord.size(); p++) {
            ll gap = ord[p + 1].first - ord[p].first;
            if (gap < bestGap) {
                bestGap = gap;
                r = p;
            }
        }

        ll a = ord[r].first;
        ll b = ord[r + 1].first;
        ll response = ask(ord[r].second, ord[r + 1].second, i);
        ll x;

        if (response < a + b) {
            x = response - b;
        } else if (response > a + b) {
            x = response - a;
        } else if (a == b) {
            x = a;
        } else if (r > 0) {
            ll below = ord[r - 1].first;
            ll second = ask(i, ord[r].second, ord[r - 1].second);
            x = second - below;
        } else {
            ll above = ord[r + 2].first;
            ll second = ask(i, ord[r + 1].second, ord[r + 2].second);
            x = second - above;
        }

        c[i] = x;
        ord.insert(lower_bound(ord.begin(), ord.end(), make_pair(x, i)), {x, i});
    }

    cout << "!";
    for (int i = 0; i < n; i++) cout << ' ' << c[i];
    cout << '\n';
    cout.flush();
    return 0;
}
