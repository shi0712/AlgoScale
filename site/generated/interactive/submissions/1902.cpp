#include <bits/stdc++.h>
using namespace std;

constexpr int MAXN = 600;
using Set = bitset<MAXN>;

int n, total = -1, fixedEdges, fixedPairs;
bool treeGraph;
Set allV, adj[MAXN], fixedV[MAXN];
unordered_map<Set, int> memo;

void answer(char type, const vector<int>& a) {
    cout << type << ' ' << a.size() << '\n';
    for (int i = 0; i < (int)a.size(); ++i)
        cout << (i ? " " : "") << a[i] + 1;
    cout << '\n' << flush;
    exit(0);
}

vector<int> elements(const Set& s) {
    vector<int> a;
    for (size_t i = s._Find_first(); i < MAXN; i = s._Find_next(i))
        a.push_back((int)i);
    return a;
}

int query(const Set& s) {
    int k = s.count();
    if (k < 2) return 0;
    auto it = memo.find(s);
    if (it != memo.end()) return it->second;

    int pairs = k * (k - 1) / 2;
    int present = 0, determined = 0;
    for (size_t i = s._Find_first(); i < MAXN; i = s._Find_next(i)) {
        present += (s & adj[i]).count();
        determined += (s & fixedV[i]).count();
    }
    present /= 2;
    determined /= 2;
    int unknown = pairs - determined;
    int lo = present, hi = present + unknown;
    if (total >= 0) {
        int rem = total - fixedEdges;
        int outside = n * (n - 1) / 2 - fixedPairs - unknown;
        lo = max(lo, present + rem - outside);
        hi = min({hi, present + rem, total - (n - k)});
    }
    int value = lo;
    if (lo < hi) {
        cout << "? " << k << '\n';
        bool first = true;
        for (size_t i = s._Find_first(); i < MAXN; i = s._Find_next(i)) {
            if (!first) cout << ' ';
            cout << i + 1;
            first = false;
        }
        cout << '\n' << flush;
        if (!(cin >> value) || value < 0) exit(0);
    }
    memo.emplace(s, value);
    if (unknown && (value == present || value == present + unknown)) {
        bool fill = value != present;
        for (size_t i = s._Find_first(); i < MAXN; i = s._Find_next(i)) {
            if (fill) {
                Set t = s & ~fixedV[i];
                t.reset(i);
                adj[i] |= t;
            }
            fixedV[i] |= s;
            fixedV[i].reset(i);
        }
        fixedPairs += unknown;
        if (fill) fixedEdges += unknown;
    }
    if (value == pairs && k >= 3) {
        auto a = elements(s);
        a.resize(3);
        answer('N', a);
    }
    return value;
}

void fixCross(const Set& f, const Set& g, bool fill) {
    for (size_t i = g._Find_first(); i < MAXN; i = g._Find_next(i)) {
        Set t = f & ~fixedV[i];
        fixedPairs += t.count();
        if (fill) {
            fixedEdges += t.count();
            adj[i] |= t;
        }
        fixedV[i] |= f;
        for (size_t j = t._Find_first(); j < MAXN; j = t._Find_next(j)) {
            fixedV[j].set(i);
            if (fill) adj[j].set(i);
        }
    }
}

// The frontier is independent. Split a group at stable binary boundaries
// so that counts of unchanged subgroups can be reused in later layers.
Set frontier, nextLayer;
int nextCheck;
Set splitGroup(const Set& g, const vector<int>& a) {
    int b = bit_floor((unsigned)(a.front() ^ a.back()));
    int cut = (a.back() / b) * b;
    Set t;
    for (int x : a) {
        if (x >= cut) break;
        t.set(x);
    }
    if (t.count() > g.count() / 2) t = g ^ t;
    return t;
}

void includeNext(const Set& g) {
    nextLayer |= g;
    if ((int)nextLayer.count() >= nextCheck) {
        if (query(nextLayer) > 0) throw 1;
        nextCheck = max(nextCheck * 3, (int)nextLayer.count() * 2);
    }
}

int crossCount(const Set& g) {
    int lower = 0, unknown = 0;
    for (size_t i = g._Find_first(); i < MAXN; i = g._Find_next(i)) {
        lower += (frontier & adj[i]).count();
        unknown += (frontier & ~fixedV[i]).count();
    }
    if (!unknown) return lower;
    int joined = query(frontier | g);
    if (joined == lower) return lower;
    return joined - query(g);
}

void discover(const Set& g, int count) {
    if (!count) {
        fixCross(frontier, g, false);
        return;
    }
    vector<int> a = elements(g);
    Set hit, possible;
    int lo = 0, hi = 0;
    for (int x : a) {
        int l = (frontier & adj[x]).count();
        int h = l + (frontier & ~fixedV[x]).count();
        if (l) hit.set(x);
        if (h) possible.set(x);
        lo += l;
        hi += h;
    }
    if (count == lo || count == hi) {
        fixCross(frontier, g, count != lo);
        includeNext(count == lo ? hit : possible);
        return;
    }
    if (possible.count() == 1 || hit == possible ||
        (treeGraph && count == (int)possible.count())) {
        includeNext(possible);
        return;
    }
    Set t;
    if (count * 4 >= (int)a.size()) {
        // A singleton has zero internal edges and needs at most one query.
        t.set(a.back());
    } else {
        t = splitGroup(g, a);
    }
    int d = crossCount(t);
    discover(t, d);
    discover(g ^ t, count - d);
}

int parentIn(int v, const Set& layer) {
    Set known = layer & adj[v];
    if (known.any()) return known._Find_first();
    Set candidates = layer & ~fixedV[v];
    while (candidates.count() > 1) {
        auto a = elements(candidates);
        Set t;
        for (size_t i = 0; i < a.size() / 2; ++i) t.set(a[i]);
        Set q = t;
        q.set(v);
        if (query(q)) candidates = t;
        else candidates ^= t;
    }
    return candidates._Find_first();
}

void oddCycle(const Set& layer, const vector<Set>& earlier) {
    auto a = elements(layer);
    int x = -1, y = -1;
    for (int u : a) {
        Set t = layer & adj[u];
        if (t.any()) {
            x = u;
            y = t._Find_first();
            break;
        }
    }
    if (x < 0) {
        // First prefix containing an edge; its last vertex is one endpoint.
        int lo = 1, hi = a.size();
        while (hi - lo > 1) {
            int mid = (lo + hi) / 2;
            Set t;
            for (int j = 0; j < mid; ++j) t.set(a[j]);
            if (query(t)) hi = mid;
            else lo = mid;
        }
        x = a[hi - 1];
        Set pref;
        for (int j = 0; j < hi - 1; ++j) pref.set(a[j]);
        y = parentIn(x, pref);
    }
    vector<int> left{x}, right{y};
    for (int i = (int)earlier.size() - 1; i >= 0; --i) {
        x = parentIn(x, earlier[i]);
        y = parentIn(y, earlier[i]);
        left.push_back(x);
        if (x == y) break;
        right.push_back(y);
    }
    reverse(right.begin(), right.end());
    left.insert(left.end(), right.begin(), right.end());
    answer('N', left);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n < 1) return 0;
    if (n <= 2) answer('Y', {0});
    for (int i = 0; i < n; ++i) allV.set(i);
    total = query(allV);
    treeGraph = total == n - 1;

    frontier.set(0);
    Set remaining = allV ^ frontier, side;
    vector<Set> layers;
    int combined = total;
    for (;;) {
        if (!treeGraph && query(frontier)) oddCycle(frontier, layers);
        if (layers.size() % 2 == 0) side |= frontier;
        layers.push_back(frontier);

        int internal = query(remaining);
        if (!internal) {
            // No remaining vertex has an edge to a layer before frontier.
            // Since the remaining set is independent, connectivity makes
            // every one of its vertices a member of the very next layer.
            if (layers.size() % 2 == 0) side |= remaining;
            if (side.count() > n / 2) side ^= allV;
            answer('Y', elements(side));
        }
        nextLayer.reset();
        nextCheck = (!treeGraph && total * 10 > n * n) ? 3 : n + 1;
        try {
            discover(remaining, combined - internal);
        } catch (int) {
            oddCycle(nextLayer, layers);
        }
        frontier = nextLayer;
        remaining ^= frontier;
        combined = internal;
    }
}
