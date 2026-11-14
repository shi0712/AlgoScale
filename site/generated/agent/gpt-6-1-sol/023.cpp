#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using ld = long double;
struct Point { int x, y; };
ll cross(Point a, Point b, Point c) {
    return (ll)(b.x-a.x)*(c.y-a.y)-(ll)(b.y-a.y)*(c.x-a.x);
}
ll norm(Point p) { return (ll)p.x*p.x+(ll)p.y*p.y; }

// Divide-and-conquer Delaunay triangulation, represented by quad-edges.
struct Delaunay {
    struct Edge { int org, next; };
    vector<Edge> e;
    vector<int> free_edges;
    const vector<Point>& p;
    Delaunay(const vector<Point>& pts): p(pts) { e.reserve(12*p.size()); }
    static int rot(int a) { return (a&~3)|((a+1)&3); }
    static int rev(int a) { return a^2; }
    int dest(int a) const { return e[a^2].org; }
    int oprev(int a) const { return rot(e[rot(a)].next); }
    int lnext(int a) const { return rot(e[rot(a)^2].next); }
    int make_edge(int a, int b) {
        int k;
        if (free_edges.empty()) { k=e.size(); e.resize(k+4); }
        else { k=free_edges.back(); free_edges.pop_back(); }
        e[k]={a,k}; e[k+1]={-1,k+3}; e[k+2]={b,k+2}; e[k+3]={-1,k+1};
        return k;
    }
    void splice(int a,int b) {
        int x=rot(e[a].next), y=rot(e[b].next);
        swap(e[x].next,e[y].next);
        swap(e[a].next,e[b].next);
    }
    void erase(int a) {
        splice(a,oprev(a)); splice(a^2,oprev(a^2));
        int k=a&~3; e[k].org=-1; free_edges.push_back(k);
    }
    int connect(int a,int b) {
        int c=make_edge(dest(a),e[b].org);
        splice(c,lnext(a)); splice(c^2,b);
        return c;
    }
    bool left(int a,int edge) const { return cross(p[e[edge].org],p[dest(edge)],p[a])>0; }
    bool right(int a,int edge) const { return cross(p[e[edge].org],p[dest(edge)],p[a])<0; }
    bool incircle(int a,int b,int c,int d) const {
        ll ax=p[a].x-p[d].x, ay=p[a].y-p[d].y;
        ll bx=p[b].x-p[d].x, by=p[b].y-p[d].y;
        ll cx=p[c].x-p[d].x, cy=p[c].y-p[d].y;
        return (__int128)(ax*ax+ay*ay)*(bx*cy-by*cx)
             + (__int128)(bx*bx+by*by)*(cx*ay-cy*ax)
             + (__int128)(cx*cx+cy*cy)*(ax*by-ay*bx)>0;
    }
    pair<int,int> build(int l,int r) {
        if (r-l==2) { int a=make_edge(l,l+1); return {a,a^2}; }
        if (r-l==3) {
            int a=make_edge(l,l+1), b=make_edge(l+1,l+2);
            splice(a^2,b);
            ll s=cross(p[l],p[l+1],p[l+2]);
            if (s==0) return {a,b^2};
            int c=connect(b,a);
            if (s>0) return {a,b^2};
            return {c^2,c};
        }
        int m=(l+r)/2;
        auto [ldo,ldi]=build(l,m);
        auto [rdi,rdo]=build(m,r);
        while (true) {
            if (left(e[rdi].org,ldi)) ldi=lnext(ldi);
            else if (right(e[ldi].org,rdi)) rdi=e[rdi^2].next;
            else break;
        }
        int base=connect(rdi^2,ldi);
        if (e[ldi].org==e[ldo].org) ldo=base^2;
        if (e[rdi].org==e[rdo].org) rdo=base;
        while (true) {
            int lc=e[base^2].next;
            if (right(dest(lc),base)) {
                while (incircle(dest(base),e[base].org,dest(lc),dest(e[lc].next))) {
                    int t=e[lc].next; erase(lc); lc=t;
                }
            }
            int rc=oprev(base);
            if (right(dest(rc),base)) {
                while (incircle(dest(base),e[base].org,dest(rc),dest(oprev(rc)))) {
                    int t=oprev(rc); erase(rc); rc=t;
                }
            }
            bool lv=right(dest(lc),base), rv=right(dest(rc),base);
            if (!lv && !rv) break;
            if (!lv || (rv && incircle(dest(lc),e[lc].org,e[rc].org,dest(rc))))
                base=connect(rc,base^2);
            else base=connect(base^2,lc^2);
        }
        return {ldo,rdo};
    }
};

// A half-plane is 2*a*x + 2*b*y <= c. Integer inputs give exact
// predicates: all intermediate integers fit in the long-double significand.
struct Line { ll a,b; ld c; };
ll det(const Line& u,const Line& v) { return u.a*v.b-u.b*v.a; }
int half(const Line& l) { return l.b<0 || (l.b==0 && l.a<0); }
bool angle_less(const Line& u,const Line& v) {
    int hu=half(u),hv=half(v);
    if (hu!=hv) return hu<hv;
    return det(u,v)>0;
}
bool tighter(const Line& u,const Line& v) {
    ll su=abs(u.a)+abs(u.b),sv=abs(v.a)+abs(v.b);
    return u.c*sv<v.c*su;
}
bool outside(const Line& l,const Line& u,const Line& v) {
    ll d=det(u,v);
    ld xn=u.c*v.b-v.c*u.b, yn=u.a*v.c-v.a*u.c;
    ld value=l.a*xn+l.b*yn-l.c*d;
    return d>0 ? value>0 : value<0;
}
struct RealPoint { ld x,y; };
RealPoint intersect(const Line& u,const Line& v) {
    ld d=2*(ld)det(u,v);
    return {(u.c*v.b-v.c*u.b)/d,(u.a*v.c-v.a*u.c)/d};
}

struct Integrator {
    vector<Line> dq;
    vector<RealPoint> vertices;
    ld center_x,center_y;
    Point reference;
    Integrator(ld x,ld y,Point ref):center_x(x),center_y(y),reference(ref) {}
    ld integrate(const vector<Line>& lines,Point p) {
        dq.resize(lines.size());
        int head=0,tail=0;
        for (size_t i=0;i<lines.size();) {
            Line l=lines[i++];
            while (i<lines.size() && half(l)==half(lines[i]) && det(l,lines[i])==0) {
                if (tighter(lines[i],l)) l=lines[i];
                ++i;
            }
            while (tail-head>=2 && outside(l,dq[tail-2],dq[tail-1])) --tail;
            while (tail-head>=2 && outside(l,dq[head],dq[head+1])) ++head;
            if (tail>head && det(l,dq[tail-1])==0) return 0;
            dq[tail++]=l;
        }
        while (tail-head>=3 && outside(dq[head],dq[tail-2],dq[tail-1])) --tail;
        while (tail-head>=3 && outside(dq[tail-1],dq[head],dq[head+1])) ++head;
        if (tail-head<3 || det(dq[tail-1],dq[head])==0) return 0;
        vertices.clear();
        for (int i=head;i<tail;++i) {
            RealPoint q=intersect(dq[i],dq[i+1==tail?head:i+1]);
            q.x-=center_x; q.y-=center_y;
            vertices.push_back(q);
        }
        ld area2=0,mx6=0,my6=0;
        for (size_t i=0,j=vertices.size()-1;i<vertices.size();j=i++) {
            auto a=vertices[j],b=vertices[i];
            ld cr=a.x*b.y-a.y*b.x;
            area2+=cr; mx6+=(a.x+b.x)*cr; my6+=(a.y+b.y)*cr;
        }
        // Subtract the same reference distance from both envelopes to reduce
        // cancellation. The quadratic terms cancel, leaving an affine function.
        ld px=p.x-reference.x,py=p.y-reference.y;
        ld constant=px*(p.x+reference.x-2*center_x)
                   +py*(p.y+reference.y-2*center_y);
        return constant*area2/2-(px*mx6+py*my6)/3;
    }
};

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    ld xl,yl,xr,yr;
    int n;
    if (!(cin>>xl>>yl>>xr>>yr>>n)) return 0;
    vector<Point> p(n);
    for (auto& a:p) cin>>a.x>>a.y;
    sort(p.begin(),p.end(),[](Point a,Point b){ return a.x!=b.x?a.x<b.x:a.y<b.y; });
    vector<int> hull;
    for (int i=0;i<n;++i) {
        while (hull.size()>=2 && cross(p[hull[hull.size()-2]],p[hull.back()],p[i])<=0) hull.pop_back();
        hull.push_back(i);
    }
    int lower=hull.size();
    for (int i=n-2;i>=0;--i) {
        while ((int)hull.size()>lower && cross(p[hull[hull.size()-2]],p[hull.back()],p[i])<=0) hull.pop_back();
        hull.push_back(i);
    }
    hull.pop_back();
    array<Line,4> box={Line{1,0,2LL*xr},Line{0,1,2LL*yr},Line{-1,0,-2LL*xl},Line{0,-1,-2LL*yl}};
    Integrator integrator((xl+xr)/2,(yl+yr)/2,p[0]);
    vector<Line> lines,raw;
    ld result=0;
    // Only hull vertices can be farthest. Their normals already have cyclic angular order.
    // Total time is O(n log n + h^2); the bounded integer grid bounds h to a few thousand.
    int h=hull.size();
    for (int i=0;i<h;++i) {
        Point a=p[hull[i]];
        raw.clear();
        int first=0;
        for (int k=1;k<h;++k) {
            Point b=p[hull[(i+k)%h]];
            raw.push_back({a.x-b.x,a.y-b.y,(ld)(norm(a)-norm(b))});
            if (angle_less(raw.back(),raw[first])) first=raw.size()-1;
        }
        rotate(raw.begin(),raw.begin()+first,raw.end());
        lines.resize(raw.size()+4);
        merge(raw.begin(),raw.end(),box.begin(),box.end(),lines.begin(),angle_less);
        result+=integrator.integrate(lines,a);
    }
    Delaunay triangulation(p);
    triangulation.build(0,n);
    vector<int> offset(n+1,0);
    for (int k=0;k<(int)triangulation.e.size();k+=4) if (triangulation.e[k].org>=0) {
        ++offset[triangulation.e[k].org+1]; ++offset[triangulation.e[k+2].org+1];
    }
    for (int i=1;i<=n;++i) offset[i]+=offset[i-1];
    vector<int> adj(offset[n]),at=offset;
    for (int k=0;k<(int)triangulation.e.size();k+=4) if (triangulation.e[k].org>=0) {
        int a=triangulation.e[k].org,b=triangulation.e[k+2].org;
        adj[at[a]++]=b; adj[at[b]++]=a;
    }
    for (int i=0;i<n;++i) {
        lines.assign(box.begin(),box.end());
        Point a=p[i];
        for (int k=offset[i];k<offset[i+1];++k) {
            Point b=p[adj[k]];
            lines.push_back({b.x-a.x,b.y-a.y,(ld)(norm(b)-norm(a))});
        }
        sort(lines.begin(),lines.end(),angle_less);
        result-=integrator.integrate(lines,a);
    }
    ld answer=acosl(-1.L)*result/((ld)(xr-xl)*(yr-yl));
    cout<<fixed<<setprecision(16)<<answer<<'\n';
}
