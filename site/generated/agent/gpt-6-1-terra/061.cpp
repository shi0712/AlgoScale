#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr int MOD = 998244353;
int add(int a,int b){int s=a+b; if(s>=MOD)s-=MOD; return s;}
int mul(int a,int b){return (ll)a*b%MOD;}
int power(int a,int b){int r=1; for(;b;b>>=1,a=mul(a,a)) if(b&1) r=mul(r,a); return r;}
int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    constexpr int L=4000;
    vector<int> fact(L+1),invfact(L+1),inv(L+1),pow2(L+1),invpow4(L+1);
    fact[0]=pow2[0]=invpow4[0]=1;
    int inv4=power(4,MOD-2);
    for(int i=1;i<=L;i++){fact[i]=mul(fact[i-1],i); pow2[i]=add(pow2[i-1],pow2[i-1]); invpow4[i]=mul(invpow4[i-1],inv4);}
    invfact[L]=power(fact[L],MOD-2);
    for(int i=L;i>=1;i--) invfact[i-1]=mul(invfact[i],i);
    inv[1]=1; for(int i=2;i<=L;i++) inv[i]=MOD-mul(MOD/i,inv[MOD%i]);
    int T; cin>>T;
    while(T--){
        int n,m; cin>>n>>m; int N=2*m;
        vector<int> a(n); for(int &x:a)cin>>x;
        if(n==1){cout<<0<<'\n'; continue;}
        // s[k] is the probability that k distinct ancestors all belonged
        // to one initial set, summed over the possible initial sets.
        vector<int> s(m+1);
        for(int x:a){
            int ratio=1;
            for(int k=1;k<=m && k<=x;k++){
                ratio=mul(ratio,mul(x-k+1,inv[N-k+1]));
                s[k]=add(s[k],ratio);
            }
        }
        // The first backward step takes N lineages to exactly m.
        int answer=1;
        vector<int> incoming(m+1); incoming[m]=1;
        for(int k=m;k>=2;k--){
            // Matching k marked elements creates j internal pairs with
            // probability pref / ((k-2j)! j! (m-k+j)! 4^j).
            int pref=mul(mul(mul(fact[k],fact[N-k]),mul(fact[m],invfact[N])),pow2[k]);
            int p0=mul(pref,mul(invfact[k],invfact[m-k]));
            int wk=mul(incoming[k],power((1-p0+MOD)%MOD,MOD-2));
            int contribution=(1-s[k])%MOD; if(contribution<0)contribution+=MOD;
            answer=add(answer,mul(wk,contribution));
            for(int j=1;j<=k/2;j++){
                int p=mul(pref,mul(mul(invfact[k-2*j],invfact[j]),mul(invfact[m-k+j],invpow4[j])));
                incoming[k-j]=add(incoming[k-j],mul(wk,p));
            }
        }
        cout<<answer<<'\n';
    }
}
