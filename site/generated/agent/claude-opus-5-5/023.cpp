#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef long double ld;
typedef __int128 lll;
struct P{ ll x,y;
  P operator-(const P&o)const{return {x-o.x,y-o.y};}
  bool operator<(const P&o)const{return tie(x,y)<tie(o.x,o.y);}
  bool operator==(const P&o)const{return x==o.x&&y==o.y;}
  ll cross(const P&o)const{return x*o.y-y*o.x;}
  ll cross(const P&a,const P&b)const{return (a-*this).cross(b-*this);}
  ll dist2()const{return x*x+y*y;}
};
P arb{LLONG_MAX,LLONG_MAX};
typedef struct Quad* Q;
struct Quad{ Q rot,o; P p=arb; bool mark;
  P& F(){return r()->p;}
  Q& r(){return rot->rot;}
  Q prev(){return rot->o->rot;}
  Q next(){return r()->prev();}
} *H;
bool circ(P p,P a,P b,P c){
  lll p2=p.dist2(),A=a.dist2()-p2,B=b.dist2()-p2,C=c.dist2()-p2;
  return (lll)p.cross(a,b)*C+(lll)p.cross(b,c)*A+(lll)p.cross(c,a)*B>0;
}
Q makeEdge(P orig,P dest){
  Q r=H?H:new Quad{new Quad{new Quad{new Quad{0}}}};
  H=r->o; r->r()->r()=r;
  for(int i=0;i<4;i++) r=r->rot, r->p=arb, r->o= (i&1)?r:r->r();
  r->p=orig; r->F()=dest;
  return r;
}
void splice(Q a,Q b){ swap(a->o->rot->o,b->o->rot->o); swap(a->o,b->o);}
Q connect(Q a,Q b){
  Q q=makeEdge(a->F(),b->p);
  splice(q,a->next());
  splice(q->r(),b);
  return q;
}
pair<Q,Q> rec(const vector<P>&s){
  if(s.size()<=3){
    Q a=makeEdge(s[0],s[1]), b=makeEdge(s[1],s.back());
    if(s.size()==2) return {a,a->r()};
    splice(a->r(),b);
    auto side=s[0].cross(s[1],s[2]);
    Q c=side?connect(b,a):0;
    return {side<0?c->r():a, side<0?c:b->r()};
  }
#define HH(e) e->F(), e->p
#define valid(e) (e->F().cross(HH(base))>0)
  Q A,B,ra,rb;
  int half=s.size()/2;
  tie(ra,A)=rec({s.begin(),s.end()-half});
  tie(B,rb)=rec({s.end()-half,s.end()});
  while((B->p.cross(HH(A))<0 && (A=A->next())) ||
        (A->p.cross(HH(B))>0 && (B=B->r()->o)));
  Q base=connect(B->r(),A);
  if(A->p==ra->p) ra=base->r();
  if(B->p==rb->p) rb=base;
#define DEL(e,init,dir) Q e=init->dir; if(valid(e)) \
    while(circ(e->dir->F(),HH(base),e->F())){ \
      Q t=e->dir; splice(e,e->prev()); splice(e->r(),e->r()->prev()); \
      e->o=H; H=e; e=t; }
  for(;;){
    DEL(LC,base->r(),o); DEL(RC,base,prev());
    if(!valid(LC)&&!valid(RC)) break;
    if(!valid(LC)||(valid(RC)&&circ(HH(RC),HH(LC))))
      base=connect(RC,base->r());
    else base=connect(base->r(),LC->r());
  }
  return {ra,rb};
}

// Half-plane: point a (doubled coords as ld), direction d (integer), keep left.
struct HP{ ld ax,ay; ll dx,dy; ld sc=0;
  int half()const{ return (dy<0||(dy==0&&dx<0)); }
};
struct Pt{ld x,y;};
static inline bool angLess(const HP&a,const HP&b){
  int ha=a.half(),hb=b.half(); if(ha!=hb) return ha<hb;
  return (lll)a.dx*b.dy-(lll)a.dy*b.dx>0;
}
static inline Pt inter(const HP&s,const HP&t){
  ld den=(ld)s.dx*t.dy-(ld)s.dy*t.dx;
  ld al=((t.ax-s.ax)*t.dy-(t.ay-s.ay)*t.dx)/den;
  return {s.ax+al*s.dx, s.ay+al*s.dy};
}
static inline bool outp(const HP&h,const Pt&r){
  ld c=(ld)h.dx*(r.y-h.ay)-(ld)h.dy*(r.x-h.ax);
  return c< -h.sc;
}
vector<Pt> hpi(vector<HP>&Hs,bool sorted=false){
  for(auto&h:Hs) h.sc=1e-12L*sqrtl((ld)h.dx*h.dx+(ld)h.dy*h.dy);
  if(!sorted) sort(Hs.begin(),Hs.end(),angLess);
  vector<HP> dq(Hs.size()+2); int fr=0,bk=0; // [fr,bk)
  for(auto&h:Hs){
    while(bk-fr>1&&outp(h,inter(dq[bk-1],dq[bk-2]))) bk--;
    while(bk-fr>1&&outp(h,inter(dq[fr],dq[fr+1]))) fr++;
    if(bk-fr>0){
      HP&l=dq[bk-1];
      if((lll)h.dx*l.dy-(lll)h.dy*l.dx==0){
        if((lll)h.dx*l.dx+(lll)h.dy*l.dy<0) return {};
        if(outp(h,Pt{l.ax,l.ay})){ bk--; }
        else continue;
      }
    }
    dq[bk++]=h;
  }
  while(bk-fr>2&&outp(dq[fr],inter(dq[bk-1],dq[bk-2]))) bk--;
  while(bk-fr>2&&outp(dq[bk-1],inter(dq[fr],dq[fr+1]))) fr++;
  int len=bk-fr; if(len<3) return {};
  vector<Pt> ret(len);
  for(int i=0;i<len;i++) ret[i]=inter(dq[fr+i],dq[fr+(i+1)%len]);
  return ret;
}
// integral of |x-p|^2 over polygon (doubled coords), returns in real units
ld integ(const vector<Pt>&poly,ll px2,ll py2){
  int m=poly.size(); if(m<3) return 0;
  ld s=0;
  for(int i=0;i<m;i++){
    ld x0=(poly[i].x-px2)/2,y0=(poly[i].y-py2)/2;
    ld x1=(poly[(i+1)%m].x-px2)/2,y1=(poly[(i+1)%m].y-py2)/2;
    ld cr=x0*y1-x1*y0;
    s+=cr*(x0*x0+x0*x1+x1*x1+y0*y0+y0*y1+y1*y1);
  }
  return s/12;
}
ld XL,YL,XR,YR;
void addRect(vector<HP>&v){
  // doubled coords
  v.push_back({(ld)2*XL,(ld)2*YL,1,0});
  v.push_back({(ld)2*XR,(ld)2*YL,0,1});
  v.push_back({(ld)2*XR,(ld)2*YR,-1,0});
  v.push_back({(ld)2*XL,(ld)2*YR,0,-1});
}
// half-plane: points closer to p than q  (keep side toward p)
HP closer(const P&p,const P&q){
  ll ax=p.x-q.x, ay=p.y-q.y; // normal toward p
  return HP{(ld)(p.x+q.x),(ld)(p.y+q.y),ay,-ax};
}
int main(){
  {double a,b,c,d; if(scanf("%lf %lf %lf %lf",&a,&b,&c,&d)!=4) return 0; XL=a;YL=b;XR=c;YR=d;}
  int n; scanf("%d",&n);
  vector<P> pts(n);
  for(auto&p:pts) scanf("%lld %lld",&p.x,&p.y);
  sort(pts.begin(),pts.end());
  // Delaunay
  vector<vector<int>> nb(n);
  {
    Q e=rec(pts).first;
    vector<Q> st={e}; e->mark=1;
    auto idx=[&](const P&p){return int(lower_bound(pts.begin(),pts.end(),p)-pts.begin());};
    while(!st.empty()){
      Q c=st.back(); st.pop_back();
      nb[idx(c->p)].push_back(idx(c->F()));
      Q a=c->o, b=c->r();
      if(!a->mark){a->mark=1;st.push_back(a);}
      if(!b->mark){b->mark=1;st.push_back(b);}
    }
  }
  ld Smin=0;
  vector<HP> hs;
  for(int i=0;i<n;i++){
    hs.clear(); addRect(hs);
    for(int j:nb[i]) hs.push_back(closer(pts[i],pts[j]));
    auto poly=hpi(hs);
    Smin+=integ(poly,2*pts[i].x,2*pts[i].y);
  }
  // convex hull strict
  vector<P> hull;
  {
    vector<P> h(2*n); int k=0;
    for(int i=0;i<n;i++){ while(k>=2&&h[k-2].cross(h[k-1],pts[i])<=0)k--; h[k++]=pts[i]; }
    for(int i=n-2,t=k+1;i>=0;i--){ while(k>=t&&h[k-2].cross(h[k-1],pts[i])<=0)k--; h[k++]=pts[i]; }
    h.resize(k-1); hull=h;
  }
  ld Smax=0; int h=hull.size();
  vector<vector<int>> fn(h);
  auto addE=[&](int u,int v){fn[u].push_back(v);fn[v].push_back(u);};
  for(int i=0;i+1<h;i++) addE(i,i+1);
  if(h>=3) addE(0,h-1);
  {
    vector<pair<int,int>> stk; if(h>=3) stk.push_back({0,h-1});
    while(!stk.empty()){
      auto [i,j]=stk.back(); stk.pop_back();
      if(j-i<2) continue;
      P A=hull[i],B=hull[j];
      ll Mx=A.x+B.x,My=A.y+B.y;
      ll ux=-(B.y-A.y),uy=B.x-A.x;
      {P q=hull[i+1]; if(ux*(2*q.x-Mx)+uy*(2*q.y-My)<0){ux=-ux;uy=-uy;}}
      ll ma=(Mx-2*A.x)*(Mx-2*A.x)+(My-2*A.y)*(My-2*A.y);
      int best=-1; ll bN=0,bD=1;
      for(int k=i+1;k<j;k++){
        ll qx=2*hull[k].x-Mx,qy=2*hull[k].y-My;
        ll N=qx*qx+qy*qy-ma, D=ux*qx+uy*qy;
        if(best<0||(lll)N*bD>(lll)bN*D){best=k;bN=N;bD=D;}
      }
      if(best-i>=2) addE(i,best);
      if(j-best>=2) addE(best,j);
      stk.push_back({i,best}); stk.push_back({best,j});
    }
  }
  for(int i=0;i<h;i++){
    hs.clear(); addRect(hs);
    for(int j:fn[i]) hs.push_back(closer(hull[j],hull[i]));
    auto poly=hpi(hs);
    Smax+=integ(poly,2*hull[i].x,2*hull[i].y);
  }
  ld area=(XR-XL)*(YR-YL);
  ld ans=acosl(-1)*(Smax-Smin)/area;
  printf("%.12Lf\n",ans);
}
