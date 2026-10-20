#include <bits/stdc++.h>
using namespace std;

struct Node {
    long long t;
    int l, r, side;
};

struct Cmp {
    bool operator()(Node const& a, Node const& b) const {
        return a.t > b.t;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, s, m;
    cin >> n >> s >> m;

    vector<long long> rel(n, -1);

    for (int i = 0; i < m; ++i) {
        long long a;
        int f;
        cin >> a >> f;
        rel[f] = max(rel[f], a);
    }

    vector<int> floors;
    vector<long long> need;

    for (int i = 0; i < n; ++i) {
        if (rel[i] >= 0) {
            floors.push_back(i);
            need.push_back(rel[i]);
        }
    }

    int k = (int)floors.size();

    if (k == 0) {
        cout << 0 << '\n';
        return 0;
    }

    sort(floors.begin(), floors.end());

    vector<long long> r(k);
    for (int i = 0; i < k; ++i) {
        r[i] = rel[floors[i]];
    }

    long long maxR = *max_element(r.begin(), r.end());
    long long L = floors[0];
    long long R = floors[k - 1];

    long long cover_time;
    if (s < L) {
        cover_time = R - s;
    } else if (s > R) {
        cover_time = s - L;
    } else {
        cover_time = (R - L) + min<long long>(s - L, R - s);
    }

    auto feasible = [&](long long T) -> bool {
        if (T < maxR) return false;

        // Necessary interval for the starting position in reversed time.
        long long lowStart = LLONG_MIN / 4;
        long long highStart = LLONG_MAX / 4;

        for (int i = 0; i < k; ++i) {
            long long d = T - r[i];
            lowStart = max(lowStart, (long long)floors[i] - d);
            highStart = min(highStart, (long long)floors[i] + d);
        }

        if (lowStart > highStart) return false;

        // Minimal time to cover the whole requested interval and end at s,
        // starting from an arbitrary floor.
        long long width = R - L;
        long long H0 = width + min<long long>(llabs((long long)s - L),
                                             llabs((long long)s - R));

        // If even the tightest deadline is at least H0, any optimal
        // unconstrained covering route works.
        if (T - maxR >= H0) return true;

        priority_queue<Node, vector<Node>, Cmp> pq;
        unordered_map<unsigned long long, long long> best;
        best.reserve(8192);
        best.max_load_factor(0.7);

        auto get_key = [](int l, int r, int side) -> unsigned long long {
            return ((unsigned long long)l << 33) |
                   ((unsigned long long)r << 1) |
                   (unsigned long long)side;
        };

        auto add_state = [&](int l, int r, int side, long long tm) {
            if (tm > T) return;

            long long p = (side == 0 ? floors[l] : floors[r]);

            bool needL = (l > 0);
            bool needR = (r + 1 < k);

            long long h = 0;

            if (needL && needR) {
                long long toLthenR = llabs(p - L) + (R - L) + llabs(R - s);
                long long toRthenL = llabs(p - R) + (R - L) + llabs(L - s);
                h = min(toLthenR, toRthenL);
            } else if (needL) {
                h = llabs(p - L) + llabs(L - s);
            } else if (needR) {
                h = llabs(p - R) + llabs(R - s);
            } else {
                h = llabs(p - s);
            }

            if (tm + h > T) return;

            unsigned long long key = get_key(l, r, side);
            auto it = best.find(key);

            if (it != best.end() && it->second <= tm) return;

            best[key] = tm;
            pq.push({tm, l, r, side});
        };

        // The reversed path may start at any requested floor.
        for (int i = 0; i < k; ++i) {
            if (lowStart <= floors[i] && floors[i] <= highStart) {
                add_state(i, i, 0, 0);
            }
        }

        while (!pq.empty()) {
            Node cur = pq.top();
            pq.pop();

            unsigned long long key = get_key(cur.l, cur.r, cur.side);
            auto it = best.find(key);

            if (it == best.end() || it->second != cur.t) continue;

            if (cur.l == 0 && cur.r == k - 1) {
                return true;
            }

            long long p = (cur.side == 0 ? floors[cur.l] : floors[cur.r]);

            // Expand to the left.
            if (cur.l > 0) {
                int nl = cur.l - 1;
                long long d = T - r[nl];
                long long arr = cur.t + llabs(p - floors[nl]);

                if (arr <= d) {
                    add_state(nl, cur.r, 0, arr);

                    long long backToRight =
                        arr + ((long long)floors[cur.r] - floors[nl]);
                    add_state(nl, cur.r, 1, backToRight);
                }
            }

            // Expand to the right.
            if (cur.r + 1 < k) {
                int nr = cur.r + 1;
                long long d = T - r[nr];
                long long arr = cur.t + llabs(p - floors[nr]);

                if (arr <= d) {
                    add_state(cur.l, nr, 1, arr);

                    long long backToLeft =
                        arr + ((long long)floors[nr] - floors[cur.l]);
                    add_state(cur.l, nr, 0, backToLeft);
                }
            }
        }

        return false;
    };

    long long lo = max(maxR, cover_time);
    long long hi = maxR + cover_time;

    while (lo < hi) {
        long long mid = (lo + hi) / 2;
        if (feasible(mid)) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    cout << lo << '\n';
    return 0;
}
