#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using ld = long double;

static constexpr int S = 100000;

struct Candidate {
    int64 cy, r;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    auto probe = [&](int x1, int y1, int x2, int y2, ld &length) -> bool {
        cout << "query " << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << '\n';
        cout.flush();
        if (!(cin >> length)) return false;
        if (length < 0) return false;
        return true;
    };

    vector<int> ys;
    for (int y = 0; y <= S; y += 199) ys.push_back(y);
    ys.push_back(S);
    ys.push_back(S / 2);
    sort(ys.begin(), ys.end());
    ys.erase(unique(ys.begin(), ys.end()), ys.end());
    sort(ys.begin(), ys.end(), [](int a, int b) {
        int da = abs(a - S / 2), db = abs(b - S / 2);
        if (da != db) return da < db;
        return a < b;
    });

    ld len = 0;
    int y0 = -1;
    int64 q0 = -1;

    for (int y : ys) {
        if (!probe(0, y, S, y, len)) return 0;
        if (len > 0.5L) {
            y0 = y;
            q0 = llround(len * len / 4.0L);
            break;
        }
    }
    if (y0 < 0) return 0;

    vector<Candidate> candidates;
    for (int64 a = 1; a * a <= q0; ++a) {
        if (q0 % a != 0) continue;
        int64 b = q0 / a;
        if (((a + b) & 1) != 0) continue;
        int64 r = (a + b) / 2;
        int64 u = (b - a) / 2;
        if (r < 100 || r > S / 2 || u >= r) continue;

        if (u == 0) {
            int64 cy = y0;
            if (cy >= r && cy <= S - r) candidates.push_back({cy, r});
        } else {
            for (int sg : {-1, 1}) {
                int64 cy = (int64)y0 + sg * u;
                if (cy >= r && cy <= S - r) candidates.push_back({cy, r});
            }
        }
    }

    auto expected_q = [](const Candidate &c, int y) -> int64 {
        int64 d = c.cy - y;
        return c.r * c.r - d * d;
    };

    if (candidates.size() > 1) {
        if (!probe(0, y0 + 1, S, y0 + 1, len)) return 0;
        bool positive = len > 0.5L;
        int64 q1 = positive ? llround(len * len / 4.0L) : 0;

        vector<Candidate> filtered;
        for (const auto &c : candidates) {
            int64 q = expected_q(c, y0 + 1);
            if (positive) {
                if (q > 0 && q == q1) filtered.push_back(c);
            } else {
                if (q <= 0) filtered.push_back(c);
            }
        }
        candidates.swap(filtered);

        if (candidates.size() > 1) {
            if (!probe(0, y0 - 1, S, y0 - 1, len)) return 0;
            positive = len > 0.5L;
            int64 q2 = positive ? llround(len * len / 4.0L) : 0;

            filtered.clear();
            for (const auto &c : candidates) {
                int64 q = expected_q(c, y0 - 1);
                if (positive) {
                    if (q > 0 && q == q2) filtered.push_back(c);
                } else {
                    if (q <= 0) filtered.push_back(c);
                }
            }
            candidates.swap(filtered);
        }
    }

    if (candidates.empty()) return 0;
    int64 cy = candidates[0].cy;
    int64 r = candidates[0].r;

    int64 cx;
    if (r == S / 2) {
        cx = S / 2;
    } else {
        int64 k = max<int64>(1, r / 4);
        int64 c = r / 2;

        int yStart = (int)(cy - k - c);
        int yEnd = (int)(cy + k - c);
        if (!probe(0, yStart, S, yEnd, len)) return 0;

        ld q = len * len / 4.0L;
        ld m = 2.0L * (ld)k / (ld)S;
        ld z2 = ((ld)r * (ld)r - q) * (1.0L + m * m);
        if (z2 < 0) z2 = 0;
        ld z = sqrt(z2);
        ld realX = ((ld)k + (ld)c - z) / m;
        cx = llround(realX);
    }

    cout << "answer " << cx << ' ' << cy << ' ' << r << '\n';
    cout.flush();
    return 0;
}
