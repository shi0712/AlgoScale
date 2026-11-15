#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64; typedef unsigned int u32;
static const u32 P=998244353;
static const int M=1000, S=M+2;
static u32 cur_[(M+1)*S], nw_[(M+1)*S];
static u32 Ftab[(M+1)][(M+1)];
static u32 C[(M+1)][(M+2)];
u64 pw(u64 a,u64 e){u64 r=1;a%=P;while(e){if(e&1)r=r*a%P;a=a*a%P;e>>=1;}return r;}
int main(){
    static u32 inv[M+3], ifact[M+3];
    for(int i=1;i<M+3;i++) inv[i]=pw(i,P-2);
    ifact[0]=1; for(int i=1;i<M+3;i++) ifact[i]=(u64)ifact[i-1]*inv[i]%P;
    u32 *cur=cur_,*nw=nw_;
    for(int i=0;i<=M;i++) cur[i]=1;
    for(int t=1;t<=M;t++) for(int i=0;i<=M;i++) cur[t*S+i]=(u64)cur[(t-1)*S+i]*i%P;
    for(int n=0;n<=M;n++) Ftab[n][0]=cur[n*S];
    static u64 Tp[M+2];
    for(int m=1;m<=M;m++){
        int imax=M-m;
        for(int i=0;i<=imax;i++){Tp[i]=0; nw[i]=cur[i+1];}
        for(int t=1;t<=M;t++){
            const u32* prev=cur+(size_t)(t-1)*S; const u32* row=cur+(size_t)t*S; u32* out=nw+(size_t)t*S;
            u64 h0=prev[0]+P;
            for(int i=0;i<=imax;i++){
                u64 d=h0-prev[i+1];
                u64 T=((u64)inv[i+1]*d+(u64)i*Tp[i])%P;
                Tp[i]=T;
                u32 v=row[i+1]+(u32)T; if(v>=P) v-=P;
                out[i]=v;
            }
        }
        swap(cur,nw);
        for(int n=m;n<=M;n++) Ftab[n][m]=cur[(size_t)n*S];
    }
    for(int n=1;n<=M;n++){
        static u32 a[M+2];
        for(int j=0;j<=n;j++) a[j]=Ftab[n][j];
        for(int k=1;k<=n;k++) for(int j=n;j>=k;j--){ u32 v=a[j]+P-a[j-1]; if(v>=P) v-=P; a[j]=v; }
        for(int k=0;k<=n;k++) C[n][k]=(u64)a[k]*ifact[k]%P;
    }
    size_t cap=1<<25;
    static char buf[1<<25];
    size_t len=fread(buf,1,cap-1,stdin); buf[len]=0;
    char*p=buf;
    auto rd=[&]()->long long{ while(*p&&(*p<'0'||*p>'9'))p++; long long x=0; while(*p>='0'&&*p<='9'){x=x*10+(*p-'0');p++;} return x;};
    int T=rd();
    vector<int> qn(T); vector<u32> qm(T);
    vector<int> cnt(M+2,0);
    for(int i=0;i<T;i++){qn[i]=rd(); qm[i]=rd()%P; cnt[qn[i]+1]++;}
    for(int i=1;i<=M+1;i++) cnt[i]+=cnt[i-1];
    vector<int> ord(T); { vector<int> pos(cnt.begin(),cnt.end()); for(int i=0;i<T;i++) ord[pos[qn[i]]++]=i; }
    vector<u32> ans(T);
    for(int n=1;n<=M;n++){
        int lo=cnt[n],hi=cnt[n+1];
        const u32* c=C[n];
        int q=lo;
        const int B=8;
        for(;q+B<=hi;q+=B){
            u64 res[B]; u32 t[B];
            for(int b=0;b<B;b++){ res[b]=c[n]; u32 mm=qm[ord[q+b]]; u32 v=mm+P-(n-1); if(v>=P) v-=P; t[b]=v; }
            for(int k=n-1;k>=0;k--){
                u64 ck=c[k];
                for(int b=0;b<B;b++){
                    res[b]=(ck+res[b]*t[b])%P;
                    u32 v=t[b]+1; t[b]= v==P?0:v;
                }
            }
            for(int b=0;b<B;b++) ans[ord[q+b]]=res[b];
        }
        for(;q<hi;q++){
            u64 mm=qm[ord[q]]; u64 res=c[n];
            for(int k=n-1;k>=0;k--){ u64 t=(mm+P-k)%P; res=(c[k]+res*t)%P; }
            ans[ord[q]]=res;
        }
    }
    string out; out.reserve((size_t)T*11);
    char tmp[16];
    for(int i=0;i<T;i++){ u32 v=ans[i]; int l=0; if(v==0) tmp[l++]='0'; while(v){tmp[l++]='0'+v%10; v/=10;} while(l) out.push_back(tmp[--l]); out.push_back('\n'); }
    fwrite(out.data(),1,out.size(),stdout);
}
