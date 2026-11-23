#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
ll N;
map<pair<ll,ll>,bool> memo;
bool q(ll x, ll y){
    if(x<1||y<1||x>N||y>N) return false;
    auto key=make_pair(x,y);
    auto it=memo.find(key);
    if(it!=memo.end()) return it->second;
    printf("? %lld %lld\n",x,y);
    fflush(stdout);
    char buf[64];
    if(scanf("%63s",buf)!=1) exit(0);
    string s(buf);
    bool r;
    if(s=="true") r=true;
    else if(s=="false") r=false;
    else exit(0);
    memo[key]=r;
    return r;
}
// find max offset t>=0 such that (x0+t*dx, y0+t*dy) is flattened (contiguous)
ll edge(ll x0, ll y0, int dx, int dy){
    ll last=0, s=1;
    while(q(x0+s*dx, y0+s*dy)){ last=s; s*=2; }
    ll lo=last, hi=s;
    while(hi-lo>1){
        ll mid=(lo+hi)/2;
        if(q(x0+mid*dx,y0+mid*dy)) lo=mid; else hi=mid;
    }
    return lo;
}
int main(){
    ll x0,y0;
    if(scanf("%lld %lld %lld",&N,&x0,&y0)!=3) return 0;
    ll r=edge(x0,y0,1,0);
    ll l=edge(x0,y0,-1,0);
    ll xr=x0+r, xl=x0-l;
    ll M=xr-xl+1;
    ll u=edge(x0,y0,0,1);
    ll yt=y0+u;
    ll cx=(xl+xr)/2;
    ll cy=yt-(M-1)/2;
    for(int i=0;i<2;i++){
        if(q(cx-2*M,cy)) cx-=2*M; else break;
    }
    for(int i=0;i<2;i++){
        if(q(cx,cy-2*M)) cy-=2*M; else break;
    }
    if(q(cx-M,cy-M)){ cx-=M; cy-=M; }
    printf("! %lld %lld\n",cx+2*M,cy+2*M);
    fflush(stdout);
    return 0;
}
