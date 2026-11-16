#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const ll NEG = -1000000000000000000LL;
static char ibuf[1<<16]; static int ilen=0, ipos=0;
static inline int gc(){ if(ipos==ilen){ ilen=(int)fread(ibuf,1,sizeof(ibuf),stdin); ipos=0; if(ilen<=0) return -1;} return (unsigned char)ibuf[ipos++]; }
static inline int readInt(){ int c=gc(); while(c!=-1&&(c<'0'||c>'9')) c=gc(); int x=0; while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();} return x; }
struct Item{ int f; int w[10]; };
int main(){
    int n=readInt(), k=readInt();
    vector<Item> grp[11]; ll totalP=0,totalFull=0;
    for(int i=0;i<n;i++){ int p=readInt(); Item it; it.f=0; memset(it.w,0,sizeof(it.w));
        for(int j=1;j<=p;j++){ int v=readInt(); if(j==p) it.f=v; else it.w[j]=v; }
        totalP+=p; totalFull+=it.f; grp[p].push_back(it); }
    if(totalP<=k){ printf("%lld\n",totalFull); return 0; }
    int m[11]; vector<ll> pre[11];
    for(int g=1;g<=10;g++){ sort(grp[g].begin(),grp[g].end(),[](const Item&a,const Item&b){return a.f>b.f;});
        m[g]=(int)grp[g].size(); pre[g].assign(m[g]+1,0);
        for(int t=1;t<=m[g];t++) pre[g][t]=pre[g][t-1]+grp[g][t-1].f; }
    int ptr[11]={0}, xc[11]={0}; int rem=k;
    while(rem>0){ int best=-1;
        for(int g=1;g<=10;g++){ if(ptr[g]>=m[g]) continue;
            if(best<0) best=g;
            else if((ll)grp[g][ptr[g]].f*best > (ll)grp[best][ptr[best]].f*g) best=g; }
        if(best<0) break; xc[best]++; ptr[best]++;
        if(best<=rem) rem-=best; else break; }
    const int B=100; int lo[11],hi[11],base=0,maxRange=0;
    for(int g=1;g<=10;g++){ lo[g]=max(0,xc[g]-B); hi[g]=min(m[g],xc[g]+B);
        base+=g*lo[g]; maxRange+=g*(hi[g]-lo[g]); }
    int CAP=maxRange+12;
    vector<ll> dp0(CAP,NEG),dp1(CAP,NEG),nd0(CAP,NEG),nd1(CAP,NEG);
    dp0[0]=0; int curSZ=1; vector<int> suf; static ll V[10][205];
    for(int g=1;g<=10;g++){
        int Wg=hi[g]-lo[g]; int newSZ=curSZ+g*Wg;
        for(int r=1;r<g;r++) for(int c=0;c<=Wg;c++) V[r][c]=NEG;
        if(m[g]>0&&g>1){ suf.assign(m[g]+2,INT_MIN);
            for(int r=1;r<g;r++){ suf[m[g]+1]=INT_MIN;
                for(int t=m[g];t>=1;t--) suf[t]=max(suf[t+1],grp[g][t-1].w[r]);
                int pmax=INT_MIN;
                for(int t=1;t<=m[g];t++){ int d=grp[g][t-1].w[r]-grp[g][t-1].f; if(d>pmax)pmax=d;
                    int c=t-1;
                    if(c>=lo[g]&&c<=hi[g]){ ll a=pre[g][t]+pmax, b=pre[g][t-1]+suf[t]; V[r][c-lo[g]]=max(a,b);} } } }
        fill(nd0.begin(),nd0.begin()+newSZ+9,NEG);
        fill(nd1.begin(),nd1.begin()+newSZ+9,NEG);
        ll* __restrict S0=dp0.data(); ll* __restrict S1=dp1.data();
        ll* __restrict T0=nd0.data(); ll* __restrict T1=nd1.data();
        for(int c=0;c<=Wg;c++){
            ll val=pre[g][lo[g]+c]; int off=g*c;
            int jm=min(newSZ-1,curSZ-1+off);
            for(int j=off;j<=jm;j++){ ll t=S0[j-off]+val; if(t>T0[j]) T0[j]=t; }
            int jm1=min(newSZ+8,curSZ+8+off);
            for(int j=off;j<=jm1;j++){ ll t=S1[j-off]+val; if(t>T1[j]) T1[j]=t; }
            for(int r=1;r<g;r++){ ll v=V[r][c]; if(v==NEG) continue;
                int st=off+r, en=min(newSZ+8,curSZ-1+off+r);
                for(int j=st;j<=en;j++){ ll t=S0[j-off-r]+v; if(t>T1[j]) T1[j]=t; } } }
        dp0.swap(nd0); dp1.swap(nd1); curSZ=newSZ;
    }
    int idx=k-base; ll ans=0;
    if(idx>=0&&idx<CAP){ if(idx<=maxRange) ans=max(ans,dp0[idx]); ans=max(ans,dp1[idx]); }
    if(ans<0) ans=0;
    printf("%lld\n",ans);
    return 0;
}
