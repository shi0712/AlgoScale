#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> par_, adjStart, adjList;
// adjacency
vector<vector<int>> adj;
// LCA via euler + sparse table
vector<int> dep, firstOcc, euler_;
vector<vector<int>> sp;
int lg2[1<<17];
int lcaDepthMin(int a,int b){ // returns min depth between occurrences
    int l=firstOcc[a], r=firstOcc[b]; if(l>r) swap(l,r);
    int k=lg2[r-l+1];
    return min(sp[k][l], sp[k][r-(1<<k)+1]);
}
inline int dist(int a,int b){ return dep[a]+dep[b]-2*lcaDepthMin(a,b); }

// centroid decomposition
vector<char> removed_;
vector<int> sz_;
struct Anc { int c; int d; int dp; }; // centroid, dist to c, dist to parent centroid of c (for B array of child) 
vector<vector<pair<int,int>>> ancs; // per vertex: list of (centroid, dist) from top to self
vector<int> cpar; // centroid parent
vector<vector<int>> A, B; // A[c][d]: count at dist d from c in comp(c); B[c][d]: count in comp(c) at dist d from cpar[c]

void calcSz(int v,int p){ sz_[v]=1; for(int w:adj[v]) if(w!=p && !removed_[w]){calcSz(w,v); sz_[v]+=sz_[w];} }
int findCent(int v,int p,int tot){ for(int w:adj[v]) if(w!=p && !removed_[w] && sz_[w]*2>tot) return findCent(w,v,tot); return v; }

void build(){
    removed_.assign(n+1,0); sz_.assign(n+1,0); ancs.assign(n+1,{}); cpar.assign(n+1,0);
    A.assign(n+1,{}); B.assign(n+1,{});
    // iterative-ish with explicit stack of (root, parentCentroid)
    vector<pair<int,int>> st; st.push_back({1,0});
    vector<int> bfsq, bfsd, bfsp;
    while(!st.empty()){
        auto [r,pc]=st.back(); st.pop_back();
        // compute sizes iteratively
        bfsq.clear(); bfsp.clear(); bfsq.push_back(r); bfsp.push_back(0);
        for(size_t i=0;i<bfsq.size();i++){int v=bfsq[i]; for(int w:adj[v]) if(w!=bfsp[i]&&!removed_[w]){bfsq.push_back(w);bfsp.push_back(v);}}
        for(int i=(int)bfsq.size()-1;i>=0;i--){int v=bfsq[i]; sz_[v]=1; for(int w:adj[v]) if(w!=bfsp[i]&&!removed_[w]) sz_[v]+=sz_[w];}
        int tot=bfsq.size(); int c=r, pp=0;
        while(true){ int nx=-1; for(int w:adj[c]) if(w!=pp && !removed_[w] && sz_[w]<sz_[c] && sz_[w]*2>tot){nx=w;break;} if(nx<0)break; pp=c; c=nx; }
        cpar[c]=pc;
        // BFS from c
        bfsq.clear(); bfsd.clear(); bfsp.clear(); bfsq.push_back(c); bfsd.push_back(0); bfsp.push_back(0);
        int mx=0;
        for(size_t i=0;i<bfsq.size();i++){int v=bfsq[i]; ancs[v].push_back({c,bfsd[i]}); mx=max(mx,bfsd[i]);
            for(int w:adj[v]) if(w!=bfsp[i]&&!removed_[w]){bfsq.push_back(w);bfsd.push_back(bfsd[i]+1);bfsp.push_back(v);}}
        A[c].assign(mx+1,0);
        if(pc){ int mb=0; for(int v:bfsq){ int d=ancs[v][ancs[v].size()-2].second; mb=max(mb,d);} B[c].assign(mb+1,0); }
        removed_[c]=1;
        for(int w:adj[c]) if(!removed_[w]) st.push_back({w,c});
    }
}

vector<int> S;
vector<char> inS;
inline int countBall(int u,int k){
    auto &L=ancs[u]; int res=0; int Lsz=L.size();
    for(int i=0;i<Lsz;i++){
        int c=L[i].first, d=L[i].second;
        int r=k-d;
        if(r>=0){ auto &a=A[c]; res+=a[min(r,(int)a.size()-1)]; }
        if(i+1<Lsz && r>=0){ auto &b=B[L[i+1].first]; res-=b[min(r,(int)b.size()-1)]; }
    }
    return res;
}

int main(){
    int T; if(scanf("%d",&T)!=1) return 0;
    for(int i=2;i<(1<<17);i++) lg2[i]=lg2[i/2]+1;
    while(T--){
        if(scanf("%d",&n)!=1) return 0;
        adj.assign(n+1,{}); par_.assign(n+1,0);
        for(int i=2;i<=n;i++){ if(scanf("%d",&par_[i])!=1) return 0; adj[par_[i]].push_back(i); adj[i].push_back(par_[i]); }
        dep.assign(n+1,0); firstOcc.assign(n+1,0); euler_.clear();
        // iterative euler tour (depth sequence)
        {
            vector<int> it(n+1,0); vector<int> stk={1}; firstOcc[1]=0; euler_.push_back(0);
            while(!stk.empty()){
                int v=stk.back();
                // children of v: adj excluding parent
                bool pushed=false;
                while(it[v]<(int)adj[v].size()){
                    int w=adj[v][it[v]++]; if(w==par_[v]) continue;
                    dep[w]=dep[v]+1; firstOcc[w]=euler_.size(); euler_.push_back(dep[w]); stk.push_back(w); pushed=true; break;
                }
                if(!pushed){ stk.pop_back(); if(!stk.empty()) euler_.push_back(dep[stk.back()]); }
            }
            int m=euler_.size(); int K=lg2[m]+1; sp.assign(K, vector<int>(m));
            sp[0]=euler_;
            for(int k=1;k<K;k++) for(int i=0;i+(1<<k)<=m;i++) sp[k][i]=min(sp[k-1][i],sp[k-1][i+(1<<(k-1))]);
        }
        build();
        S.resize(n); iota(S.begin(),S.end(),1);
        inS.assign(n+1,0);
        vector<int> sub(n+1), order; // order by index: parents < children so reverse index order is postorder-compatible
        int queries=0;
        bool fail=false;
        while(S.size()>1){
            int m=S.size();
            for(int v=1;v<=n;v++) inS[v]=0;
            for(int v:S) inS[v]=1;
            // hull
            for(int v=1;v<=n;v++) sub[v]=inS[v];
            vector<int> kids(n+1,0);
            for(int v=n;v>=2;v--){ sub[par_[v]]+=sub[v]; if(sub[v]>0) kids[par_[v]]++; }
            vector<int> hull;
            for(int v=1;v<=n;v++) if(sub[v]>0 && (inS[v] || kids[v]>=2 || sub[v]<m)) hull.push_back(v);
            int best=INT_MAX, bu=-1, bk=-1;
            long long direct=(long long)m*hull.size();
            if(direct<=4000000LL || m<=300){
                vector<int> cnt(n+2); vector<int> ds(m);
                for(int u:hull){
                    int mx=0;
                    for(int i=0;i<m;i++){ int d=dist(u,S[i]); ds[i]=d; cnt[d]++; if(d>mx)mx=d; }
                    int c=0;
                    for(int k=0;k<mx;k++){ c+=cnt[k]; int v=max(c,m-c); if(v<best){best=v;bu=u;bk=k;} }
                    for(int i=0;i<m;i++) cnt[ds[i]]=0;
                }
            } else {
                for(int c=1;c<=n;c++){ fill(A[c].begin(),A[c].end(),0); fill(B[c].begin(),B[c].end(),0); }
                for(int v:S){
                    auto &L=ancs[v];
                    for(size_t i=0;i<L.size();i++){
                        A[L[i].first][L[i].second]++;
                        if(i>0) B[L[i].first][L[i-1].second]++;
                    }
                }
                for(int c=1;c<=n;c++){ for(size_t i=1;i<A[c].size();i++) A[c][i]+=A[c][i-1]; for(size_t i=1;i<B[c].size();i++) B[c][i]+=B[c][i-1]; }
                for(int u:hull){
                    // smallest k with count >= m/2 (2*count>=m)
                    int lo=0, hi=n; // count(u,n)=m
                    while(lo<hi){ int mid=(lo+hi)/2; if(2*countBall(u,mid)>=m) hi=mid; else lo=mid+1; }
                    for(int k=max(0,lo-1);k<=lo;k++){
                        int c=countBall(u,k); if(c==m) continue; int v=max(c,m-c); if(v<best){best=v;bu=u;bk=k;}
                    }
                }
            }
            printf("? %d %d\n",bu,bk); fflush(stdout); queries++;
            int o; if(scanf("%d",&o)!=1){ fail=true; break; }
            vector<int> NS; NS.reserve(m);
            for(int v:S){ bool in=dist(bu,v)<=bk; if(in==(o==1)) NS.push_back(v); }
            S.swap(NS);
            if(S.empty()){ fail=true; break; }
        }
        if(fail) return 0;
        printf("! %d\n",S[0]); fflush(stdout);
    }
}
