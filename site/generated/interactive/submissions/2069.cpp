#include <bits/stdc++.h>
using namespace std;

// All response denominators divide q*lcm(1,...,1000). The final doubled
// area is an integer smaller than the prime, so field arithmetic is exact
// for the purpose of recovering the answer.
using F = uint64_t;
constexpr F P = (1ULL << 61) - 1;
F add(F a, F b) { F z=a+b; return z>=P ? z-P : z; }
F subf(F a, F b) { return a>=b ? a-b : a+P-b; }
F mul(F a, F b) {
    __uint128_t z=(__uint128_t)a*b;
    return add(F(z)&P,F(z>>61));
}
F num(long long x) { return x<0 ? P-F(-x) : F(x); }
F power(F a,F e) {
    F r=1;
    while(e){ if(e&1) r=mul(r,a); a=mul(a,a); e>>=1; }
    return r;
}
F invs[3001];
F parse(const string &s) {
    F r=0;
    if(s.empty()) exit(0);
    for(char c:s){
        if(c<'0'||c>'9') exit(0);
        r=add(mul(r,10),c-'0');
    }
    return r;
}
struct Q {
    int p=0,q=1;
    Q()=default;
    Q(int a,int b) { int g=gcd(a,b); p=a/g; q=b/g; }
    int key()const{return p*1001+q;}
};
map<int,F> cache;
map<int,pair<string,string>> exactReply;
F ask(Q x) {
    if(auto it=cache.find(x.key());it!=cache.end()) return it->second;
    cout<<"? "<<x.p<<" "<<x.q<<endl;
    string a,b;
    if(!(cin>>a) || a[0]=='-' || !(cin>>b) || b[0]=='-') exit(0);
    exactReply[x.key()]={a,b};
    F d=parse(b);
    if(!d) exit(0);
    return cache[x.key()]=mul(parse(a),power(d,P-2));
}
struct Pt {int x,y;};
int n;
vector<Pt> pt;
int turn(int a,int b,int c) {
    return (pt[b].x-pt[a].x)*(pt[c].y-pt[a].y)-(pt[b].y-pt[a].y)*(pt[c].x-pt[a].x);
}
struct Edge {int a,b;};
int area2(Edge e){return (pt[e.a].x-pt[e.b].x)*(pt[e.a].y+pt[e.b].y);}
vector<int> hull,hr,inEdge,outEdge;
vector<Edge> known,hullEdges;
vector<int> component,compSize;
int root(int a){
    while(a!=component[a]){component[a]=component[component[a]];a=component[a];}
    return a;
}
vector<vector<unsigned char>> visible,forbidden;
int epoch=0;
unordered_map<int,int> stalled;
vector<vector<F>> slope;
bool beforeTime(double limit){return clock()<limit*CLOCKS_PER_SEC;}

bool conflict(Edge e, Edge f) {
    if(e.a==f.a || e.a==f.b || e.b==f.a || e.b==f.b)
        return (e.a==f.a && e.b==f.b)||(e.a==f.b && e.b==f.a);
    // Edges supplied here have no other vertex in their relative interior.
    int a=turn(e.a,e.b,f.a), b=turn(e.a,e.b,f.b);
    int c=turn(f.a,f.b,e.a), d=turn(f.a,f.b,e.b);
    return (long long)a*b<0 && (long long)c*d<0;
}
bool allowed(int a,int b) {
    if(!visible[a][b] || forbidden[a][b] || outEdge[a]>=0 || inEdge[b]>=0 || inEdge[a]==b) return false;
    if(root(a)==root(b) && compSize[root(a)]<n)return false;
    if(hr[a]>=0 && hr[b]>=0 && hull[(hr[a]+1)%hull.size()]!=b) return false;
    if(hr[a]>=0 && inEdge[a]>=0 && turn(inEdge[a],a,b)<0) return false;
    if(hr[b]>=0 && outEdge[b]>=0 && turn(a,b,outEdge[b])<0) return false;
    return true;
}

struct Probe {
    Q x;
    F xf;
    Probe(Q v):x(v),xf(mul(v.p,invs[v.q])){}
    F line(int a,int b)const {
        return add(pt[a].y,mul(subf(xf,pt[a].x),slope[a][b]));
    }
    // At a breakpoint take the average of the lateral limits, plus
    // half the length of each vertical boundary edge on this line.
    F value(Edge e)const {
        int a=e.a,b=e.b;
        int A=pt[a].x*x.q, B=pt[b].x*x.q;
        if(x.p<min(A,B)||x.p>max(A,B)) return 0;
        if(A==B) return mul(abs(pt[a].y-pt[b].y),invs[2]);
        F y=line(a,b);
        if(A<B) y=subf(0,y);
        return x.p==A||x.p==B ? mul(y,invs[2]):y;
    }
};
F recorded(const Probe &p) {
    F v=0;
    for(Edge e:known) v=add(v,p.value(e));
    return v;
}
void record(Edge e) {
    epoch++;
    int a=root(e.a),b=root(e.b);
    if(a!=b){if(compSize[a]<compSize[b])swap(a,b);component[b]=a;compSize[a]+=compSize[b];}
    outEdge[e.a]=e.b; inEdge[e.b]=e.a; known.push_back(e);
}

void init() {
    for(int i=1;i<=3000;i++) invs[i]=power(i,P-2);
    sort(pt.begin(),pt.end(),[](Pt a,Pt b){return tie(a.x,a.y)<tie(b.x,b.y);});
    for(int side=0;side<2;side++){
        vector<int> chain;
        for(int i=0;i<n;i++){
            int v=side?n-1-i:i;
            while(chain.size()>=2 && turn(chain[chain.size()-2],chain.back(),v)<0) chain.pop_back();
            chain.push_back(v);
        }
        chain.pop_back();
        hull.insert(hull.end(),chain.begin(),chain.end());
    }
    hr.assign(n,-1);
    for(int i=0;i<(int)hull.size();i++)hr[hull[i]]=i;
    component.resize(n);iota(component.begin(),component.end(),0);compSize.assign(n,1);
    inEdge.assign(n,-1); outEdge.assign(n,-1);
}
void lines() {
    visible.assign(n,vector<unsigned char>(n));
    forbidden=visible;
    slope.assign(n,vector<F>(n));
    for(int i=0;i<n;i++){
        unordered_map<int,int> nearest;
        nearest.reserve(n*2);
        for(int j=0;j<n;j++) if(i!=j) {
            int dx=pt[j].x-pt[i].x,dy=pt[j].y-pt[i].y,g=gcd(dx,dy);
            int k=(dx/g+1000)*2001+dy/g+1000;
            if(dx) slope[i][j]=mul(num(dx>0?dy:-dy),invs[abs(dx)]);
            auto [it,newDir]=nearest.emplace(k,j);
            if(!newDir && abs(dx)+abs(dy)<abs(pt[it->second].x-pt[i].x)+abs(pt[it->second].y-pt[i].y))
                it->second=j;
        }
        for(auto entry:nearest)visible[i][entry.second]=1;
    }
}

// Candidates use signed edge lists, allowing a common hull to be omitted.
int code(int a,int b){return a*n+b+1;}
Edge decode(int v){--v;return {v/n,v%n};}
struct Candidate { int area; vector<int> edges; };
struct ProbeValues {
    Probe p;
    unordered_map<int,F> values;
    ProbeValues(Q x):p(x){}
    F get(int k){
        auto [it,fresh]=values.try_emplace(k);
        if(fresh){
            it->second=p.value(decode(abs(k)));
            if(k<0)it->second=subf(0,it->second);
        }
        return it->second;
    }
    F sum(const Candidate &c){
        F s=0;
        for(int k:c.edges)s=add(s,get(k));
        return s;
    }
};
mt19937 rng(712387);
vector<int> columns(const vector<int> &v) {
    vector<int> xs;
    for(int i:v) if(xs.empty()||xs.back()!=pt[i].x)xs.push_back(pt[i].x);
    return xs;
}
Q inside(int a,int b,int salt=0) {
    int p=a*997+((b-a)*997*(41+salt%17))/97;
    return Q(p+(p%997==0),997);
}
// On small candidate sets, choose a decision tree that minimizes its
// worst depth, breaking ties by the total depth of its leaves.
optional<Q> decisionQuery(const vector<Candidate>& cs,const vector<int>& xs){
    int k=cs.size();
    if(k>50||xs.size()>48||!beforeTime(5.5))return {};
    using Mask=uint64_t;
    vector<pair<Q,vector<Mask>>> probes;
    set<vector<Mask>> seen;
    auto insert=[&](Q x){
        if(cache.count(x.key()))return;
        ProbeValues p(x);
        map<F,Mask> buckets;
        for(int i=0;i<k;i++)buckets[p.sum(cs[i])]|=1ULL<<i;
        if(buckets.size()==1)return;
        vector<Mask> groups;
        for(auto [v,ids]:buckets)groups.push_back(ids);
        sort(groups.begin(),groups.end());
        if(seen.insert(groups).second)probes.emplace_back(x,move(groups));
    };
    for(int i=0;i+1<(int)xs.size();i++){
        insert(inside(xs[i],xs[i+1],i));
        insert(Q(997*xs[i]+1,997));
        insert(Q(xs[i],1));
    }
    insert(Q(xs.back(),1));
    vector<Mask> same(k);
    for(int i=0;i<k;i++)for(int j=0;j<k;j++)
        if(cs[j].area==cs[i].area)same[i]|=1ULL<<j;
    struct Score{int depth,sum,choice;};
    unordered_map<Mask,Score> memo;
    int work=0;
    auto dfs=[&](auto&& self,Mask mask)->Score{
        if(!(mask & ~same[__builtin_ctzll(mask)]))return {0,0,-1};
        if(auto it=memo.find(mask);it!=memo.end())return it->second;
        Score best{10000,10000,-1};
        for(int i=0;i<(int)probes.size();i++){
            int d=1,s=__builtin_popcountll(mask);
            for(Mask m:probes[i].second){
                if(++work>160000)return {10000,10000,-1};
                m&=mask;
                if(!m)continue;
                if(m==mask){d=10001;break;}
                auto a=self(self,m);
                d=max(d,1+a.depth);s+=a.sum;
                if(d>best.depth || (d==best.depth && s>=best.sum))break;
            }
            if(tie(d,s)<tie(best.depth,best.sum))best={d,s,i};
            if(best.depth==1)break;
        }
        memo[mask]=best;return best;
    };
    auto best=dfs(dfs,(1ULL<<k)-1);
    if(best.choice<0||work>160000)return {};
    return probes[best.choice].first;
}
int chooseArea(vector<Candidate>& cs,const vector<int>& xs,const vector<Edge>* baseline=nullptr) {
    {
        for(auto [key,raw]:cache){
            int q=key%1001,p=key/1001;
            if(p<xs.front()*q || xs.back()*q<p)continue;
            ProbeValues val(Q(p,q));
            F y=subf(raw,recorded(val.p));
            if(baseline)for(Edge e:*baseline)y=subf(y,val.p.value(e));
            cs.erase(remove_if(cs.begin(),cs.end(),[&](auto &c){return val.sum(c)!=y;}),cs.end());
            if(cs.size()<=1)break;
        }
    }
    while(true){
        int area=cs.front().area;
        if(all_of(cs.begin(),cs.end(),[&](auto &c){return c.area==area;}))return area;
        vector<int> sample(cs.size());
        iota(sample.begin(),sample.end(),0);
        if(sample.size()>420){
            shuffle(sample.begin(),sample.end(),rng); sample.resize(420);
        }
        int different=0;
        while(cs[different].area==area)different++;
        sample.push_back(0); sample.push_back(different);
        vector<int> gaps(xs.size()-1);
        iota(gaps.begin(),gaps.end(),0); shuffle(gaps.begin(),gaps.end(),rng);
        Q best;
        vector<Q> probes;
        for(int i:gaps){
            probes.push_back(inside(xs[i],xs[i+1],i));
            if(gaps.size()<28)probes.emplace_back(xs[i],1);
        }
        // High-degree rational positions separate different affine functions
        // even when their values coincide at the slab midpoint.
        long long bestCost=LLONG_MAX;
        auto score=[&](Q x) {
            if(cache.count(x.key()))return;
            ProbeValues v(x);
            unordered_map<F,pair<int,int>> hist;
            for(int k:sample){
                auto &bucket=hist[v.sum(cs[k])];
                if(!bucket.first)bucket.second=cs[k].area;
                else if(bucket.second!=cs[k].area)bucket.second=INT_MIN;
                bucket.first++;
            }
            long long cost=0;
            for(auto &it:hist)if(it.second.second==INT_MIN)
                cost+=1LL*it.second.first*it.second.first;
            if(cost<bestCost){bestCost=cost;best=x;}
        };
        int tried=0;
        if(auto optimal=decisionQuery(cs,xs)){best=*optimal;bestCost=0;}
        else for(Q x:probes){
            score(x);
            if(bestCost==0 || (++tried>=65 && bestCost<((long long)sample.size()*(long long)sample.size())))break;
        }
        if(bestCost==LLONG_MAX || bestCost==((long long)sample.size()*(long long)sample.size())){
            // Unequal integrals guarantee a distinguishing slab midpoint.
            for(int i:gaps){
                ProbeValues v(Q(xs[i]+xs[i+1],2));
                if(v.sum(cs.front())!=v.sum(cs[different])){best=v.p.x;break;}
            }
        }
        ProbeValues v(best);
        F result=subf(ask(best),recorded(v.p));
        if(baseline)for(Edge e:*baseline)result=subf(result,v.p.value(e));
        cs.erase(remove_if(cs.begin(),cs.end(),[&](auto &c){return v.sum(c)!=result;}),cs.end());
        if(cs.empty())exit(0);
    }
}

optional<int> polygonizations(const vector<int>& all,int hullArea,const vector<Edge>& gaps) {
    vector<int> inner;
    for(int v:all)if(hr[v]<0)inner.push_back(v);
    int m=inner.size(),h=gaps.size();
    double bound=1;
    for(int j=0;j<m;j++)bound*=h+j;
    if(m>10 || (bound>2e6 && n>15))return {};
    vector<Candidate> cs;
    vector<int> list;
    vector<Edge> path;
    int steps=0;
    bool stopped=false;
    auto clearEdge=[&](Edge e){
        if(!visible[e.a][e.b])return false;
        for(auto f:path)if(conflict(e,f))return false;
        return true;
    };
    // Start pockets in hull order. Inside a pocket, append an unused
    // interior vertex, or join the following hull vertex.
    auto dfs=[&](auto&& self,int gap,int tail,int mask,int area)->void {
        if(stopped)return;
        if(++steps>(m<=2?6500000:1300000) || cs.size()>=(m<=2?1020000:48000) || ((steps&4095)==0 && !beforeTime(m<=2?2.2:1.55))){
            stopped=true; return;
        }
        if(tail<0){
            if(!mask){cs.push_back({area,list});return;}
            for(int g=gap;g<h && !stopped;g++){
                int a=gaps[g].a,b=gaps[g].b;
                list.push_back(-code(a,b));
                self(self,g,a,mask,area-area2({a,b}));
                list.pop_back();
            }
            return;
        }
        if(hr[tail]<0){
            Edge e{tail,gaps[gap].b};
            if(clearEdge(e)){
                list.push_back(code(e.a,e.b)); path.push_back(e);
                self(self,gap+1,-1,mask,area+area2(e));
                list.pop_back(); path.pop_back();
            }
        }
        for(int j=0;j<m && !stopped;j++) if(mask>>j&1){
            Edge e{tail,inner[j]};
            if(!clearEdge(e))continue;
            list.push_back(code(e.a,e.b)); path.push_back(e);
            self(self,gap,e.b,mask^(1<<j),area+area2(e));
            list.pop_back(); path.pop_back();
        }
    };
    dfs(dfs,0,-1,(1<<m)-1,hullArea);
    if(stopped||cs.empty())return {};
    return chooseArea(cs,columns(all),&gaps);
}

struct Term { int cost=1000000; Q x; F factor=0,constant=0; };
F integrate(const vector<int>& vs) {
    auto xs=columns(vs);
    if(xs.size()<2)return 0;
    int k=xs.size();
    vector<int> jump(k);
    vector<bool> vert(k);
    for(int i=0,j=0;i<(int)vs.size();i++){
        int v=vs[i];
        if(pt[v].x!=xs[j])j++;
        jump[j]+=((inEdge[v]<0)-(outEdge[v]<0))*pt[v].y;
        if(i && pt[vs[i-1]].x==xs[j]){
            int u=vs[i-1];
            if(allowed(u,v)||allowed(v,u))vert[j]=true;
        }
    }
    vector<int> pref(k+1);
    F ans=0;
    for(int i=0;i<k;i++){
        pref[i+1]=pref[i]+jump[i];
        ans=add(ans,num(2LL*jump[i]*(xs.back()-xs[i])));
    }
    auto evaluate=[&](Q x){
        Probe pr(x);
        F y=subf(ask(x),recorded(pr));
        int i=lower_bound(xs.begin(),xs.end(),(x.p+x.q-1)/x.q)-xs.begin();
        y=subf(y,num(pref[i]));
        if(i<k && xs[i]*x.q==x.p)y=subf(y,mul(num(jump[i]),invs[2]));
        return y;
    };
    // Remove the known jumps caused by a vertex with one remaining
    // incident edge. The adjusted function is continuous except where an
    // unknown vertical edge can occur. Its exterior values are known.
    vector<optional<F>> node(2*k);
    auto leftId=[&](int i){return 2*i+1;};
    auto rightId=[&](int i){return 2*(i+1)+!vert[i+1];};
    if(!vert.front())node[leftId(0)]=0;
    if(!vert.back())node[rightId(k-2)]=num(-pref[k]);
    struct Equation{int a=0,b=0;F rhs=0;};
    vector<Equation> eq(k-1);
    for(auto [key,raw]:cache){
        Q x(key/1001,key%1001);
        if(x.p<xs.front()*x.q||x.p>xs.back()*x.q)continue;
        int j=lower_bound(xs.begin(),xs.end(),(x.p+x.q-1)/x.q)-xs.begin();
        if(x.p==xs[j]*x.q){
            if(!vert[j])node[leftId(j)]=evaluate(x);
            continue;
        }
        int i=j-1,a=xs[j]*x.q-x.p,b=x.p-xs[i]*x.q;
        F rhs=mul(a+b,evaluate(x));
        if(eq[i].a){
            auto e=eq[i];
            F det=subf(mul(a,e.b),mul(e.a,b)),inv=power(det,P-2);
            node[leftId(i)]=mul(subf(mul(rhs,e.b),mul(e.rhs,b)),inv);
            node[rightId(i)]=mul(subf(mul(a,e.rhs),mul(e.a,rhs)),inv);
        }else eq[i]={a,b,rhs};
    }
    for(int dir=0;dir<2;dir++)for(int t=0;t<k-1;t++){
        int i=dir?k-2-t:t;auto e=eq[i];
        if(!e.a)continue;
        auto &l=node[leftId(i)],&r=node[rightId(i)];
        if(l&&!r)r=mul(subf(e.rhs,mul(e.a,*l)),power(e.b,P-2));
        if(r&&!l)l=mul(subf(e.rhs,mul(e.b,*r)),power(e.a,P-2));
    }
    auto cell=[&](int i,int a,int b) {
        int t[2]={a,b};
        bool fixed[2]={node[leftId(i)].has_value(),node[rightId(i)].has_value()};
        F val[2]={node[leftId(i)].value_or(0),node[rightId(i)].value_or(0)};
        Term best;
        F constant=0;
        for(int j=0;j<2;j++)if(fixed[j]){
            constant=add(constant,mul(t[j],val[j])); t[j]=0;
        }
        auto option=[&](Q x,F f,F c){
            int cost=!cache.count(x.key());
            if(cost<best.cost)best={cost,x,f,add(c,constant)};
        };
        if(!t[0]&&!t[1])return Term{0,Q(),0,constant};
        if(t[0] && t[1]){
            Q x(t[0]*xs[i]+t[1]*xs[i+1],t[0]+t[1]);
            if(x.q<=1000)option(x,t[0]+t[1],0);
        }else{
            int j=t[1]!=0;
            if(!vert[i+j])option(Q(xs[i+j],1),t[j],0);
            if(fixed[1-j]){
                Q x(xs[i]+xs[i+1],2);
                option(x,2*t[j],subf(0,mul(t[j],val[1-j])));
            }
        }
        return best;
    };
    // At each node assign its trapezoidal weight to its left cell, to
    // its right cell, or split it according to the two incident widths.
    for(int l=0;l<k-1;){
        int r=l+1;
        while(r<k-1 && !vert[r])r++;
        vector<array<int,3>> dp(r-l+1),parent(r-l+1);
        auto weights=[&](int i,int opt){
            int a=(i==l)?0:xs[i]-xs[i-1], b=(i==r)?0:xs[i+1]-xs[i];
            if(i==l||i==r||opt==0)return pair(a,b);
            return opt==1?pair(a+b,0):pair(0,a+b);
        };
        for(int i=l+1;i<=r;i++){
            for(int b=0;b<3;b++){
                int best=10000000;
                for(int a=0;a<3;a++){
                    auto t=cell(i-1,weights(i-1,a).second,weights(i,b).first);
                    int cost=dp[i-1-l][a]+t.cost;
                    if(cost<best){best=cost;parent[i-l][b]=a;}
                }
                dp[i-l][b]=best;
            }
        }
        int b=0;
        for(int i=r;i>l;i--){
            int a=parent[i-l][b];
            auto t=cell(i-1,weights(i-1,a).second,weights(i,b).first);
            ans=add(ans,t.constant);
            if(t.factor)ans=add(ans,mul(t.factor,evaluate(t.x)));
            b=a;
        }
        l=r;
    }
    return ans;
}

// For a closed set of remaining slots, enumerate the small directed
// matchings. Known edges may join this set to previously solved regions.
bool fits(const vector<Edge>& es,Edge e);
optional<int> smallMatching(const vector<int>& vs) {
    vector<int> outs,ins;
    for(int v:vs){
        if(outEdge[v]<0)outs.push_back(v);
        if(inEdge[v]<0)ins.push_back(v);
    }
    if(outs.size()>9 || !beforeTime(6.8))return {};
    vector<vector<int>> opts(n);
    for(int a:outs)for(int j=0;j<(int)ins.size();j++){
        Edge e{a,ins[j]};
        if(allowed(e.a,e.b) && none_of(known.begin(),known.end(),[&](Edge f){return conflict(e,f);}))
            opts[a].push_back(j);
    }
    sort(outs.begin(),outs.end(),[&](int a,int b){return opts[a].size()<opts[b].size();});
    vector<Edge> edges;
    vector<int> list;
    vector<Candidate> cs;
    auto dfs=[&](auto&& self,int i,int mask,int area)->void{
        if(cs.size()>15000)return;
        if(i==(int)outs.size()){cs.push_back({area,list});return;}
        for(int j:opts[outs[i]]) if(!(mask>>j&1)){
            Edge e{outs[i],ins[j]};
            if(!fits(edges,e))continue;
            list.push_back(code(e.a,e.b)); edges.push_back(e);
            self(self,i+1,mask|(1<<j),area+area2(e));
            list.pop_back(); edges.pop_back();
        }
    };
    dfs(dfs,0,0,0);
    if(cs.size()>15000||cs.empty())return {};
    int remaining=chooseArea(cs,columns(vs));
    for(int k:cs.front().edges){
        if(all_of(cs.begin()+1,cs.end(),[&](const Candidate& c){
            return find(c.edges.begin(),c.edges.end(),k)!=c.edges.end();
        })){
            Edge e=decode(k);
            record(e);remaining-=area2(e);
        }
    }
    return remaining;
}

vector<int> tested;
bool possible(int a,int b) {
    if(!allowed(a,b))return false;
    int& z=tested[min(a,b)*n+max(a,b)];
    if(z<0)return false;
    for(;z<(int)known.size();z++)if(conflict({a,b},known[z])){z=-1;return false;}
    return true;
}
struct Slot {
    int v;
    bool out;
    vector<int> partners;
    Edge edge(int j)const{return out?Edge{v,j}:Edge{j,v};}
};
struct Cut {
    int count=0, separation=0;
    bool right=false;
    vector<Slot> slots;
    array<vector<int>,2> half;
    int work=INT_MAX;
    bool cached=false;
    // A negative partner represents a slot matched within the group.
};
long long balance(Cut& c){
    vector<int> ids(c.slots.size());
    iota(ids.begin(),ids.end(),0);
    sort(ids.begin(),ids.end(),[&](int u,int v){return c.slots[u].partners.size()>c.slots[v].partners.size();});
    long long product[2]={1,1};
    c.half={};
    for(int j:ids){
        int b=product[0]>product[1];
        c.half[b].push_back(j);
        product[b]=min<long long>(1000000000,product[b]*c.slots[j].partners.size());
    }
    if(product[0]>product[1])swap(c.half[0],c.half[1]);
    return product[0]+product[1];
}
int maxCutCost=2000000000;
bool planCut(const vector<int>& vs,bool right,int maxSlots,Cut& best,vector<Edge>& forced) {
    vector<Slot> slots;
    int size=vs.size();
    auto get=[&](int j){return vs[right?size-1-j:j];};
    for(int i=0;i+1<size && 2*i<size;i++){
        int a=get(i);
        for(int out=0;out<=1;out++){
            if((out?outEdge[a]:inEdge[a])>=0)continue;
            Slot s{a,(bool)out,{}};
            for(int b:vs){
                Edge e=s.edge(b);
                if(possible(e.a,e.b))s.partners.push_back(b);
            }
            if(s.partners.size()==1){
                forced.push_back(s.edge(s.partners[0]));
                return true;
            }
            slots.push_back(move(s));
        }
        if((int)slots.size()>maxSlots)break;
        if(pt[a].x==pt[get(i+1)].x)continue;
        Cut c;
        c.count=i+1; c.right=right;
        c.separation=right?size-c.count:c.count;
        for(auto &s:slots){
            Slot t{s.v,s.out,{}};
            bool internal=false;
            for(int b:s.partners){
                if(right ? b>=a : b<=a)internal=true;
                else t.partners.push_back(b);
            }
            if(internal)t.partners.push_back(-1);
            c.slots.push_back(move(t));
        }
        c.work=balance(c);
        int key=Q(pt[a].x*997+(right?-1:1),997).key();
        c.cached=cache.count(key);
        if(auto it=stalled.find(key);it!=stalled.end() && it->second==epoch)continue;
        if(c.work>maxCutCost || (c.work>35000000 && slots.size()>6) ||
           (c.work>2100000 && vs.size()<130))break;
        if(c.cached!=best.cached ? c.cached :
           c.slots.size()>best.slots.size() || (c.slots.size()==best.slots.size() && c.work<best.work))best=move(c);
    }
    return false;
}

bool fits(const vector<Edge>& es,Edge e) {
    for(Edge f:es)if(e.a==f.a||e.b==f.b||conflict(e,f))return false;
    // The finished polygon must be a single cycle.
    int v=root(e.b),end=root(e.a),len=compSize[v];
    while(v!=end){
        int w=-1;
        for(Edge f:es)if(root(f.a)==v){w=root(f.b);break;}
        if(w<0)return true;
        v=w;len+=compSize[v];
        if(len>n)return false;
    }
    return len==n;
}

struct Factor {int p,scale;};
vector<vector<Factor>> factors;
int primePower[1001],unitD[1001];
int inverseSmall(int a,int p){
    int e=p-2,v=1;
    while(e){if(e&1)v=v*a%p;a=a*a%p;e>>=1;}
    return v;
}
void prepareFactors(){
    factors.resize(1001);
    vector<int> primes;
    for(int p=2;p<=1000;p++){
        if(any_of(primes.begin(),primes.end(),[&](int v){return p%v==0;}))continue;
        primes.push_back(p);
        int pk=p;
        while(pk<=1000/p)pk*=p;
        primePower[p]=pk;
    }
    for(int p:primes)if(p>=11){
        int pk=primePower[p],unit=1;
        for(int other:primes)if(other!=p)unit=unit*primePower[other]%p;
        unitD[p]=unit;
        for(int d=pk;d<=1000;d+=pk)
            factors[d].push_back({p,unit*inverseSmall(d/pk,p)%p});
    }
}
int reduceString(const string &s,int modulus){
    long long r=0;
    for(char c:s)r=(r*10+c-'0')%modulus;
    return r;
}
long long ordinateNumerator(Edge e,Q x){
    int dx=pt[e.b].x-pt[e.a].x,dy=pt[e.b].y-pt[e.a].y;
    return -(1LL*pt[e.a].y*x.q*dx+1LL*(x.p-x.q*pt[e.a].x)*dy);
}

// Scale all ordinates by the common denominator. Modulo a large prime
// only a few possible spans contribute. Remove a partner if its residue
// cannot be completed by any choices for the other slots.
void sieve(Cut& c,Q x){
    if(x.q==1)return;
    if(factors.empty())prepareFactors();
    auto &reply=exactReply.at(x.key());
    int knownRes[1001]={};
    for(Edge e:known){
        int a=pt[e.a].x*x.q,b=pt[e.b].x*x.q;
        if(min(a,b)<x.p && x.p<max(a,b)){
            long long v=ordinateNumerator(e,x);
            for(auto [p,scale]:factors[abs(pt[e.a].x-pt[e.b].x)])
                knownRes[p]=(knownRes[p]+v*scale)%p;
        }
    }
    int k=c.slots.size();
    struct Entry{short slot,idx,res;};
    vector<vector<Entry>> rows(1001);
    vector<vector<char>> keep(k);
    vector<vector<int>> residue(k);
    for(int h=0;h<k;h++){
        auto& s=c.slots[h];
        keep[h].assign(s.partners.size(),1);
        residue[h].resize(s.partners.size());
        for(int j=0;j<(int)s.partners.size();j++)if(s.partners[j]>=0){
            Edge e=s.edge(s.partners[j]);
            long long v=ordinateNumerator(e,x);
            for(auto [p,scale]:factors[abs(pt[e.a].x-pt[e.b].x)]){
                int r=v*scale%p;
                if(r<0)r+=p;
                if(r)rows[p].push_back({(short)h,(short)j,(short)r});
            }
        }
    }
    for(int p=997;p>=11;p--)if(!rows[p].empty()){
        int pk=primePower[p],qu=x.q;
        while(qu%p==0){pk*=p;qu/=p;}
        int den=reduceString(reply.second,pk*p);
        int full=0;
        if(den%pk==0){
            full=reduceString(reply.first,p)*unitD[p]%p*(qu%p)%p*inverseSmall(den/pk,p)%p;
        }
        int target=(full-knownRes[p]+p)%p;
        vector<vector<int>> choices(k);
        for(auto [h,j,r]:rows[p])if(keep[h][j]){
            choices[h].push_back(r);residue[h][j]=r;
        }
        for(auto &v:choices){sort(v.begin(),v.end());v.erase(unique(v.begin(),v.end()),v.end());}
        for(int h=0;h<k;h++)if(!choices[h].empty()){
            array<char,1000> reach{};reach[0]=1;
            vector<int> live{0};
            for(int g=0;g<k && (int)live.size()<p;g++)if(g!=h){
                int len=live.size();
                for(int r:choices[g]){
                    for(int t=0;t<len;t++){
                        int w=r+live[t];if(w>=p)w-=p;
                        if(!reach[w]){reach[w]=1;live.push_back(w);}
                    }
                    if((int)live.size()==p)break;
                }
            }
            if((int)live.size()==p)continue;
            if(!reach[target]){
                for(int j=0;j<(int)keep[h].size();j++){
                    int w=target-residue[h][j];if(w<0)w+=p;
                    if(!reach[w])keep[h][j]=0;
                }
            }else{
                for(auto [g,j,r]:rows[p])if(g==h){
                    int w=target-r;if(w<0)w+=p;
                    if(!reach[w])keep[h][j]=0;
                }
            }
        }
        for(auto [h,j,r]:rows[p])residue[h][j]=0;
    }
    for(int h=0;h<k;h++){
        auto& v=c.slots[h].partners;int count=0;
        for(int j=0;j<(int)v.size();j++)if(keep[h][j])v[count++]=v[j];
        v.resize(count);
    }
}
constexpr long long scaleY=1<<20;
long long scaledEdge(Edge e,Q x) {
    long long a=ordinateNumerator(e,x)*scaleY, b=x.q*abs(pt[e.a].x-pt[e.b].x);
    return a/b-(a%b<0);
}
// Retaining 14 leading places of the two integers with a common decimal
// shift bounds the scaled quotient error by one (slice length <= 1000).
long long scaledLength(Q x){
    auto &[r,s]=exactReply.at(x.key());
    int drop=max(0,(int)max(r.size(),s.size())-14);
    auto prefix=[&](const string& t){
        long long z=0;
        for(int i=0;i<(int)t.size()-drop;i++)z=z*10+t[i]-'0';
        return z;
    };
    return (long long)((__int128)prefix(r)*scaleY/prefix(s));
}

struct Choice{long long y;F f;int partner;};
template<class Accept>
bool boundedSearch(const Cut& c,const Probe& pr,F target,vector<int>& pick,Accept accept){
    long long lo=scaledLength(pr.x)-2-(int)c.slots.size(),hi=scaledLength(pr.x)+2;
    for(Edge e:known){
        int a=pt[e.a].x*pr.x.q,b=pt[e.b].x*pr.x.q;
        if(min(a,b)<pr.x.p&&pr.x.p<max(a,b)){
            long long y=scaledEdge(e,pr.x);lo-=y+1;hi-=y;
        }
    }
    int k=c.slots.size();
    vector<vector<Choice>> choices(k);
    vector<int> wild;
    for(int i=0;i<k;i++){
        for(int j:c.slots[i].partners){
            if(j<0){wild.push_back(i);continue;}
            Edge e=c.slots[i].edge(j);
            choices[i].push_back({scaledEdge(e,pr.x),pr.value(e),j});
        }
        sort(choices[i].begin(),choices[i].end(),[](auto a,auto b){return a.y<b.y;});
    }
    int effort=0;
    for(int mask=0;mask<(1<<(int)wild.size());mask++){
        fill(pick.begin(),pick.end(),-2);
        int balance=0;long long low=0,high=0;
        for(int j=0;j<(int)wild.size();j++)if(mask>>j&1){
            pick[wild[j]]=-1;balance+=c.slots[wild[j]].out?1:-1;
        }
        if(balance)continue;
        vector<int> ext;
        for(int i=0;i<k;i++)if(pick[i]!=-1){
            if(choices[i].empty()){low=hi+1;break;}
            low+=choices[i].front().y;high+=choices[i].back().y;ext.push_back(i);
        }
        if(hi<low||lo>high)continue;
        // Hash the slots with the narrowest ordinate ranges. Enumerate the
        // others in order of decreasing spread, bounding each prefix by
        // the minimum and maximum of the remaining choices.
        auto spread=[&](int i){return choices[i].back().y-choices[i].front().y;};
        sort(ext.begin(),ext.end(),[&](int a,int b){return spread(a)<spread(b);});
        vector<int> hashed,outer;
        long long count=1,other=1;
        for(int i:ext)other=min(1000000000000LL,other*(long long)choices[i].size());
        for(int i:ext){
            if(count*(long long)choices[i].size()<=750000 && count<other){
                count*=choices[i].size();other=max(1LL,other/(long long)choices[i].size());
                hashed.push_back(i);
            }else outer.push_back(i);
        }
        reverse(outer.begin(),outer.end());
        vector<long long> mn(outer.size()+1),mx(outer.size()+1);
        for(int i:hashed){mn.back()+=choices[i].front().y;mx.back()+=choices[i].back().y;}
        for(int g=(int)outer.size()-1;g>=0;g--){
            auto &v=choices[outer[g]];
            mn[g]=mn[g+1]+v.front().y;mx[g]=mx[g+1]+v.back().y;
        }
        // Construct the hash table lazily, only after an outer sum falls
        // in range.
        vector<F> sums;
        vector<int> head,next;
        auto bucket=[&](F a){return int((a^(a>>31))*0x9e3779b97f4a7c15ULL>>24)&int(head.size()-1);};
        auto search=[&](auto&& self,int g,long long y,F f)->bool{
            if(++effort>3300000 || ((effort&16383)==0&&!beforeTime(6.65)))return false;
            if(g==(int)outer.size()){
                if(head.empty()){
                    effort+=count;
                    if(effort>3300000)return false;
                    sums={0};
                    for(int i:hashed){
                        vector<F> s;s.reserve(sums.size()*choices[i].size());
                        for(F a:sums)for(auto &b:choices[i])s.push_back(add(a,b.f));
                        sums.swap(s);
                    }
                    int sz=1;while(sz<2*(int)sums.size())sz*=2;
                    head.assign(sz,-1);next.resize(sums.size());
                    for(int i=0;i<(int)sums.size();i++){int b=bucket(sums[i]);next[i]=head[b];head[b]=i;}
                }
                F wanted=subf(target,f);
                for(int i=head[bucket(wanted)];i!=-1;i=next[i])if(sums[i]==wanted){
                    int z=i;
                    for(auto it=hashed.rbegin();it!=hashed.rend();++it){
                        auto &v=choices[*it];pick[*it]=v[z%v.size()].partner;z/=v.size();
                    }
                    if(!accept())return false;
                }
                return true;
            }
            int i=outer[g];auto &v=choices[i];
            long long lower=lo-y-mx[g+1],upper=hi-y-mn[g+1];
            auto it=lower_bound(v.begin(),v.end(),lower,[](const Choice &a,long long z){return a.y<z;});
            for(;it!=v.end() && it->y<=upper;it++){
                pick[i]=it->partner;
                if(!self(self,g+1,y+it->y,add(f,it->f)))return false;
            }
            return true;
        };
        if(!search(search,0,0,0))return false;
    }
    return true;
}
// Equality with the convex-hull width at a nonintegral x determines
// every crossing edge: any further crossing would leave an open gap.
bool fullHullSlice(Q x,vector<Edge>& es){
    long long numerator=0,den=1;
    vector<Edge> crossing;
    for(int i=0;i<(int)hull.size();i++){
        Edge e{hull[i],hull[(i+1)%hull.size()]};
        int a=pt[e.a].x,b=pt[e.b].x;
        if(min(a,b)*x.q>=x.p||max(a,b)*x.q<=x.p)continue;
        int d=abs(a-b);
        numerator=numerator*d+ordinateNumerator(e,x)*den;
        den*=d;
        if(outEdge[e.a]<0)crossing.push_back(e);
    }
    den*=x.q;
    long long g=gcd(numerator,den);
    auto &raw=exactReply.at(x.key());
    if(raw.first!=to_string(numerator/g)||raw.second!=to_string(den/g))return false;
    es=move(crossing);return true;
}
bool recoverCut(const vector<int>& vs,Cut &c,vector<Edge>& result) {
    int sep=c.separation;
    Q x((c.right?pt[vs[sep]].x:pt[vs[sep-1]].x)*997+(c.right?-1:1),997);
    Probe probe(x);
    F target=subf(ask(x),recorded(probe));
    if(fullHullSlice(x,result))return true;
    if(c.work>70000){
        sieve(c,x);
        c.work=balance(c);
    }
    int matches=0,attempts=0;
    vector<vector<unsigned char>> support(c.slots.size(),vector<unsigned char>(n));
    auto finish=[&](bool exhausted){
        if(exhausted && matches==1)return true;
        if(exhausted && matches>1){
            bool changed=false;
            for(int i=0;i<(int)c.slots.size();i++)for(int v:vs){
                Edge e=c.slots[i].edge(v);
                if(!support[i][v] && allowed(e.a,e.b)){
                    forbidden[e.a][e.b]=1;changed=true;
                }
            }
            if(changed)epoch++;
        }
        stalled[x.key()]=epoch;
        return false;
    };
    struct Crossing{long long y;int den;bool lower;};
    auto crossing=[&](Edge e){
        bool lower=pt[e.a].x<pt[e.b].x;
        return Crossing{ordinateNumerator(e,x)*(lower?-1:1),abs(pt[e.a].x-pt[e.b].x),lower};
    };
    vector<Crossing> crossings;
    for(Edge e:known)if(min(pt[e.a].x,pt[e.b].x)*x.q<x.p && x.p<max(pt[e.a].x,pt[e.b].x)*x.q)
        crossings.push_back(crossing(e));
    vector<int> partner(c.slots.size());
    auto accept=[&](){
        if(++attempts>12000 || ((attempts&127)==0&&!beforeTime(6.7)))return false;
        vector<Edge> es;
        vector<int> out,in;
        bool valid=true;
        for(int h=0;h<(int)partner.size();h++){
            auto &s=c.slots[h];
            if(partner[h]<0)(s.out?out:in).push_back(s.v);
            else{
                Edge e=s.edge(partner[h]);
                if(!fits(es,e)){valid=false;break;}
                es.push_back(e);
            }
        }
        if(!valid||out.size()!=in.size())return true;
        auto crossList=crossings;
        for(Edge e:es)crossList.push_back(crossing(e));
        sort(crossList.begin(),crossList.end(),[](auto a,auto b){return a.y*b.den<b.y*a.den;});
        for(int i=0;i<(int)crossList.size();i++)if(crossList[i].lower!=(i%2==0))return true;
        auto complete=[&](auto&& self,int i,int mask)->bool{
            if(i==(int)out.size())return true;
            for(int b=0;b<(int)in.size();b++)if(!(mask>>b&1)){
                Edge e{out[i],in[b]};
                if(!possible(e.a,e.b)||!fits(es,e))continue;
                es.push_back(e);
                bool ok=self(self,i+1,mask|(1<<b));
                es.pop_back();
                if(ok)return true;
            }
            return false;
        };
        if(!complete(complete,0,0))return true;
        if(++matches>160)return false;
        if(matches==1)result=es;
        for(int i=0;i<(int)c.slots.size();i++){
            if(partner[i]>=0)support[i][partner[i]]=1;
            else for(int j:c.slots[i].out?in:out)support[i][j]=1;
        }
        return true;
    };
    if(c.work>240000)
        return finish(boundedSearch(c,probe,target,partner,accept));
    vector<vector<F>> vals;
    for(auto &s:c.slots){
        vector<F> v;
        for(int j:s.partners) v.push_back(j<0?0:probe.value(s.edge(j)));
        vals.push_back(move(v));
    }
    auto sums=[&](const vector<int>& half){
        vector<F> sum{0};
        for(int i:half){
            vector<F> next;
            next.reserve(sum.size()*vals[i].size());
            for(F a:sum)for(F b:vals[i])next.push_back(add(a,b));
            sum.swap(next);
        }
        return sum;
    };
    auto left=sums(c.half[0]),right=sums(c.half[1]);
    int mask=1;
    while(mask<2*(int)left.size())mask*=2;
    vector<int> table(mask,-1),next(left.size());
    mask--;
    auto bucket=[&](F v){return int((v^(v>>31))*0x9e3779b97f4a7c15ULL>>24)&mask;};
    for(int i=0;i<(int)left.size();i++){
        int pos=bucket(left[i]);
        next[i]=table[pos];table[pos]=i;
    }
    auto unpack=[&](int half,int num){
        for(auto it=c.half[half].rbegin();it!=c.half[half].rend();++it){
            const auto &v=c.slots[*it].partners;
            partner[*it]=v[num%v.size()];num/=v.size();
        }
    };
    for(int j=0;j<(int)right.size();j++){
        F wanted=subf(target,right[j]);
        int head=table[bucket(wanted)];
        if(head<0)continue;
        for(int idx=head;idx>=0;idx=next[idx]){
            if(left[idx]!=wanted)continue;
            unpack(1,j);
            unpack(0,idx);
            if(!accept())return finish(false);
        }
    }
    return finish(true);
}

int hullMisses=0;
set<int> hullProbes;
F solve(vector<int> vs) {
    int failures=0,limit=10;
    while(true){
        vs.erase(remove_if(vs.begin(),vs.end(),[](int v){return inEdge[v]>=0 && outEdge[v]>=0;}),vs.end());
        if(vs.empty())return 0;
        if(pt[vs.front()].x==pt[vs.back()].x){
            // Remaining edges on a single column can only join neighbors.
            for(int i=1;i<(int)vs.size();i++){
                int a=vs[i-1],b=vs[i];
                if(outEdge[a]<0)record({a,b});
                else if(inEdge[a]<0)record({b,a});
            }
            return 0;
        }
        vector<int> inner;
        for(int v:vs)if(hr[v]<0)inner.push_back(v);
        if(inner.empty()){
            for(int v:vs)if(outEdge[v]<0)record({v,hull[(hr[v]+1)%hull.size()]});
            return 0;
        }
        if(inner.size()<=2 && vs.size()<(size_t)n && beforeTime(5.4) &&
           all_of(inner.begin(),inner.end(),[](int v){return inEdge[v]<0 && outEdge[v]<0;})){
            vector<Edge> gaps;
            int base=0;bool intact=true;
            for(int v:vs)if(hr[v]>=0 && outEdge[v]<0){
                Edge e=hullEdges[hr[v]];
                if(inEdge[e.b]>=0 || !binary_search(vs.begin(),vs.end(),e.b)){intact=false;break;}
                base+=area2(e);gaps.push_back(e);
            }
            if(intact && !gaps.empty()){
                // Every remaining interior path is inserted into one of
                // these complete gaps in the cyclic hull order.
                sort(gaps.begin(),gaps.end(),[](Edge a,Edge b){return hr[a.a]<hr[b.a];});
                if(auto area=polygonizations(vs,base,gaps))return num(*area);
            }
        }
        // Separate sparse pockets by testing a cut far from the remaining
        // interior vertices. A full slice isolates both sets of slots.
        if(hullMisses<2 && vs.size()>48 && inner.size()*14<vs.size()){
            int split=-1,best=-1;
            for(int i=vs.size()/4;i<(int)(3*vs.size()/4);i++){
                int a=pt[vs[i-1]].x,b=pt[vs[i]].x;
                if(a==b)continue;
                int d=2001;
                for(int v:inner)d=min(d,abs(2*pt[v].x-a-b));
                if(d>best){best=d;split=i;}
            }
            if(split>0){
                Q x=inside(pt[vs[split-1]].x,pt[vs[split]].x);
                if(hullProbes.insert(x.key()).second){
                    ask(x);vector<Edge> es;
                    if(fullHullSlice(x,es)){
                        for(Edge e:es)record(e);
                        F a=solve(vector<int>(vs.begin(),vs.begin()+split));
                        return add(a,solve(vector<int>(vs.begin()+split,vs.end())));
                    }
                    hullMisses++;
                }
            }
        }
        if(auto ans=smallMatching(vs))return num(*ans);
        if(!beforeTime(6.5) || failures>=6)return integrate(vs);
        Cut c;
        vector<Edge> forced;
        for(bool right:{false,true})
            if(planCut(vs,right,limit,c,forced))break;
        if(!forced.empty()){
            record(forced.front());continue;
        }
        if(!c.count || (failures && c.slots.size()<2))return integrate(vs);
        vector<Edge> es;
        int plannedCost=c.work,priorEpoch=epoch;
        if(!recoverCut(vs,c,es)){
            // A smaller group can break a collision. Preserve the general
            // quadrature fallback even when the field signatures coincide.
            if(epoch!=priorEpoch){limit=10;failures=0;continue;}
            if(plannedCost>240000)maxCutCost=min(maxCutCost,max(240000,plannedCost/2));
            limit=min(limit,int(c.slots.size()));
            if(limit<=0)return integrate(vs);
            failures++;continue;
        }
        maxCutCost=min(2000000000LL,1LL*maxCutCost+max(100000,maxCutCost/8));
        for(Edge e:es)record(e);
        vector<int> a(vs.begin(),vs.begin()+c.separation), b(vs.begin()+c.separation,vs.end());
        if(a.size()>b.size())a.swap(b);
        F left=solve(move(a));
        return add(left,solve(move(b)));
    }
}
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    if(!(cin>>n))return 0;
    pt.resize(n);
    for(auto &v:pt)if(!(cin>>v.x>>v.y))return 0;
    init();
    int hArea=0;
    for(int i=0;i<(int)hull.size();i++){
        hullEdges.push_back({hull[i],hull[(i+1)%hull.size()]});hArea+=area2(hullEdges.back());
    }
    F ans=hArea;
    if((int)hull.size()!=n){
        lines();tested.assign(n*n,0);
        vector<int> all(n);iota(all.begin(),all.end(),0);
        if(auto exact=polygonizations(all,hArea,hullEdges))ans=*exact;
        else{
            ans=solve(all);
            for(Edge e:known)ans=add(ans,num(area2(e)));
        }
    }
    int d=ans%2?1:2;
    cout<<"! "<<ans/d<<" "<<2/d<<endl;
}
