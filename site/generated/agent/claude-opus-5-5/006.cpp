#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull;
int THR = 20000;
int lval[11]; // discrete log base 2 mod 11 of a_d
int p, t; ll P; int cd[64]; int Cs[5][5];
inline int gfull(ll r){ int res=1; for(int i=0;i<t;i++){ int d=r%p; r/=p; res=res*Cs[d][cd[i]]%p; if(!res) return 0;} return res; }
ll inv_mod(ll a, ll m){ ll g=m,x=0,x1=1,a1=a; // ext gcd
    ll b=m; ll u=1,v=0; a1=a; while(b){ ll q=a1/b; ll tmp=a1-q*b; a1=b; b=tmp; tmp=u-q*v; u=v; v=tmp;} (void)g;(void)x;(void)x1;
    u%=m; if(u<0)u+=m; return u; }
int s_split; ll ps;
vector<pair<ll,int>> Xs, Ys; vector<int> pre; ll curE=-1;
void build(ll E){
    if(E==curE) return; curE=E;
    Xs.clear(); Ys.clear();
    for(ll r=0;r<ps;r++){ int w=1; ll x=r; for(int i=0;i<s_split;i++){ w=w*Cs[x%p][cd[i]]%p; x/=p; if(!w)break;} if(w){ ll X=(P - (__int128)r*E%P)%P; Xs.push_back({X,w}); } }
    ll hs=1; for(int i=s_split;i<t;i++) hs*=p;
    ll step=(ll)((__int128)ps*E%P);
    for(ll r=0;r<hs;r++){ int w=1; ll x=r; for(int i=s_split;i<t;i++){ w=w*Cs[x%p][cd[i]]%p; x/=p; if(!w)break;} if(w){ ll Y=(P - (__int128)r*step%P)%P; Ys.push_back({Y,w}); } }
    sort(Xs.begin(),Xs.end()); sort(Ys.begin(),Ys.end());
    pre.assign(Ys.size()+1,0); for(size_t k=0;k<Ys.size();k++) pre[k+1]=(pre[k]+Ys[k].second)%p;
}
struct Ptr{ size_t k=0; int get(ll v){ while(k<Ys.size() && Ys[k].first<v) k++; return pre[k]; } };
// sum_{q=qlo}^{qhi} g((N - D q) mod P), N>=0, D>0
int Fsum(ll N, ll D, ll qlo, ll qhi){
    if(qhi<qlo) return 0;
    ll Q=(qhi-qlo+1)%P; if(Q==0) return 0;
    ll Dm=D%P; ll Np=((N%P) - (ll)((__int128)(qlo%P)*Dm%P))%P; if(Np<0) Np+=P;
    if(Q<=THR){ ll r=Np; int s=0; for(ll q=0;q<Q;q++){ s+=gfull(r); r-=Dm; if(r<0) r+=P; } return s%p; }
    ll E=inv_mod(Dm,P); build(E);
    ll K=(ll)((__int128)Np*E%P);
    size_t A=Xs.size(); if(A==0||Ys.empty()) return 0;
    size_t idx0=lower_bound(Xs.begin(),Xs.end(),make_pair(P-K,INT_MIN))-Xs.begin();
    Ptr pl,p1,p2; ll tot=0;
    auto proc=[&](size_t k){ ll z=Xs[k].first+K; if(z>=P) z-=P; ll lo=P-z; ll v1=min(lo+Q,P), v2=max(lo+Q-P,0LL);
        int c1=p1.get(v1), c0=pl.get(lo), c2=p2.get(v2); tot+= (ll)Xs[k].second*(c1-c0+c2+p); };
    for(size_t k=idx0;k-->0;) proc(k);
    for(size_t k=A;k-->idx0;) proc(k);
    return (int)(tot%p);
}
int solveP(ll n, ll c, int pp){
    p=pp; for(int a=0;a<p;a++)for(int b=0;b<p;b++){ ll v=1; if(b>a)v=0; else { for(int i=0;i<b;i++) v=v*(a-i)/(i+1);} Cs[a][b]=v%p; }
    P=1; t=0; while(P<=c+1){ P*=p; t++; }
    ll x=c; for(int i=0;i<t;i++){ cd[i]=x%p; x/=p; }
    s_split=t/2; ps=1; for(int i=0;i<s_split;i++) ps*=p; curE=-1;
    ll total=0; ll B=1;
    for(int i=0;;i++){
        if(B>n) break;
        ll D=B*11; // may be large but < 2^63 since B<=n<=1e18
        ll U=n/B;
        for(int s=0;s<11;s++){
            int dl=((lval[s]-lval[(s+10)%11])%p+p)%p; if(!dl) continue;
            ll qlo=(s==0?1:0); if(U<s+11*qlo) continue;
            ll qhi=(U-s)/11;
            ll N=n-B*s+c;
            total+= (ll)dl*Fsum(N,D,qlo,qhi);
        }
        if(B>n/11) break; B*=11;
    }
    return (int)(total%p);
}
int main(int argc,char**argv){
    if(argc>1) THR=atoi(argv[1]);
    // a_d mod 11 for d=0..10: central trinomial
    ll a[11]; for(int d=0;d<11;d++){ // coefficient via sum_m d!/( (d-2m)! m! m!)
        ll s=0; for(int m=0;2*m<=d;m++){ ll v=1; // multinomial
            // compute d!/((d-2m)! m! m!) exactly
            ll num=1; for(int k=1;k<=d;k++) num*=k; ll den=1; for(int k=1;k<=d-2*m;k++) den*=k; for(int k=1;k<=m;k++) den*=k*k; v=num/den; s+=v; }
        a[d]=s%11; }
    for(int d=0;d<11;d++){ int pw=1; int L=-1; for(int e=0;e<10;e++){ if(pw==a[d]){L=e;break;} pw=pw*2%11; } lval[d]=L; }
    int T; if(scanf("%d",&T)!=1) return 0;
    while(T--){ ll n,c; scanf("%lld %lld",&n,&c);
        int r2=solveP(n,c,2), r5=solveP(n,c,5);
        int e=0; for(int x=0;x<10;x++) if(x%2==r2 && x%5==r5) e=x;
        int ans=1; for(int k=0;k<e;k++) ans=ans*2%11; printf("%d\n",ans); }
}
