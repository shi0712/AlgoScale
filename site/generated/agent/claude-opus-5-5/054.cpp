#pragma GCC optimize("O3,unroll-loops")
#include <bits/stdc++.h>
using namespace std;
typedef unsigned int u32; typedef unsigned long long u64;
static const u32 MOD=998244353;

__attribute__((target("avx2"))) static u32 dotp(const u32* __restrict a,const u32* __restrict b,int len){
    u64 total=0; int i=0;
    for(;i+16<=len;i+=16){
        u64 s=0;
        for(int j=0;j<16;j++) s+=(u64)a[i+j]*b[i+j];
        total+=s%MOD;
    }
    u64 s=0;
    for(;i<len;i++) s+=(u64)a[i]*b[i];
    total+=s%MOD;
    return (u32)(total%MOD);
}

int main(){
    int n; if(scanf("%d",&n)!=1) return 0;
    vector<int> a(n+2),b(n+2);
    for(int i=1;i<=n;i++) scanf("%d %d",&a[i],&b[i]);
    // wT[v][u] = w(u,v), u<v
    int W=n+2;
    vector<u32> wT((size_t)W*W,0);
    vector<u32> Abar(W,0);
    for(int u=n;u>=1;u--){
        for(int v=u+1;v<=n;v++){
            // compute Abar(e) for e in (u,v)
            for(int e=u+1;e<v;e++){
                if(a[e]>u && b[e]<v){
                    u32 s=dotp(&Abar[u+1],&wT[(size_t)e*W+u+1],e-u-1);
                    s+=wT[(size_t)e*W+u]; if(s>=MOD) s-=MOD;
                    Abar[e]=s;
                } else Abar[e]=0;
            }
            u32 Ws=dotp(&Abar[u+1],&wT[(size_t)v*W+u+1],v-u-1);
            wT[(size_t)v*W+u]=(1+MOD-Ws)%MOD;
        }
    }
    // D[F][R][y], R<y<=F
    vector<size_t> base(n+2,0);
    size_t tot=0;
    for(int F=0;F<=n;F++){ base[F]=tot; tot+=(size_t)F*(F+1)/2; }
    vector<u32> D(tot+8,0);
    auto P=[&](int F,int R)->u32*{
        return D.data()+base[F]+(size_t)R*F-(size_t)R*(R-1)/2-R-1;
    };
    u64 ans=0;
    for(int z=1;z<=n;z++){
        int az=a[z], bz=b[z];
        const u32* wz=&wT[(size_t)z*W];
        { u32* p=P(bz,0); p[z]=(p[z]+1)%MOD; }
        for(int F=z;F<=n;F++){
            int Fp=max(F,bz);
            u32* tgt0=P(Fp,0);
            // R=0
            const u32* src0=P(F,0);
            u64 acc0=0;
            for(int y=1;y<z;y++){
                u64 val=(u64)wz[y]*src0[y]%MOD;
                if(!val) continue;
                if(az<=y) acc0+=val;
                else { u32* t=P(Fp,y); t[z]=(t[z]+val)%MOD; }
            }
            for(int R=1;R<=z-2;R++){
                const u32* src=P(F,R);
                u32 d=dotp(src+R+1,wz+R+1,z-R-1);
                if(!d) continue;
                if(az<=R) acc0+=d;
                else { u32* t=P(Fp,R); t[z]=(t[z]+d)%MOD; }
            }
            tgt0[z]=(u32)((tgt0[z]+acc0)%MOD);
        }
    }
    for(int F=1;F<=n;F++){
        u32* p=P(F,0);
        for(int y=1;y<=F;y++) ans+=p[y];
    }
    printf("%llu\n",ans%MOD);
}
