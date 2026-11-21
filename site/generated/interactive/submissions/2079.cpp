#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p;
    explicit DSU(int n = 0) : p(n, -1) {}
    int root(int x) {
        while (p[x] >= 0) {
            if (p[p[x]] >= 0) p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }
    bool join(int a, int b) {
        a = root(a); b = root(b);
        if (a == b) return false;
        if (p[a] > p[b]) swap(a, b);
        p[a] += p[b]; p[b] = a;
        return true;
    }
};

struct Edge { int u, v; };
struct Group { vector<int> edges; int count; };

class Solver {
    int n, m;
    vector<Edge> edge;
    DSU equal;
    vector<int> value;
    vector<int> reference;
    vector<pair<vector<int>, int>> observations;
    map<vector<int>, int> cache;
    mt19937 rng{2747311};
    int changes = 0;

    vector<int> possibleStars;
    vector<vector<int>> predictions;
    vector<vector<char>> predictedEdges;
    int measurements=0;
    bool checkingPrediction=false;
    void checkPredictions(const vector<int>& tree,int answer) {
        ++measurements;
        if(!possibleStars.empty()) {
            vector<int> degree(n);
            for(int e:tree) {++degree[edge[e].u];++degree[edge[e].v];}
            erase_if(possibleStars,[&](int v){return degree[v]!=answer;});
        }
        for(int j=0;j<(int)predictions.size();++j) if(!predictions[j].empty()) {
            int overlap=0;
            for(int e:tree) overlap+=predictedEdges[j][e];
            if(overlap!=answer) predictions[j].clear();
        }
        if(checkingPrediction || measurements<3) return;
        checkingPrediction=true;
        if(possibleStars.size()==1) {
            int v=possibleStars[0];
            vector<int> star;
            for(int e=0;e<m;++e) if(edge[e].u==v || edge[e].v==v) star.push_back(e);
            possibleStars.clear();
            if(ask(star)==n-1) finish(star);
        }
        for(auto& candidate:predictions) if(!candidate.empty()) {
            vector<int> probe;
            probe.swap(candidate);
            if(ask(probe)==n-1) finish(probe);
        }
        checkingPrediction=false;
    }
    int status(int e) { return value[equal.root(e)]; }
    void mark(int e, int v) {
        int &s = value[equal.root(e)];
        if (s < 0) { s = v; ++changes; }
    }
    void same(int a, int b) {
        int s = max(status(a), status(b));
        if (equal.join(a, b)) {
            value[equal.root(a)] = s;
            ++changes;
        }
    }
    [[noreturn]] void finish(const vector<int>& tree) {
        cout << '!';
        for (int e : tree) cout << ' ' << e;
        cout << endl;
        exit(0);
    }
    int ask(const vector<int>& tree) {
        vector<int> key = tree;
        sort(key.begin(), key.end());
        auto found = cache.find(key);
        if (found != cache.end()) return found->second;
        int yes = 0, unknown = 0;
        for (int e : tree) {
            int s = status(e);
            yes += s == 1; unknown += s < 0;
        }
        if (!unknown) return yes;
        cout << '?';
        for (int e : tree) cout << ' ' << e;
        cout << endl;
        int answer;
        if (!(cin >> answer) || answer < 0 || answer > n-1) exit(0);
        if (answer == n-1) finish(tree);
        if (answer == yes || answer == yes + unknown)
            for (int e : tree) if (status(e) < 0) mark(e, answer != yes);
        cache.emplace(move(key), answer);
        checkPredictions(tree,answer);
        return answer;
    }
    bool add(DSU& d, int e) { return d.join(edge[e].u, edge[e].v); }

    // Contract royal edges and find bridges in the remaining multigraph.
    void topology() {
        DSU d(n);
        vector<int> royal;
        for (int i = 0; i < m; ++i) if (status(i) == 1) {
            royal.push_back(i); add(d, i);
        }
        if ((int)royal.size() == n-1) finish(royal);
        vector<vector<pair<int,int>>> adj(n);
        for (int i = 0; i < m; ++i) if (status(i) < 0) {
            int a = d.root(edge[i].u), b = d.root(edge[i].v);
            if (a == b) mark(i, 0);
            else { adj[a].emplace_back(b, i); adj[b].emplace_back(a, i); }
        }
        vector<int> tin(n), low(n);
        int timer = 0;
        auto dfs = [&](auto&& self, int x, int from) -> void {
            tin[x] = low[x] = ++timer;
            for (auto [y, e] : adj[x]) {
                if (e == from) continue;
                if (!tin[y]) {
                    self(self, y, e);
                    low[x] = min(low[x], low[y]);
                    if (low[y] > tin[x]) mark(e, 1);
                } else low[x] = min(low[x], tin[y]);
            }
        };
        dfs(dfs, d.root(0), -1);
    }
    vector<int> basis(const vector<int>& order, bool knownFirst = false) {
        DSU d(n);
        vector<int> t;
        if (knownFirst)
            for (int e : order) if (status(e) >= 0 && add(d,e)) t.push_back(e);
        for (int e : order) {
            if ((int)t.size() == n-1) break;
            if (add(d,e)) t.push_back(e);
        }
        return t;
    }

    // For small cycle spaces, directly separate the remaining spanning trees.
    void small(const vector<int>& fixed, const vector<int>& freeEdges) {
        int h = freeEdges.size(), need = n-1-fixed.size();
        if (h > 63) return;
        DSU start(n);
        for (int e : fixed) add(start,e);
        vector<uint64_t> trees;
        vector<int> parent(n), sz(n,1);
        for (int i=0;i<n;++i) parent[i]=start.root(i);
        auto root = [&](int v) { while (parent[v]!=v) v=parent[v]; return v; };
        int nodes=0;
        auto gen = [&](auto&& self, int i, int k, uint64_t mask) -> void {
            if (trees.size()>5000 || ++nodes>120000) return;
            if (!k) { trees.push_back(mask); return; }
            if (h-i<k) return;
            int e=freeEdges[i], a=root(edge[e].u), b=root(edge[e].v);
            if (a!=b) {
                if (sz[a]<sz[b]) swap(a,b);
                parent[b]=a; sz[a]+=sz[b];
                self(self,i+1,k-1,mask|(1ULL<<i));
                sz[a]-=sz[b]; parent[b]=b;
            }
            self(self,i+1,k,mask);
        };
        gen(gen,0,need,0);
        if (trees.size()>5000 || nodes>120000 || trees.empty()) return;
        auto expand = [&](uint64_t mask) {
            vector<int> t=fixed;
            while(mask) {
                t.push_back(freeEdges[countr_zero(mask)]);
                mask &= mask-1;
            }
            return t;
        };
        if(trees.size()<=16 && h>need+1) {
            // Exact decision-tree cost for uniformly weighted candidates.
            int count=trees.size(),all=(1<<count)-1;
            vector<vector<int>> bins(count,vector<int>(need+1));
            for(int p=0;p<count;++p) for(int q=0;q<count;++q)
                bins[p][popcount(trees[p]&trees[q])]|=1<<q;
            vector<int> cost(all+1,-1),action(all+1);
            auto optimize=[&](auto&& self,int mask)->int {
                if((mask&(mask-1))==0) return 0;
                if(cost[mask]>=0) return cost[mask];
                int best=INT_MAX;
                for(int p=0;p<count;++p) {
                    int sum=popcount((unsigned)mask);
                    for(int bin:bins[p]) {
                        int child=mask&bin;
                        if(child==mask) {sum=INT_MAX;break;}
                        sum+=self(self,child);
                        if(sum>=best) break;
                    }
                    if(sum<best) {best=sum;action[mask]=p;}
                }
                return cost[mask]=best;
            };
            optimize(optimize,all);
            while(all&(all-1)) {
                int p=action[all];
                int answer=ask(expand(trees[p]))-fixed.size();
                all&=bins[p][answer];
            }
            finish(expand(trees[countr_zero((unsigned)all)]));
        }
        vector<uint64_t> options=trees;
        while(options.size()>1) {
            vector<uint64_t> probes;
            if (trees.size()<=240) probes=trees;
            else {
                for (int j=0;j<100;++j) probes.push_back(trees[rng()%trees.size()]);
            }
            probes.push_back(options[0]);
            for (int j=0;j<min(24,(int)options.size());++j)
                probes.push_back(options[rng()%options.size()]);
            uint64_t best=options[0];
            long long score=LLONG_MAX;
            for (auto p : probes) {
                int hist[64]={};
                for (auto q : options) ++hist[popcount(p&q)];
                long long cost=0;
                for (int a=0;a<=need;++a) cost+=1LL*hist[a]*hist[a];
                if (cost<score || (cost==score && hist[need])) {
                    score=cost; best=p;
                }
            }
            int k=ask(expand(best))-(int)fixed.size();
            erase_if(options,[&](uint64_t x){return popcount(x&best)!=k;});
        }
        finish(expand(options.front()));
    }

    // Recover several bounded integer counts. Any subset of the coordinates
    // may be measured in one tree. Unary fields turn its predicted response
    // into a single population count for each candidate assignment.
    template<class Measure>
    vector<int> decode(const vector<int>& width, Measure measure) {
        int b=width.size();
        if (b==1) return {measure(1)};
        vector<uint64_t> field(b), cand{0};
        int shift=0;
        for (int i=0;i<b;++i) {
            field[i]=((1ULL<<width[i])-1)<<shift;
            int old=cand.size();
            for(int j=1;j<=width[i];++j) {
                uint64_t code=((1ULL<<j)-1)<<shift;
                for(int l=0;l<old;++l) cand.push_back(cand[l]|code);
            }
            shift+=width[i];
        }
        int full=(1<<b)-1;
        while(cand.size()>1) {
            vector<uint64_t> sample;
            if(cand.size()<=220) sample=cand;
            else {
                for(int i=0;i<220;++i) sample.push_back(cand[rng()%cand.size()]);
            }
            int bestMask=0;
            uint64_t bestFields=0;
            long long best=LLONG_MAX;
            auto consider = [&](int mask) {
                if(!mask) return;
                uint64_t fields=0;
                for(int bits=mask;bits;bits&=bits-1) fields |= field[countr_zero((unsigned)bits)];
                int hist[64]={};
                long long score=0;
                for(auto x:sample) { int &h=hist[popcount(x&fields)]; score+=2*h+1; ++h; }
                if(score<best) {best=score;bestMask=mask;bestFields=fields;}
            };
            if(b<=6) for(int mask=1;mask<=full;++mask) consider(mask);
            else {
                consider(full);
                for(int i=0;i<b;++i) consider(1<<i);
                for(int j=0;j<64;++j) {
                    int mask=rng()&full;
                    if(j&1) mask|=rng()&full;
                    consider(mask);
                }
            }
            if(best==static_cast<long long>(sample.size()*sample.size())) {
                uint64_t dif=cand.front()^cand.back();
                for(int i=0;i<b;++i) if(dif&field[i]) {
                    bestMask=1<<i; bestFields=field[i]; break;
                }
            }
            int result=measure(bestMask);
            erase_if(cand,[&](uint64_t x){return popcount(x&bestFields)!=result;});
        }
        vector<int> out(b);
        for(int i=0;i<b;++i) out[i]=popcount(cand[0]&field[i]);
        return out;
    }

    struct Digit {
        int radix, columnMask;
        vector<int> rows;
    };
    // Each column depends only on a subset S of the row-index bits. Its
    // alternating sum over S is a specified mixed-radix place value. Once
    // columns on supersets are removed, this sum reads out all digits on S.
    vector<Digit> integerCode(int rows,const vector<int>& widths) {
        vector<Digit> digits;
        vector<int> subsets(rows);
        iota(subsets.begin(),subsets.end(),0);
        sort(subsets.begin(),subsets.end(),[](int a,int b){
            return popcount((unsigned)a)>popcount((unsigned)b);
        });
        for(int s:subsets) {
            vector<int> even;
            for(int j=s;;j=(j-1)&s) {
                if(!(popcount((unsigned)j)&1)) even.push_back(j);
                if(!j) break;
            }
            shuffle(even.begin(),even.end(),rng);
            for(int scale=1;scale<=(int)even.size() && digits.size()<widths.size();) {
                int i=digits.size();
                vector<char> active(rows);
                for(int j=0;j<scale;++j) active[even[j]]=1;
                Digit digit{widths[i]+1,s,{}};
                for(int r=0;r<rows;++r) if(active[r&s]) digit.rows.push_back(r);
                digits.push_back(move(digit));
                scale*=widths[i]+1;
            }
        }
        return digits;
    }
    vector<int> readDigits(const vector<Digit>& digits,vector<int> readings) {
        vector<int> result(digits.size());
        for(int i=0;i<(int)digits.size();) {
            int s=digits[i].columnMask,number=0;
            for(int j=s;;j=(j-1)&s) {
                number+=(popcount((unsigned)j)&1)?-readings[j]:readings[j];
                if(!j) break;
            }
            do {
                const Digit& digit=digits[i];
                int val=number%digit.radix;
                result[i]=val;number/=digit.radix;
                for(int r:digit.rows) readings[r]-=val;
                ++i;
            } while(i<(int)digits.size() && digits[i].columnMask==s);
        }
        return result;
    }
    void useObservations() {
        for(auto& [tree,count]:observations) {
            int left=count, unknown=0;
            vector<int> classes;
            for(int e:tree) {
                int s=status(e);
                if(s==1) --left;
                if(s<0) {classes.push_back(equal.root(e));++unknown;}
            }
            sort(classes.begin(),classes.end());
            for(int i=0;i<unknown;) {
                int j=i+1;
                while(j<unknown && classes[i]==classes[j]) ++j;
                if(j-i>left) mark(classes[i],0);
                if(j-i>unknown-left) mark(classes[i],1);
                i=j;
            }
        }
    }

    // Restrict attention to a small union of sampled trees. Fitting their
    // observed sums suggests low-overlap trees to test. Every conclusion
    // about an edge still comes from an exact count.
    void learnReference(vector<int>& t,int& baseline,vector<int>& order,int fixed) {
        vector<int> pool, index(m,-1);
        auto extendPool=[&](const vector<int>& tree) {
            for(int e:tree) if(index[e]<0) {index[e]=pool.size();pool.push_back(e);}
        };
        vector<int> source=t;
        int poolSum=baseline;
        extendPool(t);
        vector<int> disjoint=order;
        stable_partition(disjoint.begin(),disjoint.end(),[&](int e){return index[e]<0;});
        vector<int> partner=basis(disjoint);
        int other=ask(partner);
        observations.emplace_back(partner,other);
        extendPool(partner);
        vector<int> mult(pool.size());
        for(int e:source) ++mult[index[e]];
        for(int e:partner) ++mult[index[e]];
        poolSum+=other;
        if(other<baseline) {baseline=other;t=partner;}
        if(baseline==fixed) return;
        const int limit=min(240,max(12,5*(baseline-fixed)+12));
        vector<vector<int>> rows;
        vector<int> scores;
        for(auto& [tree,k]:observations) {
            vector<int> row;
            for(int e:tree) if(index[e]>=0 && status(e)<0) row.push_back(index[e]);
            int known=0, missing=0;
            for(int e:tree) { known+=status(e)==1;missing+=index[e]<0 && status(e)<0; }
            if(!missing && !row.empty()) {rows.push_back(move(row));scores.push_back(k-known);}
        }
        double density=clamp(double(baseline-fixed)/max(1,n-1-fixed),0.001,0.9);
        vector<double> odds(pool.size(),density/(1-density)),cost(pool.size());
        vector<int> perm(pool.size());iota(perm.begin(),perm.end(),0);
        int stale=0;
        for(int step=0;step<limit && stale<max(14,limit/3);++step) {
            // Iterative proportional fitting of the edge probabilities.
            for(int pass=0;pass<3;++pass) for(int i=0;i<(int)rows.size();++i) {
                double sum=0,var=0;
                for(int j:rows[i]) {
                    double p=odds[j]/(1+odds[j]);
                    sum+=p;var+=p*(1-p);
                }
                double adjust=exp(clamp((scores[i]-sum)/(0.001+var),-2.,2.));
                for(int j:rows[i]) odds[j]=clamp(odds[j]*adjust,1.e-9,1.e9);
            }
            double noise=step%3==0?0.01:step%3==1?1.:3.;
            for(int j:perm) {
                int s=status(pool[j]);
                bool maximize=step%5==3;
                double weight=odds[j]*exp(noise*(rng()/(1.+rng.max())-.5));
                cost[j]=maximize ? (s==0?0:s==1?-1.e12:-weight) : (s>=0?-1:weight);
            }
            sort(perm.begin(),perm.end(),[&](int a,int b){return cost[a]<cost[b];});
            DSU d(n);
            vector<int> tree,row;
            int known=0;
            for(int j:perm) if(add(d,pool[j])) {
                int e=pool[j];tree.push_back(e);
                if(status(e)<0) row.push_back(j); else known+=status(e);
                if((int)tree.size()==n-1) break;
            }
            int k=ask(tree);
            if(k==known || k==known+(int)row.size()) {t=move(tree);baseline=k;return;}
            rows.push_back(move(row));scores.push_back(k-known);
            if(step%5==3) {
                vector<int> rem=mult,complement;
                for(int e:tree) --rem[index[e]];
                int positives=poolSum-k;
                for(int j:perm) for(int a=0;a<rem[j];++a) {
                    int s=status(pool[j]);
                    if(s<0) complement.push_back(j);
                    else positives-=s;
                }
                if(!positives || positives==(int)complement.size()) {
                    for(int j:complement) mark(pool[j],positives!=0);
                    if(!complement.empty()) {
                        observations.emplace_back(tree,k);
                        return;
                    }
                } else if(positives<baseline-fixed) {
                    rows.push_back(move(complement));scores.push_back(positives);
                }
            }
            if(k<baseline) {baseline=k;t=tree;stale=0;}
            else ++stale;
            if(k<=fixed+2 || step%8==0) observations.emplace_back(move(tree),k);
        }
    }

    void calibrate(vector<int> order) {
        vector<int> t=basis(order);
        int baseline=ask(t);
        observations.emplace_back(t,baseline);
        auto unknowns = [&](const vector<int>& a) {
            int k=0; for(int e:a) k+=status(e)<0; return k;
        };
        int forced=0;
        for(int e:t) forced+=status(e)==1;
        // In dense graphs an entirely classified reference is often obtained
        // in just a few independent trials.
        if(unknowns(t) && (baseline-forced<=5 || m>12*n)) {
            int attempts=0;
            int trials=m>n*(n-1)/4 ? 55 : 12;
            for(;attempts<trials;++attempts) {
                shuffle(order.begin(),order.end(),rng);
                vector<int> alt=basis(order,true);
                if(!unknowns(alt)) {reference=move(alt);return;}
                int a=ask(alt);
                observations.emplace_back(alt,a);
                if(!unknowns(alt)) {reference=move(alt);return;}
                if(a<baseline) {t=alt;baseline=a;}
                if(attempts>=1 && baseline-forced>3) break;
                if(m<=2*n && attempts>=1) break;
            }
        }
        if(unknowns(t) && m>=5*n && baseline-forced<=n/7)
            learnReference(t,baseline,order,forced);
        vector<int> pos(m,-1);
        auto indexTree = [&](){
            fill(pos.begin(),pos.end(),-1);
            for(int i=0;i<n-1;++i) pos[t[i]]=i;
        };
        indexTree();
        for(;;) {
            useObservations();
            topology();
            vector<int> alt=basis(order,true);
            int unresolved=unknowns(t), other=unknowns(alt);
            if(!other) {reference=move(alt);return;}
            if(!unresolved) {reference=t;return;}
            if(other*3<unresolved*2) {
                t=move(alt);baseline=ask(t);
                observations.emplace_back(t,baseline);
                indexTree();
                continue;
            }
            // The last base edge on a chord's path under this ordering can
            // be exchanged for the chord. Distinct such exchanges commute:
            // each selected edge joins the same two Kruskal components.
            vector<int> ord(n-1),priority(n-1);
            iota(ord.begin(),ord.end(),0);
            shuffle(ord.begin(),ord.end(),rng);
            vector<char> onTree(m);
            for(int i=0;i<n-1;++i) {
                priority[ord[i]]=i+(status(t[ord[i]])<0?n:0);
                onTree[equal.root(t[i])]=1;
            }
            vector<vector<pair<int,int>>> adj(n);
            for(int i=0;i<n-1;++i) {
                auto [u,v]=edge[t[i]];
                adj[u].emplace_back(v,i); adj[v].emplace_back(u,i);
            }
            vector<vector<short>> last(n,vector<short>(n,-1));
            auto walk=[&](auto&& self,int s,int v,int p,int mx)->void {
                last[s][v]=mx;
                for(auto [w,i]:adj[v]) if(w!=p)
                    self(self,s,w,v,mx<0 || priority[i]>priority[mx]?i:mx);
            };
            for(int s=0;s<n;++s) walk(walk,s,s,-1,-1);
            vector<int> choice(n-1,-1), merit(n-1,-1);
            for(int e:order) if(pos[e]<0) {
                int i=last[edge[e].u][edge[e].v], f=t[i];
                int a=status(e),b=status(f);
                if(equal.root(e)==equal.root(f) || (a>=0 && b>=0)) continue;
                if(b>=0 && !onTree[equal.root(e)]) continue;
                int worth= (a>=0 || b>=0 ? 4 : onTree[equal.root(e)] ? 3 : 1);
                if(worth>merit[i]) {merit[i]=worth;choice[i]=e;}
            }
            vector<int> pairs;
            for(int i:ord) if(choice[i]>=0) pairs.push_back(i);
            if(pairs.empty()) {
                // Inspect an entire fundamental cycle when no ordered pair
                // gives new information. A monochromatic cycle must be zero.
                bool progress=false;
                for(int e:order) if(pos[e]<0 && !progress) {
                    int s=edge[e].u,v=edge[e].v;
                    if(status(t[last[s][v]])>=0) continue;
                    vector<int> par(n,-1), via(n,-1), q{s};
                    par[s]=s;
                    for(int j=0;par[v]<0;++j) for(auto [w,i]:adj[q[j]]) if(par[w]<0) {
                        par[w]=q[j];via[w]=i;q.push_back(w);
                    }
                    vector<int> path;
                    bool relevant=false;
                    for(int w=v;w!=s;w=par[w]) {
                        path.push_back(via[w]);relevant |= status(t[via[w]])<0;
                    }
                    if(!relevant) continue;
                    for(int i:path) if(equal.root(t[i])!=equal.root(e) &&
                                           (status(e)<0 || status(t[i])<0)) {
                        choice[i]=e;pairs.push_back(i);progress=true;break;
                    }
                    if(!progress) {mark(e,0);progress=true;}
                }
                if(pairs.empty()) continue;
            }
            int first=0;
            while((int)pairs.size()-first>=36) {
                vector<int> widths,low;
                for(int j=first;j<(int)pairs.size();++j) {
                    int i=pairs[j],a=status(choice[i]),b=status(t[i]);
                    widths.push_back((a<0)+(b<0));
                    low.push_back((a<0?0:a)-(b<0?1:b));
                }
                // Previous batches may have determined further coordinates.
                bool changed=false;
                for(int j=(int)pairs.size()-1;j>=first;--j) {
                    int i=pairs[j];
                    if(!widths[j-first] || equal.root(choice[i])==equal.root(t[i])) {
                        pairs.erase(pairs.begin()+j);changed=true;
                    }
                }
                if(changed) continue;
                vector<Digit> digits;
                int rows=0;
                for(int r=128;r>=16;r/=2) {
                    auto plan=integerCode(r,widths);
                    if((int)plan.size()>=r*(r>=64?1.65:1.55)) {
                        rows=r;digits=move(plan);break;
                    }
                }
                if(!rows) break;
                vector<vector<int>> queries(rows,t);
                vector<int> offsets(rows,baseline),readings(rows);
                for(int j=0;j<(int)digits.size();++j) {
                    int i=pairs[first+j];
                    for(int r:digits[j].rows) {queries[r][i]=choice[i];offsets[r]+=low[j];}
                }
                for(int r=0;r<rows;++r) readings[r]=ask(queries[r])-offsets[r];
                vector<int> values=readDigits(digits,move(readings));
                for(int j=0;j<(int)digits.size();++j) {
                    int i=pairs[first+j],d=values[j]+low[j];
                    if(!d) same(choice[i],t[i]);
                    else {mark(choice[i],d>0);mark(t[i],d<0);}
                }
                first+=digits.size();
            }
            for(int start=first;start<(int)pairs.size();) {
                vector<int> idx,chords,lower,width;
                int states=1;
                while(start<(int)pairs.size() && idx.size()<15) {
                    int i=pairs[start],e=choice[i],f=t[i];
                    int a=status(e),b=status(f);
                    if((a>=0 && b>=0) || equal.root(e)==equal.root(f)) {++start;continue;}
                    int lo=(a<0?0:a)-(b<0?1:b);
                    int hi=(a<0?1:a)-(b<0?0:b);
                    if(states*(hi-lo+1)>32768) break;
                    ++start;states*=hi-lo+1;
                    idx.push_back(i);chords.push_back(e);lower.push_back(lo);width.push_back(hi-lo);
                }
                if(idx.empty()) continue;
                auto result=decode(width,[&](int mask){
                    vector<int> query=t;
                    int offset=baseline;
                    for(int j=0;j<(int)idx.size();++j) if(mask>>j&1) {
                        query[idx[j]]=chords[j];offset+=lower[j];
                    }
                    return ask(query)-offset;
                });
                for(int j=0;j<(int)idx.size();++j) {
                    int delta=result[j]+lower[j],e=chords[j],f=t[idx[j]];
                    if(!delta) same(e,f);
                    else {mark(e,delta>0);mark(f,delta<0);}
                }
            }
        }
    }

    int countForest(const vector<int>& forest) {
        DSU d(n);
        vector<int> query=forest;
        for(int e:forest) add(d,e);
        int filler=0;
        for(int e:reference) if(add(d,e)) {
            query.push_back(e);filler+=status(e);
        }
        return ask(query)-filler;
    }
    void simplify(Group& g) {
        int write=0;
        for(int e:g.edges) {
            int s=status(e);
            if(s<0) g.edges[write++]=e;
            else g.count-=s;
        }
        g.edges.resize(write);
        if(g.count==0 || g.count==write) {
            for(int e:g.edges) mark(e,g.count!=0);
            g.edges.clear(); g.count=0;
        }
    }
    // A feasible tree is a common basis of the graphic matroid and the
    // partition matroid defined by the known group sizes. Any other feasible
    // tree differs along cycles of their basis-exchange graph.
    void inferExchanges(const vector<Group>& groups) {
        int colors=groups.size(), total=0;
        for(const Group& g:groups) total+=g.edges.size();
        if(total>8*n || colors<2) return;
        DSU fixed(n);
        for(int e=0;e<m;++e) if(status(e)==1) add(fixed,e);
        vector<int> u,v,color,ids,capacity;
        vector<vector<int>> members(colors);
        for(int c=0;c<colors;++c) {
            capacity.push_back(groups[c].count);
            for(int e:groups[c].edges) {
                members[c].push_back(ids.size());
                ids.push_back(e);color.push_back(c);
                u.push_back(fixed.root(edge[e].u));v.push_back(fixed.root(edge[e].v));
            }
        }
        vector<char> chosen(total);
        DSU greedy(n);
        for(int i=0;i<total;++i) if(capacity[color[i]] && greedy.join(u[i],v[i])) {
            chosen[i]=1;--capacity[color[i]];
        }
        vector<vector<pair<int,int>>> adj(n);
        vector<int> parent(n),depth(n),via(n),component(n);
        auto rootForest=[&]() {
            for(auto& a:adj) a.clear();
            for(int i=0;i<total;++i) if(chosen[i]) {
                adj[u[i]].emplace_back(v[i],i);adj[v[i]].emplace_back(u[i],i);
            }
            fill(parent.begin(),parent.end(),-1);
            for(int i=0;i<n;++i) if(parent[i]<0) {
                parent[i]=i;component[i]=i;depth[i]=0;
                vector<int> q{i};
                for(int j=0;j<(int)q.size();++j) {
                    int a=q[j];
                    for(auto [b,e]:adj[a]) if(parent[b]<0) {
                        parent[b]=a;depth[b]=depth[a]+1;via[b]=e;
                        component[b]=i;q.push_back(b);
                    }
                }
            }
        };
        auto path=[&](int i,auto visit) {
            int a=u[i],b=v[i];
            while(a!=b) {
                if(depth[a]<depth[b]) swap(a,b);
                visit(via[a]);a=parent[a];
            }
        };
        for(;;) {
            rootForest();
            vector<int> q,from(colors,-2),removed(colors,-1);
            for(int c=0;c<colors;++c) if(capacity[c]) {q.push_back(c);from[c]=-1;}
            if(q.empty()) break;
            int end=-1;
            for(int j=0;j<(int)q.size() && end<0;++j) {
                int c=q[j];
                for(int i:members[c]) if(!chosen[i]) {
                    if(component[u[i]]!=component[v[i]]) {end=i;break;}
                    path(i,[&](int f){
                        int next=color[f];
                        if(from[next]!=-2) return;
                        from[next]=i;removed[next]=f;q.push_back(next);
                    });
                }
            }
            if(end<0) return;
            while(true) {
                int c=color[end];chosen[end]=1;
                if(from[c]<0) {--capacity[c];break;}
                chosen[removed[c]]=0;end=from[c];
            }
        }
        vector<vector<int>> out(colors);
        for(int c=0;c<colors;++c) {
            vector<char> seen(colors);
            for(int i:members[c]) if(!chosen[i])
                path(i,[&](int f){
                    int h=color[f];
                    if(!seen[h]) {seen[h]=true;out[c].push_back(h);}
                });
        }
        vector<int> num(colors),low(colors),part(colors,-1),stack;
        int timer=0;
        auto scc=[&](auto&& self,int c)->void {
            low[c]=num[c]=++timer;stack.push_back(c);
            for(int d:out[c]) {
                if(!num[d]) {self(self,d);low[c]=min(low[c],low[d]);}
                else if(part[d]<0) low[c]=min(low[c],num[d]);
            }
            if(num[c]==low[c]) {
                int d;
                do {d=stack.back();stack.pop_back();part[d]=c;} while(d!=c);
            }
        };
        for(int c=0;c<colors;++c) if(!num[c]) scc(scc,c);
        vector<char> variable(total);
        for(int i=0;i<total;++i) if(!chosen[i])
            path(i,[&](int f){
                if(part[color[f]]==part[color[i]]) variable[i]=variable[f]=1;
            });
        for(int i=0;i<total;++i) if(!variable[i]) mark(ids[i],chosen[i]);
    }

    // Two measurements A+B+z and A-B give both subproblem
    // measurements and a new binary coordinate z, obtained by parity.
    // A negative coefficient uses the other side of a known-count group.
    struct Code {
        vector<vector<signed char>> rows{{1}};
        vector<bool> leaf{true};
        void doubleSize() {
            int h=rows.size(), v=leaf.size();
            vector<vector<signed char>> a(2*h,vector<signed char>(2*v+h));
            for(int i=0;i<h;++i) {
                for(int j=0;j<v;++j) {
                    a[i][j]=a[i+h][j]=a[i][v+j]=rows[i][j];
                    a[i+h][v+j]=-rows[i][j];
                }
                a[i][2*v+i]=1;
            }
            vector<bool> l(2*v+h,false);
            for(int j=0;j<v;++j) l[j]=l[v+j]=leaf[j];
            rows.swap(a);leaf.swap(l);
        }
        static void recover(const vector<int>& readings, int start, int size,
                            vector<int>& out) {
            int h=readings.size()/2;
            if(!h) {out[start]=readings[0];return;}
            int v=(size-h)/2;
            vector<int> a(h),b(h);
            for(int i=0;i<h;++i) {
                int z=(readings[i]-readings[i+h])&1;
                out[start+2*v+i]=z;
                a[i]=(readings[i]+readings[i+h]-z)/2;
                b[i]=(readings[i]-readings[i+h]-z)/2;
            }
            recover(a,start,v,out);recover(b,start+v,v,out);
        }
    };
    void codedGroups(vector<Group>& groups,vector<char>& used,vector<Group>& next,
                     const vector<Code>& codes) {
        // Each row needs to be a forest. The union across different rows
        // can contain cycles, which allows substantially larger batches.
        vector<int> order;
        for(int j=(int)groups.size()-1;j>=0;--j)
            if(groups[j].edges.size()<=size_t(max(5,n/8))) order.push_back(j);
        while(order.size()>=18) {
            vector<int> id,mid,lo;
            int chosen=-1;
            for(int c=(int)codes.size()-1;c>=0;--c) {
                const Code& plan=codes[c];
                int h=plan.rows.size(),slots=plan.leaf.size();
                if((int)order.size()<slots-h/3 || groups[order[0]].edges.size()*h>size_t(n)*2)
                    continue;
                id.assign(slots,-1);mid.resize(slots);lo.resize(slots);
                vector<DSU> d(h,DSU(n));
                vector<char> taken(order.size());
                vector<int> slotOrder(slots);
                iota(slotOrder.begin(),slotOrder.end(),0);
                // Place the densest columns first. Later columns have more
                // opportunities to fit between the existing components.
                vector<int> weight(slots);
                for(const auto& row:plan.rows)
                    for(int i=0;i<slots;++i) weight[i]+=row[i]!=0;
                stable_sort(slotOrder.begin(),slotOrder.end(),[&](int a,int b){
                    return weight[a]>weight[b];
                });
                int count=0;
                for(int i:slotOrder) {
                    int tried=0;
                    for(int j=0;j<(int)order.size() && tried<10;++j) {
                        if(taken[j]) continue;
                        const Group& g=groups[order[j]];
                        int size=g.edges.size(),split=size/2;
                        if(!plan.leaf[i] && min(g.count,size-g.count)>1) continue;
                        ++tried;
                        vector<pair<int,DSU>> trial;
                        bool fits=true;
                        for(int r=0;r<h && fits;++r) if(plan.rows[r][i]) {
                            trial.emplace_back(r,d[r]);
                            DSU& row=trial.back().second;
                            int a=plan.rows[r][i]>0?0:split, b=a?size:split;
                            for(int t=a;t<b;++t) if(!add(row,g.edges[t])) {fits=false;break;}
                        }
                        if(!fits) continue;
                        for(auto& [r,row]:trial) d[r]=move(row);
                        taken[j]=true;id[i]=order[j];mid[i]=split;
                        lo[i]=max(0,g.count-size+split);
                        ++count;break;
                    }
                }
                if(count>=slots-h/3) {chosen=c;break;}
            }
            if(chosen<0) break;
            const Code& code=codes[chosen];
            int slots=id.size();
            for(int j:id) if(j>=0) used[j]=true;
            vector<int> readings;
            for(auto& row:code.rows) {
                vector<int> forest;
                int offset=0;
                for(int i=0;i<slots;++i) if(id[i]>=0 && row[i]) {
                    const Group& g=groups[id[i]];
                    auto split=g.edges.begin()+mid[i];
                    if(row[i]>0) {
                        forest.insert(forest.end(),g.edges.begin(),split);
                        offset+=lo[i];
                    } else {
                        forest.insert(forest.end(),split,g.edges.end());
                        offset+=g.count-lo[i];
                    }
                }
                readings.push_back(countForest(forest)-offset);
            }
            vector<int> vals(slots);
            Code::recover(readings,0,slots,vals);
            for(int i=0;i<slots;++i) if(id[i]>=0) {
                const Group& g=groups[id[i]];
                auto split=g.edges.begin()+mid[i];
                int k=vals[i]+lo[i];
                next.push_back({{g.edges.begin(),split},k});simplify(next.back());
                next.push_back({{split,g.edges.end()},g.count-k});simplify(next.back());
            }
            erase_if(order,[&](int j){return used[j];});
        }
    }
    void searchGroups(vector<int> order) {
        shuffle(order.begin(),order.end(),rng);
        vector<int> rest;
        int left=n-1;
        for(int e:order) {
            int s=status(e);
            if(s<0) rest.push_back(e);
            else left-=s;
        }
        vector<Group> groups;
        // Partition unknown edges into maximal forests. The last count
        // follows from the known size of the hidden tree.
        while(!rest.empty()) {
            DSU d(n);
            vector<int> forest, next;
            for(int e:rest) {
                if(add(d,e)) forest.push_back(e);
                else next.push_back(e);
            }
            int k;
            if(!left || left==(int)rest.size()) k=left?(int)forest.size():0;
            else if(next.empty()) k=left;
            else k=countForest(forest);
            left-=k;
            groups.push_back({move(forest),k});
            simplify(groups.back());
            rest.swap(next);
        }

        vector<Code> codes;
        Code code;
        for(int i=1;i<=6;++i) { code.doubleSize(); if(i>=3) codes.push_back(code); }
        while(true) {
            topology();
            auto clean=[&]() {
                int before;
                do {
                    before=changes;
                    for(auto& g:groups) simplify(g);
                } while(changes!=before);
                erase_if(groups,[](const Group& g){return g.edges.empty();});
            };
            clean();
            inferExchanges(groups);
            clean();
            if(groups.empty()) {
                vector<int> royal;
                for(int e=0;e<m;++e) if(status(e)==1) royal.push_back(e);
                finish(royal);
            }
            // Work breadth first, allowing many narrowed-down searches to
            // share measurements once their tested edges fit into one forest.
            sort(groups.begin(),groups.end(),[](const Group& a,const Group& b){
                return a.edges.size()>b.edges.size();
            });
            vector<Group> next;
            vector<char> used(groups.size());
            codedGroups(groups,used,next,codes);
            for(int start=0;start<(int)groups.size();++start) if(!used[start]) {
                DSU together(n);
                int rank=0,states=1,totalWidth=0;
                vector<vector<int>> parts,others;
                vector<int> totals,lower,width;
                for(int j=start;j<(int)groups.size() && parts.size()<15 && rank<n-1;++j) {
                    if(used[j]) continue;
                    auto& g=groups[j];
                    simplify(g);
                    if(g.edges.empty()) {used[j]=true;continue;}
                    int s=g.edges.size(), target=s/2;
                    if(groups.size()>=12 && s>n/12) target=max(1,(s+2)/4);
                    int minimum=max(1,(target+1)/2);
                    if(n-1-rank<minimum) continue;
                    DSU test=together;
                    vector<int> a,b;
                    for(int e:g.edges) {
                        if((int)a.size()<target && add(test,e)) a.push_back(e);
                        else b.push_back(e);
                    }
                    int size=a.size();
                    if(size<minimum) continue;
                    int lo=max(0,g.count-(s-size)),hi=min(g.count,size);
                    int w=hi-lo;
                    if(!parts.empty() && (1LL*states*(w+1)>32768 || totalWidth+w>60)) continue;
                    together=move(test);
                    rank+=size;states*=w+1;totalWidth+=w;
                    lower.push_back(lo);width.push_back(w);totals.push_back(g.count);
                    parts.push_back(move(a));others.push_back(move(b));
                    used[j]=true;
                    if(totalWidth>=60 || states>=20000) break;
                }
                if(parts.empty()) continue;
                auto result=decode(width,[&](int mask) {
                    vector<int> forest;
                    int offset=0;
                    for(int i=0;i<(int)parts.size();++i) if(mask>>i&1) {
                        forest.insert(forest.end(),parts[i].begin(),parts[i].end());
                        offset+=lower[i];
                    }
                    return countForest(forest)-offset;
                });
                for(int i=0;i<(int)parts.size();++i) {
                    int k=lower[i]+result[i];
                    next.push_back({move(parts[i]),k});simplify(next.back());
                    next.push_back({move(others[i]),totals[i]-k});simplify(next.back());
                }
            }
            groups.swap(next);
        }
    }

public:
    void run() {
        if(!(cin>>n>>m)) return;
        edge.resize(m);
        for(auto& [u,v]:edge) if(!(cin>>u>>v)) return;
        equal=DSU(m);value.assign(m,-1);
        topology();
        vector<int> fixed,freeEdges,order(m);
        iota(order.begin(),order.end(),0);
        for(int i:order) {
            if(status(i)==1) fixed.push_back(i);
            else if(status(i)<0) freeEdges.push_back(i);
        }
        if((int)fixed.size()==n-1) finish(fixed);
        small(fixed,freeEdges);
        if(m==n) {
            // Each tree omits one edge of the sole cycle.
            vector<int> trial=order;
            for(int j=(int)freeEdges.size()-1;j>0;--j) {
                trial.clear();
                for(int e:order) if(e!=freeEdges[j]) trial.push_back(e);
                ask(trial);
            }
            trial.clear();
            for(int e:order) if(e!=freeEdges.front()) trial.push_back(e);
            finish(trial);
        }
        vector<int> degrees(n);
        for(auto [u,v]:edge) {++degrees[u];++degrees[v];}
        for(int v=0;v<n;++v) if(degrees[v]==n-1) possibleStars.push_back(v);
        auto otherOrder=order;
        reverse(otherOrder.begin(),otherOrder.end());
        predictions.push_back(basis(otherOrder));
        stable_sort(otherOrder.begin(),otherOrder.end(),[&](int a,int b){
            return abs(edge[a].u-edge[a].v)<abs(edge[b].u-edge[b].v);
        });
        predictions.push_back(basis(otherOrder));
        for(const auto& candidate:predictions) {
            vector<char> bits(m);
            for(int e:candidate) bits[e]=true;
            predictedEdges.push_back(move(bits));
        }
        calibrate(order);
        searchGroups(move(order));
    }
};
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver().run();
}
