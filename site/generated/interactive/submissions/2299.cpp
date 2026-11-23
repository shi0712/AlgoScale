#include <bits/stdc++.h>
using namespace std;
using ll = long long; using ld = long double;
static const ll MAXV = 100000000000000LL; static const ld INF = 1e18L;
static ll L = 0, U = MAXV + 1, B = 1;
static ld richCost(ll l, ll u, ld b){ ll W=u-l; if(W<=1) return 0.0L; ld w=log2l((ld)W);
    if(l>=1){ ld c=2.0L*w+((ld)W-b)/(ld)l; return c>w?c:w; }
    if(b>=(ld)W) return w; return INF; }
static ld cost(ll l, ll u, ld b){ if(u-l<=1) return 0.0L; ld best=richCost(l,u,b);
    ld bb=b, cur=(ld)l, extra=0.0L; if(bb<cur){ bb+=cur; extra=1.0L; }
    for(int j=1;j<=64;++j){ if(bb<1.0L) break;
        if(bb>=(ld)(u-1)){ ld rem=bb-(ld)(u-1); ld c=richCost((ll)cur,u-1,rem);
            if(c<INF/2){ ld t=extra+(ld)j+c; if(t<best) best=t; } break; }
        cur=bb; bb*=2.0L; ll nl=(ll)cur;
        if(u-nl<=1){ ld t=extra+(ld)j; if(t<best) best=t; break; }
        ld c=richCost(nl,u,bb); if(c<INF/2){ ld t=extra+(ld)j+c; if(t<best) best=t; } }
    return best; }
static vector<ll> genCands(ll l, ll u, ll b, bool full){ vector<ll> cd; ll W=u-l; ll Xm=min(b,u-1);
    auto add=[&](ld v){ if(v<1.0L||v>(ld)MAXV) return; ll x=(ll)v; if(x<1||x>MAXV) return; cd.push_back(x); };
    if(l>=1) cd.push_back(l);
    if(Xm>l){ add((ld)Xm); add((ld)l+1); add((ld)l+(ld)W/3.0L); add((ld)l+(ld)W/2.0L);
        add((ld)l+(ld)W*2.0L/3.0L); add((ld)l+(ld)W/4.0L); add((ld)b-(ld)l); add((ld)b-(ld)W);
        if(full){ for(int k=1;k<=31;k++) add((ld)l+(ld)W*k/32.0L);
            for(int k=1;k<=15;k++) add((ld)l+(ld)(Xm-l)*k/16.0L);
            add((ld)b-(ld)l*0.5L); add((ld)b-(ld)l*2.0L); add((ld)b-(ld)W*0.5L);
            add((ld)b-(ld)W/3.0L); add((ld)l+(ld)W/5.0L); add((ld)l+(ld)W*2.0L/5.0L);
            add((ld)l+(ld)W*3.0L/4.0L); add((ld)l+(ld)W/8.0L); } }
    sort(cd.begin(),cd.end()); cd.erase(unique(cd.begin(),cd.end()),cd.end()); return cd; }
static ld eval1(ll l, ll u, ll b, ll x){ if(u-l<=1) return 0.0L;
    if(x<=l){ if(x!=l||l<1) return INF; ld c=cost(l,u,(ld)b+(ld)l); return c>=INF/2?INF:1.0L+c; }
    if(x>b||x>u-1||x<1||x>MAXV) return INF;
    ld cs=cost(x,u,(ld)b+(ld)x), cf=cost(l,x,(ld)b-(ld)x); ld m=max(cs,cf);
    return m>=INF/2?INF:1.0L+m; }
static ld best1(ll l, ll u, ll b, bool full){ if(u-l<=1) return 0.0L;
    vector<ll> cd=genCands(l,u,b,full); ld best=INF;
    for(ll x:cd){ ld c=eval1(l,u,b,x); if(c<best) best=c; } return best; }
static ll choose(){ vector<ll> cd=genCands(L,U,B,true); ld bc=INF; ll bx=-1;
    for(ll x:cd){ ld c;
        if(x<=L){ if(x!=L||L<1) continue; ld s=best1(L,U,B+L,false); c=s>=INF/2?INF:1.0L+s; }
        else { if(x>B||x>U-1) continue; ld cs=best1(x,U,B+x,false), cf=best1(L,x,B-x,false);
            ld m=max(cs,cf); c=m>=INF/2?INF:1.0L+m; }
        if(c<bc-1e-9L){ bc=c; bx=x; } }
    if(bx<0){ bc=INF; for(ll x:cd){ ld c=eval1(L,U,B,x); if(c<bc-1e-9L){ bc=c; bx=x; } } }
    if(bx<0) bx=(L>=1?L:min(B,U-1));
    if(bx<1) bx=1; if(bx>MAXV) bx=MAXV; return bx; }
int main(){ int guard=0;
    while(U-L>1){ if(++guard>5000) break; ll X=choose();
        printf("? %lld\n",X); fflush(stdout);
        char buf[128]; if(scanf("%127s",buf)!=1) return 0; string s(buf);
        if(s=="Lucky!"){ B+=X; if(X>L) L=X; }
        else if(s=="Fraudster!"){ B-=X; if(X<U) U=X; }
        else return 0; }
    printf("! %lld\n",L); fflush(stdout); return 0; }
