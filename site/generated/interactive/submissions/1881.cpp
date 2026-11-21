#include <bits/stdc++.h>
using namespace std;

using VI = vector<int>;

struct Solver {
    struct Task {
        VI a, b;
    };

    struct Split {
        int side = 0, quota = 0, ones = 0, zeros = 0, left = 0;
        bool byPop = false;
        vector<char> part;
        VI hit;

        void set(int i, int v) {
            if (hit[i] != -1) return;
            hit[i] = v;
            --left;
            if (v) ++ones;
            else ++zeros;
            int fill = -1;
            if (ones == quota) fill = 0;
            if (zeros == int(hit.size()) - quota) fill = 1;
            if (fill != -1) {
                for (int &x : hit) if (x == -1) x = fill;
                left = 0;
            }
        }
    };

    int n, distinct = 0;
    VI parent, weight;
    deque<int> belt;

    explicit Solver(int n) : n(n), parent(n + 1), weight(n + 1, 1) {
        iota(parent.begin(), parent.end(), 0);
    }

    int root(int x) {
        return parent[x] == x ? x : parent[x] = root(parent[x]);
    }

    void join(int a, int b) {
        a = root(a);
        b = root(b);
        if (a == b) return;
        if (weight[a] < weight[b]) swap(a, b);
        parent[b] = a;
        weight[a] += weight[b];
    }

    int readCount() {
        int v;
        if (!(cin >> v) || v < 0) exit(0);
        return v;
    }

    bool push(int x) {
        cout << "+ " << x << endl;
        int v = readCount();
        bool fresh = v > distinct;
        distinct = v;
        belt.push_back(x);
        return fresh;
    }

    pair<int, bool> pop() {
        cout << '-' << endl;
        int v = readCount();
        int x = belt.front();
        belt.pop_front();
        bool gone = v < distinct;
        distinct = v;
        return {x, gone};
    }

    void discard(int k) {
        while (k-- > 0) pop();
    }

    void clearBelt() {
        discard(int(belt.size()));
    }

    VI representatives(const VI &ids) {
        vector<char> seen(n + 1);
        VI res;
        for (int id : ids) {
            int r = root(id);
            if (!seen[r]) {
                seen[r] = 1;
                res.push_back(id);
            }
        }
        return res;
    }

    // Each task is a bijection. The classes of different tasks are disjoint.
    // cap[b], when nonzero, bounds the position of b's partner in its A list.
    void perfect(vector<Task> tasks, const VI &safe, VI cap) {
        VI rank(n + 1);
        for (auto &t : tasks) {
            int m = int(t.a.size());
            for (int i = 0; i < m; ++i) rank[t.a[i]] = i + 1;
            for (int x : t.b) if (!cap[x]) cap[x] = m;
            stable_sort(t.b.begin(), t.b.end(),
                        [&](int x, int y) { return cap[x] < cap[y]; });
        }

        bool cleaned = false;
        while (!tasks.empty()) {
            vector<Task> work;
            for (auto &t : tasks) {
                int m = int(t.a.size()), l = 0;
                for (int i = 0; i < m; ++i) {
                    if (i + 1 < m && cap[t.b[i]] >= rank[t.a[i + 1]]) continue;
                    if (l == i) {
                        join(t.a[i], t.b[i]);
                    } else {
                        work.push_back({
                            VI(t.a.begin() + l, t.a.begin() + i + 1),
                            VI(t.b.begin() + l, t.b.begin() + i + 1)
                        });
                    }
                    l = i + 1;
                }
            }
            tasks.swap(work);
            if (tasks.empty()) break;

            int z = int(tasks.size());
            VI group(n + 1, -1), side(n + 1), occ(n + 1), row(n + 1, -1);
            vector<array<int, 2>> last(z, {-1, -1});
            for (int g = 0; g < z; ++g) {
                for (int x : tasks[g].a) {
                    group[root(x)] = g;
                    side[root(x)] = 0;
                }
                for (int x : tasks[g].b) {
                    group[root(x)] = g;
                    side[root(x)] = 1;
                }
            }

            vector<char> allowed(n + 1);
            if (!cleaned) for (int x : safe) allowed[root(x)] = 1;
            int cut = -1, pos = 0;
            for (int x : belt) {
                int r = root(x), g = group[r];
                if (g != -1) last[g][side[r]] = pos;
                else if (!cleaned && !allowed[r]) cut = pos;
                ++pos;
            }
            for (auto v : last) cut = max(cut, min(v[0], v[1]));
            discard(cut + 1);
            cleaned = true;
            for (int x : belt) ++occ[root(x)];

            vector<Split> plans(z);
            int pending = 0;
            for (int g = 0; g < z; ++g) {
                auto &t = tasks[g];
                auto &s = plans[g];
                int m = int(t.a.size()), haveA = 0, haveB = 0;
                for (int x : t.a) haveA += occ[root(x)] != 0;
                for (int x : t.b) haveB += occ[root(x)] != 0;
                int have = max(haveA, haveB);
                int presentSide = haveB ? 1 : 0;

                s.byPop = have > (m + 1) / 2;
                s.side = s.byPop ? 1 - presentSide : presentSide;
                s.quota = s.byPop ? m / 2 : max(m / 2, have);
                s.left = m;
                s.part.assign(m, 0);
                s.hit.assign(m, -1);

                const VI &p = s.side == 0 ? t.a : t.b;
                const VI &r = s.side == 0 ? t.b : t.a;
                int used = 0;
                if (!s.byPop) {
                    for (int j = 0; j < m; ++j) if (occ[root(p[j])]) {
                        s.part[j] = 1;
                        ++used;
                    }
                }
                for (int j = 0; j < m && used < s.quota; ++j) if (!s.part[j]) {
                    s.part[j] = 1;
                    ++used;
                }

                if (s.side == 0) {
                    int mn[2] = {n + 1, n + 1};
                    for (int j = 0; j < m; ++j)
                        mn[s.part[j]] = min(mn[s.part[j]], rank[p[j]]);
                    for (int j = 0; j < m && s.left; ++j) {
                        if (cap[r[j]] < mn[0]) s.set(j, 1);
                        else if (cap[r[j]] < mn[1]) s.set(j, 0);
                    }
                } else {
                    int mx[2] = {0, 0};
                    for (int j = 0; j < m; ++j)
                        mx[s.part[j]] = max(mx[s.part[j]], cap[p[j]]);
                    for (int j = 0; j < m && s.left; ++j) {
                        if (rank[r[j]] > mx[0]) s.set(j, 1);
                        else if (rank[r[j]] > mx[1]) s.set(j, 0);
                    }
                }

                if (s.byPop && s.left) {
                    ++pending;
                    for (int j = 0; j < m; ++j) row[root(r[j])] = j;
                }
            }

            for (int g = 0; g < z; ++g) {
                auto &s = plans[g];
                if (!s.left || s.byPop) continue;
                const VI &p = s.side == 0 ? tasks[g].a : tasks[g].b;
                for (int j = 0; j < int(p.size()); ++j)
                    if (s.part[j] && !occ[root(p[j])]) push(p[j]);
            }

            for (int g = 0; g < z; ++g) {
                auto &s = plans[g];
                if (!s.left || !s.byPop) continue;
                const VI &r = s.side == 0 ? tasks[g].b : tasks[g].a;
                for (int j = 0; j < int(r.size()); ++j) {
                    int x = root(r[j]);
                    if (s.hit[j] == -1 && !occ[x]) {
                        push(r[j]);
                        ++occ[x];
                    }
                }
            }

            for (int g = 0; g < z; ++g) {
                auto &s = plans[g];
                if (!s.left || !s.byPop) continue;
                const VI &p = s.side == 0 ? tasks[g].a : tasks[g].b;
                for (int j = 0; j < int(p.size()); ++j) if (s.part[j]) push(p[j]);
            }

            for (int g = 0; g < z; ++g) {
                auto &s = plans[g];
                if (!s.left || s.byPop) continue;
                const VI &r = s.side == 0 ? tasks[g].b : tasks[g].a;
                for (int j = 0; j < int(r.size()) && s.left; ++j)
                    if (s.hit[j] == -1) s.set(j, !push(r[j]));
            }

            while (pending) {
                auto [id, gone] = pop();
                int x = root(id), j = row[x];
                if (j < 0 || --occ[x] != 0) continue;
                auto &s = plans[group[x]];
                if (s.left && s.hit[j] == -1) {
                    s.set(j, !gone);
                    if (!s.left) --pending;
                }
            }

            vector<Task> next;
            for (int g = 0; g < z; ++g) {
                auto &t = tasks[g];
                auto &s = plans[g];
                Task child[2];
                for (int j = 0; j < int(t.a.size()); ++j) {
                    child[s.side == 0 ? int(s.part[j]) : s.hit[j]].a.push_back(t.a[j]);
                    child[s.side == 1 ? int(s.part[j]) : s.hit[j]].b.push_back(t.b[j]);
                }
                for (auto &c : child) next.push_back(move(c));
            }
            tasks.swap(next);
        }
    }

    VI mergeLists(VI a, VI b, int overlap) {
        VI all = a;
        all.insert(all.end(), b.begin(), b.end());
        if (overlap == 0) return all;
        if (overlap == int(a.size()) && overlap == int(b.size())) {
            perfect({{a, b}}, all, VI(n + 1));
            return representatives(all);
        }

        VI side(n + 1, -1), occ(n + 1);
        for (int id : a) side[root(id)] = 0;
        for (int id : b) side[root(id)] = 1;
        int last[2] = {-1, -1}, cut = -1, pos = 0;
        for (int id : belt) {
            int s = side[root(id)];
            if (s < 0) cut = pos;
            else last[s] = pos;
            ++pos;
        }
        cut = max(cut, min(last[0], last[1]));
        discard(cut + 1);
        if ((!belt.empty() && side[root(belt.front())] == 1) ||
            (belt.empty() && b.size() < a.size())) swap(a, b);

        for (int id : belt) ++occ[root(id)];
        for (int id : a) if (!occ[root(id)]) {
            push(id);
            ++occ[root(id)];
        }

        VI bb;
        for (int id : b) {
            if (!push(id)) bb.push_back(id);
            if (int(bb.size()) == overlap) break;
        }

        int m = int(bb.size());
        if (m == 0) return all;
        VI aa;
        if (m == int(a.size())) {
            aa = a;
        } else {
            VI state(n + 1, -1);
            int yes = 0, no = 0;
            while (yes < m && no < int(a.size()) - m) {
                auto [id, gone] = pop();
                int r = root(id);
                if (--occ[r] != 0) continue;
                state[r] = !gone;
                if (gone) ++no;
                else ++yes;
            }
            for (int id : a) {
                int v = state[root(id)];
                if (v == 1 || (v == -1 && yes < m)) aa.push_back(id);
            }
        }
        perfect({{move(aa), move(bb)}}, all, VI(n + 1));
        return representatives(all);
    }

    // The belt may retain any number of copies of the known classes.
    void direct(int id, const VI &cand) {
        if (cand.size() == 1) {
            join(id, cand[0]);
            return;
        }

        VI last(n + 1, -1);
        int pos = 0;
        for (int x : belt) last[root(x)] = pos++;
        VI cuts = {0}, positions;
        for (int x : cand) {
            int p = last[root(x)];
            if (p >= 0) {
                cuts.push_back(p + 1);
                positions.push_back(p);
            }
        }
        sort(positions.begin(), positions.end());

        int bestCut = 0;
        double bestCost = 1e100;
        for (int cut : cuts) {
            auto it = lower_bound(positions.begin(), positions.end(), cut);
            int present = int(positions.end() - it);
            int absent = int(cand.size()) - present;
            double cost = 0;
            if (present >= 2) {
                for (auto j = it; j != positions.end() - 1; ++j) cost += *j + 1 - cut;
                cost += positions[positions.size() - 2] + 1 - cut;
            }
            if (absent >= 2) cost += 1.5 * (absent - 1) * (absent + 2) / 2.0;
            cost = cut + cost / cand.size();
            if (cost < bestCost) {
                bestCost = cost;
                bestCut = cut;
            }
        }

        discard(bestCut);
        VI in, out;
        for (int x : cand) {
            if (last[root(x)] >= bestCut) in.push_back(x);
            else out.push_back(x);
        }

        if (push(id)) {
            for (int j = 0; j + 1 < int(out.size()); ++j) {
                if (!push(out[j])) {
                    join(id, out[j]);
                    return;
                }
            }
            join(id, out.back());
        } else if (in.size() == 1) {
            join(id, in[0]);
        } else {
            int p = bestCut, left = int(in.size());
            vector<char> alive(n + 1);
            for (int x : in) alive[root(x)] = 1;
            while (left > 1) {
                auto [x, gone] = pop();
                int r = root(x);
                if (alive[r] && last[r] == p) {
                    if (!gone) {
                        join(id, x);
                        return;
                    }
                    alive[r] = 0;
                    --left;
                }
                ++p;
            }
            for (int x : in) if (alive[root(x)]) {
                join(id, x);
                return;
            }
        }
    }

    bool beltIsPrefix(const VI &ids) {
        if (belt.size() > ids.size()) return false;
        return equal(belt.begin(), belt.end(), ids.begin());
    }

    VI lastFlags(const VI &ids, const VI &first, int k) {
        int m = int(ids.size());
        VI last(m);
        if (!belt.empty() && beltIsPrefix(ids)) {
            for (int i = int(belt.size()); i < m; ++i) push(ids[i]);
            for (int i = 0; i < m; ++i) {
                if (distinct == m - i) {
                    fill(last.begin() + i, last.end(), 1);
                    break;
                }
                if (distinct == 1) {
                    last.back() = 1;
                    break;
                }
                last[i] = pop().second;
            }
        } else {
            clearBelt();
            int j = m - 1;
            while (j >= 0 && first[j]) {
                last[j--] = 1;
                --k;
            }
            for (; j >= 0 && k > 0; --j) {
                if (k == j + 1) {
                    fill(last.begin(), last.begin() + j + 1, 1);
                    break;
                }
                last[j] = push(ids[j]);
                k -= last[j];
            }
        }
        return last;
    }

    void oneUnknown(const VI &ids, const VI &first, int p, const VI &candidates) {
        if (int(belt.size()) > p && beltIsPrefix(ids)) {
            VI last(n + 1, -1);
            for (int j = 0; j < p; ++j) last[root(ids[j])] = j;
            int left = int(candidates.size());
            for (int j = 0; j < p && left > 1; ++j) {
                auto [x, gone] = pop();
                if (last[root(x)] != j) continue;
                if (!gone) {
                    join(ids[p], x);
                    return;
                }
                last[root(x)] = -1;
                --left;
            }
            for (int x : candidates) if (last[root(x)] != -1) {
                join(ids[p], x);
                return;
            }
        } else {
            clearBelt();
            push(ids[p]);
            for (int j = 0; j + 1 < int(candidates.size()); ++j) {
                if (!push(candidates[j])) {
                    join(ids[p], candidates[j]);
                    return;
                }
            }
            join(ids[p], candidates.back());
        }
    }

    VI classify(VI ids, VI first = {}, VI last = {}) {
        int m = int(ids.size());
        if (m <= 1) return ids;
        if (first.empty() && !last.empty()) {
            reverse(ids.begin(), ids.end());
            reverse(last.begin(), last.end());
            first.swap(last);
        }
        if (first.empty()) {
            clearBelt();
            for (int id : ids) first.push_back(push(id));
        }

        int k = accumulate(first.begin(), first.end(), 0);
        if (k == m) return ids;
        if (k == 1) {
            for (int x : ids) join(ids[0], x);
            return {ids[0]};
        }

        VI starts;
        int ambiguous = 0, unknown = -1;
        for (int i = 0; i < m; ++i) {
            if (first[i]) starts.push_back(ids[i]);
            else if (starts.size() == 1) join(starts[0], ids[i]);
            else {
                ++ambiguous;
                unknown = i;
            }
        }
        if (!ambiguous) return starts;
        if (ambiguous == 1 && last.empty()) {
            VI candidates;
            for (int i = 0; i < unknown; ++i)
                if (first[i]) candidates.push_back(ids[i]);
            oneUnknown(ids, first, unknown, candidates);
            return starts;
        }

        if (last.empty()) last = lastFlags(ids, first, k);

        vector<VI> components;
        VI cur;
        int active = 0;
        for (int i = 0; i < m; ++i) {
            if (first[i] && last[i]) continue;
            cur.push_back(i);
            active += first[i] - last[i];
            if (!active) {
                components.push_back(move(cur));
                cur.clear();
            }
        }

        vector<Task> pending;
        VI cap(n + 1);
        for (const VI &comp : components) {
            VI a, ends;
            int open = 0, anchor = 0;
            for (int i : comp) {
                int x = ids[i];
                if (first[i]) {
                    a.push_back(x);
                    anchor = (++open == 1 ? x : 0);
                } else if (last[i]) {
                    ends.push_back(x);
                    if (open == 1 && anchor) join(anchor, x);
                    --open;
                    anchor = 0;
                } else if (open == 1) {
                    if (anchor) join(anchor, x);
                    else anchor = x;
                }
            }

            int r = int(a.size());
            if (r == 1) {
                for (int i : comp) join(a[0], ids[i]);
                continue;
            }

            vector<char> seen(n + 1);
            for (int x : a) seen[root(x)] = 1;
            for (int x : ends) seen[root(x)] = 1;

            VI w, wf, wl, u, ul;
            for (int i : comp) {
                int x = ids[i], rt = root(x);
                if (!first[i] && !last[i] && seen[rt]) continue;
                seen[rt] = 1;
                w.push_back(x);
                wf.push_back(first[i]);
                wl.push_back(last[i]);
                if (!first[i]) {
                    u.push_back(x);
                    ul.push_back(last[i]);
                }
            }

            int length = int(w.size());
            if (r > 6 && length > 3 * r) {
                int cut = length / 2, best = INT_MAX, prefix = 0, closed = 0;
                int leftK = 0, rightK = 0;
                for (int j = 1; j < length; ++j) {
                    prefix += wf[j - 1];
                    closed += wl[j - 1];
                    if (j * 3 < length || j * 3 > 2 * length) continue;
                    int cost = 6 * (prefix - closed) + abs(2 * j - length);
                    if (cost < best) {
                        best = cost;
                        cut = j;
                        leftK = prefix;
                        rightK = r - closed;
                    }
                }
                VI l = classify(VI(w.begin(), w.begin() + cut),
                                VI(wf.begin(), wf.begin() + cut));
                VI b = classify(VI(w.begin() + cut, w.end()), {},
                                VI(wl.begin() + cut, wl.end()));
                mergeLists(move(l), move(b), leftK + rightK - r);
                continue;
            }

            if (r <= 6 && length > 2 * r) {
                VI safe = a;
                safe.insert(safe.end(), ends.begin(), ends.end());
                VI bound(n + 1, n + 1), ec(n + 1);
                int started = 0;
                for (int i : comp) {
                    if (first[i]) ++started;
                    else {
                        int rt = root(ids[i]);
                        bound[rt] = min(bound[rt], started);
                    }
                }
                for (int x : ends) ec[x] = bound[root(x)];
                perfect({{a, ends}}, safe, move(ec));

                vector<char> known(n + 1), live(n + 1);
                for (int x : a) known[root(x)] = 1;
                bool prepared = false;
                for (int j = 0; j < length; ++j) {
                    int x = w[j], rt = root(x);
                    if (wf[j]) live[rt] = 1;
                    else if (wl[j]) live[rt] = 0;
                    else {
                        VI candidates;
                        for (int y : a) if (live[root(y)]) candidates.push_back(y);
                        if (candidates.size() > 1 && !prepared) {
                            int cut = -1, p = 0;
                            for (int y : belt) {
                                if (!known[root(y)]) cut = p;
                                ++p;
                            }
                            discard(cut + 1);
                            prepared = true;
                        }
                        direct(x, candidates);
                        // Union by size can choose x's old root.
                        int newRoot = root(x);
                        live[newRoot] = 1;
                        known[newRoot] = 1;
                    }
                }
                continue;
            }

            VI b = int(u.size()) == r ? move(u) : classify(move(u), {}, move(ul));
            VI bound(n + 1, n + 1);
            int started = 0;
            for (int i : comp) {
                if (first[i]) ++started;
                else {
                    int rt = root(ids[i]);
                    bound[rt] = min(bound[rt], started);
                }
            }
            for (int x : b) cap[x] = bound[root(x)];
            pending.push_back({move(a), move(b)});
        }

        if (!pending.empty()) perfect(move(pending), ids, move(cap));
        return representatives(ids);
    }

    void run() {
        VI ids(n);
        iota(ids.begin(), ids.end(), 1);
        classify(ids);
        vector<VI> piles(n + 1);
        int k = 0;
        for (int x : ids) {
            auto &p = piles[root(x)];
            if (p.empty()) ++k;
            p.push_back(x);
        }
        cout << "! " << k << endl;
        for (const VI &p : piles) if (!p.empty()) {
            cout << p.size();
            for (int x : p) cout << ' ' << x;
            cout << endl;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 1) return 0;
    Solver(n).run();
    return 0;
}
