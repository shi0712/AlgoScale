#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
ll n,m;
ll ask(ll x){
    x%=n; if(x<0)x+=n;
    printf("? %lld\n",x+1);
    fflush(stdout);
    ll r;
    if(scanf("%lld",&r)!=1) exit(0);
    if(r==-1) exit(0);
    return r;
}
void answer(ll a,ll b){
    a%=n; if(a<0)a+=n;
    b%=n; if(b<0)b+=n;
    printf("! %lld %lld\n",a+1,b+1);
    fflush(stdout);
    exit(0);
}
int main(){
    if(scanf("%lld",&n)!=1) return 0;
    m=(n-1)/2;
    ll s0=ask(0);
    ll s1=ask(m);
    ll k=s0+s1-2*m;
    if(k==0){
        ll lo=max(0LL,s0-m), hi=(s0-1)/2;
        ll a;
        if(lo>=hi) a=lo;
        else{
            ll r=ask(s0/2);
            a=(s0-r)/2;
        }
        answer(a,s0-a);
    } else if(k==2){
        ll lo=max(1LL,s0-m), hi=(s0-1)/2;
        ll a;
        if(lo>=hi) a=lo;
        else{
            ll r=ask(n-s0/2);
            a=(s0-r)/2;
        }
        ll b=s0-a;
        answer(n-a,n-b);
    } else {
        ll G=n-s0;
        ll lo=max(0LL,s0-m), hi=min(m,s0-1);
        ll off,S;
        if(G<=m){off=0;S=G;}
        else{off=G;S=n-G;}
        ll pl=lo+off, ph=hi+off;
        ll mx=2*m+1-S;
        while(pl<ph){
            ll c=ph-pl+1;
            ll M=m-S;
            ll vs,ve;
            if(c<=M+2){
                vs=S; ve=S+c-1;
            } else {
                ll L=c-(m+2-S);
                ll p=L/2, q=L-p;
                vs=S-p; ve=m+1+q;
            }
            ll y=vs+ph;
            ll r=ask(y);
            ll vlo,vhi;
            if(r==S){
                vlo=vs; vhi=min(ve,S);
            } else if(r==mx){
                vlo=max(vs,m+1); vhi=ve;
            } else {
                ll v=(r+S)/2;
                vlo=vhi=v;
            }
            ll npl=y-vhi, nph=y-vlo;
            pl=npl; ph=nph;
        }
        answer(pl,pl+S);
    }
    return 0;
}
