#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll P = 1000000007LL;

ll mod_string(const string& s, ll mod) {
    ll res = 0;
    for (char c : s) {
        res = (res * 10 + (c - '0')) % mod;
    }
    return res;
}

ll pow_mod(ll a, ll b, ll mod) {
    ll res = 1;
    a %= mod;
    while (b) {
        if (b & 1) res = res * a % mod;
        a = a * a % mod;
        b >>= 1;
    }
    return res;
}

ll cross(const pair<ll,ll>& a, const pair<ll,ll>& b, const pair<ll,ll>& c) {
    return (b.first - a.first) * (c.second - a.second) - (b.second - a.second) * (c.first - a.first);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    
    int n;
    if (!(cin >> n)) return 0;
    vector<pair<ll,ll>> pts(n);
    for (int i = 0; i < n; ++i) {
        cin >> pts[i].first >> pts[i].second;
    }
    
    // Convex hull (extreme vertices only)
    vector<pair<ll,ll>> sorted_pts = pts;
    sort(sorted_pts.begin(), sorted_pts.end());
    vector<pair<ll,ll>> hull;
    for (int i = 0; i < n; ++i) {
        while (hull.size() >= 2 && cross(hull[hull.size()-2], hull.back(), sorted_pts[i]) <= 0)
            hull.pop_back();
        hull.push_back(sorted_pts[i]);
    }
    int lower_size = hull.size();
    for (int i = n - 2; i >= 0; --i) {
        while (hull.size() > lower_size && cross(hull[hull.size()-2], hull.back(), sorted_pts[i]) <= 0)
            hull.pop_back();
        hull.push_back(sorted_pts[i]);
    }
    if (!hull.empty()) hull.pop_back();
    
    int m = hull.size();
    bool all_on_boundary = true;
    for (auto& p : pts) {
        bool ok = false;
        for (int i = 0; i < m; ++i) {
            auto& a = hull[i];
            auto& b = hull[(i + 1) % m];
            if (cross(a, b, p) == 0) {
                ll dot = (p.first - a.first) * (p.first - b.first) +
                         (p.second - a.second) * (p.second - b.second);
                if (dot <= 0) {
                    ok = true;
                    break;
                }
            }
        }
        if (!ok) {
            all_on_boundary = false;
            break;
        }
    }
    
    if (all_on_boundary) {
        ll twice_area = 0;
        for (int i = 0; i < m; ++i) {
            auto& a = hull[i];
            auto& b = hull[(i + 1) % m];
            twice_area += a.first * b.second - a.second * b.first;
        }
        twice_area = llabs(twice_area);
        ll u, v;
        if (twice_area % 2 == 0) {
            u = twice_area / 2;
            v = 1;
        } else {
            u = twice_area;
            v = 2;
        }
        cout << "! " << u << " " << v << endl;
        return 0;
    }
    
    set<int> xs;
    for (auto& p : pts) xs.insert((int)p.first);
    vector<int> vx(xs.begin(), xs.end());
    
    ll area_mod = 0;
    for (int i = 0; i + 1 < (int)vx.size(); ++i) {
        int a = vx[i];
        int b = vx[i + 1];
        int p = a + b;
        int q = 2;
        cout << "? " << p << " " << q << endl;
        string r_str, s_str;
        if (!(cin >> r_str >> s_str)) return 0;
        if (r_str == "-1") return 0;
        ll r_mod = mod_string(r_str, P);
        ll s_mod = mod_string(s_str, P);
        ll inv_s = pow_mod(s_mod, P - 2, P);
        ll L_mod = r_mod * inv_s % P;
        ll width = b - a;
        area_mod = (area_mod + width * L_mod) % P;
    }
    
    ll twice_area = (2 * area_mod) % P;
    ll u, v;
    if (twice_area % 2 == 0) {
        u = twice_area / 2;
        v = 1;
    } else {
        u = twice_area;
        v = 2;
    }
    cout << "! " << u << " " << v << endl;
    return 0;
}
