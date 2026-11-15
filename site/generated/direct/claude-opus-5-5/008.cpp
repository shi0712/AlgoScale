#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static char *buf;
int main(){
    size_t cap = 1<<25;
    buf = (char*)malloc(cap);
    size_t len = fread(buf,1,cap,stdin);
    buf[len]=0;
    char *p = buf;
    auto readInt = [&]() -> ll {
        while(*p && (*p<'0'||*p>'9')) p++;
        ll x=0;
        while(*p>='0'&&*p<='9'){ x=x*10+(*p-'0'); p++; }
        return x;
    };
    int n = (int)readInt();
    ll P = readInt(), Q = readInt();
    vector<int> hs(n);
    int maxh = 0;
    for(int i=0;i<n;i++){ hs[i]=(int)readInt(); maxh=max(maxh,hs[i]); }
    int SZ = max(maxh, 1500) + 1;
    vector<int> cnt(SZ,0), r(SZ,0);
    for(int i=0;i<n;i++) cnt[hs[i]]++;
    vector<int>().swap(hs);
    {
        int x=0;
        for(int h=0;h<SZ;h++){
            while((ll)(x+1)*(x+1)<=h) x++;
            r[h]=x;
        }
    }
    vector<ll> W(1500,0);
    for(int h=2;h<=maxh;h++){
        if(!cnt[h]) continue;
        int x=r[h];
        while(x>1){ W[x]+=cnt[h]; x=r[x]; }
    }
    vector<int> gainH(SZ,0);
    vector<signed char> bH(SZ,0);
    vector<int> bucket(SZ+2,0);
    ll phiv[1500]; int bv[1500];
    auto bl = [&](ll x)->int{ return x==0?0:64-__builtin_clzll((unsigned long long)x); };
    ll best = LLONG_MIN;
    auto eval = [&](ll t, bool needB, ll &B)->ll{
        ll L = max(2*t-2, 0LL);
        int blL = bl(L);
        auto phi = [&](ll s, int &b)->ll{
            if(s<=L){ b=0; return s; }
            b = bl(s)-blL;
            if((s>>b)>L) b++;
            return (s>>b)+t*b;
        };
        for(int v=0;v<1500;v++){
            int b; phiv[v]=phi(v,b); bv[v]=b;
        }
        fill(bucket.begin(), bucket.begin()+maxh+2, 0);
        ll sumPhi=0, b0=0;
        int maxg=0;
        for(int h=0;h<=maxh;h++){
            int c=cnt[h];
            if(!c){ gainH[h]=0; continue; }
            int b; ll f=phi(h,b);
            sumPhi += f*c;
            b0 += (ll)b*c;
            bH[h]=(signed char)b;
            int g=0;
            if(h>=2){
                g=(int)(f-phiv[r[h]]);
                if(g>0){ bucket[g]+=c; if(g>maxg) maxg=g; }
                else g=0;
            }
            gainH[h]=g;
        }
        int gW[1500];
        for(int v=2;v<1500;v++){
            ll g=phiv[v]-phiv[r[v]];
            if(g<0) g=0;
            gW[v]=(int)g;
            if(g>0 && W[v]>0){
                bucket[g]+=(int)W[v];
                if(g>maxg) maxg=(int)g;
            }
        }
        ll rem=P, sumTop=0; int gstar=-1;
        for(int g=maxg; g>=1; g--){
            ll c=bucket[g];
            if(!c) continue;
            if(c<=rem){ rem-=c; sumTop+=c*g; }
            else { sumTop+=rem*g; gstar=g; break; }
        }
        ll H = sumPhi - sumTop - t*Q;
        if(needB){
            ll Bv=b0;
            ll rr = rem; // partial remaining (valid if gstar!=-1)
            for(int h=2;h<=maxh;h++){
                int g=gainH[h];
                if(g<=0) continue;
                if(g>gstar){
                    Bv += (ll)cnt[h]*(bv[r[h]]-bH[h]);
                } else if(g==gstar && rr>0){
                    ll take=min<ll>(rr,cnt[h]);
                    rr-=take;
                    Bv += take*(bv[r[h]]-bH[h]);
                }
            }
            for(int v=2;v<1500;v++){
                int g=gW[v];
                if(g<=0||W[v]==0) continue;
                if(g>gstar){
                    Bv += W[v]*(bv[r[v]]-bv[v]);
                } else if(g==gstar && rr>0){
                    ll take=min<ll>(rr,W[v]);
                    rr-=take;
                    Bv += take*(bv[r[v]]-bv[v]);
                }
            }
            B=Bv;
        }
        best=max(best,H);
        return H;
    };
    ll lo=0, hi=1000001;
    while(lo<hi){
        ll mid=(lo+hi)/2;
        ll B=0;
        eval(mid,true,B);
        if(B>Q) lo=mid+1; else hi=mid;
    }
    ll dummy;
    for(ll t=lo-1;t<=lo+1;t++){
        if(t<0) continue;
        eval(t,false,dummy);
    }
    printf("%lld\n",best);
    return 0;
}
