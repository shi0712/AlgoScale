#include <bits/stdc++.h>
using namespace std;

static constexpr double PI = acos(-1.0);
static constexpr double INF = 1e100;
static constexpr double NEG = -1e100;
static constexpr double EPS = 1e-12;

struct Solver {
    double d, v, w, V, W, t;
    double alpha, beta, gamma;

    double fang(double x) {
        return x - sin(x) * cos(x);
    }

    // Solve A*sqrt(u^2+d^2) + B*u = t, u >= 0.
    // Returns -1 if impossible.
    double solve_u(double A, double B, double tt, double dd) {
        double minT = A * dd;
        if (tt < minT - 1e-10) return -1.0;
        if (tt <= minT + 1e-12) return 0.0;

        double u = (tt - minT) / (A + B);
        if (u < 0) u = 0;

        for (int it = 0; it < 5; ++it) {
            double h = sqrt(u * u + dd * dd);
            double f = A * h + B * u - tt;
            double fp = A * u / h + B;
            if (fp < 1e-18) break;
            double nu = u - f / fp;
            if (nu < 0) nu = 0;
            if (fabs(nu - u) < 1e-14 * max(1.0, u)) {
                u = nu;
                break;
            }
            u = nu;
        }

        double h = sqrt(u * u + dd * dd);
        double val = A * h + B * u;
        if (fabs(val - tt) > 1e-6 * max(1.0, tt)) {
            // fallback: bisection if Newton failed
            double lo = 0, hi = max(1.0, tt / max(1e-12, B) + 1.0);
            while (A * sqrt(hi * hi + dd * dd) + B * hi < tt) hi *= 2;
            for (int it = 0; it < 80; ++it) {
                double mid = (lo + hi) * 0.5;
                double cur = A * sqrt(mid * mid + dd * dd) + B * mid;
                if (cur < tt) lo = mid;
                else hi = mid;
            }
            u = (lo + hi) * 0.5;
        }
        return u;
    }

    // value for P-active land cap with coast base.
    double evalLandP(double theta) {
        if (theta < 0 || theta > PI / 2 + 1e-12) return NEG;
        double s = sin(theta);
        double c = cos(theta);

        double A;
        if (s < 1e-12) {
            A = 2.0 * alpha;
        } else {
            A = 2.0 * alpha * theta / s;
        }
        double B = 2.0 * gamma;

        if (t < A * d - 1e-10) return NEG;
        double u = solve_u(A, B, t, d);
        if (u < -0.5) return NEG;
        if (u < 0) u = 0;

        // Convexity: the two side arcs must not go above P.
        if (u * s > d * c + 1e-7 * (1.0 + d + u)) return NEG;

        double cc = sqrt(u * u + d * d);
        double area;
        if (s < 1e-10) {
            area = u * d;
        } else {
            area = u * d + cc * cc / (2.0 * s * s) * fang(theta);
        }
        return V * area;
    }

    double solveLandPActive() {
        if (t <= 2.0 * alpha * d + 1e-12) return NEG;

        double l = 0.0, r = PI / 2;
        for (int it = 0; it < 45; ++it) {
            double m1 = l + (r - l) / 3.0;
            double m2 = r - (r - l) / 3.0;
            double f1 = evalLandP(m1);
            double f2 = evalLandP(m2);
            if (f1 < f2) l = m1;
            else r = m2;
        }
        double theta = (l + r) * 0.5;
        double best = evalLandP(theta);
        best = max(best, evalLandP(0.0));
        best = max(best, evalLandP(1e-8));
        return best;
    }

    // P-active mixed shape.
    // theta: half-angle of one land side arc.
    // psi: half-angle of the water cap.
    double evalMixedP(double theta, double psi) {
        if (theta < 0 || theta >= PI / 2 + 1e-12) return NEG;
        if (psi <= 1e-14 || psi >= PI - 1e-14) return NEG;

        double s = sin(theta);
        double c = cos(theta);
        double sp = sin(psi);

        double A;
        if (s < 1e-12) A = 2.0 * alpha;
        else A = 2.0 * alpha * theta / s;

        double B = 2.0 * beta * psi / sp;

        if (t < A * d - 1e-10) return NEG;
        double u = solve_u(A, B, t, d);
        if (u < -0.5) return NEG;
        if (u < 0) u = 0;

        // Convexity with respect to the fixed point P.
        if (u * s > d * c + 1e-7 * (1.0 + d + u)) return NEG;

        double cc = sqrt(u * u + d * d);

        double landArea;
        if (s < 1e-10) {
            landArea = u * d;
        } else {
            landArea = u * d + cc * cc / (2.0 * s * s) * fang(theta);
        }

        double cp = cos(psi);
        double waterArea;
        if (u < 1e-15) {
            waterArea = 0.0;
        } else {
            double r = u / sp;
            waterArea = r * r * (psi - sp * cp);
        }

        return V * landArea + W * waterArea;
    }

    double optPsiForTheta(double theta, double low, double high) {
        // Ternary search on one interval.  If no feasible point exists,
        // this returns NEG.  Valid region is assumed to be toward higher psi
        // for the major interval; the same routine is also used for the minor
        // interval, where invalid regions are handled similarly.
        double l = low, r = high;
        double best = NEG;

        for (int it = 0; it < 24; ++it) {
            double m1 = l + (r - l) / 3.0;
            double m2 = r - (r - l) / 3.0;
            double f1 = evalMixedP(theta, m1);
            double f2 = evalMixedP(theta, m2);

            best = max(best, f1);
            best = max(best, f2);

            if (f1 <= NEG / 2 && f2 <= NEG / 2) {
                // both invalid; feasible region is usually to the right
                l = m1;
            } else if (f1 <= NEG / 2) {
                l = m1;
            } else if (f2 <= NEG / 2) {
                r = m2;
            } else {
                if (f1 < f2) l = m1;
                else r = m2;
            }
        }

        double mid = (l + r) * 0.5;
        best = max(best, evalMixedP(theta, mid));
        best = max(best, evalMixedP(theta, low));
        best = max(best, evalMixedP(theta, high));
        return best;
    }

    double optPsi(double theta) {
        double ans = NEG;
        ans = max(ans, optPsiForTheta(theta, 1e-10, PI / 2));
        ans = max(ans, optPsiForTheta(theta, PI / 2, PI - 1e-10));
        // Also check a few points, useful for degenerate full-water limit.
        ans = max(ans, evalMixedP(theta, PI - 1e-8));
        ans = max(ans, evalMixedP(theta, 1e-8));
        return ans;
    }

    double solveMixedPActive() {
        if (t <= 2.0 * alpha * d + 1e-12) return NEG;

        // theta must satisfy A(theta) * d < t, A(theta)=2*alpha*theta/sin(theta).
        auto Ad = [&](double theta) -> double {
            double s = sin(theta);
            if (s < 1e-14) return 2.0 * alpha * d;
            return 2.0 * alpha * theta / s * d;
        };

        double thetaMax;
        if (Ad(PI / 2 - 1e-10) <= t) {
            thetaMax = PI / 2 - 1e-8;
        } else {
            double lo = 0.0, hi = PI / 2 - 1e-10;
            for (int it = 0; it < 60; ++it) {
                double mid = (lo + hi) * 0.5;
                if (Ad(mid) < t) lo = mid;
                else hi = mid;
            }
            thetaMax = lo * 0.999999;
        }
        if (thetaMax <= 1e-12) return NEG;

        double l = 0.0, r = thetaMax;
        double best = NEG;

        // A few endpoint / boundary checks.
        best = max(best, optPsi(0.0));
        best = max(best, optPsi(thetaMax));

        for (int it = 0; it < 22; ++it) {
            double m1 = l + (r - l) / 3.0;
            double m2 = r - (r - l) / 3.0;
            double f1 = optPsi(m1);
            double f2 = optPsi(m2);

            best = max(best, f1);
            best = max(best, f2);

            if (f1 < f2) l = m1;
            else r = m2;
        }

        double theta = (l + r) * 0.5;
        best = max(best, optPsi(theta));
        return best;
    }

    double solve() {
        alpha = 1.0 / v;
        beta = 1.0 / w;
        gamma = min(alpha, beta);

        double ans = V * (v * t) * (v * t) / (4.0 * PI); // land-only circle

        // Unconstrained land cap with a coast base.
        {
            double l = 0.0, r = PI;
            auto capValue = [&](double phi) -> double {
                double den = 2.0 * (alpha * phi + gamma * sin(phi));
                if (den <= 0) return NEG;
                double f = fang(phi);
                if (f < 0) return NEG;
                return V * t * t * f / (den * den);
            };

            for (int it = 0; it < 45; ++it) {
                double m1 = l + (r - l) / 3.0;
                double m2 = r - (r - l) / 3.0;
                double f1 = capValue(m1);
                double f2 = capValue(m2);
                if (f1 < f2) l = m1;
                else r = m2;
            }
            double phi = (l + r) * 0.5;
            double den = 2.0 * (alpha * phi + gamma * sin(phi));
            double R = t / den;
            double H = R * (1.0 - cos(phi));
            if (H >= d - 1e-7) {
                ans = max(ans, capValue(phi));
            }
        }

        // Unconstrained two-medium shape, using the stationary-angle formula.
        bool feasibleMixed = false;
        {
            double p = (alpha * W) / (beta * V);
            double rr = alpha / beta;
            double denom = p * p - rr * rr;

            if (fabs(denom) > 1e-14) {
                double C2 = (p * p - 1.0) / denom;
                if (C2 >= -1e-10 && C2 <= 1.0 + 1e-10) {
                    C2 = min(1.0, max(0.0, C2));
                    double baseC = sqrt(C2);

                    for (int sg = -1; sg <= 1; sg += 2) {
                        double C = sg * baseC;
                        double sinphi2 = 1.0 - C * C;
                        if (sinphi2 < -1e-12) continue;
                        sinphi2 = max(0.0, sinphi2);
                        double sinphi = sqrt(sinphi2);
                        if (sinphi < 1e-12) continue;

                        double sinpsi = p * sinphi;
                        if (sinpsi < -1e-10 || sinpsi > 1.0 + 1e-10) continue;
                        sinpsi = min(1.0, max(0.0, sinpsi));

                        double cospsi = -rr * C;
                        if (fabs(cospsi) > 1.0 + 1e-8) continue;
                        cospsi = min(1.0, max(-1.0, cospsi));

                        double phi = atan2(sinphi, C);
                        double psi = atan2(sinpsi, cospsi);

                        if (psi < -1e-12 || psi > PI + 1e-12) continue;
                        if (phi < -1e-12 || phi > PI + 1e-12) continue;

                        if (sinpsi < 1e-14) continue;
                        double q = sinphi / sinpsi;
                        double Ccoeff = 2.0 * (alpha * phi + beta * q * psi);
                        if (Ccoeff <= 0) continue;

                        double R = t / Ccoeff;
                        double areaL = R * R * fang(phi);
                        double rWater = R * q;
                        double areaW = rWater * rWater * fang(psi);
                        double val = V * areaL + W * areaW;

                        double H = R * (1.0 - C); // land top height above coast
                        if (H >= d - 1e-7) {
                            ans = max(ans, val);
                            feasibleMixed = true;
                        }
                    }
                }
            }
        }

        // P-active land cap with coast base.
        {
            double val = solveLandPActive();
            ans = max(ans, val);
        }

        // P-active mixed shape.
        // If the unconstrained mixed optimum is already feasible, then this
        // cannot improve it.  We still run it when there is no feasible
        // unconstrained mixed candidate or when it is clearly potentially useful.
        if (!feasibleMixed) {
            double val = solveMixedPActive();
            ans = max(ans, val);
        }

        return ans;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    cout.setf(ios::fixed);
    cout << setprecision(3);

    while (T--) {
        Solver sol;
        cin >> sol.d >> sol.v >> sol.w >> sol.V >> sol.W >> sol.t;
        double ans = sol.solve();
        if (ans < 0 && ans > -1e-9) ans = 0;
        if (fabs(ans) < 0.0005) ans = 0;
        cout << ans + 1e-9 << '\n';
    }
    return 0;
}
