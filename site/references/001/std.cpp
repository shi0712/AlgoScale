#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

using Real = long double;
using Poly = vector<Real>;
const Real PI = acosl(-1.0L);

Poly add(Poly a, const Poly &b, Real sign = 1) {
    if (a.size() < b.size()) a.resize(b.size(), 0);
    for (size_t i = 0; i < b.size(); ++i) a[i] += sign * b[i];
    return a;
}
Poly mul(const Poly &a, const Poly &b) {
    Poly c(a.size() + b.size() - 1, 0);
    for (size_t i = 0; i < a.size(); ++i)
        for (size_t j = 0; j < b.size(); ++j)
            c[i + j] = (a[i] * b[j] + c[i + j]);
    return c;
}
Poly scale(Poly a, Real k) {
    for (Real &x : a) x *= k;
    return a;
}
Real eval(const Poly &a, Real x) {
    Real y = 0;
    for (int i = (int)a.size() - 1; i >= 0; --i)
        y = (y * x + a[i]);
    return y;
}
void normalize(Poly &a) {
    Real m = 0;
    for (Real x : a) m = max(m, fabsl(x));
    if (m == 0) { a = {0}; return; }
    for (Real &x : a) x /= m;
    while (a.size() > 1 && fabsl(a.back()) < 1e-24L)
        a.pop_back();
}
void uniquePoints(vector<Real> &a) {
    sort(a.begin(), a.end());
    vector<Real> b;
    for (Real x : a)
        if (b.empty() || x - b.back() > 1e-18L) b.push_back(x);
    a.swap(b);
}

// 保留各阶导数的分割点，多保留一些点不会影响后续求解。
vector<Real> polynomialCuts(Poly a) {
    normalize(a);
    int n = (int)a.size() - 1;
    if (n <= 0) return {};
    if (n == 1) {
        Real x = -a[0] / a[1];
        if (x > 0 && x < 1) return {x};
        return {};
    }
    Poly da(n);
    for (int i = 1; i <= n; ++i) da[i - 1] = i * a[i];
    vector<Real> cuts = polynomialCuts(da);
    vector<Real> result = cuts;
    cuts.insert(cuts.begin(), 0);
    cuts.push_back(1);
    for (size_t i = 1; i < cuts.size(); ++i) {
        Real l = cuts[i - 1], r = cuts[i];
        Real fl = eval(a, l), fr = eval(a, r);
        if (fl == 0 || fr == 0 || (fl < 0) == (fr < 0)) continue;
        for (int j = 0; j < 90; ++j) {
            Real mid = (l + r) / 2;
            if (mid == l || mid == r) break;
            Real fm = eval(a, mid);
            if ((fm < 0) == (fl < 0)) l = mid;
            else r = mid;
        }
        result.push_back((l + r) / 2);
    }
    uniquePoints(result);
    return result;
}

// a(x) + b(x)*u，其中 u^2 = U(x)。
struct Quad { Poly a, b; };
struct Algebra {
    Poly U;
    Quad plus(Quad x, const Quad &y, Real sign = 1) const {
        return {add(x.a, y.a, sign), add(x.b, y.b, sign)};
    }
    Quad times(const Quad &x, const Quad &y) const {
        return {add(mul(x.a, y.a), mul(U, mul(x.b, y.b))),
                add(mul(x.a, y.b), mul(x.b, y.a))};
    }
    Quad scaled(Quad x, Real k) const {
        return {scale(x.a, k), scale(x.b, k)};
    }
    Quad sqr(const Quad &x) const { return times(x, x); }
};

// 用 s = s0 + s1*x 将有效的 sin(gamma) 区间缩放到 [0,1]。
Poly criticalPolynomial(Real h, Real q, Real p, Real s0, Real s1) {
    // h=1 时直接消元，避免展开式的退化抵消。
    if (h == 1) {
        Real a = 1 - q;
        return {-2 * a * p, p * p * (1 + a * a) + a * a,
                0, -a * a * p * p};
    }
    Real h2 = h * h;
    Poly s{s0, s1};
    Real u0 = (s0 == 1 ? 1 : (1 - h) * (1 + h));
    Algebra alg{{u0, 2 * h2 * s0 * s1, h2 * s1 * s1}};
    Quad one{{1}, {0}};
    Quad z{scale(s, -q), {1}};
    // K、M 同除以 q，减小中间系数的数量级。
    Quad K{scale(s, -h2 / q), {1}};
    Quad M{scale(s, -1 / q), add(Poly{1}, scale(s, p / q))};
    Quad c2{{1 - s0 * s0, -2 * s0 * s1, -s1 * s1}, {0}};
    Quad N = alg.plus(alg.scaled(z, p), one, -1);
    Quad E = alg.plus(
        alg.scaled(alg.times(alg.plus(one, alg.sqr(z), -1),
                             alg.sqr(M)), h2),
        alg.times(alg.times(c2, alg.sqr(K)), alg.sqr(N)), -1);
    return add(mul(E.a, E.a), mul(alg.U, mul(E.b, E.b)), -1);
}

struct Geometry { Real z, D, B; };
struct Solver {
    Real d, v, w, V, W, t, h, q, p, answer;
    Geometry geometry(Real gamma) const {
        Real s = sinl(gamma), c = cosl(gamma);
        if (gamma == 0) { s = 0; c = 1; }
        if (gamma == PI) { s = 0; c = -1; }
        Real b = h * c;
        Real u = sqrtl(max(0.0L, (1 - b) * (1 + b)));
        Real z = clamp(u - q * s, 0.0L, 1.0L);
        Real A = sqrtl(max(0.0L, (1 - z) * (1 + z)));
        Real D = A - b;
        if (b >= 0 && A + b > 0) D = q * s * (u + z) / (A + b);
        Real sn = (b >= 0 ? u * D + b * q * s : u * A - b * z);
        Real cs = b * A + u * z;
        Real delta = atan2l(max(0.0L, sn), cs);
        return {z, D, delta + q * h * (PI - gamma)};
    }
    Real f(Real gamma) const {
        Geometry a = geometry(gamma);
        return a.B - p * a.D;
    }
    void takePinned(Real gamma) {
        Geometry a = geometry(gamma);
        if (a.D <= 0) return;
        Real time = 2 * d * a.B / (v * a.D);
        if (fabsl(time - t) > 1e-8L * max(1.0L, t)) return;
        Real R = d / a.D;
        answer = max(answer, V * R * (v * t / 2 - d * a.z));
    }
    Real solve() {
        h = v / w;
        q = v * V / (w * W);
        p = v * t / (2 * d);
        answer = V * (v * t) * (v * t) / (4 * PI);
        if (p <= 1 || (h >= 1 && q >= 1)) return answer;

        // 起点不限制最优形状的最高点。
        Real den = (h - q) * (h + q);
        if (den != 0) {
            Real c2 = (1 - q) * (1 + q) / den;
            if (c2 >= 0 && c2 <= 1 && h * h * c2 <= 1) {
                Real c0 = sqrtl(c2);
                for (int sign : {-1, 1}) {
                    Real c = sign * c0;
                    Real beta = acosl(clamp(h * c, -1.0L, 1.0L));
                    Real sea = PI - acosl(clamp(c, -1.0L, 1.0L));
                    Real B = beta + q * h * sea;
                    if (B <= 0) continue;
                    Real R = v * t / (2 * B);
                    if (R * (1 - h * c) >= d - 1e-13L)
                        answer = max(answer, V * v * R * t / 2);
                }
            }
        }

        vector<pair<Real, Real>> domains;
        Real s0 = 0, s1 = 1;
        if (h < 1 && q > 1) {
            Real smax = sqrtl((1 - h) * (1 + h) / ((q - h) * (q + h)));
            smax = clamp(smax, 0.0L, 1.0L);
            Real a = asinl(smax);
            domains = {{0, a}, {PI - a, PI}};
            s1 = smax;
        } else if (h > 1 && q < 1) {
            Real c2 = (1 - q) * (1 + q) / ((h - q) * (h + q));
            c2 = clamp(c2, 0.0L, 1.0L);
            Real a = acosl(sqrtl(c2));
            domains = {{a, PI - a}};
            s0 = 1;
            s1 = -c2 / (1 + sqrtl(1 - c2));
        } else {
            domains = {{0, PI}};
        }

        vector<Real> allCuts{0, PI / 2, PI};
        for (Real x : polynomialCuts(criticalPolynomial(h, q, p, s0, s1))) {
            Real s = clamp((s1 * x + s0), 0.0L, 1.0L);
            Real a = asinl(s);
            allCuts.push_back(a);
            allCuts.push_back(PI - a);
        }
        for (auto interval : domains) {
            Real lo = interval.first, hi = interval.second;
            vector<Real> cuts{lo, hi};
            for (Real x : allCuts)
                if (x > lo && x < hi) cuts.push_back(x);
            uniquePoints(cuts);
            for (Real x : cuts)
                if (fabsl(f(x)) < 1e-15L) takePinned(x);
            for (size_t i = 1; i < cuts.size(); ++i) {
                Real l = cuts[i - 1], r = cuts[i];
                Real fl = f(l), fr = f(r);
                if ((fl < 0) == (fr < 0)) continue;
                for (int j = 0; j < 90; ++j) {
                    Real mid = (l + r) / 2;
                    if (mid == l || mid == r) break;
                    if ((f(mid) < 0) == (fl < 0)) l = mid;
                    else r = mid;
                }
                takePinned((l + r) / 2);
            }
        }
        return answer;
    }
};
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    cin >> T;
    cout << fixed << setprecision(3);
    while (T--) {
        Solver solver;
        cin >> solver.d >> solver.v >> solver.w
            >> solver.V >> solver.W >> solver.t;
        cout << solver.solve() << '\n';
    }
}
