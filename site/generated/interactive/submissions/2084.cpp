#include <bits/stdc++.h>
using namespace std;

constexpr int MAXN = 1024;
using Mask = bitset<MAXN>;
using Table = vector<vector<unsigned char>>;

struct Solver {
    int n, k, aliveCount, edges;
    // Every variety has a live representative. The remaining edges are
    // exactly the live pairs for which we still need to establish inequality.
    Table unknown;
    vector<int> degree;
    vector<bool> alive;
    struct Cup {
        int cafe;
        // At least one of these live cafes has the value of this cup.
        Mask options;
    };
    deque<Cup> memory;
    mt19937 rng{7320841};
    int resetCount = 0, queryCount = 0, knownAnswer = -1;
    bool cleanSweep = false;
    struct Model { unsigned labels; int diversity; };
    vector<Model> models;
    clock_t started = clock();

    Solver(int size, int capacity) : n(size), k(capacity), aliveCount(n),
        edges(n*(n-1)/2), unknown(n, vector<unsigned char>(n, 1)),
        degree(n, n-1), alive(n, true) {
        for (int i = 0; i < n; ++i) unknown[i][i] = 0;
        if (n<=8) enumerate(0,0,0);
    }
    void enumerate(int i, unsigned labels, int distinct) {
        if (i==n) {models.push_back({labels,distinct}); return;}
        for (int v=0; v<=distinct; ++v)
            enumerate(i+1, labels | (unsigned(v)<<(3*i)), max(distinct,v+1));
    }
    bool predicts(const Model& model, int v) const {
        unsigned label=(model.labels>>(3*v))&7;
        for (const auto& cup:memory)
            if (((model.labels>>(3*cup.cafe))&7)==label) return true;
        return false;
    }
    void reset() {
        if (memory.empty()) return;
        ++resetCount;
        cout << "R" << endl;
        memory.clear();
    }
    void different(int a, int b) {
        if (!unknown[a][b]) return;
        unknown[a][b] = unknown[b][a] = 0;
        --degree[a]; --degree[b]; --edges;
    }
    char ask(int v) {
        ++queryCount;
        cout << "? " << v+1 << endl;
        char answer;
        if (!(cin >> answer) || (answer != 'Y' && answer != 'N')) exit(0);
        return answer;
    }
    void taste(int v) {
        char answer=ask(v);
        if (!models.empty()) {
            erase_if(models,[&](const Model& m){return predicts(m,v)!=(answer=='Y');});
            int d=models.front().diversity;
            if (all_of(models.begin(),models.end(),[&](const Model& m){return m.diversity==d;}))
                knownAnswer=d;
        }
        Mask options;
        if (answer == 'Y') {
            for (const auto& cup : memory) options |= cup.options;
            for (int x = 0; x < n; ++x)
                if (x != v && !unknown[v][x]) options.reset(x);
        }
        // The other live candidates cover v's variety. Substitute them in
        // existing witnesses so that future eliminations cannot form a cycle.
        if (answer == 'Y' && !options[v] && options.any()) {
            if (options.count() == 1) {
                int x = options._Find_first();
                for (int u = 0; u < n; ++u)
                    if (!unknown[v][u] && u != v) different(x, u);
            }
            for (int u = 0; u < n; ++u) different(v, u);
            alive[v] = false;
            --aliveCount;
            for (auto& cup : memory) if (cup.options[v]) {
                cup.options.reset(v);
                cup.options |= options;
            }
        } else {
            if (answer == 'N') {
                for (const auto& cup : memory) {
                    if (alive[cup.cafe]) different(v, cup.cafe);
                    else if (cup.options.count() == 1) different(v, cup.options._Find_first());
                }
            }
            options.reset();
            options.set(v);
        }
        memory.push_back({v, options});
        if ((int)memory.size() > k) memory.pop_front();
    }

    // A final query may determine the count even when it does not
    // identify the individual equalities. Check this exactly on small inputs.
    bool smallFinish() {
        if (models.empty() || models.size()>350) return false;
        for (int v=0;v<n;++v) {
            int counts[2]={-1,-1};
            bool works=true;
            for (const auto& model:models) {
                int& d=counts[predicts(model,v)];
                if (d<0) d=model.diversity;
                else if (d!=model.diversity) {works=false;break;}
            }
            if (works) {
                knownAnswer=counts[ask(v)=='Y'];
                return true;
            }
        }
        return false;
    }

    // For capacity one, the pending comparisons are the edges of a graph.
    // Pair odd vertices so each component can be traversed by an Euler walk.
    vector<int> trails() {
        vector<int> plan;
        vector<bool> used(n);
        int last = memory.empty() ? -1 : int(memory.back().options._Find_first());
        for (int i = -1; i < n; ++i) {
            int s = i == -1 ? last : i;
            if (s < 0 || used[s] || !degree[s]) continue;
            vector<int> component{s}, odd;
            used[s] = true;
            vector<vector<int>> adj(n);
            for (int h = 0; h < (int)component.size(); ++h) {
                int v = component[h];
                if (degree[v] % 2) odd.push_back(v);
                for (int w = 0; w < n; ++w) if (unknown[v][w]) {
                    adj[v].push_back(w);
                    if (!used[w]) {
                        used[w] = true;
                        component.push_back(w);
                    }
                }
            }
            if (!odd.empty()) {
                auto it = find(odd.begin(), odd.end(), s);
                if (it != odd.end()) iter_swap(odd.begin(), it);
                else if (s != last) s = odd[0];
                // Leave two odd endpoints, or start at the current even vertex.
                int first = 2;
                if (s != odd[0]) {
                    adj[s].push_back(odd[1]);
                    adj[odd[1]].push_back(s);
                }
                for (int j = first; j < (int)odd.size(); j += 2) {
                    adj[odd[j]].push_back(odd[j+1]);
                    adj[odd[j+1]].push_back(odd[j]);
                }
            }
            vector<int> stack{s}, tour;
            while (!stack.empty()) {
                int v = stack.back();
                if (adj[v].empty()) {
                    tour.push_back(v);
                    stack.pop_back();
                } else {
                    int w = adj[v].back();
                    adj[v].pop_back();
                    auto it = find(adj[w].begin(), adj[w].end(), v);
                    *it = adj[w].back();
                    adj[w].pop_back();
                    stack.push_back(w);
                }
            }
            reverse(tour.begin(), tour.end());
            plan.insert(plan.end(), tour.begin() + (s == last), tour.end());
            last = -1;
        }
        return plan;
    }

    vector<int> order() const {
        vector<int> result, tail;
        vector<bool> in(n);
        for (auto it = memory.rbegin(); it != memory.rend(); ++it) {
            int v = it->cafe;
            if (alive[v] && degree[v] && !in[v]) {
                tail.push_back(v);
                in[v] = true;
            }
        }
        for (int v = 0; v < n; ++v)
            if (degree[v] && !in[v]) result.push_back(v);
        result.insert(result.end(), tail.rbegin(), tail.rend());
        return result;
    }

    // Cover comparisons in runs of b cafes. At distance k/b between
    // runs, the earlier run's suffix overlaps the later run's prefix.
    vector<int> blockPlan(const vector<int>& ids, int b, int style) {
        int m = (ids.size()+b-1)/b, r = k/b;
        int padding = m*b-ids.size();
        vector<vector<int>> blocks(m);
        for (int i = 0; i < (int)ids.size(); ++i)
            blocks[(padding+i)/b].push_back(ids[i]);
        Table need(m, vector<unsigned char>(m));
        vector<int> weight(m), internal(m);
        const int value[] = {0,b, b,2*b,2,b+2,b+2,2*b+2};
        auto flipped = [](int mask) {return (mask&4) | ((mask&1)<<1) | ((mask&2)>>1);};
        for (int a=0; a<m; ++a) for (int c=a; c<m; ++c) {
            int mask=0;
            for (int i=0; i<(int)blocks[a].size(); ++i)
                for (int j=0; j<(int)blocks[c].size(); ++j) {
                    if (!unknown[blocks[a][i]][blocks[c][j]]) continue;
                    if (a==c) internal[a]=1;
                    else mask |= i==j ? 4 : (i>j ? 1 : 2);
                }
            if (mask) {
                need[a][c]=mask;
                need[c][a]=flipped(mask);
                weight[a]+=value[mask];
                weight[c]+=value[mask];
            }
        }
        vector<int> history, in(m);
        // Use an intact suffix of whole runs from the current memory.
        int hi = memory.size();
        for (int a=m-1; a>=0 && (int)history.size()<r; --a) {
            int lo=hi-blocks[a].size();
            if (lo<0) break;
            bool ok=true;
            for (int i=lo; i<hi; ++i)
                if (memory[i].cafe!=blocks[a][i-lo]) ok=false;
            if (!ok) break;
            in[a]=1;
            history.push_back(a);
            hi=lo;
        }
        reverse(history.begin(),history.end());
        vector<int> plan, tie(m);
        for (int &v:tie) v=rng()%1000;
        int head=0, idle=0;
        while (true) {
            int best=-1;
            double bestScore=-1e30;
            bool pending=false;
            for (int v=0; v<m; ++v) if (weight[v] || internal[v]) {
                pending=true;
                if (in[v]) continue;
                int gain=internal[v]*b;
                for (int t=head; t<(int)history.size(); ++t) {
                    int mask=need[history[t]][v];
                    if ((int)history.size()-t==r) mask &= 5;
                    gain+=value[mask];
                }
                int prev=history.size()>(size_t)head ? history.back() : m-1;
                double score=gain + (gain? 4*r*b : 0);
                if (style%3==1) score+=0.28*weight[v]/max(1,m/r);
                else if (style%3==2) score+=0.09*weight[v]/max(1,m/r);
                if (!gain) score+=0.3*weight[v]/(1.0+2*b*m);
                if (style%2==0) score+=0.2*(m-(v-prev-1+m)%m)/m;
                score+=0.05*tie[v]/1000;
                if (score>bestScore) {bestScore=score; best=v;}
            }
            if (!pending) break;
            if (best<0) {
                plan.push_back(-1);
                fill(in.begin(),in.end(),0);
                history.clear(); head=0;
                continue;
            }
            int before=weight[best]+internal[best];
            internal[best]=0;
            for (int t=head;t<(int)history.size();++t) {
                int u=history[t], mask=need[u][best];
                int left=(int)history.size()-t==r ? mask&2 : 0;
                int diff=value[mask]-value[left];
                weight[u]-=diff; weight[best]-=diff;
                need[u][best]=left; need[best][u]=flipped(left);
            }
            if (before==weight[best]) {if (++idle>r+2) break;} else idle=0;
            plan.insert(plan.end(),blocks[best].begin(),blocks[best].end());
            if ((int)history.size()-head==r) in[history[head++]]=0;
            in[best]=1; history.push_back(best);
        }
        return plan;
    }

    // Replay a proposed schedule on the all-distinct branch, then finish
    // any uncovered pairs. Scores count each candidate's neighbors in memory.
    vector<int> finishPlan(const vector<int>& seed, int style) {
        Table rem=unknown;
        vector<int> deg=degree, score(n), in(n), plan;
        deque<int> q;
        int left=edges;
        auto enter=[&](int v) {
            if (v>=0 && in[v]++==0)
                for (int i=0;i<n;++i) score[i]+=rem[v][i];
            q.push_back(v);
        };
        auto leave=[&]() {
            int v=q.front(); q.pop_front();
            if (v>=0 && --in[v]==0)
                for (int i=0;i<n;++i) score[i]-=rem[v][i];
        };
        for (const auto& cup:memory)
            enter(cup.options.count()==1 ? int(cup.options._Find_first()) : -1);
        auto add=[&](int v) {
            plan.push_back(v);
            for (int u:q) if (u>=0 && rem[v][u]) {
                rem[v][u]=rem[u][v]=0;
                --deg[v]; --deg[u]; --left; --score[v];
            }
            if ((int)q.size()==k) leave();
            enter(v);
        };
        auto clear=[&]() {
            while (!q.empty()) leave();
            if (!plan.empty() && plan.back()==-1) return;
            plan.push_back(-1);
        };
        for (int v:seed) {
            if (!left) break;
            if (v<0) clear();
            else if (deg[v] && !in[v]) add(v);
        }
        vector<int> jitter(n);
        for (int &v:jitter) v=rng()%1024;
        while (left) {
            int best=-1;
            double bestScore=-1;
            for (int v=0; v<n; ++v) if (!in[v] && deg[v]) {
                double s=score[v] + (score[v]?k:0);
                s+= (style==1 ? 0.3*k : 0.45)*deg[v]/n;
                int prev=q.empty()?n-1:q.back();
                if (style!=2) s+=0.2*(n-(v-prev-1+n)%n)/n;
                s+=0.05*jitter[v]/1024;
                if (s>bestScore) { best=v; bestScore=s; }
            }
            if (best==-1) clear();
            else add(best);
        }
        return plan;
    }

    vector<int> prune(const vector<int>& plan) {
        vector<int> a;
        for (const auto& cup:memory)
            a.push_back(cup.options.count()==1 ? int(cup.options._Find_first()) : -2);
        int first=a.size();
        a.insert(a.end(),plan.begin(),plan.end());
        int len=a.size();
        vector<unsigned short> count(n*n);
        auto at=[&](int x,int y) -> unsigned short& {
            return count[min(x,y)*n+max(x,y)];
        };
        vector<int> prev(len),next(len);
        for (int i=0;i<len;++i) {
            prev[i]=i-1; next[i]=i+1;
            if (i<first || a[i]<0) continue;
            for (int j=i-1;j>=0 && i-j<=k && a[j]!=-1;--j)
                if (a[j]>=0 && unknown[a[j]][a[i]]) ++at(a[j],a[i]);
        }
        vector<int> l(k),r(k),incident(n);
        for (int pass=0;pass<3;++pass) {
            bool changed=false;
            for (int t=0;t<len-first;++t) {
                int i=(pass%2==0)?len-1-t:first+t;
                int v=a[i];
                if (v<0) continue;
                int nl=0,nr=0;
                bool needed=false;
                for (int j=prev[i];j>=0 && a[j]!=-1 && nl<k;j=prev[j]) {
                    int u=a[j]; l[nl++]=u;
                    if (u>=0 && unknown[v][u] && at(v,u)<=1) {needed=true; break;}
                }
                if (needed) continue;
                for (int j=next[i];j<len && a[j]!=-1 && nr<k;j=next[j]) {
                    int u=a[j]; r[nr++]=u;
                    if (unknown[v][u] && at(v,u)<=1) {needed=true; break;}
                }
                if (needed) continue;
                for (int h=0;h<nl;++h) if (l[h]>=0) ++incident[l[h]];
                for (int h=0;h<nr;++h)
                    if (unknown[v][r[h]] && at(v,r[h])<=1+incident[r[h]] && incident[r[h]]) needed=true;
                for (int h=0;h<nl;++h)
                    if (l[h]>=0 && unknown[v][l[h]] && at(v,l[h])<=incident[l[h]]) needed=true;
                for (int h=0;h<nl;++h) if (l[h]>=0) --incident[l[h]];
                for (int h=max(0,k-nr);h<nl;++h)
                    if (l[h]==r[k-1-h]) needed=true;
                if (needed) continue;
                for (int h=0;h<nl;++h) if (l[h]>=0 && unknown[v][l[h]]) --at(v,l[h]);
                for (int h=0;h<nr;++h) if (unknown[v][r[h]]) --at(v,r[h]);
                for (int h=max(0,k-nr);h<nl;++h) {
                    int x=l[h],y=r[k-1-h];
                    if (x>=0 && unknown[x][y]) ++at(x,y);
                }
                if (prev[i]>=0) next[prev[i]]=next[i];
                if (next[i]<len) prev[next[i]]=prev[i];
                a[i]=-3; changed=true;
            }
            if (!changed) break;
        }
        vector<int> result;
        for (int i=first;i<len;++i) if (a[i]!=-3) result.push_back(a[i]);
        return result;
    }

    // A cyclic stride covers the same set of offsets from every group.
    // Search for moduli where a few strides cover all offsets efficiently.
    struct Design {
        double estimate;
        int modulus;
        vector<int> steps;
    };

    vector<Design> designs(int size) {
        constexpr int limit=192;
        vector<Design> results;
        for (int p=3;p<=min(limit,size+7);++p) {
            int b=(size+p-1)/p, full=(k+1)/b-1, radius=k/b;
            if (radius<1 || full>=p/2) continue;
            full=max(full,0);
            vector<bitset<limit>> covers(p);
            vector<double> cost(p);
            for (int s=1;s<p;++s) {
                int g=gcd(p,s),period=p/g;
                for (int j=1;j<=min(full,period-1);++j) {
                    int delta=j*s%p;
                    covers[s].set(delta);
                    covers[s].set(p-delta);
                }
                if (radius>full && radius<period) covers[s].set(radius*s%p);
                int wrap=clamp(period-full-1,0,radius);
                cost[s]=size*(1.0+double(g*wrap)/p);
            }
            // Include the initial sequential scan as the first stride.
            int trials=p<85?12:5;
            Design best{1e100,p,{}};
            for (int trial=0;trial<trials;++trial) {
                bitset<limit> todo;
                for (int d=1;d<p;++d) todo.set(d);
                todo &= ~covers[1];
                Design trialDesign{double(min(radius,p-full-1))*size/p,p,{1}};
                while (todo.any()) {
                    int selected=0;
                    double score=0;
                    for (int s=2;s<p;++s) {
                        int gain=(todo & covers[s]).count();
                        double val=gain/cost[s];
                        if (trial) val*=1.0+(rng()%1000)*(trial%4)*0.00008;
                        if (val>score) {score=val;selected=s;}
                    }
                    if (!selected) {trialDesign.estimate=1e100;break;}
                    todo &= ~covers[selected];
                    trialDesign.steps.push_back(selected);
                    trialDesign.estimate+=cost[selected];
                    if (trialDesign.estimate>=best.estimate) break;
                }
                if (trialDesign.estimate<best.estimate) best=move(trialDesign);
            }
            if (!best.steps.empty()) results.push_back(move(best));
        }
        sort(results.begin(),results.end(),[](const Design& a,const Design& b){
            return a.estimate<b.estimate;
        });
        return results;
    }

    vector<int> expand(const Design& design, const vector<int>& ids) {
        int p=design.modulus, size=ids.size(), b=(size+p-1)/p;
        int full=max(0,(k+1)/b-1),radius=k/b;
        vector<int> plan;
        for (int s:design.steps) {
            int g=gcd(s,p), period=p/g;
            int wrap=clamp(period-full-1,0,radius);
            for (int a=0;a<g;++a) {
                if (s!=1) plan.push_back(-1);
                int length=wrap+(s==1?0:period);
                int block=a;
                for (int t=0;t<length;++t) {
                    int lo=block*size/p,hi=(block+1)*size/p;
                    plan.insert(plan.end(),ids.begin()+lo,ids.begin()+hi);
                    block=(block+s)%p;
                }
            }
        }
        return plan;
    }

    // Short covering walks on groups of size at most k/2. Two adjacent
    // groups compare fully. At separation two, each direction tests one
    // triangle of the comparison matrix.
    vector<int> coveringWalk(const vector<int>& ids,int m) {
        static const vector<int> eight={0,4,6,2,5,0,3,7,1,5,6,3,1,4,7,2};
        static const vector<int> sixteen={
            13,8,0,5,15,9,13,7,11,5,1,13,5,2,8,12,15,3,9,12,0,1,10,14,8,1,
            6,2,11,0,3,13,4,3,8,11,4,6,15,0,4,9,1,15,11,14,9,2,14,4,10,2,
            13,6,10,3,9,6,5,14,3,7,1,4,12,7,2,15,10,7,0,14,6,12,5,10};
        vector<int> plan;
        for (int g:(m==8?eight:sixteen)) {
            int lo=g*ids.size()/m,hi=(g+1)*ids.size()/m;
            plan.insert(plan.end(),ids.begin()+lo,ids.begin()+hi);
        }
        return plan;
    }

    vector<int> choosePlan(int round) {
        cleanSweep=false;
        if (k==1) return trails();
        vector<int> ids=order();
        vector<int> best;
        int bestCost=INT_MAX;
        auto consider=[&](const vector<int>& seed,int style) {
            vector<int> p=finishPlan(seed,style);
            if ((int)p.size() > bestCost*1.3+5) return;
            p=prune(p);
            int cost=p.size()-count(p.begin(),p.end(),-1);
            if (cost<bestCost) {best=move(p);bestCost=cost;}
        };
        consider({},0);
        if (bestCost<=1) return best;
        consider(ids,0);
        for (int m:{8,16}) if ((int)ids.size()<=m*k/2 && (int)ids.size()>m*k/4)
            consider(coveringWalk(ids,m),0);
        double limit=min(3.5,0.65+round*0.4);
        auto timeUsed=[&]() {return double(clock()-started)/CLOCKS_PER_SEC;};
        if (round==1 && (int)ids.size()>2*k && timeUsed()<limit) {
            auto candidates=designs(ids.size());
            for (int j=0;j<min(10,(int)candidates.size()) && timeUsed()<limit;++j)
            {
                auto seed=expand(candidates[j],ids);
                consider(seed,0);
                if (j<4) {erase(seed,-1);consider(seed,0);}
            }
        }
        for (int style=0; style<4 && timeUsed()<limit; ++style) {
            for (int b=1;b<=k/2 && timeUsed()<limit;b*=2) {
                int m=(ids.size()+b-1)/b;
                if (m<2 || (m>160 && b>1) || (b==1 && m>144)) continue;
                consider(blockPlan(ids,b,style),style%3);
            }
            if (timeUsed()<limit) consider({},1+style%2);
        }
        if ((int)ids.size()<=k+1 && ids.size()<=bestCost*1.65) {
            // Ambiguous cups from the previous scan can make a positive
            // response inconclusive. A fresh single pass resolves every
            // pair in this set, including on branches with positives.
            vector<double> chance(n);
            for (const Cup& cup:memory) {
                int alternatives=cup.options.count();
                if (alternatives<=1) continue;
                for (int v:ids) if (cup.options[v]) chance[v]+=1.0/alternatives;
            }
            double exposure=0;
            int age=0;
            for (int v:best) {
                if (v<0 || age>=k) break;
                exposure+=min(1.0,chance[v])*(k-age)/k;
                ++age;
            }
            if (exposure>0.5 && ids.size()<=bestCost*(1.0+0.65*min(1.0,exposure))) {
                cleanSweep=true;
                ids.insert(ids.begin(),-1);
                return ids;
            }
        }
        return best;
    }

    // A bounded completion strategy. Each pair of groups fits in a single
    // clean window. Surviving cafes in that window must all be different.
    void finishGroups() {
        int size=max(1,(k+1)/2);
        vector<int> ids=order();
        for (int i=0;i<(int)ids.size() && edges;i+=size) {
            for (int j=i;j<(int)ids.size() && edges;j+=size) {
                vector<int> group;
                for (int a=i;a<min(i+size,(int)ids.size());++a) if (degree[ids[a]]) group.push_back(ids[a]);
                if (i!=j) for (int a=j;a<min(j+size,(int)ids.size());++a) if (degree[ids[a]]) group.push_back(ids[a]);
                bool fresh=true;
                for (int x:group) {
                    if (!any_of(group.begin(),group.end(),[&](int y){return unknown[x][y];})) continue;
                    if (fresh) {reset();fresh=false;}
                    taste(x);
                    if (knownAnswer>=0) return;
                }
            }
        }
    }

    void solve() {
        if (n>1) for (int v=0;v<n && knownAnswer<0;++v) taste(v);
        int round=0;
        while (edges && knownAnswer<0) {
            if (resetCount>=10000 || queryCount>n+2*n*n/k || double(clock()-started)/CLOCKS_PER_SEC>5.0) {
                finishGroups();
                break;
            }
            int initial=aliveCount, initialEdges=edges, cost=0;
            vector<int> plan=choosePlan(++round);
            for (int v:plan) {
                if (!edges || knownAnswer>=0 || resetCount>=10000) break;
                if (v<0) {reset(); continue;}
                if (!alive[v] || !degree[v]) continue;
                bool repeated=false;
                for (const auto& cup:memory) if (cup.cafe==v) {repeated=true;break;}
                if (repeated) continue;
                if (smallFinish()) break;
                taste(v);
                ++cost;
                if (k==1 && initial!=aliveCount) break;
                if (!cleanSweep && initial-aliveCount>=max(3,initial/6)) break;
            }
            // A positive with an ambiguous witness can invalidate a planned
            // comparison. An isolated pair always resolves one pending edge.
            if (edges && knownAnswer<0 && (initialEdges==edges || !cost)) {
                int x=0;
                while (!degree[x]) ++x;
                int y=find(unknown[x].begin(),unknown[x].end(),1)-unknown[x].begin();
                reset(); taste(x); if (knownAnswer<0) taste(y);
            }
        }
        cout << "! " << (knownAnswer<0 ? aliveCount : knownAnswer) << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n,k;
    if (cin>>n>>k) Solver(n,k).solve();
}
