#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Interval {
    int type;
    int64 l, r;
};

class Solver {
    int64 n, m, s, k;

    int64 normalize(int64 x) const {
        x %= n;
        if (x < 0) x += n;
        return x;
    }

    int64 distance(int64 a, int64 b) const {
        int64 d = llabs(a - b);
        return min(d, n - d);
    }

    int64 ask(int64 x) const {
        x = normalize(x);
        cout << "? " << x + 1 << endl;

        int64 response;
        if (!(cin >> response) || response < 0) exit(0);
        return response;
    }

    // The three possible forms of the transmitter pair.
    pair<int64, int64> getPair(int type, int64 p) const {
        int64 a, b;

        if (type == 0) {
            a = p;
            b = s - p;
        } else if (type == 1) {
            a = -p;
            b = -(s - p);
        } else {
            a = p;
            b = p - s;
        }

        a = normalize(a);
        b = normalize(b);
        if (a > b) swap(a, b);
        return {a, b};
    }

    int64 evaluate(const pair<int64, int64>& transmitters,
                   int64 x) const {
        return distance(x, transmitters.first)
             + distance(x, transmitters.second);
    }

    // Predicted answer at the second queried point k.
    int64 secondResponse(int type, int64 p) const {
        if (type == 0) {
            return s - 2 * p;
        }

        if (type == 1) {
            int64 cap = min(2 * k + s, 2 * n - 2 * k - s);
            return min(cap, n - s + 2 * p);
        }

        if (s <= m) {
            return clamp(s + 2 * k - 2 * p, s, n - s);
        }

        return clamp(n - s - 2 * k + 2 * p, n - s, s);
    }

    // Find all parameters in [l,r] producing the second response.
    optional<Interval> matchingInterval(
        int type, int64 l, int64 r, int64 response
    ) const {
        if (l > r) return nullopt;

        bool increasing = (type == 1 || (type == 2 && s > m));

        auto value = [&](int64 p) {
            int64 result = secondResponse(type, p);
            return increasing ? result : -result;
        };

        int64 target = increasing ? response : -response;

        auto firstPosition = [&](bool strict) {
            int64 low = l, high = r + 1;
            while (low < high) {
                int64 mid = (low + high) / 2;
                bool sufficient = strict
                    ? value(mid) > target
                    : value(mid) >= target;

                if (sufficient) high = mid;
                else low = mid + 1;
            }
            return low;
        };

        int64 left = firstPosition(false);
        int64 right = firstPosition(true) - 1;

        if (left > right) return nullopt;
        return Interval{type, left, right};
    }

    void answer(const pair<int64, int64>& transmitters) const {
        cout << "! " << transmitters.first + 1
             << ' ' << transmitters.second + 1 << endl;
    }

public:
    void run(int64 size) {
        n = size;
        m = n / 2;

        s = ask(0);
        k = s / 2;
        int64 response = ask(k);

        vector<Interval> possibilities;

        auto add = [&](int type, int64 l, int64 r) {
            auto result = matchingInterval(type, l, r, response);
            if (result) possibilities.push_back(*result);
        };

        int64 sameSideLow = max<int64>(0, s - m);
        int64 sameSideHigh = (s - 1) / 2;

        add(0, sameSideLow, sameSideHigh);
        add(1, sameSideLow, sameSideHigh);
        add(2, max<int64>(0, s - m), min(m, s));

        for (auto current : possibilities) {
            if (current.l == current.r) continue;

            // If a non-singleton interval exists, it contains every
            // possible distinct transmitter pair.
            int type = current.type;
            int64 low = current.l, high = current.r;

            while (low < high) {
                if (type == 2 && s > m) {
                    int64 mid = (low + high) / 2;
                    int64 result = ask(mid);

                    if (result == n - s) high = mid;
                    else low = mid + 1;
                } else {
                    int64 mid = (low + high + 1) / 2;

                    if (type == 2) {
                        int64 result = ask(mid);
                        if (result == s) low = mid;
                        else high = mid - 1;
                    } else {
                        int64 x = (type == 0 ? mid : -mid);
                        int64 result = ask(x);

                        if (result == s - 2 * mid) low = mid;
                        else high = mid - 1;
                    }
                }
            }

            answer(getPair(type, low));
            return;
        }

        // Only constantly many individual candidates remain.
        vector<pair<int64, int64>> candidates;
        for (const auto& current : possibilities) {
            candidates.push_back(getPair(current.type, current.l));
        }

        sort(candidates.begin(), candidates.end());
        candidates.erase(unique(candidates.begin(), candidates.end()),
                         candidates.end());

        while (candidates.size() > 1) {
            const auto a = candidates[0];
            const auto b = candidates[1];

            array<int64, 4> points = {
                a.first, a.second, b.first, b.second
            };

            int64 x = -1;
            for (int64 point : points) {
                if (evaluate(a, point) != evaluate(b, point)) {
                    x = point;
                    break;
                }
            }

            // Distinct pairs always differ at one of these endpoints.
            if (x == -1) exit(0);

            int64 result = ask(x);
            vector<pair<int64, int64>> remaining;
            for (const auto& candidate : candidates) {
                if (evaluate(candidate, x) == result) {
                    remaining.push_back(candidate);
                }
            }
            candidates.swap(remaining);
        }

        if (candidates.empty()) exit(0);
        answer(candidates[0]);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;

    while (T--) {
        int64 n;
        if (!(cin >> n)) return 0;

        Solver solver;
        solver.run(n);
    }

    return 0;
}
