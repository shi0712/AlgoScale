#include <cstdio>
#include <cstring>

static const int MOD = 998244353;
static const int LIM = 1000000;
static const int MAXN = LIM + 6;

static int par[MAXN], ht[MAXN], offa[MAXN + 1], Fv[MAXN];
static int dn[2 * MAXN], fv[2 * MAXN];
static int fact[LIM + 1], invfact[LIM + 1], binom[LIM + 1];

static char ibuf[1 << 16]; static int ipos = 0, ilen = 0;
static inline int gc(){ if(ipos==ilen){ ilen=(int)fread(ibuf,1,sizeof(ibuf),stdin); ipos=0; if(ilen<=0) return -1;} return (unsigned char)ibuf[ipos++]; }
static inline int readInt(){ int c=gc(); while(c!=-1&&(c<'0'||c>'9')) c=gc(); int x=0; while(c>='0'&&c<='9'){ x=x*10+(c-'0'); c=gc(); } return x; }
static long long pw(long long b,long long e){ long long r=1; while(e){ if(e&1) r=r*b%MOD; b=b*b%MOD; e>>=1;} return r; }
static char obuf[1 << 16]; static int opos = 0;
static inline void writeInt(long long x){ if(opos>(1<<16)-32){ fwrite(obuf,1,opos,stdout); opos=0;} char tmp[24]; int len=0; if(!x) tmp[len++]='0'; while(x){ tmp[len++]=char('0'+x%10); x/=10;} while(len) obuf[opos++]=tmp[--len]; obuf[opos++]='\n'; }

int main(){
    fact[0]=1;
    for(int i=1;i<=LIM;i++) fact[i]=(int)((long long)fact[i-1]*i%MOD);
    invfact[LIM]=(int)pw(fact[LIM],MOD-2);
    for(int i=LIM;i>=1;i--) invfact[i-1]=(int)((long long)invfact[i]*i%MOD);
    int t=readInt();
    while(t-->0){
        int n=readInt(), k=readInt();
        for(int i=2;i<=n;i++) par[i]=readInt();
        if(k==1){ writeInt(n%MOD); continue; }
        if(k==2){ writeInt((long long)n*(n-1)%MOD); continue; }
        int r=k-1;
        for(int m=0;m<r;m++) binom[m]=0;
        long long ir=invfact[r];
        for(int m=r;m<=n;m++) binom[m]=(int)((long long)fact[m]*ir%MOD*invfact[m-r]%MOD);
        for(int i=1;i<=n;i++) ht[i]=0;
        for(int v=n;v>=2;v--){ int p=par[v]; int c=ht[v]+1; if(c>ht[p]) ht[p]=c; }
        offa[1]=0;
        for(int v=1;v<=n;v++) offa[v+1]=offa[v]+ht[v]+1;
        int total=offa[n+1];
        memset(dn,0,sizeof(int)*(size_t)total);
        for(int v=1;v<=n;v++) dn[offa[v]]=1;
        for(int v=n;v>=2;v--){
            int p=par[v], bv=offa[v], bp=offa[p], hv=ht[v];
            for(int j=0;j<=hv;j++) dn[bp+j+1]+=dn[bv+j];
        }
        long long ans=0;
        {
            int b=offa[1], h1=ht[1]; long long s=0;
            for(int d=0;d<=h1;d++) fv[b+d]=dn[b+d];
            for(int d=1;d<=h1;d++) s+=binom[fv[b+d]];
            s%=MOD; Fv[1]=(int)s; ans=s;
        }
        int b1=binom[1];
        for(int v=2;v<=n;v++){
            int p=par[v], bv=offa[v], bp=offa[p], hv=ht[v];
            long long s;
            fv[bv]=1;
            if(hv==0){
                int fp1=fv[bp+1];
                s=(long long)Fv[p]+b1-binom[fp1]+binom[fp1-1];
                s%=MOD; if(s<0) s+=MOD;
            } else {
                s=Fv[p];
                int x=dn[bv+1]+1; fv[bv+1]=x;
                s+=binom[x]-binom[fv[bp+1]];
                for(int d=2;d<=hv;d++){
                    int y=dn[bv+d]+fv[bp+d-1]-dn[bv+d-2];
                    fv[bv+d]=y;
                    s+=binom[y]-binom[fv[bp+d]];
                }
                int a=fv[bp+hv], bb=fv[bp+hv+1];
                int m1=a-dn[bv+hv-1];
                int m2=bb-dn[bv+hv];
                s+=binom[m1]-binom[bb]+binom[m2];
                s%=MOD; if(s<0) s+=MOD;
            }
            Fv[v]=(int)s;
            ans+=s;
        }
        writeInt(ans%MOD);
    }
    fwrite(obuf,1,opos,stdout);
    return 0;
}
