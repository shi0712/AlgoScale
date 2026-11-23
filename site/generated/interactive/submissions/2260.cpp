#include <bits/stdc++.h>
using namespace std;
using ll=long long; using lll=__int128;
struct Pt{int x,y;};
static int n; static vector<Pt> P;
static ll crs(const Pt&O,const Pt&A,const Pt&B){return (ll)(A.x-O.x)*(B.y-O.y)-(ll)(A.y-O.y)*(B.x-O.x);}
static int sgn(ll v){return (v>0)-(v<0);}
static bool onSeg(const Pt&a,const Pt&b,const Pt&c){return min(a.x,b.x)<=c.x&&c.x<=max(a.x,b.x)&&min(a.y,b.y)<=c.y&&c.y<=max(a.y,b.y);}
static bool segInt(const Pt&a,const Pt&b,const Pt&c,const Pt&d){int d1=sgn(crs(a,b,c)),d2=sgn(crs(a,b,d)),d3=sgn(crs(c,d,a)),d4=sgn(crs(c,d,b));
 if(((d1>0&&d2<0)||(d1<0&&d2>0))&&((d3>0&&d4<0)||(d3<0&&d4>0)))return true;
 if(d1==0&&onSeg(a,b,c))return true; if(d2==0&&onSeg(a,b,d))return true;
 if(d3==0&&onSeg(c,d,a))return true; if(d4==0&&onSeg(c,d,b))return true; return false;}
static bool isSimple(const vector<int>&pl){int m=pl.size();
 for(int i=0;i<m;i++){const Pt&a=P[pl[i]];const Pt&b=P[pl[(i+1)%m]];
  for(int j=i+1;j<m;j++){const Pt&c=P[pl[j]];const Pt&d=P[pl[(j+1)%m]];
   if(j==i+1||(i==0&&j==m-1)){const Pt*sh;const Pt*o1;const Pt*o2;
    if(j==i+1){sh=&b;o1=&a;o2=&d;}else{sh=&a;o1=&b;o2=&c;}
    if(crs(*sh,*o1,*o2)==0){ll dt=(ll)(o1->x-sh->x)*(o2->x-sh->x)+(ll)(o1->y-sh->y)*(o2->y-sh->y); if(dt>0)return false;}}
   else if(segInt(a,b,c,d))return false;}} return true;}
static ll area2(const vector<int>&pl){ll s=0;int m=pl.size();for(int i=0;i<m;i++){const Pt&a=P[pl[i]];const Pt&b=P[pl[(i+1)%m]];s+=(ll)a.x*b.y-(ll)b.x*a.y;}return llabs(s);}
struct Fr{lll num,den;};
static lll g128(lll a,lll b){while(b){lll t=a%b;a=b;b=t;}return a;}
static Fr mkfr(lll nu,lll de){if(de<0){nu=-nu;de=-de;}if(nu==0)return Fr{0,1};lll a=nu<0?-nu:nu;lll g=g128(a,de);if(g==0)g=1;return Fr{nu/g,de/g};}
static Fr addfr(const Fr&a,const Fr&b){return mkfr(a.num*b.den+b.num*a.den,a.den*b.den);}
static Fr subfr(const Fr&a,const Fr&b){return mkfr(a.num*b.den-b.num*a.den,a.den*b.den);}
static bool frcmp(const Fr&a,const Fr&b){return a.num*b.den<b.num*a.den;}
static Fr computeF(const vector<int>&pl,int p,int q){vector<Fr> ys;int m=pl.size();
 for(int i=0;i<m;i++){int xu=P[pl[i]].x,yu=P[pl[i]].y,xv=P[pl[(i+1)%m]].x,yv=P[pl[(i+1)%m]].y;
  if(xu==xv)continue; if(xu>xv){swap(xu,xv);swap(yu,yv);}
  if(!((ll)xu*q<(ll)p&&(ll)p<(ll)xv*q))continue;
  lll dx=xv-xu; lll nu=(lll)yu*q*dx+(lll)(yv-yu)*((lll)p-(lll)xu*q); lll de=(lll)q*dx; ys.push_back(mkfr(nu,de));}
 sort(ys.begin(),ys.end(),frcmp); Fr r{0,1};
 for(size_t i=0;i+1<ys.size();i+=2)r=addfr(r,subfr(ys[i+1],ys[i])); return r;}
static vector<Pt> convexHull(vector<Pt> Q){sort(Q.begin(),Q.end(),[](const Pt&a,const Pt&b){return a.x<b.x||(a.x==b.x&&a.y<b.y);});
 Q.erase(unique(Q.begin(),Q.end(),[](const Pt&a,const Pt&b){return a.x==b.x&&a.y==b.y;}),Q.end());
 int m=Q.size(); if(m<=2)return Q; vector<Pt> h(2*m); int k=0;
 for(int i=0;i<m;i++){while(k>=2&&crs(h[k-2],h[k-1],Q[i])<=0)k--;h[k++]=Q[i];}
 for(int i=m-2,t=k+1;i>=0;i--){while(k>=t&&crs(h[k-2],h[k-1],Q[i])<=0)k--;h[k++]=Q[i];}
 h.resize(k-1); return h;}
static void answerArea(ll a2){if(a2<0)a2=0; if(a2%2==0)printf("! %lld 1\n",a2/2); else printf("! %lld 2\n",a2); fflush(stdout); exit(0);}
static bool ask(int p,int q,string&rs,string&ss){printf("? %d %d\n",p,q);fflush(stdout);
 if(!(cin>>rs))return false; if(rs=="-1")return false; if(!(cin>>ss))return false; if(ss=="-1")return false; return true;}
static long double parseLD(const string&s){long double v=0;size_t i=0;bool ng=false;
 if(!s.empty()&&(s[0]=='-'||s[0]=='+')){ng=(s[0]=='-');i=1;} for(;i<s.size();i++){if(s[i]<'0'||s[i]>'9')break;v=v*10+(s[i]-'0');} return ng?-v:v;}
static lll parse128(const string&s){lll v=0;size_t i=0;bool ng=false;
 if(!s.empty()&&(s[0]=='-'||s[0]=='+')){ng=(s[0]=='-');i=1;} for(;i<s.size();i++){if(s[i]<'0'||s[i]>'9')break;v=v*10+(s[i]-'0');} return ng?-v:v;}
struct Item{lll num,den;ll a;};
int main(){
 if(!(cin>>n))return 0; P.resize(n);
 for(int i=0;i<n;i++){if(!(cin>>P[i].x>>P[i].y))return 0;}
 vector<int> xs; for(auto&p:P)xs.push_back(p.x);
 sort(xs.begin(),xs.end()); xs.erase(unique(xs.begin(),xs.end()),xs.end());
 if(xs.size()<2)answerArea(0);
 vector<Pt> h=convexHull(P); int hm=h.size();
 if(hm>=3){bool all=true;
  for(auto&p:P){bool ok=false; for(int i=0;i<hm;i++){if(crs(h[i],h[(i+1)%hm],p)==0&&onSeg(h[i],h[(i+1)%hm],p)){ok=true;break;}} if(!ok){all=false;break;}}
  if(all){ll s=0;for(int i=0;i<hm;i++)s+=(ll)h[i].x*h[(i+1)%hm].y-(ll)h[(i+1)%hm].x*h[i].y; answerArea(llabs(s));}}
 int K=(int)xs.size()-1; vector<int> qp(K),qq(K);
 for(int j=0;j<K;j++){int p=xs[j]+xs[j+1],q=2; if(p%2==0){p/=2;q=1;} qp[j]=p;qq[j]=q;}
 if(n<=10){
  vector<int> idx(n-1); for(int i=0;i<n-1;i++)idx[i]=i+1;
  vector<vector<int>> cand; vector<ll> ar;
  do{ if(idx.front()>idx.back())continue;
   vector<int> pl(n); pl[0]=0; for(int i=0;i<n-1;i++)pl[i+1]=idx[i];
   if(isSimple(pl)){cand.push_back(pl);ar.push_back(area2(pl));}
  }while(next_permutation(idx.begin(),idx.end()));
  if(!cand.empty()){
   {vector<ll> t=ar;sort(t.begin(),t.end());t.erase(unique(t.begin(),t.end()),t.end()); if(t.size()==1)answerArea(t[0]);}
   int C=cand.size();
   if(C<=20000){
    vector<vector<Fr>> fv(C,vector<Fr>(K));
    for(int c=0;c<C;c++)for(int j=0;j<K;j++)fv[c][j]=computeF(cand[c],qp[j],qq[j]);
    vector<int> alive(C); iota(alive.begin(),alive.end(),0); vector<char> used(K,0);
    while(true){
     {ll a0=ar[alive[0]];bool same=true;for(int c:alive)if(ar[c]!=a0){same=false;break;} if(same)answerArea(a0);}
     int best=-1; ll bMx=LLONG_MAX,bSq=LLONG_MAX;
     for(int j=0;j<K&&bMx>1;j++){
      if(used[j])continue;
      vector<Item> v; v.reserve(alive.size());
      for(int c:alive)v.push_back(Item{fv[c][j].num,fv[c][j].den,ar[c]});
      sort(v.begin(),v.end(),[](const Item&A,const Item&B){if(A.num!=B.num)return A.num<B.num;if(A.den!=B.den)return A.den<B.den;return A.a<B.a;});
      ll mx=0,sq=0; size_t i=0;
      while(i<v.size()){size_t j2=i;ll dis=0;ll prev=-1;
       while(j2<v.size()&&v[j2].num==v[i].num&&v[j2].den==v[i].den){if(v[j2].a!=prev){dis++;prev=v[j2].a;}j2++;}
       ll sz=(ll)(j2-i); sq+=sz*sz; mx=max(mx,dis); i=j2;}
      if(mx<bMx||(mx==bMx&&sq<bSq)){bMx=mx;bSq=sq;best=j;}
     }
     if(best<0)break;
     used[best]=1; string rs,ss;
     if(!ask(qp[best],qq[best],rs,ss))return 0;
     lll r=parse128(rs),s=parse128(ss);
     vector<int> na;
     for(int c:alive){ if(fv[c][best].num*s==fv[c][best].den*r)na.push_back(c);}
     alive.swap(na);
     if(alive.empty())break;
    }
   }
  }
 }
 long double tot=0;
 for(int j=0;j<K;j++){
  string rs,ss;
  if(!ask(qp[j],qq[j],rs,ss))return 0;
  long double r=parseLD(rs),s=parseLD(ss);
  if(s==0)return 0;
  tot+=(long double)(xs[j+1]-xs[j])*r/s;
 }
 ll a2=llroundl(2*tot);
 answerArea(a2);
 return 0;
}
