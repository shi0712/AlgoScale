#include <bits/stdc++.h>
using namespace std;

namespace {
constexpr int MMAX = 10, FIXED = MMAX + 1;
using Count = __int128_t;
using Poly = array<Count, MMAX + 1>;
using Edge = pair<int, int>;

Poly distribution(const vector<int>& weights, int r) {
    Poly p{};
    p[0] = 1;
    for (int w : weights)
        if (w > 0)
            for (int k = r; k >= w; --k) p[k] += p[k-w];
    return p;
}

Poly removeWeight(Poly p, int w, int r) {
    for (int k = w; k <= r; ++k) p[k] -= p[k-w];
    return p;
}

struct DSU {
    vector<int> parent, parity, size;
    explicit DSU(int n) : parent(n), parity(n), size(n,1) {
        iota(parent.begin(), parent.end(), 0);
    }
    int find(int x) {
        if (parent[x] == x) return x;
        int p = parent[x];
        parent[x] = find(p);
        parity[x] ^= parity[p];
        return parent[x];
    }
    void join(int x, int y, int different) {
        int a = find(x), b = find(y);
        if (a == b) return;
        if (size[a] > size[b]) swap(a,b);
        parent[a] = b;
        parity[a] = parity[x] ^ parity[y] ^ different;
        size[b] += size[a];
    }
};

struct Group {
    int count[2]{}, coin[2]{-1,-1};
    int heavy = 0, rep = -1, weight = 0, type = 0;
    bool fake = false;  // orientation of the heavy side if fixed
};

struct State {
    vector<Group> g;
    vector<int> id, side, members[FIXED+1];
    int remaining, activeRemaining, unknown = 0;
    Count total;
    Count merit[FIXED+1][FIXED+1]{};

    State(DSU& d, int n, int m) : id(n), side(n), remaining(m) {
        vector<int> index(n,-1), weights;
        for (int i = 0; i < n; ++i) {
            int root = d.find(i);
            if (index[root] < 0) {
                index[root] = (int)g.size();
                g.emplace_back();
            }
            id[i] = index[root];
            side[i] = d.parity[i];
            Group& h = g[id[i]];
            ++h.count[side[i]];
            h.coin[side[i]] = i;
        }
        for (Group& h : g) {
            h.heavy = h.count[1] > h.count[0];
            h.rep = h.coin[h.heavy];
            h.weight = abs(h.count[0]-h.count[1]);
            remaining -= h.count[1-h.heavy];
            weights.push_back(h.weight);
        }
        for (int i = 0; i < n; ++i) side[i] ^= g[id[i]].heavy;
        if (remaining < 0) exit(0);
        // Every group supplies its smaller side for certain, plus its
        // weight if the larger side is counterfeit. The coefficient at
        // remaining counts feasible orientations of all unbalanced groups.
        Poly p = distribution(weights,remaining);
        total = p[remaining];
        if (total == 0) exit(0);
        Poly removed[MMAX+1]{};
        Count one[MMAX+1]{};
        for (int a = 1; a <= remaining; ++a) {
            removed[a] = removeWeight(p,a,remaining);
            one[a] = removed[a][remaining-a];
        }
        activeRemaining = remaining;
        for (int i = 0; i < (int)g.size(); ++i) {
            Group& h = g[i];
            int a = h.weight;
            if (a && (a > remaining || one[a] == 0 || one[a] == total)) {
                h.type = FIXED;
                h.fake = a <= remaining && one[a] == total;
                if (h.fake) activeRemaining -= a;
            } else {
                h.type = a;
                ++unknown;
            }
            members[h.type].push_back(i);
        }
        // A fair split maximizes entropy. Ignore the common denominator.
        for (int a = 0; a <= FIXED; ++a) {
            for (int b = a; b <= FIXED; ++b) {
                if (members[a].empty() || members[b].empty() ||
                    (a == b && members[a].size() < 2)) continue;
                Count s = 0;
                if (!a) s = total;
                else if (a != FIXED) {
                    Count diff = one[a];
                    if (b != FIXED) {
                        Poly q = removeWeight(removed[a],b,remaining);
                        diff = q[remaining-a] + q[remaining-b];
                    }
                    s = 2 * min(diff,total-diff);
                }
                merit[a][b] = merit[b][a] = s;
            }
        }
    }
    Edge edgeFor(int a, int b) const {
        return {g[members[a][0]].rep, g[members[b][a==b]].rep};
    }
    int implied(Edge e) const {
        int x = id[e.first], y = id[e.second];
        int flip = side[e.first] ^ side[e.second];
        if (x == y) return flip;
        if (merit[g[x].type][g[y].type]) return -1;
        if (g[x].type == FIXED && g[y].type == FIXED)
            return flip ^ g[x].fake ^ g[y].fake;
        vector<int> w{g[x].weight+g[y].weight};
        for (int i = 0; i < (int)g.size(); ++i)
            if (i != x && i != y) w.push_back(g[i].weight);
        return flip ^ (distribution(w,remaining)[remaining] == 0);
    }
    Edge choose() const {
        Count best = 0;
        Edge choice{-1,-1};
        for (int a = 1; a < FIXED; ++a)
            for (int b = a; b <= FIXED; ++b)
                if (merit[a][b] > best) {
                    best = merit[a][b];
                    choice = edgeFor(a,b);
                }
        if (choice.first < 0 && !members[0].empty()) {
            for (int b = FIXED; b >= 0; --b)
                if (merit[0][b]) return edgeFor(0,b);
        }
        return choice;
    }
};

// Exact minimum expected depth for a small number of uncertain positive
// weights. Assignments of the heavy-side orientations are equiprobable.
struct Opt {
    int ways = 0, cost = 0, worst = 0;
    Edge types{-1,-1};
};
void consider(Opt& best, int a, int b, Opt same, Opt diff, bool balanced = false) {
    if (!same.ways || !diff.ways) return;
    int cost = best.ways + same.cost + (balanced ? 2*(diff.cost+diff.ways) : diff.cost);
    int worst = 1 + max(same.worst,diff.worst+(int)balanced);
    if (tie(cost,worst) < tie(best.cost,best.worst))
        best = {best.ways,cost,worst,{a,b}};
}

using Key = tuple<vector<int>,int,bool>;
map<Key,Opt> memo;

Opt endgame(vector<int> w, int r, bool anchor) {
    if (r < 0) return {};
    sort(w.begin(),w.end());
    Key key{w,r,anchor};
    auto it = memo.find(key);
    if (it != memo.end()) return it->second;
    Poly p = distribution(w,r);
    int ways = (int)p[r];
    if (ways <= 1) return {ways,0,0,{-1,-1}};

    vector<int> active;
    int newR = r;
    for (int a : w) {
        int n1 = a <= r ? (int)removeWeight(p,a,r)[r-a] : 0;
        if (n1 == ways) newR -= a;
        else if (n1) active.push_back(a);
    }
    if (active.size() < w.size())
        return memo[key] = endgame(active,newR,true);

    Opt best{ways,INT_MAX,INT_MAX,{-1,-1}};
    for (int i = 0; i < (int)w.size(); ++i) {
        int a = w[i];
        if (i && a == w[i-1]) continue;
        vector<int> rest = w;
        rest.erase(rest.begin()+i);
        if (anchor) consider(best,a,FIXED,endgame(rest,r,true),endgame(rest,r-a,true),false);
        for (int j = i+1; j < (int)w.size(); ++j) {
            int b = w[j];
            if (j > i+1 && b == w[j-1]) continue;
            vector<int> branch = rest;
            branch.erase(branch.begin()+j-1);
            branch.push_back(a+b);
            Opt same = endgame(branch,r,anchor);
            if (!same.ways) continue;
            branch.pop_back();
            if (a != b) branch.push_back(b-a);
            Opt diff = endgame(branch,r-a,anchor);
            consider(best,a,b,same,diff,a==b);
        }
    }
    return memo[key] = best;
}

// With two units of counterfeit weight, a world is either two marked
// single units or one marked double unit. This admits a larger exact DP.
class TwoUnits {
    vector<Opt> one;
    vector<vector<array<Opt,2>>> table;
public:
    explicit TwoUnits(int n) : one(n+1), table(n+1) {
        one[0] = {0,0,0,{-1,-1}};
        one[1] = {1,0,0,{-1,-1}};
        one[2] = {2,2,1,{1,FIXED}};
        for (int k = 3; k <= n; ++k)
            one[k] = {k,k+one[k-2].cost+2,1+max(1,one[k-2].worst),{1,1}};
    }
    Opt solve(int singles, int doubles, bool anchor) {
        int ways = singles*(singles-1)/2 + doubles;
        if (ways <= 1) return {ways,0,0,{-1,-1}};
        if (singles < 2) {
            Opt v = one[doubles];
            v.types = {2,doubles == 2 ? FIXED : 2};
            return v;
        }
        if (table[singles].empty()) table[singles].resize((table.size()-1-singles)/2+1);
        Opt& entry = table[singles][doubles][anchor];
        if (entry.ways) return entry;
        Opt best{ways,INT_MAX,INT_MAX,{-1,-1}};
        if (singles >= 3)
            consider(best,1,1,solve(singles-2,doubles+1,anchor),one[singles-2],true);
        if (doubles)
            consider(best,1,2,solve(singles-1,doubles-1,true),one[singles]);
        if (doubles >= 2)
            consider(best,2,2,solve(singles,doubles-2,true),one[1],true);
        if (anchor) {
            consider(best,1,FIXED,solve(singles-1,doubles,true),one[singles-1]);
            if (doubles)
                consider(best,2,FIXED,solve(singles,doubles-1,true),one[1]);
        }
        return entry = best;
    }
};

Edge preferred(const State& s, TwoUnits& two) {
    // After a long run of agreements, try to leave one group of ten and
    // groups of three. Only the ten can supply the required residue mod 3.
    if (s.id.size() == 1000 && s.remaining == 10 && s.g.size() <= 665) {
        if (s.members[10].empty()) {
            for (int a = 8; a >= 2; a -= 2)
                if (s.merit[2][a]) return s.edgeFor(2,a);
        } else {
            if (s.merit[1][2]) return s.edgeFor(1,2);
            if (!s.members[1].empty())
                for (int i : s.members[FIXED])
                    if (s.g[i].weight == 2) return {s.g[s.members[1][0]].rep,s.g[i].rep};
        }
    }
    bool anchor = !s.members[FIXED].empty();
    int positives = s.unknown - (int)s.members[0].size();
    int scale = 0;
    for (int a = 1; a <= s.activeRemaining; ++a)
        if (!s.members[a].empty()) scale = gcd(scale,a);
    int units = scale ? s.activeRemaining/scale : 0;
    if (units%2 && units >= (scale == 1 ? 5 : 3) && !s.members[2*scale].empty()) {
        // With exactly two odd weights, precisely one of them is marked.
        // Growing both to the remaining weight resolves all the even groups.
        int oddGroups = 0, smaller = 0;
        for (int a = scale; a <= s.activeRemaining; a += 2*scale) {
            oddGroups += (int)s.members[a].size();
            if (!smaller && !s.members[a].empty()) smaller = a;
        }
        if (oddGroups == 2 && smaller+2*scale <= s.activeRemaining && s.merit[smaller][2*scale])
            return s.edgeFor(smaller,2*scale);
    }
    int unit = s.activeRemaining/2;
    if (unit && s.activeRemaining == 2*unit) {
        int singles = (int)s.members[unit].size(), doubles = (int)s.members[2*unit].size();
        if (singles+doubles == positives && singles >= 2) {
            auto [a,b] = two.solve(singles,doubles,anchor).types;
            if (a > 0) return s.edgeFor(a*unit,b == FIXED ? FIXED : b*unit);
        }
    }
    if (positives > 0 && positives <= 11) {
        vector<int> w;
        for (const Group& h : s.g)
            if (h.type && h.type != FIXED) w.push_back(h.weight);
        Opt opt = endgame(w,s.activeRemaining,anchor);
        if (opt.types.first >= 0) return s.edgeFor(opt.types.first,opt.types.second);
    }
    return s.choose();
}

struct Item { int weight, rep; };

// Construct a response-independent completion containing the desired first
// comparison. We measure distance by the sum of the original group weights
// on which two possible assignments differ. It is even and at most 2*r.
bool completion(const State& s, Edge first, int budget, vector<Edge>& result, bool allowEven) {
    int x = s.id[first.first], y = s.id[first.second];
    int ref = s.members[FIXED].empty() ? -1 : s.g[s.members[FIXED][0]].rep;
    vector<Item> items;
    int unit = 0;
    for (int i = 0; i < (int)s.g.size(); ++i) {
        const Group& h = s.g[i];
        if (!h.type || h.type == FIXED) continue;
        unit = gcd(unit,h.weight);
        if (i != x && i != y) items.push_back({h.weight,h.rep});
    }
    // The two orientations in the contracted item flip together if both
    // assignments give the same response, even when that response is 1.
    if (s.g[x].type == 0 || s.g[y].type == 0 || !unit) return false;
    if (s.g[x].type != FIXED && s.g[y].type != FIXED)
        items.push_back({s.g[x].weight+s.g[y].weight,s.g[x].rep});
    int r = s.activeRemaining/unit;
    int odd = (r+1) | 1;
    vector<vector<int>> byWeight(2*MMAX+1);
    int left = (int)items.size();
    for (auto item : items) byWeight[item.weight/unit].push_back(item.rep);
    vector<Edge> edges;
    vector<pair<int,int>> bins;
    int partial = -1, partialWeight = 0;
    while (left) {
        int goal = allowEven ? r+1 : (partialWeight < odd ? odd : 2*r+1);
        int cap = goal-partialWeight;
        int a = min(cap,2*MMAX);
        while (a > 0 && byWeight[a].empty()) --a;
        if (a <= 0) {
            a = 2*MMAX;
            while (byWeight[a].empty()) --a;
        }
        int v = byWeight[a].back();
        byWeight[a].pop_back();
        --left;
        if (partial >= 0) edges.emplace_back(partial,v);
        partial = v;
        partialWeight += a;
        if ((partialWeight > r && (allowEven || partialWeight%2)) || partialWeight > 2*r) {
            bins.emplace_back(partial,partialWeight);
            partial = -1;
            partialWeight = 0;
        }
    }
    if (partial >= 0) {
        if (ref >= 0) edges.emplace_back(partial,ref);
        else {
            if (bins.empty()) return false;
            edges.emplace_back(partial,bins[0].first);
            int w = partialWeight+bins[0].second;
            if (!allowEven && w <= 2*r && !(w%2)) {
                if (bins.size() < 2) return false;
                edges.emplace_back(partial,bins[1].first);
            }
        }
    }
    // Any two surviving assignments differ on at most one whole bin.
    // Thus at most two survive. With the odd-bin condition even that
    // difference is impossible, since their distance must be even.
    for (int i : s.members[0]) edges.emplace_back(s.g[i].rep,first.first);
    if (1+(int)edges.size()+(int)allowEven > budget) return false;
    result = move(edges);
    return true;
}

struct Backup {
    vector<Edge> edges;
    bool blockTail = false;
};

Edge boundedChoice(const State& s, Edge desired, Backup& backup, int budget) {
    auto& plan = backup.edges;
    // A single connected component can be oriented by the total count.
    int simple = s.unknown - (s.members[FIXED].empty() ? 1 : 0);
    if (simple <= budget || (s.total << s.members[0].size()) <= 2) return desired;
    auto types = [&](Edge e) -> Edge {
        int a = s.g[s.id[e.first]].type, b = s.g[s.id[e.second]].type;
        return {min(a,b),max(a,b)};
    };
    Edge target = types(desired);
    int best = -1;
    Count score = 0;
    for (int i = 0; i < (int)plan.size(); ++i) {
        if (s.id[plan[i].first] == s.id[plan[i].second]) continue;
        auto t = types(plan[i]);
        if (t == target) {
            Edge e = plan[i];
            plan.erase(plan.begin()+i);
            return e;
        }
        Count v = s.merit[t.first][t.second];
        if (v > score) { score = v; best = i; }
    }
    if (completion(s,desired,budget,plan,false) || completion(s,desired,budget,plan,true)) {
        backup.blockTail = false;
        return desired;
    }
    if (best < 0) {
        if (!backup.blockTail) return desired;
        if (!plan.empty()) {
            Edge e = plan.back();
            plan.pop_back();
            return e;  // already implied, so join these groups for free
        }
        // Each finished block has at least six coins. At most one pure
        // block can be counterfeit. Two pure blocks give an anchor whether
        // they agree (both genuine) or differ (every other one genuine).
        if (s.members[FIXED].empty()) {
            int first = -1;
            for (const Group& h : s.g) if (!h.count[1-h.heavy]) {
                if (first >= 0) return {first,h.rep};
                first = h.rep;
            }
        } else {
            for (const Group& h : s.g)
                if (h.type != FIXED && h.count[1-h.heavy])
                    return {h.rep,s.g[s.members[FIXED][0]].rep};
        }
        return desired;
    }
    Edge e = plan[best];
    plan.erase(plan.begin()+best);
    return e;
}
} // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n,m;
    if (!(cin >> n >> m) || m < 0 || m > MMAX || 2*m >= n) return 0;
    vector<int> label(n);
    iota(label.begin(),label.end(),1);
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count()
                   ^ (uint64_t(random_device{}()) << 32));
    shuffle(label.begin(),label.end(),rng);
    DSU d(n);
    TwoUnits two(max(n,2));
    Backup backup;
    auto& plan = backup.edges;
    int budget;
    if (n == 1000 && m == 10) {
        // 330 triples plus a separate ten. The remaining backup edges
        // collect the triples into 66 six-coin and 66 nine-coin blocks.
        for (int i = 0; i < 990; i += 3) plan.emplace_back(i,i+1);
        for (int i = 990; i < 1000; i += 2) plan.emplace_back(i,i+1);
        for (int i = 992; i < 1000; i += 2) plan.emplace_back(990,i);
        for (int i = 0; i < 990; i += 3) plan.emplace_back(i,i+2);
        for (int i = 0; i < 396; i += 6) plan.emplace_back(i,i+3);
        for (int i = 396; i < 990; i += 9) {
            plan.emplace_back(i,i+3);
            plan.emplace_back(i,i+6);
        }
        backup.blockTail = true;
        // 867 forest edges. If there is a pure counterfeit block, its
        // size leaves at most four mixed blocks. Once oriented, at most
        // 66 equal-size candidates remain for the single marked block.
        budget = (int)plan.size() + 1 + 4 + 34;
    } else {
        int odd = (m+1) | 1;
        int bins = n/odd;
        if (bins%2 != n%2) --bins;
        bins = max(1,bins);
        int start = 0;
        for (int i = 0; i < bins; ++i) {
            int stop = (i+1 == bins) ? n : start+odd;
            for (int j = start+1; j < stop; ++j) plan.emplace_back(j-1,j);
            start = stop;
        }
        budget = (int)plan.size();
    }
    while (true) {
        State s(d,n,m);
        if (!s.unknown) {
            cout << '!';
            for (int i = 0; i < n; ++i)
                if (s.g[s.id[i]].fake ^ s.side[i]) cout << ' ' << label[i];
            cout << endl;
            return 0;
        }
        Edge desired = preferred(s,two);
        if (desired.first < 0) return 0;
        Edge e = budget > 0 ? boundedChoice(s,desired,backup,budget) : desired;
        int known = s.implied(e);
        if (known >= 0) {
            d.join(e.first,e.second,known);
            continue;
        }
        cout << "? " << label[e.first] << ' ' << label[e.second] << endl;
        int reply;
        if (!(cin >> reply) || (reply != 0 && reply != 1)) return 0;
        d.join(e.first,e.second,reply);
        --budget;
    }
}
