#include <bits/stdc++.h>
using namespace std;

using ll = long long;

ll n, T, L0, cur_i;
vector<pair<ll, ll>> pref;
unordered_map<ll, ll> prefMap;

ll C2(ll x) {
    if (x < 2) return 0;
    return (ll)((__int128)x * (x - 1) / 2);
}

ll isqrt128(__int128 x) {
    if (x < 0) return -1;
    ll lo = 0, hi = 3037000500LL;
    while (lo <= hi) {
        ll mid = (lo + hi) / 2;
        __int128 sq = (__int128)mid * mid;
        if (sq <= x) lo = mid + 1;
        else hi = mid - 1;
    }
    return hi;
}

ll tri_inv(ll rem) {
    if (rem < 0) return -1;
    __int128 D = (__int128)1 + (__int128)8 * rem;
    ll s = isqrt128(D);
    if ((__int128)s * s != D) return -1;
    if ((s - 1) % 2 != 0) return -1;
    ll y = (s + 1) / 2;
    if (y < 1) return -1;
    if (C2(y) != rem) return -1;
    return y;
}

ll ask(ll l, ll r) {
    cout << "? " << l << " " << r << "\n";
    cout.flush();
    ll ans;
    if (!(cin >> ans)) exit(0);
    if (ans == -1) exit(0);
    return ans;
}

ll ask_prefix(ll m) {
    if (m < 1) m = 1;
    if (m > n) m = n;
    if (m == 1) return 0;
    auto it = prefMap.find(m);
    if (it != prefMap.end()) return it->second;
    ll ans = ask(1, m);
    prefMap[m] = ans;
    pref.push_back({m, ans});
    return ans;
}

vector<ll> solve_x_from_A(ll L, ll A) {
    vector<ll> res;
    if (L < 2) return res;
    __int128 D = (__int128)4 * A - (__int128)L * (L - 2);
    if (D < 0) return res;
    ll s = isqrt128(D);
    if ((__int128)s * s != D) return res;

    auto add = [&](ll x) {
        if (x >= 2 && x <= L) res.push_back(x);
    };

    if (L >= s && (L - s) % 2 == 0) add((L - s) / 2);
    if ((L + s) % 2 == 0) add((L + s) / 2);

    sort(res.begin(), res.end());
    res.erase(unique(res.begin(), res.end()), res.end());
    return res;
}

bool valid_x(ll xx, ll &yy) {
    if (xx < 2 || xx > L0) return false;
    ll cx = C2(xx);
    if (cx > T) return false;
    ll rem = T - cx;
    ll y = tri_inv(rem);
    if (y < 1) return false;
    ll N = xx + y;
    if (N > n - cur_i + 1) return false;
    if (N <= L0) return false;
    yy = y;
    return true;
}

bool resolve(ll i, ll &x, ll &y) {
    cur_i = i;

    ll loX = 2, hiX = L0;
    bool discrete = false;
    vector<ll> vals;

    for (auto [m, A] : pref) {
        if (A == 0 || A == T) continue;
        ll d = m - i + 1;
        if (d <= 1) continue;

        ll maxA = C2(d);
        if (A > maxA) continue;

        if (A == maxA) {
            if (discrete) {
                vector<ll> nv;
                for (ll xx : vals) if (xx >= d) nv.push_back(xx);
                vals.swap(nv);
            } else {
                loX = max(loX, d);
            }
        } else {
            vector<ll> cand = solve_x_from_A(d, A);
            vector<ll> filtered;
            for (ll xx : cand) {
                if (xx >= 2 && xx < d && xx <= L0) filtered.push_back(xx);
            }
            if (filtered.empty()) continue;

            if (discrete) {
                sort(filtered.begin(), filtered.end());
                vector<ll> nv;
                for (ll xx : vals) {
                    if (binary_search(filtered.begin(), filtered.end(), xx)) nv.push_back(xx);
                }
                vals.swap(nv);
            } else {
                vector<ll> nv;
                for (ll xx : filtered) {
                    if (xx >= loX && xx <= hiX) nv.push_back(xx);
                }
                vals.swap(nv);
                discrete = true;
            }
        }
    }

    auto try_output = [&](ll xx) -> bool {
        ll yy;
        if (valid_x(xx, yy)) {
            x = xx;
            y = yy;
            return true;
        }
        return false;
    };

    if (discrete) {
        vector<ll> nv;
        for (ll xx : vals) {
            ll yy;
            if (valid_x(xx, yy)) nv.push_back(xx);
        }
        vals.swap(nv);

        if (vals.size() == 1) return try_output(vals[0]);
        if (vals.empty()) {
            discrete = false;
            loX = 2;
            hiX = L0;
        }
    } else {
        if (loX > hiX) {
            discrete = false;
            loX = 2;
            hiX = L0;
        }
        if (loX == hiX) {
            if (try_output(loX)) return true;
            discrete = false;
            loX = 2;
            hiX = L0;
        }
    }

    ll m1 = i + L0 - 1;
    ll A1 = ask_prefix(m1);

    vector<ll> cand = solve_x_from_A(L0, A1);
    vector<ll> newvals;

    if (discrete) {
        sort(cand.begin(), cand.end());
        for (ll xx : vals) {
            if (binary_search(cand.begin(), cand.end(), xx)) newvals.push_back(xx);
        }
    } else {
        for (ll xx : cand) {
            if (xx >= loX && xx <= hiX) newvals.push_back(xx);
        }
    }

    vector<ll> nv;
    for (ll xx : newvals) {
        ll yy;
        if (valid_x(xx, yy)) nv.push_back(xx);
    }
    newvals.swap(nv);

    if (newvals.size() == 1) return try_output(newvals[0]);

    if (newvals.size() == 2) {
        ll x1 = newvals[0], x2 = newvals[1];
        ll l = i + x1 - 1;
        ll r = i + x1;
        ll B = ask(l, r);
        if (B == 0) {
            if (try_output(x1)) return true;
        }
        if (try_output(x2)) return true;
    }

    return false;
}

void fallback(ll i, ll &x, ll &y) {
    cur_i = i;

    ll lo = i + 1, hi = n;
    while (lo < hi) {
        ll mid = (lo + hi) / 2;
        ll A = ask_prefix(mid);
        if (A == T) hi = mid;
        else lo = mid + 1;
    }
    ll mT = lo;

    vector<pair<ll, ll>> poss;

    auto add_candidate = [&](ll xx, ll yy) {
        ll tmp;
        if (valid_x(xx, tmp) && tmp == yy) poss.push_back({xx, yy});
    };

    ll N1 = mT - i + 1;
    __int128 D = (__int128)4 * T - (__int128)N1 * N1 + 2 * N1;
    if (D >= 0) {
        ll s = isqrt128(D);
        if ((__int128)s * s == D) {
            if (N1 >= s && (N1 - s) % 2 == 0) {
                ll xx = (N1 - s) / 2;
                add_candidate(xx, N1 - xx);
            }
            if ((N1 + s) % 2 == 0) {
                ll xx = (N1 + s) / 2;
                add_candidate(xx, N1 - xx);
            }
        }
    }

    ll x_tri = tri_inv(T);
    if (x_tri >= 2 && mT - i + 1 == x_tri) {
        add_candidate(x_tri, 1);
    }

    sort(poss.begin(), poss.end());
    poss.erase(unique(poss.begin(), poss.end()), poss.end());

    if (poss.empty()) {
        x = 2;
        y = 1;
        return;
    }

    for (size_t idx = 0; idx + 1 < poss.size(); ++idx) {
        ll xx = poss[idx].first;
        ll B = ask(i + xx - 1, i + xx);
        if (B == 0) {
            x = poss[idx].first;
            y = poss[idx].second;
            return;
        }
    }

    x = poss.back().first;
    y = poss.back().second;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    T = ask(1, n);
    prefMap[n] = T;
    pref.push_back({n, T});

    L0 = isqrt128((__int128)2 * T) + 1;

    ll max_i = n - L0;
    if (max_i < 1) max_i = 1;

    ll lo = 2, hi = max_i + 1;

    while (lo < hi) {
        ll mid = (lo + hi) / 2;
        ll A = ask_prefix(mid);

        if (A == 0) {
            lo = mid + 1;
        } else if (A == T) {
            ll new_hi = mid - L0 + 2;
            if (new_hi < lo) new_hi = lo;
            if (new_hi > hi) new_hi = hi;
            hi = new_hi;
        } else {
            ll new_lo = mid - 2 * L0 + 3;
            if (new_lo < lo) new_lo = lo;
            if (new_lo > hi) new_lo = hi;
            lo = new_lo;
            hi = mid;
        }
    }

    ll i = lo - 1;
    ll x, y;

    if (!resolve(i, x, y)) {
        fallback(i, x, y);
    }

    cout << "! " << i << " " << i + x << " " << i + x + y - 1 << "\n";
    cout.flush();
    return 0;
}
