#include <bits/stdc++.h>
using namespace std;using i64=long long;
constexpr int MOD=1000000007;
struct Sieve{
    vector<int> spf,mu,omega,D,M;
    static constexpr int SMALL=2048,WORDS=SMALL/64;
    using Bits=array<unsigned long long,WORDS>;
    vector<Bits> coprime_mask;
    vector<int> mask_index;
    Bits positive{},negative{};
    explicit Sieve(int n):spf(n+1),mu(n+1),omega(n+1),D(n+1),M(n+1){
        vector<int> primes,exponent(n+1),tau(n+1);mu[1]=tau[1]=1;
        for(int i=2;i<=n;++i){
            if(!spf[i]){spf[i]=i;primes.push_back(i);mu[i]=-1;tau[i]=2;exponent[i]=1;omega[i]=1;}
            for(int p:primes){if((i64)i*p>n)break;int x=i*p;spf[x]=p;
                if(i%p==0){mu[x]=0;omega[x]=omega[i];exponent[x]=exponent[i]+1;tau[x]=tau[i]/(exponent[i]+1)*(exponent[x]+1);break;}
                mu[x]=-mu[i];omega[x]=omega[i]+1;exponent[x]=1;tau[x]=tau[i]*2;
            }
        }
        for(int i=1;i<=n;++i){D[i]=(D[i-1]+tau[i])%MOD;M[i]=M[i-1]+mu[i];}
        // Constant-width bitsets for small coprime Mobius queries, all computed online.
        vector<Bits> prime_mask(SMALL+1);for(auto&a:prime_mask)a.fill(~0ULL);
        for(int p=2;p<=min(n,SMALL);++p)if(spf[p]==p)
            for(int t=p;t<=SMALL;t+=p)prime_mask[p][(t-1)/64]&=~(1ULL<<((t-1)%64));
        mask_index.resize(n+1);int mask_count=0;
        for(int m=1;m<=n;++m)if(mu[m])mask_index[m]=mask_count++;
        coprime_mask.resize(mask_count);coprime_mask[mask_index[1]].fill(~0ULL);
        for(int m=2;m<=n;++m)if(mu[m]){int p=spf[m];
            for(int w=0;w<WORDS;++w)coprime_mask[mask_index[m]][w]=coprime_mask[mask_index[m/p]][w]&(p<=SMALL?prime_mask[p][w]:~0ULL);
        }
        for(int t=1;t<=min(n,SMALL);++t){if(mu[t]>0)positive[(t-1)/64]|=1ULL<<((t-1)%64);if(mu[t]<0)negative[(t-1)/64]|=1ULL<<((t-1)%64);}
    }
    int small_mobius(int m,int l,int r)const{
        if(l>r)return 0;int ret=0;
        auto low=[](int k){return k==64?~0ULL:((1ULL<<k)-1);};
        for(int w=(l-1)/64;w<=(r-1)/64;++w){
            int a=max(0,l-1-w*64),b=min(64,r-w*64);
            auto mask=coprime_mask[mask_index[m]][w]&(low(b)^low(a));
            ret+=__builtin_popcountll(mask&positive[w])-__builtin_popcountll(mask&negative[w]);
        }return ret;
    }
};
struct Solver{
    const Sieve& sv;vector<int> factors,suffix;
    i64 mobius(int index,int l,int r)const{
        if(l==r)return 0;
        if(index==(int)factors.size()||factors[index]>r)return sv.M[r]-sv.M[l];
        if(r<=Sieve::SMALL)return sv.small_mobius(suffix[index],l+1,r);
        int p=factors[index];i64 ret=0;
        for(int power=1;;){ret+=mobius(index+1,l/power,r/power);if(power>r/p)break;power*=p;}
        return ret;
    }
    int solve(int A,int B,int C){
        array<int,3> lim{A,B,C};sort(lim.begin(),lim.end());A=lim[0];B=lim[1];C=lim[2];i64 answer=0;
        // These are divisor-prefix values at current input quotients, computed online.
        // They avoid repeated division in short intervals; they are not answer tables.
        vector<int> DA(C+1),DB(C+1),DC(C+1);
        for(int l=1;l<=C;){int qa=A/l,qb=B/l,qc=C/l,r=C;
            if(qa)r=min(r,A/qa);if(qb)r=min(r,B/qb);r=min(r,C/qc);
            fill(DA.begin()+l,DA.begin()+r+1,sv.D[qa]);
            fill(DB.begin()+l,DB.begin()+r+1,sv.D[qb]);
            fill(DC.begin()+l,DC.begin()+r+1,sv.D[qc]);l=r+1;
        }
        for(int d=1;d<=A;++d)if(sv.mu[d]){
            int weight=1<<sv.omega[d];answer=(answer+(i64)weight*sv.D[A/d]%MOD*sv.D[B/d]%MOD*sv.D[C/d])%MOD;
            for(int s=1;(i64)d*s*(s+1)<=C;++s){int ds=d*s;if(!sv.mu[ds])continue;
                for(int r=1;r<=s && (i64)ds*r<=A;++r){int m=ds*r;if(!sv.mu[m])continue;
                    int dr=d*r,U=min(B/dr,C/ds),L=s+1;if(L>U)continue;
                    int v[3]={DA[m],DB[m],DC[m]};
                    auto value=[&](int tr,int ts)->i64{
                        if(r==s)return ((i64)v[0]*DB[tr]%MOD*DC[tr]+(i64)v[1]*DA[tr]%MOD*DC[tr]+(i64)v[2]*DA[tr]%MOD*DB[tr])%MOD;
                        i64 x=((i64)DB[tr]*DC[ts]+(i64)DC[tr]*DB[ts])%MOD;
                        i64 y=((i64)DA[tr]*DC[ts]+(i64)DC[tr]*DA[ts])%MOD;
                        i64 z=((i64)DA[tr]*DB[ts]+(i64)DB[tr]*DA[ts])%MOD;
                        return (v[0]*x+v[1]*y+v[2]*z)%MOD;
                    };
                    int coeff=weight*sv.mu[r]*sv.mu[s];
                    if(U>Sieve::SMALL && U-L+1<=32){
                        for(int t=L;t<=U;++t)if(sv.mu[ds*t]&&sv.mu[r*t])
                            answer=(answer+(i64)coeff*sv.mu[t]*value(dr*t,ds*t))%MOD;
                        continue;
                    }
                    bool factored=false;
                    int qr[3]={A/dr,B/dr,C/dr},qs[3]={A/ds,B/ds,C/ds};
                    while(L<=U){int R=U;
                        for(int i=0;i<3;++i){int x=qr[i]/L,y=qs[i]/L;if(x)R=min(R,qr[i]/x);if(y)R=min(R,qs[i]/y);}
                        i64 sum=0;
                        if(m==1)sum=sv.M[R]-sv.M[L-1];
                        else if(R<=Sieve::SMALL)sum=sv.small_mobius(m,L,R);
                        else if(R-L+1<=32){for(int t=L;t<=R;++t)if(sv.mu[ds*t]&&sv.mu[r*t])sum+=sv.mu[t];}
                        else{
                            if(!factored){factors.clear();for(int z=m;z>1;){int p=sv.spf[z];factors.push_back(p);while(z%p==0)z/=p;}
                                suffix.assign(factors.size()+1,1);for(int i=(int)factors.size()-1;i>=0;--i)suffix[i]=suffix[i+1]*factors[i];factored=true;}
                            sum=mobius(0,L-1,R);
                        }
                        if(sum)answer=(answer+(i64)coeff*value(dr*L,ds*L)%MOD*(sum%MOD))%MOD;
                        L=R+1;
                    }
                }
            }
        }
        if(answer<0)answer+=MOD;return answer;
    }
};
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);int T;if(!(cin>>T))return 0;vector<array<int,3>> cases(T);int N=1;for(auto&a:cases)for(int&x:a){cin>>x;N=max(N,x);}Sieve sv(N);Solver solver{sv,{},{}};for(auto a:cases)cout<<solver.solve(a[0],a[1],a[2])<<'\n';}
