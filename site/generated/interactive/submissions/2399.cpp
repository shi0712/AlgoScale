#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;

int n, nb;
vector<int> cm;               // cell mask per barrier
vector<pair<int,int>> bar;
vector<ull> Ms;
vector<int> Vs;
vector<array<uint8_t,10>> cnt;
vector<int> vis;
ull s;
mt19937_64 rng(712367);

static inline double nowSec(){
    return chrono::duration<double>(chrono::steady_clock::now().time_since_epoch()).count();
}

int rebuild(){
    int T = Ms.size();
    cnt.assign(T, array<uint8_t,10>());
    vis.assign(T, 0);
    int E = 0;
    for(int t=0;t<T;t++){
        for(int x=0;x<10;x++) cnt[t][x]=0;
        ull st = s ^ Ms[t];
        for(int b=0;b<nb;b++){
            if((st>>b)&1){
                int m = cm[b];
                for(int x=0;x<n;x++) if((m>>x)&1) cnt[t][x]++;
            }
        }
        int v=0;
        for(int x=0;x<n;x++) if(cnt[t][x]==0) v++;
        vis[t]=v;
        E += abs(v - Vs[t]);
    }
    return E;
}

void solve(double budget){
    int E = rebuild();
    if(E==0) return;
    double start = nowSec();
    int T = Ms.size();
    double expt[64];
    double temp = 0.45;
    for(int i=0;i<64;i++) expt[i]=exp(-i/temp);
    long long it=0;
    while(E>0){
        if((it&255)==0){
            if(nowSec()-start > budget) break;
        }
        it++;
        int b = rng()%nb;
        int m = cm[b];
        int dE=0;
        int nv[128];
        for(int t=0;t<T;t++){
            int e = ((s>>b)&1) ^ ((Ms[t]>>b)&1);
            int v = vis[t];
            int nvv = v;
            if(e){
                for(int x=0;x<n;x++) if(((m>>x)&1) && cnt[t][x]==1) nvv++;
            } else {
                for(int x=0;x<n;x++) if(((m>>x)&1) && cnt[t][x]==0) nvv--;
            }
            nv[t]=nvv;
            dE += abs(nvv - Vs[t]) - abs(v - Vs[t]);
        }
        bool acc = false;
        if(dE<=0) acc = true;
        else if(dE<64){
            double r = (rng()>>11) * (1.0/9007199254740992.0);
            if(r < expt[dE]) acc = true;
        }
        if(acc){
            for(int t=0;t<T;t++){
                int e = ((s>>b)&1) ^ ((Ms[t]>>b)&1);
                for(int x=0;x<n;x++) if((m>>x)&1){
                    if(e) cnt[t][x]--; else cnt[t][x]++;
                }
                vis[t]=nv[t];
            }
            s ^= (1ULL<<b);
            E += dE;
        }
    }
    if(E>0){
        // keep whatever we have; rebuild for consistency
        rebuild();
    }
}

int main(){
    int v0;
    if(!(cin>>n)) return 0;
    if(!(cin>>v0)) return 0;
    if(v0==n || v0<0) return 0;
    for(int i=1;i<=n;i++) for(int j=i;j<=n;j++){
        bar.push_back({i,j});
        int m=0;
        for(int x=i;x<=j;x++) m |= 1<<(x-1);
        cm.push_back(m);
    }
    nb = bar.size();
    s = rng() & ((nb==64)?~0ULL:((1ULL<<nb)-1));
    Ms.push_back(0);
    Vs.push_back(v0);
    ull M = 0;
    double totalStart = nowSec();
    int steps = 0;
    solve(0.1);
    while(true){
        steps++;
        // current hypothesized state
        int T = Ms.size();
        int bestB=-1, bestGain=-1;
        // ensure cnt/vis valid for last constraint
        ull cur = s ^ M;
        for(int b=0;b<nb;b++){
            if((cur>>b)&1){
                int m = cm[b];
                int g=0;
                for(int x=0;x<n;x++) if(((m>>x)&1) && cnt[T-1][x]==1) g++;
                if(g>bestGain || (g==bestGain && (rng()&1))){
                    bestGain=g; bestB=b;
                }
            }
        }
        if(bestB<0){
            // hypothesis says all inactive but not finished; pick random barrier
            bestB = rng()%nb;
        }
        printf("%d %d\n", bar[bestB].first, bar[bestB].second);
        fflush(stdout);
        int v;
        if(scanf("%d",&v)!=1) return 0;
        if(v<0 || v==n) return 0;
        M ^= (1ULL<<bestB);
        Ms.push_back(M);
        Vs.push_back(v);
        double elapsed = nowSec()-totalStart;
        double budget = 0.1;
        if(elapsed > 7.0) budget = 0.01;
        else if(elapsed > 5.0) budget = 0.04;
        solve(budget);
    }
    return 0;
}
