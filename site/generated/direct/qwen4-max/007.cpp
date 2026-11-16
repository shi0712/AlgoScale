#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull; typedef __int128 lll;
static char ibuf[1<<16]; static int ipos=0,ilen=0;
static inline int gc(){ if(ipos==ilen){ilen=(int)fread(ibuf,1,sizeof(ibuf),stdin);ipos=0;if(ilen<=0)return -1;} return (unsigned char)ibuf[ipos++]; }
static inline ll rdInt(){ int c=gc(); while(c!=-1&&c!='-'&&(c<'0'||c>'9'))c=gc(); bool ng=false; if(c=='-'){ng=true;c=gc();} ll x=0; while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();} return ng?-x:x; }
static int n; static vector<int> CX,CY,CR,CV; static int curX; static vector<ull> prs;
static inline void addP(int i,int j){ if(i==j)return; if(i>j)swap(i,j); prs.push_back(((ull)i<<20)|(ull)j); }
static inline void tryP(int i,int j){ if(i==j)return; ll dx=(ll)CX[j]-CX[i],dy=(ll)CY[j]-CY[i],d2=dx*dx+dy*dy,s=(ll)CR[i]+CR[j],f=(ll)CR[i]-CR[j]; if(d2==s*s||(f!=0&&d2==f*f))addP(i,j); }
static inline int sgnS(ll p,ll q,ll A){ if(q==0||A==0)return (p>0)-(p<0); if(p>=0&&q>0)return 1; if(p<=0&&q<0)return -1; lll pp=(lll)p*p,qa=(lll)q*q*A; int s=(pp>qa)-(pp<qa); return p>0?s:-s; }
static inline int cmpArc(int a,int b){ int i=a>>1,j=b>>1,ti=a&1,tj=b&1; ll da=(ll)curX-CX[i],db=(ll)curX-CX[j]; ll A=(ll)CR[i]*CR[i]-da*da,B=(ll)CR[j]*CR[j]-db*db;
 double ya=(double)CY[i]+(ti?sqrt((double)A):-sqrt((double)A)),yb=(double)CY[j]+(tj?sqrt((double)B):-sqrt((double)B)); double df=ya-yb; if(df>1e-5)return 1; if(df<-1e-5)return -1;
 ll D=(ll)CY[i]-CY[j]; int sa=ti?1:-1,sb=tj?1:-1; int sf=sgnS(D,sa,A),sg=(B==0)?0:sb;
 if(sf>=0&&sg<=0)return (sf==0&&sg==0)?0:1; if(sf<=0&&sg>=0)return (sf==0&&sg==0)?0:-1;
 ll P=D*D+A-B,Q=2*D*sa; int s=sgnS(P,Q,A); return sf>0?s:-s; }
struct ArcCmp{ bool operator()(int a,int b)const{ int i=a>>1,j=b>>1; if(i==j)return (a&1)<(b&1); int c=cmpArc(a,b); if(c)return c<0; int ti=a&1,tj=b&1;
 if(curX==CX[i]-CR[i]||curX==CX[i]+CR[i]){ if(ti!=tj)return ti<tj; return ti==0?(CR[i]>CR[j]):(CR[i]<CR[j]); }
 else{ if(ti!=tj)return ti>tj; return ti==1?(CR[i]<CR[j]):(CR[i]>CR[j]); } } };
static void pkey(int i,int j,ll&a,ll&b,ll&d){ ll dx=(ll)CX[j]-CX[i],dy=(ll)CY[j]-CY[i],d2=dx*dx+dy*dy,s=(ll)CR[i]+CR[j];
 if(d2==s*s){ d=s; a=(ll)CX[i]*d+dx*CR[i]; b=(ll)CY[i]*d+dy*CR[i]; }
 else{ int bg=i,sm=j; if(CR[i]<CR[j]){bg=j;sm=i;} d=(ll)CR[bg]-CR[sm]; ll ex=(ll)CX[sm]-CX[bg],ey=(ll)CY[sm]-CY[bg]; a=(ll)CX[bg]*d+ex*CR[bg]; b=(ll)CY[bg]*d+ey*CR[bg]; }
 ll am=a%d; if(am<0)am=-am; ll bm=b%d; if(bm<0)bm=-bm; ll g=__gcd(__gcd(am,d),__gcd(bm,d)); a/=g;b/=g;d/=g; }
struct Inc{ ll dx,dy; int node,circ; };
static inline int hf(ll dx,ll dy){ return (dy>0||(dy==0&&dx>0))?0:1; }
static bool angC(const Inc&a,const Inc&b){ int ha=hf(a.dx,a.dy),hb=hf(b.dx,b.dy); if(ha!=hb)return ha<hb; lll c=(lll)a.dx*b.dy-(lll)a.dy*b.dx; return c>0; }
struct PK{ ll a,b; int d,pi; }; struct EP{ int px,py,id; };
int main(){ n=(int)rdInt(); ll Sx=rdInt(),Sy=rdInt(),Tx=rdInt(),Ty=rdInt(); CX.resize(n);CY.resize(n);CR.resize(n);CV.resize(n);
 for(int i=0;i<n;i++){CX[i]=(int)rdInt();CY[i]=(int)rdInt();CR[i]=(int)rdInt();CV[i]=(int)rdInt();}
 if(Sx==Tx&&Sy==Ty){printf("0.0000000000\n");return 0;} prs.reserve((size_t)2*n+16);
 { vector<EP> ep((size_t)2*n); for(int i=0;i<n;i++){ep[2*i]=EP{CX[i]-CR[i],CY[i],i};ep[2*i+1]=EP{CX[i]+CR[i],CY[i],i};}
  sort(ep.begin(),ep.end(),[](const EP&a,const EP&b){if(a.px!=b.px)return a.px<b.px;return a.py<b.py;});
  for(size_t k=1;k<ep.size();k++) if(ep[k].px==ep[k-1].px&&ep[k].py==ep[k-1].py) addP(ep[k].id,ep[k-1].id); }
 { vector<ull> ev((size_t)2*n); for(int i=0;i<n;i++){ ull xl=(ull)(CX[i]-CR[i]+1100000000),xr=(ull)(CX[i]+CR[i]+1100000000);
   ev[2*i]=(xl<<32)|(1ULL<<31)|(ull)i; ev[2*i+1]=(xr<<32)|(ull)i; } sort(ev.begin(),ev.end());
  set<int,ArcCmp> st; vector<set<int,ArcCmp>::iterator> its((size_t)2*n);
  for(size_t k=0;k<ev.size();k++){ ull e=ev[k]; curX=(int)(ll)(e>>32)-1100000000; int t=(int)((e>>31)&1ULL),i=(int)(e&0x7fffffffULL);
   if(t==0){ for(int q=0;q<2;q++){ auto it=its[2*i+q]; int a=-1,b=-1; if(it!=st.begin()){auto p=it;--p;a=*p;} {auto nx=it;++nx;if(nx!=st.end())b=*nx;} st.erase(it); if(a>=0&&b>=0)tryP(a>>1,b>>1);} }
   else{ for(int q=0;q<2;q++){ auto rs=st.insert(2*i+q); auto it=rs.first; its[2*i+q]=it; if(it!=st.begin()){auto p=it;--p;tryP((*p)>>1,i);} auto nx=it;++nx; if(nx!=st.end())tryP(i,(*nx)>>1);} } } }
 sort(prs.begin(),prs.end()); prs.erase(unique(prs.begin(),prs.end()),prs.end()); size_t P=prs.size();
 vector<PK> pk(P); for(size_t k=0;k<P;k++){ int i=(int)(prs[k]>>20),j=(int)(prs[k]&0xfffffULL); ll a,b,d; pkey(i,j,a,b,d); pk[k]=PK{a,b,(int)d,(int)k}; }
 sort(pk.begin(),pk.end(),[](const PK&x,const PK&y){if(x.a!=y.a)return x.a<y.a;if(x.b!=y.b)return x.b<y.b;return x.d<y.d;});
 vector<ll> NA,NB; vector<int> ND; vector<pair<int,int>> inc; inc.reserve(2*P+4); int V=0;
 for(size_t k=0;k<P;k++){ if(k==0||pk[k].a!=pk[k-1].a||pk[k].b!=pk[k-1].b||pk[k].d!=pk[k-1].d){NA.push_back(pk[k].a);NB.push_back(pk[k].b);ND.push_back(pk[k].d);V++;}
  int u=V-1; ull pr=prs[pk[k].pi]; inc.push_back({(int)(pr>>20),u}); inc.push_back({(int)(pr&0xfffffULL),u}); }
 vector<PK>().swap(pk); vector<ull>().swap(prs);
 auto findK=[&](ll px,ll py)->int{ int lo=0,hi=V-1; while(lo<=hi){int m=(lo+hi)>>1; ll a=NA[m],b=NB[m]; int d=ND[m];
   if(a<px||(a==px&&(b<py||(b==py&&d<1))))lo=m+1; else if(a==px&&b==py&&d==1)return m; else hi=m-1;} return -1; };
 int Sn=findK(Sx,Sy),Tn=findK(Tx,Ty);
 if(Sn<0){Sn=V++;NA.push_back(Sx);NB.push_back(Sy);ND.push_back(1);} if(Tn<0){Tn=V++;NA.push_back(Tx);NB.push_back(Ty);ND.push_back(1);}
 for(int i=0;i<n;i++){ ll dx=Sx-CX[i],dy=Sy-CY[i]; if(dx*dx+dy*dy==(ll)CR[i]*CR[i])inc.push_back({i,Sn});
  ll ex=Tx-CX[i],ey=Ty-CY[i]; if(ex*ex+ey*ey==(ll)CR[i]*CR[i])inc.push_back({i,Tn}); }
 sort(inc.begin(),inc.end()); inc.erase(unique(inc.begin(),inc.end()),inc.end()); size_t M=inc.size();
 vector<int> cs(n+1,0); for(auto&p:inc)cs[p.first+1]++; for(int i=0;i<n;i++)cs[i+1]+=cs[i];
 vector<Inc> arr(M); for(size_t m=0;m<M;m++){int c=inc[m].first,u=inc[m].second; arr[m].dx=NA[u]-(ll)CX[c]*ND[u]; arr[m].dy=NB[u]-(ll)CY[c]*ND[u]; arr[m].node=u; arr[m].circ=c;}
 vector<pair<int,int>>().swap(inc); vector<ll>().swap(NA); vector<ll>().swap(NB); vector<int>().swap(ND);
 vector<double> wn(M,0.0); const double PI=acos(-1.0);
 for(int c=0;c<n;c++){ int s=cs[c],e=cs[c+1]; if(e-s<2)continue; sort(arr.begin()+s,arr.begin()+e,angC); double co=(double)CV[c]*(double)CR[c];
  for(int m=s;m<e;m++){ int m2=(m+1<e)?m+1:s; lll cr=(lll)arr[m].dx*arr[m2].dy-(lll)arr[m].dy*arr[m2].dx; lll dt=(lll)arr[m].dx*arr[m2].dx+(lll)arr[m].dy*arr[m2].dy;
   double an=atan2((double)cr,(double)dt); if(an<0)an+=2*PI; wn[m]=an*co; } }
 vector<int> no(M),co(M); for(size_t m=0;m<M;m++){no[m]=arr[m].node;co[m]=arr[m].circ;} vector<Inc>().swap(arr);
 vector<int> ns(V+1,0); for(size_t m=0;m<M;m++){int c=co[m]; if(cs[c+1]-cs[c]>=2)ns[no[m]+1]++;} for(int u=0;u<V;u++)ns[u+1]+=ns[u];
 vector<int> ni(ns[V]); { vector<int> fl(ns.begin(),ns.end()-1); for(size_t m=0;m<M;m++){int c=co[m]; if(cs[c+1]-cs[c]>=2)ni[fl[no[m]]++]=(int)m;} }
 const double INF=1e300; vector<double> dist(V,INF); vector<int> hp(V,-1),heap(V); int hz=0;
 auto up=[&](int idx){ int v=heap[idx]; double dv=dist[v]; while(idx>0){int p=(idx-1)>>2,pv=heap[p]; if(dist[pv]<=dv)break; heap[idx]=pv;hp[pv]=idx;idx=p;} heap[idx]=v;hp[v]=idx; };
 auto dn=[&](int idx){ int v=heap[idx]; double dv=dist[v]; while(true){int c=(idx<<2)+1; if(c>=hz)break; int bs=c,lm=min(c+4,hz); for(int k=c+1;k<lm;k++)if(dist[heap[k]]<dist[heap[bs]])bs=k; int bv=heap[bs]; if(!(dist[bv]<dv))break; heap[idx]=bv;hp[bv]=idx;idx=bs;} heap[idx]=v;hp[v]=idx; };
 dist[Sn]=0; heap[hz++]=Sn; hp[Sn]=0;
 while(hz>0){ int u=heap[0]; hz--; if(hz>0){heap[0]=heap[hz];hp[heap[0]]=0;dn(0);} hp[u]=-2; if(u==Tn)break; double du=dist[u];
  for(int idx=ns[u];idx<ns[u+1];idx++){ int m=ni[idx],c=co[m],s=cs[c],e=cs[c+1]; int mn=(m+1<e)?m+1:s,mp=(m>s)?m-1:e-1;
   { int to=no[mn]; double nd=du+wn[m]; if(nd<dist[to]){dist[to]=nd; if(hp[to]<0){heap[hz]=to;hp[to]=hz;hz++;up(hz-1);}else up(hp[to]);} }
   { int to=no[mp]; double nd=du+wn[mp]; if(nd<dist[to]){dist[to]=nd; if(hp[to]<0){heap[hz]=to;hp[to]=hz;hz++;up(hz-1);}else up(hp[to]);} } } }
 if(dist[Tn]>=INF*0.5)printf("-1\n"); else printf("%.10f\n",dist[Tn]);
 return 0; }
