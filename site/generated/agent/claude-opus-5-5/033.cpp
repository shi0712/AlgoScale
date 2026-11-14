#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef __int128_t lll;
struct P {
    ll x, y;
    P(ll x_=0, ll y_=0):x(x_),y(y_){}
    bool operator<(const P&o)const{return tie(x,y)<tie(o.x,o.y);}
    bool operator==(const P&o)const{return x==o.x&&y==o.y;}
    P operator-(const P&o)const{return P(x-o.x,y-o.y);}
    ll cross(const P&o)const{return x*o.y-y*o.x;}
    ll cross(const P&a,const P&b)const{return (a-*this).cross(b-*this);}
    ll dist2()const{return x*x+y*y;}
};
typedef struct Quad* Q;
P arb(LLONG_MAX,LLONG_MAX);
struct Quad {
    Q rot, o; P p = arb; bool mark;
    P& F() { return r()->p; }
    Q& r() { return rot->rot; }
    Q prev() { return rot->o->rot; }
    Q next() { return r()->prev(); }
} *H;
bool circ(P p, P a, P b, P c) {
    lll p2 = p.dist2(), A = a.dist2()-p2, B = b.dist2()-p2, C = c.dist2()-p2;
    return p.cross(a,b)*C + p.cross(b,c)*A + p.cross(c,a)*B > 0;
}
Q makeEdge(P orig, P dest) {
    Q r = H ? H : new Quad{new Quad{new Quad{new Quad{0}}}};
    H = r->o; r->r()->r() = r;
    for(int i=0;i<4;i++) r = r->rot, r->p = arb, r->o = i & 1 ? r : r->r();
    r->p = orig; r->F() = dest;
    return r;
}
void splice(Q a, Q b) { swap(a->o->rot->o, b->o->rot->o); swap(a->o, b->o); }
Q connect(Q a, Q b) {
    Q q = makeEdge(a->F(), b->p);
    splice(q, a->next());
    splice(q->r(), b);
    return q;
}
pair<Q,Q> rec(const vector<P>& s) {
    if (s.size() <= 3) {
        Q a = makeEdge(s[0], s[1]), b = makeEdge(s[1], s.back());
        if (s.size() == 2) return { a, a->r() };
        splice(a->r(), b);
        auto side = s[0].cross(s[1], s[2]);
        Q c = side ? connect(b, a) : 0;
        return {side < 0 ? c->r() : a, side < 0 ? c : b->r() };
    }
#define HH(e) e->F(), e->p
#define valid(e) (e->F().cross(HH(base)) > 0)
    Q A, B, ra, rb;
    int half = s.size() / 2;
    tie(ra, A) = rec({s.begin(), s.begin() + half});
    tie(B, rb) = rec({s.begin() + half, s.end()});
    while ((B->p.cross(HH(A)) < 0 && (A = A->next())) ||
           (A->p.cross(HH(B)) > 0 && (B = B->r()->o)));
    Q base = connect(B->r(), A);
    if (A->p == ra->p) ra = base->r();
    if (B->p == rb->p) rb = base;
#define DEL(e, init, dir) Q e = init->dir; if (valid(e)) \
        while (circ(e->dir->F(), HH(base), e->F())) { \
            Q t = e->dir; \
            splice(e, e->prev()); \
            splice(e->r(), e->r()->prev()); \
            e->o = H; H = e; e = t; \
        }
    for (;;) {
        DEL(LC, base->r(), o);  DEL(RC, base, prev());
        if (!valid(LC) && !valid(RC)) break;
        if (!valid(LC) || (valid(RC) && circ(HH(RC), HH(LC))))
            base = connect(RC, base->r());
        else
            base = connect(base->r(), LC->r());
    }
    return { ra, rb };
}
vector<P> triangulate(vector<P> pts) {
    sort(pts.begin(), pts.end());
    if (pts.size() < 2) return {};
    Q e = rec(pts).first;
    vector<Q> q = {e};
    size_t qi = 0;
    while (e->o->F().cross(e->F(), e->p) < 0) e = e->o;
#define ADD { Q c = e; do { c->mark = 1; pts.push_back(c->p); \
    q.push_back(c->r()); c = c->next(); } while (c != e); }
    ADD; pts.clear();
    while (qi < q.size()) if (!(e = q[qi++])->mark) ADD;
    return pts;
}

int n;
vector<P> pt;
struct Edge { P L, R; bool right; };
vector<Edge> E;
static ll orient(const P&p,const P&q,const P&r){return (q-p).cross(r-p);}
struct QP { double x, y; };
static double yat(const Edge&e,double x){
    return e.L.y + (double)(e.R.y-e.L.y)*(x-e.L.x)/(double)(e.R.x-e.L.x);
}
struct Cmp {
    using is_transparent = void;
    bool operator()(int i,int j)const{
        if(i==j) return false;
        const Edge&a=E[i];const Edge&b=E[j];
        if(a.L.x>=b.L.x){
            ll s=orient(b.L,b.R,a.L); if(s==0) s=orient(b.L,b.R,a.R);
            return s<0;
        } else {
            ll s=orient(a.L,a.R,b.L); if(s==0) s=orient(a.L,a.R,b.R);
            return s>0;
        }
    }
    bool operator()(int i,const QP&q)const{ return yat(E[i],q.x)<q.y; }
    bool operator()(const QP&q,int i)const{ return q.y<yat(E[i],q.x); }
};

int main(){
    scanf("%d",&n);
    pt.resize(n);
    for(auto&p:pt) scanf("%lld %lld",&p.x,&p.y);
    vector<P> tr = triangulate(pt);
    vector<pair<P,int>> srt(n);
    for(int i=0;i<n;i++) srt[i]={pt[i],i};
    sort(srt.begin(),srt.end());
    auto idx=[&](const P&p){
        auto it=lower_bound(srt.begin(),srt.end(),make_pair(p,INT_MIN));
        return it->second;
    };
    int T=tr.size()/3;
    vector<vector<int>> adj(n);
    vector<array<int,3>> tris(T);
    for(int t=0;t<T;t++){
        for(int k=0;k<3;k++) tris[t][k]=idx(tr[3*t+k]);
        for(int k=0;k<3;k++){
            int a=tris[t][k], b=tris[t][(k+1)%3];
            adj[a].push_back(b); adj[b].push_back(a);
        }
    }
    for(auto&v:adj){ sort(v.begin(),v.end()); v.erase(unique(v.begin(),v.end()),v.end()); }
    double best=0, best2=0;
    // walk along polygon edges (CSR adjacency)
    vector<int> st0(n+1,0); vector<ll> AX, AY; vector<int> AI;
    for(int i=0;i<n;i++) st0[i+1]=st0[i]+adj[i].size();
    AX.resize(st0[n]); AY.resize(st0[n]); AI.resize(st0[n]);
    for(int i=0;i<n;i++) for(size_t k=0;k<adj[i].size();k++){int d=adj[i][k]; AX[st0[i]+k]=pt[d].x; AY[st0[i]+k]=pt[d].y; AI[st0[i]+k]=d;}
    for(int i=0;i<n;i++){
        int u=i, v=(i+1)%n;
        ll ux=pt[u].x, uy=pt[u].y, dvx=pt[v].x-ux, dvy=pt[v].y-uy;
        int c=u;
        while(c!=v){
            ll cx=pt[c].x, cy=pt[c].y;
            ll bn=0,bd=0; int bk=-1;
            for(int k=st0[c];k<st0[c+1];k++){
                ll dcx=AX[k]-cx, dcy=AY[k]-cy;
                ll D=dvx*dcx+dvy*dcy;
                if(D<=0) continue;
                ll N=dcx*(AX[k]+cx-2*ux)+dcy*(AY[k]+cy-2*uy);
                if(bk<0 || N*bd < bn*D){ bn=N; bd=D; bk=k; }
            }
            if(bk<0) break;
            double lam=(double)bn/(double)(2*bd);
            double px=ux+lam*dvx, py=uy+lam*dvy;
            double f=(px-cx)*(px-cx)+(py-cy)*(py-cy);
            if(f>best2) best2=f;
            c=AI[bk];
        }
    }
    best=sqrt(best2);
    // circumcenters inside polygon
    ll mnx=LLONG_MAX,mxx=LLONG_MIN,mny=LLONG_MAX,mxy=LLONG_MIN;
    for(auto&p:pt){mnx=min(mnx,p.x);mxx=max(mxx,p.x);mny=min(mny,p.y);mxy=max(mxy,p.y);}
    vector<QP> qs; vector<double> qr;
    for(int t=0;t<T;t++){
        P a=pt[tris[t][0]], b=pt[tris[t][1]], c=pt[tris[t][2]];
        double bx=b.x-a.x, by=b.y-a.y, cx=c.x-a.x, cy=c.y-a.y;
        double d=2*(bx*cy-by*cx);
        if(d==0) continue;
        double b2=bx*bx+by*by, c2=cx*cx+cy*cy;
        double ux=(cy*b2-by*c2)/d, uy=(bx*c2-cx*b2)/d;
        double r=hypot(ux,uy);
        if(r<=best) continue;
        double X=a.x+ux, Y=a.y+uy;
        if(X<mnx||X>mxx||Y<mny||Y>mxy) continue;
        qs.push_back({X,Y}); qr.push_back(r);
    }
    for(int i=0;i<n;i++){
        P a=pt[i], b=pt[(i+1)%n];
        if(a.x==b.x) continue;
        if(a.x<b.x) E.push_back({a,b,true}); else E.push_back({b,a,false});
    }
    // events: (x, type, id) type 0 remove, 1 insert, 2 query
    struct Ev{ double x; int type; int id; };
    vector<Ev> ev;
    for(int i=0;i<(int)E.size();i++){ ev.push_back({(double)E[i].L.x,1,i}); ev.push_back({(double)E[i].R.x,0,i}); }
    for(int i=0;i<(int)qs.size();i++) ev.push_back({qs[i].x,2,i});
    sort(ev.begin(),ev.end(),[](const Ev&a,const Ev&b){ if(a.x!=b.x) return a.x<b.x; return a.type<b.type;});
    set<int,Cmp> st;
    vector<set<int,Cmp>::iterator> its(E.size());
    for(auto&e:ev){
        if(e.type==0) st.erase(its[e.id]);
        else if(e.type==1) its[e.id]=st.insert(e.id).first;
        else {
            if(qr[e.id]<=best) continue;
            auto it=st.lower_bound(qs[e.id]);
            if(it==st.begin()) continue;
            --it;
            if(E[*it].right) best=max(best,qr[e.id]);
        }
    }
    printf("%.9f\n",best);
}
