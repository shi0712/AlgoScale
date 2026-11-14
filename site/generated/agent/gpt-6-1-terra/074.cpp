#include <bits/stdc++.h>
using namespace std;
using ll=long long;
struct Seg { int type; ll l,r; };
ll n,m,d;
ll mod(ll x){x%=n;if(x<0)x+=n;return x;}
ll val(int t,ll a,ll x){ll b=d-a; ll p=t==2?mod(-a):a; ll q=t==0?b:mod(-b); auto dist=[&](ll z){ll k=llabs(z-x);return min(k,n-k);};return dist(p)+dist(q);}
vector<Seg> split(const Seg&s,ll x){
 vector<ll> z={s.l,s.r+1};
 // Each transmitter's distance changes slope at its own position and its antipode.
 for(ll v: {x,mod(x+m),mod(x+m+1)}){
  ll a=(s.type==2?mod(-v):v);
  ll b=(s.type==0?v:mod(-v));
  for(ll c:{a,d-b}) for(int j=-2;j<=2;j++) if(c+j>s.l && c+j<=s.r) z.push_back(c+j);
 }
 sort(z.begin(),z.end());z.erase(unique(z.begin(),z.end()),z.end());
 vector<Seg> out;for(int i=0;i+1<(int)z.size();i++)out.push_back({s.type,z[i],z[i+1]-1});return out;
}
vector<Seg> filter(const vector<Seg>&ss,ll x,ll y){vector<Seg> out; for(auto s:ss)for(auto q:split(s,x)){
 ll v=val(q.type,q.l,x), w=val(q.type,q.r,x); if(v==w){if(v==y)out.push_back(q);}else {ll delta=(w-v)/(q.r-q.l); if((y-v)%delta==0){ll a=q.l+(y-v)/delta;if(a>=q.l&&a<=q.r)out.push_back({q.type,a,a});}}
 } sort(out.begin(),out.end(),[](auto a,auto b){return tie(a.type,a.l)<tie(b.type,b.l);}); vector<Seg> merged;for(auto s:out){if(!merged.empty()&&merged.back().type==s.type&&s.l<=merged.back().r+1)merged.back().r=max(merged.back().r,s.r);else merged.push_back(s);}return merged;}
ll score(const vector<Seg>&ss,ll x){
 // Count possible pairs for every response; non-flat pieces form step-two ranges.
 map<ll,ll> flat; vector<pair<ll,ll>> ev[2];
 for(auto s:ss)for(auto q:split(s,x)){ll a=val(q.type,q.l,x),b=val(q.type,q.r,x);if(a==b)flat[a]+=q.r-q.l+1;else{ll lo=min(a,b),hi=max(a,b);ev[lo%2].push_back({lo,1});ev[lo%2].push_back({hi+2,-1});}}
 ll best=0;for(int p=0;p<2;p++){for(auto [v,c]:flat)if(v%2==p){ev[p].push_back({v,0});ev[p].push_back({v+2,0});}sort(ev[p].begin(),ev[p].end());ll active=0;for(int i=0;i<(int)ev[p].size();){ll v=ev[p][i].first;while(i<(int)ev[p].size()&&ev[p][i].first==v)active+=ev[p][i++].second;best=max(best,active+flat[v]);}}return best;
}
ll ask(ll x){cout<<"? "<<x+1<<endl;ll y;if(!(cin>>y)||y<0)exit(0);return y;}
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);int T;if(!(cin>>T))return 0;while(T--){cin>>n;m=n/2;d=ask(0);vector<Seg> ss;ll lo=max(0LL,d-m),hi=min(m,d/2);if(lo<=hi)ss.push_back({0,lo,hi});lo=max(1LL,d-m);hi=min(m,d-1);if(lo<=hi)ss.push_back({1,lo,hi});lo=max(0LL,d-m);hi=min(m,d/2);if(lo<=hi)ss.push_back({2,lo,hi}); // distinct for same-side
 for(auto &s:ss)if(s.type!=1 && d%2==0 && s.r==d/2)s.r--;ss.erase(remove_if(ss.begin(),ss.end(),[](auto s){return s.l>s.r;}),ss.end());
 while(true){ll count=0;for(auto s:ss)count+=s.r-s.l+1;if(count==1)break;
 vector<ll> cand={m, m/2, (3*m)/2};
 for(auto s:ss){for(ll a:{s.l,(s.l+s.r)/2,(s.l+s.r+1)/2,s.r}){ll b=d-a;ll p=s.type==2?mod(-a):a;ll q=s.type==0?b:mod(-b);for(ll v:{p,q,mod(p+m),mod(p+m+1),mod(q+m),mod(q+m+1)}){cand.push_back(v);cand.push_back(mod(v+1));}}}
 sort(cand.begin(),cand.end());cand.erase(unique(cand.begin(),cand.end()),cand.end());ll best=LLONG_MAX,x=0;for(ll c:cand){ll sc=score(ss,c);if(sc<best)best=sc,x=c;}ll y=ask(x);ss=filter(ss,x,y);if(ss.empty())return 0;
 }
 auto s=ss[0];ll a=s.l,b=d-a;ll p=s.type==2?mod(-a):a;ll q=s.type==0?b:mod(-b);cout<<"! "<<p+1<<' '<<q+1<<endl;
 }}
