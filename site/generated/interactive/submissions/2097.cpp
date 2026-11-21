#include <bits/stdc++.h>
using namespace std;

using ll = long long;
constexpr int M = 1000000000;
using Triple = array<int, 3>;

struct Interaction {
    int n;
    vector<int> label, value;
    map<Triple, int> replies;

    explicit Interaction(int size) : n(size), label(n), value(n, -1) {
        iota(label.begin(), label.end(), 1);
        mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
        // Start with a short consecutive sample. The shared index of the
        // first two queries is its center, with two neighbors on each side.
        int block = n >= 7 ? 7 : n;
        int start = uniform_int_distribution<int>(0,n-block)(rng);
        if (rng() & 1) reverse(label.begin(),label.end());
        rotate(label.begin(),label.begin()+start,label.end());
        const int layout[] = {2,0,1,4,3};
        auto original = label;
        for (int i = 0; i < 5; ++i) label[i] = original[layout[i]];
        shuffle(label.begin()+block,label.end(),rng);
    }

    int ask(int i, int j, int k) {
        Triple t{i, j, k};
        sort(t.begin(), t.end());
        auto it = replies.find(t);
        if (it != replies.end()) return it->second;
        cout << "? " << label[i] << ' ' << label[j] << ' ' << label[k] << endl;
        ll r;
        if (!(cin >> r) || r < 0 || r > 2LL * M) exit(0);
        return replies[t] = (int)r;
    }
};

// For a fixed ordering every observation is an equation x_i+x_j=r.
// Store upper bounds on differences of the signed variables x_i and -x_i.
struct Bounds {
    ll d[14][14], half[14];
    int size;

    explicit Bounds(int k) : size(2*k) {
        for (int i = 0; i < size; ++i)
            for (int j = 0; j < size; ++j)
                d[i][j] = i == j ? 0 : ((!(i&1)) + (j&1)) * (ll)M;
    }

    void constrain(int i, int j, ll b) {
        d[i][j] = min(d[i][j], b);
        d[j^1][i^1] = min(d[j^1][i^1], b);
    }

    bool close() {
        for (int k = 0; k < size; ++k) {
            if (d[k][k] < 0) return false;
            for (int i = 0; i < size; ++i) {
                ll left = d[i][k];
                for (int j = 0; j < size; ++j)
                    d[i][j] = min(d[i][j], left + d[k][j]);
            }
        }
        for (int i = 0; i < size; ++i) {
            if (d[i][i] < 0) return false;
            half[i] = d[i][i^1] >> 1;
        }
        // Integer tight closure: each unary difference is twice an integer.
        for (int i = 0; i < size; i += 2)
            if (half[i] + half[i+1] < 0) return false;
        return true;
    }

    int upper(int i, int j) const {
        return (int)min(d[i][j], half[i] + half[j^1]);
    }
};

struct Order {
    array<int, 7> seq{};
    array<pair<int,int>, 35> ends{};
};

struct SeedSolver {
    Interaction &io;
    int k;
    vector<Triple> triples;
    vector<Order> orders;

    SeedSolver(Interaction &io, int k) : io(io), k(k) {
        for (int a = 0; a < k; ++a)
            for (int b = a+1; b < k; ++b)
                for (int c = b+1; c < k; ++c)
                    triples.push_back({a,b,c});
        Order o;
        iota(o.seq.begin(), o.seq.end(), 0);
        do {
            int rank[7];
            for (int i = 0; i < k; ++i) rank[o.seq[i]] = i;
            for (int t = 0; t < (int)triples.size(); ++t) {
                auto v = triples[t];
                sort(v.begin(), v.end(), [&](int a, int b){return rank[a] < rank[b];});
                o.ends[t] = {2*v[0], 2*v[2]+1};
            }
            orders.push_back(o);
        } while (next_permutation(o.seq.begin(), o.seq.begin()+k));
    }

    // Estimate how well a query distinguishes orders, and reward acquiring
    // a new equation within an order. The estimate only chooses the query.
    static double merit(const vector<pair<int,int>> &ranges) {
        vector<pair<int,double>> events;
        events.reserve(2*ranges.size());
        double within = 0, gain = 0;
        for (auto [l,h] : ranges) {
            double w = (double)h-l+1;
            events.emplace_back(l, 1/w);
            events.emplace_back(h+1, -1/w);
            double bits = log2(w);
            within += bits;
            gain += min(5.0, bits);
        }
        sort(events.begin(), events.end());
        double density = 0, entropy = 0;
        int prev = events[0].first;
        for (auto [x,delta] : events) {
            if (x > prev && density > 0)
                entropy -= (x-prev) * density * log2(density);
            density += delta;
            prev = x;
        }
        return (entropy - within + gain) / (double)ranges.size() + log2(ranges.size());
    }

    void solve() {
        int nt = (int)triples.size();
        for (;;) {
            vector<pair<int,int>> known;
            vector<int> unused;
            for (int t = 0; t < nt; ++t) {
                auto it = io.replies.find(triples[t]);
                if (it == io.replies.end()) unused.push_back(t);
                else known.emplace_back(t, it->second);
            }
            vector<vector<pair<int,int>>> ranges(nt);
            array<int,7> low, high;
            low.fill(M);
            high.fill(0);
            size_t kept = 0;
            for (const auto &o : orders) {
                Bounds b(k);
                for (int i = 1; i < k; ++i) {
                    int x = o.seq[i-1], y = o.seq[i];
                    b.constrain(2*x, 2*y, -(x>y)); // break equal-score ties by index
                }
                for (auto [t,r] : known) {
                    auto [x,y] = o.ends[t];
                    b.constrain(x,y,r);
                    b.constrain(y,x,-r);
                }
                if (!b.close()) continue;
                for (int i = 0; i < k; ++i) {
                    low[i] = min(low[i], (int)-b.half[2*i+1]);
                    high[i] = max(high[i], (int)b.half[2*i]);
                }
                for (int t : unused) {
                    auto [x,y] = o.ends[t];
                    ranges[t].emplace_back(-b.upper(y,x), b.upper(x,y));
                }
                orders[kept++] = o;
            }
            orders.resize(kept);
            if (kept == 0) exit(0);
            if (equal(low.begin(), low.begin()+k, high.begin())) {
                copy_n(low.begin(), k, io.value.begin());
                return;
            }
            int best = -1;
            double bestMerit = -1;
            for (int t : unused) {
                const auto &v = ranges[t];
                if (all_of(v.begin(), v.end(), [&](auto p) {
                    return p.first == v[0].first && p.second == v[0].first;
                })) continue;
                double score = merit(v);
                if (score > bestMerit) bestMerit = score, best = t;
            }
            // All triples distinguish the scores: for x_i < x_j some pair
            // of the other indices gives a strict comparison (three other
            // indices cannot all pairwise bracket [x_i,x_j]). In sorted
            // order we then know all sums at index distance >= 2. These
            // edges are connected and contain the triangle 1,3,5.
            if (best < 0) exit(0);
            auto [x,y,z] = triples[best];
            io.ask(x,y,z);
        }
    }
};

// In a triple with reply r, the sorted scores have the form t,u,r-t,
// where max(0,r-M) <= t <= floor(r/2) and t <= u <= r-t.
// A single score, or a min/max of two scores, fixes one of these three
// numbers in each possible ordering.
struct Probe {
    int i, j, anchor;
    bool maximum;
    int predict(const Triple &a) const {
        return maximum ? max(a[i],a[j]) : min(a[i],a[j]);
    }
};

struct GroupPlan {
    const vector<Probe> &probes;
    vector<Triple> candidates;
    vector<double> weights;
    map<vector<int>,pair<double,int>> memo;

    explicit GroupPlan(const vector<Probe> &p) : probes(p) {}

    pair<double,int> choose(const vector<int> &ids) {
        if (ids.size() == 1) return {0,-1};
        auto it = memo.find(ids);
        if (it != memo.end()) return it->second;
        double mass = 0;
        for (int id : ids) mass += weights[id];
        pair<double,int> best{1e100,-1};
        for (int q = 0; q < (int)probes.size(); ++q) {
            map<int,vector<int>> split;
            for (int id : ids)
                split[probes[q].predict(candidates[id])].push_back(id);
            if (split.size() == 1) continue;
            double cost = mass;
            for (const auto &entry : split) {
                cost += choose(entry.second).first;
                if (cost >= best.first) break;
            }
            if (cost < best.first) best = {cost,q};
        }
        return memo[ids] = best;
    }
};

struct Decoder {
    Interaction &io;
    vector<pair<int,int>> known;
    int left = -1, right = -1;
    int zeros = 0, tops = 0;
    map<int,int> frequency;
    double base[2]{}, samples = 0;

    static constexpr int strides = 4;
    vector<int> at;
    int trials[2][strides+1]{}, hits[2][strides+1]{};

    explicit Decoder(Interaction &session) : io(session), at(io.n+1,-1) {}

    void add(int i, int x) {
        io.value[i] = x;
        for (auto [v,j] : known) {
            if (left < 0 || abs(x-v) < io.value[right]-io.value[left]) {
                left = v <= x ? j : i;
                right = v <= x ? i : j;
            }
        }
        known.emplace_back(x,i);
        ++frequency[x];
        int pos = io.label[i];
        at[pos] = x;
        for (int d = 1; d <= strides; ++d) {
            for (int other : {pos-d,pos+d}) {
                if (other < 1 || other > io.n || at[other] < 0) continue;
                for (int b : {0,1}) {
                    int v = b*M;
                    trials[b][d] += (x == v) + (at[other] == v);
                    hits[b][d] += 2 * (x == v && at[other] == v);
                }
            }
        }
        zeros += x == 0;
        tops += x == M;
    }

    int readOne(int i) {
        int a = left, b = right;
        int x = io.value[a], y = io.value[b];
        int r = io.ask(i,a,b);
        if (x == y) return r-x;
        if (r < x+y) return r-y;
        if (r > x+y) return r-x;
        // A closest distinct pair has no other known score in its interior.
        for (auto [v,j] : known) {
            if (j == a || j == b) continue;
            if (v <= x) return io.ask(i,j,a) - v;
            if (v >= y) return io.ask(i,j,b) - v;
        }
        exit(0);
    }

    void group(int first, int count, int boundary) {
        int anchor = -1;
        for (auto [v,j] : known)
            if (v == boundary) { anchor = j; break; }
        int z = count == 3 ? first+2 : anchor;
        int r = io.ask(first, first+1, z);
        int l = max(0, r-M), h = min(M,r);
        if (l == h) {
            for (int j = 0; j < count; ++j) add(first+j,l);
            return;
        }
        int below = -1, above = -1;
        for (auto [v,j] : known) {
            if (v <= l) below = j;
            if (v >= h) above = j;
        }
        vector<Probe> probes;
        for (int i = 0; i < count; ++i) {
            probes.push_back({i,i,-1,false});
            for (int j = 0; j < i; ++j) {
                if (below >= 0) probes.push_back({j,i,below,true});
                if (above >= 0) probes.push_back({j,i,above,false});
            }
        }
        set<int> grid{l,h,r/2,r-r/2};
        // Model frequent previously seen interior scores as well.
        pair<int,int> most{0,0};
        for (auto it = frequency.lower_bound(l); it != frequency.end() && it->first <= h; ++it)
            if (!grid.count(it->first)) most = max(most,{it->second,it->first});
        if (most.first > 1) {
            grid.insert(most.second);
            grid.insert(r-most.second);
        }
        vector<pair<int,int>> observations;
        for (;;) {
            GroupPlan plan{probes};
            for (int t : grid) {
                if (t > r-t) break;
                for (int u : grid) {
                    if (u < t || u > r-t) continue;
                    Triple a{t,u,r-t};
                    double weight = 1;
                    for (int v : a) {
                        auto it = frequency.find(v);
                        weight *= 1.0 + (it == frequency.end() ? 0 : it->second);
                    }
                    do {
                        if (count == 2 && a[2] != boundary) continue;
                        bool fits = true;
                        for (auto [q,result] : observations)
                            if (probes[q].predict(a) != result) { fits = false; break; }
                        if (fits) {
                            plan.candidates.push_back(a);
                            plan.weights.push_back(weight);
                        }
                    } while (next_permutation(a.begin(),a.end()));
                }
            }
            vector<int> ids(plan.candidates.size());
            iota(ids.begin(),ids.end(),0);
            while (ids.size() > 1) {
                int q = plan.choose(ids).second;
                const Probe &p = probes[q];
                int result = p.i == p.j ? readOne(first+p.i)
                                        : io.ask(first+p.i,first+p.j,p.anchor) - io.value[p.anchor];
                observations.emplace_back(q,result);
                erase_if(ids,[&](int id){return p.predict(plan.candidates[id]) != result;});
                if (ids.empty()) {
                    grid.insert(result);
                    grid.insert(r-result);
                    break;
                }
            }
            // For any fixed ordering, observing a coordinate fixes t or u.
            // All feasible extremes of t and u therefore lie in the grid,
            // which contains each observation and its reflection about r/2.
            if (ids.size() == 1) {
                const auto &a = plan.candidates[ids.front()];
                for (int j = 0; j < count; ++j) add(first+j,a[j]);
                return;
            }
        }
    }

    // Boundary scores can cluster at nearby positions. If observed pairs
    // support this, collect a batch near known occurrences of that score.
    int nearbyBatch(int done, int count) {
        if (count < 2) return -1;
        vector<tuple<double,int,int>> choices;
        for (int b : {0,1}) {
            double rate = base[b] / samples;
            for (int d = 1; d <= strides; ++d) {
                if (hits[b][d] < 2) continue;
                double p = (hits[b][d] + 3*rate + 0.5)/(trials[b][d]+4);
                if (p >= 0.72 && p >= max(base[0],base[1])/samples - 0.08)
                    choices.emplace_back(p,b,d);
            }
        }
        sort(choices.rbegin(),choices.rend());
        for (auto [p,b,d] : choices) {
            vector<int> chosen;
            for (int distance = 1; distance <= count && (int)chosen.size() < count; ++distance) {
                for (int j = done; j < io.n && (int)chosen.size() < count; ++j) {
                    int pos = io.label[j];
                    bool close = false;
                    for (int step : {-d,d}) {
                        int k = pos + step*distance;
                        if (k < 1 || k > io.n || at[k] != b*M) continue;
                        bool clear = true;
                        for (int a = pos+step; a != k; a += step)
                            if (at[a] != -1) clear = false;
                        if (clear) close = true;
                    }
                    if (close && find(chosen.begin(),chosen.end(),j) == chosen.end())
                        chosen.push_back(j);
                }
            }
            if ((int)chosen.size() < count) continue;
            // Ascending swaps preserve the indices of later chosen labels.
            sort(chosen.begin(),chosen.end());
            for (int j = 0; j < count; ++j) swap(io.label[done+j],io.label[chosen[j]]);
            return b*M;
        }
        return -1;
    }

    void finish(int done) {
        for (int i = 0; i < done; ++i) add(i,io.value[i]);
        samples = done;
        base[0] = zeros;
        base[1] = tops;
        while (done < io.n) {
            int remain = io.n-done;
            int start = done;
            double popular = max(base[0],base[1])/samples;
            int count = min(3,remain);
            int boundary = nearbyBatch(done,count);
            // Batch when a boundary score occurs often enough that one
            // response is likely to certify an entire fresh triple.
            if (remain >= 2 && (boundary >= 0 || popular >= (remain==2 ? 0.65 : 0.58))) {
                group(done,count,boundary >= 0 ? boundary : (base[0] >= base[1] ? 0 : M));
                done += count;
            } else {
                int x = readOne(done);
                add(done++,x);
            }
            // Only the unselected positions estimate the rate in the
            // remaining shuffled population.
            if (boundary < 0) {
                for (int j = start; j < done; ++j) {
                    base[0] = 0.98*base[0] + (io.value[j] == 0);
                    base[1] = 0.98*base[1] + (io.value[j] == M);
                    samples = 0.98*samples + 1;
                }
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 5 || n > 1000) return 0;
    Interaction io(n);
    int first = io.ask(0,1,2);
    int done = 3;
    if (first == 0 || first == 2*M) {
        fill_n(io.value.begin(),3,first/2);
    } else {
        int second = io.ask(0,3,4);
        done = 5;
        if (n >= 6 && first == second && first % 2 == 0) {
            vector<Triple> lines;
            if (n == 6) {
                done = 6;
                // In these ten triples, every nonzero assignment of signs
                // to the six deviations has a triple containing just one
                // sign of nonzero deviation. Its sum would differ.
                lines = {{2,3,5},{0,1,3},{1,3,5},{0,2,5},
                         {1,2,4},{0,4,5},{0,2,3},{0,1,5}};
            } else {
                // The seven lines of the Fano plane certify seven equal
                // scores. Any line touching a nonzero deviation must have
                // both signs. If every line did, this would 2-color the
                // plane. With a zero line, the other four points must be
                // pairwise opposite, forcing them all to zero.
                done = 7;
                lines = {{0,5,6},{1,3,5},{1,4,6},{2,3,6},{2,4,5}};
            }
            bool same = true;
            for (auto [i,j,k] : lines)
                if (io.ask(i,j,k) != first) { same = false; break; }
            if (same) fill_n(io.value.begin(),done,first/2);
            else SeedSolver(io,done).solve();
        } else SeedSolver(io,done).solve();
    }
    Decoder{io}.finish(done);
    vector<int> answer(n);
    for (int i = 0; i < n; ++i) answer[io.label[i]-1] = io.value[i];
    cout << "!";
    for (int v : answer) cout << ' ' << v;
    cout << endl;
}
