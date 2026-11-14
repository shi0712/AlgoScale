#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static char buf[1<<25];
int M; ll P,Q;
vector<int> cnt, c, isq; vector<ll> cnt1, hist;
// cnt1[v]: number of mountains whose first sqrt gives v (v<=1414)
ll evalE(ll lam){
    // c[v] = min_b floor(v/2^b)+lam*b
    c[0]=0;
    for(int v=1;v<=M;v++){ ll t=lam+c[v>>1]; c[v]= t<v? (int)t : v; }
    ll base=0; int maxg=0;
    for(int h=0;h<=M;h++) if(cnt[h]){
        base+=(ll)cnt[h]*c[h];
        int g=c[h]-c[isq[h]];
        hist[g]+=cnt[h]; if(g>maxg)maxg=g;
    }
    int S=(int)cnt1.size()-1;
    for(int v=0;v<=S;v++) if(cnt1[v]){
        int x=v;
        while(x>1){ int y=isq[x]; int g=c[x]-c[y]; hist[g]+=cnt1[v]; if(g>maxg)maxg=g; x=y; }
    }
    ll rem=P, taken=0;
    for(int g=maxg;g>=1;g--){
        if(hist[g]){ ll t=min(rem,hist[g]); taken+=t*g; rem-=t; hist[g]=0; if(rem==0)break; }
    }
    for(int g=0;g<=maxg;g++) hist[g]=0;
    return base-taken;
}
int main(){
    size_t len=fread(buf,1,sizeof(buf)-1,stdin); buf[len]=0;
    char*p=buf;
    auto rd=[&]()->ll{ while(*p<'0'||*p>'9')p++; ll x=0; while(*p>='0'&&*p<='9'){x=x*10+(*p-'0');p++;} return x;};
    int n=rd(); P=rd(); Q=rd();
    vector<int> h(n); M=1;
    for(int i=0;i<n;i++){h[i]=rd(); M=max(M,h[i]);}
    cnt.assign(M+1,0); for(int x:h)cnt[x]++;
    vector<int>().swap(h);
    isq.assign(M+1,0); { int r=0; for(int v=0;v<=M;v++){ while((ll)(r+1)*(r+1)<=v) r++; isq[v]=r; } }
    cnt1.assign(isq[M]+1,0); for(int v=0;v<=M;v++) if(cnt[v]) cnt1[isq[v]]+=cnt[v];
    c.assign(M+1,0); hist.assign(M+2,0);
    auto H=[&](ll lam){ return evalE(lam)-lam*Q; };
    // H concave in lam; find max over lam in [0, M]
    ll lo=0, hi=M+1; // find smallest lam with H(lam+1)-H(lam) <= 0
    while(lo<hi){ ll mid=(lo+hi)/2; if(H(mid+1)-H(mid)<=0) hi=mid; else lo=mid+1; }
    printf("%lld\n",H(lo));
}
