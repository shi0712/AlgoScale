// C++17 implementation of the algorithm discussed at QOJ discussion 2639.
// Same polynomial/diagonal recurrence; no answer tables.
#include <bits/stdc++.h>
using namespace std;
constexpr int MOD=998244353;
struct Mint {
    int v;
    Mint(long long x=0){x%=MOD;if(x<0)x+=MOD;v=int(x);}
    Mint& operator+=(Mint b){v+=b.v;if(v>=MOD)v-=MOD;return *this;}
    Mint& operator-=(Mint b){v-=b.v;if(v<0)v+=MOD;return *this;}
    Mint& operator*=(Mint b){v=int(1LL*v*b.v%MOD);return *this;}
    friend Mint operator+(Mint a,Mint b){return a+=b;}
    friend Mint operator-(Mint a,Mint b){return a-=b;}
    friend Mint operator*(Mint a,Mint b){return a*=b;}
    Mint pow(int e)const{Mint a=*this,r=1;for(;e;e>>=1,a*=a)if(e&1)r*=a;return r;}
};
void radix(vector<int>& a){
    vector<int>b(a.size());
    for(int shift:{0,9}){
        array<int,512>cnt{};
        for(int x:a)++cnt[(x>>shift)&511];
        int s=0;
        for(int&c:cnt){int z=c;c=s;s+=z;}
        for(int x:a)b[cnt[(x>>shift)&511]++]=x;
        a.swap(b);
    }
}
array<int,3> solve(int n,int m,int k,vector<int> pieces){
    radix(pieces);
    int K=k+1,lim=max(n+k,m);
    vector<Mint> fac(lim+1),ifac(lim+1),inv(lim+1);
    fac[0]=1;
    for(int i=1;i<=lim;i++)fac[i]=fac[i-1]*i;
    ifac[lim]=fac[lim].pow(MOD-2);
    for(int i=lim;i>0;i--)ifac[i-1]=ifac[i]*i;
    for(int i=1;i<=lim;i++)inv[i]=fac[i-1]*ifac[i];
    Mint inverse_total=fac[k]*ifac[n+k]*ifac[m];
    vector<Mint>f(K+2),prev(K+2),nxt(K+2);f[K]=1;
    int A=0,B=0;
    for(int at=int(pieces.size())-1;at>=0;){
        int value=pieces[at]>>1,p=0,q=0;
        while(at>=0&&(pieces[at]>>1)==value){if(pieces[at]&1)++q;else ++p;--at;}
        inverse_total*=fac[p]*fac[q];
        int a=A+K,b=B+1,u=max(p-q,0),v=max(q-p,0);
        for(int t=0;t<u;t++)for(int i=0;i<=K;i++)f[i]*=Mint(a+t-i)*inv[t+1];
        for(int t=0;t<v;t++)for(int i=0;i<=K;i++)
            f[i]=(Mint(b+t-i)*f[i]+Mint(i+1)*f[i+1])*inv[t+1];
        fill(prev.begin(),prev.end(),Mint(0));
        int alpha=a+p-q;
        for(int t=0;t<min(p,q);t++){
            ++u;++v;
            long long C=1LL*(a+u-1)*(b+v-1)+1LL*(u-1)*(v-1);
            long long D=1LL*(a+u-2)*(b+v-2);
            Mint scale=inv[u]*inv[v];
            for(int i=0;i<=K;i++){
                long long diagonal=C-1LL*i*(a+b-1-i);
                long long upper=1LL*(i+1)*(alpha-i-1);
                nxt[i]=(Mint(diagonal)*f[i]+Mint(upper)*f[i+1]-Mint(D)*prev[i])*scale;
            }
            prev.swap(f);f.swap(nxt);
        }
        A+=p;B+=q;
    }
    Mint y=f[0]*inverse_total,draw=f[1]*inv[K]*inverse_total;
    return {(Mint(1)-y-draw).v,y.v,draw.v};
}
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int cases=1;
#ifdef BATCH_TEST
    cin>>cases;
#endif
    while(cases--){
        int n,m,k;if(!(cin>>n>>m>>k))return 1;
        vector<int>v;v.reserve(n+m);
        for(int i=0,x;i<n;i++){cin>>x;v.push_back(x<<1);}
        for(int i=0,x;i<m;i++){cin>>x;v.push_back((x<<1)|1);}
        auto ans=solve(n,m,k,move(v));
        for(int x:ans)cout<<x<<'\n';
    }
    return 0;
}
