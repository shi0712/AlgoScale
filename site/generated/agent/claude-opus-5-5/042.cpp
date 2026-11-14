#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull;
const ll MOD=998244353;
ll pw(ll b,ll e){b%=MOD;ll r=1;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
int main(){
    ll n,k; cin>>n>>k;
    // lines (a,b): value G(n-a, k-a*t-b, t) = [x^M] e_t(x)^{n-a}
    map<pair<int,int>,int> id; vector<pair<int,int>> L;
    auto get=[&](int a,int b){auto key=make_pair(a,b);auto it=id.find(key);if(it!=id.end())return it->second;int x=L.size();id[key]=x;L.push_back(key);return x;};
    get(0,0);
    for(size_t q=0;q<L.size();q++){
        auto [a,b]=L[q]; if(a==n) continue; int j=n-a;
        for(int s=1;s<=j;s++) get(a+s,b+a+s);
        for(int u=0;u<a;u++) get(a+1,b+u+1);
    }
    int C=L.size();
    vector<vector<int>> bdep(C), mdep(C);
    for(int q=0;q<C;q++){auto [a,b]=L[q]; if(a==n) continue; int j=n-a;
        for(int s=1;s<=j;s++) bdep[q].push_back(id[{a+s,b+a+s}]);
        for(int u=0;u<a;u++) mdep[q].push_back(id[{a+1,b+u+1}]);}
    int N=max<ll>(k+2,20);
    vector<ll> fact(N+1),ifact(N+1),inv(N+1);
    fact[0]=1;for(int i=1;i<=N;i++)fact[i]=fact[i-1]*i%MOD;
    ifact[N]=pw(fact[N],MOD-2);for(int i=N;i>0;i--)ifact[i-1]=ifact[i]*i%MOD;
    for(int i=1;i<=N;i++)inv[i]=ifact[i]*fact[i-1]%MOD;
    ll binom[12][12]={};for(int i=0;i<12;i++){binom[i][0]=1;for(int j2=1;j2<=i;j2++)binom[i][j2]=(binom[i-1][j2-1]+(j2<=i-1?binom[i-1][j2]:0))%MOD;}
    vector<ll> cur(C),nxt(C);
    vector<int> la(C),lb(C);for(int q=0;q<C;q++){la[q]=L[q].first;lb[q]=L[q].second;}
    // t=0
    for(int q=0;q<C;q++) cur[q]=(k-lb[q]==0)?1:0;
    ll nk=pw(n,k), ans=0;
    for(ll t=0;t<k;t++){
        // values for closed-form lines (a==n) at t
        for(int q=0;q<C;q++) if(la[q]==n) cur[q]=(k-n*t-lb[q]==0)?1:0;
        ans=(ans+nk-fact[k]*cur[0]%MOD+MOD)%MOD;
        if(t==k-1)break;
        ll ift=ifact[t], ift1=ifact[t+1];
        ll ipw[12];ipw[0]=1;for(int s=1;s<12;s++)ipw[s]=ipw[s-1]*ift1%MOD;
        for(int q=0;q<C;q++){
            int a=la[q]; if(a==n) continue;
            ll j=n-a; ll M1=k-a*(t+1)-lb[q];
            if(M1<0||M1>j*(t+1)){nxt[q]=0;continue;}
            ll M=k-a*t-lb[q];
            ll val=cur[q];
            ll jf=j*ift%MOD, ij=inv[j];
            for(int u=0;u<a;u++){
                ll m=M-u;
                if(m<=0){val=0;break;}
                val=((m%MOD)*val+jf*cur[mdep[q][u]])%MOD*ij%MOD;
            }
            for(int s=1;s<=j;s++){
                val=(val+binom[j][s]*ipw[s]%MOD*cur[bdep[q][s-1]])%MOD;
            }
            nxt[q]=val;
        }
        for(int q=0;q<C;q++) if(la[q]!=n) cur[q]=nxt[q];
    }
    cout<<ans%MOD<<endl;
}
