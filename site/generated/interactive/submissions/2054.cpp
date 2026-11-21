#include <bits/stdc++.h>
using namespace std;

int n, pairs_total;

int ask(int l, int r, int k) {
    if (l == r) return 1;
    cout << "? " << l << ' ' << r << ' ' << k << endl;
    int answer;
    if (!(cin >> answer) || answer < 1 || answer > r - l + 1) exit(0);
    return answer;
}

// Dividing by 2 leaves 1 alone, pairs (2,3), (4,5), ... and, for even
// n, a second singleton n. A is the later position of a pair, B the earlier.
// Length minus distinct counts pairs wholly inside the queried interval.
// A prefix thus counts its A's, and a suffix counts its B's. The third
// rank distinguishes n from 1 when both singletons are still in play.
int rank_at(int type, int cut) {
    if (type == 0) return cut - ask(1, cut, 2);
    if (type == 1) return pairs_total - (n - cut - ask(cut + 1, n, 2));
    if (cut >= 2) return ask(1, cut, n) == 2;
    return ask(cut + 1, n, n) == 1;
}

// The segment contains 1, with exactly a later endpoints, b earlier
// endpoints, and t singletons. The prior counts refer to its left prefix.
struct Segment {
    int before, prior_a, prior_b, a, b, t;
    int size() const { return a + b + t; }
    int open() const { return prior_b - prior_a; }
    int offset(int type) const { return type == 0 ? prior_a : prior_b; }
};

// For a short segment, optimize over arbitrary sequences of rank queries.
// Each world has a known answer and a row of predicted query responses.
struct World {
    int target;
    long double weight;
    vector<unsigned char> replies;
};

struct VectorHash {
    size_t operator()(const vector<int>& v) const {
        size_t hash = 0;
        for (int x : v) hash = hash * 1000003 + x + 1;
        return hash;
    }
};
struct Decision { long double cost; int depth, query; };

class SmallSearch {
    vector<World> worlds;
    array<unordered_map<vector<int>, Decision, VectorHash>, 8> memo;

    Decision plan(const vector<int>& ids, int budget) {
        int target = worlds[ids.front()].target;
        if (all_of(ids.begin(), ids.end(), [&](int id) { return worlds[id].target == target; }))
            return {0, 0, -1};
        if (budget == 0) return {numeric_limits<long double>::infinity(), 0, -1};
        auto found = memo[budget].find(ids);
        if (found != memo[budget].end()) return found->second;
        long double mass = 0;
        for (int id : ids) mass += worlds[id].weight;
        Decision best{numeric_limits<long double>::infinity(), INT_MAX, -1};
        for (int q = 0; q < (int)worlds.front().replies.size(); ++q) {
            array<vector<int>, 10> groups;
            for (int id : ids) groups[worlds[id].replies[q]].push_back(id);
            if (groups[worlds[ids.front()].replies[q]].size() == ids.size()) continue;
            long double cost = mass;
            int depth = 1;
            for (const auto& group : groups) if (!group.empty()) {
                Decision child = plan(group, budget-1);
                cost += child.cost;
                if (!isfinite(cost)) break;
                depth = max(depth, 1 + child.depth);
                if (cost > best.cost) break;
            }
            if (isfinite(cost) && (cost < best.cost || (cost == best.cost && depth < best.depth)))
                best = {cost, depth, q};
        }
        memo[budget].emplace(ids, best);
        return best;
    }

    int run(const Segment& s, int types) {
        vector<int> ids(worlds.size());
        iota(ids.begin(), ids.end(), 0);
        // Two ranks per bisection suffice, with at most one extra query
        // to separate n from 1. Keep the optimized tail within that bound.
        int budget = 2 * bit_width(unsigned(s.size()-1)) + (s.t == 2);
        while (!ids.empty()) {
            int q = plan(ids, budget).query;
            --budget;
            if (q < 0) return worlds[ids.front()].target;
            int type = q % types;
            int cut = s.before + 1 + q / types;
            int response = rank_at(type, cut) - (type < 2 ? s.offset(type) : 0);
            erase_if(ids, [&](int id) { return worlds[id].replies[q] != response; });
        }
        exit(0);
    }
public:
    int segment(const Segment& s, bool clustered) {
        int types = s.t == 2 ? 3 : 2;
        vector<int> word(s.a, 0);
        word.insert(word.end(), s.b, 1);
        word.push_back(2); // singleton 1
        if (s.t == 2) word.push_back(3); // singleton n
        do {
            int open = s.open();
            World w{0, 1, {}};
            array<int, 3> count{};
            int span = 0;
            for (int i = 0; i < s.size(); ++i) {
                if (word[i] == 0) {
                    if (!open) { w.weight = 0; break; }
                    w.weight *= open--;
                    ++count[0];
                } else if (word[i] == 1) { ++open; ++count[1]; }
                else if (word[i] == 2) w.target = s.before + i + 1;
                else ++count[2];
                span += open - s.open();
                if (i+1 < s.size()) w.replies.insert(w.replies.end(), count.begin(), count.begin()+types);
            }
            // In a cluster of nearby mates, shorter total pair spans are
            // more plausible. This changes query order only.
            if (clustered) w.weight = ldexp(w.weight, -2 * span);
            if (w.weight) worlds.push_back(move(w));
        } while (next_permutation(word.begin(), word.end()));
        return run(s, types);
    }
};

struct Split { int a, b, t; long double prob; };

// Count the ways of ordering endpoints inside the segment. An opening
// endpoint has one transition. A closing endpoint can match any open pair.
class Distribution {
    int a, b, t, width;
    vector<long double> left, right;
    int idx(int i, int j) const { return i * width + j; }
    static int choose(int m, int k) {
        if (k == 0) return 1;
        if (k == 1) return m;
        return m * (m-1) / 2;
    }
public:
    Distribution(int h, int a, int b, int t): a(a), b(b), t(t), width(b+1), left((a+1)*width), right(left.size()) {
        left[0] = 1;
        right[idx(a,b)] = 1;
        for (int i = 0; i <= a; ++i) for (int j = 0; j <= b; ++j) {
            int open = h + j - i;
            if (open < 0) continue;
            if (i) left[idx(i,j)] += (open+1) * left[idx(i-1,j)];
            if (j) left[idx(i,j)] += left[idx(i,j-1)];
        }
        for (int i = a; i >= 0; --i) for (int j = b; j >= 0; --j) {
            int open = h + j - i;
            if (open < 0) continue;
            if (i < a) right[idx(i,j)] += open * right[idx(i+1,j)];
            if (j < b) right[idx(i,j)] += right[idx(i,j+1)];
        }
    }
    vector<Split> splits(int cut) const {
        int m = a+b+t;
        vector<Split> out;
        for (int k = max(0,t-(m-cut)); k <= min(t,cut); ++k) {
            long double placement = (long double)choose(cut,k) * choose(m-cut,t-k) / choose(m,t);
            for (int i = max(0,cut-k-b); i <= min(a,cut-k); ++i) {
                int j = cut-k-i;
                long double paths = left[idx(i,j)] * right[idx(i,j)];
                if (paths > 0) out.push_back({i,j,k,placement * paths / right[0]});
            }
        }
        return out;
    }
};

// Optimize the cost of distinguishing the possible outcomes at one cut.
// The extra bit in an outcome names the side containing the target.
struct Outcome { array<int,3> reply; bool target_left; long double prob; };
vector<Outcome> outcomes(const vector<Split>& splits, int t) {
    vector<Outcome> out;
    for (auto s : splits) {
        if (t == 2 && s.t == 1) {
            out.push_back({{s.a,s.b,0},true,s.prob/2});
            out.push_back({{s.a,s.b,1},false,s.prob/2});
        } else out.push_back({{s.a,s.b,int(t == 2 && s.t == 2)},s.t > 0,s.prob});
    }
    return out;
}
// Measure savings relative to asking both pair ranks, plus the identity
// query if the two singletons split. Summing savings directly preserves
// tiny differences in the chances of inferring a rank without querying it.
struct RankChoice { long double saved; int type; };
RankChoice rank_plan(const vector<Outcome>& rows, int mask, int t) {
    if (rows.size() == 1) {
        const auto& r = rows.front();
        int split = t == 2 && r.target_left != bool(r.reply[2]);
        int used = (t == 2 ? 3 : 2) - popcount(unsigned(mask));
        return {r.prob * (2 + split - used), -1};
    }
    RankChoice best{-numeric_limits<long double>::infinity(),-1};
    for (int q = 0; q < 3; ++q) if (mask & (1<<q)) {
        int lo = rows.front().reply[q], hi = lo;
        for (auto &r : rows) { lo = min(lo,r.reply[q]); hi = max(hi,r.reply[q]); }
        if (lo == hi) continue;
        vector<vector<Outcome>> groups(hi-lo+1);
        for (auto &r : rows) groups[r.reply[q]-lo].push_back(r);
        long double saved = 0;
        for (auto &g : groups) if (!g.empty()) saved += rank_plan(g,mask^(1<<q),t).saved;
        if (saved > best.saved) best = {saved,q};
    }
    return best;
}

struct CutChoice { double cost; int cut; };
map<array<int,4>, CutChoice> cut_cache;
constexpr int planned_size = 48;
CutChoice choose_cut(int h, int a, int b, int t) {
    int m = a+b+t;
    if (m <= 1) return {0,0};
    auto key = array{h,a,b,t};
    if (auto it = cut_cache.find(key); it != cut_cache.end()) return it->second;
    Distribution d(h,a,b,t);
    CutChoice best{numeric_limits<double>::infinity(),0};
    for (int cut = 1; cut < m; ++cut) {
        auto splits = d.splits(cut);
        // A and B at the same cut determine the singleton count. After
        // the first rank, ask for the other only if several counts remain.
        array<vector<pair<int,int>>,2> range{vector<pair<int,int>>(a+1,{3,-1}),vector<pair<int,int>>(b+1,{3,-1})};
        for (auto &s : splits) for (int q = 0; q < 2; ++q) {
            auto &v = range[q][q ? s.b : s.a];
            v.first = min(v.first,s.t); v.second = max(v.second,s.t);
        }
        array<long double,2> cost{};
        for (int q = 0; q < 2; ++q)
            cost[q] = count_if(range[q].begin(),range[q].end(),[](auto v){return v.second >= 0;}) > 1;
        long double future = 0;
        for (auto &s : splits) {
            for (int q = 0; q < 2; ++q) {
                auto v = range[q][q ? s.b : s.a];
                if (v.first < v.second) cost[q] += s.prob;
            }
            if (s.t) future += s.prob * s.t / t * choose_cut(h,s.a,s.b,s.t).cost;
            if (t-s.t) future += s.prob * (t-s.t) / t * choose_cut(h+s.b-s.a,a-s.a,b-s.b,t-s.t).cost;
            if (t == 2 && s.t == 1) future += s.prob;
        }
        double total = future + min(cost[0],cost[1]);
        if (total < best.cost-1e-12 || (abs(total-best.cost)<1e-12 && abs(2*cut-m)<abs(2*best.cut-m))) best={total,cut};
    }
    return cut_cache[key] = best;
}

int solve() {
    int t = n % 2 ? 1 : 2;
    pairs_total = (n-t)/2;
    Segment s{0,0,0,pairs_total,pairs_total,t};
    bool clustered = false;
    while (s.size() > (s.t == 1 || s.open() == 0 ? 8 : 7)) {
        int cut = !clustered && s.size() <= planned_size ? choose_cut(s.open(),s.a,s.b,s.t).cut : s.size()/2;
        Distribution d(s.open(),s.a,s.b,s.t);
        auto splits = d.splits(cut);
        if (clustered) {
            // Given the singleton count, favor cuts with few crossing pairs.
            array<long double,3> mass{}, norm{};
            for (auto& x : splits) {
                mass[x.t] += x.prob;
                x.prob = ldexp(1.0L,-2 * (s.open() + x.b - x.a));
                norm[x.t] += x.prob;
            }
            for (auto& x : splits) x.prob *= mass[x.t] / norm[x.t];
        }
        auto rows = outcomes(splits,s.t);
        int mask = s.t == 1 ? 3 : 7;
        while (rows.size() > 1) {
            int q = rank_plan(rows,mask,s.t).type;
            if (q < 0) exit(0);
            int response = rank_at(q,s.before+cut) - (q < 2 ? s.offset(q) : 0);
            erase_if(rows,[&](auto &r){return r.reply[q] != response;});
            if (rows.empty()) exit(0);
            mask ^= 1<<q;
        }
        auto row = rows.front();
        int a = row.reply[0], b = row.reply[1], left_t = cut-a-b;
        bool first_cut = s.size() == n;
        if (row.target_left) { s.a = a; s.b = b; s.t = left_t; }
        else s = {s.before+cut,s.prior_a+a,s.prior_b+b,s.a-a,s.b-b,s.t-left_t};

        // An extreme number of pairs crossing the first cut is characteristic
        // of ordered inputs. Check the outside endpoint, where only one pair
        // rank can change. A failed check still removes that position.
        int crossing = b-a;
        if (first_cut && row.prob < 0.002L && crossing <= max(1,n/20)) clustered = true;
        if (first_cut && s.t == 1 && row.prob < 0.0001L &&
            (crossing <= 1 || crossing >= pairs_total-1)) {
            if (s.before == 0) {
                if (rank_at(1,1) == 0) return 1;
                ++s.before; ++s.prior_b; --s.b;
            } else {
                if (rank_at(0,n-1) == pairs_total) return n;
                --s.a;
            }
            // When all other endpoints have one type, the inside endpoint
            // can also be checked and removed with one query.
            if (s.a == 0 || s.b == 0) {
                int type = s.a ? 0 : 1;
                int& count = type == 0 ? s.a : s.b;
                if (row.target_left) {
                    int end = s.before + s.size();
                    if (rank_at(type,end-1) == s.offset(type) + count) return end;
                    --count;
                } else {
                    if (rank_at(type,s.before+1) == s.offset(type)) return s.before+1;
                    --count;
                    ++s.before;
                    if (type == 0) ++s.prior_a;
                    else ++s.prior_b;
                }
            }
        }
    }
    return SmallSearch().segment(s, clustered);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n < 3 || n > 1024) return 0;
    int target = solve();
    cout << "! " << target << endl;
}
