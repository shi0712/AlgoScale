#include <bits/stdc++.h>
using namespace std;

int n;
long long curK;

long long query(int i, long long j){
    printf("%d %lld\n", i, j);
    fflush(stdout);
    long long k;
    if(scanf("%lld",&k)!=1) exit(0);
    if(k==1 || k<0) exit(0);
    curK=k;
    return k;
}

int main(){
    if(scanf("%d",&n)!=1) return 0;
    long long k;
    if(scanf("%lld",&k)!=1) return 0;
    if(k<=1) return 0;
    curK=k;
    vector<long long> kv(n,-1);
    kv[0]=k;
    for(int v=1;v<=n-2;v++) kv[v]=query(1,1);
    int pos1=n-2;
    vector<int> occ(n,0);
    auto determineFromLevels=[&](int cnt)->bool{
        long long mn=LLONG_MAX,mx=LLONG_MIN;
        for(int v=0;v<cnt;v++){mn=min(mn,kv[v]);mx=max(mx,kv[v]);}
        if(mx==mn) return false;
        long long L0=mn; int z=0;
        for(int v=0;v<cnt;v++){ occ[v]=(kv[v]==L0); z+=occ[v]; }
        if(cnt<n) occ[n-1]=(L0-z==1);
        return true;
    };
    if(!determineFromLevels(n-1)){
        long long L=kv[0];
        if(n==3){
            kv[2]=query(1,1);
            pos1=2;
            determineFromLevels(3);
        } else {
            if(L==n-1){
                for(int v=0;v<n-1;v++) occ[v]=1;
                occ[n-1]=0;
            } else {
                for(int v=0;v<n-1;v++) occ[v]=0;
                occ[n-1]=1;
            }
        }
    }
    int T=pos1;
    vector<int> inU(n,0);
    for(int v=0;v<n;v++) inU[v]=occ[v];
    inU[T]=1;
    for(int i=2;i<=n;i++){
        int r=n-i+1;
        int Usz=0; for(int v=0;v<n;v++) Usz+=inU[v];
        struct C{int p,a;};
        vector<C> cands;
        bool tight=(r==Usz-1);
        for(int p=0;p<n;p++){
            if(!inU[p]) continue;
            if(p==T){
                if(!tight) cands.push_back({p,0});
            } else {
                cands.push_back({p,1});
                if(!tight) cands.push_back({p,0});
            }
        }
        int off=0;
        auto predict=[&](const C&c,int x)->long long{
            if(x==c.p) return Usz;
            return (long long)Usz - c.a + (inU[x]?0:1);
        };
        auto distinctP=[&](const vector<C>&v)->int{
            set<int> s; for(auto&c:v) s.insert(c.p); return (int)s.size();
        };
        while(true){
            int dp=distinctP(cands);
            if(dp==1){
                int p=cands[0].p;
                int cur=((p+off)%n+n)%n;
                if(cur==T){
                    int a=cands[0].a;
                    if(a==1 && p!=T) inU[p]=0;
                    break;
                }
                int j=((T-cur)%n+n)%n;
                long long kk=query(i,j);
                off=(off+j)%n;
                vector<C> nc;
                for(auto&c:cands){
                    int x=(c.p+off)%n;
                    if(predict(c,x)==kk) nc.push_back(c);
                }
                if(nc.empty()) nc=cands;
                cands=nc;
                continue;
            }
            double best=1e18; int bestj=1;
            for(int j=1;j<n;j++){
                map<long long, vector<C>> groups;
                for(auto&c:cands){
                    int x=(c.p+off+j)%n;
                    groups[predict(c,x)].push_back(c);
                }
                double cost=0;
                for(auto&g:groups){
                    auto&S=g.second;
                    int d=distinctP(S);
                    double h;
                    if(d==1){
                        int x=(S[0].p+off+j)%n;
                        h=(x==T)?0.0:1.0;
                    } else h=log2((double)d)+1.0;
                    cost+=(double)S.size()*h;
                }
                cost/=cands.size();
                if(cost<best-1e-12){best=cost;bestj=j;}
            }
            long long kk=query(i,bestj);
            off=(off+bestj)%n;
            vector<C> nc;
            for(auto&c:cands){
                int x=(c.p+off)%n;
                if(predict(c,x)==kk) nc.push_back(c);
            }
            if(nc.empty()) nc=cands;
            cands=nc;
        }
    }
    return 0;
}
