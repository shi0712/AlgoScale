#include <bits/stdc++.h>
using namespace std;

struct Fenwick {
    int n, top;
    vector<int> f;

    explicit Fenwick(int n) : n(n), top(1), f(n + 1, 0) {
        while (2 * top <= n) top *= 2;
    }

    void add(int i, int d) {
        for (; i <= n; i += i & -i) f[i] += d;
    }

    // Return the endpoint containing rank k, and the rank within its interval.
    pair<int, int> locate(int k) const {
        int p = 0;
        for (int b = top; b; b >>= 1) {
            int q = p + b;
            if (q <= n && f[q] < k) {
                k -= f[q];
                p = q;
            }
        }
        return {p + 1, k};
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, t;
    if (!(cin >> n >> t) || n < 1 || t < 1) return 0;

    // mass[r] counts current zeros since the preceding endpoint.
    // Only the rightmost interval may store a lower bound instead.
    set<int> ends{0, n};
    vector<int> mass(n + 1);
    Fenwick tree(n);
    int total = 0;

    auto change = [&](int r, int d) {
        mass[r] += d;
        total += d;
        tree.add(r, d);
    };

    auto split = [&](int r, int p, int zeros) {
        // p is strictly inside the interval ending at r.
        int take = min(mass[r], zeros);
        change(r, -take);
        change(p, zeros);
        ends.insert(p);
    };

    for (int round = 0; round < t; ++round) {
        int k;
        if (!(cin >> k) || k <= 0) return 0;

        // Future rounds also require at least one distinct zero each.
        int need = max(k, t - round);
        if (total < need) change(n, need - total);

        auto [r, rank] = tree.locate(k);
        int l = *prev(ends.find(r));
        int dir = 0, stride = 0;

        while (true) {
            int len = r - l;
            int z = mass[r];
            int lo = l + rank;
            int hi = r - (z - rank);

            if (lo == hi) {
                int x = lo;
                // Cache both prefix counts implied by knowing this answer.
                if (x - 1 > l) split(r, x - 1, rank - 1);
                if (x < r) split(r, x, 1);
                change(x, -1);
                cout << "! " << x << endl;
                break;
            }

            double estimatedZeros = z;
            if (r == n) {
                // Estimate the unseen suffix density from the observed prefix.
                // All previously guessed positions in that prefix were
                // originally zeros. A guess at n itself leaves a singleton
                // rightmost interval, which never needs interpolation.
                int known = total - mass[n];
                double density = (known + round + 1.0) / (l + 2.0);
                estimatedZeros = clamp(density * len, double(z), double(len));
            }

            // Interpolate the rank among the interval's zeros. Split only
            // between feasible answers, so a query always makes progress.
            double frac = (rank - 1.0 / 3) / (estimatedZeros + 1.0 / 3);
            int p = clamp(lo + int(frac * (len - estimatedZeros)), lo, hi - 1);

            // Gallop when consecutive estimates undershoot on the same side.
            int step = min(stride, (hi - lo - 1) / 2);
            if (dir > 0) p = max(p, lo + step);
            if (dir < 0) p = min(p, hi - 1 - step);

            cout << "? " << l + 1 << ' ' << p << endl;
            int ones;
            if (!(cin >> ones) || ones < 0 || ones > p - l) return 0;
            int leftZeros = p - l - ones;
            split(r, p, leftZeros);

            if (rank <= leftZeros) {
                stride = min(n, max(dir == -1 ? 2 * stride : 1, hi - p));
                dir = -1;
                r = p;
            } else {
                stride = min(n, max(dir == 1 ? 2 * stride : 1, p - lo + 1));
                dir = 1;
                l = p;
                rank -= leftZeros;
            }
        }
    }
    return 0;
}
