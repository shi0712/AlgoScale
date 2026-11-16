#include <bits/stdc++.h>
using namespace std;
using ll = long long;

int main(){
    int n, k;
    if(scanf("%d %d", &n, &k)!=2) return 0;
    vector<pair<int,int>> a(n);              // (t, v)
    for(int i=0;i<n;i++){
        int v,t; scanf("%d %d",&v,&t);
        a[i]=make_pair(t,v);
    }
    sort(a.begin(), a.end(), [](const pair<int,int>&x, const pair<int,int>&y){
        if(x.first!=y.first) return x.first>y.first;   // weight descending
        return x.second>y.second;                      // value descending
    });

    const int J   = min(k,26);      // doublings cap
    const int D   = 338;            // |difference| cap
    const int PAD = 52;             // >= 2*max t
    const int LEN = 2*D+1;
    const int W   = LEN+2*PAD;
    const int OFF = PAD+D;          // index representing difference 0
    const ll  NEG = -1000000000000000000LL;

    size_t SZ = (size_t)(J+1)*W;
    vector<ll> A(SZ, NEG), B(SZ, NEG);
    ll *cur=A.data(), *nxt=B.data();
    cur[OFF]=0;

    for(int i=0;i<n;i++){
        int t=a[i].first, t2=2*t;
        ll v=a[i].second;
        {   // j = 0 : no doubling available
            const ll* __restrict c=cur;
            ll* __restrict d=nxt;
            for(int idx=PAD; idx<PAD+LEN; idx++){
                ll best=c[idx];
                ll p=c[idx-t]+v;  if(p>best) best=p;   // into S
                ll q=c[idx+t]+v;  if(q>best) best=q;   // into T
                d[idx]=best;
            }
        }
        for(int j=1;j<=J;j++){
            const ll* __restrict c=cur+(size_t)j*W;
            const ll* __restrict p=cur+(size_t)(j-1)*W;
            ll* __restrict d=nxt+(size_t)j*W;
            for(int idx=PAD; idx<PAD+LEN; idx++){
                ll best=c[idx];
                ll x1=c[idx-t]+v;   if(x1>best) best=x1;   // S, plain
                ll x2=c[idx+t]+v;   if(x2>best) best=x2;   // T, plain
                ll x3=p[idx-t2]+v;  if(x3>best) best=x3;   // S, doubled
                ll x4=p[idx+t2]+v;  if(x4>best) best=x4;   // T, doubled
                d[idx]=best;
            }
        }
        swap(cur,nxt);
    }

    ll ans=0;
    for(int j=0;j<=J;j++) ans=max(ans,cur[(size_t)j*W+OFF]);
    printf("%lld\n",ans);
    return 0;
}
