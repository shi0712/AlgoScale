#include <bits/stdc++.h>
using namespace std;
static const long long MOD=998244353;
static char buf[1<<25];int bl,bp;
inline int gc(){if(bp==bl){bl=fread(buf,1,sizeof buf,stdin);bp=0;if(bl<=0)return -1;}return buf[bp++];}
inline int rd(){int c=gc();while(c<'0')c=gc();int x=0;while(c>='0'){x=x*10+c-'0';c=gc();}return x;}
long long pw(long long a,long long e){long long r=1;a%=MOD;while(e){if(e&1)r=r*a%MOD;a=a*a%MOD;e>>=1;}return r;}
int main(){
    const int MX=1000001;
    vector<long long> fac(MX+1),inv(MX+1);
    fac[0]=1;for(int i=1;i<=MX;i++)fac[i]=fac[i-1]*i%MOD;
    inv[MX]=pw(fac[MX],MOD-2);for(int i=MX;i>0;i--)inv[i-1]=inv[i]*i%MOD;
    int t=rd();
    vector<int> par,h,deg;vector<long long> off,F;vector<int> dn,cn;
    string out;
    while(t--){
        int n=rd(),k=rd();
        par.assign(n+1,0);h.assign(n+1,0);
        for(int i=2;i<=n;i++)par[i]=rd();
        if(k==1){out+=to_string(n)+"\n";continue;}
        int r=k-1;
        auto g=[&](int x)->long long{ if(x<r) return 0; return fac[x]*inv[r]%MOD*inv[x-r]%MOD;};
        for(int i=n;i>=2;i--)h[par[i]]=max(h[par[i]],h[i]+1);
        off.assign(n+2,0);
        for(int v=1;v<=n;v++)off[v+1]=off[v]+h[v]+3;
        long long tot=off[n+1];
        dn.assign(tot,0);cn.assign(tot,0);
        for(int v=1;v<=n;v++)dn[off[v]]=1;
        for(int i=n;i>=2;i--){int p=par[i];int*a=&dn[off[i]];int*b=&dn[off[p]+1];for(int d=0;d<=h[i];d++)b[d]+=a[d];}
        F.assign(n+1,0);
        // root
        {int*c=&cn[off[1]];int*a=&dn[off[1]];for(int d=0;d<=h[1];d++)c[d]=a[d];long long s=0;for(int d=1;d<=h[1];d++)s+=g(c[d]);F[1]=s%MOD;}
        long long ans=F[1];
        for(int i=2;i<=n;i++){
            int p=par[i];int hi=h[i];int*c=&cn[off[i]];int*a=&dn[off[i]];int*cp=&cn[off[p]];
            c[0]=1;
            long long s=F[p];
            for(int d=1;d<=hi+2;d++){
                int v=(d<=hi?a[d]:0)+cp[d-1]-(d>=2&&d-2<=hi?a[d-2]:0);
                c[d]=v; s+=g(v);
            }
            for(int e=1;e<=hi+1;e++) s-=g(cp[e]);
            s%=MOD;if(s<0)s+=MOD;F[i]=s;ans+=s;
        }
        ans%=MOD;
        out+=to_string(ans)+"\n";
    }
    fwrite(out.data(),1,out.size(),stdout);
}
