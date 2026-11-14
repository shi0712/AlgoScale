#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
int main(){
    int n,m; ll d;
    if(scanf("%d %d %lld",&n,&m,&d)!=3) return 0;
    vector<ll> L(n);
    for(auto&x:L) scanf("%lld",&x);
    sort(L.rbegin(),L.rend());
    vector<pair<ll,int>> P;
    for(int i=0;i<m;i++){ll e;int t;scanf("%lld %d",&e,&t); if(e<=L[0]) P.push_back({e,t-1});}
    sort(P.begin(),P.end(),[](auto&a,auto&b){return a.first>b.first;});
    int N=P.size();
    vector<ll> e(N); vector<int> ty(N);
    for(int i=0;i<N;i++){e[i]=P[i].first;ty[i]=P[i].second;} // 0 = junior, 1 = senior
    // below[t][i]: smallest index > i with type t
    vector<array<int,2>> below(N+1);
    below[N]={N,N};
    for(int i=N-1;i>=0;i--){ below[i]=below[i+1]; } // shift
    {
        array<int,2> cur={N,N};
        for(int i=N-1;i>=0;i--){ below[i]=cur; cur[ty[i]]=i; }
    }
    // patterns: (ta,tb)
    int pa[4]={0,0,1,1}, pb[4]={0,1,0,1};
    vector<vector<int>> nxt(4, vector<int>(N+1,N));
    for(int p=0;p<4;p++){
        for(int i=N-1;i>=0;i--){
            nxt[p][i]=nxt[p][i+1];
            if(ty[i]==pb[p]){int a=below[i][pa[p]]; if(a<N && e[i]-e[a]<=d) nxt[p][i]=i;}
        }
    }
    vector<int> capStart(n);
    for(int k=0;k<n;k++){
        // first index with e<=L[k]
        int lo=0,hi=N;
        while(lo<hi){int mid=(lo+hi)/2; if(e[mid]<=L[k]) hi=mid; else lo=mid+1;}
        capStart[k]=lo;
    }
    // fronts: cur[j] = vector of (pos,sum), pos increasing, sum increasing
    vector<vector<pair<int,ll>>> cur(1), nx;
    cur[0].push_back({-1,0});
    size_t maxFront=0,totalSt=0;
    int cpat[4]; for(int p=0;p<4;p++) cpat[p]=(pa[p]==0)+(pb[p]==0);
    vector<vector<pair<int,ll>>> tmp[4];
    for(int k=0;k<n;k++){
        int J=cur.size();
        for(int p=0;p<4;p++){
            tmp[p].assign(J,{});
            for(int j=0;j<J;j++){
                auto &out=tmp[p][j];
                for(auto [pos,s]:cur[j]){
                    int st=max(pos+1,capStart[k]);
                    if(st>=N) break;
                    int b=nxt[p][st]; if(b>=N) break;
                    int a=below[b][pa[p]];
                    ll ns=s+e[a]+e[b];
                    if(!out.empty() && out.back().first==a) out.back().second=max(out.back().second,ns);
                    else out.push_back({a,ns});
                }
            }
        }
        nx.assign(2*(k+1)+1,{});
        for(int jj=0;jj<(int)nx.size();jj++){
            const vector<pair<int,ll>>* L4[4]; int cnt=0;
            for(int p=0;p<4;p++){int j=jj-cpat[p]; if(j>=0&&j<J&&!tmp[p][j].empty()) L4[cnt++]=&tmp[p][j];}
            if(!cnt) continue;
            size_t idx[4]={0,0,0,0};
            auto &w=nx[jj]; ll best=LLONG_MIN;
            while(true){
                int bi=-1; int bp=INT_MAX; ll bs=LLONG_MIN;
                for(int q=0;q<cnt;q++) if(idx[q]<L4[q]->size()){
                    auto &x=(*L4[q])[idx[q]];
                    if(x.first<bp||(x.first==bp&&x.second>bs)){bp=x.first;bs=x.second;bi=q;}
                }
                if(bi<0) break;
                idx[bi]++;
                if(bs>best){best=bs; w.push_back({bp,bs});}
            }
            maxFront=max(maxFront,w.size()); totalSt+=w.size();
        }
        cur.swap(nx);
    }
    if(getenv("DBG")) fprintf(stderr,"maxFront=%zu total=%zu\n",maxFront,totalSt);
    for(int j=0;j<=2*n;j++){
        ll ans=-1;
        if(j<(int)cur.size() && !cur[j].empty()) ans=cur[j].back().second;
        printf("%lld%c",ans,j==2*n?'\n':' ');
    }
}
