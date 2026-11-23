#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> adjStart, adjList;
vector<char> inS, inSt;
vector<int> distv, stampv;
int curStamp = 0;
vector<int> cntd;
vector<int> bq;

int main(){
    if(scanf("%d",&n)!=1) return 0;
    vector<int> par(n+1,0);
    vector<int> deg(n+1,0);
    for(int i=2;i<=n;i++){ if(scanf("%d",&par[i])!=1) return 0; deg[i]++; deg[par[i]]++; }
    adjStart.assign(n+2,0);
    for(int i=1;i<=n;i++) adjStart[i+1]=adjStart[i]+deg[i];
    adjList.assign(2*(n-1)+1,0);
    {
        vector<int> pos(adjStart.begin(),adjStart.end());
        for(int i=2;i<=n;i++){ adjList[pos[i]++]=par[i]; adjList[pos[par[i]]++]=i; }
    }
    auto T0 = chrono::steady_clock::now();
    auto elapsed=[&](){ return chrono::duration<double>(chrono::steady_clock::now()-T0).count(); };
    const double TOTAL=0.85;
    mt19937 rng(12345);
    inS.assign(n+1,1); inSt.assign(n+1,0);
    distv.assign(n+1,0); stampv.assign(n+1,0);
    cntd.assign(n+2,0); bq.assign(n+1,0);
    vector<int> S; for(int i=1;i<=n;i++) S.push_back(i);
    vector<int> d2(n+1), wv(n+1), mc(n+1), pp(n+1);

    // BFS within Steiner from u, returns maxd; fills cntd[0..maxd] with S counts
    auto bfs=[&](int u)->int{
        curStamp++;
        int h=0,t=0; bq[t++]=u; stampv[u]=curStamp; distv[u]=0;
        int maxd=0;
        while(h<t){
            int v=bq[h++]; int dv=distv[v];
            if(inS[v]){ if(dv>maxd){ for(int k=maxd+1;k<=dv;k++) cntd[k]=0; maxd=dv; } }
            for(int e=adjStart[v];e<adjStart[v+1];e++){
                int w=adjList[e];
                if(inSt[w] && stampv[w]!=curStamp){ stampv[w]=curStamp; distv[w]=dv+1; bq[t++]=w; }
            }
        }
        return maxd;
    };

    while(true){
        int m=S.size();
        if(m==1){ printf("! %d\n",S[0]); fflush(stdout); return 0; }
        // Steiner tree
        for(int i=1;i<=n;i++){ inSt[i]=1; d2[i]=deg[i]; }
        {
            vector<int> q;
            for(int i=1;i<=n;i++) if(!inS[i] && d2[i]<=1) q.push_back(i);
            size_t h=0;
            while(h<q.size()){
                int v=q[h++]; if(!inSt[v]) continue;
                inSt[v]=0;
                for(int e=adjStart[v];e<adjStart[v+1];e++){
                    int w=adjList[e];
                    if(inSt[w]){ d2[w]--; if(!inS[w] && d2[w]<=1) q.push_back(w); }
                }
            }
        }
        vector<int> stv;
        for(int i=1;i<=n;i++) if(inSt[i]) stv.push_back(i);
        // centroid
        int root=S[0];
        int cen=root;
        {
            curStamp++;
            int h=0,t=0; bq[t++]=root; stampv[root]=curStamp; pp[root]=0;
            while(h<t){
                int v=bq[h++];
                for(int e=adjStart[v];e<adjStart[v+1];e++){
                    int w=adjList[e];
                    if(inSt[w]&&stampv[w]!=curStamp){stampv[w]=curStamp;pp[w]=v;bq[t++]=w;}
                }
            }
            vector<int> ord(bq.begin(),bq.begin()+t);
            for(int v:ord){ wv[v]=inS[v]?1:0; mc[v]=0; }
            for(int i=t-1;i>=0;i--){ int v=ord[i]; if(pp[v]){ wv[pp[v]]+=wv[v]; mc[pp[v]]=max(mc[pp[v]],wv[v]); } }
            int best=INT_MAX;
            for(int v:ord){ int c=max(mc[v],m-wv[v]); if(c<best){best=c;cen=v;} }
        }
        vector<int> nearOrd;
        {
            curStamp++;
            int h=0,t=0; bq[t++]=cen; stampv[cen]=curStamp;
            while(h<t){
                int v=bq[h++];
                for(int e=adjStart[v];e<adjStart[v+1];e++){
                    int w=adjList[e];
                    if(inSt[w]&&stampv[w]!=curStamp){stampv[w]=curStamp;bq[t++]=w;}
                }
            }
            nearOrd.assign(bq.begin(),bq.begin()+t);
        }
        vector<int> rnd=stv; shuffle(rnd.begin(),rnd.end(),rng);
        int lg=0; while((1<<lg)<m) lg++;
        double rem=TOTAL-elapsed();
        double budget=max(0.0,rem/max(1,lg));
        double startT=elapsed();
        vector<char> tried(n+1,0);
        int bestVal=INT_MAX,bestU=-1,bestK=-1;
        int target=(m+1)/2;
        size_t ia=0,ib=0; int iter=0; bool turn=false;
        while(ia<nearOrd.size()||ib<rnd.size()){
            int u;
            if((!turn&&ia<nearOrd.size())||ib>=rnd.size()) u=nearOrd[ia++]; else u=rnd[ib++];
            turn=!turn;
            if(tried[u]) continue;
            tried[u]=1;
            for(int k=0;k<=0;k++) cntd[0]=0;
            int maxd=bfs(u);
            // recount histogram
            for(int k=0;k<=maxd;k++) cntd[k]=0;
            for(int x:S) cntd[distv[x]]++;
            int in=0;
            for(int k=0;k<maxd;k++){
                in+=cntd[k];
                if(in>0 && in<m){
                    int val=max(in,m-in);
                    if(val<bestVal){bestVal=val;bestU=u;bestK=k;}
                }
            }
            iter++;
            if(bestVal==target) break;
            if(bestU!=-1 && elapsed()-startT>budget) break;
        }
        printf("? %d %d\n",bestU,bestK); fflush(stdout);
        int o; if(scanf("%d",&o)!=1) return 0;
        if(o==-1) return 0;
        bfs(bestU);
        vector<int> NS;
        for(int x:S){ bool inside = distv[x]<=bestK; if(inside==(o==1)) NS.push_back(x); }
        for(int x:S) inS[x]=0;
        S=NS;
        for(int x:S) inS[x]=1;
        if(S.empty()) return 0;
    }
}
