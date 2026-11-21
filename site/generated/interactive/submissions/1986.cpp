#include <bits/stdc++.h>
using namespace std;

using ll = long long;

static ll query3(int i, int j, int k) {
    cout << "? " << (i + 1) << ' ' << (j + 1) << ' ' << (k + 1) << '\n';
    cout.flush();
    ll s;
    if (!(cin >> s)) exit(0);
    return s;
}

// Solve the values of labels 0..4 from all C(5,3) triple sums.
// Every possible sorted order (permutation of labels) is tried.
// For a fixed order the queries form a graph whose edges are sums
// v_u + v_v = s.  We solve exactly using v = a*t+b, where a is +/-1.
static vector<ll> solve_base(const ll ans[5][5][5]) {
    vector<int> p = {0, 1, 2, 3, 4};
    do {
        int rank[5];
        for (int r = 0; r < 5; ++r) rank[p[r]] = r;

        vector<vector<pair<int, ll>>> adj(5);
        for (int i = 0; i < 5; ++i)
            for (int j = i + 1; j < 5; ++j)
                for (int k = j + 1; k < 5; ++k) {
                    int rmin = min({rank[i], rank[j], rank[k]});
                    int rmax = max({rank[i], rank[j], rank[k]});
                    ll s = ans[i][j][k];
                    adj[rmin].push_back({rmax, s});
                    adj[rmax].push_back({rmin, s});
                }

        int a[5];
        ll b[5];
        bool vis[5] = {false, false, false, false, false};
        bool ok = true;
        bool global_t_set = false;
        ll global_t2 = 0;

        for (int st = 0; st < 5 && ok; ++st) {
            if (vis[st]) continue;

            bool comp_t_set = false;
            ll comp_t2 = 0;

            vector<int> q;
            q.push_back(st);
            vis[st] = true;
            a[st] = 1;
            b[st] = 0;

            for (size_t qi = 0; qi < q.size() && ok; ++qi) {
                int u = q[qi];
                for (auto [v, s] : adj[u]) {
                    int na = -a[u];
                    ll nb = s - b[u];
                    if (!vis[v]) {
                        vis[v] = true;
                        a[v] = na;
                        b[v] = nb;
                        q.push_back(v);
                    } else {
                        // a[v]*t+b[v] == -a[u]*t + (s-b[u])
                        int coeff = a[v] + a[u];
                        ll R = s - b[u] - b[v];
                        if (coeff == 0) {
                            if (R != 0) {
                                ok = false;
                                break;
                            }
                        } else {
                            // coeff is either +2 or -2
                            ll nt2 = (coeff == 2 ? R : -R);
                            if (!comp_t_set) {
                                comp_t_set = true;
                                comp_t2 = nt2;
                            } else if (comp_t2 != nt2) {
                                ok = false;
                                break;
                            }
                        }
                    }
                }
            }

            if (!ok) break;
            // Each component must determine t; with all ten triples the
            // graph is connected and non-bipartite, so this always holds.
            if (!comp_t_set) {
                ok = false;
                break;
            }
            if (!global_t_set) {
                global_t_set = true;
                global_t2 = comp_t2;
            } else if (global_t2 != comp_t2) {
                ok = false;
                break;
            }
        }

        if (!ok || !global_t_set) continue;

        ll v[5];
        bool good = true;
        for (int i = 0; i < 5; ++i) {
            ll num = (ll)a[i] * global_t2 + 2 * b[i];
            if (num % 2 != 0) {
                good = false;
                break;
            }
            ll value = num / 2;
            if (value < 0 || value > 1000000000LL) {
                good = false;
                break;
            }
            v[i] = value;
        }
        if (!good) continue;
        for (int i = 0; i + 1 < 5; ++i)
            if (v[i] > v[i + 1]) {
                good = false;
                break;
            }
        if (!good) continue;

        vector<ll> val(5);
        for (int lab = 0; lab < 5; ++lab) val[lab] = v[rank[lab]];

        bool verify = true;
        for (int i = 0; i < 5 && verify; ++i)
            for (int j = i + 1; j < 5 && verify; ++j)
                for (int k = j + 1; k < 5 && verify; ++k) {
                    ll got = min({val[i], val[j], val[k]}) +
                             max({val[i], val[j], val[k]});
                    if (got != ans[i][j][k]) verify = false;
                }
        if (!verify) continue;
        return val;
    } while (next_permutation(p.begin(), p.end()));

    return {};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    ll ans[5][5][5];
    memset(ans, 0, sizeof(ans));
    for (int i = 0; i < 5; ++i)
        for (int j = i + 1; j < 5; ++j)
            for (int k = j + 1; k < 5; ++k)
                ans[i][j][k] = query3(i, j, k);

    vector<ll> base = solve_base(ans);
    // The base is always uniquely determined by the ten queries.
    if ((int)base.size() != 5) return 0;

    vector<ll> value(n, 0);
    for (int i = 0; i < 5; ++i) value[i] = base[i];

    vector<pair<ll, int>> known;
    for (int i = 0; i < 5; ++i) known.push_back({base[i], i});
    sort(known.begin(), known.end());

    // If two known values are equal, queries with those two indices always
    // return (common value + x), so every remaining value is obtained with
    // exactly one query and no ambiguity.
    int dup_a = -1, dup_b = -1;
    ll dup_c = 0;
    for (int i = 0; i + 1 < (int)known.size(); ++i) {
        if (known[i].first == known[i + 1].first) {
            dup_a = known[i].second;
            dup_b = known[i + 1].second;
            dup_c = known[i].first;
            break;
        }
    }

    int idx = 5;
    mt19937_64 rng((unsigned long long)chrono::steady_clock::now().time_since_epoch().count() ^
                   (unsigned long long)(uintptr_t)&rng);

    if (dup_a == -1) {
        for (; idx < n; ++idx) {
            int m = (int)known.size();
            // Pick one of the m-1 consecutive intervals uniformly at random.
            int g = (int)(rng() % (unsigned long long)(m - 1));

            ll vi = known[g].first, vj = known[g + 1].first;
            int ii = known[g].second, jj = known[g + 1].second;

            ll s = query3(ii, jj, idx);
            ll x;

            if (s == vi + vj) {
                // x is in [vi, vj].
                if (vi == vj) {
                    x = vi;
                } else if (g >= 1) {
                    ll a = known[g - 1].first;
                    int ia = known[g - 1].second;
                    int ib = known[g].second;
                    ll q = query3(ia, ib, idx);
                    x = q - a;
                } else {
                    ll b = known[g + 2].first;
                    int ia = known[g + 1].second;
                    int ib = known[g + 2].second;
                    ll q = query3(ia, ib, idx);
                    x = q - b;
                }
            } else if (s < vi + vj) {
                x = s - vj;
            } else {
                x = s - vi;
            }

            value[idx] = x;

            auto it = lower_bound(known.begin(), known.end(), make_pair(x, INT_MIN));
            if (it != known.end() && it->first == x) {
                // A duplicate appeared: switch to the universal pair.
                dup_a = it->second;
                dup_b = idx;
                dup_c = x;
                ++idx;
                break;
            }
            known.insert(lower_bound(known.begin(), known.end(), make_pair(x, idx)),
                         {x, idx});
        }
    }

    if (dup_a != -1) {
        for (; idx < n; ++idx) {
            ll s = query3(dup_a, dup_b, idx);
            value[idx] = s - dup_c;
        }
    }

    cout << '!';
    for (int i = 0; i < n; ++i) cout << ' ' << value[i];
    cout << '\n';
    cout.flush();
    return 0;
}
