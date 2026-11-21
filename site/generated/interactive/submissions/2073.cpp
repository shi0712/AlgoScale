#include <bits/stdc++.h>
using namespace std;
using Mask = uint64_t;
static int n, k;
static mt19937 rng;
static int mod(int x) { x %= n; return x < 0 ? x+n : x; }
static Mask bit(int i) { return Mask(1) << i; }
static void moveWheel(int i, int d) {
    d = mod(d);
    if (!d) return;
    if (2*d > n) d -= n;
    cout << i+1 << ' ' << d << endl;
    if (!(cin >> k) || k <= 1 || k > n) exit(0);
}
static int population(Mask m) { return __builtin_popcountll(m); }
static int first(Mask m) { return __builtin_ctzll(m); }
static Mask rotated(Mask m, int d) {
    return ((m << d) & (bit(n)-1)) | (m >> (n-d));
}

// Minimum-total-length decision trees for the uniform set of relative states.
// Each row holds (wheel, rotation) and the next row for each reported count.
// They follow V(S) = |S| + min_action sum_response V(successor), V(solved)=0.
static void tinySolver() {
    static constexpr unsigned char t3[][6] = {
        {0,0,0,0,0,0},{0,1,0,3,2,0},{0,1,0,4,0,0},{0,1,0,0,2,0},
        {1,2,0,0,5,0},{0,1,0,6,0,0},{1,2,0,0,0,0},
    };
    static constexpr unsigned char t4[][6] = {
        {0,0,0,0,0,0},{0,1,0,4,5,0},{0,1,0,7,6,3},{0,1,0,0,8,0},
        {1,1,0,10,9,0},{1,2,0,12,11,13},{1,1,0,16,14,15},{1,2,0,18,17,0},
        {1,2,0,20,19,0},{0,2,0,23,22,21},{2,1,0,0,24,0},{2,2,0,25,26,0},
        {0,1,0,28,27,0},{0,1,0,0,29,0},{1,2,0,31,30,0},{0,1,0,0,32,0},
        {2,2,0,34,33,0},{2,3,0,36,37,35},{0,2,0,38,39,0},{0,2,0,10,40,0},
        {2,3,0,0,41,0},{0,2,0,0,42,0},{0,1,0,43,0,0},{1,3,0,44,0,0},
        {2,3,0,45,0,0},{0,1,0,28,46,0},{0,1,0,48,47,0},{0,2,0,49,0,0},
        {0,2,0,0,0,0},{2,2,0,51,50,0},{2,2,0,53,52,0},{0,1,0,0,54,0},
        {1,2,0,20,55,0},{2,3,0,56,57,0},{2,1,0,0,58,0},{0,1,0,0,59,0},
        {1,1,0,60,0,0},{1,2,0,51,61,0},{2,2,0,62,0,0},{0,2,0,60,0,0},
        {2,2,0,49,39,0},{2,1,0,51,0,0},{2,1,0,49,63,0},{1,2,0,0,0,0},
        {0,3,0,0,0,0},{3,1,0,0,0,0},{0,3,0,45,0,0},{1,1,0,51,61,0},
        {1,2,0,28,0,0},{1,3,0,0,0,0},{2,1,0,64,0,0},{3,3,0,0,0,0},
        {1,2,0,66,65,0},{0,3,0,0,67,0},{2,1,0,44,68,0},{2,3,0,69,70,0},
        {1,1,0,66,0,0},{0,3,0,71,0,0},{0,2,0,73,72,0},{1,3,0,71,0,0},
        {1,1,0,0,0,0},{0,1,0,71,0,0},{0,2,0,48,0,0},{0,1,0,64,0,0},
        {3,2,0,0,0,0},{0,2,0,74,0,0},{0,1,0,0,0,0},{0,1,0,51,0,0},
        {1,1,0,64,0,0},{0,2,0,71,0,0},{0,1,0,49,0,0},{2,2,0,0,0,0},
        {1,2,0,74,0,0},{2,1,0,0,0,0},{2,3,0,0,0,0},
    };
    auto table=n==3?t3:t4;
    int state=k-1;
    for (;;) {
        auto row=table[state];
        moveWheel(row[0],row[1]);
        state=row[k+1];
    }
}

// An exact list of possible relative configurations is practical on small machines.
using Small = array<unsigned char,16>;
static void smallSolver() {
    if (n > 15) return;
    double stir[16][16] = {};
    stir[0][0] = 1;
    for (int i=1; i<=n; ++i) for (int j=1; j<=i; ++j)
        stir[i][j] = j*stir[i-1][j] + stir[i-1][j-1];
    double total = stir[n][k];
    for (int j=1; j<k; ++j) total *= n-j;
    if (total > 150000) return;
    vector<Small> states;
    states.reserve((int)total);
    Small pos{};
    auto gen = [&](auto&& self, int i, Mask used, int count) -> void {
        if (i==n) { states.push_back(pos); return; }
        Mask options = bit(n)-1;
        if (count==k) options = used;
        if (count+n-i==k) options &= ~used;
        while (options) {
            int x=first(options); options &= options-1;
            pos[i]=x;
            self(self,i+1,used|bit(x),count+!(used&bit(x)));
        }
    };
    gen(gen,1,1,1);
    shuffle(states.begin(),states.end(),rng);
    int unchanged=0;
    for (int steps=0; steps<12*n && unchanged<4; ++steps) {
        if (states.size()==1) {
            int cnt[16]={};
            for (int i=0;i<n;++i) ++cnt[states[0][i]];
            int t=max_element(cnt,cnt+n)-cnt;
            for (int i=0;i<n;++i) moveWheel(i,t-states[0][i]);
        }
        struct Stat { int ways[3]={}, work=0, wins=0; } stat[16][16];
        int samples=min<int>(states.size(),2200);
        for (int s=0;s<samples;++s) {
            auto &a=states[s]; int cnt[16]={}, most=0, freq=0;
            for(int i=0;i<n;++i) most=max(most,++cnt[a[i]]);
            for(int x=0;x<n;++x) freq+=cnt[x]==most;
            for(int i=0;i<n;++i) {
                int x=a[i], base=most-(cnt[x]==most && freq==1), y=x;
                for(int d=1;d<n;++d) {
                    if (++y==n) y=0;
                    int delta=(cnt[y]==0)-(cnt[x]==1);
                    auto &v=stat[i][d]; ++v.ways[delta+1];
                    v.work+=n-max(base,cnt[y]+1);
                    v.wins+=k+delta==1;
                }
            }
        }
        auto value = [](const Stat &v,int count) {
            double score=0.73*v.work-0.4*v.wins;
            for(int z:v.ways) if(z) score+=z*log2(double(z)/count);
            return score/count;
        };
        struct Action { double score; int i,d; };
        vector<Action> actions;
        for(int i=0;i<n;++i) for(int d=1;d<n;++d)
            actions.push_back({value(stat[i][d],samples),i,d});
        stable_sort(actions.begin(),actions.end(),[](auto a,auto b){return a.score<b.score;});
        vector<Action> shortlist;
        for(auto act:actions) {
            // Equivalent immediate statistics often come from interchangeable wheels.
            bool repeat=false;
            for(auto old:shortlist) if(old.d==act.d && abs(old.score-act.score)<1e-10) repeat=true;
            if(!repeat) shortlist.push_back(act);
            if(shortlist.size()==8) break;
        }
        // Include a temporary landmark, particularly for periodic symbol sets.
        for(auto act:actions) if(stat[act.i][act.d].ways[2]*2>samples) {
            shortlist.push_back(act);break;
        }
        double best=1e100; int who=0,shift=1;
        int look=min(samples,144);
        for(auto act:shortlist) {
            Stat next[3][16][16]{}; int groups[3]={};
            for(int s=0;s<look;++s) {
                auto a=states[s]; a[act.i]=mod(a[act.i]+act.d);
                int cnt[16]={}, distinct=0,most=0,freq=0;
                for(int i=0;i<n;++i) {distinct+=cnt[a[i]]==0;most=max(most,++cnt[a[i]]);}
                int z=1+distinct-k; ++groups[z];
                if(distinct==1) continue;
                for(int x=0;x<n;++x) freq+=cnt[x]==most;
                for(int i=0;i<n;++i) {
                    int x=a[i],base=most-(cnt[x]==most && freq==1),y=x;
                    for(int d=1;d<n;++d) {
                        if(++y==n) y=0;
                        int delta=(cnt[y]==0)-(cnt[x]==1);
                        auto &v=next[z][i][d];++v.ways[1+delta];
                        v.work+=n-max(base,cnt[y]+1);v.wins+=distinct+delta==1;
                    }
                }
            }
            double score=0;
            for(int z=0;z<3;++z) if(groups[z]) {
                double prob=double(groups[z])/look, future=0;
                if(k+z-1>1) {
                    future=1e100;
                    for(int i=0;i<n;++i) for(int d=1;d<n;++d)
                        future=min(future,1+value(next[z][i][d],groups[z]));
                }
                score+=prob*(log2(prob)+future);
            }
            score+=0.12*act.score;
            if(score<best-1e-10) {best=score;who=act.i;shift=act.d;}
        }
        int before=k; moveWheel(who,shift);
        size_t size=states.size(), keep=0;
        for(auto &a:states) {
            int x=a[who], y=mod(x+shift); bool atX=false,atY=false;
            for(int i=0;i<n;++i) if(i!=who) {
                atX |= a[i]==x; atY |= a[i]==y;
            }
            if(before+int(atX)-int(atY)==k) {
                a[who]=y; states[keep++]=a;
            }
        }
        states.resize(keep);
        unchanged=keep==size?unchanged+1:0;
    }
}

// Coordinates are measured relative to the survey wheel's starting symbol.
// map is the exact occupied set between calibrations. fixed counts calibrated
// wheels, and [lo,hi] bounds the multiplicity of the remaining wheels per slot.
struct Solver {
    int originalCount=k, initial=k, target=0, left=n-1;
    int batchSize=0;
    bool diagnosed=false, useGuards=false;
    Mask batchSlots=0;
    array<int,50> rejectedDelta, conditionalLow, conditionalHigh;
    int lo[50]{}, hi[50]{}, removed[50]{}, fixed[50]{};
    vector<int> orig=vector<int>(n,-1), now=orig, order;
    vector<vector<int>> patterns;
    Mask map=0, initialSymbols=0, originalOthers=0;
    struct Grouping {
        vector<int> label, position;
        Mask assigned=0;
        int repeats=0;
        bool valid=true;
    };
    vector<Grouping> groups;
    void learnGroups(int i,int x) {
        for(auto &g:groups) if(g.valid) {
            int &y=g.position[g.label[i]];
            if(y==x) ++g.repeats;
            else if(y>=0 || (g.assigned&bit(x))) g.valid=false;
            else {y=x;g.assigned|=bit(x);}
        }
    }
    int confidence() const {return initial<=3?5:2;}
    void chooseWheel(int index) {
        if(index==0 || !patterns.empty()) return;
        int best=-1,idx=index;
        for(int j=index;j<(int)order.size();++j) {
            int score=0;
            for(auto &g:groups) if(g.valid) {
                bool fresh=g.position[g.label[order[j]]]<0;
                score+= (g.repeats>=confidence() ? fresh : !fresh) * (1+min(g.repeats,5));
            }
            if(score>best) {best=score;idx=j;}
        }
        swap(order[index],order[idx]);
    }
    void finish(const vector<int>& a) {
        int cnt[50]={}; for(int x:a) ++cnt[x];
        int t=max_element(cnt,cnt+n)-cnt;
        for(int i=0;i<n;++i) moveWheel(i,t-a[i]);
    }
    // A proposed complete configuration can be tested transactionally. Each
    // reported count either agrees with the plan or triggers an exact rewind.
    void tryConfiguration(vector<int> a) {
        int counts[50]={};vector<int> plan(n);iota(plan.begin(),plan.end(),0);
        for(int x:a) ++counts[x];
        int t=target;
        for(int x=0;x<n;++x) if(counts[x]>counts[t]) t=x;
        stable_sort(plan.begin(),plan.end(),[&](int u,int v){
            auto rank=[&](int i){return pair(now[i]>=0,(counts[a[i]]==1)==(k*2>n));};
            return rank(u)<rank(v);
        });
        vector<pair<int,int>> done;
        for(int i:plan) if(a[i]!=t) {
            int prediction=k-(counts[a[i]]==1)+(counts[t]==0);
            int d=t-a[i]; moveWheel(i,d);done.emplace_back(i,d);
            if(prediction!=k) break;
            --counts[a[i]];++counts[t];
        }
        for(auto it=done.rbegin();it!=done.rend();++it) moveWheel(it->first,-it->second);
    }
    // Check regular arrangements while observations can still disprove them
    // cheaply. The subsequent occupancy-map strategy also works without them.
    void diagnose(int probe) {
        if(diagnosed || patterns.empty() || patterns.size()>8) return;
        diagnosed=true;
        for(int round=0;round<4 && !patterns.empty();++round) {
            int ways[50][50][3]{};
            auto delta=[&](const vector<int>& a,int i,int d) {
                int x=i?a[i]:probe, y=mod(x+d),at=0,to=0;
                for(int j=0;j<n;++j) {int p=j?a[j]:probe;at+=p==x;to+=p==y;}
                return (to==0)-(at==1);
            };
            for(auto &a:patterns) {
                int cnt[50]={}; for(int j=0;j<n;++j) ++cnt[j?a[j]:probe];
                for(int i=1;i<n;++i) for(int d=1;d<n;++d) {
                    int z=(cnt[mod(a[i]+d)]==0)-(cnt[a[i]]==1);
                    ++ways[i][d][1+z];
                }
            }
            double best=-1;int wheel=-1,shift=0;
            for(int i=1;i<n;++i) for(int d=1;d<n;++d) {
                auto &w=ways[i][d];double total=patterns.size();
                double score=(0.8*w[2]+0.12*w[1])/total;
                if(patterns.size()>1 && *max_element(w,w+3)==int(total)) continue;
                for(int z:w) if(z)score-=z/total*log2(z/total);
                if(score>best) {best=score;wheel=i;shift=d;}
            }
            if(wheel<0) break;
            int before=k;moveWheel(wheel,shift);
            erase_if(patterns,[&](auto &a){return before+delta(a,wheel,shift)!=k;});
            moveWheel(wheel,-shift);
            if(patterns.size()==1) {
                auto a=patterns.front();a[0]=probe;patterns.clear();tryConfiguration(a);break;
            }
        }
    }
    // Map the other n-1 wheels in coordinates relative to wheel zero's start.
    Mask survey(int &where) {
        vector<int> trial(n), seen(n,-1); iota(trial.begin(),trial.end(),0);
        shuffle(trial.begin()+1,trial.end(),rng);
        for(int i=1;i<min(5,n);++i) {
            int x=mod((i%2?1:-1)*((i+1)/2));
            iter_swap(trial.begin()+i,find(trial.begin()+i,trial.end(),x));
        }
        vector<Mask> models;
        for(auto &a:patterns) {
            Mask m=0;for(int i=1;i<n;++i)m|=bit(a[i]);models.push_back(m);
        }
        sort(models.begin(),models.end());models.erase(unique(models.begin(),models.end()),models.end());
        Mask subgroup=0;
        if(n%initial==0 && initial<n) for(int x=0;x<n;x+=n/initial) subgroup|=bit(x);
        for(int step=0;step<n;++step) {
            int x=trial[step];
            if(step && !models.empty()) {
                double best=-1;int idx=step;
                for(int j=step;j<n;++j) {
                    double score=0;
                    for(Mask m:models) {
                        int size=population(m); bool occ=m&bit(trial[j]);
                        score+=occ ? double(n-size)/size : double(size)/(n-size);
                    }
                    if(score>best) {best=score;idx=j;}
                }
                swap(trial[step],trial[idx]);x=trial[step];
            }
            if(subgroup && step>=2) {
                bool shared=true,alone=true;
                for(int j=0;j<n;++j) if(seen[j]>=0) {
                    shared &= seen[j]==initial+!(subgroup&bit(j));
                    alone &= seen[j]==initial-1+(!(subgroup&bit(j)) || j==0);
                }
                if(shared || alone) {
                    for(int j=step;j<n;++j) if(subgroup&bit(trial[j])) {
                        swap(trial[j],trial[step]);x=trial[step];break;
                    }
                } else subgroup=0;
            }
            if(seen[x]<0) {moveWheel(0,x-where); where=x; seen[x]=k;}
            erase_if(models,[&](Mask m){return population(m)+!(m&bit(x))!=seen[x];});
            int viable=0, knownSize=0; bool complete=false; Mask answer=0, yesInfo=0,noInfo=0;
            for(int size=initial-1;size<=min(n-1,initial);++size) {
                Mask yes=0,no=0; bool ok=true;
                for(int j=0;j<n;++j) if(seen[j]!=-1) {
                    if(seen[j]==size) yes|=bit(j);
                    else if(seen[j]==size+1) no|=bit(j);
                    else ok=false;
                }
                if(batchSize && originalCount==n) {
                    Mask holes=no&~Mask(1);
                    yes|=(rotated(holes,1)|rotated(holes,n-1))&~Mask(1);
                    if(yes&no) ok=false;
                }
                if(!ok || population(yes)>size || population(no)>n-size) continue;
                ++viable;knownSize=size;yesInfo=yes;noInfo=no;
                complete=population(yes)==size || population(no)==n-size;
                answer=population(yes)==size?yes:((bit(n)-1)^no);
            }
            if(viable==1 && complete) return answer;
            if(!subgroup && initial>4 && !diagnosed && step>=3 && !models.empty() && models.size()<=4) {
                erase_if(patterns,[&](auto &a){
                    Mask m=0;for(int j=1;j<n;++j)m|=bit(a[j]);
                    return find(models.begin(),models.end(),m)==models.end();
                });
                if(step>=6 || population(models.front())<n/4 || population(models.front())>n*3/4)
                    diagnose(where);
            }
            if(viable==1) for(int j=0;j<n;++j) {
                if(yesInfo&bit(j)) seen[j]=knownSize;
                if(noInfo&bit(j)) seen[j]=knownSize+1;
            }
        }
        abort();
    }
    void makePatterns() {
        set<vector<int>> valid;
        for(int form=0;form<3;++form) for(int step=1;step<n;++step) {
            vector<int>a(n); Mask m=0;
            for(int i=0;i<n;++i) {
                int v=form==0?i:(form==1?i%initial:i*initial/n);
                a[i]=v*step%n; if(i) m|=bit(a[i]);
            }
            if(population(m|1)==initial) valid.insert(a);
        }
        patterns.assign(valid.begin(),valid.end());
    }
    // Every accepted unit rotation vacates a distinct slot, and its successor
    // stays occupied. This gives a small common origin set for the batch.
    void batch() {
        rejectedDelta.fill(-1);
        if(n<18 || originalCount < n-max(1,n/12)) return;
        int desired=max(3,n/5);
        int startingModels=patterns.size();
        for(int i=1;i<n && batchSize<desired && i<=2*desired;++i) {
            int before=k;moveWheel(i,1);
            erase_if(patterns,[&](auto &a){
                Mask m=0;for(int j=0;j<n;++j)m|=bit(mod(a[j]+(i==j)));
                return population(m)!=k;
            });
            if(k==before-1) {
                order.push_back(i);++batchSize;
                for(auto &a:patterns) a[i]=mod(a[i]+1);
            } else {rejectedDelta[i]=k-before;moveWheel(i,-1);}
            if(i>=2 && patterns.size()*3<=size_t(startingModels)) diagnose(0);
        }
        initial=k;
    }
    void rejectionEvidence(int wheel) {
        conditionalLow.fill(0);conditionalHigh.fill(n);
        if(rejectedDelta[wheel]<0) return;
        Mask laterHoles=0;
        for(int j=0;j<batchSize;++j) {
            int i=order[j];Mask hole=bit(mod(orig[i]-1));
            if(i>wheel) laterHoles|=hole;
        }
        Mask occupied=initialSymbols|laterHoles;
        Mask laterArrivals=rotated(laterHoles,1);
        for(int x=0;x<n;++x) {
            int single=!(occupied&bit(mod(x+1)))-rejectedDelta[wheel];
            int other=removed[x]+(x==0)-bool(laterArrivals&bit(x));
            if(single<0) conditionalHigh[x]=0;
            else if(single) conditionalLow[x]=conditionalHigh[x]=1-other;
            else conditionalLow[x]=2-other;
        }
    }
    void tryPattern(int processed) {
        vector<int> a;
        if(patterns.size()==1 && processed>=(initial<4?6:2)) {
            a=patterns.front();patterns.clear();
        } else {
            for(auto &g:groups) if(g.valid && g.repeats>=confidence() && population(g.assigned)>=initial-1) {
                vector<int> slots=g.position;
                for(int &x:slots) if(x<0) x=first(initialSymbols^g.assigned);
                for(int label:g.label) a.push_back(slots[label]);
                g.valid=false;break;
            }
        }
        if(a.empty()) return;
        for(int i=0;i<n;++i) if(now[i]>=0) a[i]=now[i];
        tryConfiguration(a);
    }
    // Pick a resting place for a calibrated wheel. A nontrivial period could
    // make two origins observationally identical on the next wheel. If base
    // itself is periodic, adding one outside point always breaks the symmetry:
    // translating base either preserves it or changes at least four bits.
    // A translation preserving the augmented set must then fix the added point.
    int park(Mask base,int current) {
        int best=-1; double bestCost=1e99;
        int minimum=accumulate(lo,lo+n,0);Mask candidates=0;
        for(int x=0;x<n;++x) if(hi[x] && (lo[x] || minimum<left)) candidates|=bit(x);
        int count=population(candidates);
        int pairs[50]={};for(int d=1;d<n;++d) pairs[d]=population(candidates&rotated(candidates,d));
        for(int p=0;p<n;++p) {
            Mask m=base|bit(p);int minDiff=n,bestSplit=0;double similarity=0;
            for(int d=1;d<n;++d) {
                Mask shift=rotated(m,d);int diff=population(m^shift);
                minDiff=min(minDiff,diff);
                if(!diff) break;
                int hits=population(candidates&shift);
                bestSplit=max(bestSplit,min(hits,count-hits));
                similarity+=double(pairs[d])/(diff*diff);
            }
            if(!minDiff) continue;
            double cost=(p!=current)+(p!=target)+0.12*(p!=target);
            if(count>2 && left>3) {
                cost+=0.14*abs(population(m)-n*0.48);
                cost+=0.8*count/max(1,bestSplit);
                cost+=1.8*similarity/count;
                if(p!=target && hi[p]>0 && !fixed[p]) cost+=0.13;
            }
            if(useGuards && count<=8 && left>=3*count && (candidates&bit(p)) && !fixed[p])
                cost-=1.5;
            if(cost<bestCost) {bestCost=cost;best=p;}
        }
        return best;
    }

    // Conditional count distributions for the yet untouched wheels. Bounds,
    // rather than the probabilities, determine which outcomes remain possible.
    double weight[2][50]{};
    void weights() {
        double f[51][51]{}, b[51][51]{}, coeff[50][51]{};
        f[0][0]=b[n][0]=1;
        for(int x=0;x<n;++x) {
            double q=1;
            for(int c=0;c<=min(hi[x],left);++c) {
                if(c>=lo[x]) coeff[x][c]=q;
                q*=double(removed[x]+2+c)/(c+1);
            }
        }
        for(int i=0;i<n;++i) for(int j=0;j<=left;++j) {
            for(int c=lo[i];c<=min(j,hi[i]);++c) f[i+1][j]+=f[i][j-c]*coeff[i][c];
            int x=n-1-i;
            for(int c=lo[x];c<=min(j,hi[x]);++c) b[x][j]+=b[x+1][j-c]*coeff[x][c];
        }
        for(int x=0;x<n;++x) {
            weight[0][x]=weight[1][x]=0;
            for(int c=max({1,lo[x],conditionalLow[x]});c<=min({hi[x],left,conditionalHigh[x]});++c) {
                double ways=0;
                for(int j=0;j<=left-c;++j) ways+=f[x][j]*b[x+1][left-c-j];
                weight[c+fixed[x]==1][x]+=ways*coeff[x][c]*c/(f[n][left]*left);
            }
        }
    }
    // Also give weight to allocations with a single heavily repeated symbol.
    // Only the exact count bounds above are used to eliminate hypotheses.
    void weightHeavySymbol() {
        if(batchSize) return;
        int extra=n-1-population(originalOthers);
        if(extra<2) return;
        double heavy[2][50]={},total=0;int mostSeen=0;
        for(Mask m=originalOthers;m;m&=m-1) {
            int candidate=first(m);bool valid=true;
            int remaining[50]={};
            for(int x=0;x<n;++x) {
                remaining[x]=bool(originalOthers&bit(x))+(x==candidate?extra:0)-removed[x];
                if(remaining[x]<lo[x] || remaining[x]>hi[x]) {valid=false;break;}
            }
            if(!valid) continue;
            double prob=1;
            for(int j=0;j<removed[candidate];++j)prob*=1+extra-j;
            mostSeen=max(mostSeen,removed[candidate]);total+=prob;
            for(int x=0;x<n;++x) if(remaining[x])
                heavy[remaining[x]+fixed[x]==1][x]+=prob*remaining[x]/left;
        }
        if(total) {
            double scale=0.25+min(mostSeen,6)*0.34;
            for(int h=0;h<2;++h) for(int x=0;x<n;++x) weight[h][x]+=scale*heavy[h][x]/total;
        }
    }
    // m[0] and m[1] contain possible origins at which the active wheel
    // respectively shares its symbol or is its only occupant.
    struct Options { Mask m[2]{}; };
    Options split[50][3];
    double table[2][7][256];
    double mass(Options a) const {
        double w=0;
        for(int h=0;h<2;++h) for(int b=0;a.m[h];++b,a.m[h]>>=8) w+=table[h][b][a.m[h]&255];
        return w;
    }
    void prepare(int wheel) {
        if(!patterns.empty()) {
            int votes[50]={}; for(auto &p:patterns) ++votes[p[wheel]];
            for(int x=0;x<n;++x) {
                double w=weight[0][x]+weight[1][x];
                if(w && votes[x]) for(int h=0;h<2;++h)
                    weight[h][x]*=1+0.28*votes[x]/(patterns.size()*w);
            }
        }
        for(auto &g:groups) if(g.valid) {
            int y=g.position[g.label[wheel]];
            Mask possible=y<0?(initialSymbols^g.assigned):bit(y);
            double w=0;
            for(Mask m=possible;m;m&=m-1) w+=weight[0][first(m)]+weight[1][first(m)];
            if(w>0) for(Mask m=possible;m;m&=m-1) for(int h=0;h<2;++h)
                weight[h][first(m)]*=1+min(2.0,0.15*(1<<min(4,g.repeats)))/w;
        }
        for(int h=0;h<2;++h) for(int b=0;b*8<n;++b) {
            auto &row=table[h][b];row[0]=0;
            for(int v=1;v<256;++v) {
                int x=8*b+first(v);row[v]=row[v&(v-1)]+(x<n?weight[h][x]:0);
            }
        }
        // Relative to the count at offset zero, a nonzero offset contributes
        // [destination was empty] - [origin was a singleton].
        for(int d=1;d<n;++d) {
            Mask occupied=rotated(map,n-d), empty=(bit(n)-1)^occupied;
            split[d][0]={{0,occupied}};
            split[d][1]={{occupied,empty}};
            split[d][2]={{empty,0}};
        }
    }
    static Options intersect(Options a, Options b) {return {{a.m[0]&b.m[0],a.m[1]&b.m[1]}};}
    static int size(Options a) {return population(a.m[0])+population(a.m[1]);}
    // Optimize the complete remaining decision tree when few hypotheses survive.
    void exact(Options &a,int wheel,int start,int &offset) {
        int num=size(a), all=(1<<num)-1;
        vector<array<int,3>> parts(n);
        vector<int> aligned(n); vector<double>w(1<<num), dp(1<<num); vector<int> action(1<<num);
        int idx=0;
        for(int h=0;h<2;++h) for(Mask m=a.m[h];m;m&=m-1,++idx) {
            int x=first(m), b=1<<idx; w[b]=weight[h][x];
            for(int d=1;d<n;++d) {
                int y=mod(x+d); int z=1+!(map&bit(y))-h;
                parts[d][z]|=b; if(y==target) aligned[d]|=b;
            }
        }
        for(int s=1;s<=all;++s) if(s&(s-1)) w[s]=w[s&(s-1)]+w[s&-s];
        auto solve=[&](auto&& self,int s)->double {
            if(action[s]) return dp[s];
            dp[s]=1e50;
            for(int d=1;d<n;++d) {
                double cost=w[s];bool useful=true;
                for(int z:parts[d]) {
                    int sub=z&s;
                    if(sub==s) {useful=false;break;}
                    if(sub&(sub-1)) cost+=self(self,sub);
                    else if(!(sub&aligned[d])) cost+=w[sub];
                }
                if(useful && cost<dp[s]-1e-13*w[s]) {dp[s]=cost;action[s]=d;}
            }
            return dp[s];
        };
        solve(solve,all);
        for(int s=all;s&(s-1);) {
            int d=action[s];moveWheel(wheel,d-offset);offset=d;
            int z=k-start+1;s&=parts[d][z];a=intersect(a,split[d][z]);
        }
    }
    double splitCost(Options a,int d) const {
        double score=0;
        for(auto mask:split[d]) {
            Options sub=intersect(a,mask);
            if(sub.m[0]==a.m[0] && sub.m[1]==a.m[1]) return 1e50;
            double w=mass(sub); if(w==0) continue;
            score+=w*log2(w);
            if(size(sub)==1 && ((sub.m[0]|sub.m[1])&bit(mod(target-d)))) score-=w;
        }
        return score;
    }
    int choose(Options a) const {
        vector<pair<double,int>> options;
        for(int d=1;d<n;++d) {
            double score=splitCost(a,d);
            if(score<1e49) options.emplace_back(score,d);
        }
        int count=min<int>(options.size(),8);
        partial_sort(options.begin(),options.begin()+count,options.end());
        double best=1e100;int res=-1;
        for(int j=0;j<count;++j) {
            auto [immediate,d]=options[j];double score=0.12*immediate;
            for(auto mask:split[d]) {
                Options sub=intersect(a,mask);int num=size(sub); if(!num) continue;
                double w=mass(sub);
                if(num==1) {
                    score+=w*log2(w);
                    if((sub.m[0]|sub.m[1])&bit(mod(target-d))) score-=w;
                } else {
                    double next=1e100;
                    for(int e=1;e<n;++e) next=min(next,splitCost(sub,e));
                    score+=w+next;
                }
            }
            if(score<best) {best=score;res=d;}
        }
        return res;
    }
    void run() {
        makePatterns();
        batch();
        int p=0;
        Mask others=survey(p);
        initialSymbols=others|1;
        originalOthers=others;
        erase_if(patterns,[&](const auto &a){
            Mask m=0; for(int j=1;j<n;++j)m|=bit(a[j]); return m!=others;
        });
        if(!batchSize && initial<n) for(int form=0;form<2;++form) {
            Grouping g; g.position.assign(initial,-1); int same=0;
            for(int i=0;i<n;++i) {int y=form?i*initial/n:i%initial;g.label.push_back(y);same+=y==0;}
            if((same>1)==bool(others&1)) groups.push_back(g);
        }
        learnGroups(0,0);
        for(int x=0;x<n;++x) if(others&bit(x)) {lo[x]=1;hi[x]=left;}
        batchSlots=rotated((bit(n)-1)^initialSymbols,1)&others;
        if(batchSize && originalCount==n) for(int x=0;x<n;++x)
            lo[x]=hi[x]=bool(initialSymbols&bit(x))+bool(batchSlots&bit(x))-(x==0);
        orig[0]=0;
        // Favor the starting symbol if the probe shared it with another wheel.
        target=(others&1)?0:((others&bit(p))?p:first(others));
        for(int d=1;d<n;++d) useGuards|=rotated(initialSymbols,d)==initialSymbols;
        useGuards |= initial>=3 && initial<=6 && n>=5*initial;
        if(useGuards && originalCount<n && !batchSize) {
            int best=n*n;
            for(int t=0;t<n;++t) if(!(initialSymbols&bit(t))) {
                int collisions=0;
                for(Mask m=others;m;m&=m-1)
                    collisions+=population(rotated(others,mod(t-first(m)))&initialSymbols);
                if(collisions<best) {best=collisions;target=t;}
            }
        }
        if(population(others)==1) {
            vector<int>a(n,first(others));a[0]=p;finish(a);
        }
        int q=park(others,p);
        moveWheel(0,q-p);
        now[0]=q;
        ++fixed[q];
        map=others|bit(q);
        for(int i=1;i<n;++i) if(find(order.begin(),order.end(),i)==order.end()) order.push_back(i);
        shuffle(order.begin()+max(1,batchSize),order.end(),rng);
        if(batchSize) stable_partition(order.begin()+batchSize,order.end(),[&](int i){return rejectedDelta[i]>=0;});
        for(int index=0;index<(int)order.size();++index) {
            if(!batchSize) chooseWheel(index);
            int i=order[index];
            rejectionEvidence(i);
            weights();
            weightHeavySymbol();
            Options a;
            for(int h=0;h<2;++h) for(int x=0;x<n;++x) if(weight[h][x]>0) a.m[h]|=bit(x);
            if(index<batchSize) {
                a.m[0]&=batchSlots;
                a.m[1]&=batchSlots&1;
            }
            prepare(i);
            int start=k,offset=0;
            while(size(a)>13) {
                int d=choose(a);moveWheel(i,d-offset);offset=d;
                a=intersect(a,split[d][k-start+1]);
            }
            if(size(a)>1) exact(a,i,start,offset);
            int x=first(a.m[0]|a.m[1]);
            bool single=a.m[1]!=0;
            orig[i]=x;
            ++removed[x];
            --left;
            lo[x]=max(lo[x],conditionalLow[x]); hi[x]=min(hi[x],conditionalHigh[x]);
            if(single) lo[x]=hi[x]=0;
            else {lo[x]=max(lo[x]-1,int(!fixed[x]));--hi[x];}
            Mask base=single?(map^bit(x)):map;
            int cur=mod(x+offset);
            if(index<batchSize) {
                batchSlots &= ~bit(x);
                if(x!=0) lo[x]=max(lo[x],1);
            }
            int minimum=accumulate(lo,lo+n,0);Mask rest=0;
            for(int j=0;j<n;++j) if(hi[j] && (lo[j] || minimum<left)) rest|=bit(j);
            if(population(rest)<=1) {
                vector<int> v=now;v[i]=cur;
                for(int &y:v) if(y<0)y=first(rest);
                finish(v);
            }
            q=park(base,cur);
            moveWheel(i,q-cur);
            now[i]=q;
            ++fixed[q];
            map=base|bit(q);
            erase_if(patterns,[&](auto &pat){return pat[i]!=x;});
            learnGroups(i,x);
            tryPattern(n-1-left);
        }
        finish(now);
    }
};
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>n>>k) || n<3 || n>50 || k<=1 || k>n) return 0;
    rng.seed(982723u+1009*n+k);
    if(n<=4) tinySolver();
    smallSolver();
    Solver solver;
    solver.run();
}
