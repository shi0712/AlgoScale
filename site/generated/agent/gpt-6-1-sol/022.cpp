#include <bits/stdc++.h>
using namespace std;
static constexpr int MOD=998244353;
inline int mul(int a,int b){return (long long)a*b%MOD;}
inline int add(int a,int b){int c=a+b;return c>=MOD?c-MOD:c;}
inline int modnorm(long long a){a%=MOD;if(a<0)a+=MOD;return (int)a;}
vector<int> fac,ifac,invs;
inline int choose(int n,int r){
    if(r<0)return 0;
    if(n>=0){if(n<r)return 0;return mul(fac[n],mul(ifac[r],ifac[n-r]));}
    int v=mul(fac[r-n-1],mul(ifac[r],ifac[-n-1]));
    return (r&1)&&v?MOD-v:v;
}
inline int sinv(int v){return v<0?MOD-invs[-v]:invs[v];}
struct Group{int a,b;};

/*
Ordinary words use one commuting variable per strength, and z marks bombs.
Let D be the generating function of draws without bombs and put
F = (1-Y) * (generating function of Y wins without bombs), where Y is
the sum of Y's variables. Splitting a play at mutual removals gives
  draws with bombs = D/(1-zF),   Y wins with bombs = F/((1-Y)(1-zF)).

On adding the next largest strength, with variables x,y, and P=1-X_low,
Q=1-Y_low, the ordinary functions satisfy
  D_new = PQ*D_low/(PQ-xy),
  F_new = (P*(Q-y)*F_low+y)/(PQ-xy).

We extract [ordinary counts] F^(k+1)/(1-Y), descending by strength.
At each step dp[h] multiplies F_low^h/(1-X_low)^(P0-h)/(1-Y_low)^q.
For counts a,b, h -> h-t has multiplier binom(h,t)*S_t, where
 S_t = sum_r binom(h+r-1,r) binom(P0-h+a-r-1,a-r)
                         * binom(q-h+b-r-1,b-t-r).
The binomial sum satisfies Hahn's second order difference equation in t.
Two direct sums plus this recurrence cost O(a+b) per h; at singular
coefficients we evaluate a direct sum again (at most twice).

The same extraction for D*F^k has state h-1 and multipliers binom(h-1,t).
Their ratio telescopes: its dp[h-1] equals h/(k+1) times this dp[h].
Consequently dp[0] counts Y wins, and dp[1]/(k+1) counts draws.
Time O(100000+(n+m)(k+1)), memory O(100000+n+m+k).
*/
pair<int,int> coefficient(const vector<Group>& groups,int K){
    const int d=0,p0=0,q0=1;
    vector<int> dp(K+1),ndp(K+1),iden(K+1);
    dp[K]=1;
    int P0=p0+K,q=q0;
    for(auto [a,b]:groups){
        if(!b){
            for(int h=0;h<=K;h++)if(dp[h])
                dp[h]=mul(dp[h],choose(P0-h+a-1,a));
            P0+=a;
            continue;
        }
        fill(ndp.begin(),ndp.end(),0);
        if(a){
            for(int j=0;j<=K;j++){
                int u=1-P0-q-d+j,v=q-j;
                iden[j]=(u&&v)?mul(sinv(u),sinv(v)):0;
            }
        }
        for(int h=0;h<=K;h++)if(dp[h]){
            int p=P0-h,lim=min(h,b);
            auto direct=[&](int t)->int{
                int j=h-t,lo=0,hi=min(a,b-t);
                if(p==0)lo=max(lo,a);
                if(h+d==0)hi=min(hi,0);
                if(q-j<=0)lo=max(lo,b-h+q);
                int ans=0;
                for(int r=lo;r<=hi;r++){
                    int v=mul(choose(h+d+r-1,r),choose(p+a-r-1,a-r));
                    v=mul(v,choose(q-h+b-r-1,b-t-r));
                    ans=add(ans,v);
                }
                return ans;
            };
            int small[4];
            bool tiny=(a==2 && (b==2 || b==3));
            if(tiny){
                int H=modnorm((long long)q-h),H1=add(H,1),H2=add(H,2);
                int v0=mul(mul(p,p+1),invs[2]),v1=mul(h,p);
                int v2=mul(mul(h,h+1),invs[2]);
                int c2=mul(mul(H,H1),invs[2]);
                if(b==2){
                    small[0]=add(add(mul(v0,c2),mul(v1,H)),v2);
                    small[1]=add(mul(v0,H1),v1);small[2]=v0;
                }else{
                    int c3=mul(mul(c2,H2),invs[3]);
                    small[0]=add(add(mul(v0,c3),mul(v1,c2)),mul(v2,H));
                    small[1]=add(add(mul(v0,mul(mul(H1,H2),invs[2])),mul(v1,H1)),v2);
                    small[2]=add(mul(v0,H2),v1);small[3]=v0;
                }
            }
            int prev=0,cur=0,ch=1;
            for(int t=0;t<=lim;t++){
                int s;
                if(tiny)s=small[t];
                else if(!a)s=choose(q-h+b-1,b-t);
                else if(a==1){
                    s=add(mul(p,choose(q-h+b-1,b-t)),
                          mul(h,choose(q-h+b-2,b-t-1)));
                }
                else if(b==1){
                    int v=choose(p+a-1,a);
                    if(t)s=v;
                    else s=add(mul(v,modnorm(q-h)),mul(h,choose(p+a-2,a-1)));
                }
                else if(t<2 || !iden[h-t+1])s=direct(t);
                else{
                    // Hahn's difference equation for the binomial sum.
                    int tt=t-1,x=b-tt,H=q-h;
                    long long u=x-p-a+1;
                    long long v=1-P0-d-H-tt;
                    int c1=modnorm(u*(1-H-tt)+(long long)x*v+(long long)a*(h+d));
                    int c2=modnorm(u*(x+1LL));
                    s=mul(add(mul(c1,cur),mul(c2,prev)),iden[h-tt]);
                }
                int j=h-t;
                ndp[j]=add(ndp[j],mul(dp[h],mul(ch,s)));
                prev=cur;cur=s;
                if(t<lim)ch=mul(mul(ch,h-t),invs[t+1]);
            }
        }
        dp.swap(ndp);
        P0+=a;q+=b;
    }
    return {dp[0],mul(dp[1],invs[K])};
}
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int n,m,k;if(!(cin>>n>>m>>k))return 0;
    const int V=100000;
    vector<int>a(V+1),b(V+1);
    for(int i=0,x;i<n;i++){cin>>x;a[x]++;}
    for(int i=0,x;i<m;i++){cin>>x;b[x]++;}
    if(k>m){cout<<1<<'\n'<<0<<'\n'<<0<<'\n';return 0;}
    int sz=n+m+k+10;
    fac.resize(sz);ifac.resize(sz);invs.resize(sz);
    fac[0]=ifac[0]=1;invs[1]=1;
    for(int i=1;i<sz;i++){
        if(i>1)invs[i]=MOD-mul(MOD/i,invs[MOD%i]);
        fac[i]=mul(fac[i-1],i);ifac[i]=mul(ifac[i-1],invs[i]);
    }
    vector<Group>groups;
    int scale=mul(fac[k],mul(ifac[n+k],ifac[m]));
    for(int v=V;v;v--){
        scale=mul(scale,mul(fac[a[v]],fac[b[v]]));
        if(a[v]||b[v])groups.push_back({a[v],b[v]});
    }
    auto [yc,dc]=coefficient(groups,k+1);
    int draw=mul(dc,scale);
    int yw=mul(yc,scale);
    int xw=modnorm(1LL-yw-draw);
    cout<<xw<<'\n'<<yw<<'\n'<<draw<<'\n';
    return 0;
}
