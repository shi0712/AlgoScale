#include <bits/stdc++.h>
using namespace std;

int n, width;
mt19937 rng(712367);
struct Query { int i, j, r; };
vector<Query> history;

int ask(int i, int j) {
    cout << "? " << i << ' ' << j << endl;
    int r;
    if (!(cin >> r) || r < 0 || r >= width) exit(0);
    history.push_back({i, j, r});
    return r;
}

int multiplicity(const vector<int>& p) {
    int count = 0;
    for (int d = 0; d < n; ++d) {
        bool ok = true;
        for (int i = 0; i < n && ok; ++i) {
            int k = i ^ d;
            ok = k < n && p[k] == (p[i] ^ d);
        }
        count += ok;
    }
    return count;
}

[[noreturn]] void finish(const vector<int>& p) {
    cout << "!" << endl << multiplicity(p) << endl;
    for (int i = 0; i < n; ++i) cout << (i ? " " : "") << p[i];
    cout << endl;
    exit(0);
}

// Local indices into the lists of still available positions and values.
struct Hypothesis {
    array<unsigned char, 16> p{}, b{};
};

// Exact minimum expected number of further queries for a small posterior.
// Each bit represents one consistent permutation with equal prior weight.
template<class Reply>
pair<int,int> smallTree(int count, int side, int m, Reply get) {
    using Mask=uint32_t;
    Mask all=(Mask(1)<<count)-1;
    vector<Mask> same(count,all);
    struct Test { int i,j; vector<Mask> groups; };
    vector<Test> tests;
    set<vector<Mask>> seen;
    for (int i=0;i<side;++i) for (int j=0;j<side;++j) {
        if (i==m && j==m) continue;
        map<int,Mask> byReply;
        for (int a=0;a<count;++a) byReply[get(a,i,j)] |= Mask(1)<<a;
        if (byReply.size()<2) continue;
        vector<Mask> groups;
        for (auto [r,g]:byReply) {
            groups.push_back(g);
            for (Mask t=g;t;t&=t-1) same[countr_zero(t)] &= g;
        }
        sort(groups.begin(),groups.end());
        if (seen.insert(groups).second) tests.push_back({i,j,move(groups)});
    }
    sort(tests.begin(),tests.end(),[](const Test& a,const Test& b) {
        return a.groups.size()>b.groups.size();
    });
    pair<int,int> chosen{-1,-1};
    unordered_map<Mask,int> memo;
    auto solve = [&](auto&& self, Mask s) -> int {
        if ((s & ~same[countr_zero(s)])==0) return 0;
        auto found=memo.find(s);
        if (found!=memo.end()) return found->second;
        int base=popcount(s), best=1000000;
        for (auto& t:tests) {
            int cost=base;
            for (Mask g:t.groups) {
                Mask b=s&g;
                if (b==s) { cost=best; break; }
                if (b && (b&(b-1))) cost+=self(self,b);
                if (cost>=best) break;
            }
            if (cost<best) {
                best=cost;
                if (s==all) chosen={t.i,t.j};
                if (best==base) break;
            }
        }
        memo[s]=best;
        return best;
    };
    solve(solve,all);
    return chosen;
}

void distinguish(vector<Hypothesis>& h, const vector<int>& rows, const vector<int>& vals,
                 vector<int> p) {
    int m = rows.size();
    int anchor = -1;
    for (int i = 0; i < n; ++i) if (p[i] >= 0) { anchor = i; break; }
    vector<int> freq(width), used;
    vector<double> cost(h.size() + 1);
    for (size_t k = 1; k < cost.size(); ++k) cost[k] = k * log2(double(k));
    auto reply = [&](const Hypothesis& a, int i, int j) {
        int v = i == m ? p[anchor] : vals[a.p[i]];
        int k = j == m ? anchor : rows[a.b[j]];
        return v ^ k;
    };
    while (h.size() > 1) {
        double best = cost[h.size()];
        int bi = -1, bj = -1;
        if (h.size()<=20) {
            tie(bi,bj)=smallTree(h.size(),m+(anchor>=0),m,
                [&](int a,int i,int j){return reply(h[a],i,j);});
        } else {
            for (int i = 0; i < m + (anchor >= 0); ++i) {
                for (int j = 0; j < m + (anchor >= 0); ++j) {
                    if (i == m && j == m) continue;
                    used.clear();
                    for (auto& a : h) {
                        int r = reply(a, i, j);
                        if (freq[r]++ == 0) used.push_back(r);
                    }
                    double score = 0;
                    for (int r : used) { score += cost[freq[r]]; freq[r] = 0; }
                    if (score + 1e-8 < best) { best = score; bi = i; bj = j; }
                    if (best == 0) break;
                }
                if (best == 0) break;
            }
        }
        if (bi < 0) break;
        int r = ask(bi == m ? anchor : rows[bi], bj == m ? p[anchor] : vals[bj]);
        erase_if(h, [&](const Hypothesis& a) { return reply(a, bi, bj) != r; });
    }
    if (h.empty()) exit(0);
    for (int u = 0; u < m; ++u) p[rows[u]] = vals[h[0].p[u]];
    finish(p);
}

// Use common permutation patterns only to choose informative probes.
// Every inferred entry still follows from the replies and bijectivity.
struct Predictor {
    int type=-1, shift=0;
    explicit Predictor(const vector<int>& p) {
        vector<vector<int>> freq(3,vector<int>(width));
        int best=0, seen=0;
        for (int i=0;i<n;++i) if (p[i]>=0) {
            ++seen;
            int shifts[]={p[i]^i,(p[i]-i+n)%n,(p[i]+i)%n};
            for (int t=0;t<3;++t) if (++freq[t][shifts[t]]>best) {
                best=freq[t][shifts[t]]; type=t; shift=shifts[t];
            }
        }
        if (seen<2 || best*4<seen*3) type=-1;
    }
    int value(int i) const {
        if (type==0) return i^shift;
        if (type==1) return (i+shift)%n;
        return (shift-i+n)%n;
    }
    int inverse(int j) const {
        return type==1 ? (j-shift+n)%n : value(j);
    }
};

// A table entry (i,v) means that p[i] can still equal v. A query
// links this entry to (v XOR r,j), in both directions.
class Domains {
    vector<unsigned char> possible;
    vector<int> rc, cc, rx, cx;
    vector<vector<pair<int,int>>> rq, cq;
    vector<int> removed;
    vector<pair<bool,int>> single;
    vector<int> rowOrder, colOrder;
    Predictor model;
    int id(int i, int v) const { return i * n + v; }

    void drop(int i, int v) {
        int a = id(i, v);
        if (!possible[a]) return;
        possible[a] = 0;
        rx[i] ^= v;
        cx[v] ^= i;
        if (--rc[i] == 1) single.emplace_back(true, i);
        if (--cc[v] == 1) single.emplace_back(false, v);
        if (!rq[i].empty() || !cq[v].empty()) removed.push_back(a);
    }

    void propagate() {
        while (!single.empty() || !removed.empty()) {
            if (!single.empty()) {
                auto [row, x] = single.back(); single.pop_back();
                int i = row ? x : cx[x], v = row ? rx[x] : x;
                if (row && cc[v] != 1) {
                    for (int k = 0; k < n; ++k) if (k != i) drop(k, v);
                }
                if (!row && rc[i] != 1) {
                    for (int w = 0; w < n; ++w) if (w != v) drop(i, w);
                }
            } else {
                int a = removed.back(); removed.pop_back();
                int i = a / n, v = a % n;
                for (auto [j,r] : rq[i]) if ((v ^ r) < n) drop(v ^ r, j);
                for (auto [k,r] : cq[v]) if ((i ^ r) < n) drop(k, i ^ r);
            }
        }
    }

    void link(int i, int j) {
        int r = ask(i,j);
        rq[i].emplace_back(j,r);
        cq[j].emplace_back(i,r);
        for (int v = 0; v < n; ++v) {
            if (!possible[id(i,v)]) continue;
            int k = v ^ r;
            if (k >= n || !possible[id(k,j)] || ((k == i) != (v == j))) drop(i,v);
        }
        for (int k = 0; k < n; ++k) {
            if (!possible[id(k,j)]) continue;
            int v = k ^ r;
            if (v >= n || !possible[id(i,v)]) drop(k,j);
        }
        propagate();
    }

    void endgame(const vector<int>& rows, const vector<int>& vals) {
        int m = rows.size();
        vector<int> ri(n,-1), vi(n,-1), p(n,-1);
        for (int u = 0; u < m; ++u) { ri[rows[u]] = u; vi[vals[u]] = u; }
        for (int i = 0; i < n; ++i) if (rc[i] == 1) p[i] = rx[i];
        vector<Query> tests;
        for (auto [i,j,r] : history)
            if (ri[i] >= 0 && vi[j] >= 0) tests.push_back({ri[i],vi[j],r});
        vector<Hypothesis> h;
        Hypothesis a;
        auto gen = [&](auto&& self, int u, int mask) -> void {
            if (u == m) {
                for (auto [i,j,r] : tests) if ((vals[a.p[i]] ^ rows[a.b[j]]) != r) return;
                h.push_back(a);
                return;
            }
            for (int v = 0; v < m; ++v) {
                if ((mask >> v & 1) || !possible[id(rows[u], vals[v])]) continue;
                a.p[u] = v; a.b[v] = u;
                self(self,u+1,mask | (1<<v));
            }
        };
        gen(gen,0,0);
        distinguish(h,rows,vals,p);
    }

public:
    explicit Domains(const vector<int>& p) : possible(n*n), rc(n), cc(n), rx(n), cx(n), rq(n), cq(n), model(p) {
        vector<int> b(n,-1);
        for (int i = 0; i < n; ++i) if (p[i] >= 0) b[p[i]] = i;
        for (int i = 0; i < n; ++i) {
            for (int v = 0; v < n; ++v) {
                if (p[i] == v || (p[i] < 0 && b[v] < 0)) {
                    possible[id(i,v)] = 1;
                    ++rc[i]; ++cc[v]; rx[i] ^= v; cx[v] ^= i;
                }
            }
        }
        rowOrder.resize(n); iota(rowOrder.begin(),rowOrder.end(),0);
        colOrder = rowOrder;
        shuffle(rowOrder.begin(),rowOrder.end(),rng);
        shuffle(colOrder.begin(),colOrder.end(),rng);
    }

    void solve() {
        int active = -1, badPredictions=0;
        vector<int> rows, vals;
        while (true) {
            rows.clear(); vals.clear();
            int fresh = 0;
            for (int i : rowOrder) if (rc[i] > 1) { rows.push_back(i); fresh += rq[i].empty(); }
            for (int v : colOrder) if (cc[v] > 1) vals.push_back(v);
            int m = rows.size();
            if (m <= 9) endgame(rows,vals);

            // Grow independent stars. Once a star has few possibilities
            // left, subsequent permutation deductions can finish it for free.
            int threshold = max(3, int(sqrt(m)));
            if (active < 0 || cc[active] == 1 || (cc[active] <= threshold && m-fresh < max(6, m/6)) || fresh == 0) {
                active = -1;
                if (fresh > 0) {
                    int best = INT_MAX;
                    for (int v : vals) if (cq[v].empty() && cc[v] < best) { active=v; best=cc[v]; }
                }
                if (active < 0) {
                    // Resolve the most constrained outstanding star with a
                    // fresh row, or by reading its value against a known row.
                    active = *min_element(vals.begin(),vals.end(),[&](int a,int b){return cc[a]<cc[b];});
                }
            }
            int i = -1;
            int bestCost = INT_MAX;
            for (int k : rows) {
                if (!rq[k].empty()) continue;
                // Avoid using the position of this star's center as a leaf
                // when its domain already rules out other positions.
                int cost = (possible[id(k,active)] ? n+1 : 0) + rc[k];
                if (cost < bestCost) { bestCost=cost; i=k; }
            }
            if (model.type>=0 && fresh>0 && badPredictions<4) {
                vector<int> ks;
                for (int k:rows) if (possible[id(k,active)]) ks.push_back(k);
                int actual=model.inverse(active);
                if (actual<n && possible[id(actual,active)]) {
                    int bestLeft=INT_MAX, tested=0;
                    int start=rng()%rows.size();
                    for (int l=0;l<m && tested<20;++l) {
                        int k=rows[(start+l)%m];
                        if (!rq[k].empty() || k==actual) continue;
                        int v=model.value(k);
                        if (v>=n || !possible[id(k,v)]) continue;
                        ++tested;
                        int r=v^actual, left=0;
                        for (int t:ks) {
                            int u=r^t;
                            if (u<n && ((t==k)==(u==active)) && possible[id(k,u)]) ++left;
                            if (left>=bestLeft) break;
                        }
                        if (left<bestLeft) { bestLeft=left; i=k; }
                        if (left==1) break;
                    }
                }
            }
            if ((int)cq[active].size()>=max(6,int(2*sqrt(m))) && cc[active]>8) i=-1;
            if (i < 0) {
                for (int k : rowOrder) if (rc[k] == 1) { i=k; break; }
            }
            if (i < 0) exit(0);
            link(i,active);
            if (model.type>=0 && (model.value(i)^model.inverse(active))!=history.back().r) ++badPredictions;
        }
    }
};

// Once there are few possible anchor values, keep a partial permutation
// for each. The same star can recover a further inverse entry in every
// surviving permutation, even if those entries have different values.
struct World {
    vector<int> p, b;
    int missing;
    Predictor model;
    int errors=0;
    explicit World(vector<int> a) : p(move(a)), b(n,-1), missing(n), model(p) {
        for (int i=0; i<n; ++i) if (p[i]>=0) { b[p[i]]=i; --missing; }
    }
    bool fits(int i, int v) const {
        return (p[i]<0 || p[i]==v) && (b[v]<0 || b[v]==i);
    }
    void set(int i,int v) {
        if (p[i]<0) { p[i]=v; b[v]=i; --missing; }
    }
    void completeLast() {
        if (missing!=1) return;
        int i=find(p.begin(),p.end(),-1)-p.begin();
        int v=find(b.begin(),b.end(),-1)-b.begin();
        set(i,v);
    }
};

void decideComplete(vector<World>& w, const vector<int>& rows, const vector<int>& vals) {
    vector<int> freq(width);
    while (w.size()>1) {
        int best=INT_MAX, u=-1, v=-1;
        auto score = [&](int i,int j) {
            int cost=0;
            for (auto& a:w) cost+=freq[a.p[i]^a.b[j]]++;
            for (auto& a:w) freq[a.p[i]^a.b[j]]=0;
            if (cost<best) { best=cost; u=i; v=j; }
        };
        for (int i:rows) {
            for (int j:vals) { score(i,j); if (!best) break; }
            if (!best) break;
        }
        for (int i=0;i<n && best;i++) { score(i,0); score(0,i); }
        if (2*best==int(w.size()*(w.size()-1))) break;
        int r=ask(u,v);
        erase_if(w,[&](const World& a){return (a.p[u]^a.b[v])!=r;});
    }
    finish(w.front().p);
}

void solveWorlds(vector<World> w, const vector<int>& order, const vector<int>& values) {
    while (true) {
        int m=0;
        for (auto& a:w) { a.completeLast(); m=max(m,a.missing); }
        if (m==0) decideComplete(w,{},{});
        if (w.size()==1) { Domains(w[0].p).solve(); return; }
        if (m<=5) {
            vector<World> complete;
            set<int> rows, vals;
            for (auto& a:w) {
                vector<int> rr, vv;
                for (int i=0;i<n;++i) {
                    if (a.p[i]<0) rr.push_back(i);
                    if (a.b[i]<0) vv.push_back(i);
                }
                rows.insert(rr.begin(),rr.end()); vals.insert(vv.begin(),vv.end());
                do {
                    complete.push_back(a);
                    for (size_t t=0;t<rr.size();++t) complete.back().set(rr[t],vv[t]);
                } while (next_permutation(vv.begin(),vv.end()));
            }
            decideComplete(complete,{rows.begin(),rows.end()},{vals.begin(),vals.end()});
        }

        // Prefer a value whose inverse is still unknown in many worlds.
        int j=-1, best=-1;
        for (int v:values) {
            int score=0;
            for (auto& a:w) score+=a.b[v]<0;
            if (score>best) { j=v; best=score; }
            if (best==(int)w.size()) break;
        }
        vector<vector<int>> choices(w.size());
        for (size_t h=0; h<w.size(); ++h) {
            if (w[h].b[j]>=0) choices[h].push_back(w[h].b[j]);
            else for (int i:order) if (w[h].p[i]<0) choices[h].push_back(i);
        }
        vector<pair<int,int>> leaves;
        vector<bool> used(n,false);
        while (any_of(choices.begin(),choices.end(),[](const auto& a){return a.size()>1;})) {
            // A known row immediately determines the center in its world.
            // A fresh row also reveals a new permutation entry.
            int i=-1, bestScore=-1;
            for (int k:order) if (!used[k]) {
                int gain=0;
                for (size_t h=0;h<w.size();++h) {
                    if (w[h].p[k]<0) gain+=4;
                    else if (choices[h].size()>1) gain+=3;
                }
                if (gain>bestScore) { i=k; bestScore=gain; }
                if (gain==int(4*w.size())) break;
            }
            int guide=-1;
            for (int h=0;h<(int)w.size();++h) {
                auto& a=w[h];
                if (a.model.type<0 || a.errors>=4 || choices[h].size()<2) continue;
                int k=a.model.inverse(j);
                if (find(choices[h].begin(),choices[h].end(),k)!=choices[h].end()) {
                    guide=h; break;
                }
            }
            if (guide>=0 && bestScore==(int)(4*w.size())) {
                auto& a=w[guide];
                int bestLeft=INT_MAX, tried=0, start=rng()%n;
                int predicted=a.model.inverse(j);
                for (int t=0;t<n && tried<20;++t) {
                    int k=order[(start+t)%n];
                    if (used[k] || k==predicted) continue;
                    bool fresh=true;
                    for (auto& z:w) if (z.p[k]>=0) { fresh=false; break; }
                    int v=a.model.value(k);
                    if (!fresh || v>=n || !a.fits(k,v)) continue;
                    ++tried;
                    int r=v^predicted, left=0;
                    for (int z:choices[guide]) {
                        int u=r^z;
                        if (u<n && ((z==k)==(u==j)) && a.fits(k,u)) ++left;
                        if (left>=bestLeft) break;
                    }
                    if (left<bestLeft) { bestLeft=left; i=k; }
                    if (bestLeft==1) break;
                }
            }
            if ((int)leaves.size()>=max(6,int(2*sqrt(m))) &&
                any_of(choices.begin(),choices.end(),[](const auto& c){return c.size()>8;})) i=0;
            used[i]=true;
            int r=ask(i,j);
            leaves.emplace_back(i,r);
            for (int h=int(w.size())-1; h>=0; --h) {
                auto& a=w[h];
                if (a.model.type>=0 && (a.model.value(i)^a.model.inverse(j))!=r) ++a.errors;
                erase_if(choices[h],[&](int k) {
                    int v=r^k;
                    return v>=n || !a.fits(i,v) || ((k==i)!=(v==j));
                });
                if (choices[h].empty()) {
                    choices.erase(choices.begin()+h);
                    w.erase(w.begin()+h);
                }
            }
            if (w.empty()) exit(0);
        }
        for (size_t h=0;h<w.size();++h) {
            int k=choices[h][0];
            w[h].set(k,j);
            for (auto [i,r]:leaves) w[h].set(i,r^k);
        }
    }
}

struct Cross {
    vector<int> row, col, invRow, invCol, rows, cols, offsets, order, colOrder;
    bool identity = true;
    int extra = 0;
    vector<int> anomalies;

    Cross() : row(n,-1), col(n,-1), invRow(width,-1), invCol(width,-1), offsets(n), order(n) {
        iota(offsets.begin(),offsets.end(),0);
        order = offsets;
        shuffle(order.begin(),order.end(),rng);
        iter_swap(find(order.begin(),order.end(),n-1),order.end()-1);
        colOrder = order;
        shuffle(colOrder.begin(),colOrder.end(),rng);
    }

    void record(bool isRow, int index, int sig) {
        auto& a = isRow ? row : col;
        auto& inv = isRow ? invRow : invCol;
        auto& opp = isRow ? col : row;
        auto& invOpp = isRow ? invCol : invRow;
        (isRow ? rows : cols).push_back(index);
        a[index]=sig; inv[sig]=index;
        identity &= sig == index;
        if (isRow && sig != index) anomalies.push_back(index);
        erase_if(offsets,[&](int x) {
            int v = sig ^ x, t = index ^ x;
            return v >= n || (opp[v] >= 0 && opp[v] != t) ||
                   (invOpp[t] >= 0 && invOpp[t] != v);
        });
    }

    vector<int> partial(int x) const {
        vector<int> p(n,-1);
        for (int i : rows) p[i] = row[i] ^ x;
        for (int j : cols) p[col[j] ^ x] = j;
        return p;
    }

    void small() const {
        if (n > 16) return;
        int total = 0;
        for (int x : offsets) {
            auto p = partial(x);
            int ways = 1;
            for (int t=2; t<=count(p.begin(),p.end(),-1) && ways<80000; ++t) ways*=t;
            total += ways;
            if (total > 80000) return;
        }
        vector<Hypothesis> h;
        for (int x : offsets) {
            auto p = partial(x);
            vector<int> freeRows, v;
            vector<bool> used(n);
            for (int i=0;i<n;++i) {
                if (p[i]<0) freeRows.push_back(i);
                else used[p[i]]=true;
            }
            for (int j=0;j<n;++j) if (!used[j]) v.push_back(j);
            do {
                for (size_t k=0;k<v.size();++k) p[freeRows[k]]=v[k];
                Hypothesis a;
                for (int i=0;i<n;++i) { a.p[i]=p[i]; a.b[p[i]]=i; }
                h.push_back(a);
            } while (next_permutation(v.begin(),v.end()));
        }
        vector<int> all(n); iota(all.begin(),all.end(),0);
        distinguish(h,all,all,vector<int>(n,-1));
    }

    // With all but one row signature known, every offset fixes one permutation.
    void close() {
        int miss = find(row.begin(),row.end(),-1) - row.begin();
        int sum=0;
        for (int i=0;i<n;++i) { sum ^= i; if (i!=miss) sum ^= row[i]; }
        vector<int> p = partial(offsets[0]);
        p[miss] = sum ^ ((n%2==0) ? offsets[0] : 0);
        if (multiplicity(p) == (int)offsets.size()) finish(p);
        auto pred = [&](int j,int x) {
            if (j==n) return sum ^ ((n%2) ? x : 0);
            int k=invRow[j^x];
            return (k<0 ? miss : k) ^ x;
        };
        vector<int> freq(width);
        while (offsets.size()>1) {
            double best=0;
            int bestJ=-1, tested=0;
            auto score = [&](int j) {
                double s=0;
                for (int x:offsets) s += freq[pred(j,x)]++;
                for (int x:offsets) freq[pred(j,x)]=0;
                double gain=double(offsets.size())*(offsets.size()-1)/2-s;
                if (gain==0 && j<n) col[j]=pred(j,offsets[0]);
                if (gain>best) { best=gain; bestJ=j; }
            };
            score(n);
            for (int i:anomalies) {
                if (anomalies.size()>32) break;
                int j=row[i];
                if (j<n && col[j]<0) score(j);
                if (col[i]<0) score(i);
            }
            for (int j : colOrder) {
                if (col[j]>=0) continue;
                score(j);
                if (++tested>=48 && best>0) break;
            }
            if (bestJ<0) break;
            int r=bestJ==n ? ask(miss,0) : (ask(0,bestJ)^row[0]);
            if (bestJ<n) col[bestJ]=r;
            erase_if(offsets,[&](int x){return pred(bestJ,x)!=r;});
        }
        int x=offsets[0];
        for (int i : rows) p[i]=row[i]^x;
        p[miss]=sum ^ ((n%2==0) ? x : 0);
        finish(p);
    }

    void solve() {
        record(false,0,0);
        record(true,0,ask(0,0));
        int probes=0, lastSize=n;
        int anomalyChecks=0;
        while (offsets.size()>1) {
            if (!identity || n!=width) small();
            if ((int)rows.size()==n-1) close();
            if (offsets.size()<=8) {
                vector<World> w;
                for (int x:offsets) w.emplace_back(partial(x));
                vector<int> all=order;
                vector<bool> present(n);
                for (int i:all) present[i]=true;
                for (int i:rows) if (!present[i]) all.push_back(i);
                solveWorlds(move(w),all,colOrder);
                return;
            }
            int c=offsets.size();
            // Spend most initial queries on rows. A few inverse signatures
            // then test overlaps under many competing offsets at once.
            if (anomalyChecks<2 && !anomalies.empty() && anomalies.size()*4 <= rows.size()+2) {
                int j=anomalies.front();
                if (col[j]>=0) j=row[j];
                if (j<n && col[j]<0) {
                    record(false,j,ask(0,j)^row[0]);
                    ++anomalyChecks;
                    continue;
                }
            }
            bool probe = !identity && rows.size()>=3 &&
                         int(cols.size()) <= min(int(rows.size())/3,1+int(2*sqrt(n))) &&
                         (probes<4 || c*4 < lastSize*3 || int(rows.size())>=2*extra+4);
            if (probe) {
                int j=-1, best=-1;
                vector<int> freq(width);
                auto rate = [&](int v) {
                    if (col[v]!=-1) return;
                    int forced=0, largest=0;
                    for (int x : offsets) {
                        int i=invRow[v^x];
                        if (i>=0) { ++forced; largest=max(largest,++freq[i^x]); }
                    }
                    for (int x:offsets) if (invRow[v^x]>=0) freq[invRow[v^x]^x]=0;
                    int s=2*forced-largest;
                    if (s>best) { j=v; best=s; }
                };
                for (int t=0;t<40;++t) {
                    int v = row[rows[rng()%rows.size()]] ^ offsets[rng()%c];
                    if (v<n) rate(v);
                }
                if (j<0) for (int v:colOrder) if (col[v]<0) { j=v; break; }
                if (j>=0) {
                    int sig=-1; bool forced=true;
                    for (int x:offsets) {
                        int i=invRow[j^x];
                        if (i<0 || (sig>=0 && sig!=(i^x))) { forced=false; break; }
                        sig=i^x;
                    }
                    if (!forced) sig=ask(0,j)^row[0];
                    record(false,j,sig);
                    ++probes; extra=rows.size(); lastSize=c;
                    continue;
                }
            }
            // For translation-like replies, boundary positions can rule out
            // whole sets of offsets at once.
            if (identity && width!=n && rows.size()<64 && order.size()>1) {
                int bestPos=int(order.size())-1, best=-1;
                for (int t=0;t<28;++t) {
                    int pos=t==0 ? int(order.size())-1 : int(rng()%order.size());
                    int k=order[pos];
                    if (row[k]>=0) continue;
                    int gain=0;
                    for (int x:offsets) gain+=((k^x)>=n);
                    if (gain>best) { best=gain; bestPos=pos; }
                }
                swap(order[bestPos],order.back());
            }
            int i=order.back(); order.pop_back();
            if (row[i]>=0) continue;
            int sig=-1; bool forced=true;
            for (int x : offsets) {
                int j=invCol[i^x];
                if (j<0 || (sig>=0 && sig!=(j^x))) { forced=false; break; }
                sig=j^x;
            }
            record(true,i,forced ? sig : ask(i,0));
        }
        if (offsets.empty()) exit(0);
        Domains(partial(offsets[0])).solve();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n<1 || n>5000) return 0;
    width=1; while (width<n) width*=2;
    if (n<=9) {
        vector<int> all(n);
        iota(all.begin(),all.end(),0);
        vector<int> perm=all;
        vector<Hypothesis> h;
        do {
            Hypothesis a;
            for (int i=0;i<n;++i) { a.p[i]=perm[i]; a.b[perm[i]]=i; }
            h.push_back(a);
        } while (next_permutation(perm.begin(),perm.end()));
        distinguish(h,all,all,vector<int>(n,-1));
    }
    Cross().solve();
}
