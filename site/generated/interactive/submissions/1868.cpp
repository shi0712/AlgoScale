#include <bits/stdc++.h>
using namespace std;

using Mask = bitset<200>;

[[noreturn]] void answer(bool connected) {
    cout << "! " << int(connected) << '\n' << flush;
    exit(0);
}

struct Solver {
    struct Group {
        Mask seeds;
        Mask members;
        int size;
        int born;
    };

    int n, covered = 0, epoch = 0;
    Mask all, seeds;
    vector<Group> groups;
    vector<int> deferred;
    vector<int> order;
    unordered_map<Mask, int> cache;
    mt19937 rng{static_cast<unsigned>(
        chrono::steady_clock::now().time_since_epoch().count())};

    explicit Solver(int n) : n(n), deferred(n, -1), order(n) {
        for (int i = 0; i < n; ++i) all.set(i);
        iota(order.begin(), order.end(), 0);
        shuffle(order.begin(), order.end(), rng);
        cache.reserve(4096);
        cache[Mask()] = 0;
        cache[all] = n;
    }

    static Mask one(int v) {
        Mask m;
        m.set(v);
        return m;
    }

    static Mask block(const vector<int>& a, int l, int r) {
        Mask m;
        for (int i = l; i < r; ++i) m.set(a[i]);
        return m;
    }

    // The oracle value used internally is |N[S]|, including S itself.
    int ask(const Mask& m) {
        auto it = cache.find(m);
        if (it != cache.end()) return it->second;

        string s(n, '0');
        for (int i = 0; i < n; ++i)
            if (m[i]) s[i] = '1';
        cout << "? " << s << '\n' << flush;

        int q;
        if (!(cin >> q) || q < 0 || q > n - int(m.count())) exit(0);
        int z = q + int(m.count());
        cache.emplace(m, z);
        return z;
    }

    void checkGroup(const Group& g) {
        if (g.size == n) answer(true);
        if (int(g.members.count()) == g.size) answer(false);
    }

    // The one uncovered vertex joins the connected cover iff its closed
    // neighborhood contains at least two vertices.
    [[noreturn]] void lastUncovered() {
        const Group& g = groups[0];
        vector<int> a;
        for (int v : order)
            if (!g.members[v] && deferred[v] < 0) a.push_back(v);

        int l = 0, r = int(a.size());
        while (r - l > 1) {
            int m = (l + r) / 2;
            bool left = ask(g.seeds | block(a, l, m)) == n;
            if (!left) {
                l = m;
            } else if (ask(g.seeds | block(a, m, r)) == n) {
                answer(true);
            } else {
                r = m;
            }
        }
        answer(false);
    }

    void checkCover() {
        if (groups.size() == 1 && covered == n - 1) lastUncovered();
    }

    // The covers of the current groups are pairwise disjoint. Given the
    // total overlap with a range of groups, one query splits that total.
    void findOverlaps(const vector<int>& ids, int l, int r, int weight,
                      int capacity, const Mask& x, int degree,
                      vector<int>& hit) {
        if (weight == 0 || l == r) return;
        if (r - l == 1 || weight > capacity - groups[ids[r - 1]].size) {
            for (int i = l; i < r; ++i) hit.push_back(ids[i]);
            return;
        }

        int cut = l + 1, leftCap = groups[ids[l]].size;
        int pref = leftCap;
        for (int i = l + 1; i + 1 < r; ++i) {
            pref += groups[ids[i]].size;
            if (abs(capacity - 2 * pref) < abs(capacity - 2 * leftCap)) {
                leftCap = pref;
                cut = i + 1;
            }
        }
        int rightCap = capacity - leftCap;
        Mask left = x, right = x;
        for (int i = l; i < cut; ++i) left |= groups[ids[i]].seeds;
        for (int i = cut; i < r; ++i) right |= groups[ids[i]].seeds;

        int wleft;
        auto it = cache.find(right);
        if (it != cache.end()) {
            wleft = weight - (degree + rightCap - it->second);
        } else {
            wleft = degree + leftCap - ask(left);
        }
        findOverlaps(ids, l, cut, wleft, leftCap, x, degree, hit);
        findOverlaps(ids, cut, r, weight - wleft, rightCap, x, degree, hit);
    }

    void insertProbe(int v, int nextCovered) {
        Mask x = one(v);
        int gain = nextCovered - covered;
        if (gain > 0) ++epoch;

        vector<int> ids, hit;
        int capacity = 0;
        for (int i = 0; i < int(groups.size()); ++i) {
            if (gain > 0 || groups[i].born <= deferred[v]) {
                ids.push_back(i);
                capacity += groups[i].size;
            }
        }
        sort(ids.begin(), ids.end(), [&](int i, int j) {
            return groups[i].size > groups[j].size;
        });

        if (gain == 0 && ids.size() == 1) {
            hit.push_back(ids[0]);
        } else if (gain == 0 && !ids.empty() && !cache.count(x) &&
                   2 * groups[ids[0]].size >= capacity) {
            int i = ids[0];
            int outside = ask(groups[i].seeds | x) - groups[i].size;
            if (outside == 0) {
                hit.push_back(i);
            } else if (outside == 1 && ids.size() == 2) {
                // A deferred vertex was covered by a different probe, so
                // its closed neighborhood cannot have size one.
                hit = ids;
            } else {
                int degree = ask(x);
                if (degree > outside) hit.push_back(i);
                findOverlaps(ids, 1, int(ids.size()), outside,
                             capacity - groups[i].size, x, degree, hit);
            }
        } else {
            int degree = groups.empty() ? nextCovered : ask(x);
            if (degree == 1) answer(false);
            if (degree == n) answer(true);
            findOverlaps(ids, 0, int(ids.size()), degree - gain,
                         capacity, x, degree, hit);
        }

        if (gain == 0 && hit.size() == 1) {
            Group& g = groups[hit[0]];
            g.members.set(v);
            deferred[v] = -1;
            checkGroup(g);
            return;
        }

        Group joined{gain > 0 ? x : Mask(), x, gain, epoch};
        vector<bool> used(groups.size(), false);
        for (int i : hit) {
            used[i] = true;
            joined.seeds |= groups[i].seeds;
            joined.members |= groups[i].members;
            joined.size += groups[i].size;
            joined.born = min(joined.born, groups[i].born);
        }
        checkGroup(joined);

        vector<Group> next;
        next.reserve(groups.size() + 1);
        for (int i = 0; i < int(groups.size()); ++i)
            if (!used[i]) next.push_back(groups[i]);
        next.push_back(joined);
        groups.swap(next);
        if (gain > 0) seeds |= x;
        covered = nextCovered;
        deferred[v] = -1;
        checkCover();
    }

    void noGain(const vector<int>& a, int l, int r) {
        if (groups.size() == 1) {
            groups[0].members |= block(a, l, r);
            checkGroup(groups[0]);
        } else {
            for (int i = l; i < r; ++i) deferred[a[i]] = epoch;
        }
    }

    // After handling the left half, its entire cover is already present.
    // The original target is therefore also the target for the right half.
    void coverRange(const vector<int>& a, int l, int r, int target) {
        if (target == covered) {
            noGain(a, l, r);
        } else if (r - l == 1) {
            insertProbe(a[l], target);
        } else {
            int m = (l + r) / 2;
            int left = ask(seeds | block(a, l, m));
            coverRange(a, l, m, left);
            coverRange(a, m, r, target);
        }
    }

    // With two disjoint covers of all vertices, a vertex merges them
    // exactly when its closed neighborhood touches both.
    [[noreturn]] void finishTwo(const vector<int>& pending, int start) {
        vector<int> a;
        for (int i = start; i < int(pending.size()); ++i) {
            int v = pending[i];
            if (deferred[v] < groups[0].born) {
                groups[1].members.set(v);
            } else if (deferred[v] < groups[1].born) {
                groups[0].members.set(v);
            } else {
                a.push_back(v);
            }
        }
        checkGroup(groups[0]);
        checkGroup(groups[1]);

        auto need = [&](int i) {
            return groups[i].size - int(groups[i].members.count());
        };
        int side = need(0) >= need(1) ? 0 : 1;
        Mask negatives;
        int limit = 1;

        auto classify = [&](auto&& self, int l, int r, bool knownBad) -> bool {
            Group& g = groups[side];
            if (!knownBad) {
                Mask m = block(a, l, r);
                if (ask(g.seeds | m) == g.size) {
                    g.members |= m;
                    checkGroup(g);
                    return false;
                }
            }
            if (r - l == 1) {
                negatives.set(a[l]);
                if (int(negatives.count()) > need(side ^ 1)) answer(true);
            } else {
                int m = (l + r) / 2;
                bool leftBad = self(self, l, m, false);
                self(self, m, r, !leftBad);
            }
            return true;
        };

        for (int l = 0; l < int(a.size());) {
            int rem = int(a.size()) - l;
            int other = need(side ^ 1) - int(negatives.count());
            int len = clamp(rem / (3 * max(1, other)), 1, 24);
            int r = min(int(a.size()), l + len);
            classify(classify, l, r, false);
            l = r;

            if (negatives.any() &&
                (int(negatives.count()) >= limit || l == int(a.size()))) {
                Group& g = groups[side ^ 1];
                if (ask(g.seeds | negatives) > g.size) answer(true);
                g.members |= negatives;
                checkGroup(g);
                negatives.reset();
                limit = min(16, 2 * limit);
                side = need(0) >= need(1) ? 0 : 1;
            }
        }
        answer(false);
    }

    int leafNeighbor(int a, int b) {
        vector<int> candidates;
        for (int v : order)
            if (v != a && v != b) candidates.push_back(v);
        int l = 0, r = int(candidates.size());
        while (r - l > 1) {
            int m = (l + r) / 2;
            Mask s = block(candidates, l, m);
            int z = ask(s);
            bool inside = z == n ||
                (z > m - l && ask(s | one(a)) == z);
            if (inside) r = m;
            else l = m;
        }
        return candidates[l];
    }

    int huntHub(const vector<int>& a, int threshold) {
        int l = 0, r = int(a.size());
        while (r - l > 1) {
            int m = (l + r) / 2;
            if (ask(block(a, l, m)) >= threshold) {
                r = m;
            } else if (ask(block(a, m, r)) >= threshold) {
                l = m;
            } else {
                return -1;
            }
        }
        return r > l && ask(one(a[l])) >= threshold ? a[l] : -1;
    }

    void run() {
        if (n <= 1) answer(true);
        if (n == 2) answer(ask(one(order[0])) == 2);
        if (n == 3) {
            for (int v : order) {
                int z = ask(one(v));
                if (z == 1) answer(false);
                if (z == 3) answer(true);
            }
            answer(false);
        }

        int a = order[0], b = order[1];
        int sa = ask(one(a));
        insertProbe(a, sa);
        int sab = ask(one(a) | one(b));
        coverRange(order, 1, 2, sab);

        vector<int> todo(order.begin() + 2, order.end());
        auto it = cache.find(one(b));
        int sb = it == cache.end() ? 0 : it->second;

        // Two sampled leaves suggest that probing a neighbor directly
        // can either expose a tiny component or find a useful hub.
        if (n >= 14 && sa == 2 && sb == 2) {
            int c = leafNeighbor(a, b);
            Mask xc = one(c), xb = one(b), xa = one(a);
            int sc = ask(xc);
            if (sc == 2) answer(false);
            if (sc == n) answer(true);
            groups.clear();
            ++epoch;
            if (sab == 3) {
                seeds = xc;
                covered = sc;
                groups.push_back({xc, xa | xb | xc, sc, epoch});
            } else {
                seeds = xc | xb;
                covered = ask(seeds);
                if (covered < sc + 2) {
                    groups.push_back({seeds, xa | xb | xc, covered, epoch});
                } else {
                    groups.push_back({xc, xa | xc, sc, epoch});
                    groups.push_back({xb, xb, 2, epoch});
                }
            }
            for (const Group& g : groups) checkGroup(g);
            checkCover();
            todo.erase(find(todo.begin(), todo.end(), c));
        } else if (n >= 24 && min(sa, sb) >= 3 &&
                   max(sa, sb) <= max(4, n / 7) &&
                   sa + sb - sab >= 2 && sab <= max(sa, sb) + 2) {
            int v = huntHub(todo, n - max(1, n / 30));
            if (v >= 0) {
                if (ask(one(v)) == n) answer(true);
                auto p = find(todo.begin(), todo.end(), v);
                int i = int(p - todo.begin());
                coverRange(todo, i, i + 1, ask(seeds | one(v)));
                todo.erase(p);
            }
        }

        int quiet = 0;
        for (int l = 0; l < int(todo.size());) {
            if (covered == n) {
                noGain(todo, l, int(todo.size()));
                break;
            }
            int rem = int(todo.size()) - l;
            int missing = n - covered;
            int len = clamp(rem / (3 * missing), 1, 24);
            if (quiet > 0 && missing * 3 < rem)
                len = min(32, len << min(quiet, 3));
            int r = min(int(todo.size()), l + len);
            int old = covered;
            int target = r == int(todo.size()) ? n
                       : ask(seeds | block(todo, l, r));
            coverRange(todo, l, r, target);
            quiet = old == covered ? quiet + 1 : 0;
            l = r;
        }

        vector<int> pending;
        for (int v : order)
            if (deferred[v] >= 0) pending.push_back(v);
        stable_sort(pending.begin(), pending.end(), [&](int x, int y) {
            return deferred[x] < deferred[y];
        });

        for (int i = 0; i < int(pending.size()); ++i) {
            if (groups.size() == 2) finishTwo(pending, i);
            insertProbe(pending[i], n);
        }
        answer(groups.size() == 1);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 0 || n > 200) return 0;
    Solver(n).run();
}
