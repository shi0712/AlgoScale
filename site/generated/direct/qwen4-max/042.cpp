#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64; typedef unsigned int u32;
static const u64 MOD = 998244353;
static const int MAXL = 1000050;
static u32 fct[MAXL], ifct[MAXL];
static u64 bufA[12][12], bufB[12][12];
static u64 pw(u64 b,u64 e){ b%=MOD; u64 r=1; while(e){ if(e&1) r=r*b%MOD; b=b*b%MOD; e>>=1;} return r; }
int main(){
    int n,k; if(scanf("%d %d",&n,&k)!=2) return 0;
    int LIM=k+n+2;
    fct[0]=1;
    for(int i=1;i<=LIM;i++) fct[i]=(u32)((u64)fct[i-1]*i%MOD);
    ifct[LIM]=(u32)pw(fct[LIM],MOD-2);
    for(int i=LIM;i>=1;i--) ifct[i-1]=(u32)((u64)ifct[i]*i%MOD);
    u64 C[12][12]; memset(C,0,sizeof C);
    for(int i=0;i<=n;i++){ C[i][0]=1; for(int j=1;j<=i;j++) C[i][j]=(C[i-1][j-1]+C[i-1][j])%MOD; }
    u64 ans=0;
    for(int a0=1;a0<=n&&a0<=k;a0++){
        u64 T;
        if(a0==1){ T=(u64)k%MOD*pw(n,k-1)%MOD; }
        else{
            const int b=n-a0; const int M0=k/a0, r=k-a0*M0;
            u64 bp[12]; bp[0]=1;
            for(int i=1;i<=a0;i++) bp[i]=bp[i-1]*(u64)b%MOD;
            const u64 bpa0=bp[a0];
            u64 (*cur)[12]=bufA; u64 (*prv)[12]=bufB;
            memset(bufB,0,sizeof bufB); T=0;
            {   // m = M0+1 (only high offsets can be non-zero)
                const int m=M0+1, E=k-a0*m, a0m=a0*m;
                for(int i=0;i<=a0;i++){ int e=E+i; cur[0][i]=(e>=0)?bp[e]:0ULL; }
                for(int a=1;a<=a0;a++){
                    const int j=a0-a, L=k-j*m;
                    cur[a][0]=0;
                    const u64 ca=(u64)a*ifct[m-1]%MOD, ab=(u64)(a+b);
                    for(int i=1;i<=j;i++){
                        if(a0m<=k+i){
                            const int Lp=L+i-1;
                            u64 t=ca*fct[Lp]%MOD*ifct[Lp-m+1]%MOD*cur[a-1][i]%MOD;
                            cur[a][i]=(ab*cur[a][i-1]+t)%MOD;
                        } else cur[a][i]=0;
                    }
                }
                u64 (*tmp)[12]=cur; cur=prv; prv=tmp;
            }
            u64 pwr=bp[r];
            for(int m=M0;m>=1;m--){
                for(int i=0;i<=a0;i++) cur[0][i]=pwr*bp[i]%MOD;
                const u64 ifm=ifct[m], ifm1=ifct[m-1];
                for(int a=1;a<=a0;a++){
                    const int j=a0-a, L=k-j*m;
                    u64 s=0,p=1;
                    for(int l=1;l<=a;l++){
                        p=p*ifm%MOD;
                        u64 t=C[a][l]*p%MOD*ifct[L-l*m]%MOD*cur[a-l][0]%MOD;
                        if(l&1) s+=t; else s+=MOD-t;
                    }
                    s%=MOD;
                    cur[a][0]=(prv[a][j]+(u64)fct[L]*s)%MOD;
                    const u64 ca=(u64)a*ifm1%MOD, ab=(u64)(a+b);
                    for(int i=1;i<=j;i++){
                        const int Lp=L+i-1;
                        u64 t=ca*fct[Lp]%MOD*ifct[Lp-m+1]%MOD*cur[a-1][i]%MOD;
                        cur[a][i]=(ab*cur[a][i-1]+t)%MOD;
                    }
                }
                T+=cur[a0][0]; if(T>=MOD)T-=MOD;
                u64 (*tmp)[12]=cur; cur=prv; prv=tmp;
                pwr=pwr*bpa0%MOD;
            }
        }
        u64 term=C[n][a0]*T%MOD;
        if(a0&1) ans=(ans+term)%MOD; else ans=(ans+MOD-term)%MOD;
    }
    printf("%llu\n",ans);
    return 0;
}
