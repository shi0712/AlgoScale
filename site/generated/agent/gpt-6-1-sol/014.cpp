#include <bits/stdc++.h>
using namespace std;
/*
   Cut the exterior of the lake into vertical trapezoids.  Its dual graph
   consists of one cycle with trees attached.  A shortest path never visits
   a convex cell twice, so there are at most two possible cell sequences.

   For one sequence, the distance along an entry portal is a sequence of
   functions d + |p - root|.  Extending the separating rays across a cell,
   and adding the two portal endpoints, propagates this shortest-path map.
   Each resulting region is a convex polygon, and its reachable part is
   its intersection with the disk of radius k-d about its root.

   Process both directions of the cycle, and both arrivals together in its
   attached trees.  Areas of polygon/disk intersections (and intersections
   with two disks where the arrivals overlap) are integrated analytically.
   Equal bounding edges are merged across consecutive vertical slabs.
   There are only 401 possible vertex x coordinates; a map ray crosses
   only O(401) vertical walls.
*/
using ld = long double;
const double EPS=1e-10;
const ld PI=acosl(-1.0L), TAU=2*PI;
struct Pt { double x,y; Pt operator+(Pt b)const{return {x+b.x,y+b.y};} Pt operator-(Pt b)const{return {x-b.x,y-b.y};} Pt operator*(double t)const{return {x*t,y*t};} };
static inline double dot(Pt a,Pt b){return a.x*b.x+a.y*b.y;}
static inline double cross(Pt a,Pt b){return a.x*b.y-a.y*b.x;}
static inline double norm(Pt a){return hypot(a.x,a.y);}
struct Seg {double a,b; double y(double x)const{return a*x+b;}};
struct Cell {double l,r;int bot,top;};
struct Edge {int u,v;double x,lo,hi;int other(int a)const{return u^v^a;}};
struct Root {Pt p;double d;};
struct Piece {Root root;double lo,hi;};
using Profile=vector<Piece>;
struct Band {Root root;Seg low,high;bool haslow=false,hashigh=false;};
struct Map {vector<Band> bands;};
vector<Seg> segs;
vector<Cell> cells;
vector<Edge> edges;
vector<vector<int>> adj;
double K;

static inline bool sameRoot(const Root&a,const Root&b){return abs(a.p.x-b.p.x)<1e-10&&abs(a.p.y-b.p.y)<1e-10&&abs(a.d-b.d)<1e-10;}
static void addPiece(Profile &p,Root root,double lo,double hi){
    if(hi-lo<1e-11)return;
    if(!p.empty()&&sameRoot(p.back().root,root)&&abs(p.back().hi-lo)<1e-8)p.back().hi=hi;
    else p.push_back({root,lo,hi});
}
static void prune(Profile &p,double x){
    double best=1e100;
    for(auto &a:p){double y=clamp(a.root.p.y,a.lo,a.hi);best=min(best,a.root.d+hypot(x-a.root.p.x,y-a.root.p.y));}
    if(best>K+1e-9)p.clear();
}
static Map sourceMap(){Map m;m.bands.push_back({{{0,0},0},{},{},false,false});return m;}
static Map makeMap(const Profile&p,const Edge&e){
    Map m;if(p.empty())return m;
    // A root on the entry wall reduces to the closest point of the portal.
    if(p.size()==1&&abs(p[0].root.p.x-e.x)<1e-10){
        Root r=p[0].root;double y=clamp(r.p.y,e.lo,e.hi);r.d+=abs(y-r.p.y);r.p={e.x,y};
        m.bands.push_back({r,{},{},false,false});return m;
    }
    auto ray=[&](Root r,double y){double a=(y-r.p.y)/(e.x-r.p.x);return Seg{a,y-a*e.x};};
    Seg first=ray(p.front().root,e.lo),last=ray(p.back().root,e.hi);
    Root lower{{e.x,e.lo},p.front().root.d+hypot(e.x-p.front().root.p.x,e.lo-p.front().root.p.y)};
    Root upper{{e.x,e.hi},p.back().root.d+hypot(e.x-p.back().root.p.x,e.hi-p.back().root.p.y)};
    m.bands.reserve(p.size()+2);
    m.bands.push_back({lower,{},first,false,true});
    for(auto &a:p)m.bands.push_back({a.root,ray(a.root,a.lo),ray(a.root,a.hi),true,true});
    m.bands.push_back({upper,last,{},true,false});
    return m;
}
static Profile outProfile(const Map&m,const Edge&e){
    Profile p;p.reserve(m.bands.size());
    for(auto &b:m.bands){
        double lo=e.lo,hi=e.hi;
        if(b.haslow)lo=max(lo,b.low.y(e.x));
        if(b.hashigh)hi=min(hi,b.high.y(e.x));
        addPiece(p,b.root,lo,hi);
    }
    prune(p,e.x);return p;
}
static double distanceAt(const Piece&a,double x,double y){return a.root.d+hypot(x-a.root.p.x,y-a.root.p.y);}
// If one arrival is no better anywhere on the portal, discard it for the entire branch.
static void dominance(Profile&a,Profile&b,const Edge&e){
    if(a.empty()||b.empty())return;
    size_t i=0,j=0;bool ale=true,ble=true;
    while(i<a.size()&&j<b.size()){
        double lo=max(a[i].lo,b[j].lo),hi=min(a[i].hi,b[j].hi);
        if(lo<=hi+1e-10){
            auto check=[&](double y){double d=distanceAt(a[i],e.x,y)-distanceAt(b[j],e.x,y);if(d>1e-10)ale=false;if(d< -1e-10)ble=false;};
            check(lo);check(hi);
            double aa=abs(a[i].root.p.x-e.x),bb=abs(b[j].root.p.x-e.x);
            if(abs(aa-bb)>1e-12){double y=(bb*a[i].root.p.y-aa*b[j].root.p.y)/(bb-aa);if(y>lo&&y<hi)check(y);}
            double y=a[i].root.p.y;if(y>lo&&y<hi)check(y);
            y=b[j].root.p.y;if(y>lo&&y<hi)check(y);
        }
        double ah=a[i].hi,bh=b[j].hi;
        if(ah<=bh+1e-10)++i;
        if(bh<=ah+1e-10)++j;
        if(!ale&&!ble)return;
    }
    if(ale)b.clear();else if(ble)a.clear();
}
using Poly=vector<Pt>;
static Poly cellPoly(int id){
    auto &c=cells[id];return {{c.l,segs[c.bot].y(c.l)},{c.r,segs[c.bot].y(c.r)},{c.r,segs[c.top].y(c.r)},{c.l,segs[c.top].y(c.l)}};
}
static Poly clip(const Poly&p,Seg s,bool lower){
    Poly q;if(p.empty())return q;q.reserve(p.size()+1);
    auto val=[&](Pt a){double v=a.y-s.y(a.x);return lower?v:-v;};
    Pt a=p.back();double va=val(a);
    for(Pt b:p){double vb=val(b);bool ia=va>=0,ib=vb>=0;
        if(ia!=ib){double t=va/(va-vb);q.push_back(a+(b-a)*t);}
        if(ib)q.push_back(b);a=b;va=vb;
    }
    return q;
}
static ld polyArea(const Poly&p){
    if(p.size()<3)return 0;
    ld a=0;Pt o=p[0];for(size_t i=1;i+1<p.size();++i){Pt u=p[i]-o,v=p[i+1]-o;a+=(ld)u.x*v.y-(ld)u.y*v.x;}return max((ld)0,a/2);
}
static ld circlePoly(const Poly&p,Pt c,double radius){
    if(p.size()<3||radius<=0)return 0;
    ld r=radius,r2=r*r,res=0;
    for(size_t i=0;i<p.size();++i){
        ld ax=(ld)p[i].x-c.x,ay=(ld)p[i].y-c.y,bx=(ld)p[(i+1)%p.size()].x-c.x,by=(ld)p[(i+1)%p.size()].y-c.y;
        ld vx=bx-ax,vy=by-ay,A=vx*vx+vy*vy;if(A<1e-30L)continue;
        ld B=ax*vx+ay*vy,C=ax*ax+ay*ay-r2,D=B*B-A*C;
        ld t[4]={0,1,0,0};int cnt=2;
        if(D>0){ld h=sqrtl(D),u=(-B-h)/A,v=(-B+h)/A;if(u>0&&u<1)t[cnt++]=u;if(v>0&&v<1)t[cnt++]=v;}
        sort(t,t+cnt);
        for(int j=1;j<cnt;++j){ld l=t[j-1],h=t[j],mx=ax+vx*(l+h)/2,my=ay+vy*(l+h)/2;
            ld ux=ax+vx*l,uy=ay+vy*l,wx=ax+vx*h,wy=ay+vy*h,cr=ux*wy-uy*wx;
            if(mx*mx+my*my<=r2)res+=cr;
            else res+=r2*atan2l(cr,ux*wx+uy*wy);
        }
    }
    return max((ld)0,res/2);
}
struct Arc {ld l,r;};
static void restrictArcs(vector<Arc>&arcs,ld phi,ld t){
    if(arcs.empty()||t<=-1)return;
    if(t>=1){arcs.clear();return;}
    ld alpha=acosl(t);while(phi<0)phi+=TAU;while(phi>=TAU)phi-=TAU;
    ld l=phi-alpha,r=phi+alpha;while(l<0){l+=TAU;r+=TAU;}
    Arc allowed[2];int count;
    if(r<=TAU){allowed[0]={l,r};count=1;}
    else {allowed[0]={0,r-TAU};allowed[1]={l,TAU};count=2;}
    vector<Arc> out;out.reserve(arcs.size()+1);
    for(auto a:arcs)for(int j=0;j<count;++j){ld u=max(a.l,allowed[j].l),v=min(a.r,allowed[j].r);if(v>u)out.push_back({u,v});}
    arcs.swap(out);
}
static ld circleBoundary(const Poly&p,Pt c,double radius,Pt other,double ro){
    ld r=radius;vector<Arc> arcs{{0,TAU}};
    for(size_t i=0;i<p.size()&&!arcs.empty();++i){
        Pt a=p[i],b=p[(i+1)%p.size()];ld dx=(ld)b.x-a.x,dy=(ld)b.y-a.y,len=hypotl(dx,dy);if(len<1e-18L)continue;
        ld cr=dx*((ld)c.y-a.y)-dy*((ld)c.x-a.x);
        restrictArcs(arcs,atan2l(dx,-dy),-cr/(r*len));
    }
    ld dx=(ld)other.x-c.x,dy=(ld)other.y-c.y,len=hypotl(dx,dy);
    if(len>1e-18L)restrictArcs(arcs,atan2l(dy,dx),(r*r+len*len-(ld)ro*ro)/(2*r*len));
    else if(radius>ro)arcs.clear();
    ld sum=0;
    for(auto a:arcs)sum+=r*r*(a.r-a.l)+r*c.x*(sinl(a.r)-sinl(a.l))-r*c.y*(cosl(a.r)-cosl(a.l));
    return sum/2;
}
static ld twoCirclesPoly(const Poly&p,Pt c1,double r1,Pt c2,double r2){
    if(r1<=0||r2<=0||p.size()<3)return 0;
    double dist=norm(c1-c2);
    if(dist>=r1+r2)return 0;
    if(dist+r1<=r2+1e-12)return circlePoly(p,c1,r1);
    if(dist+r2<=r1+1e-12)return circlePoly(p,c2,r2);
    // Translate the integral to the first center for improved precision.
    Poly q=p;for(auto &a:q)a=a-c1;c2=c2-c1;c1={0,0};
    ld result=0;
    for(size_t i=0;i<q.size();++i){
        Pt a=q[i],b=q[(i+1)%q.size()];ld vx=(ld)b.x-a.x,vy=(ld)b.y-a.y,A=vx*vx+vy*vy;if(A<1e-30L)continue;
        ld lo=0,hi=1;
        auto inside=[&](Pt c,double r){ld ax=(ld)a.x-c.x,ay=(ld)a.y-c.y,B=ax*vx+ay*vy,C=ax*ax+ay*ay-(ld)r*r,D=B*B-A*C;if(D<0){lo=1;hi=0;return;}ld h=sqrtl(max((ld)0,D));lo=max(lo,(-B-h)/A);hi=min(hi,(-B+h)/A);};
        inside(c1,r1);inside(c2,r2);
        if(hi>lo)result+=((ld)a.x*b.y-(ld)a.y*b.x)*(hi-lo)/2;
    }
    result+=circleBoundary(q,c1,r1,c2,r2)+circleBoundary(q,c2,r2,c1,r1);
    return max((ld)0,result);
}
struct Region {Poly p;Root root;double radius;Band band;bool full;double minx,maxx,miny,maxy;};
struct Regions {vector<Region> parts;ld area=0;bool full=false;};
static Regions regions(const Map&m,const Poly&cp,ld ca){
    Regions all;if(m.bands.empty())return all;bool full=true;
    for(const auto &b:m.bands){
        Poly p=cp;if(b.haslow)p=clip(p,b.low,true);if(b.hashigh)p=clip(p,b.high,false);
        ld pa=polyArea(p);if(pa<1e-17L)continue;
        double r=K-b.root.d;if(r<=0){full=false;continue;}
        bool inside=true;
        for(Pt a:p){double d=dot(a-b.root.p,a-b.root.p);if(d>r*r+1e-10)inside=false;}
        ld a=inside?pa:min(pa,circlePoly(p,b.root.p,r));
        if(!inside)full=false;
        if(a<=1e-17L)continue;
        Region reg{move(p),b.root,r,b,inside,1e100,-1e100,1e100,-1e100};
        for(Pt a:reg.p){reg.minx=min(reg.minx,a.x);reg.maxx=max(reg.maxx,a.x);reg.miny=min(reg.miny,a.y);reg.maxy=max(reg.maxy,a.y);}
        all.area+=a;all.parts.push_back(move(reg));
    }
    all.full=full;if(full)all.area=ca;return all;
}
static ld reachableArea(int id,const Map&a,const Map&b){
    Poly cp=cellPoly(id);ld ca=polyArea(cp);if(ca<1e-17L)return 0;
    Regions aa=regions(a,cp,ca);if(aa.full)return ca;
    Regions bb=regions(b,cp,ca);if(bb.full)return ca;
    if(aa.parts.empty())return bb.area;if(bb.parts.empty())return aa.area;
    ld overlap=0;
    for(auto &x:aa.parts)for(auto &y:bb.parts){
        if(x.maxx<=y.minx+1e-12||y.maxx<=x.minx+1e-12||x.maxy<=y.miny+1e-12||y.maxy<=x.miny+1e-12)continue;
        if(!x.full&&!y.full&&norm(x.root.p-y.root.p)>=x.radius+y.radius)continue;
        Poly p=x.p;if(y.band.haslow)p=clip(p,y.band.low,true);if(y.band.hashigh)p=clip(p,y.band.high,false);
        ld ar=polyArea(p);if(ar<1e-17L)continue;
        ld inter;
        if(x.full&&y.full)inter=ar;
        else if(x.full)inter=circlePoly(p,y.root.p,y.radius);
        else if(y.full)inter=circlePoly(p,x.root.p,x.radius);
        else inter=twoCirclesPoly(p,x.root.p,x.radius,y.root.p,y.radius);
        overlap+=min(ar,inter);
    }
    return clamp(aa.area+bb.area-overlap,(ld)0,ca);
}

struct ActiveCmp {
    const double *x;const vector<Seg>*s;
    bool operator()(int a,int b)const {double ya=(*s)[a].y(*x),yb=(*s)[b].y(*x);if(ya!=yb)return ya<yb;return a<b;}
};
static void buildDecomposition(const vector<Pt>&p){
    int n=p.size();double R=max(201.0,K)+1;
    vector<double> xs{-R,R,0};for(Pt a:p)xs.push_back(a.x);sort(xs.begin(),xs.end());xs.erase(unique(xs.begin(),xs.end()),xs.end());
    segs.resize(n+2);segs[n]={0,-R};segs[n+1]={0,R};
    vector<vector<int>> starts(xs.size()),ends(xs.size());
    for(int i=0;i<n;++i){Pt a=p[i],b=p[(i+1)%n];if(a.x==b.x)continue;double slope=(b.y-a.y)/(b.x-a.x);segs[i]={slope,a.y-slope*a.x};
        int l=lower_bound(xs.begin(),xs.end(),min(a.x,b.x))-xs.begin(),r=lower_bound(xs.begin(),xs.end(),max(a.x,b.x))-xs.begin();starts[l].push_back(i);ends[r].push_back(i);
    }
    double sweep=0;set<int,ActiveCmp> active((ActiveCmp{&sweep,&segs}));
    vector<set<int,ActiveCmp>::iterator> its(n);
    vector<int> stamp(n+2,-10),prevTop(n+2),prevCell(n+2);
    vector<int> oldgap,newgap;
    cells.reserve(2*n+10);edges.reserve(2*n+10);
    for(int s=0;s+1<(int)xs.size();++s){
        double l=xs[s],r=xs[s+1];for(int id:ends[s])active.erase(its[id]);sweep=(l+r)/2;for(int id:starts[s])its[id]=active.insert(id).first;
        newgap.clear();int bottom=n;bool outside=true;
        auto gap=[&](int bot,int top){
            int id;
            if(l!=0&&stamp[bot]==s-1&&prevTop[bot]==top){id=prevCell[bot];cells[id].r=r;}
            else {id=cells.size();cells.push_back({l,r,bot,top});}
            stamp[bot]=s;prevTop[bot]=top;prevCell[bot]=id;newgap.push_back(id);
        };
        for(int id:active){if(outside)gap(bottom,id);else bottom=id;outside=!outside;}
        gap(bottom,n+1);
        size_t i=0,j=0;
        while(i<oldgap.size()&&j<newgap.size()){
            int u=oldgap[i],v=newgap[j];double al=segs[cells[u].bot].y(l),ah=segs[cells[u].top].y(l),bl=segs[cells[v].bot].y(l),bh=segs[cells[v].top].y(l);
            double lo=max(al,bl),hi=min(ah,bh);
            if(u!=v&&hi-lo>1e-9)edges.push_back({u,v,l,lo,hi});
            if(ah<=bh+1e-9)++i;if(bh<=ah+1e-9)++j;
        }
        oldgap.swap(newgap);
    }
    adj.resize(cells.size());for(int i=0;i<(int)edges.size();++i){adj[edges[i].u].push_back(i);adj[edges[i].v].push_back(i);}
}

static ld answer=0;
struct Task {int v,entry;Profile a,b;};
static void branch(int v,int entry,Profile a,Profile b){
    if(a.empty()&&b.empty())return;
    vector<Task> stack;stack.push_back({v,entry,move(a),move(b)});
    while(!stack.empty()){
        Task t=move(stack.back());stack.pop_back();dominance(t.a,t.b,edges[t.entry]);
        Map ma=makeMap(t.a,edges[t.entry]),mb=makeMap(t.b,edges[t.entry]);
        answer+=reachableArea(t.v,ma,mb);
        for(int e:adj[t.v])if(e!=t.entry){
            Profile pa=outProfile(ma,edges[e]),pb=outProfile(mb,edges[e]);
            if(!pa.empty()||!pb.empty())stack.push_back({edges[e].other(t.v),e,move(pa),move(pb)});
        }
    }
}
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int n;cin>>n>>K;vector<Pt> p(n);for(auto &a:p)cin>>a.x>>a.y;
    buildDecomposition(p);
    int N=cells.size();vector<int> degree(N);vector<char> cycle(N,true);queue<int> q;
    for(int i=0;i<N;++i){degree[i]=adj[i].size();if(degree[i]<=1)q.push(i);}
    while(!q.empty()){int v=q.front();q.pop();cycle[v]=false;for(int e:adj[v]){int u=edges[e].other(v);if(--degree[u]==1)q.push(u);}}
    int source=-1;for(int i=0;i<N;++i){auto &c=cells[i];if(c.l<=0&&c.r>=0&&segs[c.bot].y(0)<=1e-9&&segs[c.top].y(0)>=-1e-9){source=i;break;}}
    if(source==-1){cout<<"0\n";return 0;}
    vector<int> parent(N,-2),parentEdge(N,-1);parent[source]=-1;q.push(source);int attach=-1;
    while(!q.empty()){int v=q.front();q.pop();if(cycle[v]){attach=v;break;}for(int e:adj[v]){int u=edges[e].other(v);if(parent[u]==-2){parent[u]=v;parentEdge[u]=e;q.push(u);}}}
    vector<int> path;for(int v=attach;v!=-1;v=parent[v])path.push_back(v);reverse(path.begin(),path.end());
    Profile incoming;Map attachMap;
    for(int i=0;i<(int)path.size();++i){
        int v=path[i];Map m=i==0?sourceMap():makeMap(incoming,edges[parentEdge[v]]);
        answer+=reachableArea(v,m,{});
        int pre=i?parentEdge[v]:-1,nxt=i+1<(int)path.size()?parentEdge[path[i+1]]:-1;
        for(int e:adj[v])if(e!=pre&&e!=nxt&&!cycle[edges[e].other(v)])branch(edges[e].other(v),e,outProfile(m,edges[e]),{});
        if(nxt!=-1)incoming=outProfile(m,edges[nxt]);else attachMap=move(m);
    }
    vector<int> cv{attach},ce;int v=attach,last=-1;
    do {
        int chosen=-1;for(int e:adj[v])if(e!=last&&cycle[edges[e].other(v)]){chosen=e;break;}
        if(chosen==-1)break;ce.push_back(chosen);v=edges[chosen].other(v);last=chosen;if(v!=attach)cv.push_back(v);
    }while(v!=attach);
    int L=cv.size();vector<Profile> clockwise(L);
    if(!ce.empty()){
        Profile cur=outProfile(attachMap,edges[ce[0]]);
        for(int i=1;i<L;++i){clockwise[i]=move(cur);Map m=makeMap(clockwise[i],edges[ce[i-1]]);cur=outProfile(m,edges[ce[i]]);}
        cur=outProfile(attachMap,edges[ce[L-1]]);
        for(int i=L-1;i>=1;--i){
            Map a=makeMap(clockwise[i],edges[ce[i-1]]),b=makeMap(cur,edges[ce[i]]);answer+=reachableArea(cv[i],a,b);
            for(int e:adj[cv[i]])if(!cycle[edges[e].other(cv[i])])branch(edges[e].other(cv[i]),e,outProfile(a,edges[e]),outProfile(b,edges[e]));
            cur=outProfile(b,edges[ce[i-1]]);
        }
    }
    cout<<fixed<<setprecision(12)<<answer<<'\n';
}
