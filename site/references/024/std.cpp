// QOJ 7875 / Hefei 2023 B. O(n + M log(M+1)), M = sum(a).
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const int MOD = 998244353;
using Poly = vector<int>;
int power(int a, int b) { int r=1; for(;b;b>>=1,a=(ll)a*a%MOD) if(b&1) r=(ll)r*a%MOD; return r; }
void ntt(Poly& a, bool inverse) {
    int n=a.size();
    for(int i=1,j=0;i<n;++i){int b=n>>1;for(;j&b;b>>=1)j^=b;j^=b;if(i<j)swap(a[i],a[j]);}
    for(int len=2;len<=n;len<<=1){
        int w=power(3,(MOD-1)/len);if(inverse)w=power(w,MOD-2);
        for(int i=0;i<n;i+=len){int x=1;for(int j=0;j<len/2;++j){
            int u=a[i+j],v=(ll)a[i+j+len/2]*x%MOD;
            a[i+j]=u+v>=MOD?u+v-MOD:u+v;
            a[i+j+len/2]=u-v<0?u-v+MOD:u-v;x=(ll)x*w%MOD;
        }}
    }
    if(inverse){int z=power(n,MOD-2);for(int&x:a)x=(ll)x*z%MOD;}
}
Poly mul(Poly a, Poly b, int cap) {
    if(a.empty()||b.empty()||!cap)return {};
    int need=min(cap,(int)a.size()+(int)b.size()-1);
    if(min(a.size(),b.size())<=24){Poly c(need);for(int i=0;i<(int)a.size()&&i<need;++i)
        for(int j=0;j<(int)b.size()&&i+j<need;++j)c[i+j]=(c[i+j]+(ll)a[i]*b[j])%MOD;return c;}
    int n=1;while(n<(int)a.size()+(int)b.size()-1)n<<=1;
    assert(n<=(1<<23));a.resize(n);b.resize(n);ntt(a,false);ntt(b,false);
    for(int i=0;i<n;++i)a[i]=(ll)a[i]*b[i]%MOD;ntt(a,true);a.resize(need);return a;
}
Poly inv_series(const Poly& a,int n){
    Poly b{power(a[0],MOD-2)};
    while((int)b.size()<n){int k=min(n,2*(int)b.size());Poly c(a.begin(),a.begin()+min(k,(int)a.size()));
        c=mul(c,b,k);c.resize(k);for(int&x:c)x=x?MOD-x:0;c[0]=(c[0]+2)%MOD;
        b=mul(b,c,k);b.resize(k);
    }return b;
}
Poly exp_series(const Poly& h,const Poly& inv){
    int n=h.size();Poly f{1};
    while((int)f.size()<n){int s=f.size(),k=min(n,2*s);Poly df(max(0,s-1));
        for(int j=1;j<s;++j)df[j-1]=(ll)f[j]*j%MOD;
        Poly dl=mul(df,inv_series(f,k),k-1);dl.resize(k-1);Poly correction(k-s);
        for(int j=s;j<k;++j)correction[j-s]=(h[j]-(ll)dl[j-1]*inv[j]%MOD+MOD)%MOD;
        correction=mul(f,correction,k-s);correction.resize(k-s);f.resize(k);
        for(int j=s;j<k;++j)f[j]=correction[j-s];
    }return f;
}
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int n;if(!(cin>>n))return 0;vector<int>a(n);int m=0,r=0;
    for(int&x:a){cin>>x;m+=x;r+=x>0;}
    int k=m/2;Poly inv(m+2),fact(m+1,1),ifact(m+1,1),freq(k+1),h(k+1);
    inv[1]=1;for(int j=2;j<=m+1;++j)inv[j]=MOD-(ll)(MOD/j)*inv[MOD%j]%MOD;
    for(int j=1;j<=m;++j)fact[j]=(ll)fact[j-1]*j%MOD;
    ifact[m]=power(fact[m],MOD-2);for(int j=m;j;--j)ifact[j-1]=(ll)ifact[j]*j%MOD;
    for(int x:a)if(x>0&&x+1<=k)++freq[x+1];
    for(int j=1;j<=k;++j)h[j]=r;
    for(int d=2;d<=k;++d)if(freq[d])for(int j=d;j<=k;j+=d)h[j]=(h[j]-(ll)d*freq[d]%MOD+MOD)%MOD;
    for(int j=1;j<=k;++j)h[j]=(ll)h[j]*inv[j]%MOD;
    Poly c=exp_series(h,inv);int ans=0,previous_c=0,previous_choose=0;
    for(int j=0;j<=k;++j){int choose=(ll)fact[m]*ifact[j]%MOD*ifact[m-j]%MOD;
        ans=(ans+(ll)(c[j]-previous_c+MOD)*(choose-previous_choose+MOD))%MOD;
        previous_c=c[j];previous_choose=choose;
    }cout<<ans<<'\n';
}
