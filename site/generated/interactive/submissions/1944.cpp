#include <bits/stdc++.h>
using namespace std;

int n;
int ask(int i) {
    cout << "? " << i + 1 << endl;
    int r;
    if (!(cin >> r) || r < 0) exit(0);
    return r;
}
int distTo(int x, int y) { return (y - x + n) % n; }

struct Bounds {
    array<int, 100> l{}, u{};
    int sl = 0, su = 0;

    void sums() {
        sl = accumulate(l.begin(), l.begin() + n, 0);
        su = accumulate(u.begin(), u.begin() + n, 0);
    }
    bool observe(int x, int bit, int total) {
        if (bit && !l[x]) l[x] = 1, ++sl;
        if (!bit) su -= u[x], u[x] = 0;
        return l[x] <= u[x] && sl <= total && su >= total;
    }
    void tighten(int total) {
        sums();
        int a = sl, b = su;
        for (int x = 0; x < n; ++x) {
            int lo = max(l[x], total - (b - u[x]));
            int hi = min(u[x], total - (a - l[x]));
            l[x] = lo;
            u[x] = hi;
        }
        sums();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;

    // The original coordinate of dial 0 is the coordinate origin.
    vector<int> pos(n, -1), fixed(n);
    Bounds b;
    int h = 0, occupied = 0;
    for (int t = 1; t <= n; ++t) {
        if (occupied == n - 1) break;
        h = t % n;
        if (ask(0)) {
            ++occupied;
            b.l[h] = 1;
            b.u[h] = n - 1;
        }
    }
    pos[0] = h;
    fixed[h] = 1;

    for (int id = 1; id < n; ++id) {
        int m = n - id;
        b.tighten(m);
        vector<int> starts;
        for (int x = 0; x < n; ++x)
            if (b.u[x]) starts.push_back(x);
        if (starts.size() == 1) {
            for (int i = id; i < n; ++i) pos[i] = starts[0];
            break;
        }

        // Estimate a good parking coordinate. Account for the increments
        // needed just to distinguish the cyclic observation strings.
        vector<double> weight(n);
        double low = 0, high = n;
        for (int it = 0; it < 35; ++it) {
            double z = (low + high) / 2, sum = 0;
            for (int x : starts) sum += min<double>(b.u[x], b.l[x] + z);
            if (sum < m) low = z;
            else high = z;
        }
        for (int x : starts) weight[x] = min<double>(b.u[x], b.l[x] + high);

        int helperSteps = 0, park = h;
        double best = 1e100;
        for (int k = 0; k <= (id == 1 ? n - 1 : min(3, n - 1)); ++k) {
            int nh = (h + k) % n;
            vector<int> mask(n), need(n, 1);
            for (int x = 0; x < n; ++x)
                mask[x] = (fixed[x] - (x == h) + (x == nh) > 0 ||
                           weight[x] >= 0.55);
            for (int d = 1; d < n; ++d) {
                int run = 0;
                for (int j = 2*n-1; j >= 0; --j) {
                    int x = j % n, y = (x + d) % n;
                    if (j < n && b.u[x] && b.u[y])
                        need[x] = max(need[x], min(n, run + 1));
                    run = mask[x] == mask[y] ? run + 1 : 0;
                }
            }
            double uncertainty = 0;
            for (int s : starts) uncertainty += weight[s] * need[s] / m;
            for (int t = 0; t < n; ++t) {
                double cost = 1.15 * k + uncertainty * 0.7;
                cost += distTo(nh, t) - distTo(h, t);
                for (int x = 0; x < n; ++x) cost += fixed[x] * distTo(x, t);
                for (int s : starts) {
                    int q = need[s];
                    cost += weight[s] * (q + distTo((s + q) % n, t));
                }
                if (cost < best - 1e-8) {
                    best = cost;
                    helperSteps = k;
                    park = t;
                }
            }
        }

        while (helperSteps--) {
            --fixed[h];
            h = (h + 1) % n;
            int r = ask(0);
            if (!fixed[h]) b.observe(h, r, m);
            ++fixed[h];
        }
        pos[0] = h;
        b.tighten(m);
        starts.clear();

        // For each possible start of the active dial, maintain bounds on
        // the counts of the OTHER still-unlocated dials.
        vector<Bounds> hyp(n);
        for (int s = 0; s < n; ++s) if (b.u[s]) {
            hyp[s] = b;
            hyp[s].l[s] = max(0, b.l[s] - 1);
            --hyp[s].u[s];
            hyp[s].tighten(m - 1);
            starts.push_back(s);
        }

        int shift = 0;
        auto advanceActive = [&]() {
            int r = ask(id);
            shift = (shift + 1) % n;
            erase_if(starts, [&](int s) {
                int x = (s + shift) % n;
                if (fixed[x]) return r == 0;
                return !hyp[s].observe(x, r, m - 1);
            });
        };
        for (int step = 0; starts.size() > 1 && step < n; ++step)
            advanceActive();

        if (starts.size() > 1) {
            // We have a full cyclic scan. Change its bitmap at a known
            // coordinate. A 0->1 change identifies the helper's new site;
            // a 1->0 change identifies its old site.
            for (int k = 1; k < n && starts.size() > 1; ++k) {
                --fixed[h];
                h = (h + 1) % n;
                int r = ask(0);
                erase_if(starts, [&](int s) {
                    if (fixed[h] || s == h) return r == 0;
                    return !hyp[s].observe(h, r, m - 1);
                });
                ++fixed[h];
                if (!r) break;
            }
            // If no empty destination was found, all n-1 other sites
            // held a dial, so leaving the old helper site emptied it.
            pos[0] = h;
            while (starts.size() > 1) advanceActive();
        }

        int s = starts.front();
        int x = (s + shift) % n;
        b = hyp[s];
        b.tighten(m - 1);

        // If the remaining coordinates are already determined, optimize
        // the common destination exactly before making any parking moves.
        int options = 0, only = 0;
        for (int j = 0; j < n; ++j) if (b.u[j]) ++options, only = j;
        if (options <= 1) {
            pos[id] = x;
            for (int i = id + 1; i < n; ++i) pos[i] = only;
            break;
        }

        while (x != park) {
            x = (x + 1) % n;
            int r = ask(id);
            if (!fixed[x]) b.observe(x, r, m - 1);
            if (b.sl == m - 1 || b.su == m - 1) {
                b.tighten(m - 1);
                int cnt = 0;
                for (int j = 0; j < n; ++j) cnt += b.u[j] > 0;
                if (cnt == 1) break;
            }
        }
        pos[id] = x;
        ++fixed[x];
    }

    int target = 0, best = INT_MAX;
    for (int t = 0; t < n; ++t) {
        int cost = 0;
        for (int x : pos) cost += distTo(x, t);
        if (cost < best) best = cost, target = t;
    }
    for (int i = 0; i < n; ++i)
        for (int k = distTo(pos[i], target); k; --k) ask(i);
    cout << "!" << endl;
    return 0;
}
