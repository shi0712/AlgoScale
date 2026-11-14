#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull; typedef long long ll;
const ull MOD=998244353;
ull pw(ull b,ull e){ull r=1;b%=MOD;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
int main(){
    int n; if(scanf("%d",&n)!=1) return 0;
    const int V=6000; vector<int> cnt(V+1,0);
    for(int i=0;i<n;i++){int a;scanf("%d",&a);cnt[a]++;}
    vector<int> sp; for(int p=2;p<=V;p++){bool ok=true;for(int d=2;d*d<=p;d++)if(p%d==0){ok=false;break;} if(ok&&p*p<=V) sp.push_back(p);}
    int K=sp.size(); // 21
    int FULL=1<<K;
    vector<unsigned> base(FULL,1);
    // per large prime: map q -> vector
    map<int,vector<unsigned>> grp; map<int,int> gk;
    auto kOf=[&](int q){int lim=V/q;int k=0;while(k<K&&sp[k]<=lim)k++;return k;};
    for(int v=1;v<=V;v++){ if(!cnt[v]) continue;
        int x=v,mask=0;
        for(int i=0;i<K;i++) if(x%sp[i]==0){mask|=1<<i; while(x%sp[i]==0)x/=sp[i];}
        ull val=pw(1+v,cnt[v]);
        if(x==1){ base[mask]=base[mask]*val%MOD; }
        else { int q=x; if(!grp.count(q)){int k=kOf(q);gk[q]=k;grp[q].assign(1<<k,1);} auto&g=grp[q]; g[mask]=g[mask]*val%MOD; }
    }
    auto zeta=[&](vector<unsigned>&a,int k){ int N=1<<k; for(int b=0;b<k;b++){int B=1<<b; for(int U=0;U<N;U++) if(U&B) a[U]=(ull)a[U]*a[U^B]%MOD;}};
    zeta(base,K);
    vector<vector<unsigned>> acc(K+1);
    for(int k=0;k<=K;k++) acc[k].assign(1<<k,1);
    for(auto&[q,g]:grp){ int k=gk[q]; zeta(g,k); ull c=(ull)(q-1)*pw(q,MOD-2)%MOD; auto&A=acc[k];
        for(int U=0;U<(1<<k);U++){ ull h=(1+c*((g[U]+MOD-1)%MOD))%MOD; A[U]=A[U]*h%MOD; } }
    for(int k=0;k<=K;k++){ if(k==K){ for(int U=0;U<FULL;U++) base[U]=(ull)base[U]*acc[K][U]%MOD; } else {
        bool triv=true; for(auto x:acc[k]) if(x!=1){triv=false;break;} if(triv) continue; int m=(1<<k)-1;
        for(int U=0;U<FULL;U++) base[U]=(ull)base[U]*acc[k][U&m]%MOD; } }
    // mobius inversion over subsets
    for(int b=0;b<K;b++){int B=1<<b; for(int U=0;U<FULL;U++) if(U&B) base[U]=(base[U]+MOD-base[U^B])%MOD;}
    vector<unsigned> w(FULL); w[0]=1; for(int b=0;b<K;b++){ ull c=(ull)(sp[b]-1)*pw(sp[b],MOD-2)%MOD; int B=1<<b; for(int U=0;U<B;U++) w[U|B]=w[U]*c%MOD; }
    ull ans=0; for(int U=0;U<FULL;U++) ans=(ans+(ull)base[U]*w[U])%MOD;
    printf("%llu\n",ans);
}
