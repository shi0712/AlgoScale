#include <bits/stdc++.h>
using namespace std;

map<long long,long long> P; // prefix sums known
long long n;

long long ask(long long r){
    auto it=P.find(r);
    if(it!=P.end()) return it->second;
    printf("? 1 %lld\n",r);
    fflush(stdout);
    long long v;
    if(scanf("%lld",&v)!=1) exit(0);
    if(v==-1) exit(0);
    P[r]=v;
    return v;
}

int main(){
    if(scanf("%lld",&n)!=1) return 0;
    P[0]=0;
    long long S=ask(n);
    long long l=1,r=n,s=S;
    while(true){
        long long len=r-l+1;
        if(len==1){ printf("! %lld\n",s); fflush(stdout); return 0; }
        // candidate check
        {
            vector<long long> cand;
            for(int j=1;j<62 && (1LL<<j)<=len;j++){
                long long pw=1LL<<j;
                if(s%pw) continue;
                long long v=s/pw;
                if((v&(v-1))!=0) continue;
                if(v> s+1-len) continue;
                cand.push_back(v);
            }
            if(cand.size()==1){ printf("! %lld\n",cand[0]); fflush(stdout); return 0; }
        }
        long long base=P[l-1];
        long long half=s/2;
        long long T=base+half;
        long long lo=max(l, r-half), hi=min(r-1, l-1+half);
        long long found=-1;
        auto upd=[&](long long x,long long px){
            if(px==T){found=x;return;}
            if(px<T){ lo=max(lo,x+1); hi=min(hi,x+(T-px)); }
            else { hi=min(hi,x-1); lo=max(lo,x-(px-T)); }
        };
        for(auto it=P.lower_bound(l-1); it!=P.end() && it->first<=r; ++it){
            upd(it->first,it->second);
            if(found!=-1) break;
        }
        while(found==-1 && lo<hi){
            long long mid=(lo+hi)/2;
            long long pm=ask(mid);
            upd(mid,pm);
        }
        long long p = (found!=-1)?found:lo;
        P[p]=T;
        long long L1=p-l+1, L2=r-p;
        if(L1<=L2){ r=p; }
        else { l=p+1; }
        s=half;
    }
}
