#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    auto query = [&](long long x1, long long y1, long long x2, long long y2) -> double {
        cout << "query " << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
        double c;
        if (!(cin >> c)) exit(0);
        return c;
    };

    // Find a vertical line that intersects the disk.
    long long x0 = -1, x1 = -1;
    double c0 = 0, c1 = 0;
    for (long long x = 0; x <= 100000; x += 199) {
        double c = query(x, 0, x, 100000);
        if (c > 1e-6) {
            x0 = x;
            c0 = c;
            break;
        }
    }
    if (x0 == -1) {
        double c = query(100000, 0, 100000, 100000);
        if (c > 1e-6) {
            x0 = 100000;
            c0 = c;
        }
    }
    if (x0 == -1) {
        for (long long x = 0; x <= 100000; ++x) {
            double c = query(x, 0, x, 100000);
            if (c > 1e-6) {
                x0 = x;
                c0 = c;
                break;
            }
        }
    }

    // Find another vertical line close to x0.
    if (x0 + 1 <= 100000) {
        double c = query(x0 + 1, 0, x0 + 1, 100000);
        if (c > 1e-6) {
            x1 = x0 + 1;
            c1 = c;
        }
    }
    if (x1 == -1 && x0 - 1 >= 0) {
        double c = query(x0 - 1, 0, x0 - 1, 100000);
        if (c > 1e-6) {
            x1 = x0 - 1;
            c1 = c;
        }
    }
    if (x1 == -1) {
        for (int d = 2; d <= 10; ++d) {
            if (x0 + d <= 100000) {
                double c = query(x0 + d, 0, x0 + d, 100000);
                if (c > 1e-6) {
                    x1 = x0 + d;
                    c1 = c;
                    break;
                }
            }
            if (x0 - d >= 0) {
                double c = query(x0 - d, 0, x0 - d, 100000);
                if (c > 1e-6) {
                    x1 = x0 - d;
                    c1 = c;
                    break;
                }
            }
        }
    }

    // Compute center x-coordinate and radius.
    long double a_ld = ((long double)x0 * x0 - (long double)x1 * x1 -
                        ((long double)c1 * c1 - (long double)c0 * c0) / 4.0L) /
                       (2.0L * (x0 - x1));
    long long a = llround(a_ld);
    long double r_ld = sqrt((long double)(x0 - a) * (x0 - a) +
                            ((long double)c0 / 2.0L) * ((long double)c0 / 2.0L));
    long long r = llround(r_ld);

    // Find a horizontal line that intersects the disk.
    long long y0 = -1, y1 = -1;
    double d0 = 0, d1 = 0;
    for (long long y = 0; y <= 100000; y += 199) {
        double c = query(0, y, 100000, y);
        if (c > 1e-6) {
            y0 = y;
            d0 = c;
            break;
        }
    }
    if (y0 == -1) {
        double c = query(0, 100000, 100000, 100000);
        if (c > 1e-6) {
            y0 = 100000;
            d0 = c;
        }
    }
    if (y0 == -1) {
        for (long long y = 0; y <= 100000; ++y) {
            double c = query(0, y, 100000, y);
            if (c > 1e-6) {
                y0 = y;
                d0 = c;
                break;
            }
        }
    }

    // Find another horizontal line close to y0.
    if (y0 + 1 <= 100000) {
        double c = query(0, y0 + 1, 100000, y0 + 1);
        if (c > 1e-6) {
            y1 = y0 + 1;
            d1 = c;
        }
    }
    if (y1 == -1 && y0 - 1 >= 0) {
        double c = query(0, y0 - 1, 100000, y0 - 1);
        if (c > 1e-6) {
            y1 = y0 - 1;
            d1 = c;
        }
    }
    if (y1 == -1) {
        for (int d = 2; d <= 10; ++d) {
            if (y0 + d <= 100000) {
                double c = query(0, y0 + d, 100000, y0 + d);
                if (c > 1e-6) {
                    y1 = y0 + d;
                    d1 = c;
                    break;
                }
            }
            if (y0 - d >= 0) {
                double c = query(0, y0 - d, 100000, y0 - d);
                if (c > 1e-6) {
                    y1 = y0 - d;
                    d1 = c;
                    break;
                }
            }
        }
    }

    // Compute center y-coordinate.
    long double b_ld = ((long double)y0 * y0 - (long double)y1 * y1 -
                        ((long double)d1 * d1 - (long double)d0 * d0) / 4.0L) /
                       (2.0L * (y0 - y1));
    long long b = llround(b_ld);

    cout << "answer " << a << " " << b << " " << r << endl;
    return 0;
}
