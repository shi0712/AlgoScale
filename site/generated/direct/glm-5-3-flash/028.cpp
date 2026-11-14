#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
typedef unsigned int u32;
const u32 MOD = 998244353;
const int MAXV = 6000;
const int K = 21; // core primes: 2..73
int cnt_[MAXV + 1];
u32 pw(u32 b, int e){ u64 r=1,m=b%MOD; while(e){ if(e&1) r=r*m%MOD; m=m*m%MOD; e>>=1; } return (u32)r; }
int main(){
    int n; if(scanf("%d",&n)!=1) return 0;
    for(int i=0;i<n;i++){ int a; scanf("%d",&a); cnt_[a]++; }
    vector<int> spf(MAXV+1,0); vector<int> primes;
    for(int i=2;i<=MAXV;i++){
        if(!spf[i]){ spf[i]=i; primes.push_back(i); }
        for(int j=0;j<(int)primes.size() && primes[j]<=spf[i] && (long long)primes[j]*i<=MAXV; j++)
            spf[primes[j]*i]=primes[j];
    }
    vector<int> bitidx(MAXV+1,-1);
    for(int i=0;i<K;i++) bitidx[primes[i]]=i;
    const int P79=primes[K];
    vector<u32> inv(MAXV+1,1);
    for(int i=2;i<=MAXV;i++) inv[i]=(u64)(MOD-MOD/i)*inv[MOD%i]%MOD;
    vector<u32> f(MAXV+1,1);
    for(int v=1;v<=MAXV;v++) if(cnt_[v]) f[v]=pw((u32)(v+1),cnt_[v]);
    u32 C=1; vector<u32> w(MAXV+1,0);
    for(int p:primes){ w[p]=inv[p-1]; C=(u64)C*(p-1)%MOD*inv[p]%MOD; }
    vector<vector<pair<int,u32>>> valsCore(K+1);
    vector<pair<int,u32>> vals79;
    vector<vector<pair<int,u32>>> itemsFor(MAXV+1);
    for(int v=2;v<=MAXV;v++){
        if(!cnt_[v]) continue;
        int x=v,mp=0,mask=0;
        while(x>1){ int p=spf[x]; mp=p; if(bitidx[p]>=0) mask|=1<<bitidx[p]; while(x%p==0)x/=p; }
        int mask2=(bitidx[mp]>=0)?(mask&~(1<<bitidx[mp])):mask;
        u32 fv=f[v];
        if(bitidx[mp]>=0) valsCore[bitidx[mp]+1].push_back({mask2,fv});
        else if(mp==P79) vals79.push_back({mask2,fv});
        else itemsFor[mp].push_back({mask2,fv});
    }
    auto merge=[](vector<pair<int,u32>> v){
        sort(v.begin(),v.end()); vector<pair<int,u32>> r;
        for(auto&pr:v){ if(!r.empty()&&r.back().first==pr.first) r.back().second=(u64)r.back().second*pr.second%MOD; else r.push_back(pr); }
        return r;
    };
    for(int k=1;k<=K;k++) valsCore[k]=merge(valsCore[k]);
    vals79=merge(vals79);
    vector<vector<pair<u32,vector<pair<int,u32>>>>> msgs(K);
    vector<u32> scalarMsg(K,1);
    for(int idx=K+1; idx<(int)primes.size(); idx++){
        int u=primes[idx];
        int j=(int)(upper_bound(primes.begin(),primes.begin()+K,MAXV/u)-primes.begin());
        if(itemsFor[u].empty()) scalarMsg[j]=(u64)scalarMsg[j]*(w[u]+1)%MOD;
        else msgs[j].push_back({w[u],merge(itemsFor[u])});
    }
    const int SZ=1<<K;
    vector<u32> A(SZ,0),buf(SZ),buf2(SZ);
    A[0]=f[1];
    for(int k=1;k<=K;k++){
        int hb=1<<(k-1),full=hb-1;
        u32 sc=scalarMsg[k-1];
        if(sc!=1) for(int s=0;s<hb;s++) A[s]=(u64)A[s]*sc%MOD;
        for(auto&ms:msgs[k-1]){
            fill(buf.begin(),buf.begin()+hb,1u);
            for(auto&pr:ms.second){
                int allowed=full&~pr.first; u32 fv=pr.second;
                for(int s=allowed;;s=(s-1)&allowed){ buf[s]=(u64)buf[s]*fv%MOD; if(!s)break; }
            }
            u32 wu=ms.first;
            for(int s=0;s<hb;s++) A[s]=(u64)A[s]*(wu+buf[s])%MOD;
        }
        fill(buf2.begin(),buf2.begin()+hb,1u);
        for(auto&pr:valsCore[k]){
            int allowed=full&~pr.first; u32 fv=pr.second;
            for(int s=allowed;;s=(s-1)&allowed){ buf2[s]=(u64)buf2[s]*fv%MOD; if(!s)break; }
        }
        u32 wq=w[primes[k-1]];
        for(int s=0;s<hb;s++){
            u64 t=A[s];
            A[s]=(u64)t*buf2[s]%MOD;
            A[s|hb]=(u64)t*wq%MOD;
        }
    }
    fill(buf.begin(),buf.end(),1u);
    for(auto&pr:vals79){
        int allowed=(SZ-1)&~pr.first; u32 fv=pr.second;
        for(int s=allowed;;s=(s-1)&allowed){ buf[s]=(u64)buf[s]*fv%MOD; if(!s)break; }
    }
    u32 w79=w[P79];
    u64 G=0;
    for(int s=0;s<SZ;s++) G=(G+(u64)A[s]*((w79+buf[s])%MOD))%MOD;
    printf("%u\n",(u32)((u64)G*C%MOD));
    return 0;
}
