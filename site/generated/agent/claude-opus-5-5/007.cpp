#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef __int128 LL;
static int n; static vector<ll> X,Y,R,V;
static int curC; static ll qx,qy;
static inline bool qAbove(int arc){int c=arc>>1;int up=arc&1;ll dx=qx-X[c];ll D=R[c]*R[c]-dx*dx;ll t=qy-Y[c];LL t2=(LL)t*t;
  if(up) return t>=0&&t2>(LL)D; else return t>=0||t2<=(LL)D;}
struct Cmp{bool operator()(int a,int b)const{int ca=a>>1,cb=b>>1;if(ca==cb)return (a&1)<(b&1);
  if(ca==curC)return !qAbove(b); if(cb==curC)return qAbove(a); return a<b;}};
struct Inc{ll nx,ny;int den;int c;};
static vector<Inc> inc;
static vector<unsigned long long> prs;
static inline bool tangent(int i,int j){ll dx=X[i]-X[j],dy=Y[i]-Y[j];LL d2=(LL)dx*dx+(LL)dy*dy;
  LL s=R[i]+R[j],t=R[i]-R[j];return d2==s*s||d2==t*t;}
static void addPoint(ll nx,ll ny,ll den,int c){ll g=std::gcd(std::gcd(llabs(nx),llabs(ny)),den);nx/=g;ny/=g;den/=g;inc.push_back({nx,ny,(int)den,c});}
static void chk(int a,int b){int ca=a>>1,cb=b>>1;if(ca==cb)return;if(tangent(ca,cb)){if(ca>cb)swap(ca,cb);prs.push_back(((unsigned long long)ca<<32)|cb);}}
static char buf[1<<25];static int bl=0,bp=0;
static inline int gc(){if(bp==bl){bl=fread(buf,1,sizeof(buf),stdin);bp=0;if(bl<=0)return -1;}return buf[bp++];}
static inline ll rd(){int c=gc();while(c!='-'&&(c<'0'||c>'9'))c=gc();bool neg=false;if(c=='-'){neg=true;c=gc();}ll r=0;while(c>='0'&&c<='9'){r=r*10+(c-'0');c=gc();}return neg?-r:r;}
int main(){n=rd();ll Sx=rd(),Sy=rd(),Tx=rd(),Ty=rd();X.resize(n);Y.resize(n);R.resize(n);V.resize(n);
  for(int i=0;i<n;i++){X[i]=rd();Y[i]=rd();R[i]=rd();V[i]=rd();}
  if(Sx==Tx&&Sy==Ty){printf("0\n");return 0;}
  // sweep
  {
    struct Ev{ll x;ll key;int c;};
    vector<Ev> ev;ev.reserve(2*n);
    for(int i=0;i<n;i++){ev.push_back({X[i]-R[i],1LL*1000000000LL- R[i]+1,i});ev.push_back({X[i]+R[i],0,i});}
    // key: deletions (0) first, then insertions by radius descending (key = 1e9 - r + 1 >0, larger r -> smaller key)
    sort(ev.begin(),ev.end(),[](const Ev&a,const Ev&b){if(a.x!=b.x)return a.x<b.x;return a.key<b.key;});
    set<int,Cmp> st;vector<set<int,Cmp>::iterator> its(2*n);
    for(auto&e:ev){int c=e.c;
      if(e.key==0){for(int a=2*c;a<2*c+2;a++){auto it=its[a];bool hp=it!=st.begin();auto nx=next(it);bool hn=nx!=st.end();int pv=hp?*prev(it):-1,nv=hn?*nx:-1;st.erase(it);if(hp&&hn)chk(pv,nv);}}
      else{curC=c;qx=X[c]-R[c];qy=Y[c];auto r1=st.insert(2*c);auto r2=st.insert(r1.first,2*c+1);
        its[2*c]=r1.first;its[2*c+1]=r2;
        if(r1.first!=st.begin())chk(*prev(r1.first),2*c);
        auto nx=next(r2);if(nx!=st.end())chk(2*c+1,*nx);}
    }
  }
  sort(prs.begin(),prs.end());prs.erase(unique(prs.begin(),prs.end()),prs.end());
  inc.reserve(prs.size()*2+2*n/4+16);
  for(auto p:prs){int i=p>>32,j=p&0xffffffffu;ll s=R[i]+R[j];ll dx=X[i]-X[j],dy=Y[i]-Y[j];LL d2=(LL)dx*dx+(LL)dy*dy;
    ll nx,ny,den;
    if(d2==(LL)s*s){den=s;nx=X[i]*den+R[i]*(X[j]-X[i]);ny=Y[i]*den+R[i]*(Y[j]-Y[i]);}
    else{int b=i,sm=j;if(R[b]<R[sm])swap(b,sm);den=R[b]-R[sm];nx=X[b]*den+R[b]*(X[sm]-X[b]);ny=Y[b]*den+R[b]*(Y[sm]-Y[b]);}
    addPoint(nx,ny,den,i);addPoint(nx,ny,den,j);}
  vector<unsigned long long>().swap(prs);
  {
    struct E{ll x,y;int c;};vector<E> ex;ex.reserve(2*n);
    for(int i=0;i<n;i++){ex.push_back({X[i]-R[i],Y[i],i});ex.push_back({X[i]+R[i],Y[i],i});}
    sort(ex.begin(),ex.end(),[](const E&a,const E&b){if(a.x!=b.x)return a.x<b.x;if(a.y!=b.y)return a.y<b.y;return a.c<b.c;});
    for(size_t i=0;i<ex.size();){size_t j=i;while(j<ex.size()&&ex[j].x==ex[i].x&&ex[j].y==ex[i].y)j++;
      if(j-i>=2)for(size_t k=i;k<j;k++)inc.push_back({ex[k].x,ex[k].y,1,ex[k].c});i=j;}
  }
  for(int i=0;i<n;i++){ll dx=Sx-X[i],dy=Sy-Y[i];if((LL)dx*dx+(LL)dy*dy==(LL)R[i]*R[i])inc.push_back({Sx,Sy,1,i});
    dx=Tx-X[i];dy=Ty-Y[i];if((LL)dx*dx+(LL)dy*dy==(LL)R[i]*R[i])inc.push_back({Tx,Ty,1,i});}
  sort(inc.begin(),inc.end(),[](const Inc&a,const Inc&b){if(a.nx!=b.nx)return a.nx<b.nx;if(a.ny!=b.ny)return a.ny<b.ny;if(a.den!=b.den)return a.den<b.den;return a.c<b.c;});
  inc.erase(unique(inc.begin(),inc.end(),[](const Inc&a,const Inc&b){return a.nx==b.nx&&a.ny==b.ny&&a.den==b.den&&a.c==b.c;}),inc.end());
  size_t M=inc.size();
  vector<ll> NX,NY;vector<int> ND;vector<int> nodeOf(M);int S=-1,T=-1;
  for(size_t i=0;i<M;i++){if(i==0||inc[i].nx!=inc[i-1].nx||inc[i].ny!=inc[i-1].ny||inc[i].den!=inc[i-1].den){NX.push_back(inc[i].nx);NY.push_back(inc[i].ny);ND.push_back(inc[i].den);
      int id=NX.size()-1;if(inc[i].den==1&&inc[i].nx==Sx&&inc[i].ny==Sy)S=id;if(inc[i].den==1&&inc[i].nx==Tx&&inc[i].ny==Ty)T=id;}
    nodeOf[i]=NX.size()-1;}
  int N=NX.size();
  if(S<0||T<0){printf("-1\n");return 0;}
  vector<int> cs(n+1,0);for(size_t i=0;i<M;i++)cs[inc[i].c+1]++;for(int i=0;i<n;i++)cs[i+1]+=cs[i];
  vector<int> lst(M);{vector<int> pos(cs.begin(),cs.end()-1);for(size_t i=0;i<M;i++)lst[pos[inc[i].c]++]=nodeOf[i];}
  vector<Inc>().swap(inc);vector<int>().swap(nodeOf);
  struct Ed{int u,v;double w;};vector<Ed> eds;
  for(int c=0;c<n;c++){int k=cs[c+1]-cs[c];if(k<2)continue;int*a=&lst[cs[c]];
    auto vec=[&](int p,ll&ux,ll&uy){ux=NX[p]-X[c]*ND[p];uy=NY[p]-Y[c]*ND[p];};
    sort(a,a+k,[&](int p,int q){ll ux,uy,wx,wy;vec(p,ux,uy);vec(q,wx,wy);bool hp=(uy>0||(uy==0&&ux>0)),hq=(wy>0||(wy==0&&wx>0));
      if(hp!=hq)return hp;LL cr=(LL)ux*wy-(LL)uy*wx;return cr>0;});
    for(int i=0;i<k;i++){int p=a[i],q=a[(i+1)%k];ll ux,uy,wx,wy;vec(p,ux,uy);vec(q,wx,wy);
      LL cr=(LL)ux*wy-(LL)uy*wx,dt=(LL)ux*wx+(LL)uy*wy;long double th=atan2l((long double)cr,(long double)dt);
      if(th<0)th+=2*acosl(-1.0L);
      eds.push_back({p,q,(double)(th*(long double)R[c]*(long double)V[c])});}
  }
  vector<int>().swap(lst);
  vector<int> st(N+1,0);for(auto&e:eds){st[e.u+1]++;st[e.v+1]++;}for(int i=0;i<N;i++)st[i+1]+=st[i];
  vector<pair<int,double>> adj(st[N]);{vector<int> pos(st.begin(),st.end()-1);for(auto&e:eds){adj[pos[e.u]++]={e.v,e.w};adj[pos[e.v]++]={e.u,e.w};}}
  vector<Ed>().swap(eds);
  vector<double> dist(N,1e300);priority_queue<pair<double,int>,vector<pair<double,int>>,greater<>> pq;dist[S]=0;pq.push({0,S});
  while(!pq.empty()){auto [d,u]=pq.top();pq.pop();if(d>dist[u])continue;if(u==T)break;
    for(int i=st[u];i<st[u+1];i++){auto [v,w]=adj[i];if(d+w<dist[v]){dist[v]=d+w;pq.push({dist[v],v});}}}
  if(dist[T]>=1e299){printf("-1\n");return 0;}
  printf("%.17g\n",dist[T]);
}
