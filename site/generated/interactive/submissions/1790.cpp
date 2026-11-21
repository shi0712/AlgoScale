#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Point {
    ll x, y;

    bool operator<(const Point& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

static const ll MOD = 1000000007LL;

ll cross2(const Point& a, const Point& b, const Point& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

vector<Point> convex_hull(vector<Point> p) {
    sort(p.begin(), p.end());
    p.erase(unique(p.begin(), p.end()), p.end());

    if (p.size() <= 1) return p;

    vector<Point> h;

    for (const Point& pt : p) {
        while (h.size() >= 2 && cross2(h[h.size() - 2], h.back(), pt) <= 0)
            h.pop_back();
        h.push_back(pt);
    }

    int lower_size = (int)h.size();

    for (int i = (int)p.size() - 2; i >= 0; --i) {
        const Point& pt = p[i];
        while ((int)h.size() > lower_size &&
               cross2(h[h.size() - 2], h.back(), pt) <= 0)
            h.pop_back();
        h.push_back(pt);
    }

    h.pop_back();
    return h;
}

ll polygon_area2(const vector<Point>& p) {
    int n = (int)p.size();
    if (n <= 2) return 0;

    __int128 s = 0;
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        s += (__int128)p[i].x * p[j].y - (__int128)p[i].y * p[j].x;
    }

    if (s < 0) s = -s;
    return (ll)s;
}

bool between(const Point& a, const Point& b, const Point& p) {
    return min(a.x, b.x) <= p.x && p.x <= max(a.x, b.x) &&
           min(a.y, b.y) <= p.y && p.y <= max(a.y, b.y);
}

bool on_segment(const Point& a, const Point& b, const Point& p) {
    return cross2(a, b, p) == 0 && between(a, b, p);
}

ll mod_pow(ll a, ll e) {
    ll r = 1;
    while (e > 0) {
        if (e & 1) r = (__int128)r * a % MOD;
        a = (__int128)a * a % MOD;
        e >>= 1;
    }
    return r;
}

ll mod_decimal(const string& s) {
    ll r = 0;
    for (char c : s) {
        if ('0' <= c && c <= '9') {
            r = (r * 10 + (c - '0')) % MOD;
        }
    }
    return r;
}

void print_answer(ll area2) {
    if (area2 % 2 == 0) {
        cout << "! " << area2 / 2 << " 1\n";
    } else {
        cout << "! " << area2 << " 2\n";
    }
    cout.flush();
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<Point> pts(n);
    for (int i = 0; i < n; ++i) {
        cin >> pts[i].x >> pts[i].y;
    }

    vector<Point> hull = convex_hull(pts);

    if (hull.size() <= 2) {
        print_answer(0);
    }

    bool all_on_boundary = true;

    for (const Point& p : pts) {
        bool ok = false;

        for (int i = 0; i < (int)hull.size(); ++i) {
            int j = (i + 1) % (int)hull.size();
            if (on_segment(hull[i], hull[j], p)) {
                ok = true;
                break;
            }
        }

        if (!ok) {
            all_on_boundary = false;
            break;
        }
    }

    if (all_on_boundary) {
        print_answer(polygon_area2(hull));
    }

    vector<int> xs;
    xs.reserve(n);

    for (const Point& p : pts) {
        xs.push_back((int)p.x);
    }

    sort(xs.begin(), xs.end());
    xs.erase(unique(xs.begin(), xs.end()), xs.end());

    if (xs.size() <= 1) {
        print_answer(0);
    }

    ll area2_mod = 0;

    for (int i = 0; i + 1 < (int)xs.size(); ++i) {
        ll a = xs[i];
        ll b = xs[i + 1];

        ll sum = a + b;
        ll p, q;

        if (sum % 2 == 0) {
            p = sum / 2;
            q = 1;
        } else {
            p = sum;
            q = 2;
        }

        cout << "? " << p << " " << q << '\n';
        cout.flush();

        string rstr, sstr;

        if (!(cin >> rstr)) return 0;
        if (rstr[0] == '-') return 0;

        if (!(cin >> sstr)) return 0;
        if (sstr[0] == '-') return 0;

        ll rmod = mod_decimal(rstr);
        ll smod = mod_decimal(sstr);

        ll inv_s = mod_pow(smod, MOD - 2);

        ll width = b - a;
        ll factor = (2LL * (width % MOD)) % MOD;

        ll term = factor * rmod % MOD * inv_s % MOD;

        area2_mod += term;
        area2_mod %= MOD;
    }

    print_answer(area2_mod);
    return 0;
}
