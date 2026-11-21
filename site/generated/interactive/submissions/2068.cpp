#include <bits/stdc++.h>
using namespace std;

using U64 = uint64_t;
struct Signal { int u = 0, k = 0; };
struct Option { Signal q; int inside; U64 part; };

int n;
vector<vector<int>> adj, rmq, up;
vector<int> dep, firstVisit, pastVisit;
vector<U64> tag;
U64 seedValue = 92413761;
U64 randomWord() {
    U64 z = (seedValue += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}
void initTree() {
    dep.resize(n+1);
    up.assign(bit_width(unsigned(n)), vector<int>(n+1));
    adj.resize(n+1);
    up[0][1] = 1;
    for (int i=2; i<=n; ++i) {
        int p;
        if (!(cin >> p)) exit(0);
        up[0][i]=p; dep[i]=dep[p]+1;
        adj[p].push_back(i); adj[i].push_back(p);
    }
    for (int j=1; j<(int)up.size(); ++j)
        for (int i=1; i<=n; ++i) up[j][i]=up[j-1][up[j-1][i]];

    vector<int> tour, stack{1}, iter(n+1);
    firstVisit.resize(n+1); pastVisit.resize(n+1);
    while (!stack.empty()) {
        int v=stack.back();
        if (iter[v]==0) firstVisit[v]=tour.size();
        tour.push_back(v);
        if (iter[v]<(int)adj[v].size() && adj[v][iter[v]]==up[0][v]) ++iter[v];
        if (iter[v]==(int)adj[v].size()) { pastVisit[v]=tour.size(); stack.pop_back(); }
        else stack.push_back(adj[v][iter[v]++]);
    }
    rmq.push_back(move(tour));
    for (int j=1; (1<<j)<=(int)rmq[0].size(); ++j) {
        int len=rmq[0].size()-(1<<j)+1;
        vector<int> row(len);
        for (int i=0; i<len; ++i) {
            int a=rmq[j-1][i], b=rmq[j-1][i+(1<<(j-1))];
            row[i]=dep[a]<dep[b]?a:b;
        }
        rmq.push_back(move(row));
    }
    tag.resize(n+1);
    for (auto &x:tag) x=randomWord();
}
inline int lca(int a, int b) {
    int i=firstVisit[a], j=firstVisit[b];
    if (i>j) swap(i,j);
    int h=bit_width(unsigned(j-i+1))-1;
    int x=rmq[h][i], y=rmq[h][j-(1<<h)+1];
    return dep[x]<dep[y]?x:y;
}
inline int dist(int a, int b) {
    return dep[a]+dep[b]-2*dep[lca(a,b)];
}
int lift(int v, int d) {
    while (d) {
        v=up[countr_zero(unsigned(d))][v];
        d &= d-1;
    }
    return v;
}
int onPath(int a, int b, int ancestor, int d, int t) {
    return t<=dep[a]-dep[ancestor]?lift(a,t):lift(b,d-t);
}

// For a live set, this index counts any ball by centroid inclusion-exclusion.
class BallIndex {
    struct Entry { int d, all, sub; };
    vector<vector<Entry>> path;
    vector<vector<int>> freq;
    vector<char> removed;
    vector<int> size, parent;
    void build(int start) {
        vector<int> order{start};
        parent[start]=0;
        for (size_t i=0; i<order.size(); ++i) {
            int v=order[i];
            size[v]=1;
            for (int w:adj[v]) if (w!=parent[v] && !removed[w]) {
                parent[w]=v;
                order.push_back(w);
            }
        }
        for (int i=int(order.size())-1; i>0; --i) size[parent[order[i]]]+=size[order[i]];
        int c=start, total=order.size();
        while (true) {
            int next=0;
            for (int w:adj[c]) if (!removed[w] && parent[w]==c && 2*size[w]>total) next=w;
            if (!next) break;
            c=next;
        }
        removed[c]=1;
        int all=freq.size(), maxD=0;
        freq.emplace_back();
        path[c].push_back({0,all,-1});
        vector<array<int,3>> todo;
        for (int w:adj[c]) if (!removed[w]) {
            int sub=freq.size(), mx=0;
            todo.push_back({w,c,1});
            while (!todo.empty()) {
                auto [v,p,d]=todo.back(); todo.pop_back();
                path[v].push_back({d,all,sub});
                mx=max(mx,d);
                for (int x:adj[v]) if (x!=p && !removed[x]) todo.push_back({x,v,d+1});
            }
            freq.emplace_back(mx+1);
            maxD=max(mx,maxD);
        }
        freq[all].resize(maxD+1);
        for (int w:adj[c]) if (!removed[w]) build(w);
    }
    int count(int v, int k) const {
        int ans=0;
        for (const Entry &p:path[v]) {
            int r=k-p.d;
            if (r<0) continue;
            const auto &a=freq[p.all];
            ans+=a[min(r,int(a.size())-1)];
            if (p.sub>=0) {
                const auto &b=freq[p.sub];
                ans-=b[min(r,int(b.size())-1)];
            }
        }
        return ans;
    }
public:
    // The median distance changes by at most one across an edge.
    vector<Signal> closest(const vector<int>& s) {
        if (path.empty()) {
            path.resize(n+1); removed.resize(n+1); size.resize(n+1); parent.resize(n+1);
            build(1);
        }
        for (auto &f:freq) fill(f.begin(),f.end(),0);
        for (int v:s) for (const Entry &p:path[v]) {
            ++freq[p.all][p.d];
            if (p.sub>=0) ++freq[p.sub][p.d];
        }
        for (auto &f:freq) partial_sum(f.begin(),f.end(),f.begin());
        int m=s.size(), half=(m+1)/2, best=m, ties=0;
        vector<Signal> picked;
        auto consider=[&](int u,int k,int cnt) {
            if (cnt==0 || cnt==m) return;
            int cost=max(cnt,m-cnt);
            if (cost>best) return;
            if (cost<best) { picked.clear(); ties=0; best=cost; }
            ++ties;
            if (picked.size()<16) picked.push_back({u,k});
            else {
                int at=randomWord()%ties;
                if (at<16) picked[at]={u,k};
            }
        };
        vector<int> med(n+1), depths;
        for (int v:s) depths.push_back(dep[v]);
        nth_element(depths.begin(),depths.begin()+half-1,depths.end());
        med[1]=depths[half-1];
        for (int v=1; v<=n; ++v) {
            int r=v==1?med[1]:med[up[0][v]];
            int a=count(v,r), b;
            if (a<half) { b=a; a=count(v,++r); }
            else {
                b=count(v,r-1);
                if (b>=half) { --r; a=b; b=count(v,r-1); }
            }
            med[v]=r;
            consider(v,r,a); consider(v,r-1,b);
        }
        return picked;
    }
} ballIndex;

// Keep a on the boundary. Vertex b is inside precisely on the b side of
// the edge that crosses the midpoint of a--b (ties belong to b).
// In an Euler tour that side is one interval or the complement of one.
template<class Weight,class Emit>
void sweepAnchor(const vector<int>& s,int anchor,Weight weight,Emit emit) {
    int a=s[anchor], cnt=1;
    U64 sig=weight(anchor);
    vector<unsigned> events;
    events.reserve(2*s.size());
    for (int i=0; i<(int)s.size(); ++i) if (i!=anchor) {
        int b=s[i], p=lca(a,b), d=dep[a]+dep[b]-2*dep[p], half=(d+1)/2;
        bool outside=half<=dep[a]-dep[p];
        int v;
        if (outside) { v=lift(a,half-1); ++cnt; sig^=weight(i); }
        else v=lift(b,d-half);
        unsigned id=unsigned(i)<<1;
        events.push_back((unsigned(firstVisit[v])<<16)|id|!outside);
        events.push_back((unsigned(pastVisit[v])<<16)|id|outside);
    }
    sort(events.begin(),events.end());
    unsigned pos=0;
    size_t j=0;
    while (pos<rmq[0].size()) {
        while (j<events.size() && (events[j]>>16)==pos) {
            unsigned e=events[j++];
            cnt+=(e&1)?1:-1; sig^=weight((e&65535)>>1);
        }
        emit(rmq[0][pos],cnt,sig);
        if (j==events.size()) break;
        pos=events[j]>>16;
    }
}

vector<int> hist, touched;
vector<U64> binTag;
struct Proposals {
    const vector<int>& s;
    U64 total;
    int m, minSize;
    vector<int> centers;
    vector<Option> opts;
    unordered_set<U64> seen;
    Proposals(const vector<int>& s, U64 total, int low):
        s(s), total(total), m(s.size()), minSize(low) {}
    void add(int u, int k, int num, U64 sig) {
        if (min(num,m-num)<minSize) return;
        U64 canon=min(sig,total^sig);
        if (opts.size()<24) {
            for (auto &x:opts) if (x.part==canon) return;
        } else {
            if (opts.size()==24 && seen.empty())
                for (auto &x:opts) seen.insert(x.part);
            if (!seen.insert(canon).second) return;
        }
        opts.push_back({{u,k},num,canon});
    }
    void center(int u) {
        if (find(centers.begin(),centers.end(),u)!=centers.end()) return;
        centers.push_back(u);
        int mn=n, mx=-1;
        touched.clear();
        for (int v:s) {
            int d=dist(u,v);
            if (hist[d]++==0) touched.push_back(d);
            binTag[d]^=tag[v];
            mn=min(mn,d); mx=max(mx,d);
        }
        int cnt=0; U64 sig=0;
        auto visit=[&](int d) {
            if (2*(cnt+hist[d])>=m) {
                add(u,d-1,cnt,sig);
                add(u,d,cnt+hist[d],sig^binTag[d]);
                return true;
            }
            cnt+=hist[d]; sig^=binTag[d]; return false;
        };
        if (mx-mn<4*m+32) {
            for (int d=mn; d<=mx; ++d) if (visit(d)) break;
        } else {
            sort(touched.begin(),touched.end());
            for (int d:touched) if (visit(d)) break;
        }
        for (int d:touched) { hist[d]=0; binTag[d]=0; }
    }
    int sampled() const { return s[randomWord()%m]; }
    void initial() {
        int a=s[0];
        for (int v:s) if (dep[v]>dep[a]) a=v;
        int b=a, len=0;
        for (int v:s) {
            int d=dist(a,v);
            if (d>len) { len=d; b=v; }
        }
        center(a); center(b);
        int p=lca(a,b);
        center(p); center(onPath(a,b,p,len,len/2));
    }
    void sample(int times) {
        for (int i=0; i<times; ++i) {
            int a=sampled(), b=sampled(), p=lca(a,b);
            if (i%4==0) center(p);
            else if (i%4==1) {
                int d=dep[a]+dep[b]-2*dep[p];
                center(onPath(a,b,p,d,d/2));
            } else center(a);
        }
    }
    void boundary(int i) {
        sweepAnchor(s,i,[&](int j){return tag[s[j]];},[&](int u,int cnt,U64 sig) {
            if (min(cnt,m-cnt)>=minSize) add(u,dist(u,s[i]),cnt,sig);
        });
    }
    void exhaustive() {
        for (int i=0; i<m; ++i) boundary(i);
    }
    int cost(const Option &o) const { return max(o.inside,m-o.inside); }
    void rankFrom(int start) {
        stable_sort(opts.begin()+start,opts.end(),[&](const Option &a,const Option &b) {
            return cost(a)<cost(b);
        });
    }
};


// Compare the next two decisions on a common sample. Each center supplies a
// sorted family of balls, so a binary search locates its median on any subset.
class Forecast {
    static constexpr int words=16;
    using Mask=array<U64,words>;
    struct Family { vector<Mask> balls; };
    struct Split { int size, id, index, cost; };
    int sampleSize, width;
    vector<int> vertices;
    vector<Family> families;
    Mask full{};
    int weight(const Mask& a, const Mask& b) const {
        int ans=0;
        for (int i=0; i<width; ++i) ans+=popcount(a[i]&b[i]);
        return ans;
    }
    void halves(const Mask& base, const Mask& cut, Mask& a, Mask& b) const {
        for (int i=0; i<width; ++i) {
            a[i]=base[i]&cut[i]; b[i]=base[i]^a[i];
        }
    }
    int bestSplit(const Mask& base,int size, vector<Split>* choices=nullptr) const {
        if (size<2) return size;
        int ideal=(size+1)/2, best=size;
        for (int j=0; j<(int)families.size(); ++j) {
            const auto &balls=families[j].balls;
            int lo=-1, hi=int(balls.size())-1, left=0, right=size;
            while (hi-lo>1) {
                int mid=(hi+lo)/2, c=weight(base,balls[mid]);
                if (c<ideal) { lo=mid; left=c; }
                else { hi=mid; right=c; }
            }
            int cost=min(size-left,right);
            best=min(best,cost);
            if (!choices && best==ideal) return best;
            if (choices && cost<size) {
                if (size-left<=right) choices->push_back({left,j,lo,size-left});
                if (right<=size-left) choices->push_back({right,j,hi,right});
            }
        }
        return best;
    }
    // Return the largest expected group after one and two more signals.
    pair<double,double> future(const Mask& base,int size,int actual) const {
        if (size<4) return {actual*.5,actual*.25};
        vector<Split> choices;
        int best=bestSplit(base,size,&choices), best2=size;
        stable_sort(choices.begin(),choices.end(),[](auto &a,auto &b){return a.cost<b.cost;});
        vector<Mask> tried;
        for (const auto &x:choices) {
            if ((x.cost+1)/2>=best2) break;
            Mask a{},b{};
            halves(base,families[x.id].balls[x.index],a,b);
            bool duplicate=false;
            for (auto &t:tried) {
                int common=weight(t,a), sizeT=weight(t,t);
                int diff=sizeT+x.size-2*common;
                if (min(diff,size-diff)<=size/128) { duplicate=true; break; }
            }
            if (duplicate) continue;
            tried.push_back(a);
            best2=min(best2,max(bestSplit(a,x.size),bestSplit(b,size-x.size)));
            if (tried.size()==4) break;
        }
        return {double(best)*actual/size,double(best2)*actual/size};
    }
public:
    Signal choose(Proposals& gen) {
        int m=gen.m;
        sampleSize=min(words*64,m);
        width=(sampleSize+63)/64;
        for (int i=0;i<sampleSize;++i) {
            int l=i*m/sampleSize, r=(i+1)*m/sampleSize;
            vertices.push_back(gen.s[l+randomWord()%(r-l)]);
            full[i/64]|=1ULL<<(i%64);
        }
        vector<int> centers=gen.centers;
        if (centers.size()>88) centers.resize(88);
        vector<pair<int,int>> sorted(sampleSize);
        for (int u:centers) {
            for (int i=0; i<sampleSize; ++i) sorted[i]={dist(u,vertices[i]),i};
            sort(sorted.begin(),sorted.end());
            Family f;
            Mask accum{};
            for (int i=0; i<sampleSize; ++i) {
                int at=sorted[i].second;
                accum[at/64]|=1ULL<<(at%64);
                if (i+1==sampleSize || sorted[i].first!=sorted[i+1].first)
                    f.balls.push_back(accum);
            }
            families.push_back(move(f));
        }
        double best=1e30;
        Signal answer=gen.opts[0].q;
        vector<Mask> tried;
        int cap=1<<(bit_width(unsigned(m-1))-1);
        bool withinBound=gen.cost(gen.opts.front())<=cap;
        for (const Option &o:gen.opts) {
            int largest=gen.cost(o);
            if (largest>=best || (withinBound && largest>cap)) break;
            Mask a{},b{};
            for (int i=0; i<sampleSize; ++i) if (dist(vertices[i],o.q.u)<=o.q.k)
                a[i/64]|=1ULL<<(i%64);
            int na=weight(a,full);
            bool duplicate=false;
            for (auto &t:tried) {
                int diff=na+weight(t,t)-2*weight(a,t);
                if (min(diff,sampleSize-diff)<=sampleSize/160) { duplicate=true; break; }
            }
            if (duplicate) continue;
            tried.push_back(a);
            for (int j=0; j<width; ++j) b[j]=full[j]^a[j];
            auto [x,xx]=future(a,na,o.inside);
            auto [y,yy]=future(b,sampleSize-na,m-o.inside);
            double score=.25*largest+.40*2*max(x,y)+.35*4*max(xx,yy);
            if (score<best) { best=score; answer=o.q; }
            if (tried.size()>=20) break;
        }
        return answer;
    }
};

U64 salts[2][32];
unordered_map<U64, Signal> plan, prefixPlan;
int cutHeight=-1;
unordered_set<U64> failed;
clock_t deadline;
int calls;
bool timedOut;
bool expired() {
    if ((++calls & 31)==0 && clock()>=deadline) timedOut=true;
    return timedOut;
}
bool prove(const vector<int>& s, U64 sig, int h, bool strict,
           const vector<Signal>* extra=nullptr) {
    int m=s.size();
    if (m>(1<<h) || (strict && h>=1 && m<(1<<(h-1)))) return false;
    if (m<=3 || h<=cutHeight) return true;
    U64 key=sig^salts[strict][h];
    if (plan.count(key) || (cutHeight>=0 && prefixPlan.count(key))) return true;
    if (expired() || failed.count(key)) return false;
    int cap=1<<(h-1);
    int low=max({1,m-cap,strict?cap/2:1});
    Proposals gen(s,sig,low);
    int entered=calls;
    auto localLimit=[&]() { return calls-entered>180+4*m; };
    auto search=[&](int from) {
        gen.rankFrom(from);
        for (int i=from; i<(int)gen.opts.size(); ++i) {
            if (expired() || localLimit()) break;
            auto o=gen.opts[i];
            vector<int> in,out;
            in.reserve(o.inside); out.reserve(m-o.inside);
            U64 z=0;
            for (int v:s) {
                if (dist(v,o.q.u)<=o.q.k) { in.push_back(v); z^=tag[v]; }
                else out.push_back(v);
            }
            if (in.size()<out.size()) { in.swap(out); z^=sig; }
            if (prove(in,z,h-1,strict) && prove(out,z^sig,h-1,strict)) {
                (cutHeight<0?plan:prefixPlan)[key]=o.q; return true;
            }
        }
        return false;
    };
    if (extra && !extra->empty()) gen.center((*extra)[0].u);
    gen.initial();
    if (m>8 && (gen.opts.empty() || none_of(gen.opts.begin(),gen.opts.end(),
            [&](const Option &o) { return gen.cost(o)<=(m+1)/2+m/45; })))
        gen.sample(extra?24:10);
    if (extra) for (auto q:*extra) gen.center(q.u);
    if (search(0)) return true;
    int from=gen.opts.size();
    if (!timedOut && !localLimit() && m>12) {
        gen.sample(extra?48:12);
        if (search(from)) return true;
        from=gen.opts.size();
    }
    if (!timedOut && !localLimit() && m<=128) {
        gen.exhaustive();
        if (search(from)) return true;
    } else if (!timedOut && !localLimit() && m<=2048) {
        for (int i=0; i<8; ++i) gen.boundary(randomWord()%m);
        if (search(from)) return true;
    }
    if (!timedOut) failed.insert(key);
    return false;
}

// Once the live set fits in a word, enumerate every attainable partition.
// Among trees of the minimum feasible height, minimize total leaf depth.
class SmallGame {
    static constexpr int inf=1000000;
    struct Ball { U64 mask; Signal q; };
    struct Entry { int cost=inf, excluded=-1, choice=-1; };
    struct Move { U64 a; int id, big; };
    const vector<int>& vertices;
    vector<Ball> balls;
    vector<unordered_map<U64,Entry>> memo;
    int ideal[65]{};
    int solve(U64 s,int h,int limit) {
        int m=popcount(s);
        if (m>(1<<h) || ideal[m]>limit) return inf;
        if (m<=3) return ideal[m];
        Entry &e=memo[h][s];
        int bound=min(limit,e.cost-1);
        if (bound<ideal[m] || bound<=e.excluded || expired())
            return e.cost<=limit?e.cost:inf;
        int cap=1<<(h-1);
        vector<Move> moves;
        for (int i=0; i<(int)balls.size(); ++i) {
            U64 a=s&balls[i].mask, b=s^a;
            if (!a || !b) continue;
            int x=popcount(a), big=max(x,m-x);
            if (big<=cap && ideal[x]+ideal[m-x]+m<=bound)
                moves.push_back({min(a,b),i,big});
        }
        sort(moves.begin(),moves.end(),[](auto &a,auto &b) {
            return a.big!=b.big?a.big<b.big:a.a<b.a;
        });
        U64 prev=0;
        for (auto move:moves) {
            if (move.a==prev) continue;
            prev=move.a;
            U64 a=move.a,b=s^a;
            int x=popcount(a), y=m-x;
            if (ideal[x]+ideal[y]+m>bound || timedOut) break;
            if (x<y) { swap(x,y); swap(a,b); }
            int ca=solve(a,h-1,bound-m-ideal[y]);
            if (ca==inf) continue;
            int cb=solve(b,h-1,bound-m-ca);
            if (cb==inf) continue;
            e.cost=ca+cb+m; e.choice=move.id;
            bound=e.cost-1;
            if (bound<ideal[m]) break;
        }
        if (!timedOut) e.excluded=max(e.excluded,bound);
        return e.cost<=limit?e.cost:inf;
    }
    void save(U64 s,int h) {
        int m=popcount(s);
        if (m<=3) return;
        const Entry &e=memo[h].at(s);
        Ball ball=balls[e.choice];
        U64 sig=0;
        for (U64 t=s; t; t&=t-1) sig^=tag[vertices[countr_zero(t)]];
        plan[sig^salts[false][h]]=ball.q;
        if (e.cost==ideal[m]) {
            int localH=bit_width(unsigned(m-1));
            plan[sig^salts[true][localH]]=ball.q;
        }
        U64 a=s&ball.mask;
        save(a,h-1); save(s^a,h-1);
    }
public:
    explicit SmallGame(const vector<int>& s):vertices(s) {}
    int run(int h) {
        int m=vertices.size();
        U64 all=~U64(0)>>(64-m);
        unordered_set<U64> seen;
        for (int i=0; i<m; ++i) {
            sweepAnchor(vertices,i,[](int j){return U64(1)<<j;},
                [&](int u,int cnt,U64 mask) {
                    if (cnt==m) return;
                    mask=min(mask,mask^all);
                    if (seen.insert(mask).second)
                        balls.push_back({mask,{u,dist(vertices[i],u)}});
                });
        }
        for (int i=1; i<=m; ++i) {
            int k=bit_width(unsigned(i-1));
            ideal[i]=(k+1)*i-(1<<k);
        }
        memo.resize(h+4);
        for (int level=h; level<=h+3 && !timedOut; ++level) {
            if (solve(all,level,level*m)<inf) { save(all,level); return level; }
        }
        return 0;
    }
};
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin>>n)) return 0;
    initTree();
    hist.resize(n+1); binTag.resize(n+1);
    for (auto &row:salts) for (auto &v:row) v=randomWord();
    vector<int> s(n); iota(s.begin(),s.end(),1);
    bool longPaths=*max_element(dep.begin(),dep.end())>100;
    int knownH=0; bool knownStrict=false;
    while (s.size()>1) {
        int m=s.size(), h=bit_width(unsigned(m-1));
        U64 sig=0;
        for (int v:s) sig^=tag[v];
        Signal q;
        bool certified=false;
        int nextH=0; bool nextStrict=false;
        auto use=[&](int level,bool strict) {
            auto it=plan.find(sig^salts[strict][level]);
            if (it==plan.end()) return false;
            q=it->second; certified=true; nextH=level-1; nextStrict=strict;
            return true;
        };
        if (m<=3) q={s[0],0};
        else if (!use(h,true)) {
            vector<Signal> options;
            auto attempt=[&](int level,bool strict,double seconds) {
                if (use(level,strict)) return true;
                clock_t now=clock();
                deadline=min(clock_t(0.57*CLOCKS_PER_SEC),now+clock_t(seconds*CLOCKS_PER_SEC));
                if (now>=deadline) return false;
                failed.clear(); calls=0; timedOut=false;
                return prove(s,sig,level,strict,&options) && use(level,strict);
            };
            bool have=false;
            if (m<=64 && clock()<0.77*CLOCKS_PER_SEC) {
                calls=0; timedOut=false; deadline=clock()+clock_t(0.028*CLOCKS_PER_SEC);
                int level=SmallGame(s).run(h);
                if (level) have=use(h,true) || use(level,false);
            }
            if (!have) have=attempt(h,true,longPaths?0.094:0.036) || (m+1<(1<<h) && attempt(h,false,0.025));
            if (!have && knownH>=h) have=use(knownH,knownStrict);
            if (!have) {
                Proposals gen(s,sig,1);
                bool spareTime=clock()<0.82*CLOCKS_PER_SEC;
                gen.initial(); gen.sample(spareTime?96:16);
                if (m<=32) gen.exhaustive();
                gen.rankFrom(0);
                if (spareTime && (gen.opts.empty() || gen.cost(gen.opts.front())>(m+1)/2)) {
                    options=ballIndex.closest(s);
                    for (auto o:options) gen.center(o.u);
                    gen.rankFrom(0);
                }
                q=gen.opts.empty()?Signal{s[0],0}:gen.opts[0].q;
                if (!options.empty()) {
                    have=attempt(h,true,0.027) || attempt(h,false,0.025);
                }
                if (!have && m<=48) have=attempt(h+1,false,0.012);
                if (!have && gen.opts.size()>1 && clock()<0.73*CLOCKS_PER_SEC)
                    q=Forecast().choose(gen);
                if (!have && m>64 && (1<<h)-m<=m/16 &&
                    !gen.opts.empty() && gen.cost(gen.opts[0])<=(1<<(h-1)) &&
                    clock()<0.71*CLOCKS_PER_SEC) {
                    vector<Signal> seeds{q};
                    for (auto &o:gen.opts) {
                        if (seeds.size()==16 || gen.cost(o)>(1<<(h-1))) break;
                        seeds.push_back(o.q);
                    }
                    deadline=clock()+clock_t(0.019*CLOCKS_PER_SEC);
                    for (int levels : {2,4}) {
                        prefixPlan.clear(); failed.clear(); calls=0; timedOut=false;
                        cutHeight=h-levels;
                        if (!prove(s,sig,h,false,&seeds)) break;
                        auto it=prefixPlan.find(sig^salts[false][h]);
                        if (it!=prefixPlan.end()) q=it->second;
                        seeds[0]=q;
                    }
                    cutHeight=-1;
                }
            }
        }
        vector<int> in,out;
        for (int v:s) (dist(q.u,v)<=q.k?in:out).push_back(v);
        if (in.empty() || out.empty()) {
            q={s.back(),0}; in={s.back()}; out=s; out.pop_back();
            certified=false;
        }
        cout<<"? "<<q.u<<' '<<q.k<<endl;
        int reply;
        if (!(cin>>reply) || (reply!=0 && reply!=1)) return 0;
        s=reply?move(in):move(out);
        knownH=certified?nextH:0; knownStrict=nextStrict;
    }
    cout<<"! "<<s[0]<<endl;
    return 0;
}
