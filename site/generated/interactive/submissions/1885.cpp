#include <bits/stdc++.h>
using namespace std;

class Solver {
    static constexpr int MAX_DIST = 1'000'000;
    static constexpr int INF = 1'000'000'000;

    struct DiameterBounds {
        int low, high, probe;
    };

    struct Phase {
        int pivot = -1;
        int positives = 0;
        vector<int> negatives;
    };

    int n, half, a, length;
    vector<int> depth, lo, hi;
    vector<vector<int>> cache;

    int ask(int i, int j) {
        if (i == j) return 0;
        if (cache[i][j] != -1) return cache[i][j];
        cout << "? " << i << ' ' << j << endl;
        int d;
        if (!(cin >> d) || d <= 0) exit(0);
        return cache[i][j] = cache[j][i] = d;
    }

    // Positions are measured from a along the path from a to town 0.
    int endpointDistance(int i, int x) const {
        return 2 * x + depth[i] - length;
    }

    void resolve(int i) {
        int x = (length + ask(a, i) - depth[i]) / 2;
        lo[i] = hi[i] = x;
    }

    DiameterBounds diameterBounds() const {
        DiameterBounds b{length, length, -1};
        for (int i = 1; i < n; ++i) {
            if (i == a) continue;
            b.low = max(b.low, endpointDistance(i, lo[i]));
            int u = endpointDistance(i, hi[i]);
            if (u > b.high) {
                b.high = u;
                b.probe = i;
            }
        }
        return b;
    }

    static int eccentricity(int x, int diameter) {
        return max(x, diameter - x);
    }

    int findRadius() {
        // A farthest town from 0 is a diameter endpoint. For a path city
        // at x, its eccentricity is max(x, D-x): a town projecting to the
        // left is at distance at most x, and one to the right at most D-x.
        // Every city on this path is the projection of some other town.
        while (true) {
            DiameterBounds d = diameterBounds();
            int lower = INF, upper = INF, probe = -1;
            for (int i = 1; i < n; ++i) {
                if (i == a) continue;
                int x = clamp(d.low / 2, lo[i], hi[i]);
                int l = eccentricity(x, d.low);
                if (l < lower || (l == lower && lo[i] != hi[i] &&
                                   (probe == -1 || lo[probe] == hi[probe]))) {
                    lower = l;
                    probe = i;
                }
                upper = min(upper, max(hi[i], d.high - lo[i]));
            }
            if (lower == upper) return lower;
            resolve(d.low < d.high ? d.probe : probe);
        }
    }

    DiameterBounds tighten(int radius) {
        // If even the upper bound on the diameter makes a position have
        // eccentricity less than R, no city can occupy that position.
        while (true) {
            DiameterBounds d = diameterBounds();
            int left = d.high - radius;
            bool changed = false;
            for (int i = 1; i < n; ++i) {
                if (i == a) continue;
                int l = lo[i], h = hi[i];
                if (l > left && l < radius) lo[i] = radius;
                if (h > left && h < radius) hi[i] = left;
                changed |= (l != lo[i] || h != hi[i]);
            }
            if (!changed) return d;
        }
    }

    bool hasHeavyBranch(const vector<int>& towns, int x) {
        const int s = (int)towns.size();
        auto same = [&](int i, int j) {
            int hi = depth[i] + x - length;
            int hj = depth[j] + x - length;
            return ask(i, j) < hi + hj;
        };

        // Cancel pairs from different branches. In each completed phase
        // all positive towns share one branch, and all negatives differ
        // from it. This also makes verification cheaper than a rescan.
        vector<Phase> done;
        Phase cur;
        int pairs = 0;
        for (int i : towns) {
            if (cur.positives == (int)cur.negatives.size()) {
                if (cur.positives) done.push_back(move(cur));
                cur = Phase{i, 1, {}};
            } else if (same(cur.pivot, i)) {
                if (++cur.positives > half) return true;
            } else {
                cur.negatives.push_back(i);
                if (++pairs >= s - half) return false;
            }
        }
        if (cur.positives == (int)cur.negatives.size()) return false;

        int count = cur.positives;
        int possible = s - pairs;
        sort(done.begin(), done.end(), [](const Phase& p, const Phase& q) {
            return p.positives > q.positives;
        });
        for (const Phase& p : done) {
            if (same(cur.pivot, p.pivot)) {
                count += p.positives;
                if (count > half) return true;
            } else {
                for (int i : p.negatives) {
                    if (same(cur.pivot, i)) {
                        if (++count > half) return true;
                    } else if (--possible <= half) {
                        return false;
                    }
                }
            }
        }
        return count > half;
    }

    // 0: not a balanced city, 1: balanced city,
    // 2: an off-path component is heavy (so no path city is balanced).
    int testPosition(int x, int radius) {
        if (x <= 0 || x >= length) return 0;
        while (true) {
            tighten(radius);
            int left = 0, right = 0, maxLeft = 0, maxRight = 0;
            int canBeHere = 0, canBeInHeavyBranch = 0;
            bool isCity = false;
            vector<int> eligible;
            for (int i = 0; i < n; ++i) {
                left += hi[i] < x;
                right += lo[i] > x;
                maxLeft += lo[i] < x;
                maxRight += hi[i] > x;
                if (lo[i] <= x && x <= hi[i]) {
                    ++canBeHere;
                    bool fixed = lo[i] == hi[i];
                    isCity |= fixed;
                    // A town at height 1 forms a singleton component.
                    if (depth[i] + x - length > 1) {
                        ++canBeInHeavyBranch;
                        if (fixed) eligible.push_back(i);
                    }
                }
            }

            if (!canBeHere || left > half || right > half) return 0;
            bool needLeft = maxLeft > half, needRight = maxRight > half;
            bool needGroup = canBeInHeavyBranch > half;
            if (isCity && !needLeft && !needRight) {
                if (!needGroup) return 1;
                if ((int)eligible.size() == canBeInHeavyBranch)
                    return hasHeavyBranch(eligible, x) ? 2 : 1;
            }

            int best = -1, bestScore = 0;
            for (int i = 1; i < n; ++i) {
                if (i == a || lo[i] == hi[i]) continue;
                bool here = lo[i] <= x && x <= hi[i];
                int score = (needLeft && lo[i] < x && hi[i] >= x)
                          + (needRight && lo[i] <= x && hi[i] > x)
                          + (!isCity && here)
                          + (needGroup && here && depth[i] + x - length > 1);
                if (score > bestScore ||
                    (score == bestScore && score > 0 &&
                     hi[i] - lo[i] < hi[best] - lo[best])) {
                    best = i;
                    bestScore = score;
                }
            }
            resolve(best);
        }
    }

    bool balancedHub(int radius) {
        // A hub can only be at x = R or x = D-R. At x = R, any city
        // is automatically a hub since D <= 2R.
        int rightResult = testPosition(radius, radius);
        if (rightResult != 0) return rightResult == 1;

        while (true) {
            DiameterBounds d = tighten(radius);

            // A balanced position also has to lie in the median interval
            // of the projected towns.
            vector<int> lower = lo, upper = hi;
            nth_element(lower.begin(), lower.begin() + (n - 1) / 2, lower.end());
            nth_element(upper.begin(), upper.begin() + n / 2, upper.end());
            int l = max({1, d.low - radius, lower[(n - 1) / 2]});
            int h = min({length - 1, radius - 1, d.high - radius, upper[n / 2]});

            int first = INF, last = -INF;
            if (l <= h) {
                for (int i = 1; i < n; ++i) {
                    if (i == a) continue;
                    int x = max(l, lo[i]), y = min(h, hi[i]);
                    if (x <= y) {
                        first = min(first, x);
                        last = max(last, y);
                    }
                }
            }

            if (first > last) return false;
            if (first == last) {
                if (testPosition(first, radius) != 1) return false;
                // Only this position can still be a balanced hub.
                while (true) {
                    d = tighten(radius);
                    if (d.high <= first + radius) return true;
                    if (d.low > first + radius) return false;
                    resolve(d.probe);
                }
            }
            resolve(d.probe);
        }
    }

public:
    explicit Solver(int n)
        : n(n), half(n / 2), a(0), length(0), depth(n), lo(n), hi(n),
          cache(n, vector<int>(n, -1)) {}

    int run() {
        for (int i = 1; i < n; ++i) {
            depth[i] = ask(0, i);
            if (depth[i] > length) {
                length = depth[i];
                a = i;
            }
        }
        lo[0] = hi[0] = length;
        lo[a] = hi[a] = 0;
        for (int i = 1; i < n; ++i) {
            if (i == a) continue;
            lo[i] = max(1, length - depth[i] + 1);
            hi[i] = min(length - 1, (MAX_DIST + length - depth[i]) / 2);
        }

        // No city off the a--0 path can be a hub. If its projection
        // is at x >= D/2, the distance to a alone is too large relative
        // to the projection. If x < D/2, a farthest town from a projects
        // at or beyond D/2, and its distance gives the same conclusion.
        int radius = findRadius();
        return balancedHub(radius) ? radius : -radius;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 0) return 0;
    Solver solver(n);
    int answer = solver.run();
    cout << "! " << answer << endl;
    return 0;
}
