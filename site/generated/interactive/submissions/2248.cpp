#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

const int64 MOD = 1000000007LL;

struct Point {
    int64 x, y;
    bool operator<(const Point& other) const {
        return x < other.x || (x == other.x && y < other.y);
    }
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

int64 cross(const Point& a, const Point& b, const Point& c) {
    return (b.x - a.x) * (c.y - a.y) -
           (b.y - a.y) * (c.x - a.x);
}

void printArea2(int64 area2) {
    if (area2 < 0) area2 = -area2;
    if (area2 % 2 == 0) {
        cout << "! " << area2 / 2 << " 1\n";
    } else {
        cout << "! " << area2 << " 2\n";
    }
    cout.flush();
}

int64 modPow(int64 a, int64 e) {
    int64 result = 1;
    while (e > 0) {
        if (e & 1) result = result * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return result;
}

int64 parseMod(const string& s) {
    int64 value = 0;
    for (char c : s) {
        if (c >= '0' && c <= '9') {
            value = (value * 10 + (c - '0')) % MOD;
        }
    }
    return value;
}

string normalizedNumber(string s) {
    size_t pos = 0;
    while (pos + 1 < s.size() && s[pos] == '0') ++pos;
    return s.substr(pos);
}

struct Rat {
    int64 n, d;
};

Rat makeRat(i128 n, i128 d) {
    if (d < 0) {
        n = -n;
        d = -d;
    }
    if (n < 0) n = -n;
    int64 nn = (int64)n;
    int64 dd = (int64)d;
    int64 g = std::gcd(nn, dd);
    return {nn / g, dd / g};
}

Rat subtractRat(const Rat& a, const Rat& b) {
    i128 n = (i128)a.n * b.d - (i128)b.n * a.d;
    i128 d = (i128)a.d * b.d;
    return makeRat(n, d);
}

Rat intersectionY(const Point& a, const Point& b, int64 p, int64 q) {
    i128 dx = (i128)b.x - a.x;
    i128 dy = (i128)b.y - a.y;
    i128 numerator = (i128)a.y * q * dx + (i128)(p - q * a.x) * dy;
    i128 denominator = (i128)q * dx;
    return makeRat(numerator, denominator);
}

Rat verticalLength(const vector<Point>& polygon, int64 p, int64 q) {
    vector<Rat> intersections;
    int m = (int)polygon.size();
    for (int i = 0; i < m; ++i) {
        const Point& a = polygon[i];
        const Point& b = polygon[(i + 1) % m];
        i128 left = (i128)p - q * a.x;
        i128 right = (i128)p - q * b.x;
        if (left * right < 0) {
            intersections.push_back(intersectionY(a, b, p, q));
        }
    }
    if (intersections.size() != 2) return {0, 1};
    return subtractRat(intersections[0], intersections[1]);
}

Rat triangleVerticalLength(const Point& a, const Point& b, const Point& c,
                           int64 p, int64 q) {
    Point triangle[3] = {a, b, c};
    vector<Rat> intersections;
    for (int i = 0; i < 3; ++i) {
        const Point& u = triangle[i];
        const Point& v = triangle[(i + 1) % 3];
        i128 left = (i128)p - q * u.x;
        i128 right = (i128)p - q * v.x;
        if (left * right < 0) {
            intersections.push_back(intersectionY(u, v, p, q));
        }
    }
    if (intersections.size() != 2) return {0, 1};
    return subtractRat(intersections[0], intersections[1]);
}

bool sameResponse(const Rat& value, const string& r, const string& s) {
    return normalizedNumber(r) == to_string(value.n) &&
           normalizedNumber(s) == to_string(value.d);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<Point> points(n);
    for (Point& p : points) {
        if (!(cin >> p.x >> p.y)) return 0;
    }

    vector<Point> sorted = points;
    sort(sorted.begin(), sorted.end());

    vector<Point> lower, upper;
    for (const Point& p : sorted) {
        while (lower.size() >= 2 &&
               cross(lower[lower.size() - 2], lower.back(), p) <= 0) {
            lower.pop_back();
        }
        lower.push_back(p);
    }
    for (int i = n - 1; i >= 0; --i) {
        const Point& p = sorted[i];
        while (upper.size() >= 2 &&
               cross(upper[upper.size() - 2], upper.back(), p) <= 0) {
            upper.pop_back();
        }
        upper.push_back(p);
    }

    vector<Point> hull;
    for (int i = 0; i + 1 < (int)lower.size(); ++i) hull.push_back(lower[i]);
    for (int i = 0; i + 1 < (int)upper.size(); ++i) hull.push_back(upper[i]);

    int64 hullArea2 = 0;
    int h = (int)hull.size();
    for (int i = 0; i < h; ++i) {
        const Point& a = hull[i];
        const Point& b = hull[(i + 1) % h];
        hullArea2 += a.x * b.y - a.y * b.x;
    }
    if (hullArea2 < 0) hullArea2 = -hullArea2;

    if (hullArea2 == 0) {
        printArea2(0);
        return 0;
    }

    bool allOnHull = true;
    for (const Point& p : points) {
        bool onBoundary = false;
        for (int i = 0; i < h && !onBoundary; ++i) {
            const Point& a = hull[i];
            const Point& b = hull[(i + 1) % h];
            if (cross(a, b, p) == 0) {
                int64 dot = (p.x - a.x) * (p.x - b.x) +
                            (p.y - a.y) * (p.y - b.y);
                if (dot <= 0) onBoundary = true;
            }
        }
        if (!onBoundary) {
            allOnHull = false;
            break;
        }
    }

    if (allOnHull) {
        printArea2(hullArea2);
        return 0;
    }

    bool oneInterior = (n == h + 1);
    Point interior{0, 0};
    vector<int64> candidateArea2;
    vector<char> active;

    if (oneInterior) {
        for (const Point& p : points) {
            bool isHullVertex = false;
            for (const Point& v : hull) {
                if (p == v) {
                    isHullVertex = true;
                    break;
                }
            }
            if (!isHullVertex) {
                interior = p;
                break;
            }
        }

        candidateArea2.resize(h);
        active.assign(h, 1);
        for (int i = 0; i < h; ++i) {
            candidateArea2[i] = std::llabs(
                cross(hull[i], hull[(i + 1) % h], interior));
        }

        bool same = true;
        for (int i = 1; i < h; ++i) {
            if (candidateArea2[i] != candidateArea2[0]) {
                same = false;
                break;
            }
        }
        if (same) {
            printArea2(hullArea2 - candidateArea2[0]);
            return 0;
        }
    }

    vector<int64> xs;
    for (const Point& p : points) xs.push_back(p.x);
    sort(xs.begin(), xs.end());
    xs.erase(unique(xs.begin(), xs.end()), xs.end());

    int slabCount = (int)xs.size() - 1;
    vector<int> order(max(0, slabCount));
    iota(order.begin(), order.end(), 0);

    if (oneInterior) {
        sort(order.begin(), order.end(), [&](int a, int b) {
            int64 da = std::llabs(xs[a] + xs[a + 1] - 2 * interior.x);
            int64 db = std::llabs(xs[b] + xs[b + 1] - 2 * interior.x);
            if (da != db) return da < db;
            return a < b;
        });
    }

    int64 twiceAreaMod = 0;
    int activeCount = h;

    for (int index : order) {
        int64 a = xs[index];
        int64 b = xs[index + 1];
        int64 p = a + b;
        int64 q = 2;
        int64 width = b - a;

        cout << "? " << p << " " << q << "\n";
        cout.flush();

        string rs, ss;
        if (!(cin >> rs)) return 0;
        if (rs == "-1") return 0;
        if (!(cin >> ss)) return 0;
        if (ss == "-1") return 0;

        int64 rMod = parseMod(rs);
        int64 sMod = parseMod(ss);
        int64 inverse = modPow(sMod, MOD - 2);
        int64 term = (2LL * width) % MOD;
        term = term * rMod % MOD;
        term = term * inverse % MOD;
        twiceAreaMod += term;
        if (twiceAreaMod >= MOD) twiceAreaMod -= MOD;

        if (oneInterior) {
            Rat hullLength = verticalLength(hull, p, q);

            for (int i = 0; i < h; ++i) {
                if (!active[i]) continue;
                Rat triangleLength = triangleVerticalLength(
                    hull[i], hull[(i + 1) % h], interior, p, q);
                Rat predicted = subtractRat(hullLength, triangleLength);
                if (!sameResponse(predicted, rs, ss)) {
                    active[i] = 0;
                    --activeCount;
                }
            }

            if (activeCount == 0) {
                oneInterior = false;
            } else {
                int64 commonArea = -1;
                bool same = true;
                for (int i = 0; i < h; ++i) {
                    if (!active[i]) continue;
                    if (commonArea == -1) {
                        commonArea = candidateArea2[i];
                    } else if (candidateArea2[i] != commonArea) {
                        same = false;
                        break;
                    }
                }
                if (same && commonArea != -1) {
                    printArea2(hullArea2 - commonArea);
                    return 0;
                }
            }
        }
    }

    printArea2(twiceAreaMod);
    return 0;
}
