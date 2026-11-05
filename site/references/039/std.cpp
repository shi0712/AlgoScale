// Standard solution for "Fence Beacons" / 围栏信标.
// Delaunay quad-edge divide-and-conquer follows the Guibas-Stolfi framework;
// implementation structure was cross-checked against CP-Algorithms:
// https://cp-algorithms.com/geometry/delaunay.html
// See LICENSE_NOTES.md in the author package for attribution/redistribution notes.

#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using i128 = __int128_t;
using ld = long double;

static constexpr ll COORD = 10000;
static constexpr ll K = 2 * COORD + 1; // 20001, makes X = K*x+y injective on all allowed integer sites.
static constexpr ll XLIM = K * COORD + COORD + 1; // strictly outside every polygon vertex in sweep X.

struct Point {
    ll x, y;
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    bool operator<(const Point& o) const { return x != o.x ? x < o.x : y < o.y; }
};

static inline ll cross(const Point& a, const Point& b, const Point& c) {
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
}
static inline ll sqr(const Point& a) { return a.x*a.x + a.y*a.y; }

struct QPoint { // (x/w, y/w), w > 0, reduced
    ll x, y, w;
};
struct Fraction { ll n, d; }; // d > 0

static int cmpFrac(const Fraction& a, const Fraction& b) {
    i128 z = (i128)a.n*b.d - (i128)b.n*a.d;
    return (z>0) - (z<0);
}
static Fraction sweepX(const QPoint& p, int dir=1) {
    return {dir*(K*p.x + p.y), p.w};
}

struct Line { ll a,b,c; }; // a*x+b*y=c

static bool lineIntersect(const Line& A, const Line& B, QPoint& p) {
    ll w = A.a*B.b - A.b*B.a;
    if (!w) return false;
    ll x = A.c*B.b - A.b*B.c;
    ll y = A.a*B.c - A.c*B.a;
    if (w < 0) x=-x, y=-y, w=-w;
    ll g = std::gcd(std::gcd(llabs(x), llabs(y)), w);
    p = {x/g, y/g, w/g};
    return true;
}
static Line through(const Point& p, const Point& q) {
    return {p.y-q.y, q.x-p.x, p.y*q.x-p.x*q.y};
}
static Line bisector(const Point& p, const Point& q) {
    return {2*(q.x-p.x), 2*(q.y-p.y), q.x*q.x+q.y*q.y-p.x*p.x-p.y*p.y};
}
static QPoint circumcenter(const Point& a, const Point& b, const Point& c) {
    QPoint q{};
    bool ok = lineIntersect(bisector(a,b), bisector(a,c), q);
    assert(ok);
    return q;
}

// ---------------- exact divide-and-conquer Delaunay (Guibas-Stolfi) ----------------

struct QBlock;
struct QEdge {
    int org = -1; // index in sorted-point array, -1 for dual edge
    QEdge* rot = nullptr;
    QEdge* onext = nullptr;
    QBlock* owner = nullptr;
    bool used = false;
    QEdge* rev() const { return rot->rot; }
    QEdge* lnext() const { return rot->rev()->onext->rot; }
    QEdge* oprev() const { return rot->onext->rot; }
    int dest() const { return rev()->org; }
};
struct QBlock { QEdge e[4]; size_t slot = 0; };

struct DelaunayBuilder {
    struct SP { Point p; int orig; };
    vector<SP> p;
    vector<QBlock*> alive;

    ~DelaunayBuilder() { clear(); }
    void clear() {
        for (auto* b: alive) delete b;
        alive.clear();
    }

    QEdge* makeEdge(int from, int to) {
        QBlock* b = new QBlock;
        b->slot = alive.size();
        alive.push_back(b);
        QEdge *e1=&b->e[0], *e2=&b->e[1], *e3=&b->e[2], *e4=&b->e[3];
        e1->org=from; e2->org=to; e3->org=e4->org=-1;
        e1->rot=e3; e2->rot=e4; e3->rot=e2; e4->rot=e1;
        e1->onext=e1; e2->onext=e2; e3->onext=e4; e4->onext=e3;
        for (int i=0;i<4;i++) b->e[i].owner=b, b->e[i].used=false;
        return e1;
    }
    static void splice(QEdge* a, QEdge* b) {
        swap(a->onext->rot->onext, b->onext->rot->onext);
        swap(a->onext, b->onext);
    }
    void deleteEdge(QEdge* e) {
        splice(e, e->oprev());
        splice(e->rev(), e->rev()->oprev());
        QBlock* b=e->owner;
        size_t s=b->slot;
        QBlock* last=alive.back();
        alive[s]=last; last->slot=s;
        alive.pop_back();
        delete b;
    }
    QEdge* connect(QEdge* a, QEdge* b) {
        QEdge* e=makeEdge(a->dest(), b->org);
        splice(e, a->lnext());
        splice(e->rev(), b);
        return e;
    }
    ll orient(int a,int b,int c) const { return cross(p[a].p,p[b].p,p[c].p); }
    bool leftOf(int x,QEdge* e) const { return orient(x,e->org,e->dest())>0; }
    bool rightOf(int x,QEdge* e) const { return orient(x,e->org,e->dest())<0; }

    template<class T>
    static T det3(T a1,T a2,T a3,T b1,T b2,T b3,T c1,T c2,T c3) {
        return a1*(b2*c3-c2*b3)-a2*(b1*c3-c1*b3)+a3*(b1*c2-c1*b2);
    }
    bool inCircle(int ia,int ib,int ic,int id) const {
        const Point &a=p[ia].p,&b=p[ib].p,&c=p[ic].p,&d=p[id].p;
        i128 det = -det3<i128>(b.x,b.y,sqr(b), c.x,c.y,sqr(c), d.x,d.y,sqr(d));
        det += det3<i128>(a.x,a.y,sqr(a), c.x,c.y,sqr(c), d.x,d.y,sqr(d));
        det -= det3<i128>(a.x,a.y,sqr(a), b.x,b.y,sqr(b), d.x,d.y,sqr(d));
        det += det3<i128>(a.x,a.y,sqr(a), b.x,b.y,sqr(b), c.x,c.y,sqr(c));
        return det>0;
    }

    pair<QEdge*,QEdge*> build(int l,int r) {
        if (r-l+1==2) {
            QEdge* e=makeEdge(l,r);
            return {e,e->rev()};
        }
        if (r-l+1==3) {
            QEdge* a=makeEdge(l,l+1);
            QEdge* b=makeEdge(l+1,r);
            splice(a->rev(),b);
            ll cr=orient(l,l+1,r);
            if (!cr) return {a,b->rev()};
            QEdge* c=connect(b,a);
            if (cr>0) return {a,b->rev()};
            return {c->rev(),c};
        }
        int m=(l+r)>>1;
        QEdge *ldo,*ldi,*rdo,*rdi;
        tie(ldo,ldi)=build(l,m);
        tie(rdi,rdo)=build(m+1,r);
        while (true) {
            if (leftOf(rdi->org,ldi)) { ldi=ldi->lnext(); continue; }
            if (rightOf(ldi->org,rdi)) { rdi=rdi->rev()->onext; continue; }
            break;
        }
        QEdge* basel=connect(rdi->rev(),ldi);
        auto valid=[&](QEdge* e){ return rightOf(e->dest(),basel); };
        if (ldi->org==ldo->org) ldo=basel->rev();
        if (rdi->org==rdo->org) rdo=basel;
        while (true) {
            QEdge* lcand=basel->rev()->onext;
            if (valid(lcand)) {
                while (inCircle(basel->dest(),basel->org,lcand->dest(),lcand->onext->dest())) {
                    QEdge* t=lcand->onext;
                    deleteEdge(lcand);
                    lcand=t;
                }
            }
            QEdge* rcand=basel->oprev();
            if (valid(rcand)) {
                while (inCircle(basel->dest(),basel->org,rcand->dest(),rcand->oprev()->dest())) {
                    QEdge* t=rcand->oprev();
                    deleteEdge(rcand);
                    rcand=t;
                }
            }
            if (!valid(lcand) && !valid(rcand)) break;
            if (!valid(lcand) || (valid(rcand) && inCircle(lcand->dest(),lcand->org,rcand->org,rcand->dest())))
                basel=connect(rcand,basel->rev());
            else
                basel=connect(basel->rev(),lcand->rev());
        }
        return {ldo,rdo};
    }

    vector<array<int,3>> run(const vector<Point>& sites) {
        int n=sites.size();
        p.resize(n);
        for(int i=0;i<n;i++) p[i]={sites[i],i};
        sort(p.begin(),p.end(),[](const SP&a,const SP&b){return a.p<b.p;});
        build(0,n-1);

        vector<array<int,3>> tris;
        tris.reserve(2*n);
        // Every live primal directed edge belongs to exactly one left face.
        // Traverse every face; keep only positively oriented triangular faces.
        for (QBlock* b: alive) for (int z=0;z<2;z++) {
            QEdge* start=&b->e[z];
            if (start->used) continue;
            vector<int> face;
            QEdge* e=start;
            do {
                e->used=true;
                face.push_back(e->org);
                e=e->lnext();
            } while(e!=start);
            if (face.size()==3) {
                ll cr=orient(face[0],face[1],face[2]);
                if (cr>0) tris.push_back({p[face[0]].orig,p[face[1]].orig,p[face[2]].orig});
            }
        }
        clear();
        vector<SP>().swap(p);
        return tris;
    }
};

// ---------------- sweep-line primitives ----------------

struct Segment {
    QPoint p,q;
    Line line;
    int site; // -1 red polygon edge; >=0 blue Voronoi edge, with one nearest site
};
struct Candidate { QPoint p; int site; };
struct Height { ll a,b,d; }; // Y=(a*X+b)/d in sheared coordinates, d>0

static vector<Height> makeHeights(const vector<Segment>& s,int dir) {
    vector<Height> h; h.reserve(s.size());
    for (auto& e:s) {
        Height t{-dir*e.line.a, K*e.line.c, K*e.line.b-e.line.a};
        assert(t.d!=0);
        if(t.d<0) t.a=-t.a,t.b=-t.b,t.d=-t.d;
        h.push_back(t);
    }
    return h;
}
struct YQuery { ll y,w; };
struct StatusCmp {
    using is_transparent=void;
    const vector<Height>* h;
    const Fraction* x;
    const bool* rightSide;
    i128 num(int i) const { return (i128)(*h)[i].a*x->n + (i128)(*h)[i].b*x->d; }
    bool operator()(int i,int j) const {
        if(i==j) return false;
        const auto&A=(*h)[i]; const auto&B=(*h)[j];
        i128 u=num(i)*B.d, v=num(j)*A.d;
        if(u!=v) return u<v;
        u=(i128)A.a*B.d; v=(i128)B.a*A.d;
        if(u!=v) return *rightSide ? u<v : u>v;
        return i<j;
    }
    int compareY(int i,YQuery q) const {
        i128 u=num(i)*q.w;
        i128 v=(i128)q.y*(*h)[i].d*x->d;
        return (u>v)-(u<v);
    }
    bool operator()(int i,YQuery q) const { return compareY(i,q)<0; }
    bool operator()(YQuery q,int i) const { return compareY(i,q)>0; }
};
struct Event { Fraction x; int type,u,v; };
struct EventGreater {
    bool operator()(const Event&a,const Event&b) const {
        int c=cmpFrac(a.x,b.x); if(c) return c>0;
        return tie(a.type,a.u,a.v)>tie(b.type,b.u,b.v);
    }
};

static vector<Point> sites;
static ld ans2=0;
static void relax(const QPoint& q,int site) {
    ld x=(ld)q.x/q.w, y=(ld)q.y/q.w;
    ld dx=x-sites[site].x, dy=y-sites[site].y;
    ans2=max(ans2,dx*dx+dy*dy);
}

// Batch point location for all Voronoi vertices. Red edges never properly intersect.
static void checkVertices(const vector<Segment>& red,const vector<Candidate>& qs) {
    int n=red.size(); auto h=makeHeights(red,1);
    Fraction x{0,1}; bool rightSide=true;
    StatusCmp comp{&h,&x,&rightSide};
    set<int,StatusCmp> st(comp);
    vector<set<int,StatusCmp>::iterator> pos(n);
    vector<char> forward(n);
    vector<Event> ev; ev.reserve(2*n+qs.size());
    for(int i=0;i<n;i++) {
        auto L=sweepX(red[i].p), R=sweepX(red[i].q);
        forward[i]=cmpFrac(L,R)<0;
        if(!forward[i]) swap(L,R);
        ev.push_back({L,1,i,-1}); ev.push_back({R,0,i,-1});
    }
    for(int i=0;i<(int)qs.size();i++) ev.push_back({sweepX(qs[i].p),2,i,-1});
    sort(ev.begin(),ev.end(),[](const Event&a,const Event&b){
        int c=cmpFrac(a.x,b.x); if(c) return c<0;
        return tie(a.type,a.u)<tie(b.type,b.u); // delete, insert, query
    });
    for(auto&e:ev) {
        x=e.x; rightSide=e.type!=0;
        if(e.type==0) st.erase(pos[e.u]);
        else if(e.type==1) pos[e.u]=st.insert(e.u).first;
        else {
            const auto&q=qs[e.u]; YQuery yy{q.p.y,q.p.w};
            auto it=st.lower_bound(yy);
            bool inside=(it!=st.end() && comp.compareY(*it,yy)==0); // boundary
            if(!inside && it!=st.begin()) inside=forward[*prev(it)];
            if(inside) relax(q.p,q.site);
        }
    }
}

// In one sweep direction, report the first red/blue crossing of every blue edge,
// then remove that blue edge. Since red-red and blue-blue interiors do not cross,
// only O(N) crossing events become effective.
static void sweepFirst(const vector<Segment>& s,int dir) {
    int n=s.size(); auto h=makeHeights(s,dir);
    vector<Fraction> lo(n),hi(n); vector<char> alive(n,false),marked(n,false);
    Fraction x{0,1}; bool rightSide=true;
    set<int,StatusCmp> st(StatusCmp{&h,&x,&rightSide});
    vector<set<int,StatusCmp>::iterator> pos(n);
    priority_queue<Event,vector<Event>,EventGreater> pq;
    for(int i=0;i<n;i++) {
        lo[i]=sweepX(s[i].p,dir); hi[i]=sweepX(s[i].q,dir);
        if(cmpFrac(lo[i],hi[i])>0) swap(lo[i],hi[i]);
        pq.push({lo[i],1,i,-1}); pq.push({hi[i],0,i,-1});
    }
    auto adjacent=[&](int a,int b)->bool {
        if(a<0||b<0||!alive[a]||!alive[b]) return false;
        auto ia=pos[a], ib=pos[b];
        if(ia==st.end()||ib==st.end()) return false;
        auto na=next(ia); if(na!=st.end() && *na==b) return true;
        auto nb=next(ib); return nb!=st.end() && *nb==a;
    };
    auto schedule=[&](int a,int b) {
        if(a<0||b<0) return;
        if((s[a].site<0)==(s[b].site<0)) return;
        QPoint q; if(!lineIntersect(s[a].line,s[b].line,q)) return;
        auto t=sweepX(q,dir);
        if(cmpFrac(t,x)<=0) return;
        if(cmpFrac(lo[a],t)>=0||cmpFrac(t,hi[a])>=0||cmpFrac(lo[b],t)>=0||cmpFrac(t,hi[b])>=0) return;
        pq.push({t,2,a,b});
    };
    vector<int> rem,add;
    while(!pq.empty()) {
        x=pq.top().x; rightSide=false; rem.clear(); add.clear();
        auto mark=[&](int id){ if(alive[id]&&!marked[id]) marked[id]=true,rem.push_back(id); };
        while(!pq.empty()&&cmpFrac(pq.top().x,x)==0) {
            Event e=pq.top(); pq.pop();
            if(e.type==0) mark(e.u);
            else if(e.type==1) add.push_back(e.u);
            else if(alive[e.u]&&alive[e.v]&&adjacent(e.u,e.v)) {
                int blue=s[e.u].site>=0?e.u:e.v;
                QPoint q; bool ok=lineIntersect(s[e.u].line,s[e.v].line,q); assert(ok);
                relax(q,s[blue].site); mark(blue);
            }
        }
        for(int id:rem) {
            auto it=pos[id]; auto nx=next(it);
            int a=it==st.begin()?-1:*prev(it), b=nx==st.end()?-1:*nx;
            st.erase(it); alive[id]=false; marked[id]=false;
            schedule(a,b);
        }
        rightSide=true;
        for(int id:add) {
            auto it=st.insert(id).first; pos[id]=it; alive[id]=true;
            int a=it==st.begin()?-1:*prev(it); auto nx=next(it); int b=nx==st.end()?-1:*nx;
            schedule(a,id); schedule(id,b);
        }
    }
}

// ---------------- Delaunay -> Voronoi ----------------
struct EdgeRec { int a,b,t,ou,ov; };

static bool clipFinite(const Line& line,QPoint a,QPoint b,QPoint& Lq,QPoint& Rq) {
    Fraction xa=sweepX(a),xb=sweepX(b); if(cmpFrac(xa,xb)>0) swap(a,b),swap(xa,xb);
    Fraction L{-XLIM,1},R{XLIM,1};
    if(cmpFrac(xb,L)<0||cmpFrac(R,xa)<0) return false;
    Lq=a; Rq=b;
    if(cmpFrac(xa,L)<0) { bool ok=lineIntersect(line,{K,1,-XLIM},Lq); assert(ok); }
    if(cmpFrac(R,xb)<0) { bool ok=lineIntersect(line,{K,1, XLIM},Rq); assert(ok); }
    return cmpFrac(sweepX(Lq),sweepX(Rq))<0;
}
static bool clipRay(const Line& line,const QPoint& start,ll dX,QPoint& Lq,QPoint& Rq) {
    Fraction xs=sweepX(start),L{-XLIM,1},R{XLIM,1};
    QPoint capL{},capR{}; bool a=lineIntersect(line,{K,1,-XLIM},capL),b=lineIntersect(line,{K,1,XLIM},capR); assert(a&&b);
    if(dX>0) {
        if(cmpFrac(xs,R)>0) return false;
        Lq=cmpFrac(xs,L)<0?capL:start; Rq=capR;
    } else {
        if(cmpFrac(xs,L)<0) return false;
        Lq=capL; Rq=cmpFrac(xs,R)>0?capR:start;
    }
    return cmpFrac(sweepX(Lq),sweepX(Rq))<0;
}

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int n; if(!(cin>>n)) return 0;
    sites.resize(n); for(auto& p:sites) cin>>p.x>>p.y;

    vector<Segment> red; red.reserve(n);
    for(int i=0;i<n;i++) {
        Point a=sites[i],b=sites[(i+1)%n];
        red.push_back({{a.x,a.y,1},{b.x,b.y,1},through(a,b),-1});
    }

    DelaunayBuilder db;
    vector<array<int,3>> tri=db.run(sites);
    int T=tri.size();
    vector<QPoint> cc(T);
    vector<Candidate> queries; queries.reserve(T);
    vector<EdgeRec> er; er.reserve(3*T);
    for(int t=0;t<T;t++) {
        int a=tri[t][0],b=tri[t][1],c=tri[t][2];
        if(cross(sites[a],sites[b],sites[c])<0) swap(b,c),swap(tri[t][1],tri[t][2]);
        cc[t]=circumcenter(sites[a],sites[b],sites[c]);
        queries.push_back({cc[t],a});
        int v[3]={a,b,c};
        for(int k=0;k<3;k++) {
            int u=v[k],w=v[(k+1)%3];
            er.push_back({min(u,w),max(u,w),t,u,w});
        }
    }
    checkVertices(red,queries);

    sort(er.begin(),er.end(),[](const EdgeRec&A,const EdgeRec&B){
        if(A.a!=B.a) return A.a<B.a;
        if(A.b!=B.b) return A.b<B.b;
        return A.t<B.t;
    });
    vector<Segment> all=red; all.reserve(n+er.size()/2+10);
    for(size_t i=0;i<er.size();) {
        size_t j=i+1; while(j<er.size()&&er[j].a==er[i].a&&er[j].b==er[i].b) j++;
        // In a triangulation an undirected edge has one or two incident triangles.
        assert(j-i==1 || j-i==2);
        int u=er[i].a,v=er[i].b; Line line=bisector(sites[u],sites[v]);
        QPoint Lq{},Rq{}; bool ok=false;
        if(j-i==2) {
            ok=clipFinite(line,cc[er[i].t],cc[er[i+1].t],Lq,Rq);
        } else {
            // er[i].ou -> er[i].ov is a CCW triangle edge; interior is left, outward normal is (dy,-dx).
            Point a=sites[er[i].ou],b=sites[er[i].ov];
            ll dx=b.x-a.x,dy=b.y-a.y;
            ll dX=K*dy-dx; assert(dX!=0);
            ok=clipRay(line,cc[er[i].t],dX,Lq,Rq);
        }
        if(ok) all.push_back({Lq,Rq,line,u});
        i=j;
    }

    // Free structures no longer needed before the two largest sweeps.
    vector<array<int,3>>().swap(tri);
    vector<QPoint>().swap(cc);
    vector<Candidate>().swap(queries);
    vector<EdgeRec>().swap(er);

    sweepFirst(all,1);
    sweepFirst(all,-1);
    cout<<fixed<<setprecision(12)<<sqrtl(ans2)<<'\n';
}
