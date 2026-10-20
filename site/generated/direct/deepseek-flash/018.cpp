#include <bits/stdc++.h>
using namespace std;

struct Point {
    double x, y;
    Point() : x(0), y(0) {}
    Point(double _x, double _y) : x(_x), y(_y) {}
    Point operator+(const Point& p) const { return Point(x + p.x, y + p.y); }
    Point operator-(const Point& p) const { return Point(x - p.x, y - p.y); }
    Point operator*(double d) const { return Point(x * d, y * d); }
    Point operator/(double d) const { return Point(x / d, y / d); }
    double operator*(const Point& p) const { return x * p.x + y * p.y; }
    double operator^(const Point& p) const { return x * p.y - y * p.x; }
    double len() const { return hypot(x, y); }
    double len2() const { return x * x + y * y; }
    Point rot90() const { return Point(-y, x); }
    Point norm() const { double l = len(); return l < 1e-12 ? Point(0, 0) : Point(x / l, y / l); }
};

const double PI = acos(-1.0);
const double INF = 1e100;

int n;
double K;
vector<Point> p;

vector<Point> convex_hull(vector<Point> pts) {
    sort(pts.begin(), pts.end(), [](const Point& a, const Point& b) {
        if (fabs(a.x - b.x) > 1e-12) return a.x < b.x;
        return a.y < b.y;
    });
    pts.erase(unique(pts.begin(), pts.end(), [](const Point& a, const Point& b) {
        return fabs(a.x - b.x) < 1e-12 && fabs(a.y - b.y) < 1e-12;
    }), pts.end());
    if (pts.size() <= 1) return pts;
    vector<Point> h;
    for (int i = 0; i < (int)pts.size(); i++) {
        while (h.size() >= 2) {
            Point a = h[h.size() - 1] - h[h.size() - 2];
            Point b = pts[i] - h[h.size() - 2];
            if ((a ^ b) <= 1e-12) h.pop_back();
            else break;
        }
        h.push_back(pts[i]);
    }
    int lower = (int)h.size();
    for (int i = (int)pts.size() - 2; i >= 0; i--) {
        while ((int)h.size() > lower) {
            Point a = h[h.size() - 1] - h[h.size() - 2];
            Point b = pts[i] - h[h.size() - 2];
            if ((a ^ b) <= 1e-12) h.pop_back();
            else break;
        }
        h.push_back(pts[i]);
    }
    h.pop_back();
    return h;
}

struct EdgeInterval {
    int i;
    double l, r;
    bool operator<(const EdgeInterval& o) const { return l < o.l; }
};

vector<Point> H;
int m;
vector<double> perim;
vector<double> pref;
vector<EdgeInterval> nearEdges, farEdges;
double angleStart;
double angleSpan;

double normAngle(double a) {
    while (a < 0) a += 2 * PI;
    while (a >= 2 * PI) a -= 2 * PI;
    return a;
}

// prepare angular intervals for a convex polygon and source at origin
void prepareIntervals() {
    m = (int)H.size();
    perim.resize(m);
    double tot = 0;
    pref.resize(m + 1);
    pref[0] = 0;
    for (int i = 0; i < m; i++) {
        int j = (i + 1) % m;
        double d = (H[j] - H[i]).len();
        perim[i] = d;
        pref[i + 1] = pref[i] + d;
        tot += d;
    }
    vector<double> ang(m);
    for (int i = 0; i < m; i++) {
        ang[i] = atan2(H[i].y, H[i].x);
        if (ang[i] < 0) ang[i] += 2 * PI;
    }
    vector<double> tmp = ang;
    sort(tmp.begin(), tmp.end());
    double gap = -1;
    double gapStart = 0;
    for (int i = 0; i < m; i++) {
        double a1 = tmp[i];
        double a2 = tmp[(i + 1) % m];
        if (i == m - 1) a2 += 2 * PI;
        double g = a2 - a1;
        if (g > gap) {
            gap = g;
            gapStart = a2;
        }
    }
    angleStart = normAngle(gapStart);
    angleSpan = 2 * PI - gap;
    vector<double> u(m);
    for (int i = 0; i < m; i++) {
        double a = ang[i];
        double rel = a - angleStart;
        while (rel < -1e-12) rel += 2 * PI;
        while (rel >= 2 * PI - 1e-12) rel -= 2 * PI;
        u[i] = rel;
    }
    nearEdges.clear();
    farEdges.clear();
    for (int i = 0; i < m; i++) {
        int j = (i + 1) % m;
        Point e = H[j] - H[i];
        double cr = e ^ (-H[i]);
        double a1 = u[i], a2 = u[j];
        if (a1 > a2) swap(a1, a2);
        EdgeInterval ei{i, a1, a2};
        if (cr < -1e-10) {
            nearEdges.push_back(ei);
        } else {
            farEdges.push_back(ei);
        }
    }
    sort(nearEdges.begin(), nearEdges.end());
    sort(farEdges.begin(), farEdges.end());
}

Point rayIntersectEdge(double theta, int idx) {
    int j = (idx + 1) % m;
    Point A = H[idx], B = H[j];
    Point u(cos(theta), sin(theta));
    Point v = B - A;
    double den = u ^ v;
    if (fabs(den) < 1e-14) return Point(INF, INF);
    double t = (A ^ v) / den; // O + t u = A + s v
    // double s = (A ^ u) / den;
    Point P = u * t;
    return P;
}

bool rayPolygonInterval(double theta, double& ta, double& tb) {
    double rel = theta - angleStart;
    while (rel < -1e-12) rel += 2 * PI;
    while (rel >= 2 * PI - 1e-12) rel -= 2 * PI;
    if (rel < -1e-12 || rel > angleSpan + 1e-12) return false;
    auto findIn = [&](vector<EdgeInterval>& arr) -> double {
        if (arr.empty()) return INF;
        int lo = 0, hi = (int)arr.size() - 1, pos = -1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if (arr[mid].r + 1e-12 >= rel) {
                pos = mid;
                hi = mid - 1;
            } else lo = mid + 1;
        }
        if (pos == -1) pos = (int)arr.size() - 1;
        // due to numerical reasons, try neighbors
        for (int d = -2; d <= 2; d++) {
            int k = pos + d;
            if (k < 0 || k >= (int)arr.size()) continue;
            double l = arr[k].l, r = arr[k].r;
            if (rel >= l - 1e-10 && rel <= r + 1e-10) {
                Point P = rayIntersectEdge(angleStart + rel, arr[k].i);
                return P.len();
            }
        }
        // fallback: scan small? use nearest interval
        double best = INF;
        for (int k = max(0, pos - 3); k <= min((int)arr.size() - 1, pos + 3); k++) {
            Point P = rayIntersectEdge(angleStart + rel, arr[k].i);
            best = min(best, P.len());
        }
        return best;
    };
    double d1 = findIn(nearEdges);
    double d2 = findIn(farEdges);
    if (d1 >= INF / 2) return false;
    if (d2 >= INF / 2) d2 = d1;
    ta = min(d1, d2);
    tb = max(d1, d2);
    return true;
}

// tangent vertices from an external point P to convex polygon H, return up to 2 indices
vector<int> tangents(Point P) {
    int N = H.size();
    vector<int> res;
    for (int i = 0; i < N; i++) {
        int prv = (i - 1 + N) % N;
        int nxt = (i + 1) % N;
        Point v = H[i] - P;
        Point a = H[prv] - P;
        Point b = H[nxt] - P;
        double c1 = v ^ a;
        double c2 = v ^ b;
        if (c1 * c2 <= 1e-10) {
            res.push_back(i);
        }
    }
    if (res.size() < 2) {
        // fallback by angular extrema
        double mx = -1e100, mn = 1e100;
        int imx = 0, imn = 0;
        for (int i = 0; i < N; i++) {
            double a = atan2(H[i].y - P.y, H[i].x - P.x);
            if (a > mx) { mx = a; imx = i; }
            if (a < mn) { mn = a; imn = i; }
        }
        res.clear();
        res.push_back(imx);
        res.push_back(imn);
    }
    sort(res.begin(), res.end());
    res.erase(unique(res.begin(), res.end()), res.end());
    if (res.size() > 2) res.resize(2);
    return res;
}

double perimeterDist(int a, int b) {
    if (a <= b) return pref[b] - pref[a];
    return pref[m] - (pref[a] - pref[b]);
}

double shortestPath(Point P) {
    // direct if segment OP does not enter polygon interior
    // Use ray / inside test: if P outside and segment OP has no intersection with interior.
    Point O(0, 0);
    double direct = P.len();
    // check if segment OP intersects the convex polygon interior.
    // Since O is outside (assumed), it intersects interior if ray OP enters before P.
    double ang = atan2(P.y, P.x);
    if (ang < 0) ang += 2 * PI;
    double a, b;
    bool hit = rayPolygonInterval(ang, a, b);
    bool directValid = true;
    if (hit && b < direct - 1e-10) directValid = false;
    if (directValid) return direct;
    vector<int> tp = tangents(P);
    vector<int> to = tangents(O);
    double ans = INF;
    for (int A : to) {
        double dO = (H[A] - O).len();
        for (int B : tp) {
            double dP = (H[B] - P).len();
            double d1 = dO + perimeterDist(A, B) + dP;
            double d2 = dO + (pref[m] - perimeterDist(A, B)) + dP;
            ans = min(ans, min(d1, d2));
        }
    }
    if (ans > INF / 2) ans = direct;
    return ans;
}

double rayMaxFar(double theta, double b) {
    if (b > K) return b;
    if (shortestPath(Point(cos(theta) * b, sin(theta) * b)) > K) return b;
    double lo = b, hi = K;
    for (int it = 0; it < 50; it++) {
        double mid = (lo + hi) * 0.5;
        double d = shortestPath(Point(cos(theta) * mid, sin(theta) * mid));
        if (d <= K) lo = mid;
        else hi = mid;
    }
    return lo;
}

double integrand(double theta) {
    double a, b;
    bool hit = rayPolygonInterval(theta, a, b);
    double nearR = K;
    double farContribution = 0;
    if (hit) {
        double near = min(a, K);
        nearR = near;
        if (b < K) {
            double rmax = rayMaxFar(theta, b);
            if (rmax > b) {
                farContribution = 0.5 * (rmax * rmax - b * b);
            }
        }
    }
    double nearContribution = 0.5 * nearR * nearR;
    return nearContribution + farContribution;
}

double simpson(double l, double r, double fl, double fm, double fr) {
    return (r - l) * (fl + 4 * fm + fr) / 6.0;
}

double adaptive(double l, double r, double fl, double fm, double fr, double eps, int depth) {
    double mid = (l + r) * 0.5;
    double lm = (l + mid) * 0.5;
    double rm = (mid + r) * 0.5;
    double flm = integrand(lm);
    double frm = integrand(rm);
    double s1 = simpson(l, mid, fl, flm, fm);
    double s2 = simpson(mid, r, fm, frm, fr);
    double s = simpson(l, r, fl, fm, fr);
    if (depth <= 0 || fabs(s1 + s2 - s) < eps) {
        return s1 + s2 + (s1 + s2 - s) / 15.0;
    }
    return adaptive(l, mid, fl, flm, fm, eps * 0.5, depth - 1) +
           adaptive(mid, r, fm, frm, fr, eps * 0.5, depth - 1);
}

double integrateArea() {
    int N = 4096;
    double ans = 0;
    double step = 2 * PI / N;
    for (int i = 0; i < N; i++) {
        double l = i * step;
        double r = (i + 1) * step;
        double mid = (l + r) * 0.5;
        double fl = integrand(l);
        double fm = integrand(mid);
        double fr = integrand(r);
        double eps = 1e-8;
        ans += adaptive(l, r, fl, fm, fr, eps, 20);
    }
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cin >> n >> K;
    p.resize(n);
    for (int i = 0; i < n; i++) {
        cin >> p[i].x >> p[i].y;
    }
    H = convex_hull(p);
    if (H.size() < 3) {
        // Degenerate lake: no area
        cout << fixed << setprecision(12) << PI * K * K << '\n';
        return 0;
    }
    prepareIntervals();
    double area = integrateArea();
    cout << fixed << setprecision(12) << area << '\n';
    return 0;
}
