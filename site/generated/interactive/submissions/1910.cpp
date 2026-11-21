#include <bits/stdc++.h>
using namespace std;

struct Component {
    // side[0] is the larger class (either class in a tie).
    vector<int> side[2];
    int bin = -1;
    int weight() const {
        return int(side[0].size()) - int(side[1].size());
    }
};

int ask(int a, int b) {
    cout << "? " << a << ' ' << b << endl;
    int r;
    if (!(cin >> r) || (r != 0 && r != 1)) exit(0);
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<Component> c(n);
    vector<int> labels(n);
    iota(labels.begin(), labels.end(), 1);
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    shuffle(labels.begin(), labels.end(), rng);
    for (int i = 0; i < n; ++i) c[i].side[0].push_back(labels[i]);

    bool packed = false, triedPacking = false;
    while (true) {
        int k = c.size(), rem = m;
        vector<int> w(k), count(m + 1);
        for (int i = 0; i < k; ++i) {
            rem -= int(c[i].side[1].size());
            w[i] = c[i].weight();
            if (0 < w[i] && w[i] <= m) ++count[w[i]];
        }
        if (rem < 0) return 0;

        // Possible extra counterfeit counts after excluding up to two
        // components. Taking an item means its larger class is counterfeit.
        auto possibilities = [&](int a, int b) {
            unsigned bits = 1;
            for (int v = 1; v <= rem; ++v) {
                int cnt = min(rem / v, count[v] - (v == a) - (v == b));
                for (int t = 0; t < cnt; ++t) bits |= bits << v;
            }
            return bits;
        };
        vector<int> status(m + 1, 0);
        for (int v = 1; v <= rem; ++v) if (count[v]) {
            unsigned bits = possibilities(v, -1);
            bool light = (bits >> rem) & 1;
            bool heavy = (bits >> (rem - v)) & 1;
            status[v] = light && heavy ? -1 : int(heavy);
        }

        vector<int> active, tied, val(k);
        int anchor = -1;
        for (int i = 0; i < k; ++i) {
            if (!w[i]) {
                tied.push_back(i);
            } else {
                val[i] = w[i] <= m ? status[w[i]] : 0;
                if (val[i] < 0) active.push_back(i);
                else anchor = i;
            }
        }
        if (active.empty()) {
            for (int i : tied) {
                if (anchor < 0) return 0;
                val[i] = val[anchor] ^ ask(c[anchor].side[0][0], c[i].side[0][0]);
            }
            cout << '!';
            for (int i = 0; i < k; ++i)
                for (int x : c[i].side[val[i] ? 0 : 1]) cout << ' ' << x;
            cout << endl;
            return 0;
        }

        int x = -1, y = -1, relation = -1;
        int s = active.size();
        if (s <= 14) {
            // With few unknown orientations, choose the comparison that
            // splits the remaining feasible assignments most evenly.
            int target = rem;
            for (int i = 0; i < k; ++i)
                if (w[i] && val[i] == 1) target -= w[i];
            vector<int> masks, sum(1 << s);
            for (int z = 0; z < (1 << s); ++z) {
                if (z) sum[z] = sum[z & (z - 1)] + w[active[__builtin_ctz(z)]];
                if (sum[z] == target) masks.push_back(z);
            }
            int best = 0;
            for (int i = 0; i < s; ++i) {
                for (int j = i + 1; j < s + (anchor >= 0); ++j) {
                    int ones = 0;
                    for (int z : masks) ones += ((z >> i) ^ (z >> j)) & 1;
                    int score = min(ones, int(masks.size()) - ones);
                    if (score > best) {
                        best = score;
                        x = active[i];
                        y = j == s ? anchor : active[j];
                    }
                }
            }
        } else {
            // Pair light components first: a mismatch immediately charges
            // part of the counterfeit budget, often clearing many pairs.
            sort(active.begin(), active.end(), [&](int a, int b) {
                return pair(w[a], a) < pair(w[b], b);
            });

            // If almost every early pair agreed, preserve the ability to
            // finish blocks of size 11, rather than rounding 11 up to 16.
            int bins = n / (m + 1);
            if (!triedPacking && rem >= 8 && count[1] <= bins && bins > 0) {
                triedPacking = packed = true;
                priority_queue<pair<int,int>, vector<pair<int,int>>,
                               greater<pair<int,int>>> pq;
                for (int b = 0; b < bins; ++b) pq.emplace(0, b);
                for (auto it = active.rbegin(); it != active.rend(); ++it) {
                    auto [load, b] = pq.top();
                    pq.pop();
                    c[*it].bin = b;
                    pq.emplace(load + w[*it], b);
                }
            }
            if (rem < 8) packed = false;
            if (packed) {
                vector<int> first(bins, -1);
                for (int i : active) {
                    int b = c[i].bin;
                    if (b < 0) continue;
                    if (first[b] != -1) {
                        x = first[b];
                        y = i;
                        break;
                    }
                    first[b] = i;
                }
                if (x < 0) packed = false;
            }
            if (x < 0) x = active[0], y = active[1];

            unsigned bits = possibilities(w[x], w[y]);
            auto feasible = [&](int spent) {
                return spent <= rem && ((bits >> (rem - spent)) & 1u);
            };
            bool same = feasible(0) || feasible(w[x] + w[y]);
            bool diff = feasible(w[x]) || feasible(w[y]);
            if (!same || !diff) relation = int(diff);
        }
        if (x < 0 || y < 0) return 0;
        if (relation < 0) relation = ask(c[x].side[0][0], c[y].side[0][0]);
        for (int j = 0; j < 2; ++j) {
            auto &dst = c[x].side[j ^ relation];
            dst.insert(dst.end(), c[y].side[j].begin(), c[y].side[j].end());
        }
        if (c[x].side[0].size() < c[x].side[1].size())
            swap(c[x].side[0], c[x].side[1]);
        if (y != k - 1) c[y] = move(c.back());
        c.pop_back();
    }
}
