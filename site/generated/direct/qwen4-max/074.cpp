#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static ll n,m,N2; static int qcnt;
static inline ll md(ll a){a%=N2; if(a<0)a+=N2; return a;}
static inline ll dst(ll a,ll b){ll d=md(a-b); return min(d,N2-d);}
static inline ll clmp(ll d,ll g){return max(g,min(d,n-g));}
static inline ll fdiv(ll a,ll b){return a>=0?a/b:-((-a+b-1)/b);}
static inline ll cdiv(ll a,ll b){return -fdiv(-a,b);}
struct Comp{int type; ll g,C,start,cnt;};
static ll cntArc(ll st,ll cnt,ll A,ll len){ if(cnt<=0)return 0; if(len>=N2)return cnt; ll t=0;
 for(int sh=-1;sh<=1;++sh){ll lo=A+(ll)sh*N2,hi=lo+len; ll i0=max((ll)0,cdiv(lo-st,2)),i1=min(cnt-1,fdiv(hi-st,2)); if(i1>=i0)t+=i1-i0+1;} return t;}
static ll cntAns(const Comp&c,ll p,ll v){ if(v<1||v>n-1)return 0;
 if(c.type==0){ll g=c.g; if(v<g||v>n-g)return 0;
  if(v==g)return cntArc(c.start,c.cnt,md(p-g),2*g);
  if(v==n-g)return cntArc(c.start,c.cnt,md(p+n-g),2*(n-g));
  ll r=0,q1=md(p-v),q2=md(p+v),d1=md(q1-c.start),d2=md(q2-c.start);
  if(!(d1&1)&&d1/2<c.cnt)++r; if(!(d2&1)&&d2/2<c.cnt)++r; return r;}
 ll d=dst(p,c.C);
 if(v==d){ll lim=min(d,n-d),hi=min(c.start+2*(c.cnt-1),lim); if(hi<c.start)return 0; return (hi-c.start)/2+1;}
 ll g=(v>d?v:n-v); if(g<1||g>m)return 0; if(clmp(d,g)!=v)return 0;
 ll df=g-c.start; if(df<0||(df&1)||df/2>=c.cnt)return 0; return 1;}
static vector<Comp> filt(const vector<Comp>&cs,ll p,ll v){vector<Comp>o;
 for(const Comp&c:cs){ if(c.type==0){ll g=c.g; if(v<g||v>n-g)continue;
   if(v==g||v==n-g){ll A,len; if(v==g){A=md(p-g);len=2*g;}else{A=md(p+n-g);len=2*(n-g);}
    if(len>=N2){o.push_back(c);continue;}
    for(int sh=-1;sh<=1;++sh){ll lo=A+(ll)sh*N2,hi=lo+len; ll i0=max((ll)0,cdiv(lo-c.start,2)),i1=min(c.cnt-1,fdiv(hi-c.start,2));
     if(i1>=i0){Comp d;d.type=0;d.g=g;d.C=0;d.start=md(c.start+2*i0);d.cnt=i1-i0+1;o.push_back(d);}}}
   else{ll qs[2]={md(p-v),md(p+v)}; for(int t=0;t<2;++t){ll df=md(qs[t]-c.start);
     if(!(df&1)&&df/2<c.cnt){Comp d;d.type=0;d.g=g;d.C=0;d.start=qs[t];d.cnt=1;o.push_back(d);}}}}
  else{ll d=dst(p,c.C);
   if(v==d){ll lim=min(d,n-d),hi=min(c.start+2*(c.cnt-1),lim);
    if(hi>=c.start){Comp e;e.type=1;e.g=0;e.C=c.C;e.start=c.start;e.cnt=(hi-c.start)/2+1;o.push_back(e);}}
   else{ll g=(v>d?v:n-v); if(g>=1&&g<=m&&clmp(d,g)==v){ll df=g-c.start;
     if(df>=0&&!(df&1)&&df/2<c.cnt){Comp e;e.type=1;e.g=0;e.C=c.C;e.start=g;e.cnt=1;o.push_back(e);}}}}}
 return o;}
static ll total(const vector<Comp>&cs){ll s=0;for(const Comp&c:cs)s+=c.cnt;return s;}
struct Ev{ll maxG,ss;};
static Ev eval(const vector<Comp>&cs,ll p){vector<ll>av;
 for(const Comp&c:cs){ if(c.type==0){ if(cntAns(c,p,c.g)>0)av.push_back(c.g); if(cntAns(c,p,n-c.g)>0)av.push_back(n-c.g);}
  else{ll d=dst(p,c.C); if(cntAns(c,p,d)>0)av.push_back(d);}}
 sort(av.begin(),av.end()); av.erase(unique(av.begin(),av.end()),av.end());
 ll mx=0,ss=0,bt=0,tot=0; for(const Comp&c:cs)tot+=c.cnt;
 for(ll v:av){ll cnt=0;for(const Comp&c:cs)cnt+=cntAns(c,p,v); mx=max(mx,cnt); ss+=cnt*cnt; bt+=cnt;}
 ll sg=tot-bt; if(sg>0){mx=max(mx,min(sg,(ll)cs.size())); ss+=sg;} return Ev{mx,ss};}
static vector<ll> cands(const vector<Comp>&cs){vector<ll>cd;
 auto add=[&](ll p){ll q=md(p); if(q&1){cd.push_back(md(q-1));cd.push_back(md(q+1));}else cd.push_back(q);};
 for(const Comp&c:cs){ if(c.type==0){ll g=c.g,lo=c.start,k=c.cnt,r=m-g;
   add(lo-g);add(lo-g-n);add(lo+2*(k-1)-g);add(lo+2*(k-1)-g-n);
   ll j0=fdiv(k-r-2,2);
   for(ll dj=-2;dj<=2;++dj){ll j=j0+dj; if(j<0||j>k-1)continue; add(lo+2*j-g);add(lo+2*j-g-n);}}
  else{add(c.C);add(c.C-1);add(c.C+1); ll gm=c.start+2*(c.cnt-1),gi=c.start+(c.cnt-1);
   add(c.C+gm);add(c.C-gm);add(c.C+gi);add(c.C-gi);}}
 add(0);add(n/2);add(n);add(n+n/2);
 sort(cd.begin(),cd.end()); cd.erase(unique(cd.begin(),cd.end()),cd.end()); return cd;}
static ll ask(ll x){printf("? %lld\n",x+1);fflush(stdout);ll d;if(scanf("%lld",&d)!=1)exit(0);++qcnt;return d;}
int main(){int T; if(scanf("%d",&T)!=1)return 0;
 while(T--){ if(scanf("%lld",&n)!=1)return 0; m=(n-1)/2;N2=2*n;qcnt=0;
  ll v1=ask(0), w=min(v1,n-v1), c0=(v1<=m?0:n);
  vector<Comp> cs;
  {Comp b;b.type=0;b.g=w;b.C=0;
   if(c0==0){b.start=md(-w);b.cnt=w+1;}else{b.start=md(n-w+1);b.cnt=w;}
   cs.push_back(b);
   ll Cs[2],gmax;
   if(c0==0){Cs[0]=md(-w);Cs[1]=md(w);gmax=w-2;}else{Cs[0]=md(n-w);Cs[1]=md(n+w);gmax=w-1;}
   for(int t=0;t<2;++t){ll gs=((Cs[t]&1)?1:2);
    if(gmax>=gs){Comp s;s.type=1;s.g=0;s.C=Cs[t];s.start=gs;s.cnt=(gmax-gs)/2+1;cs.push_back(s);}}}
  if(total(cs)>1&&4*w<=n){
   ll p=(c0==0?md(-2*w):md(n-2*w+1)); ll v=ask(p/2); cs=filt(cs,p,v);
   if(total(cs)>1&&!cs.empty()){ll C=(cs[0].type==0?cs[0].start:cs[0].C); ll p2=(C&1)?md(C-1):C;
    ll v2=ask(p2/2); cs=filt(cs,p2,v2);}}
  while(total(cs)>1&&qcnt<30){
   bool allSpike=true; for(const Comp&c:cs)if(c.type==0)allSpike=false;
   bool singleC=true; ll C0=-1;
   for(const Comp&c:cs){ll cc; if(c.type==0){if(c.cnt>1){singleC=false;break;} cc=c.start;} else cc=c.C;
    if(C0<0)C0=cc; else if(C0!=cc){singleC=false;break;}}
   ll p;
   if(singleC&&C0>=0)p=(C0&1)?md(C0-1):C0;
   else if(allSpike){ll cc=cs[0].C;p=(cc&1)?md(cc-1):cc;}
   else{vector<ll>cd=cands(cs); ll bp=cd[0]; Ev be{LLONG_MAX,LLONG_MAX};
    for(ll q:cd){Ev e=eval(cs,q); if(e.maxG<be.maxG||(e.maxG==be.maxG&&e.ss<be.ss)){be=e;bp=q;}} p=bp;}
   ll v=ask(p/2); cs=filt(cs,p,v); if(cs.empty())break;}
  ll C,g;
  if(cs.empty()){C=1;g=1;}
  else if(cs[0].type==0){C=cs[0].start;g=cs[0].g;}
  else{C=cs[0].C;g=cs[0].start;}
  ll a=md(C-g)/2,b=md(C+g)/2;
  printf("! %lld %lld\n",a+1,b+1); fflush(stdout);}
 return 0;}
