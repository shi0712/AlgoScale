#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

struct BigInt {
    static constexpr uint32_t BASE = 1000000000;
    vector<uint32_t> d;

    BigInt(uint64_t value = 0) {
        while (value) {
            d.push_back(value % BASE);
            value /= BASE;
        }
    }
    BigInt(const string& s) {
        for (int i = (int)s.size(); i > 0; i -= 9) {
            int begin = max(0, i - 9);
            d.push_back(stoul(s.substr(begin, i - begin)));
        }
        normalize();
    }

    void normalize() {
        while (!d.empty() && d.back() == 0) d.pop_back();
    }
    bool isZero() const { return d.empty(); }

    friend bool operator==(const BigInt& a, const BigInt& b) { return a.d == b.d; }
    friend bool operator<(const BigInt& a, const BigInt& b) {
        if (a.d.size() != b.d.size()) return a.d.size() < b.d.size();
        for (int i = (int)a.d.size() - 1; i >= 0; --i) {
            if (a.d[i] != b.d[i]) return a.d[i] < b.d[i];
        }
        return false;
    }
    friend bool operator!=(const BigInt& a, const BigInt& b) { return !(a == b); }

    friend BigInt operator+(const BigInt& a, const BigInt& b) {
        BigInt result;
        uint64_t carry = 0;
        size_t size = max(a.d.size(), b.d.size());
        result.d.resize(size);
        for (size_t i = 0; i < size; ++i) {
            uint64_t sum = carry;
            if (i < a.d.size()) sum += a.d[i];
            if (i < b.d.size()) sum += b.d[i];
            result.d[i] = sum % BASE;
            carry = sum / BASE;
        }
        if (carry) result.d.push_back(carry);
        return result;
    }
    friend BigInt operator-(const BigInt& a, const BigInt& b) {
        BigInt result = a;
        int64_t borrow = 0;
        for (size_t i = 0; i < result.d.size(); ++i) {
            int64_t value = (int64_t)result.d[i] - borrow -
                            (i < b.d.size() ? b.d[i] : 0);
            if (value < 0) {
                value += BASE;
                borrow = 1;
            } else {
                borrow = 0;
            }
            result.d[i] = value;
        }
        result.normalize();
        return result;
    }
    friend BigInt operator*(const BigInt& a, const BigInt& b) {
        if (a.isZero() || b.isZero()) return BigInt();
        BigInt result;
        result.d.assign(a.d.size() + b.d.size(), 0);
        for (size_t i = 0; i < a.d.size(); ++i) {
            uint64_t carry = 0;
            for (size_t j = 0; j < b.d.size(); ++j) {
                uint64_t value = result.d[i + j] + carry +
                                 (uint64_t)a.d[i] * b.d[j];
                result.d[i + j] = value % BASE;
                carry = value / BASE;
            }
            size_t pos = i + b.d.size();
            while (carry) {
                uint64_t value = result.d[pos] + carry;
                result.d[pos] = value % BASE;
                carry = value / BASE;
                ++pos;
            }
        }
        result.normalize();
        return result;
    }
    BigInt multiplied(uint32_t value) const {
        if (value == 0 || isZero()) return BigInt();
        BigInt result;
        result.d.resize(d.size());
        uint64_t carry = 0;
        for (size_t i = 0; i < d.size(); ++i) {
            uint64_t cur = (uint64_t)d[i] * value + carry;
            result.d[i] = cur % BASE;
            carry = cur / BASE;
        }
        if (carry) result.d.push_back(carry);
        return result;
    }
    static pair<BigInt, BigInt> divmod(const BigInt& a, const BigInt& b) {
        if (a < b) return {BigInt(), a};
        BigInt quotient, remainder;
        quotient.d.assign(a.d.size(), 0);
        for (int i = (int)a.d.size() - 1; i >= 0; --i) {
            remainder.d.insert(remainder.d.begin(), a.d[i]);
            remainder.normalize();
            uint32_t low = 0, high = BASE - 1;
            while (low < high) {
                uint32_t mid = low + (uint64_t(high) - low + 1) / 2;
                if (b.multiplied(mid) < remainder || b.multiplied(mid) == remainder) {
                    low = mid;
                } else {
                    high = mid - 1;
                }
            }
            quotient.d[i] = low;
            if (low) remainder = remainder - b.multiplied(low);
        }
        quotient.normalize();
        return {quotient, remainder};
    }
    friend BigInt operator/(const BigInt& a, const BigInt& b) {
        return divmod(a, b).first;
    }
    friend BigInt operator%(const BigInt& a, const BigInt& b) {
        return divmod(a, b).second;
    }
    BigInt& operator+=(const BigInt& other) {
        *this = *this + other;
        return *this;
    }
    BigInt& operator/=(const BigInt& other) {
        *this = *this / other;
        return *this;
    }

    string str() const {
        if (isZero()) return "0";
        string result = to_string(d.back());
        for (int i = (int)d.size() - 2; i >= 0; --i) {
            string part = to_string(d[i]);
            result.append(9 - part.size(), '0');
            result += part;
        }
        return result;
    }
};

static ostream& operator<<(ostream& out, const BigInt& value) {
    return out << value.str();
}

using cpp_int = BigInt;

struct Point {
    long long x, y;
};

static long long cross(const Point& a, const Point& b, const Point& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static bool onSegment(const Point& a, const Point& b, const Point& p) {
    return cross(a, b, p) == 0 &&
           min(a.x, b.x) <= p.x && p.x <= max(a.x, b.x) &&
           min(a.y, b.y) <= p.y && p.y <= max(a.y, b.y);
}

static bool segmentsIntersect(const Point& a, const Point& b,
                              const Point& c, const Point& d) {
    long long c1 = cross(a, b, c);
    long long c2 = cross(a, b, d);
    long long c3 = cross(c, d, a);
    long long c4 = cross(c, d, b);
    if (((c1 > 0 && c2 < 0) || (c1 < 0 && c2 > 0)) &&
        ((c3 > 0 && c4 < 0) || (c3 < 0 && c4 > 0))) {
        return true;
    }
    return (c1 == 0 && onSegment(a, b, c)) ||
           (c2 == 0 && onSegment(a, b, d)) ||
           (c3 == 0 && onSegment(c, d, a)) ||
           (c4 == 0 && onSegment(c, d, b));
}

static bool simpleOrder(const vector<int>& order, const vector<Point>& pts) {
    int n = (int)order.size();
    for (int i = 0; i < n; ++i) {
        const Point& prev = pts[order[(i + n - 1) % n]];
        const Point& cur = pts[order[i]];
        const Point& next = pts[order[(i + 1) % n]];
        if (cross(prev, cur, next) == 0 &&
            (prev.x - cur.x) * (next.x - cur.x) +
            (prev.y - cur.y) * (next.y - cur.y) > 0) {
            return false;
        }
    }
    for (int i = 0; i < n; ++i) {
        int ni = (i + 1) % n;
        for (int j = i + 1; j < n; ++j) {
            int nj = (j + 1) % n;
            if (i == j || ni == j || nj == i) continue;
            if (segmentsIntersect(pts[order[i]], pts[order[ni]],
                                  pts[order[j]], pts[order[nj]])) {
                return false;
            }
        }
    }
    return true;
}

static long long doubledArea(const vector<int>& order, const vector<Point>& pts) {
    long long area = 0;
    int n = (int)order.size();
    for (int i = 0; i < n; ++i) {
        const Point& a = pts[order[i]];
        const Point& b = pts[order[(i + 1) % n]];
        area += a.x * b.y - a.y * b.x;
    }
    return llabs(area);
}

static cpp_int gcdBig(cpp_int a, cpp_int b) {
    while (!b.isZero()) {
        cpp_int r = a % b;
        a = b;
        b = r;
    }
    return a;
}

static void answer(cpp_int num, cpp_int den) {
    if (num == 0) {
        cout << "! 0 1" << endl;
        return;
    }
    cpp_int g = gcdBig(num, den);
    num /= g;
    den /= g;
    cout << "! " << num << " " << den << endl;
}

static bool readLength(cpp_int& r, cpp_int& s) {
    string rs, ss;
    if (!(cin >> rs >> ss)) return false;
    r = cpp_int(rs);
    s = cpp_int(ss);
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    vector<Point> pts(n);
    for (Point& p : pts) {
        if (!(cin >> p.x >> p.y)) return 0;
    }

    vector<Point> sorted = pts;
    sort(sorted.begin(), sorted.end(), [](const Point& a, const Point& b) {
        return tie(a.x, a.y) < tie(b.x, b.y);
    });
    sorted.erase(unique(sorted.begin(), sorted.end(), [](const Point& a, const Point& b) {
        return a.x == b.x && a.y == b.y;
    }), sorted.end());

    vector<Point> hull;
    if (sorted.size() <= 2) {
        hull = sorted;
    } else {
        vector<Point> h(2 * sorted.size());
        int k = 0;
        for (const Point& p : sorted) {
            while (k >= 2 && cross(h[k - 2], h[k - 1], p) <= 0) --k;
            h[k++] = p;
        }
        int lower = k;
        for (int i = (int)sorted.size() - 2; i >= 0; --i) {
            const Point& p = sorted[i];
            while (k > lower && cross(h[k - 2], h[k - 1], p) <= 0) --k;
            h[k++] = p;
        }
        if (k > 1) --k;
        h.resize(k);
        hull = h;
    }

    long long hullArea2 = 0;
    for (int i = 0; i < (int)hull.size(); ++i) {
        const Point& a = hull[i];
        const Point& b = hull[(i + 1) % hull.size()];
        hullArea2 += a.x * b.y - a.y * b.x;
    }
    hullArea2 = llabs(hullArea2);
    if (hullArea2 == 0) {
        answer(0, 1);
        return 0;
    }

    bool allOnHull = true;
    for (const Point& p : pts) {
        bool found = false;
        for (int i = 0; i < (int)hull.size(); ++i) {
            if (onSegment(hull[i], hull[(i + 1) % hull.size()], p)) {
                found = true;
                break;
            }
        }
        if (!found) {
            allOnHull = false;
            break;
        }
    }
    if (allOnHull) {
        answer(hullArea2, 2);
        return 0;
    }

    vector<long long> xs;
    for (const Point& p : pts) xs.push_back(p.x);
    sort(xs.begin(), xs.end());
    xs.erase(unique(xs.begin(), xs.end()), xs.end());
    int slabs = (int)xs.size() - 1;

    if (n <= 12 && slabs > 0) {
        vector<vector<int>> polygons;
        vector<long long> areas;
        vector<int> order(n), rest;
        order[0] = 0;
        for (int i = 1; i < n; ++i) rest.push_back(i);
        do {
            for (int i = 1; i < n; ++i) order[i] = rest[i - 1];
            if (order[1] > order[n - 1]) continue;
            if (!simpleOrder(order, pts)) continue;
            long long area = doubledArea(order, pts);
            if (area == 0) continue;
            polygons.push_back(order);
            areas.push_back(area);
        } while (next_permutation(rest.begin(), rest.end()));

        if (!polygons.empty()) {
            bool sameArea = true;
            for (long long area : areas) sameArea &= (area == areas[0]);
            if (sameArea) {
                answer(areas[0], 2);
                return 0;
            }

            cpp_int common = 2;
            for (int d = 2; d <= 1000; ++d) {
                common = common / gcdBig(common, cpp_int(d)) * d;
            }

            int m = (int)polygons.size();
            vector<vector<cpp_int>> lengths(m, vector<cpp_int>(slabs));
            for (int s = 0; s < slabs; ++s) {
                long long twiceX = xs[s] + xs[s + 1];
                map<pair<int, int>, cpp_int> edgeHeight;
                for (int i = 0; i < n; ++i) {
                    for (int j = i + 1; j < n; ++j) {
                        Point a = pts[i], b = pts[j];
                        if (a.x > b.x) swap(a, b);
                        if (2 * a.x < twiceX && twiceX < 2 * b.x) {
                            long long dx = b.x - a.x;
                            cpp_int den = 2 * dx;
                            cpp_int num = 2 * a.y * dx +
                                          (b.y - a.y) * (twiceX - 2 * a.x);
                            edgeHeight[{i, j}] = num * (common / den);
                        }
                    }
                }
                for (int id = 0; id < m; ++id) {
                    vector<cpp_int> hits;
                    for (int i = 0; i < n; ++i) {
                        int a = polygons[id][i];
                        int b = polygons[id][(i + 1) % n];
                        if (a > b) swap(a, b);
                        auto it = edgeHeight.find({a, b});
                        if (it != edgeHeight.end()) hits.push_back(it->second);
                    }
                    sort(hits.begin(), hits.end());
                    cpp_int len = 0;
                    for (int i = 0; i + 1 < (int)hits.size(); i += 2) {
                        len += hits[i + 1] - hits[i];
                    }
                    lengths[id][s] = len;
                }
            }

            vector<int> current(m);
            iota(current.begin(), current.end(), 0);
            while (true) {
                set<long long> remainingAreas;
                for (int id : current) remainingAreas.insert(areas[id]);
                if (remainingAreas.size() == 1) {
                    answer(areas[current[0]], 2);
                    return 0;
                }

                int bestSlab = -1;
                tuple<int, int, long long> bestMetric{INT_MAX, INT_MAX, LLONG_MAX};
                for (int s = 0; s < slabs; ++s) {
                    map<cpp_int, vector<int>> groups;
                    for (int id : current) groups[lengths[id][s]].push_back(id);
                    int worstAreas = 0, worstCandidates = 0;
                    long long squaredSizes = 0;
                    for (const auto& entry : groups) {
                        set<long long> groupAreas;
                        for (int id : entry.second) groupAreas.insert(areas[id]);
                        worstAreas = max(worstAreas, (int)groupAreas.size());
                        worstCandidates = max(worstCandidates, (int)entry.second.size());
                        squaredSizes += (long long)entry.second.size() * entry.second.size();
                    }
                    auto metric = make_tuple(worstAreas, worstCandidates, squaredSizes);
                    if (metric < bestMetric) {
                        bestMetric = metric;
                        bestSlab = s;
                    }
                }

                cout << "? " << xs[bestSlab] + xs[bestSlab + 1] << " 2" << endl;
                cpp_int r, q;
                if (!readLength(r, q)) return 0;
                vector<int> next;
                for (int id : current) {
                    if (r * common == q * lengths[id][bestSlab]) next.push_back(id);
                }
                if (next.empty()) return 0;
                current.swap(next);
            }
        }
    }

    cpp_int areaNum = 0, areaDen = 1;
    for (int i = 0; i < slabs; ++i) {
        cout << "? " << xs[i] + xs[i + 1] << " 2" << endl;
        cpp_int r, s;
        if (!readLength(r, s)) return 0;
        cpp_int addNum = r * (xs[i + 1] - xs[i]);
        cpp_int addDen = s;
        cpp_int newDen = areaDen / gcdBig(areaDen, addDen) * addDen;
        areaNum = areaNum * (newDen / areaDen) + addNum * (newDen / addDen);
        areaDen = newDen;
        cpp_int g = gcdBig(areaNum, areaDen);
        if (g != 0) {
            areaNum /= g;
            areaDen /= g;
        }
    }
    answer(areaNum, areaDen);
    return 0;
}
