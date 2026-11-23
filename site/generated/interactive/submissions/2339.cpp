#include <bits/stdc++.h>
using namespace std;
int R,C; vector<string> g;
int dr[4]={-1,0,1,0}, dc[4]={0,1,0,-1};
int main(){
    if(!(cin>>R>>C)) return 0; g.resize(R); for(auto&s:g)cin>>s;
    vector<vector<int>> id(R, vector<int>(C,-1)); vector<int> cr,cc; int cnt=0;
    for(int i=0;i<R;i++)for(int j=0;j<C;j++) if(g[i][j]=='.'){id[i][j]=cnt++;cr.push_back(i);cc.push_back(j);}
    int n=cnt*4;
    vector<int> dist(n), succ[3]; for(int a=0;a<3;a++) succ[a].assign(n,-1);
    vector<int> pS(n,-1);
    auto isOpen=[&](int r,int c){return r>=0&&r<R&&c>=0&&c<C&&g[r][c]=='.';};
    for(int ce=0;ce<cnt;ce++)for(int d=0;d<4;d++){
        int s=ce*4+d; int r=cr[ce],c=cc[ce],k=0;
        while(isOpen(r+dr[d]*(k+1),c+dc[d]*(k+1)))k++;
        dist[s]=k; succ[0][s]=ce*4+(d+3)%4; succ[1][s]=ce*4+(d+1)%4;
        if(k>0) succ[2][s]=id[r+dr[d]][c+dc[d]]*4+d;
    }
    for(int s=0;s<n;s++) if(succ[2][s]>=0) pS[succ[2][s]]=s;
    // components
    vector<int> comp(cnt,-1); int nc=0;
    for(int i=0;i<cnt;i++) if(comp[i]<0){
        queue<int> q; q.push(i); comp[i]=nc;
        while(!q.empty()){int x=q.front();q.pop();for(int d=0;d<4;d++){int r=cr[x]+dr[d],c=cc[x]+dc[d];if(isOpen(r,c)&&comp[id[r][c]]<0){comp[id[r][c]]=nc;q.push(id[r][c]);}}}
        nc++;
    }
    // refinement
    vector<int> par, lvl, dep; vector<vector<int>> members; vector<int> cls(n);
    par.push_back(0); lvl.push_back(-1); dep.push_back(0); members.push_back({});
    {
        map<int,int> dn;
        for(int s=0;s<n;s++){
            auto it=dn.find(dist[s]); int nd;
            if(it==dn.end()){nd=par.size();par.push_back(0);lvl.push_back(0);dep.push_back(1);members.push_back({});dn[dist[s]]=nd;}
            else nd=it->second;
            cls[s]=nd; members[nd].push_back(s);
        }
    }
    vector<int> changed(n); iota(changed.begin(),changed.end(),0);
    vector<int> mark(par.size(),0); int stamp=0; int k=1;
    while(true){
        stamp++; if(mark.size()<par.size()) mark.resize(par.size(),0);
        vector<int> cand;
        for(int u:changed){ int ps[3]={succ[1][u],succ[0][u],pS[u]};
            for(int p:ps) if(p>=0){int X=cls[p]; if(mark[X]!=stamp){mark[X]=stamp;cand.push_back(X);}}}
        vector<pair<int,vector<array<int,4>>>> pend;
        for(int X:cand){
            if(members[X].size()<2) continue;
            vector<array<int,4>> v; v.reserve(members[X].size());
            for(int s:members[X]) v.push_back({cls[succ[0][s]],cls[succ[1][s]],succ[2][s]>=0?cls[succ[2][s]]:-1,s});
            sort(v.begin(),v.end());
            auto&f=v.front(); auto&b=v.back();
            if(f[0]!=b[0]||f[1]!=b[1]||f[2]!=b[2]) pend.push_back({X,move(v)});
        }
        if(pend.empty()) break;
        changed.clear();
        for(auto&[X,v]:pend){
            members[X].clear();
            int cur=-1;
            for(size_t i=0;i<v.size();i++){
                if(i==0||v[i][0]!=v[i-1][0]||v[i][1]!=v[i-1][1]||v[i][2]!=v[i-1][2]){
                    cur=par.size();par.push_back(X);lvl.push_back(k);dep.push_back(dep[X]+1);members.push_back({});
                }
                cls[v[i][3]]=cur; members[cur].push_back(v[i][3]); changed.push_back(v[i][3]);
            }
        }
        k++;
    }
    int N=par.size(); const int LOG=17;
    vector<vector<int>> up(LOG, vector<int>(N));
    up[0]=par; for(int b=1;b<LOG;b++) for(int v=0;v<N;v++) up[b][v]=up[b-1][up[b-1][v]];
    auto lca=[&](int a,int b){
        if(dep[a]<dep[b]) swap(a,b);
        int df=dep[a]-dep[b]; for(int i=0;i<LOG;i++) if(df>>i&1) a=up[i][a];
        if(a==b) return a;
        for(int i=LOG-1;i>=0;i--) if(up[i][a]!=up[i][b]){a=up[i][a];b=up[i][b];}
        return up[0][a];
    };
    auto ancAtDepth=[&](int v,int D){int df=dep[v]-D; for(int i=0;i<LOG;i++) if(df>>i&1) v=up[i][v]; return v;};
    auto classAt=[&](int u,int j){
        int v=cls[u]; if(lvl[v]<=j) return v;
        for(int b=LOG-1;b>=0;b--){int w=up[b][v]; if(lvl[w]>j) v=w;}
        return up[0][v];
    };
    const char* names[3]={"left","right","step"};
    vector<int> S(n); iota(S.begin(),S.end(),0);
    int pending=-1;
    deque<int> plan; int planState=-1;
    int d;
    while(cin>>d){
        if(d<0) return 0;
        if(pending>=0){ for(auto&s:S) s=succ[pending][s]; }
        { vector<int> T; for(int s:S) if(dist[s]==d) T.push_back(s); S.swap(T); }
        if(S.empty()){ cout<<"no"<<endl; return 0; }
        // all same cell?
        bool same=true; for(int s:S) if(s/4!=S[0]/4){same=false;break;}
        if(same){ cout<<"yes "<<cr[S[0]/4]+1<<" "<<cc[S[0]/4]+1<<endl; return 0; }
        // groups
        vector<pair<int,int>> gp; for(int s:S) gp.push_back({cls[s],s});
        sort(gp.begin(),gp.end());
        bool anySolv=false; int numGroups=0; int navCenter=-1;
        for(size_t i=0;i<gp.size();){
            size_t j=i; while(j<gp.size()&&gp[j].first==gp[i].first) j++;
            numGroups++;
            // check
            bool ok=true; int center=-1;
            bool sc=true; for(size_t t=i;t<j;t++) if(gp[t].second/4!=gp[i].second/4) sc=false;
            if(sc){ center=gp[i].second/4; }
            else{
                int dm[4]={0,0,0,0};
                for(size_t t=i;t<j;t++){ if(dm[gp[t].second%4]++){ok=false;break;} }
                int A=gp[i].second; int ra=cr[A/4],ca=cc[A/4],dA=A%4;
                for(size_t t=i+1;t<j&&ok;t++){
                    int B=gp[t].second; int rb=cr[B/4],cb=cc[B/4],dB=B%4;
                    int kk=(dB-dA+4)%4; int crr,ccc;
                    int Dr=rb-ra,Dc=cb-ca;
                    if(kk==0){ok=false;break;}
                    if(kk==2){ if((ra+rb)%2!=0||(ca+cb)%2!=0){ok=false;break;} crr=(ra+rb)/2;ccc=(ca+cb)/2; }
                    else{
                        if(((Dr-Dc)%2+2)%2!=0){ok=false;break;}
                        int a,b;
                        if(kk==1){ b=(Dr-Dc)/2; a=(-Dc-Dr)/2; }
                        else { a=(Dc-Dr)/2; b=(-Dr-Dc)/2; }
                        crr=ra-a; ccc=ca-b;
                    }
                    if(!isOpen(crr,ccc)){ok=false;break;}
                    int ce=id[crr][ccc];
                    if(comp[ce]!=comp[A/4]){ok=false;break;}
                    if(center==-1) center=ce; else if(center!=ce){ok=false;break;}
                }
            }
            if(ok){ anySolv=true; navCenter=center; }
            i=j;
        }
        if(!anySolv){ cout<<"no"<<endl; return 0; }
        int act=-1;
        if(numGroups==1){
            if(plan.empty()||planState!=S[0]){
                plan.clear();
                vector<int> prv(n,-2), pa(n,-1); queue<int> q; q.push(S[0]); prv[S[0]]=-1; int tgt=-1;
                while(!q.empty()){
                    int x=q.front();q.pop();
                    if(x/4==navCenter){tgt=x;break;}
                    for(int a=0;a<3;a++){int y=succ[a][x]; if(y>=0&&prv[y]==-2){prv[y]=x;pa[y]=a;q.push(y);}}
                }
                if(tgt<0){ cout<<"no"<<endl; return 0; }
                vector<int> path; for(int x=tgt;prv[x]!=-1;x=prv[x]) path.push_back(pa[x]);
                reverse(path.begin(),path.end());
                for(int a:path) plan.push_back(a);
                planState=S[0];
            }
            act=plan.front(); plan.pop_front(); planState=succ[act][S[0]];
        } else {
            plan.clear(); planState=-1;
            int X=cls[S[0]]; for(int s:S) X=lca(X,cls[s]);
            int v=ancAtDepth(cls[S[0]],dep[X]+1); int kk=lvl[v];
            size_t best=SIZE_MAX;
            for(int a=0;a<3;a++){
                if(succ[a][S[0]]<0) continue;
                vector<int> cl; cl.reserve(S.size());
                for(int s:S) cl.push_back(classAt(succ[a][s],kk-1));
                sort(cl.begin(),cl.end());
                size_t mx=0,run=0; int distinctC=0;
                for(size_t i=0;i<cl.size();i++){ if(i==0||cl[i]!=cl[i-1]){distinctC++;run=0;} run++; mx=max(mx,run); }
                if(distinctC>=2&&mx<best){best=mx;act=a;}
            }
            if(act<0){ act=0; }
        }
        cout<<names[act]<<endl;
        pending=act;
    }
    return 0;
}
