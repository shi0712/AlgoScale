#include <bits/stdc++.h>
using namespace std;

using Word = uint64_t;
constexpr int W = 16;
using Bits = array<Word, W>;
using Edge = pair<int,int>;

static bool has(const Bits &a, int i) { return a[i/64] >> (i%64) & 1; }
static void put(Bits &a, int i) { a[i/64] |= Word(1) << (i%64); }
static void eraseBit(Bits &a, int i) { a[i/64] &= ~(Word(1) << (i%64)); }

string ask(const vector<int>& radii) {
    cout << '?';
    for (int d : radii) cout << ' ' << d;
    cout << endl;
    string s;
    if (!(cin >> s) || s.size() != radii.size() || s.find_first_not_of("01") != string::npos)
        exit(0);
    return s;
}

struct DSU {
    vector<int> p;
    DSU(int n) : p(n,-1) {}
    int root(int v) { return p[v]<0 ? v : p[v]=root(p[v]); }
    bool join(int a, int b) {
        a=root(a); b=root(b);
        if (a==b) return false;
        if (p[a]>p[b]) swap(a,b);
        p[a]+=p[b]; p[b]=a;
        return true;
    }
};

struct Observation {
    Bits sources{};
    string lamps;
};

struct Solver {
    int n, words, length=1, weight=1, step=0;
    DSU forest;
    vector<Bits> possible, known;
    vector<Edge> edges;
    vector<Observation> history;
    vector<int> code, countPoss;
    int remaining=0;
    mt19937 rng{19070413};

    Solver(int size) : n(size), words((n+63)/64), forest(n), possible(n), known(n), countPoss(n) {
        vector<int> pool;
        for (;;length++) {
            weight=max(1, length/2);
            auto choose=[&](int w) { int c=1; for(int i=1;i<=w;i++) c=c*(length-i+1)/i; return c; };
            if (choose(weight)<n) continue;
            while (weight>1 && choose(weight-1)>=n) --weight;
            break;
        }
        for (int i=0; i<(1<<length); i++) if (__builtin_popcount((unsigned)i)==weight) pool.push_back(i);
        shuffle(pool.begin(),pool.end(),rng);
        code.assign(pool.begin(),pool.begin()+n);
        for (int u=0;u<n;u++) for(int v=0;v<n;v++) if(u!=v) put(possible[u],v);
    }

    bool intersects(const Bits &a, const Bits &b) const {
        for(int w=0;w<words;w++) if (a[w]&b[w]) return true;
        return false;
    }
    void add(int a,int b) {
        if (!forest.join(a,b)) return;
        edges.emplace_back(a,b);
        put(known[a],b); put(known[b],a);
    }
    void observe(const vector<int>& d) {
        Observation o;
        for(int i=0;i<n;i++) if(d[i]) put(o.sources,i);
        o.lamps.assign(n,'0');
        for(int v=0;v<n;v++) {
            if(intersects(known[v],o.sources)) { o.lamps[v]='1'; continue; }
            if(!intersects(possible[v],o.sources)) continue;
            Bits outside{};
            bool hasOutside=false,hasKnown=false;
            for(int w=0;w<words;w++) {
                outside[w]=possible[v][w]&~o.sources[w];
                hasOutside|=(outside[w]!=0); hasKnown|=(known[v][w]!=0);
            }
            bool forced=!hasOutside && !hasKnown;
            for(const auto& old:history) if(old.lamps[v]=='1' && !intersects(old.sources,known[v]) && !intersects(old.sources,outside)) {
                forced=true; break;
            }
            if(forced) o.lamps[v]='1';
            else { o.lamps=ask(d); break; }
        }
        Bits off{};
        for(int i=0;i<n;i++) if(o.lamps[i]=='0') put(off,i);
        for(int i=0;i<n;i++) for(int w=0;w<words;w++) {
            if(d[i]) possible[i][w]&=~off[w];
            if(o.lamps[i]=='0') possible[i][w]&=~o.sources[w];
        }
        history.push_back(move(o));
    }

    // The only possible edge across a cut must be present.
    void bridges(const vector<int>& comp) {
        vector<vector<pair<int,int>>> adj(n);
        vector<Edge> pool;
        for(int u=0;u<n;u++) for(int w=u/64;w<words;w++) {
            Word x=possible[u][w];
            while(x) {
                int v=64*w+__builtin_ctzll(x); x&=x-1;
                if(v<=u) continue;
                int id=pool.size(); pool.emplace_back(u,v);
                adj[comp[u]].emplace_back(comp[v],id);
                adj[comp[v]].emplace_back(comp[u],id);
            }
        }
        vector<int> entry(n),low(n);
        int timer=0;
        auto visit=[&](auto&& self,int v,int pe)->void {
            entry[v]=low[v]=++timer;
            for(auto [u,id]:adj[v]) if(id!=pe) {
                if(!entry[u]) {
                    self(self,u,id); low[v]=min(low[v],low[u]);
                    if(low[u]>entry[v]) add(pool[id].first,pool[id].second);
                } else low[v]=min(low[v],entry[u]);
            }
        };
        visit(visit,comp[0],-1);
    }

    bool bundles() {
        DSU clusters=forest;
        vector<Edge> pool;
        vector<vector<int>> incident(n),clauses;
        for(int u=0;u<n;u++) for(int w=u/64;w<words;w++) {
            Word x=possible[u][w];
            while(x) {
                int v=64*w+__builtin_ctzll(x); x&=x-1;
                if(v<=u) continue;
                int id=pool.size(); pool.emplace_back(u,v);
                incident[u].push_back(id); incident[v].push_back(id);
            }
        }
        for(int v=0;v<n;v++) if(!incident[v].empty()) for(const auto& o:history) {
            if(o.lamps[v]=='0' || intersects(known[v],o.sources)) continue;
            vector<int> c;
            for(int id:incident[v]) {
                auto [a,b]=pool[id];
                if(has(o.sources,a^b^v)) c.push_back(id);
            }
            clauses.push_back(move(c));
        }
        vector<int> group(pool.size(),-1),marked(pool.size());
        vector<vector<int>> groups;
        int tick=0;
        bool progress;
        // A bundle is a set containing exactly one edge of the hidden tree.
        // Joining its two clusters makes every other unassigned internal edge
        // impossible, even while the endpoints of the joining edge are unknown.
        auto process=[&](const vector<int>& c) {
            int key=INT_MAX;
            for(int id:c) {
                int t=group[id];
                if(t==-2) continue;
                if(t==-1) {
                    auto [a,b]=pool[id]; a=clusters.root(a); b=clusters.root(b);
                    if(a==b) continue;
                    if(a>b) swap(a,b);
                    t=a*n+b;
                } else t=-1-t;
                if(key!=INT_MAX && key!=t) return;
                key=t;
            }
            if(key==INT_MAX) return;
            if(key>=0) {
                int a=key/n,b=key%n;
                vector<int> g;
                for(int id:c) if(group[id]==-1 && clusters.root(pool[id].first)!=clusters.root(pool[id].second)) {
                    group[id]=groups.size(); g.push_back(id);
                }
                groups.push_back(move(g)); clusters.join(a,b);
                progress=true;
            } else {
                auto& g=groups[-1-key];
                ++tick;
                for(int id:c) marked[id]=tick;
                int keep=0;
                for(int id:g) {
                    if(marked[id]==tick) g[keep++]=id;
                    else {group[id]=-2; progress=true;}
                }
                g.resize(keep);
            }
        };
        do {
            progress=false;
            for(const auto& c:clauses) process(c);
            vector<vector<int>> cuts(n);
            for(int i=0;i<(int)pool.size();i++) if(group[i]==-1) {
                auto [a,b]=pool[i]; a=clusters.root(a); b=clusters.root(b);
                if(a!=b) { cuts[a].push_back(i); cuts[b].push_back(i); }
            }
            for(const auto& c:cuts) if(!c.empty()) process(c);
        } while(progress);
        bool changed=false;
        for(int i=0;i<(int)pool.size();i++) {
            auto [a,b]=pool[i];
            if(group[i]==-2 || (group[i]==-1 && clusters.root(a)==clusters.root(b))) {
                eraseBit(possible[a],b); eraseBit(possible[b],a); changed=true;
            }
        }
        for(const auto& g:groups) if(g.size()==1) {
            auto [a,b]=pool[g[0]]; add(a,b); changed=true;
        }
        return changed;
    }

    // Every component needs an external edge. Unsatisfied lamp replies may
    // require several distinct external neighbors of a particular vertex.
    // The sum of all external degrees is exactly twice the missing edge count.
    bool degrees(const vector<int>& comp) {
        vector<Word> sent(n), need(n);
        vector<vector<int>> adj(n);
        vector<int> minimum(n),total(n);
        int start=max(0,(int)history.size()-64);
        for(int t=start;t<(int)history.size();t++) for(int v=0;v<n;v++) {
            Word b=Word(1)<<(t-start);
            if(has(history[t].sources,v)) sent[v]|=b;
            if(history[t].lamps[v]=='1' && !intersects(history[t].sources,known[v])) need[v]|=b;
        }
        for(int v=0;v<n;v++) {
            auto& c=adj[v];
            for(int w=0;w<words;w++) {
                Word x=possible[v][w];
                while(x) {c.push_back(64*w+__builtin_ctzll(x));x&=x-1;}
            }
            Word req=need[v];
            if(!req) continue;
            int lb=2;
            for(int u:c) if(!(req&~sent[u])) {lb=1;break;}
            if(lb==2 && c.size()<=120) {
                lb=3;
                for(int i=0;i<(int)c.size() && lb==3;i++)
                    for(int j=0;j<i;j++) if(comp[c[i]]!=comp[c[j]] && !(req&~(sent[c[i]]|sent[c[j]]))) {lb=2;break;}
            }
            minimum[v]=lb; total[comp[v]]+=lb;
        }
        int slack=2*(n-1-edges.size());
        for(int v=0;v<n;v++) if(comp[v]==v) slack-=max(1,total[v]);
        if(slack>3) return false;
        bool changed=false;
        for(int v=0;v<n;v++) {
            auto& c=adj[v];
            int cap=minimum[v]+slack+(total[comp[v]]==0);
            if(c.empty() || cap>3 || (cap==3 && c.size()>38) || (cap==2 && c.size()>200)) continue;
            vector<int> used(c.size());
            int ways=(!need[v]);
            for(int i=0;cap>=1 && i<(int)c.size();i++) {
                Word left=need[v]&~sent[c[i]];
                if(!left) {++ways;++used[i];}
                for(int j=0;cap>=2 && j<i;j++) if(comp[c[i]]!=comp[c[j]]) {
                    Word rest=left&~sent[c[j]];
                    if(!rest) {++ways;++used[i];++used[j];}
                    for(int k=0;cap>=3 && k<j;k++) if(comp[c[k]]!=comp[c[i]] && comp[c[k]]!=comp[c[j]] && !(rest&~sent[c[k]])) {
                        ++ways; ++used[i]; ++used[j]; ++used[k];
                    }
                }
            }
            for(int i=0;i<(int)c.size();i++) if(!used[i]) {
                eraseBit(possible[v],c[i]); eraseBit(possible[c[i]],v); changed=true;
            } else if(used[i]==ways) {
                add(v,c[i]); changed=true;
            }
        }
        return changed;
    }

    void deduce() {
        while((int)edges.size()<n-1) {
            auto old=edges.size();
            vector<int> comp(n);
            vector<Bits> members(n);
            for(int i=0;i<n;i++) { comp[i]=forest.root(i); put(members[comp[i]],i); }
            remaining=0;
            for(int v=0;v<n;v++) {
                int count=0;
                for(int w=0;w<words;w++) {
                    possible[v][w]&=~members[comp[v]][w];
                    count+=__builtin_popcountll(possible[v][w]);
                }
                countPoss[v]=count;
                remaining+=count;
                if(!count) continue;
                if(count==1 && forest.p[comp[v]]==-1 && comp[v]==v) {
                    for(int w=0;w<words;w++) if(possible[v][w]) add(v,64*w+__builtin_ctzll(possible[v][w]));
                }
                for(const auto& o:history) {
                    if(o.lamps[v]=='0' || intersects(o.sources,known[v])) continue;
                    int only=-1;
                    for(int w=0;w<words;w++) {
                        Word x=possible[v][w]&o.sources[w];
                        if(!x) continue;
                        if(only!=-1 || (x&(x-1))) { only=-1; break; }
                        only=64*w+__builtin_ctzll(x);
                    }
                    if(only!=-1) add(v,only);
                }
            }
            remaining/=2;
            if(old==edges.size() && remaining<12*n) bridges(comp);
            if(old!=edges.size()) continue;
            if(remaining<=22*n && degrees(comp)) continue;
            if(remaining<=12*n && bundles()) continue;
            break;
        }
    }

    vector<int> adaptive() {
        // Estimate which surviving edges most readily explain the positive lamps.
        vector<Word> sent(n), need(n);
        int start=max(0,(int)history.size()-64);
        for(int j=start;j<(int)history.size();j++) for(int v=0;v<n;v++) {
            Word bit=Word(1)<<(j-start);
            if(has(history[j].sources,v)) sent[v]|=bit;
            if(history[j].lamps[v]=='1' && !intersects(known[v],history[j].sources)) need[v]|=bit;
        }
        vector<vector<int>> candidates(n),neighbors(n);
        vector<vector<float>> prior(n,vector<float>(n));
        for(int v=0;v<n;v++) {
            for(int w=0;w<words;w++) {
                Word x=possible[v][w];
                while(x) { candidates[v].push_back(64*w+__builtin_ctzll(x)); x&=x-1; }
                x=known[v][w];
                while(x) { neighbors[v].push_back(64*w+__builtin_ctzll(x)); x&=x-1; }
            }
            auto &c=candidates[v];
            if(c.empty()) continue;
            int bits=__builtin_popcountll(need[v]);
            int mini=bits;
            for(int u:c) mini=min(mini,__builtin_popcountll(need[v]&~sent[u]));
            double sum=0;
            for(int u:c) {
                int missed=__builtin_popcountll(need[v]&~sent[u])-mini;
                sum+=(prior[v][u]=pow(0.56,missed));
            }
            double deg=(bits ? 1.12+0.46*mini : 0.38);
            if(neighbors[v].empty()) deg=max(1.1,deg);
            for(int u:c) prior[v][u]=min(0.96,prior[v][u]*deg/sum);
        }
        // For each potential transmitter, keep its effect on every listening lamp.
        constexpr int samples=48;
        struct Effect {
            int vertex;
            double weight;
            Word scenarios;
        };
        vector<vector<Effect>> effects(n);
        vector<char> modeled(n);
        vector<int> active;
        vector<vector<Word>> flags(n);
        for(int v=0;v<n;v++) if(!candidates[v].empty()) {
            active.push_back(v);
            auto& c=candidates[v];
            int k=c.size();
            flags[v].resize(k);
            if(k>95) continue;
            struct Subset {int a,b,c; double cumulative;};
            vector<Subset> feasible;
            double total=0;
            auto include=[&](int i,int j,int l,double mass) {
                total+=mass; feasible.push_back({i,j,l,total});
            };
            vector<double> odds(k);
            vector<int> root(k);
            for(int i=0;i<k;i++) {
                double p=prior[c[i]][v];
                odds[i]=min(6.,p/(1.-p)+0.35/k);
                root[i]=forest.root(c[i]);
            }
            bool mandatory=neighbors[v].empty();
            if(!need[v] && !mandatory) include(-1,-1,-1,1);
            for(int i=0;i<k;i++) {
                Word rem=need[v]&~sent[c[i]];
                if(!rem) include(i,-1,-1,odds[i]);
                for(int j=0;j<i;j++) if(root[i]!=root[j]) {
                    Word missing=rem&~sent[c[j]];
                    double mass=odds[i]*odds[j];
                    if(!missing) include(i,j,-1,mass);
                    if(k<=29) for(int l=0;l<j;l++)
                        if(root[l]!=root[i] && root[l]!=root[j] && !(missing&~sent[c[l]]))
                            include(i,j,l,mass*odds[l]);
                }
            }
            if(feasible.empty()) continue;
            modeled[v]=true;
            int pos=0;
            for(int t=0;t<samples;t++) {
                double pick=total*(t+generate_canonical<double,32>(rng))/samples;
                while(pos+1<(int)feasible.size() && feasible[pos].cumulative<pick) ++pos;
                auto h=feasible[pos];
                for(int i:{h.a,h.b,h.c}) if(i>=0) flags[v][i]|=Word(1)<<t;
            }
        }
        for(int v:active) for(int i=0;i<(int)candidates[v].size();i++) {
            int u=candidates[v][i];
            double p=max(prior[u][v],prior[v][u]);
            effects[u].push_back({v,-log1p(-min(0.97,p)),flags[v][i]});
        }
        // Balance informative lamps, rewarding tests that isolate an edge.
        vector<vector<double>> bonus(n);
        for(int v:active) {
            int k=countPoss[v];
            bonus[v].resize(k+1);
            for(int i=1;i<=k;i++) bonus[v][i]=0.25*double(i)/k + (i==1 ? 0.18 : 0);
        }
        vector<double> heat(n);
        vector<int> masks(n),on(n),chosen(n),best(n);
        vector<array<short,samples>> counters(n);
        vector<int> lit(n);
        auto value=[&](int v) {
            if(masks[v] || !on[v]) return 0.0;
            double z=exp(-heat[v]);
            if(modeled[v]) z=0.15*z+0.85*(1.-double(lit[v])/samples);
            return 4*z*(1-z)+bonus[v][on[v]]*(0.35+z);
        };
        auto flip=[&](int u,int sign) {
            for(auto [v,w,f]:effects[u]) {
                heat[v]+=sign*w; on[v]+=sign;
                while(f) {
                    int i=__builtin_ctzll(f); f&=f-1;
                    auto& c=counters[v][i];
                    if(sign>0) lit[v]+=(c++==0);
                    else lit[v]-=(--c==0);
                }
            }
            for(int v:neighbors[u]) masks[v]+=sign;
            chosen[u]=(sign==1);
        };
        double bestScore=-1;
        int rounds=remaining>15000 ? 2 : 6;
        for(int rep=0;rep<rounds;rep++) {
            fill(counters.begin(),counters.end(),array<short,samples>{});
            fill(lit.begin(),lit.end(),0);
            fill(heat.begin(),heat.end(),0); fill(on.begin(),on.end(),0);
            fill(masks.begin(),masks.end(),0); fill(chosen.begin(),chosen.end(),0);
            for(int u:active) if(rng()%100<35) flip(u,1);
            for(int pass=0;pass<4;pass++) {
                shuffle(active.begin(),active.end(),rng);
                bool changed=false;
                for(int u:active) {
                    double delta=0;
                    for(auto [v,w,f]:effects[u]) delta-=value(v);
                    for(int v:neighbors[u]) delta-=value(v);
                    int sign=chosen[u] ? -1 : 1;
                    flip(u,sign);
                    for(auto [v,w,f]:effects[u]) delta+=value(v);
                    for(int v:neighbors[u]) delta+=value(v);
                    if(delta>1e-9) changed=true;
                    else flip(u,-sign);
                }
                if(!changed) break;
            }
            double s=0;
            for(int v:active) s+=value(v);
            if(s>bestScore) { bestScore=s; best=chosen; }
        }
        return best;
    }

    struct Search {
        int k;
        vector<Edge> ends, links;
        vector<vector<int>> clauses, solutions;
        int budget;
        bool exhausted=false;

        // State 1 includes an edge, -1 excludes it, and 0 leaves it undecided.
        void visit(vector<signed char> state) {
            if(exhausted) return;
            if(--budget<0 || solutions.size()>=1024) { exhausted=true; return; }
            vector<int> branch;
            for(;;) {
                DSU fixed(k);
                int added=0;
                for(int i=0;i<(int)links.size();i++) if(state[i]==1) {
                    if(!fixed.join(links[i].first,links[i].second)) return;
                    ++added;
                }
                vector<vector<int>> cuts(k);
                for(int i=0;i<(int)links.size();i++) if(!state[i]) {
                    int x=fixed.root(links[i].first),y=fixed.root(links[i].second);
                    if(x==y) state[i]=-1;
                    else { cuts[x].push_back(i); cuts[y].push_back(i); }
                }
                bool changed=false;
                branch.clear();
                auto require=[&](const vector<int>& choices) {
                    if(choices.empty()) return false;
                    if(choices.size()==1) { state[choices[0]]=1; changed=true; }
                    else if(branch.empty() || choices.size()<branch.size()) branch=choices;
                    return true;
                };
                if(added<k-1) for(int j=0;j<k;j++) if(fixed.p[j]<0 && !require(cuts[j])) return;
                vector<int> open;
                for(const auto& c:clauses) {
                    open.clear();
                    bool found=false;
                    for(int i:c) {
                        if(state[i]==1) { found=true; break; }
                        if(!state[i]) open.push_back(i);
                    }
                    if(!found && !require(open)) return;
                }
                if(changed) continue;
                if(added==k-1) {
                    vector<int> sol;
                    for(int i=0;i<(int)links.size();i++) if(state[i]==1) sol.push_back(i);
                    solutions.push_back(move(sol));
                    return;
                }
                break;
            }
            for(int i:branch) {
                state[i]=1;
                visit(state);
                if(exhausted) return;
                state[i]=-1;
            }
        }
    };

    // Select a radius-one query by comparing the complete lamp replies of
    // the remaining trees. Fixed edges can mask the same lamps in every tree.
    void finish(Search& s) {
        vector<int> active;
        vector<int> idx(n,-1);
        for(auto [a,b]:s.ends) for(int v:{a,b}) if(idx[v]<0) {
            idx[v]=active.size(); active.push_back(v);
        }
        int a=active.size();
        vector<vector<int>> fixedAdj(a);
        for(int i=0;i<a;i++) for(int j=0;j<a;j++) if(has(known[active[i]],active[j])) fixedAdj[i].push_back(j);
        vector<Edge> local=s.ends;
        for(auto &[x,y]:local) { x=idx[x]; y=idx[y]; }
        vector<Word> salt(a);
        for(Word& x:salt) x=(Word(rng())<<32)|rng();
        vector<int> live(s.solutions.size());
        iota(live.begin(),live.end(),0);
        while(live.size()>1) {
            vector<int> choice(a),best;
            vector<Word> lamps(a);
            long long bestCost=LLONG_MAX;
            int tries=min(1600,max(100,240000/(int)live.size()));
            for(int t=0;t<tries+a;t++) {
                if(t<a) { fill(choice.begin(),choice.end(),0); choice[t]=1; }
                else {
                    int density=15+rng()%60;
                    for(int &v:choice) v=(int)(rng()%100)<density;
                }
                fill(lamps.begin(),lamps.end(),0);
                for(int i=0;i<a;i++) if(choice[i]) for(int j:fixedAdj[i]) lamps[j]=1;
                unordered_map<Word,int> sizes;
                long long cost=0;
                Word tag=1;
                for(int id:live) {
                    ++tag;
                    Word h=0;
                    auto light=[&](int v) {
                        if(lamps[v]!=1 && lamps[v]!=tag) { lamps[v]=tag; h^=salt[v]; }
                    };
                    for(int e:s.solutions[id]) {
                        auto [x,y]=local[e];
                        if(choice[x]) light(y);
                        if(choice[y]) light(x);
                    }
                    cost+=sizes[h]++;
                }
                if(cost<bestCost) { bestCost=cost; best=choice; }
                if(!cost) break;
            }
            // Single-source queries distinguish any two different edge sets.
            long long hypotheses=live.size();
            if(bestCost==hypotheses*(hypotheses-1)/2) {
                fill(best.begin(),best.end(),0);
                for(int e:s.solutions[live[0]]) if(!binary_search(s.solutions[live[1]].begin(),s.solutions[live[1]].end(),e)) {
                    best[local[e].first]=1; break;
                }
            }
            vector<int> d(n);
            for(int i=0;i<a;i++) d[active[i]]=best[i];
            auto radiusTwoReply=[&](int id,const vector<int>& q) {
                vector<int> two(n);
                string lit(n,'0');
                auto direct=[&](Edge e) {
                    auto [u,v]=e;
                    if(q[u]) lit[v]='1';
                    if(q[v]) lit[u]='1';
                    two[u]+=(q[v]==2); two[v]+=(q[u]==2);
                };
                auto twoSteps=[&](Edge e) {
                    auto [u,v]=e;
                    if(two[u]>(q[v]==2)) lit[v]='1';
                    if(two[v]>(q[u]==2)) lit[u]='1';
                };
                for(auto e:edges) direct(e);
                for(int e:s.solutions[id]) direct(s.ends[e]);
                for(auto e:edges) twoSteps(e);
                for(int e:s.solutions[id]) twoSteps(s.ends[e]);
                return lit;
            };
            bool useTwo=false;
            int attempts=min(180,3000000/(n*(int)live.size()));
            for(int t=0;t<attempts && bestCost;t++) {
                vector<int> q(n);
                int density=8+rng()%55;
                for(int v:active) if((int)(rng()%100)<density) q[v]=1+(rng()%2);
                unordered_map<string,int> sizes;
                long long cost=0;
                for(int id:live) {
                    cost+=sizes[radiusTwoReply(id,q)]++;
                    if(cost>=bestCost) break;
                }
                if(cost<bestCost) {bestCost=cost; d=move(q); useTwo=true;}
            }
            if(useTwo) {
                string reply=ask(d);
                live.erase(remove_if(live.begin(),live.end(),[&](int id){return radiusTwoReply(id,d)!=reply;}),live.end());
                if(live.empty()) exit(0);
                continue;
            }
            string reply=ask(d);
            vector<int> base(a);
            for(int i=0;i<a;i++) if(best[i]) for(int j:fixedAdj[i]) base[j]=1;
            vector<int> next;
            for(int id:live) {
                vector<int> lit=base;
                for(int e:s.solutions[id]) {
                    auto [x,y]=local[e];
                    if(best[x]) lit[y]=1;
                    if(best[y]) lit[x]=1;
                }
                bool match=true;
                for(int i=0;i<a && match;i++) match=(lit[i]==reply[active[i]]-'0');
                if(match) next.push_back(id);
            }
            if(next.empty()) exit(0);
            live.swap(next);
        }
        for(int e:s.solutions[live[0]]) add(s.ends[e].first,s.ends[e].second);
    }

    bool exact() {
        if(remaining>155 || n-1-(int)edges.size()>40) return false;
        Search s;
        s.budget=1600;
        vector<int> label(n,-1);
        s.k=0;
        for(int i=0;i<n;i++) if(forest.root(i)==i) label[i]=s.k++;
        vector<vector<pair<int,int>>> adj(n);
        for(int u=0;u<n;u++) for(int w=u/64;w<words;w++) {
            Word x=possible[u][w];
            while(x) {
                int v=64*w+__builtin_ctzll(x); x&=x-1;
                if(v<=u) continue;
                int id=s.ends.size();
                s.ends.emplace_back(u,v);
                s.links.emplace_back(label[forest.root(u)],label[forest.root(v)]);
                adj[u].emplace_back(v,id); adj[v].emplace_back(u,id);
            }
        }
        for(int v=0;v<n;v++) if(!adj[v].empty()) for(const auto& o:history) {
            if(o.lamps[v]=='0' || intersects(known[v],o.sources)) continue;
            vector<int> c;
            for(auto [u,id]:adj[v]) if(has(o.sources,u)) c.push_back(id);
            s.clauses.push_back(move(c));
        }
        sort(s.clauses.begin(),s.clauses.end());
        s.clauses.erase(unique(s.clauses.begin(),s.clauses.end()),s.clauses.end());
        s.visit(vector<signed char>(s.ends.size()));
        if(s.exhausted || s.solutions.empty()) return false;
        finish(s);
        return true;
    }

    static vector<int> labels(int size, int ones) {
        vector<int> labels;
        for(int i=0;i<1<<size;i++) if(__builtin_popcount((unsigned)i)==ones) labels.push_back(i);
        return labels;
    }

    string probe(const vector<int>& d) {
        observe(d);
        string lamps=history.back().lamps;
        deduce();
        int lit=count(lamps.begin(),lamps.end(),'1');
        if(lit<=min(3,n/12) && accumulate(d.begin(),d.end(),0)>1) {
            vector<int> singleton(n);
            for(int i=0;i<n && (int)edges.size()<n-1;i++) if(lamps[i]=='1' && countPoss[i]) {
                singleton[i]=1; observe(singleton); deduce(); singleton[i]=0;
            }
        }
        return lamps;
    }

    void prefix() {
        // Disjoint initial source sets distinguish vertices with neighbors of
        // several colors. Keep the two probes inside the ordinary binary code.
        vector<vector<int>> bucket(3);
        for(;;) {
            for(auto& b:bucket) b.clear();
            for(int m:labels(length,weight)) if((m&3)!=3) bucket[m&3].push_back(m);
            if((int)(bucket[0].size()+2*bucket[1].size())>=n) break;
            if(weight<length/2) ++weight;
            else { ++length; weight=length/2; }
        }
        int part=min((int)bucket[1].size(),max(n/3,(n-(int)bucket[0].size()+1)/2));
        vector<int> order(n),color(n),hits(n),d(n);
        iota(order.begin(),order.end(),0); shuffle(order.begin(),order.end(),rng);
        for(auto& b:bucket) shuffle(b.begin(),b.end(),rng);
        for(int j=0;j<n;j++) {
            int c=j<part ? 1 : j<2*part ? 2 : 0;
            int i=order[j]; color[i]=c; code[i]=bucket[c].back(); bucket[c].pop_back();
        }
        for(int c:{1,2}) {
            for(int i=0;i<n;i++) d[i]=color[i]==c;
            string lamps=probe(d);
            if((int)edges.size()==n-1) return;
            for(int i=0;i<n;i++) hits[i]+=lamps[i]-'0';
        }
        step=2;
        // With degree two the probability of missing both independent colors
        // is approximately the square of their omitted fraction. A different
        // profile favors probing the third color to locate branching vertices.
        double outside=1.-2.*part/n;
        int shared=count(hits.begin(),hits.end(),2);
        if(shared && shared<=3) {
            for(int i=0;i<n;i++) if(hits[i]==2 && countPoss[i]) {
                fill(d.begin(),d.end(),0); d[i]=1; observe(d); deduce();
                if((int)edges.size()==n-1) return;
            }
        }
        if(abs(double(count(hits.begin(),hits.end(),0))/n-outside*outside)<0.036) return;

        for(int i=0;i<n;i++) d[i]=color[i]==0;
        string lamps=probe(d);
        if((int)edges.size()==n-1) return;
        for(int i=0;i<n;i++) hits[i]+=lamps[i]-'0';
        if(count(hits.begin(),hits.end(),3)<=max(1,n/200)) return;
        for(int i=0;i<n;i++) d[i]=hits[i]==1;
        lamps=probe(d);
        if((int)edges.size()==n-1) return;

        // Among these sources, unlit vertices only have neighbors in the
        // multi-color set. Their incident edges can be learned from that set.
        array<vector<int>,3> high,low;
        int demand=1;
        for(int i:order) if(countPoss[i]) {
            if(hits[i]>1) high[color[i]].push_back(i);
            else if(lamps[i]=='1') low[color[i]].push_back(i);
        }
        for(int c=0;c<3;c++) demand=max({demand,(int)high[c].size(),(int)low[c].size()});
        step=0;
        length=1;
        while((int)labels(length,(length+1)/2).size()<demand) ++length;
        weight=1;
        while((int)labels(length,weight).size()<demand) ++weight;
        auto pool=labels(length,weight);
        fill(code.begin(),code.end(),0);
        for(int c=0;c<3;c++) {
            shuffle(pool.begin(),pool.end(),rng);
            int j=0;
            for(int i:high[c]) code[i]=pool[j++];
            for(int i:low[c]) {code[i]=pool[j%pool.size()];++j;}
        }
    }

    void run() {
        if(n>=36) prefix();
        int stale=0;
        vector<int> priority;
        bool complement=false;
        while((int)edges.size()<n-1) {
            vector<int> d(n);
            while(!priority.empty() && !countPoss[priority.back()]) priority.pop_back();
            if(!priority.empty()) {
                d[priority.back()]=1; priority.pop_back();
            } else if(complement) {
                for(int i=0;i<n;i++) d[i]=!has(history.back().sources,i) && countPoss[i];
                complement=false;
            } else if(!history.empty() && exact()) break;
            else if(step<length && (step==0 || remaining>max(10,2*(n-1-(int)edges.size())))) {
                for(int v=0;v<n;v++) if(!step || countPoss[v]) d[v]=code[v]>>step&1;
                ++step;
            } else if(stale>=1 || history.size()>=45) {
                // Select transmitters whose possible and confirmed neighbors
                // cannot mask one another. This resolves every selected row.
                vector<int> order(n);
                iota(order.begin(),order.end(),0);
                sort(order.begin(),order.end(),[&](int a,int b){return countPoss[a]>countPoss[b];});
                Bits covered{},wanted{};
                for(int u:order) {
                    if(!countPoss[u]) break;
                    if(intersects(possible[u],covered) || intersects(known[u],wanted)) continue;
                    d[u]=1;
                    for(int w=0;w<words;w++) {
                        wanted[w]|=possible[u][w];
                        covered[w]|=possible[u][w]|known[u][w];
                    }
                }
            } else { step=length; d=adaptive(); }
            int before=remaining;
            observe(d);
            deduce();
            if(n>=12 && (int)edges.size()<n-1) {
                int lit=count(history.back().lamps.begin(),history.back().lamps.end(),'1');
                if(lit>0 && lit<=min(4,n/10) && accumulate(d.begin(),d.end(),0)>1) {
                    priority.clear();
                    for(int v=0;v<n;v++) if(history.back().lamps[v]=='1' && countPoss[v]) priority.push_back(v);
                    sort(priority.begin(),priority.end(),[&](int a,int b){return countPoss[a]<countPoss[b];});
                }
                if(history.size()==1 && lit>n*0.92) complement=true;
            }
            stale=(remaining==before && step>=length) ? stale+1 : 0;
        }
    }
};

// For small instances, keep the whole family of labeled trees. A query
// partitions that family according to its exact (including self-exclusion) reply.
struct SmallTree {
    array<array<unsigned char,7>,7> ball{};
    vector<Edge> edges;
};
vector<Edge> smallSolve(int n) {
    int num=1;
    for(int i=2;i<n;i++) num*=n;
    vector<SmallTree> trees(num);
    for(int seq=0;seq<num;seq++) {
        array<int,7> degree{},code{},adj{};
        degree.fill(1);
        int z=seq;
        for(int i=0;i<n-2;i++) { code[i]=z%n; z/=n; ++degree[code[i]]; }
        auto link=[&](int x,int y) {
            adj[x]|=1<<y; adj[y]|=1<<x;
            trees[seq].edges.emplace_back(x,y);
        };
        for(int i=0;i<n-2;i++) {
            int j=0; while(degree[j]!=1) ++j;
            link(j,code[i]); --degree[j]; --degree[code[i]];
        }
        int x=0,y=n-1;
        while(degree[x]!=1) ++x;
        while(degree[y]!=1) --y;
        link(x,y);
        for(int i=0;i<n;i++) {
            int mask=1<<i;
            for(int r=1;r<n;r++) {
                int next=mask;
                for(int j=0;j<n;j++) if(mask>>j&1) next|=adj[j];
                mask=next;
                trees[seq].ball[i][r]=mask^(1<<i);
            }
        }
    }
    vector<int> live(num); iota(live.begin(),live.end(),0);
    mt19937 rng(471894);
    while(live.size()>1) {
        vector<int> best,d(n);
        long long bestCost=LLONG_MAX;
        auto reply=[&](int id,const vector<int>& query) {
            int mask=0;
            for(int i=0;i<n;i++) mask|=trees[id].ball[i][query[i]];
            return mask;
        };
        auto evaluate=[&]() {
            array<int,128> freq{};
            long long cost=0;
            for(int id:live) {
                cost+=freq[reply(id,d)]++;
                if(cost>=bestCost) return;
            }
            bestCost=cost; best=d;
        };
        if(live.size()==trees.size()) {
            auto order=[&](auto&& self,int pos,int lo)->void {
                if(pos==n) {evaluate();return;}
                for(int i=lo;i<n-1;i++) { d[pos]=i; self(self,pos+1,i); }
            };
            order(order,0,0);
        } else {
            for(int mask=1;mask<1<<n && bestCost;mask++) {
                for(int i=0;i<n;i++) d[i]=mask>>i&1;
                evaluate();
            }
            if(n<=5 || (n==6 && live.size()<=50) || (n==7 && live.size()<=16)) {
                auto enumerate=[&](auto&& self,int pos)->void {
                    if(!bestCost) return;
                    if(pos==n) {evaluate();return;}
                    for(int i=0;i<n-1;i++) {d[pos]=i; self(self,pos+1);}
                };
                enumerate(enumerate,0);
            } else for(int t=0;t<1800 && bestCost;t++) {
                for(int &x:d) { int u=rng()%100; x=(u<57 ? 0 : u<83 ? 1 : u<96 ? 2 : 3); }
                evaluate();
            }
        }
        string s=ask(best);
        int mask=0;
        for(int i=0;i<n;i++) if(s[i]=='1') mask|=1<<i;
        live.erase(remove_if(live.begin(),live.end(),[&](int id){return reply(id,best)!=mask;}),live.end());
        if(live.empty()) exit(0);
    }
    return trees[live[0]].edges;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if(!(cin>>n) || n<2 || n>1000) return 0;
    vector<Edge> edges;
    if(n<=7) edges=smallSolve(n);
    else {
        Solver solver(n);
        solver.run();
        edges=move(solver.edges);
    }
    cout << '!' << endl;
    for(auto [u,v]:edges) cout << u+1 << ' ' << v+1 << endl;
}
