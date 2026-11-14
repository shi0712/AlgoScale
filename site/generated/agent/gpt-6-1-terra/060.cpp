#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using i128 = __int128_t;
ll cd(ll a,ll b){return a/b+(a%b!=0);}
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);
 int t;cin>>t;while(t--){ll h,w,s;cin>>h>>w>>s;
 if((i128)h*w<=s){cout<<0<<'\n';continue;}
 // Search maximum height of a horizontal strip. For any such height,
 // the largest permissible width is floor(s / height).
 ll end=min(h,s), best=h+w;
 auto eval=[&](ll a){ if(a<1||a>end)return; ll b=min(w,s/a); best=min(best,cd(h,a)+cd(w,b));};
 long double opt=sqrtl((long double)h*(long double)s/(long double)w);
 ll p= opt>=end?end:max(1LL,(ll)opt);
 eval(1);eval(end);eval(p);if(p<end)eval(p+1);
 // Widths beyond w are equivalent, and their best height is floor(s/w).
 eval(min(end,s/w));
 struct Interval{ll l,r;}; vector<Interval> st;st.push_back({1,end});
 while(!st.empty()){
  auto [l,r]=st.back();st.pop_back();
  ll bl=min(w,s/l),br=min(w,s/r);
  if(bl==br){eval(r);continue;}
  // h/a + max(1, w*a/s) bounds the number of strips from below.
  // Its minimum on an interval is at the clamped unconstrained optimum
  // or at the transition a=s/w.
  long double lo=l,hi=r, ss=s, hh=h,ww=w;
  long double x=max(lo,min(hi,opt));
  long double boundary=ss/ww;
  x=max(x,min(hi,max(lo,boundary)));
  long double lower=hh/x+max(1.0L,ww*x/ss);
  // Allow a generous rounding margin; this can only reduce pruning.
  long double margin=1e-15L*max(1.0L,fabsl(lower))+1e-9L;
  if(lower-margin > (long double)best-1.0L) continue;
  if(r-l<=8){for(ll a=l;a<=r;a++)eval(a);continue;}
  ll mid=l+(r-l)/2;
  st.push_back({l,mid});st.push_back({mid+1,r});
 }
 cout<<best-2<<'\n';
 }
}
