#include <bits/stdc++.h>
using namespace std;

struct Candidate {
    int value;
    int single;
};

class Solver {
    int n, k;
    vector<int> base, occupied;
    int anchor = 0;
    int probePos = 0;

    int mod(int x) const {
        x %= n;
        if (x < 0) x += n;
        return x;
    }

    void rotateWheel(int i, int delta) {
        delta = mod(delta);
        if (delta == 0) return;
        cout << i + 1 << ' ' << delta << endl;
        if (!(cin >> k) || k < 1 || k > n) exit(0);
        if (k == 1) exit(0);
    }

    bool aperiodic(const vector<int>& a) const {
        for (int shift = 1; shift < n; ++shift) {
            bool same = true;
            for (int x = 0; x < n; ++x) {
                if (a[x] != a[(x + shift) % n]) {
                    same = false;
                    break;
                }
            }
            if (same) return false;
        }
        return true;
    }

    void rebuild() {
        occupied = base;
        occupied[probePos] = 1;
    }

    void moveProbe(int to) {
        rotateWheel(0, to - probePos);
        probePos = to;
        rebuild();
    }

    void initialize() {
        // 以第 1 个转轮的初始符号为 0，观察位置 0,...,n-2。
        vector<int> seen(n, -1);
        seen[0] = k;
        for (int t = 1; t <= n - 2; ++t) {
            rotateWheel(0, 1);
            probePos = t;
            seen[t] = k;
        }

        int lo = *min_element(seen.begin(), seen.end() - 1);
        int hi = *max_element(seen.begin(), seen.end() - 1);
        int c;
        if (lo != hi) {
            c = lo;
        } else if (n >= 4) {
            // n-1 次反馈全相同，只可能是全部命中或全部未命中。
            c = (lo == n - 1 ? n - 1 : 1);
        } else {
            // n=3 时两种情况重合，需要再观察最后一个位置。
            rotateWheel(0, 1);
            probePos = n - 1;
            seen[n - 1] = k;
            c = *min_element(seen.begin(), seen.end());
        }

        base.assign(n, 0);
        int ones = 0;
        for (int x = 0; x < n; ++x) {
            if (seen[x] != -1) {
                base[x] = (seen[x] == c);
                ones += base[x];
            }
        }
        if (seen[n - 1] == -1) base[n - 1] = c - ones;

        anchor = int(find(base.begin(), base.end(), 1) - base.begin());
        if (aperiodic(base)) {
            if (base[probePos]) anchor = probePos;
            moveProbe(anchor);
        } else {
            int to = probePos;
            if (base[to]) to = int(find(base.begin(), base.end(), 0) - base.begin());
            moveProbe(to);
        }
    }

    int predict(const Candidate& c, int offset, int initialK) const {
        // offset 非零，转动后的符号一定不等于原符号。
        return initialK + 1 - c.single - occupied[(c.value + offset) % n];
    }

    void solve() {
        initialize();
        for (int wheel = 1; wheel < n; ++wheel) {
            int remaining = n - wheel;
            int b = accumulate(base.begin(), base.end(), 0);
            bool first = (wheel == 1);
            vector<Candidate> cand;

            for (int x = 0; x < n; ++x) if (base[x]) {
                if (first) {
                    // 第一轮：base 中每个符号都至少有一个未处理转轮。
                    if (x == probePos) {
                        cand.push_back({x, 0});
                    } else {
                        cand.push_back({x, 1});
                        if (remaining > b) cand.push_back({x, 0});
                    }
                } else if (x == anchor) {
                    // 另外 b-1 个符号各自至少需要一个未处理转轮。
                    if (remaining >= b) cand.push_back({x, 0});
                } else {
                    cand.push_back({x, 1});
                    if (remaining >= b) cand.push_back({x, 0});
                }
            }

            const int initialK = k;
            int offset = 0;
            while (cand.size() > 1) {
                int bestOffset = -1;
                int bestMax = INT_MAX;
                int bestSquares = INT_MAX;
                for (int d = 1; d < n; ++d) {
                    int count[3] = {};
                    for (const auto& c : cand) {
                        ++count[predict(c, d, initialK) - initialK + 1];
                    }
                    int worst = max({count[0], count[1], count[2]});
                    int squares = count[0] * count[0]
                                + count[1] * count[1]
                                + count[2] * count[2];
                    if (worst < bestMax ||
                        (worst == bestMax && squares < bestSquares)) {
                        bestMax = worst;
                        bestSquares = squares;
                        bestOffset = d;
                    }
                }
                rotateWheel(wheel, bestOffset - offset);
                offset = bestOffset;
                vector<Candidate> next;
                for (const auto& c : cand) {
                    if (predict(c, offset, initialK) == k) next.push_back(c);
                }
                cand.swap(next);
            }
            const Candidate answer = cand[0];
            rotateWheel(wheel, anchor - answer.value - offset);
            if (answer.single) base[answer.value] = 0;
            base[anchor] = 1;
            rebuild();

            if (accumulate(base.begin(), base.end(), 0) == 1) {
                moveProbe(anchor);  // 除辅助轮外，其他转轮已经全部对齐。
                return;
            }

            if (!aperiodic(occupied)) {
                if (probePos != anchor) {
                    moveProbe(anchor);
                } else {
                    int to = int(find(base.begin(), base.end(), 0) - base.begin());
                    moveProbe(to);
                }
            }
        }
    }

public:
    void run() {
        if (!(cin >> n >> k)) return;
        if (n < 3 || n > 50 || k <= 1 || k > n) return;
        solve();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver().run();
}
