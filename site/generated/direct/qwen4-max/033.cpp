#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef __int128 lll;
static char ibuf[1<<22]; static int ipos=0,ilen=0;
static inline int gc(){ if(ipos==ilen){ilen=(int)fread(ibuf,1,sizeof(ibuf),stdin);ipos=0;if(ilen<=0)return -1;} return (unsigned char)ibuf[ipos++]; }
static inline bool readInt(int&out){int c=gc();while(c!=-1&&(c<'0'||c>'9')&&c!='-')c=gc();if(c==-1)return false;int s=1;if(c=='-'){s=-1;c=gc();}int x=0;while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();}out=x*s;return true;}
static int n; static vector<ll> PX,PY;
static inline ll orient(int a,int b,int c){return (PX[b]-PX[a])*(PY[c]-PY[a])-(PY[b]-PY[a])*(PX[c]-PX[a]);}
static inline bool inCircle(int a,int b,int c,int d){
 ll adx=PX[a]-PX[d],ady=PY[a]-PY[d],bdx=PX[b]-PX[d],bdy=PY[b]-PY[d],cdx=PX[c]-PX[d],cdy=PY[c]-PY[d];
 lll al=(lll)adx*adx+(lll)ady*ady,bl=(lll)bdx*bdx+(lll)bdy*bdy,cl=(lll)cdx*cdx+(lll)cdy*cdy;
 lll det=al*((lll)bdx*cdy-(lll)bdy*cdx)-bl*((lll)adx*cdy-(lll)ady*cdx)+cl*((lll)adx*bdy-(lll)ady*bdx);
 if(det>0)return true; if(det<0)return false;
 int ids[4]={a,b,c,d};
 for(int i=1;i<4;i++){int k=ids[i],j=i-1;while(j>=0&&ids[j]<k){ids[j+1]=ids[j];--j;}ids[j+1]=k;}
 for(int t=0;t<4;t++){int p=ids[t];ll o; if(p==a)o=orient(b,c,d);else if(p==b)o=-orient(a,c,d);else if(p==c)o=orient(a,b,d);else o=-orient(a,b,c); if(o!=0)return o>0;} return false;}
struct Tri{int v[3],nb[3];};
static vector<Tri> tris; static vector<int> tmark,freeIds,fanA,fanB; static vector<char> alive; static int stampCnt=0,hintTri;
static int allocTri(int a,int b,int c,int n0,int n1,int n2){int id; if(!freeIds.empty()){id=freeIds.back();freeIds.pop_back();}else{id=(int)tris.size();tris.push_back(Tri());tmark.push_back(0);alive.push_back(1);} tris[id].v[0]=a;tris[id].v[1]=b;tris[id].v[2]=c;tris[id].nb[0]=n0;tris[id].nb[1]=n1;tris[id].nb[2]=n2;alive[id]=1;return id;}
static void insertPoint(int p){
 int t=hintTri; long long g=0;
 while(true){int nx=-1;for(int i=0;i<3;i++){int A=tris[t].v[(i+1)%3],B=tris[t].v[(i+2)%3];if(orient(A,B,p)<0){nx=tris[t].nb[i];break;}}if(nx<0)break;t=nx;if(++g>2000000)break;}
 bool ok=true; for(int i=0;i<3;i++) if(orient(tris[t].v[(i+1)%3],tris[t].v[(i+2)%3],p)<0) ok=false;
 if(!ok){for(int q=0;q<(int)tris.size();q++){if(!alive[q])continue;bool in=true;for(int i=0;i<3;i++)if(orient(tris[q].v[(i+1)%3],tris[q].v[(i+2)%3],p)<0){in=false;break;}if(in){t=q;break;}}}
 ++stampCnt; static vector<int> cav,nt; cav.clear();nt.clear(); cav.push_back(t);tmark[t]=stampCnt;
 for(size_t i=0;i<cav.size();i++){int cu=cav[i];for(int j=0;j<3;j++){int nb=tris[cu].nb[j];if(nb<0||tmark[nb]==stampCnt)continue;if(inCircle(tris[nb].v[0],tris[nb].v[1],tris[nb].v[2],p)){tmark[nb]=stampCnt;cav.push_back(nb);}}}
 for(int cu:cav)for(int j=0;j<3;j++){int nb=tris[cu].nb[j];if(nb>=0&&tmark[nb]==stampCnt)continue;int A=tris[cu].v[(j+1)%3],B=tris[cu].v[(j+2)%3];int id=allocTri(A,B,p,-1,-1,nb);
  if(nb>=0){for(int k=0;k<3;k++)if(tris[nb].v[(k+1)%3]==B&&tris[nb].v[(k+2)%3]==A){tris[nb].nb[k]=id;break;}}
  nt.push_back(id);fanA[A]=id;fanB[B]=id;}
 for(int id:nt){int A=tris[id].v[0],B=tris[id].v[1];tris[id].nb[0]=fanA[B];tris[id].nb[1]=fanB[A];}
 for(int cu:cav){alive[cu]=0;freeIds.push_back(cu);} hintTri=nt[0];}
static ll hilbertOrder(int x,int y,int pw,int rot){if(pw==0)return 0;int h=1<<(pw-1);int sg=(x<h)?((y<h)?0:3):((y<h)?1:2);sg=(sg+rot)&3;static const int rd[4]={3,0,0,1};int nx=x&(h-1),ny=y&(h-1),nr=(rot+rd[sg])&3;ll sub=1LL<<(2*pw-2);ll ans=(ll)sg*sub;ll add=hilbertOrder(nx,ny,pw-1,nr);ans+=(sg==1||sg==2)?add:(sub-add-1);return ans;}
struct KDTree{int n;vector<double> px,py;vector<int> idx;double best,qx,qy;int bestId;
 void brec(int l,int r,int d){if(r-l<=1)return;int m=(l+r)/2;if(d&1)nth_element(idx.begin()+l,idx.begin()+m,idx.begin()+r,[&](int a,int b){return py[a]<py[b];});else nth_element(idx.begin()+l,idx.begin()+m,idx.begin()+r,[&](int a,int b){return px[a]<px[b];});brec(l,m,d+1);brec(m+1,r,d+1);}
 void build(vector<ll>&X,vector<ll>&Y,int nn){n=nn;px.resize(n);py.resize(n);idx.resize(n);for(int i=0;i<n;i++){px[i]=(double)X[i];py[i]=(double)Y[i];idx[i]=i;}brec(0,n,0);}
 void qrec(int l,int r,int d){if(r<=l)return;int m=(l+r)/2;int id=idx[m];double dx=px[id]-qx,dy=py[id]-qy;double d2=dx*dx+dy*dy;if(d2<best){best=d2;bestId=id;}double df=(d&1)?(qy-py[id]):(qx-px[id]);if(df<0){qrec(l,m,d+1);if(df*df<best)qrec(m+1,r,d+1);}else{qrec(m+1,r,d+1);if(df*df<best)qrec(l,m,d+1);}}
 int nearest(double x,double y,double i2,int ii){qx=x;qy=y;best=i2;bestId=ii;qrec(0,n,0);return bestId;}};
struct PIP{int m=0;vector<ll> ysi;vector<double> ysd;vector<vector<int>> tr;vector<double> ex,ey,es;
 void ins(int nd,int l,int r,int ql,int qr,int id){if(qr<l||r<ql)return;if(ql<=l&&r<=qr){tr[nd].push_back(id);return;}int md=(l+r)/2;ins(nd*2,l,md,ql,qr,id);ins(nd*2+1,md+1,r,ql,qr,id);}
 void snode(int nd,int l,int r){if(!tr[nd].empty()){double ym=((double)ysi[l]+(double)ysi[r+1])*0.5;auto&v=tr[nd];sort(v.begin(),v.end(),[&](int a,int b){return ex[a]+(ym-ey[a])*es[a]<ex[b]+(ym-ey[b])*es[b];});}if(l==r)return;int md=(l+r)/2;snode(nd*2,l,md);snode(nd*2+1,md+1,r);}
 void build(int nn,vector<ll>&X,vector<ll>&Y){ysi.assign(Y.begin(),Y.begin()+nn);sort(ysi.begin(),ysi.end());ysi.erase(unique(ysi.begin(),ysi.end()),ysi.end());m=(int)ysi.size()-1;ysd.assign(ysi.begin(),ysi.end());if(m<=0)return;tr.assign(4*m+4,vector<int>());ex.assign(nn,0);ey.assign(nn,0);es.assign(nn,0);
  for(int i=0;i<nn;i++){int j=(i+1)%nn;if(Y[i]==Y[j])continue;int a=i,b=j;if(Y[a]>Y[b])swap(a,b);ex[i]=(double)X[a];ey[i]=(double)Y[a];es[i]=(double)(X[b]-X[a])/(double)(Y[b]-Y[a]);int lo=(int)(lower_bound(ysi.begin(),ysi.end(),Y[a])-ysi.begin());int hi=(int)(lower_bound(ysi.begin(),ysi.end(),Y[b])-ysi.begin())-1;ins(1,0,m-1,lo,hi,i);} snode(1,0,m-1);}
 bool inside(double qx,double qy){if(m<=0)return false;int i=(int)(upper_bound(ysd.begin(),ysd.end(),qy)-ysd.begin())-1;if(i<0||i>=m)return false;int cnt=0,nd=1,l=0,r=m-1;
  while(true){vector<int>&v=tr[nd];if(!v.empty()){int lo=0,hi=(int)v.size();while(lo<hi){int md=(lo+hi)/2;int id=v[md];if(ex[id]+(qy-ey[id])*es[id]>qx)hi=md;else lo=md+1;}cnt+=(int)v.size()-lo;}if(l==r)break;int md=(l+r)/2;if(i<=md){nd=nd*2;r=md;}else{nd=nd*2+1;l=md+1;}} return (cnt&1)!=0;}};
struct Cand{double x,y,r;}; struct It{double t1,t2;int v1,v2;};
int main(){
 if(!readInt(n))return 0; PX.resize(n+3);PY.resize(n+3);
 for(int i=0;i<n;i++){int x,y;readInt(x);readInt(y);PX[i]=x;PY[i]=y;}
 vector<int> ord(n);for(int i=0;i<n;i++)ord[i]=i; vector<ll> hk(n);
 for(int i=0;i<n;i++)hk[i]=hilbertOrder((int)(PX[i]+16384),(int)(PY[i]+16384),15,0);
 sort(ord.begin(),ord.end(),[&](int a,int b){return hk[a]<hk[b];});
 PX[n]=-100000;PY[n]=-100000;PX[n+1]=300000;PY[n+1]=-100000;PX[n+2]=-100000;PY[n+2]=300000;
 fanA.assign(n+3,-1);fanB.assign(n+3,-1);
 tris.reserve(2*n+64);tmark.reserve(2*n+64);alive.reserve(2*n+64);
 hintTri=allocTri(n,n+1,n+2,-1,-1,-1);
 for(int i=0;i<n;i++)insertPoint(ord[i]);
 vector<Cand> cs;cs.reserve(2*n);
 for(int t=0;t<(int)tris.size();t++){ if(!alive[t])continue; int a=tris[t].v[0],b=tris[t].v[1],c=tris[t].v[2]; if(a>=n||b>=n||c>=n)continue;
  long double ax=PX[a],ay=PY[a],bx=PX[b]-ax,by=PY[b]-ay,cx=PX[c]-ax,cy=PY[c]-ay; long double d=2*(bx*cy-by*cx); if(d==0)continue;
  long double b2=bx*bx+by*by,c2=cx*cx+cy*cy; long double ux=(cy*b2-by*c2)/d,uy=(bx*c2-cx*b2)/d; long double r=sqrtl(ux*ux+uy*uy);
  cs.push_back(Cand{(double)(ax+ux),(double)(ay+uy),(double)r}); }
 sort(cs.begin(),cs.end(),[](const Cand&A,const Cand&B){return A.r>B.r;});
 static PIP pip; pip.build(n,PX,PY);
 double R=0;
 for(size_t i=0;i<cs.size();i++){ if(cs[i].r<=R)break; if(pip.inside(cs[i].x,cs[i].y)){R=cs[i].r;break;} }
 static KDTree kd; kd.build(PX,PY,n);
 vector<It> st;st.reserve(4096);
 for(int i=0;i<n;i++){
  int a=i,b=(i+1)%n; double ax=(double)PX[a],ay=(double)PY[a];
  double dx=(double)(PX[b]-PX[a]),dy=(double)(PY[b]-PY[a]); double L=sqrt(dx*dx+dy*dy);
  if(L*0.5<=R)continue; double ux=dx/L,uy=dy/L; st.clear(); st.push_back(It{0.0,L,a,b}); int guard=0;
  while(!st.empty()){
   if(++guard>2000000)break;
   It it=st.back();st.pop_back(); double t1=it.t1,t2=it.t2;int v1=it.v1,v2=it.v2;
   if(min(t2,L-t1)<=R)continue;
   double x1=ax+t1*ux,y1=ay+t1*uy,x2=ax+t2*ux,y2=ay+t2*uy;
   double p1x=x1-(double)PX[v1],p1y=y1-(double)PY[v1],p2x=x2-(double)PX[v1],p2y=y2-(double)PY[v1];
   double q1x=x1-(double)PX[v2],q1y=y1-(double)PY[v2],q2x=x2-(double)PX[v2],q2y=y2-(double)PY[v2];
   double d1v1=sqrt(p1x*p1x+p1y*p1y),d2v1=sqrt(p2x*p2x+p2y*p2y);
   double d1v2=sqrt(q1x*q1x+q1y*q1y),d2v2=sqrt(q2x*q2x+q2y*q2y);
   double U=min(max(d1v1,d2v1),max(d1v2,d2v2)); if(U<=R)continue;
   if(v1==v2){double vv=max(d1v1,d2v1);if(vv>R)R=vv;continue;}
   double Ax=(double)PX[v1]-ax,Ay=(double)PY[v1]-ay,Bx=(double)PX[v2]-ax,By=(double)PY[v2]-ay;
   double num=(Ax*Ax+Ay*Ay)-(Bx*Bx+By*By),den=2*(ux*(Ax-Bx)+uy*(Ay-By));
   double tc; if(den==0)tc=0.5*(t1+t2); else tc=num/den;
   if(!(tc>t1))tc=t1; if(!(tc<t2))tc=t2;
   double qx=ax+tc*ux,qy=ay+tc*uy;
   double fx=qx-(double)PX[v1],fy=qy-(double)PY[v1]; double dd2=fx*fx+fy*fy;
   int w=kd.nearest(qx,qy,dd2,v1);
   double gx=qx-(double)PX[w],gy=qy-(double)PY[w]; double dw=sqrt(gx*gx+gy*gy);
   if(dw>R)R=dw;
   if(w==v1||w==v2)continue;
   if(tc-t1>1e-9)st.push_back(It{t1,tc,v1,w});
   if(t2-tc>1e-9)st.push_back(It{tc,t2,w,v2});
  }
 }
 printf("%.10f\n",R);
 return 0;
}
