#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
using namespace std; using namespace __gnu_pbds;
using ld=long double;
struct P { double x,y; };
vector<P> pts;
struct Seg { int a,b; ld slope; }; vector<Seg> segs;
ld sweepx, queryy;
struct Cmp {
 bool operator()(int a,int b) const {
  if(a==b)return false;
  auto val=[](int i)->pair<ld,ld>{if(i==(int)segs.size())return {queryy, -numeric_limits<ld>::infinity()}; auto s=segs[i]; return {(ld)pts[s.a].y+((ld)pts[s.b].y-pts[s.a].y)*(sweepx-pts[s.a].x)/((ld)pts[s.b].x-pts[s.a].x),s.slope};};
  auto x=val(a),y=val(b); if(x.first!=y.first)return x.first<y.first; if(x.second!=y.second)return x.second<y.second; return a<b;
 }
};
using Tree=tree<int,null_type,Cmp,rb_tree_tag,tree_order_statistics_node_update>;
struct KDNode {int p,l=-1,r=-1; double xmin,xmax,ymin,ymax;}; vector<KDNode> kd; vector<int> ids;
int build(int l,int r,int dep){if(l>=r)return -1; int m=(l+r)/2,axis=dep%2; nth_element(ids.begin()+l,ids.begin()+m,ids.begin()+r,[&](int a,int b){return axis?pts[a].y<pts[b].y:pts[a].x<pts[b].x;}); int z=kd.size(); kd.push_back({ids[m]}); int lc=build(l,m,dep+1),rc=build(m+1,r,dep+1); kd[z].l=lc;kd[z].r=rc; auto p=pts[kd[z].p]; kd[z].xmin=kd[z].xmax=p.x;kd[z].ymin=kd[z].ymax=p.y; for(int c:{lc,rc})if(c>=0){kd[z].xmin=min(kd[z].xmin,kd[c].xmin);kd[z].xmax=max(kd[z].xmax,kd[c].xmax);kd[z].ymin=min(kd[z].ymin,kd[c].ymin);kd[z].ymax=max(kd[z].ymax,kd[c].ymax);}return z;}
double boxdist(int i,P p){if(i<0)return 1e100; auto &q=kd[i];double dx=max({q.xmin-p.x,0.,p.x-q.xmax}),dy=max({q.ymin-p.y,0.,p.y-q.ymax});return dx*dx+dy*dy;}
void nearest(int z,P p,double &best,int &who){if(z<0)return; auto q=pts[kd[z].p];double dx=p.x-q.x,dy=p.y-q.y,d=dx*dx+dy*dy;if(d<best){best=d;who=kd[z].p;}int a=kd[z].l,b=kd[z].r;double da=boxdist(a,p),db=boxdist(b,p);if(da>db){swap(a,b);swap(da,db);}if(da<best)nearest(a,p,best,who);if(db<best)nearest(b,p,best,who);}
struct Edge {int o=-1; Edge *rot=nullptr,*next=nullptr; bool alive=false; Edge* rev(){return rot->rot;} int dest(){return rev()->o;} Edge* lnext(){return rot->rev()->next->rot;} Edge* oprev(){return rot->next->rot;} }; 
vector<array<Edge,4>*> blocks; vector<array<Edge,4>*> freeblocks;
Edge* makeedge(int a,int b){array<Edge,4>*q;if(freeblocks.empty()){q=new array<Edge,4>();blocks.push_back(q);}else{q=freeblocks.back();freeblocks.pop_back();}for(int i=0;i<4;i++){(*q)[i]=Edge();(*q)[i].rot=&(*q)[(i+1)%4];(*q)[i].alive=true;}auto e=&(*q)[0];e->o=a;e->rev()->o=b;e->next=e;e->rev()->next=e->rev();e->rot->next=e->rot->rev();e->rot->rev()->next=e->rot;return e;}
void splice(Edge*a,Edge*b){swap(a->next->rot->next,b->next->rot->next);swap(a->next,b->next);}
Edge* connect(Edge*a,Edge*b){Edge*e=makeedge(a->dest(),b->o);splice(e,a->lnext());splice(e->rev(),b);return e;}
void eraseedge(Edge*e){splice(e,e->oprev());splice(e->rev(),e->rev()->oprev());Edge* base=e;while(base->o==-1)base=base->rot;if(base!=e && base!=e->rev())abort();if(base>base->rev())base=base->rev();auto q=reinterpret_cast<array<Edge,4>*>(base);for(auto &x:*q)x.alive=false;freeblocks.push_back(q);}
long long cross(int a,int b,int c){return (long long)(pts[b].x-pts[a].x)*(long long)(pts[c].y-pts[a].y)-(long long)(pts[b].y-pts[a].y)*(long long)(pts[c].x-pts[a].x);}
bool leftof(int p,Edge*e){return cross(e->o,e->dest(),p)>0;}bool rightof(int p,Edge*e){return cross(e->o,e->dest(),p)<0;}
bool incircle(int a,int b,int c,int d){ __int128 ax=(long long)(pts[a].x-pts[d].x),ay=(long long)(pts[a].y-pts[d].y),bx=(long long)(pts[b].x-pts[d].x),by=(long long)(pts[b].y-pts[d].y),cx=(long long)(pts[c].x-pts[d].x),cy=(long long)(pts[c].y-pts[d].y);return (ax*ax+ay*ay)*(bx*cy-by*cx)-(bx*bx+by*by)*(ax*cy-ay*cx)+(cx*cx+cy*cy)*(ax*by-ay*bx)>0;}
vector<int> ord;
pair<Edge*,Edge*> delaunay(int l,int r){int sz=r-l;if(sz==2){auto a=makeedge(ord[l],ord[l+1]);return {a,a->rev()};}if(sz==3){auto a=makeedge(ord[l],ord[l+1]),b=makeedge(ord[l+1],ord[l+2]);splice(a->rev(),b);auto cr=cross(ord[l],ord[l+1],ord[l+2]);if(cr>0){connect(b,a);return {a,b->rev()};}if(cr<0){auto c=connect(b,a);return {c->rev(),c};}return {a,b->rev()};}int m=(l+r)/2;auto [ldo,ldi]=delaunay(l,m);auto [rdi,rdo]=delaunay(m,r);while(true){if(leftof(rdi->o,ldi))ldi=ldi->lnext();else if(rightof(ldi->o,rdi))rdi=rdi->rev()->next;else break;}Edge* base=connect(rdi->rev(),ldi);if(ldi->o==ldo->o)ldo=base->rev();if(rdi->o==rdo->o)rdo=base;while(true){Edge*lc=base->rev()->next;if(rightof(lc->dest(),base)){while(incircle(base->dest(),base->o,lc->dest(),lc->next->dest())){Edge*t=lc->next;eraseedge(lc);lc=t;}}Edge*rc=base->oprev();if(rightof(rc->dest(),base)){while(incircle(base->dest(),base->o,rc->dest(),rc->oprev()->dest())){Edge*t=rc->oprev();eraseedge(rc);rc=t;}}bool lv=rightof(lc->dest(),base),rv=rightof(rc->dest(),base);if(!lv&&!rv)break;if(!lv||(rv&&incircle(lc->dest(),lc->o,rc->o,rc->dest())))base=connect(rc,base->rev());else base=connect(base->rev(),lc->rev());}return {ldo,rdo};}
struct Event {double x; int type,id;};
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);int n;if(!(cin>>n))return 0;pts.resize(n);for(auto &p:pts)cin>>p.x>>p.y;ord.resize(n);iota(ord.begin(),ord.end(),0);sort(ord.begin(),ord.end(),[](int a,int b){return pts[a].x!=pts[b].x?pts[a].x<pts[b].x:pts[a].y<pts[b].y;});delaunay(0,n);
 vector<P> verts;for(auto block:blocks)if((*block)[0].alive)for(int k:{0,2}){Edge*e=&(*block)[k];Edge*f=e->lnext();Edge*g=f->lnext();if(g->lnext()!=e)continue;int a=e->o,b=f->o,c=g->o;if(cross(a,b,c)<=0)continue;ld bx=pts[b].x-pts[a].x,by=pts[b].y-pts[a].y,cx=pts[c].x-pts[a].x,cy=pts[c].y-pts[a].y,den=2*(bx*cy-by*cx),bb=bx*bx+by*by,cc=cx*cx+cy*cy;verts.push_back({(double)(pts[a].x+(bb*cy-cc*by)/den),(double)(pts[a].y+(bx*cc-cx*bb)/den)});}
 vector<Event> ev;ev.reserve(2*n+verts.size());for(int i=0;i<n;i++){int j=(i+1)%n;if(pts[i].x==pts[j].x)continue;int a=i,b=j;if(pts[a].x>pts[b].x)swap(a,b);int id=segs.size();segs.push_back({a,b,((ld)pts[b].y-pts[a].y)/((ld)pts[b].x-pts[a].x)});ev.push_back({pts[b].x,0,id});ev.push_back({pts[a].x,1,id});}for(int i=0;i<(int)verts.size();i++)ev.push_back({verts[i].x,2,i});sort(ev.begin(),ev.end(),[](auto a,auto b){return a.x!=b.x?a.x<b.x:a.type<b.type;});
 ids.resize(n);iota(ids.begin(),ids.end(),0);kd.reserve(n);int root=build(0,n,0);double ans2=0;Tree active;vector<Tree::iterator> it(segs.size());for(auto e:ev){sweepx=e.x;if(e.type==0)active.erase(it[e.id]);else if(e.type==1)it[e.id]=active.insert(e.id).first;else{auto p=verts[e.id];queryy=p.y;if(active.order_of_key(segs.size())%2){double best=1e100;int who=-1;nearest(root,p,best,who);ans2=max(ans2,best);}}}
 // Each interval carries two sites. Their distances give a rigorous upper bound.
 struct Task{double lo,hi;int a,b;};vector<Task> st;
 for(int i=0;i<n;i++){P A=pts[i],B=pts[(i+1)%n];double vx=B.x-A.x,vy=B.y-A.y;st.clear();st.push_back({0,1,i,(i+1)%n});while(!st.empty()){auto [l,r,a,b]=st.back();st.pop_back();auto d2=[&](double t,int s){double x=A.x+vx*t-pts[s].x,y=A.y+vy*t-pts[s].y;return x*x+y*y;};double dl1=d2(l,a),dl2=d2(l,b),dr1=d2(r,a),dr2=d2(r,b);double upper=max(min(dl1,dl2),min(dr1,dr2));double fl=dl1-dl2,fr=dr1-dr2,t=(l+r)*.5;if((fl<0&&fr>0)||(fl>0&&fr<0)){t=l+(r-l)*(-fl)/(fr-fl);upper=max(upper,min(d2(t,a),d2(t,b)));}double tol=1e-9*max(1.0,sqrt(ans2));if(upper <= (sqrt(ans2)+tol)*(sqrt(ans2)+tol))continue;P p{A.x+vx*t,A.y+vy*t};double best=1e100;int who=-1;nearest(root,p,best,who);ans2=max(ans2,best);if(t<=l||t>=r)continue;st.push_back({l,t,a,who});st.push_back({t,r,who,b});}}
 cout<<setprecision(15)<<sqrt(ans2)<<'\n';}
