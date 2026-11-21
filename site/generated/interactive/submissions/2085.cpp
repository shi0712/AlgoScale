#include <bits/stdc++.h>
using namespace std;

using Num = long long;
constexpr Num LIMIT = 1000000000LL;
constexpr Num INF = 4000000000LL;
using Triple = array<int, 3>;

int n, alive, remainingPairs, learningBudget;
vector<bool> liveIndex;
map<Triple, Num> cache;
mt19937 rng(827513);

void excludeIndex(int i) {
    if (liveIndex[i]) {
        liveIndex[i] = false;
        --alive;
    }
}

void finishIfReady() {
    if (alive > 2) return;
    vector<int> answer;
    for (int i = 0; i < n; ++i)
        if (liveIndex[i]) answer.push_back(i + 1);
    if (answer.empty()) exit(0);
    cout << "! " << answer.front() << ' ' << answer.back() << endl;
    exit(0);
}

Triple key(int i, int j, int k) {
    Triple t{i, j, k};
    sort(t.begin(), t.end());
    return t;
}

Num ask(Triple t) {
    if (auto it = cache.find(t); it != cache.end()) return it->second;
    cout << "? " << t[0]+1 << ' ' << t[1]+1 << ' ' << t[2]+1 << endl;
    Num r;
    if (!(cin >> r) || r < 0) exit(0);
    cache[t] = r;
    if (r == 0) for (int i : t) excludeIndex(i);
    if (r == LIMIT)
        for (int i = 0; i < n; ++i)
            if (!binary_search(t.begin(), t.end(), i)) excludeIndex(i);
    finishIfReady();
    if (r==LIMIT) {
        // This triple contains both absolute endpoints. If deleting one of
        // its entries still gives LIMIT, the two retained entries cover zero.
        int other=0;
        while (binary_search(t.begin(),t.end(),other)) ++other;
        ask(key(t[1],t[2],other));
        ask(key(t[0],t[2],other));
        excludeIndex(t[2]); // Both deleted entries were unique endpoints.
        finishIfReady();
    }
    return r;
}

// The coordinates may be translated and reflected. All stored coordinates
// use one common choice of origin and direction.
map<Num, vector<int>> at;
vector<Num> coordinate;
void locate(int i, Num v) {
    if (coordinate[i] != INF) return;
    auto &ids = at[v];
    if (!ids.empty()) {
        excludeIndex(ids.front());
        excludeIndex(i);
    }
    ids.push_back(i);
    coordinate[i] = v;
    auto it = at.find(v);
    auto dropInterior = [](auto pos) {
        if (pos != at.begin() && next(pos) != at.end())
            excludeIndex(pos->second.front());
    };
    dropInterior(it);
    if (it != at.begin()) dropInterior(prev(it));
    if (next(it) != at.end()) dropInterior(next(it));
}

struct BootstrapOrder {
    array<int, 8> order;
    Num upper[8][8]; // upper[i][j] bounds value[j] - value[i].
};

bool bootstrap(const vector<int>& ids, bool equalFirstTwo) {
    const int m = ids.size();
    vector<Triple> triples;
    for (int k = 2; k < m; ++k)
        for (int j = 1; j < k; ++j)
            for (int i = 0; i < j; ++i) triples.push_back({i,j,k});
    while (true) {
        vector<pair<Triple,Num>> seen;
        vector<Triple> unused;
        for (auto t : triples) {
            auto it = cache.find(key(ids[t[0]],ids[t[1]],ids[t[2]]));
            if (it == cache.end()) unused.push_back(t);
            else seen.emplace_back(t,it->second);
        }
        bool canBeMin[8]{};
        Num minDist[8][8], maxDist[8][8]{};
        for (auto &row : minDist) fill(begin(row),end(row),INF);
        vector<BootstrapOrder> orders;
        array<int,8> perm{0,1,2,3,4,5,6,7};
        do {
            BootstrapOrder w{};
            w.order = perm;
            int pos[8]{};
            for (int j=0; j<m; ++j) pos[perm[j]]=j;
            auto &d=w.upper;
            for (int i=0; i<m; ++i)
                for (int j=0; j<m; ++j) d[i][j]=pos[i]<pos[j] ? LIMIT : 0;
            if (m==n) d[perm[1]][perm[0]]=-1;
            if (equalFirstTwo) d[0][1]=d[1][0]=0;
            bool valid=true;
            for (auto [t,r] : seen) {
                auto [a,b]=minmax_element(t.begin(),t.end(),[&](int i,int j){return pos[i]<pos[j];});
                if (d[*a][*b]<r || d[*b][*a]<-r) {valid=false; break;}
                d[*a][*b]=r; d[*b][*a]=-r;
            }
            if (!valid) continue;
            for (int k=0; k<m; ++k)
                for (int i=0; i<m; ++i)
                    for (int j=0; j<m; ++j)
                        d[i][j]=min(d[i][j],d[i][k]+d[k][j]);
            for (int i=0; i<m; ++i) if (d[i][i]<0) valid=false;
            if (!valid) continue;
            if (d[perm[0]][perm[1]]>=1) canBeMin[perm[0]]=true;
            for (int i=0; i<m; ++i)
                for (int j=i+1; j<m; ++j) {
                    int a=i,b=j;
                    if (pos[a]>pos[b]) swap(a,b);
                    minDist[i][j]=min(minDist[i][j],-d[b][a]);
                    maxDist[i][j]=max(maxDist[i][j],d[a][b]);
                }
            orders.push_back(w);
        } while (next_permutation(perm.begin(),perm.begin()+m));
        for (int i=0; i<m; ++i) if (!canBeMin[i]) excludeIndex(ids[i]);
        finishIfReady();
        auto dist = [&](int i,int j) -> Num {
            if (i==j) return 0;
            if (i>j) swap(i,j);
            return minDist[i][j]==maxDist[i][j] ? minDist[i][j] : INF;
        };
        for (int a=0; a<m; ++a) for (int b=a+1; b<m; ++b) {
            Num width=dist(a,b);
            if (width==0 || width==INF) continue;
            vector<pair<int,Num>> fixed;
            bool covers=true;
            for (int i=0; i<m; ++i) {
                Num da=dist(a,i),db=dist(b,i);
                if (da==INF || db==INF) {
                    if (liveIndex[ids[i]]) covers=false;
                } else fixed.emplace_back(ids[i],db==width+da ? -da : da);
            }
            if (m!=n && fixed.size()>=3 && covers) {
                for (auto [i,v] : fixed) locate(i,v);
                finishIfReady();
                return true;
            }
        }
        if (unused.empty()) return false;
        // Prefer queries whose range is still free in many possible orders.
        double best=-numeric_limits<double>::infinity();
        Triple choice{};
        for (auto t : unused) {
            // For a complete small case, the minimum is the zero. Sweep the
            // possible reply intervals to estimate how many candidates survive.
            struct Event { array<int,8> changes{}; double density=0; };
            map<Num,Event> events;
            Num smallest=INF, largest=-1;
            double score=0;
            for (auto &w : orders) {
                int a=-1,b=-1;
                for (int j=0; j<m; ++j) {
                    int i=w.order[j];
                    if (!binary_search(t.begin(),t.end(),i)) continue;
                    if (a<0) a=i;
                    b=i;
                }
                Num lo=-w.upper[b][a], hi=w.upper[a][b];
                smallest=min(smallest,lo); largest=max(largest,hi);
                score+=hi>lo;
                if (m==n) {
                    auto &start=events[lo], &end=events[hi+1];
                    ++start.changes[w.order[0]]; --end.changes[w.order[0]];
                    double density=1.0/(hi-lo+1);
                    start.density+=density; end.density-=density;
                }
            }
            if (smallest>=largest) continue;
            if (m==n) {
                array<int,8> active{};
                double density=0, cost=0;
                for (auto it=events.begin(); it!=events.end(); ++it) {
                    for (int i=0; i<m; ++i) active[i]+=it->second.changes[i];
                    density+=it->second.density;
                    auto nextEvent=next(it);
                    if (nextEvent==events.end()) break;
                    int candidates=count_if(active.begin(),active.begin()+m,[](int a){return a>0;});
                    if (candidates>2) {
                        int possibilities=accumulate(active.begin(),active.begin()+m,0);
                        cost+=density*(nextEvent->first-it->first)*(candidates+0.15*log2(possibilities));
                    }
                }
                score=0.002*score-cost;
            }
            if (score>best) {best=score;choice=t;}
        }
        if (!isfinite(best)) return false;
        ask(key(ids[choice[0]],ids[choice[1]],ids[choice[2]]));
    }
}

// Bounds on x, y, and y-x describe a convex set of possible placements.
struct Zone {
    Num xl,xh,yl,yh,dl,dh;
    auto operator<=>(const Zone&) const = default;

    bool tighten() {
        xl=max(xl,yl-dh); xh=min(xh,yh-dl);
        yl=max(yl,xl+dl); yh=min(yh,xh+dh);
        dl=max(dl,yl-xh); dh=min(dh,yh-xl);
        return xl<=xh && yl<=yh && dl<=dh;
    }
};

struct Probe {
    Triple indices;
    int kind; // 0 uses x and y, 1 uses only x, 2 uses only y.
    Num a,b;
    Num result(Num x,Num y) const {
        Num u=kind==2 ? y : x, v=kind==0 ? y : b;
        return max({a,u,v})-min({a,u,v});
    }
};

vector<Zone> observe(const vector<Zone>& worlds, const Probe& p, Num r) {
    vector<Zone> out;
    auto add=[&](Zone z) {
        if (z.tighten() && find(out.begin(),out.end(),z)==out.end()) out.push_back(z);
    };
    for (auto z : worlds) {
        if (p.kind==0) {
            z.xl=max(z.xl,p.a-r); z.xh=min(z.xh,p.a+r);
            z.yl=max(z.yl,p.a-r); z.yh=min(z.yh,p.a+r);
            z.dl=max(z.dl,-r); z.dh=min(z.dh,r);
            if (!z.tighten()) continue;
            // At least one of the three pairwise distances equals the reply.
            if (z.xl==p.a-r) {auto w=z; w.xh=w.xl; add(w);}
            if (z.xh==p.a+r) {auto w=z; w.xl=w.xh; add(w);}
            if (z.yl==p.a-r) {auto w=z; w.yh=w.yl; add(w);}
            if (z.yh==p.a+r) {auto w=z; w.yl=w.yh; add(w);}
            if (z.dl==-r) {auto w=z; w.dh=w.dl; add(w);}
            if (z.dh==r) {auto w=z; w.dl=w.dh; add(w);}
        } else {
            if (r<p.b-p.a) continue;
            Num &lo=p.kind==1 ? z.xl : z.yl, &hi=p.kind==1 ? z.xh : z.yh;
            lo=max(lo,p.b-r); hi=min(hi,p.a+r);
            if (r==p.b-p.a) add(z);
            else if (lo<=hi) {
                if (lo==p.b-r) {auto w=z; (p.kind==1 ? w.xh : w.yh)=lo; add(w);}
                if (hi==p.a+r) {auto w=z; (p.kind==1 ? w.xl : w.yl)=hi; add(w);}
            }
        }
    }
    return out;
}

struct Bounds {
    Num xl=INF,xh=-INF,yl=INF,yh=-INF;
    bool x=false,y=false,low=false,high=false;
};

Bounds summarize(const vector<Zone>& worlds,Num l,Num h,bool single) {
    Bounds s;
    for (auto &z : worlds) {
        s.xl=min(s.xl,z.xl); s.xh=max(s.xh,z.xh);
        s.yl=min(s.yl,z.yl); s.yh=max(s.yh,z.yh);
        s.x |= (z.xl<l && z.dh>0) || (z.xh>h && z.dl<0);
        s.y |= (z.yl<l && z.dl<0) || (z.yh>h && z.dh>0);
        s.low |= z.xh>l && (single || z.yh>l);
        s.high |= z.xl<h && (single || z.yl<h);
    }
    return s;
}

auto closest(Num target) {
    auto it=at.lower_bound(target);
    if (it==at.end()) return prev(it);
    if (it!=at.begin() && target-prev(it)->first < it->first-target) return prev(it);
    return it;
}

int centralAnchor() {
    Num sum=at.begin()->first+at.rbegin()->first;
    auto it=at.lower_bound((sum+1)/2);
    if (it==at.end()) return at.rbegin()->second.front();
    if (it!=at.begin()) {
        auto before=prev(it);
        Num d1=abs(2*before->first-sum),d2=abs(2*it->first-sum);
        if (d1<d2 || (d1==d2 && before->second.size()>it->second.size())) it=before;
    }
    return it->second.front();
}

void reduceFour();

void processPair(int ix,int iy,int earlierAnchor=-1) {
    const bool single=iy<0;
    const int low=at.begin()->second.front(), high=at.rbegin()->second.front();
    const Num l=coordinate[low],h=coordinate[high],width=h-l;
    int mid=centralAnchor();
    if (single) iy=mid;
    Zone initial{h-LIMIT,l+LIMIT,h-LIMIT,l+LIMIT,-LIMIT,LIMIT};
    if (single) initial.yl=initial.yh=coordinate[iy];
    initial.tighten();
    vector<Zone> worlds{initial};
    vector<int> anchors;
    auto addPosition=[&](auto it) {
        for (int j=0;j<min(2,(int)it->second.size());++j) {
            int i=it->second[j];
            if (find(anchors.begin(),anchors.end(),i)==anchors.end()) anchors.push_back(i);
        }
    };
    addPosition(at.begin()); addPosition(prev(at.end()));
    addPosition(at.find(coordinate[mid]));
    if (earlierAnchor>=0) addPosition(at.find(coordinate[earlierAnchor]));
    addPosition(closest(l+width/4)); addPosition(closest(h-width/4));
    if (anchors.size()<3) {
        // Include an interior point even when both quartiles round to an end.
        if (at.size()>2) addPosition(next(at.begin()));
    }
    vector<Probe> probes;
    for (int i : anchors) {
        if (!single) probes.push_back({key(ix,iy,i),0,coordinate[i],0});
        for (int j : anchors) if (j<i) {
            auto [a,b]=minmax(coordinate[i],coordinate[j]);
            probes.push_back({key(ix,i,j),1,a,b});
            if (!single) probes.push_back({key(iy,i,j),2,a,b});
        }
    }
    vector<bool> used(probes.size());
    bool haveInfo=false;
    for (int i=0; i<(int)probes.size(); ++i) {
        auto it=cache.find(probes[i].indices);
        if (it!=cache.end()) {
            worlds=observe(worlds,probes[i],it->second);
            used[i]=true; haveInfo=true;
        }
    }
    bool triedLearning=false;
    while (true) {
        Bounds s=summarize(worlds,l,h,single);
        if (!s.x) excludeIndex(ix);
        if (!single && !s.y) excludeIndex(iy);
        if (!s.low) excludeIndex(low);
        if (!s.high) excludeIndex(high);
        finishIfReady();
        bool needX=liveIndex[ix] && s.xl!=s.xh;
        bool needY=!single && liveIndex[iy] && s.yl!=s.yh;
        if (!needX && !needY) {
            // A more central reference can certify many subsequent pairs.
            // Occasionally localize an already safe point for that purpose.
            Num error=abs(2*coordinate[mid]-l-h);
            if (!triedLearning && !single && learningBudget>0 && remainingPairs>12
                && error>width/sqrt(double(remainingPairs)) && s.xl<s.xh
                && s.xh-s.xl<=3*error) {
                Num bestPossible=INF;
                for (const auto &z : worlds)
                    bestPossible=min(bestPossible,max({0LL,2*z.xl-l-h,l+h-2*z.xh}));
                if (3*bestPossible<2*error) {
                    for (int i=0; i<(int)probes.size(); ++i) {
                        auto &p=probes[i];
                        if (used[i] || p.kind!=1 || (p.b>s.xl && p.a<s.xh)) continue;
                        --learningBudget;
                        used[i]=triedLearning=true;
                        worlds=observe(worlds,p,ask(p.indices));
                        break;
                    }
                    if (triedLearning) continue;
                }
            }
            if (s.xl==s.xh) locate(ix,s.xl);
            if (!single && s.yl==s.yh) locate(iy,s.yl);
            finishIfReady();
            return;
        }
        int chosen=-1;
        if (!haveInfo) {
            Triple preferred=single ? key(low,high,ix) : key(mid,ix,iy);
            for (int i=0; i<(int)probes.size(); ++i)
                if (probes[i].indices==preferred) {chosen=i;break;}
        }
        if (chosen<0) {
            // Try feasible placements inside the current span and just outside.
            // These samples choose the probe only. Elimination uses every zone.
            struct Sample { Num x,y; double weight; };
            vector<Sample> samples;
            for (const Zone& w : worlds) {
                for (int mode=0; mode<3; ++mode) {
                    Zone z=w;
                    Num margin=mode==0 ? 0 : max(1LL,width/5);
                    if (mode!=2) {
                        z.xl=max(z.xl,l-margin); z.xh=min(z.xh,h+margin);
                        z.yl=max(z.yl,l-margin); z.yh=min(z.yh,h+margin);
                        if (!z.tighten()) continue;
                    }
                    double weight=mode==0 ? 24.0 : mode==1 ? 3.0 : 0.02;
                    if (z.xl==z.xh && z.yl==z.yh && width>10) weight*=0.1;
                    for (int f : {0,2,4}) {
                        Num x=z.xl+(z.xh-z.xl)*f/4;
                        Num a=max(z.yl,x+z.dl), b=min(z.yh,x+z.dh);
                        Num y=a+(b-a)*(4-f)/4;
                        samples.push_back({x,y,weight});
                    }
                }
            }
            double best=numeric_limits<double>::infinity();
            for (int i=0; i<(int)probes.size(); ++i) if (!used[i]) {
                auto &p=probes[i];
                map<Num,double> freq;
                for (auto sample : samples) freq[p.result(sample.x,sample.y)]+=sample.weight;
                double score=0;
                for (auto [r,f] : freq) {
                    auto after=observe(worlds,p,r);
                    Bounds b=summarize(after,l,h,single);
                    bool xCan=liveIndex[ix] && b.x;
                    bool yCan=!single && liveIndex[iy] && b.y;
                    int count=alive-(liveIndex[ix] && !xCan)
                                   -(!single && liveIndex[iy] && !yCan)
                                   -(liveIndex[low] && !b.low)
                                   -(liveIndex[high] && !b.high);
                    if (count<=2 || r==LIMIT) continue;
                    int missing=(xCan && b.xl!=b.xh)+(yCan && b.yl!=b.yh);
                    if (!missing) continue;
                    double uncertainty=0;
                    if (xCan) uncertainty+=log1p(double(b.xh-b.xl));
                    if (yCan) uncertainty+=log1p(double(b.yh-b.yl));
                    score+=f*(1+0.4*(missing-1)+0.006*uncertainty);
                }
                if (score<best) {best=score;chosen=i;}
            }
        }
        if (chosen<0) { reduceFour(); return; }
        used[chosen]=true;
        Num r=ask(probes[chosen].indices);
        worlds=observe(worlds,probes[chosen],r);
        haveInfo=true;
    }
}

// Three queried triples from four entries suffice to keep at most two
// candidates. At least one covers the entire range. Omitting a unique
// extreme strictly decreases the reply.
void reduceFour() {
    while (alive>2) {
        vector<int> v;
        for (int i=0; i<n && v.size()<4; ++i) if (liveIndex[i]) v.push_back(i);
        for (int i=0; v.size()<4; ++i) if (!liveIndex[i]) v.push_back(i);
        array<Num,3> r;
        for (int j=0; j<3; ++j) r[j]=ask(key(v[(j+1)%4],v[(j+2)%4],v[(j+3)%4]));
        Num d=*max_element(r.begin(),r.end());
        int lower=0;
        for (int j=0; j<3; ++j) {
            if (r[j]==d) excludeIndex(v[j]);
            else ++lower;
        }
        if (lower==2) excludeIndex(v[3]);
        finishIfReady();
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin>>n) || n<4) return 0;
    alive=n; liveIndex.assign(n,true); coordinate.assign(n,INF);
    vector<int> seed;
    for (int i : {0,n-1,(n-1)/2,(n-1)/4,3*(n-1)/4}) {
        if (find(seed.begin(),seed.end(),i)==seed.end()) seed.push_back(i);
    }
    for (int i=0; i<n && seed.size()<5; ++i)
        if (find(seed.begin(),seed.end(),i)==seed.end()) seed.push_back(i);
    Triple t=key(seed[0],seed[1],seed[2]);
    bool equal=ask(t)==0;
    if (n<=8 && !equal) {
        for (int i=0; i<n; ++i)
            if (find(seed.begin(),seed.end(),i)==seed.end()) seed.push_back(i);
        bootstrap(seed,false);
        reduceFour();
        return 0;
    }
    vector<int> rest;
    for (int i=0; i<n; ++i)
        if (find(t.begin(),t.end(),i)==t.end()) rest.push_back(i);
    vector<bool> screened(n);
    if (equal) {
        // Disjoint constant triples eliminate three indices per reply.
        vector<int> mixed;
        if (rest.size()==3) {
            ask(key(t[0],t[1],rest[0]));
            mixed=rest;
        }
        int balance=3;
        for (int j=0; rest.size()>3 && j+2<(int)rest.size(); j+=3) {
            auto q=key(rest[j],rest[j+1],rest[j+2]);
            for (int i : q) screened[i]=true;
            if (ask(q)==0) balance=min(8,balance+2);
            else {
                if (mixed.empty()) mixed.assign(q.begin(),q.end());
                if (--balance==0) break;
            }
        }
        if (mixed.empty()) {finishIfReady(); return 0;}
        seed={t[0],t[1],mixed[0],mixed[1],mixed[2]};
    }
    if (!bootstrap(seed,equal)) {reduceFour(); return 0;}

    // A constant triple is a stronger certificate than two bounded positions.
    // Try more disjoint triples when calibration found a frequent equal value.
    int repeats=0;
    for (const auto& entry : at) repeats=max(repeats,(int)entry.second.size());
    if (repeats>=3 || equal) {
        vector<int> fresh;
        for (int i=0; i<n; ++i)
            if (!screened[i] && liveIndex[i] && coordinate[i]==INF) fresh.push_back(i);
        int balance=2;
        for (int j=0; j+2<(int)fresh.size() && balance>0; j+=3) {
            Num r=ask(key(fresh[j],fresh[j+1],fresh[j+2]));
            balance=r==0 ? min(balance+2,8) : balance-1;
        }
    }
    vector<int> pending;
    for (int i=0; i<n; ++i) if (liveIndex[i] && coordinate[i]==INF) pending.push_back(i);
    // Shuffle pairs as blocks to sample different parts of the input early
    // while preserving the benefit of locally similar values.
    vector<pair<int,int>> pairs;
    for (int j=0; j+1<(int)pending.size(); j+=2) pairs.emplace_back(pending[j],pending[j+1]);
    shuffle(pairs.begin(),pairs.end(),rng);
    // Within a batch, resolve the largest spans first. A new extreme can
    // then certify smaller spans using their original replies.
    learningBudget=(int)sqrt(pairs.size());
    for (int start=0; start<(int)pairs.size();) {
        int count=min((int)pairs.size()-start,min(48,3+start/2));
        remainingPairs=pairs.size()-start;
        int anchor=centralAnchor();
        vector<tuple<Num,int,int>> batch;
        for (int j=start; j<start+count; ++j) {
            auto [x,y]=pairs[j];
            if (!liveIndex[x] && !liveIndex[y]) continue;
            Num r=ask(key(anchor,x,y));
            batch.emplace_back(r,x,y);
        }
        sort(batch.rbegin(),batch.rend());
        for (auto [r,x,y] : batch) processPair(x,y,anchor);
        start+=count;
    }
    if (pending.size()%2) processPair(pending.back(),-1);
    finishIfReady();
    reduceFour();
}
