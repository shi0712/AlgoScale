#include <bits/stdc++.h>
using namespace std;

constexpr int M = 501;
using Set = bitset<M>;
int first(const Set& s) { return int(s._Find_first()); }
int next(const Set& s, int i) { return int(s._Find_next(i)); }

struct Clause {
    int subject;
    Set choices;
};

struct Classifier {
    int n, value = 0, moves = 0, edgesRemoved = 0, classes = 0;
    array<int, M> parent{};
    array<Set, M> possible;
    Set live, palette, lastPalette, dirty;
    bool lastComplete = false;
    deque<int> belt;
    vector<Clause> clauses;
    mt19937 rng{725771};

    explicit Classifier(int size) : n(size) {
        for (int i = 0; i < n; ++i) {
            live.set(i);
            parent[i] = i;
        }
        for (int i = 0; i < n; ++i) possible[i] = live;
    }

    int root(int x) {
        return parent[x] == x ? x : parent[x] = root(parent[x]);
    }

    int readValue() {
        int v;
        if (!(cin >> v) || v < 0 || v > n) exit(0);
        ++moves;
        return v;
    }

    bool push(int x) {
        cout << "+ " << x + 1 << endl;
        int v = readValue();
        bool present = v == value;
        belt.push_back(x);
        value = v;
        return present;
    }

    bool pop() {
        cout << "-" << endl;
        int v = readValue();
        bool present = v == value;
        belt.pop_front();
        value = v;
        return present;
    }

    Set onBelt(bool skipFront = false) {
        Set s;
        for (size_t i = skipFront; i < belt.size(); ++i) s.set(root(belt[i]));
        return s;
    }

    // Every deleted edge is a proved inequality between two classes.
    void exclude(int x, Set removed) {
        removed &= possible[x];
        if (removed.none()) return;
        possible[x] &= ~removed;
        edgesRemoved += removed.count();
        if (palette[x] || lastPalette[x]) dirty |= removed;
        if ((removed & (palette | lastPalette)).any()) dirty.set(x);
        for (int y = first(removed); y < M; y = next(removed,y))
            possible[y].reset(x);
    }

    void unite(int x, int y) {
        if (x == y) return;
        if (palette[y] || (!palette[x] && lastPalette[y])) swap(x,y);
        Set common = possible[x] & possible[y];
        exclude(x, possible[x] & ~common);
        for (int z = first(possible[y]); z < M; z = next(possible[y],z))
            possible[z].reset(y);
        possible[y].reset();
        parent[y] = x;
        live.reset(y);
        if (palette[y]) palette.set(x);
        if (lastPalette[y]) lastPalette.set(x);
        palette.reset(y);
        lastPalette.reset(y);
        for (Clause& c : clauses) {
            if (c.subject == y) c.subject = x;
            if (c.choices[y]) {
                c.choices.reset(y);
                c.choices.set(x);
            }
        }
        dirty = live;
        ++edgesRemoved;
    }

    // Each complete palette has exactly one member of every class.
    void propagate() {
        if (!classes) return;
        while (dirty.any()) {
            int x = first(dirty);
            dirty.reset(x);
            for (int k = 0; k < (classes != 0) + lastComplete; ++k) {
                const Set& p = k ? lastPalette : palette;
                if (p[x]) continue;
                Set d = p & possible[x];
                size_t count = d.count();
                if (count == 1) {
                    unite(x, first(d));
                    break;
                }
                if (count <= 18) {
                    Set support;
                    for (int y = first(d); y < M; y = next(d,y))
                        support |= possible[y];
                    exclude(x, ~support);
                }
            }
        }
    }

    // A successful membership test leaves a disjunction of equalities.
    // Retire the disjunction once a complete palette already implies it.
    bool simplify(Clause& c) {
        int x = c.subject;
        c.choices &= possible[x];
        if (c.choices[x]) return true;
        if (c.choices.count() == 1) {
            unite(x, first(c.choices));
            return true;
        }
        for (int k = 0; k < (classes != 0) + lastComplete; ++k) {
            const Set& p = k ? lastPalette : palette;
            Set outside = (possible[x] & p) & ~c.choices;
            if (outside.none()) return true;
            if ((c.choices & ~p).none()) {
                exclude(x, outside);
                return true;
            }
        }
        Set support;
        for (int y = first(c.choices); y < M; y = next(c.choices,y))
            support |= possible[y];
        exclude(x, ~support);
        return false;
    }

    void review() {
        for (size_t i = 0; i < clauses.size();) {
            if (simplify(clauses[i])) {
                clauses[i] = clauses.back();
                clauses.pop_back();
            } else ++i;
        }
        propagate();
    }

    void observe(int x, const Set& others, bool hit) {
        if (!hit) exclude(x, others);
        else {
            Clause c{x, others & possible[x]};
            if (!simplify(c)) clauses.push_back(c);
        }
        propagate();
    }

    bool finished() const { return live == palette; }

    vector<vector<int>> localGroups;

    Set canonical(Set s) {
        Set r;
        for (int x = first(s); x < M; x = next(s,x)) r.set(root(x));
        return r;
    }

    void clique(const Set& group) {
        for (int x = first(group); x < M; x = next(group,x)) {
            Set rest = group;
            rest.reset(x);
            exclude(x,rest);
        }
        localGroups.emplace_back();
        for (int x = first(group); x < M; x = next(group,x))
            localGroups.back().push_back(x);
    }

    // A distinct group requires an injective assignment to the global
    // representatives. Test support by alternating paths in a matching.
    void filterGroup(Set group) {
        Set rows = group & ~palette;
        if (rows.count() < 2) return;
        array<int,M> mate, assigned;
        mate.fill(-1);
        array<Set,M> domain;
        Set used;
        for (int x = first(rows); x < M; x = next(rows,x))
            domain[x] = possible[x] & palette;
        auto augment = [&](auto& self,int x,Set& visited)->bool {
            Set todo = domain[x] & ~visited;
            for (int y = first(todo); y < M; y = next(todo,y)) {
                if (visited[y]) continue;
                visited.set(y);
                if (mate[y]<0 || self(self,mate[y],visited)) {
                    mate[y]=x;
                    assigned[x]=y;
                    return true;
                }
            }
            return false;
        };
        for (int x=first(rows);x<M;x=next(rows,x)) {
            Set unused = domain[x] & ~used;
            if (unused.any()) {
                int y=first(unused);
                mate[y]=x;
                assigned[x]=y;
                used.set(y);
            } else {
                Set seen;
                if (!augment(augment,x,seen)) return;
                used.reset();
                for (int y=first(palette);y<M;y=next(palette,y))
                    if (mate[y]>=0) used.set(y);
            }
        }
        // Contract each matched edge. A reassignment is feasible if it
        // reaches a free color or closes an alternating cycle.
        Set freeColors = palette & ~used;
        array<Set,M> reach;
        for (int y=first(used);y<M;y=next(used,y)) reach[y]=domain[mate[y]];
        for (int k=first(used);k<M;k=next(used,k))
            for (int y=first(used);y<M;y=next(used,y))
                if (reach[y][k]) reach[y] |= reach[k];
        Set fixed;
        for (int y=first(used);y<M;y=next(used,y))
            if ((reach[y]&freeColors).none()) fixed.set(y);
        for (int x=first(rows);x<M;x=next(rows,x)) {
            Set check=domain[x]&fixed, remove;
            for (int y=first(check);y<M;y=next(check,y))
                if (!reach[y][assigned[x]]) remove.set(y);
            exclude(x,remove);
        }
    }

    void strongReview() {
        review();
        vector<pair<int,int>> equalities;
        for (const auto& g : localGroups) {
            Set group;
            for (int u : g) group.set(root(u));
            if (group.count() == size_t(classes)) {
                for (int x=first(live & ~group);x<M;x=next(live,x)) {
                    if (group[x]) continue;
                    Set d=possible[x]&group;
                    if (d.count()==1) equalities.emplace_back(x,first(d));
                }
            }
            filterGroup(group);
        }
        for (auto [x,y] : equalities) unite(root(x),root(y));
        propagate();
    }

    // With repeated oldest-to-newest matches, keep the current window width.
    // Even after the pattern breaks, every new class is recorded on its first
    // insertion. Positive replies give ordinary membership constraints.
    Set rollingScan(int i, Set& introduced) {
        for (; i<n; ++i) {
            Set prior=onBelt();
            bool distinct = int(belt.size())==value;
            bool hit=push(i);
            if (!hit) introduced.set(i);
            observe(i,prior,hit);
            int head=root(belt.front());
            bool stayed=pop();
            if (distinct && hit && stayed) unite(root(head),root(i));
            else observe(head,onBelt(),stayed);
            if (int(belt.size())==value && !distinct) {
                Set recovered=onBelt();
                for (int x=first(recovered);x<M;x=next(recovered,x)) {
                    Set different=recovered;
                    different.reset(x);
                    exclude(x,different);
                }
            }
        }
        Set dropped;
        while (int(onBelt().count())!=value) {
            int x=root(belt.front());
            bool hit=pop();
            if (!hit) dropped.set(x);
            observe(x,onBelt(),hit);
        }
        dropped=canonical(dropped);
        for (int x=first(dropped);x<M;x=next(dropped,x)) push(x);
        Set complete=onBelt();
        clique(complete);
        return complete;
    }

    void initialize() {
        Set introduced, localFirst;
        int i = 0, rotations = 0;
        bool cycling = true, globalScan = false;
        while (true) {
            Set block = onBelt();
            localFirst = block;

            while (i < n) {
                int x = i++;
                bool hit = push(x);
                if (!hit) {
                    rotations = 0;
                    exclude(x,block);
                    localFirst.set(x);
                    introduced.set(x);
                } else {
                    Set candidates = possible[x] & block;
                    if (candidates.count()==1) unite(first(candidates),x);
                }
                block.set(root(x));
                // In a long distinct prefix, first test a possible repetition
                // of its oldest class. A failed prediction costs just one
                // restoration and leaves every global class represented.
                if (hit && localGroups.empty() && !globalScan &&
                        value >= max(8,int(sqrt(n))) && int(belt.size()) == value+1
                        && block.count() == belt.size() && i<n) {
                    int head = root(belt.front());
                    if (pop()) {
                        unite(head,x);
                        block = onBelt();
                        if (++rotations >= 3) {
                            localFirst = rollingScan(i,introduced);
                            i = n;
                            break;
                        }
                        continue;
                    }
                    exclude(head,onBelt());
                    push(head);
                    globalScan = true;
                }
                if (!globalScan && i < n && value >= 2 && int(block.count())-value >= 
                        (cycling ? 1 : max(1,value/3)) && (cycling || n-i > value/2)) break;
            }
            if (i == n) break;

            int beforeDrain = belt.size();
            Set localLast;
            while (true) {
                Set remaining = onBelt();
                if (int(remaining.count()) == value) {
                    localLast |= remaining;
                    break;
                }
                int x = root(belt.front());
                bool hit = pop();
                Set others = onBelt();
                if (!hit) {
                    exclude(x,others);
                    localLast.set(x);
                } else {
                    Set candidates = possible[x] & others;
                    if (candidates.count()==1) unite(x,first(candidates));
                }
            }
            cycling = beforeDrain - int(belt.size()) == 1;
            localFirst = canonical(localFirst);
            localLast = canonical(localLast);
            clique(localFirst);
            clique(localLast);
            for (int x = first(block); x < M; x = next(block,x)) {
                int y = root(x);
                if (!localFirst[y]) clauses.push_back({y,localFirst});
                if (!localLast[y]) clauses.push_back({y,localLast});
            }
            // If several entire runs have stayed together, a long window
            // preserves the positional exclusions between successive runs.
            if (!globalScan && i >= 6 && i < n/2) {
                int runs = 1, length = 1;
                bool repeated = true;
                for (int j = 1; j < i; ++j) {
                    if (root(j) == root(j-1)) ++length;
                    else {
                        repeated &= length >= 2;
                        length = 1;
                        ++runs;
                    }
                }
                globalScan = repeated && length >= 2 && runs >= 3;
            }
        }

        // Every globally new class was new in the window in which it arrived.
        // Test any such representatives still outside the final window.
        for (int pass=0;pass<8;++pass) {
            int previous=edgesRemoved;
            review();
            if (previous == edgesRemoved) break;
        }
        Set current = onBelt(), covered = current;
        palette = canonical(localFirst);
        Set candidates = canonical(introduced) & ~current;
        while (true) {
            size_t oldSize;
            do {
                oldSize = covered.count();
                for (const Clause& c : clauses) {
                    if (((c.choices & possible[c.subject]) & ~covered).none())
                        covered.set(c.subject);
                }
            } while (covered.count() != oldSize);
            candidates &= ~covered;
            if (candidates.none()) break;
            int x = first(candidates);
            if (!push(x)) {
                exclude(x,covered);
                palette.set(x);
            } else {
                clauses.push_back({x,current});
            }
            covered.set(x);
            current.set(x);
        }
        classes = value;
        dirty = live;
        propagate();
        review();
        while (!finished()) {
            Set all = onBelt();
            if ((int)all.count() == value || value == 1) {
                if (value == 1) {
                    int x = first(all);
                    for (int y = next(all,x); y < M; y = next(all,y))
                        unite(root(x),root(y));
                    all = onBelt();
                }
                lastPalette |= all;
                break;
            }
            int x = root(belt.front());
            bool hit = pop();
            Set others = onBelt();
            if (!hit) lastPalette.set(x);
            observe(x,others,hit);
        }
        if (finished()) return;
        lastComplete = true;
        clique(lastPalette);
        dirty = live;
        propagate();
        review();
    }
    void queryPush(int x) {
        Set others = onBelt();
        bool hit = push(x);
        observe(x,others,hit);
    }
    void queryPop() {
        int x = root(belt.front());
        bool hit = pop();
        observe(x,onBelt(),hit);
    }

    // Resolve one unknown directly if adaptive queries have stalled.
    void direct() {
        while (!belt.empty() && !finished()) queryPop();
        if (finished()) return;
        int x = first(live & ~palette);
        Set choices = possible[x] & palette;
        int needed = choices.count() / 2;
        for (int y = first(choices); needed > 0; y = next(choices,y), --needed)
            queryPush(y);
        if (!palette[root(x)]) queryPush(root(x));
    }

    struct Estimate {
        double gain, presence;
        int partners;
    };

    Estimate estimate(int x, Set members) {
        Set d = possible[x];
        members &= d;
        if (members.none()) return {0,0,0};
        if (members[x]) return {0,1,0};
        Set a = d & palette, b = d & lastPalette;
        int na = a.count(), nb = b.count();
        int ha = (a & members).count(), hb = (b & members).count();
        if (ha == na || hb == nb) return {0,1,0};
        // Members in both palettes represent known classes, so count them once.
        int both = (members & a & b).count();
        int extra = (members & ~(a | b)).count();
        double miss = (1.0 - double(ha)/na);
        if (nb > both) miss *= 1.0 - double(hb-both)/(nb-both);
        if (extra) {
            double inv = 1.0 / max(2.0, sqrt(double(na)*nb));
            if (palette[x]) inv = 1.0 / max(2,nb);
            if (lastPalette[x]) inv = 1.0 / max(2,na);
            miss *= pow(1.0-inv,extra);
        }
        double score = 4*miss*(1-miss);
        if (extra || (ha && hb > both))
            score *= 0.87;
        return {score,1-miss,int(members.count())};
    }

    void adaptive() {
        int prevEdges = edgesRemoved, stalled = 0, sinceDirect = 0;
        while (!finished()) {
            if (moves % 41 == 0) strongReview();
            else if (moves % 9 == 0) review();
            if (finished()) break;
            if (prevEdges == edgesRemoved) ++stalled;
            else stalled = 0;
            prevEdges = edgesRemoved;
            if (stalled > 20 + int(live.count())/2 || ++sinceDirect > 8*n+50) {
                direct();
                review();
                stalled = sinceDirect = 0;
                continue;
            }

            Set active = onBelt(), available = live & ~active;
            int front = belt.empty() ? -1 : root(belt.front());
            Estimate head{0,0,0};
            if (front >= 0) head = estimate(front,onBelt(true));
            double popScore = head.gain;
            if (front >= 0) {
                // Advancing the FIFO exposes another possible test.
                popScore += 0.12 + 0.12*min(1.0,double(belt.size())/classes);
                if (head.presence > 0.97) popScore += 0.12;
            } else popScore = -1;

            int best = -1;
            double bestScore = -2;
            for (int x = first(available); x < M; x = next(available,x)) {
                Estimate e = estimate(x,active);
                double score = e.gain;
                int degree = possible[x].count()-1;
                if (!degree) continue;
                // Adding a possible mate can make the pending removal informative.
                if (front >= 0 && possible[front][x] && head.presence < 0.48)
                    score += 0.43 / (1.0 + 0.25*head.partners);
                score += 0.05*min(1.0, degree / double(classes));
                score += 0.035*(rng() / double(mt19937::max()));
                if (score > bestScore) bestScore = score, best = x;
            }
            if (best >= 0 && bestScore > popScore) queryPush(best);
            else if (front >= 0) queryPop();
            else direct();
        }
    }

    // Partition remaining samples into sets of mutually different classes.
    // In a short sequence, all the first occurrences form one such set and
    // all the last occurrences form another. Interior samples can be batched again.
    vector<vector<int>> cohorts() {
        vector<vector<int>> out;
        vector<int> initial;
        for (int x = first(lastPalette & ~palette); x < M; x = next(lastPalette,x))
            if (!palette[x]) initial.push_back(x);
        if (!initial.empty()) out.push_back(initial);
        Set todo = live & ~(palette | lastPalette);
        for (const auto& g : localGroups) {
            vector<int> row;
            for (int u : g) {
                int x = root(u);
                if (todo[x]) {
                    todo.reset(x);
                    row.push_back(x);
                }
            }
            if (!row.empty()) out.push_back(move(row));
        }
        while (todo.any()) {
            vector<int> list;
            for (int x = first(todo); x < M; x = next(todo,x)) list.push_back(x);
            todo.reset();
            for (int i = 0; i < (int)list.size();) {
                vector<int> add, rem, block;
                int limit = min((int)list.size()-i, max(4,2*classes));
                for (int j = 0; j < limit; ++j) {
                    int x = root(list[i++]);
                    if (palette[x]) continue;
                    Set before = onBelt();
                    bool hit = push(x);
                    if (!hit) add.push_back(x);
                    observe(x,before,hit);
                    if (finished()) return out;
                    block.push_back(x);
                }
                while (!belt.empty()) {
                    int x = root(belt.front());
                    bool hit = pop();
                    if (!hit) rem.push_back(x);
                    observe(x,onBelt(),hit);
                    if (finished()) return out;
                }
                Set lhs, rhs;
                for (int x : add) lhs.set(root(x));
                for (int x : rem) rhs.set(root(x));
                Set leftOnly = lhs & ~rhs, rightOnly = rhs & ~lhs;
                Set both = lhs & rhs;
                for (int x = first(both); x < M; x = next(both,x)) {
                    (leftOnly.count() < rightOnly.count() ? leftOnly : rightOnly).set(x);
                }
                for (Set s : {leftOnly,rightOnly}) {
                    s &= ~palette;
                    vector<int> v;
                    for (int x = first(s); x < M; x = next(s,x)) v.push_back(x);
                    if (!v.empty()) out.push_back(move(v));
                }
                for (int y : block) {
                    int x = root(y);
                    if (!lhs[x] && !rhs[x] && !palette[x]) todo.set(x);
                }
            }
            Set clean;
            for (int x = first(todo); x < M; x = next(todo,x))
                if (!palette[root(x)]) clean.set(root(x));
            todo = clean;
        }
        return out;
    }

    // Interleave the removal of one known-distinct list and the insertion of
    // another. A pair is tested at both ends precisely when its intervals
    // on the belt overlap. Choose cuts which balance those binary outcomes.
    void transition(const vector<int>& incoming, bool colorsIncoming) {
        vector<int> old;
        for (int u : belt) old.push_back(root(u));
        int m = old.size(), t = incoming.size();
        vector<vector<double>> weight(t,vector<double>(m));
        array<int,M> degree{};
        for (int x = first(live); x < M; x = next(live,x))
            degree[x] = (possible[x] & palette).count();
        for (int j = 0; j < t; ++j) for (int i = 0; i < m; ++i) {
            int b = colorsIncoming ? old[i] : incoming[j];
            int c = colorsIncoming ? incoming[j] : old[i];
            if (possible[b][c]) weight[j][i] = 1.0/degree[b];
        }
        if (colorsIncoming) {
            for (auto& w : weight) {
                double s = accumulate(w.begin(),w.end(),0.0);
                if (s > 1) for (double& v : w) v /= s;
            }
        } else {
            for (int i = 0; i < m; ++i) {
                double s = 0;
                for (auto& w : weight) s += w[i];
                if (s > 1) for (auto& w : weight) w[i] /= s;
            }
        }
        auto entropy = [](double p) {
            static const auto table = [] {
                array<double,1025> a{};
                for (int k=1;k<1024;++k) {
                    double q=k/1024.0;
                    a[k] = -q*log2(q)-(1-q)*log2(1-q);
                }
                return a;
            }();
            return table[clamp(int(p*1024+0.5),0,1024)];
        };
        vector<int> cut(t,m), order(t);
        iota(order.begin(),order.end(),0);
        vector<double> mass(m);
        for (int iter=0;iter<4;++iter) {
            shuffle(order.begin(),order.end(),rng);
            for (int j : order) {
                const auto& w = weight[j];
                for (int i=cut[j];i<m;++i) mass[i]-=w[i];
                double rem = 0, delta = 0, best = 0;
                cut[j]=m;
                for (int i=m-1;i>=0;--i) if (w[i]>0) {
                    rem+=w[i];
                    delta+=entropy(mass[i]+w[i])-entropy(mass[i]);
                    double score=entropy(rem)+delta;
                    if (score>best+1e-8) {
                        best=score;
                        cut[j]=i;
                    }
                }
                for (int i=cut[j];i<m;++i) mass[i]+=w[i];
            }
        }
        vector<vector<int>> at(m+1);
        for (int j : order) at[cut[j]].push_back(incoming[j]);
        for (int i=0; i<=m && !finished(); ++i) {
            for (int u : at[i]) {
                int x = root(u);
                if (colorsIncoming || !palette[x]) queryPush(x);
                if (finished()) return;
            }
            if (i<m) queryPop();
        }
    }

    void batchSolve() {
        while (!belt.empty() && !finished()) queryPop();
        if (finished()) return;
        auto batches = cohorts();
        localGroups.insert(localGroups.end(),batches.begin(),batches.end());
        int recent = -1, previous = edgesRemoved, stagnant = 0;
        while (!finished()) {
            strongReview();
            if (finished()) break;
            if (edgesRemoved == previous) ++stagnant;
            else stagnant = 0;
            previous = edgesRemoved;
            if (stagnant >= 3) {
                direct();
                while (!belt.empty() && !finished()) queryPop();
                stagnant = 0;
                continue;
            }
            Set mask = onBelt();
            int best = -1;
            double bestScore = -1;
            vector<int> batch;
            for (int i = 0; i < (int)batches.size(); ++i) {
                vector<int> items;
                double val = 0;
                for (int u : batches[i]) {
                    int x = root(u);
                    if (palette[x]) continue;
                    Set d = possible[x] & palette;
                    int a = (d & mask).count(), b = d.count()-a;
                    double gain = 4.0*a*b/((a+b)*(a+b));
                    val += 0.35 + gain + (a && b && min(a,b)==1 ? 0.15 : 0);
                    items.push_back(x);
                }
                if (items.empty()) continue;
                if (i == recent) val *= 0.43;
                if (val > bestScore) {
                    bestScore = val;
                    best = i;
                    batch = move(items);
                }
            }
            if (best < 0) { adaptive(); return; }
            recent = best;
            // Add other samples when proved different from all chosen samples.
            Set forbidden;
            for (int x : batch) forbidden |= possible[x];
            Set more = live & ~(forbidden | palette);
            while (more.any()) {
                int x = first(more);
                batch.push_back(x);
                more &= ~possible[x];
            }
            transition(batch,false);
            if (finished()) break;

            Set rows = onBelt(), colors, chosen;
            vector<int> ids;
            for (int x = first(rows); x < M; x = next(rows,x)) {
                if (palette[x]) continue;
                colors |= possible[x] & palette;
                ids.push_back(x);
            }

            // Choose a common subset which bisects many candidate lists.
            vector<int> order;
            for (int c = first(colors); c < M; c = next(colors,c)) {
                if (!rows[c]) order.push_back(c);
            }
            shuffle(order.begin(),order.end(),rng);
            array<int,M> amount{}, degree{};
            for (int x : ids) degree[x] = (possible[x] & palette).count();
            for (int c : order) {
                double benefit = 0;
                for (int x : ids) if (possible[x][c]) {
                    int d = degree[x];
                    benefit += 4.0*(1.36*d-2*amount[x]-1)/(d*d);
                }
                if (benefit > 0.035) {
                    chosen.set(c);
                    for (int x : ids) if (possible[x][c]) ++amount[x];
                }
            }
            if (chosen.none() && !ids.empty()) {
                chosen.set(first(possible[ids[0]] & palette));
            }
            vector<int> refs;
            for (int y = first(chosen); y < M; y = next(chosen,y)) refs.push_back(y);
            transition(refs,true);
        }
    }

    // For small cases retain every partition consistent with the transcript.
    // The lookahead uses a sample for speed; filtering always uses all of them.
    bool smallCase() {
        if (n>12) return false;
        vector<int> reps;
        vector<vector<int>> choices(n);
        for (int i=0;i<n;++i) {
            if (!push(i)) {
                reps.push_back(i);
                choices[i]={i};
            } else choices[i]=reps;
        }
        struct Hypothesis { array<unsigned short,12> same{}; };
        vector<Hypothesis> hypotheses;
        array<int,12> color{};
        auto enumerate=[&](auto& self,int i)->void {
            if (i==n) {
                Hypothesis h;
                for (int x=0;x<n;++x) for (int y=0;y<n;++y)
                    if (color[x]==color[y]) h.same[x] |= 1u<<y;
                hypotheses.push_back(h);
                return;
            }
            for (int c : choices[i]) {
                color[i]=c;
                self(self,i+1);
            }
        };
        enumerate(enumerate,0);
        vector<int> ids(hypotheses.size());
        iota(ids.begin(),ids.end(),0);
        int stale=0;
        while (ids.size()>1) {
            vector<int> sample=ids;
            shuffle(sample.begin(),sample.end(),rng);
            sample.resize(min(size_t(64),sample.size()));
            int actualAction=-2;
            vector<int> q(belt.begin(),belt.end());
            auto plan=[&](auto& self,const vector<int>& h,vector<int>& state,
                          int depth,bool top)->double {
                if (h.size()<2 || depth==0) return 0;
                unsigned mask=0, tail=0;
                for (int x : state) mask |= 1u<<x;
                for (size_t i=1;i<state.size();++i) tail |= 1u<<state[i];
                double best=-1;
                for (int a=-1;a<n;++a) {
                    if (a<0 && state.empty()) continue;
                    if (a>=0 && (mask>>a&1)) continue;
                    int x=a<0?state.front():a;
                    unsigned s=a<0?tail:mask;
                    vector<int> yes,no;
                    for (int k : h) {
                        ((hypotheses[k].same[x]&s)?yes:no).push_back(k);
                    }
                    double p=double(yes.size())/h.size(), gain=0;
                    if (p>0 && p<1) gain=-p*log2(p)-(1-p)*log2(1-p);
                    if (depth>1) {
                        if (a<0) state.erase(state.begin());
                        else state.push_back(a);
                        gain+=0.86*(p*self(self,yes,state,depth-1,false)
                                   +(1-p)*self(self,no,state,depth-1,false));
                        if (a<0) state.insert(state.begin(),x);
                        else state.pop_back();
                    }
                    if (gain>best+1e-8) {
                        best=gain;
                        if (top) actualAction=a;
                    }
                }
                return best;
            };
            double gain=plan(plan,sample,q,3,true);
            auto perform=[&](int a) {
                int x=a<0?belt.front():a;
                unsigned mask=0;
                for (size_t i=(a<0); i<belt.size(); ++i) mask |= 1u<<belt[i];
                bool hit=a<0?pop():push(a);
                erase_if(ids,[&](int k){return bool(hypotheses[k].same[x]&mask)!=hit;});
            };
            size_t before=ids.size();
            if (stale>=2*n+4) {
                while (!belt.empty() && ids.size()>1) perform(-1);
                if (ids.size()>1) {
                    auto& a=hypotheses[ids.front()];
                    auto& b=hypotheses[ids.back()];
                    for (int x=0;x<n;++x) if (a.same[x]!=b.same[x]) {
                        int y=countr_zero(unsigned(a.same[x]^b.same[x]));
                        perform(x);
                        perform(y);
                        break;
                    }
                }
            } else {
                if (gain<1e-8 && !belt.empty()) actualAction=-1;
                perform(actualAction);
            }
            stale=before==ids.size()?stale+1:0;
        }
        const auto& h=hypotheses[ids[0]];
        for (int x=0;x<n;++x) parent[x]=countr_zero(unsigned(h.same[x]));
        return true;
    }

    void run() {
        if (n != 1 && !smallCase()) {
            initialize();
            if (!finished()) {
                if (classes <= 3 || (live & ~(palette | lastPalette)).count() < 1 + live.count()/15) adaptive();
                else batchSolve();
            }
        }
        map<int, vector<int>> groups;
        for (int i = 0; i < n; ++i) groups[root(i)].push_back(i+1);
        cout << "! " << groups.size() << endl;
        for (const auto& [r,g] : groups) {
            cout << g.size();
            for (int i : g) cout << ' ' << i;
            cout << endl;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (cin >> n && n >= 1 && n <= 500) Classifier(n).run();
}
