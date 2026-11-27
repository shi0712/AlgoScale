#include <bits/stdc++.h>
using namespace std;

static bool parseDecimal(const string& s, long double& value) {
    if (s.empty()) return false;

    size_t pos = 0;
    if (s[pos] == '+') ++pos;
    if (pos == s.size()) return false;

    value = 0;
    for (; pos < s.size(); ++pos) {
        if (s[pos] < '0' || s[pos] > '9') return false;
        value = value * 10 + (s[pos] - '0');
    }
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> xs(n);
    for (int i = 0; i < n; ++i) {
        int x, y;
        cin >> x >> y;
        xs[i] = x;
    }

    sort(xs.begin(), xs.end());
    xs.erase(unique(xs.begin(), xs.end()), xs.end());

    long double area = 0;
    long double correction = 0;

    for (int i = 0; i + 1 < (int)xs.size(); ++i) {
        int a = xs[i];
        int b = xs[i + 1];

        int p = a + b;
        int q = 2;
        int g = gcd(p, q);
        p /= g;
        q /= g;

        cout << "? " << p << ' ' << q << endl;

        string rs, ss;
        if (!(cin >> rs)) return 0;
        if (!rs.empty() && rs[0] == '-') return 0;
        if (!(cin >> ss)) return 0;
        if (!ss.empty() && ss[0] == '-') return 0;

        long double r, s;
        if (!parseDecimal(rs, r) || !parseDecimal(ss, s)) return 0;

        long double length = r / s;
        long double term = static_cast<long double>(b - a) * length;

        long double y = term - correction;
        long double t = area + y;
        correction = (t - area) - y;
        area = t;
    }

    // Integer-coordinate polygons have half-integer areas.
    long long twiceArea = llroundl(2.0L * area);
    if (twiceArea < 0) twiceArea = -twiceArea;

    long long u = twiceArea;
    long long v = 2;

    if (u == 0) {
        u = 0;
        v = 1;
    } else if (u % 2 == 0) {
        u /= 2;
        v = 1;
    }

    cout << "! " << u << ' ' << v << endl;
    return 0;
}
