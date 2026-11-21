#include <bits/stdc++.h>
using namespace std;

using Set = bitset<200>;

int n;
unordered_map<Set, int> memo;

[[noreturn]] void answer(int x) {
    cout << "! " << x << endl;
    exit(0);
}

// Size of the union of the closed neighborhoods of s.
int volume(const Set& s) {
    int k = (int)s.count();
    if (k == 0 || k == n) return k;
    auto it = memo.find(s);
    if (it != memo.end()) return it->second;

    string b(n, '0');
    for (int i = 0; i < n; ++i)
        if (s[i]) b[i] = '1';
    cout << "? " << b << endl;
    int r;
    if (!(cin >> r) || r < 0 || r > n - k) exit(0);
    if (r == 0) answer(0);
    if (k == 1 && r == n - 1) answer(1);
    return memo[s] = k + r;
}

struct Group {
    Set s;
    int vol;
    int frontier() const { return vol - (int)s.count(); }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    if (n == 1) answer(1);

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count());
    shuffle(order.begin(), order.end(), rng);

    // The closed neighborhood of each group lies in a single component,
    // and the different groups' closed neighborhoods are disjoint.
    vector<Group> g;
    Set processed;
    int total = 0, pos = 0;
    int redundantRun = 0;

    auto check = [&](const Group& a) {
        if (a.vol == n) answer(1);
        if (a.frontier() == 0) answer(0);
    };

    while (pos < n) {
        int rem = n - pos;
        sort(g.begin(), g.end(), [](const Group& a, const Group& b) {
            return a.frontier() > b.frontier();
        });

        Set v;
        v.set(order[pos]);

        // When many remaining vertices are likely to be entirely covered
        // by one group, try absorbing several with a single equality test.
        bool absorbed = false;
        if (!g.empty()) {
            int f = g[0].frontier();
            int batch = 1;
            if (redundantRun >= 2 && f * 4 >= 3 * rem) {
                batch = min({f, redundantRun + 1,
                             max(2, rem / max(1, rem - f))});
            }
            if (batch > 1 || (total == n && g.size() == 2) ||
                (g.size() > 1 && 4 * f > 3 * rem)) {
                Set t = v;
                for (int i = 1; i < batch; ++i) t.set(order[pos + i]);
                if (volume(g[0].s | t) == g[0].vol) {
                    g[0].s |= t;
                    processed |= t;
                    pos += batch;
                    redundantRun += batch;
                    check(g[0]);
                    absorbed = true;
                } else if (batch == 1 && total == n && g.size() == 2) {
                    // The two disjoint covers partition V. Unless N[v] is
                    // contained in the other one, it joins the two.
                    if (volume(g[1].s | v) != g[1].vol) answer(1);
                    g[1].s |= v;
                    processed |= v;
                    ++pos;
                    check(g[1]);
                    redundantRun = 0;
                    absorbed = true;
                } else if (batch > 1) {
                    redundantRun = 0;
                }
            }
        }
        if (absorbed) continue;

        int nextTotal = total == n ? n : volume(processed | v);
        int gain = nextTotal - total;
        if (gain == 0 && g.size() == 1) {
            g[0].s |= v;
            check(g[0]);
            processed |= v;
            ++pos;
            ++redundantRun;
            continue;
        }
        redundantRun = 0;

        int sv = g.empty() ? nextTotal : volume(v);
        int overlap = sv - gain;
        vector<char> hit(g.size(), false);

        // Given the total intersection of N[v] with a range of disjoint
        // covers, querying one half also determines the other half.
        auto locate = [&](auto&& self, int l, int r, int mass, int inter) -> void {
            if (inter == 0) return;
            if (r - l == 1 || inter == mass) {
                fill(hit.begin() + l, hit.begin() + r, true);
                return;
            }
            bool forced = true;
            for (int i = l; i < r; ++i)
                if (mass - g[i].vol >= inter) { forced = false; break; }
            if (forced) {
                fill(hit.begin() + l, hit.begin() + r, true);
                return;
            }
            int m = l + 1, leftMass = g[l].vol;
            Set s = g[l].s;
            while (m + 1 < r &&
                   abs(2 * (leftMass + g[m].vol) - mass) <
                   abs(2 * leftMass - mass)) {
                leftMass += g[m].vol;
                s |= g[m++].s;
            }
            int a = sv + leftMass - volume(s | v);
            self(self, l, m, leftMass, a);
            self(self, m, r, mass - leftMass, inter - a);
        };
        locate(locate, 0, (int)g.size(), total, overlap);

        Group joined{v, gain};
        vector<Group> keep;
        for (int i = 0; i < (int)g.size(); ++i) {
            if (hit[i]) {
                joined.s |= g[i].s;
                joined.vol += g[i].vol;
            } else {
                keep.push_back(g[i]);
            }
        }
        check(joined);
        keep.push_back(joined);
        g.swap(keep);
        processed |= v;
        total = nextTotal;
        ++pos;
    }
    answer(g.size() == 1);
}
