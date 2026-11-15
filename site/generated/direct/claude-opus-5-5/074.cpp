#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

ll n, g, flipv, W0, K;
ll gmin_, gmax_, par_;

ll floorDiv2(ll a){ return a>=0 ? a/2 : -((-a+1)/2); }
ll ceilDiv2(ll a){ return -floorDiv2(-a); }

ll askpos(ll Y){
    ll y = ((Y/2) % n + n) % n;
    if(y==0) y=n;
    printf("? %lld\n", y);
    fflush(stdout);
    ll d;
    if(scanf("%lld",&d)!=1) exit(0);
    if(d<0) exit(0);
    return flipv ? n-d : d;
}
bool allowedG(ll gg){
    return gg>=gmin_ && gg<=gmax_ && ((gg&1)==par_);
}
bool consB(ll E, ll Y, ll r){
    ll m2=2*n;
    ll d=(((Y-E)%m2)+m2)%m2;
    ll D=min(d,m2-d);
    if(gmin_>gmax_) return false;
    if(r==D){
        if(gmin_<=D && D<=n-gmin_) return true;
    }
    if(allowedG(r) && D<=r) return true;
    if(allowedG(n-r) && D>=r) return true;
    return false;
}
void finishAns(ll W, ll gf){
    ll M2 = W - flipv*n;
    ll a=(M2-gf)/2, b=(M2+gf)/2;
    a=((a-1)%n+n)%n+1;
    b=((b-1)%n+n)%n+1;
    printf("! %lld %lld\n", a, b);
    fflush(stdout);
}
ll verifyW(ll W){
    ll Y = (((W%2)+2)%2==0) ? W : W+1;
    return askpos(Y);
}
struct Out{ int type; ll jl,jr; bool bm; }; // 0 NODE,1 BPLUS,2 BMINUS,3 FAIL
Out stepF(ll jl, ll jr, ll Y, ll r, bool bmAlive){
    ll thr=Y-g;
    if(r<g) return {1,0,0,false};
    ll j0=ceilDiv2(thr-W0);
    ll s=max(jl,min(j0,jr+1));
    if(r==g){
        if(s>jr) return {3,0,0,false};
        return {0,s,jr,false};
    }
    ll sEnd=min(jr,s-1);
    bool have=false; ll a1=jl,a2=-1;
    if(r==n-g){
        ll jm=floorDiv2(Y-(n-g)-W0);
        ll e=min(sEnd,jm);
        if(e>=jl){have=true;a2=e;}
    } else if(r<n-g){
        ll W=Y-r; ll t=W-W0;
        if((((t%2)+2)%2)==0){
            ll j=t/2;
            if(j>=jl && j<=sEnd){have=true;a1=a2=j;}
        }
    }
    bool consb = bmAlive && consB(2-g,Y,r);
    if(have){
        bool bm = (flipv==0) && consb && a1==0;
        return {0,a1,a2,bm};
    }
    if(consb) return {2,0,0,false};
    return {3,0,0,false};
}

int main(){
    int T;
    if(scanf("%d",&T)!=1) return 0;
    while(T--){
        scanf("%lld",&n);
        printf("? 1\n"); fflush(stdout);
        ll f1; if(scanf("%lld",&f1)!=1) return 0;
        if(f1<0) return 0;
        flipv = (2*f1>n)?1:0;
        ll h1 = flipv ? n-f1 : f1;
        g=h1;
        K = flipv ? g : g+1;
        W0 = flipv ? 3-g : 2-g;
        par_ = (g+flipv)&1;
        gmin_ = par_?1:2;
        gmax_ = (((g-1)&1)==par_) ? g-1 : g-2;
        bool bexists = gmin_<=gmax_;
        // second query
        ll Y2 = (g%2==0)? g+2 : g+1;
        ll r2 = askpos(Y2);
        Out o = stepF(0,K-1,Y2,r2,bexists);
        if(o.type==1){ finishAns(2+g, r2); continue; }
        if(o.type==2){ ll z=verifyW(2-g); finishAns(2-g,z); continue; }
        if(o.type==3){ finishAns(W0 + 0, g); continue; }
        ll jl=o.jl, jr=o.jr; bool bm=o.bm;
        int R=28;
        bool done=false;
        while(!done){
            if(jl==jr){
                if(bm){ ll z=verifyW(W0+2*jl); finishAns(W0+2*jl,z); }
                else finishAns(W0+2*jl,g);
                done=true; break;
            }
            ll H = 1LL<<max(R-1,0);
            ll bmw=bm?1:0;
            ll lowS=max(jl+1,jr+1-H), highS=min(jr,jl+H-bmw);
            ll s=(jl+jr+1)/2;
            if(lowS<=highS){ s=max(s,lowS); s=min(s,highS); }
            ll Y;
            if(flipv==0) Y=g+(W0+2*s);
            else Y=g+(W0+2*(s-1))+1;
            ll r=askpos(Y);
            R--;
            Out q=stepF(jl,jr,Y,r,bm);
            if(q.type==0){ jl=q.jl; jr=q.jr; bm=q.bm; }
            else if(q.type==2){ ll z=verifyW(2-g); finishAns(2-g,z); done=true; }
            else if(q.type==1){ finishAns(2+g, r); done=true; }
            else { finishAns(W0+2*jl,g); done=true; }
        }
    }
    return 0;
}
