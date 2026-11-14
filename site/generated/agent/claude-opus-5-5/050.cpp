#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll BIG=1000000000000LL, INF=LLONG_MAX/4;
int n,m,N,T,US,NB;
vector<array<int,3>> optOwner; vector<array<ll,3>> optCost, optB; vector<int> nopt;
vector<int> own, cnt, capv; int kus=0;
vector<ll> pot, dred; vector<int> pv, pc;
// cost of T->o (requires cnt[o]>0)
ll costTo(int o){ if(o==US) return cnt[US]<=kus?BIG:0; return 0;}
ll ownCost(int i,int o){ for(int t=0;t<nopt[i];t++) if(optOwner[i][t]==o) return optCost[i][t]; return INF;}
void dijkstra(){
    // build adjacency
    vector<int> head(N+1,0);
    for(int i=0;i<n;i++) head[own[i]]+=nopt[i]-1;
    vector<int> start(N+1,0); for(int v=0;v<N;v++) start[v+1]=start[v]+head[v];
    vector<int> pos(start.begin(),start.end()-1);
    vector<int> eTo(start[N]), eC(start[N]); vector<ll> eW(start[N]);
    for(int i=0;i<n;i++){ int q=own[i]; ll cq=ownCost(i,q); for(int t=0;t<nopt[i];t++){int r=optOwner[i][t]; if(r==q) continue; int id=pos[q]++; eTo[id]=r; eC[id]=i; eW[id]=optCost[i][t]-cq;}}
    dred.assign(N,INF); pv.assign(N,-1); pc.assign(N,-1);
    priority_queue<pair<ll,int>,vector<pair<ll,int>>,greater<>> pq;
    dred[T]=0;
    for(int o=0;o<T;o++) if(cnt[o]>0){ ll w=costTo(o)+pot[T]-pot[o]; if(w<dred[o]){dred[o]=w;pv[o]=T;pc[o]=-1;pq.push({w,o});}}
    while(!pq.empty()){ auto [d,u]=pq.top(); pq.pop(); if(d!=dred[u]) continue;
        for(int id=start[u];id<start[u+1];id++){ int v=eTo[id]; ll w=eW[id]+pot[u]-pot[v]; if(d+w<dred[v]){dred[v]=d+w;pv[v]=u;pc[v]=eC[id];pq.push({dred[v],v});}}}
}
void updPot(){
    ll D=0; for(int v=0;v<N;v++) if(dred[v]<INF) D=max(D,dred[v]);
    ll s=D; for(int v=0;v<N;v++) if(dred[v]>=INF) s=max(s,pot[T]-pot[v]);
    for(int v=0;v<N;v++) pot[v]+= (dred[v]<INF?dred[v]:s);
}
void augment(int t){
    int v=t; while(v!=T){ int u=pv[v]; if(pc[v]>=0){ int i=pc[v]; cnt[own[i]]--; own[i]=v; cnt[v]++; } v=u; }
}
int main(){
    scanf("%d %d",&n,&m); N=m+3; T=m+2; US=m; NB=m+1;
    vector<vector<ll>> a(n,vector<ll>(m));
    for(auto&r:a)for(auto&x:r)scanf("%lld",&x);
    optOwner.resize(n); optCost.resize(n); optB.resize(n); nopt.assign(n,0);
    for(int i=0;i<n;i++){
        vector<pair<ll,int>> v; for(int j=0;j<m;j++) v.push_back({a[i][j],j});
        sort(v.begin(),v.end());
        vector<pair<ll,int>> uni;
        for(int j=0;j<m;){int k=j; while(k<m&&v[k].first==v[j].first)k++; if(k-j==1&&v[j].first>0) uni.push_back(v[j]); j=k;}
        ll v1= uni.empty()?0:uni.back().first;
        ll c=v1+1; {set<ll> s; for(auto&p:v) s.insert(p.first); while(s.count(c)) c++;}
        int t=0;
        if(uni.empty()){ optOwner[i][t]=NB; optCost[i][t]=0; optB[i][t]=0; t++; }
        else{
            int W1=uni.back().second; optOwner[i][t]=W1; optCost[i][t]=0; optB[i][t]=0; t++;
            int W2= uni.size()>=2? uni[uni.size()-2].second : NB;
            optOwner[i][t]=W2; optCost[i][t]=v1; optB[i][t]=v1; t++;
        }
        optOwner[i][t]=US; optCost[i][t]=c; optB[i][t]=c; t++;
        nopt[i]=t;
    }
    own.assign(n,0); cnt.assign(N,0); capv.assign(m,0);
    for(int i=0;i<n;i++){ int best=-1; ll bc=INF; for(int t=0;t<nopt[i];t++){int o=optOwner[i][t]; if(o<m) continue; if(optCost[i][t]<bc){bc=optCost[i][t];best=o;}} own[i]=best; cnt[best]++; }
    // Bellman-Ford initial potentials
    pot.assign(N,0);
    for(int it=0;it<N+2;it++){ bool ch=false;
        auto rel=[&](int u,int v,ll w){ if(pot[u]+w<pot[v]){pot[v]=pot[u]+w;ch=true;} };
        for(int i=0;i<n;i++){int q=own[i]; ll cq=ownCost(i,q); for(int t=0;t<nopt[i];t++){int r=optOwner[i][t]; if(r!=q) rel(q,r,optCost[i][t]-cq);}}
        for(int o=0;o<T;o++){ if(cnt[o]>0) rel(T,o,costTo(o));
            if(o<m){ if(cnt[o]<capv[o]) rel(o,T,0);} else if(o==NB) rel(o,T,0); else rel(o,T,cnt[US]<kus?-BIG:0); }
        if(!ch) break; }
    auto total=[&](){ ll s=0; for(int i=0;i<n;i++) s+=ownCost(i,own[i]); return s; };
    ll bestCost=total(); vector<int> bestOwn=own;
    for(int k=1;k<=n;k++){
        vector<int> cand;
        for(int p=0;p<m;p++){ if(cnt[p]==k-1) cand.push_back(p); else capv[p]=k; }
        while(true){
            dijkstra();
            int bp=-1; ll bd=0; int bidx=-1;
            for(int idx=0;idx<(int)cand.size();idx++){ int p=cand[idx]; if(dred[p]>=INF) continue; ll d=dred[p]+pot[p]-pot[T]; if(d<bd){bd=d;bp=p;bidx=idx;} }
            updPot();
            if(bp<0){ for(int p:cand) capv[p]=k; break; }
            augment(bp); capv[bp]=k; cand.erase(cand.begin()+bidx);
        }
        if(cnt[US]==k-1){ dijkstra(); updPot(); augment(US); }
        kus=k;
        ll tc=total(); if(tc<bestCost){bestCost=tc;bestOwn=own;}
    }
    for(int i=0;i<n;i++){ ll b=0; for(int t=0;t<nopt[i];t++) if(optOwner[i][t]==bestOwn[i]) b=optB[i][t]; printf("%lld%c",b,i+1<n?' ':'\n'); }
}
