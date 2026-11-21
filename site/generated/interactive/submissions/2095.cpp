#include <bits/stdc++.h>
using namespace std;

class Tournament {
    struct Group {
        int size, root;
    };
    struct Challenge {
        int size, player, parent;
        bool operator<(const Challenge& other) const {
            return tie(size, player) < tie(other.size, other.player);
        }
    };

    mt19937 generator{83057};
    int n, k;
    bool reversed;
    function<int(int,int)> match;
    vector<int> component, seen, stackBuffer;
    vector<vector<int>> above;
    int timestamp = 0;
    vector<vector<Group>> defeated;

    int find(int a) {
        while (component[a] >= 0) {
            if (component[component[a]] >= 0)
                component[a] = component[component[a]];
            a = component[a];
        }
        return a;
    }

    bool reaches(int from, int target) {
        ++timestamp;
        stackBuffer.clear();
        stackBuffer.push_back(from);
        seen[from] = timestamp;
        while (!stackBuffer.empty()) {
            int a = stackBuffer.back();
            stackBuffer.pop_back();
            for (int b : above[a]) {
                if (b == target) return true;
                if (seen[b] != timestamp) {
                    seen[b] = timestamp;
                    stackBuffer.push_back(b);
                }
            }
        }
        return false;
    }

    // In the complementary problem, "better" means weaker in the contest.
    int better(int a, int b) {
        if (a == b) return a;
        int ca = find(a), cb = find(b);
        if (ca == cb) {
            if (reaches(b, a)) return a;
            if (reaches(a, b)) return b;
        }
        int w = match(a, b);
        if (reversed) w = w == a ? b : a;
        above[w == a ? b : a].push_back(w);
        if (ca != cb) {
            if (component[ca] > component[cb]) swap(ca, cb);
            component[ca] += component[cb];
            component[cb] = ca;
        }
        return w;
    }

    Group mergeGroups(Group a, Group b) {
        if (better(a.root, b.root) == b.root) swap(a, b);
        defeated[a.root].push_back(b);
        return {a.size + b.size, a.root};
    }

    vector<Group> makeGroups(vector<Group> groups, int count) {
        // Stable tie order retains the input shuffle at each equal size.
        using Entry = tuple<int,int,int>;
        priority_queue<Entry,vector<Entry>,greater<Entry>> q;
        int serial=0;
        for (Group g : groups) q.emplace(g.size,serial++,g.root);
        auto take = [&]() {
            auto [size,order,root]=q.top();
            q.pop();
            return Group{size,root};
        };
        while ((int)q.size() > count) {
            Group a=take(), b=take();
            Group g=mergeGroups(a,b);
            q.emplace(g.size,serial++,g.root);
        }
        vector<Group> result;
        while (!q.empty()) {
            result.push_back(take());
        }
        return result;
    }

    // This pool has at most 64 players. Rank bounds certify both membership
    // and exclusion. Pivots partition dense ranks, while small extreme ranks
    // only require matches among the remaining possible extrema.
    vector<int> smallSelect(vector<int> players, int wanted) {
        using Mask = uint64_t;
        int count = players.size();
        vector<Mask> upper(count), lower(count);
        for (int i = 0; i < count; ++i) {
            for (int j = 0; j < i; ++j) {
                if (find(players[i]) != find(players[j])) continue;
                if (reaches(players[i], players[j])) {
                    upper[i] |= Mask(1) << j;
                    lower[j] |= Mask(1) << i;
                } else if (reaches(players[j], players[i])) {
                    upper[j] |= Mask(1) << i;
                    lower[i] |= Mask(1) << j;
                }
            }
        }
        auto compare = [&](int a, int b) {
            if (lower[a] >> b & 1) return a;
            if (upper[a] >> b & 1) return b;
            if (better(players[a],players[b]) == players[b]) swap(a,b);
            Mask high = upper[a] | (Mask(1) << a);
            Mask low = lower[b] | (Mask(1) << b);
            for (Mask x = high; x; x &= x-1) lower[countr_zero(x)] |= low;
            for (Mask x = low; x; x &= x-1) upper[countr_zero(x)] |= high;
            return a;
        };

        Mask active = ~Mask(0) >> (64-count);
        vector<int> answer;
        mt19937 gen(34875 + count);
        while (active) {
            int size = popcount(active);
            Mask keep = 0, drop = 0;
            for (Mask x = active; x; x &= x-1) {
                int i = countr_zero(x);
                if (popcount(upper[i]&active) >= wanted) drop |= Mask(1)<<i;
                if (popcount(lower[i]&active) >= size-wanted) keep |= Mask(1)<<i;
            }
            if (keep || drop) {
                for (Mask x=keep;x;x&=x-1) answer.push_back(players[countr_zero(x)]);
                wanted -= popcount(keep);
                active &= ~(keep|drop);
                continue;
            }
            if (min(wanted,size-wanted) <= max(3,size/5)) {
                bool bottom = wanted > size-wanted;
                int a=-1,b=-1,first=INT_MAX,second=INT_MAX;
                for(Mask x=active;x;x&=x-1) {
                    int i=countr_zero(x);
                    if ((bottom?lower[i]:upper[i]) & active) continue;
                    int weight = popcount((bottom?upper[i]:lower[i]) & active);
                    if (weight < first) {
                        b=a;second=first;a=i;first=weight;
                    } else if (weight < second) {
                        b=i;second=weight;
                    }
                }
                compare(a,b);
                continue;
            }
            vector<int> sample;
            for(Mask x=active;x;x&=x-1)sample.push_back(countr_zero(x));
            shuffle(sample.begin(),sample.end(),gen);
            int s=min(size, size>=21 ? 9 : 5);
            sample.resize(s);
            int target=clamp((int)round((wanted+0.5)*(s+1)/(size+1.)-1),0,s-1);
            nth_element(sample.begin(),sample.begin()+target,sample.end(),
                [&](int a,int b){return a!=b && compare(a,b)==a;});
            int pivot=sample[target];
            while(true) {
                Mask unknown = active & ~(upper[pivot] | lower[pivot] | (Mask(1)<<pivot));
                if (!unknown) break;
                int best=-1;
                double bestGain=-1;
                for(Mask x=unknown;x;x&=x-1) {
                    int i=countr_zero(x);
                    int a=popcount(upper[i]&unknown),b=popcount(lower[i]&unknown);
                    double prob = (b+1.)/(a+b+2.);
                    double gain = a*prob + b*(1-prob);
                    if(gain>bestGain) {bestGain=gain;best=i;}
                }
                compare(pivot,best);
            }
        }
        return answer;
    }

    // Take prefixes of sorted lists. If the allocation is wrong, move a
    // whole prefix from the list with the best omitted player into the set.
    vector<int> fromRuns(const vector<vector<int>>& runs, int wanted) {
        vector<int> used(runs.size());
        for (int left = wanted; left > 0;) {
            int best = -1;
            for (int j = 0; j < (int)runs.size(); ++j) {
                if (used[j] == (int)runs[j].size()) continue;
                if (best < 0 || better(runs[j][used[j]], runs[best][used[best]]) == runs[j][used[j]])
                    best = j;
            }
            int count = min(left, (int)runs[best].size() - used[best]);
            used[best] += count;
            left -= count;
        }
        for (;;) {
            int in = -1, out = -1;
            for (int j = 0; j < (int)runs.size(); ++j) {
                if (used[j] && (in < 0 || better(runs[j][used[j]-1], runs[in][used[in]-1]) == runs[in][used[in]-1]))
                    in = j;
                if (used[j] < (int)runs[j].size() &&
                    (out < 0 || better(runs[j][used[j]], runs[out][used[out]]) == runs[j][used[j]]))
                    out = j;
            }
            if (out < 0 || in == out) break;
            auto improves = [&](int d) {
                int a = runs[in][used[in]-d], b = runs[out][used[out]+d-1];
                return better(a,b) == b;
            };
            if (!improves(1)) break;
            int low = 1, high = min(used[in], (int)runs[out].size()-used[out]);
            if (low < high && improves(high)) low = high;
            while (low < high) {
                int mid = (low+high+1)/2;
                if (improves(mid)) low = mid;
                else high = mid-1;
            }
            used[in] -= low;
            used[out] += low;
        }
        vector<int> result;
        for (int j = 0; j < (int)runs.size(); ++j)
            result.insert(result.end(), runs[j].begin(), runs[j].begin()+used[j]);
        return result;
    }

    // Read disjoint monotone runs, keeping a proof of each adjacent order.
    vector<vector<int>> readRuns(const vector<int>& items, int limit, int& pos) {
        vector<vector<int>> runs;
        pos = 0;
        while (pos < (int)items.size() && (int)runs.size() < limit) {
            vector<int> run{items[pos++]};
            if (pos < (int)items.size()) {
                bool descending = better(run.back(), items[pos]) == run.back();
                run.push_back(items[pos++]);
                while (pos < (int)items.size() &&
                       (better(run.back(), items[pos]) == run.back()) == descending)
                    run.push_back(items[pos++]);
                if (!descending) reverse(run.begin(),run.end());
            }
            runs.push_back(move(run));
        }
        return runs;
    }

    vector<int> select() {
        if (k == 0) return {};
        if (k == 1) {
            int best = 1;
            for (int a = 2; a <= n; ++a) best = better(best, a);
            return {best};
        }

        vector<Group> groups;
        vector<int> mate(n+1);
        array<int,4> cut{0,k,n-k,n};
        int right = 0, pairs = 0, changes = 0, last = -1;
        for (int j = 0; j < 3; ++j) {
            int a = cut[j]+1;
            for (; a+1 <= cut[j+1]; a+=2) {
                Group g = mergeGroups({1,a},{1,a+1});
                groups.push_back(g);
                mate[g.root] = a + (a+1) - g.root;
                int direction = g.root != a;
                right += direction;
                changes += last >= 0 && last != direction;
                last = direction;
                ++pairs;
                if (n <= 64 && k >= 8 && 4*k >= n && changes >= 2) {
                    vector<int> all(n);
                    iota(all.begin(),all.end(),1);
                    return smallSelect(all,k);
                }
            }
            if (a <= cut[j+1]) groups.push_back({1,a});
        }

        bool directional = min(right,pairs-right) == 0 ||
                           (pairs >= 8 && 5*min(right,pairs-right) <= pairs);
        if (directional) {
            int low = 2*right > pairs ? n-k+1 : 1;
            auto inside = [&](int a) { return low <= a && a < low+k; };
            vector<int> seed, minima;
            for (Group g : groups) {
                if (!inside(g.root)) continue;
                seed.push_back(g.root);
                if (mate[g.root]) seed.push_back(mate[g.root]);
                minima.push_back(mate[g.root] ? mate[g.root] : g.root);
            }
            auto weakest = [&](const vector<int>& candidates) {
                int worst = candidates.front();
                for (size_t i = 1; i < candidates.size(); ++i)
                    if (better(worst, candidates[i]) == worst) worst = candidates[i];
                return worst;
            };
            int cutoff = weakest(minima);
            if (k == 2) {
                vector<Group> outside;
                for (Group g : groups) if (!inside(g.root)) outside.push_back(g);
                int best = makeGroups(move(outside), 1).front().root;
                if (better(cutoff, best) == cutoff) return seed;
                int other = seed.front();
                if (better(other, best) == best) {
                    for (Group g : defeated[best]) other = better(other, g.root);
                }
                return {best, other};
            }

            // Screen from the nearby end. On a noisy monotone sequence one
            // outlier in the seed can depress its cutoff. A batch of nearby
            // challengers can supply a stronger, equally certified cutoff.
            if (low > 1) reverse(groups.begin(), groups.end());
            const int minor = min(right, pairs-right);
            bool retry = minor >= 2 && 20*minor <= pairs && n > 8*k;
            const int limit = max(2, k/4);
            vector<Group> remaining;
            vector<int> pool = seed;
            int hits = 0;
            for (Group g : groups) {
                if (!inside(g.root) && hits <= limit) {
                    if (better(cutoff, g.root) == cutoff) continue;
                    pool.push_back(g.root);
                    if (mate[g.root]) pool.push_back(mate[g.root]);
                    ++hits;
                    if (hits > limit && retry) {
                        retry = false;
                        pool = smallSelect(pool, k);
                        cutoff = weakest(pool);
                        hits = 0;
                    }
                }
                remaining.push_back(g);
            }
            if (hits <= limit) {
                if (!hits) return pool;
                return smallSelect(pool, k);
            }
            groups.swap(remaining);
            if (low > 1) reverse(groups.begin(), groups.end());
        }

        // Pair winners inherit the local order on many structured inputs.
        // Their own partners need examination only for the best k winners.
        if (directional || (pairs >= 8 && changes <= max(2,pairs/16))) {
            vector<int> heads;
            for (Group g : groups) heads.push_back(g.root);
            int pos;
            auto runs = readRuns(heads,max(4,k/2),pos);
            if (pos == (int)heads.size()) {
                vector<int> best = fromRuns(runs,min(k,pos));
                vector<bool> chosen(n+1);
                for (int a : best) chosen[a] = true;
                for (auto& run : runs) {
                    while (!run.empty() && !chosen[run.back()]) run.pop_back();
                }
                vector<int> partners;
                for (int a : heads) if (chosen[a] && mate[a]) partners.push_back(mate[a]);
                auto extra = readRuns(partners,k,pos);
                for (auto& run : extra) runs.push_back(move(run));
                return fromRuns(runs,k);
            }
            // Keep the completed chains as max trees for the general case.
            vector<Group> packed(groups.begin()+pos,groups.end());
            for (const auto& run : runs) {
                Group tail{0,0};
                for (auto it=run.rbegin(); it!=run.rend(); ++it) {
                    if (tail.size) defeated[*it].push_back(tail);
                    tail = {tail.size+1+(mate[*it]!=0),*it};
                }
                packed.push_back(tail);
            }
            groups.swap(packed);
        }

        return fromForest(move(groups));
    }

    vector<int> fromForest(vector<Group> groups) {
        // Spread correlated labels between the knockout groups.
        shuffle(groups.begin(),groups.end(),generator);
        int target = k >= 12 ? 2*k : k;
        groups = makeGroups(move(groups),target);
        while ((int)groups.size() < k) {
            auto it = max_element(groups.begin(),groups.end(),[](Group a, Group b){
                return a.size < b.size;
            });
            Group child = defeated[it->root].back();
            defeated[it->root].pop_back();
            it->size -= child.size;
            groups.push_back(child);
        }
        vector<int> initial;
        for (Group g : groups) initial.push_back(g.root);
        if ((int)initial.size() > k) initial = smallSelect(initial,k);
        priority_queue<Challenge> pending;
        vector<int> value = initial, slot(n+1, -1);
        for (int i = 0; i < k; ++i) slot[value[i]] = i;
        // Only a player with no known weaker incumbent can be the cutoff.
        auto findCutoff = [&]() {
            vector<unsigned> higher(k);
            unsigned eligible = (1u << k) - 1;
            for (int i = 0; i < k; ++i) {
                ++timestamp;
                stackBuffer = {value[i]};
                seen[value[i]] = timestamp;
                while (!stackBuffer.empty()) {
                    int a = stackBuffer.back();
                    stackBuffer.pop_back();
                    for (int b : above[a]) {
                        if (seen[b] == timestamp) continue;
                        seen[b] = timestamp;
                        stackBuffer.push_back(b);
                        if (slot[b] >= 0) higher[i] |= 1u << slot[b];
                    }
                }
                eligible &= ~higher[i];
            }
            using Entry = pair<int,int>;
            priority_queue<Entry,vector<Entry>,greater<Entry>> q;
            for (unsigned mask = eligible; mask; mask &= mask-1) {
                int i = countr_zero(mask);
                q.emplace(popcount(higher[i]), i);
            }
            while (q.size() > 1) {
                int a=q.top().second; q.pop();
                int b=q.top().second; q.pop();
                if(better(value[a],value[b])==value[a])swap(a,b);
                higher[a] |= higher[b] | (1u << b);
                q.emplace(popcount(higher[a]),a);
            }
            return q.top().second;
        };
        // Each pending root bounds its entire subtree. The cutoff only
        // improves, so pruning a root also permanently prunes its children.
        auto expose = [&](int a) {
            for (auto g : defeated[a])
                pending.push({g.size, g.root, a});
        };
        for (int a : initial) expose(a);
        int pos = -1;
        while (!pending.empty()) {
            Challenge c = pending.top();
            pending.pop();
            if (slot[c.parent] < 0) continue;
            if (pos < 0) pos = findCutoff();
            int cutoff = value[pos];
            if (c.parent == cutoff || better(cutoff, c.player) == cutoff) continue;
            slot[cutoff] = -1;
            slot[c.player] = pos;
            value[pos] = c.player;
            pos = -1;
            expose(c.player);
        }
        return value;
    }

public:
    Tournament(int size, int take, function<int(int,int)> ask)
        : n(size), k(min(take, size-take)), reversed(take > size-take),
          match(move(ask)), component(n+1, -1), seen(n+1), above(n+1),
          defeated(n+1) {}

    vector<int> solve() {
        vector<int> result = select();
        if (!reversed) return result;
        vector<bool> excluded(n+1);
        for (int a : result) excluded[a] = true;
        result.clear();
        for (int a = 1; a <= n; ++a)
            if (!excluded[a]) result.push_back(a);
        return result;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, k;
    if (!(cin >> n >> k)) return 0;
    auto ask = [](int a, int b) {
        cout << "? " << a << ' ' << b << endl;
        int w;
        if (!(cin >> w) || (w != a && w != b)) exit(0);
        return w;
    };
    vector<int> result = Tournament(n, k, ask).solve();
    cout << '!';
    for (int a : result) cout << ' ' << a;
    cout << endl;
    return 0;
}
