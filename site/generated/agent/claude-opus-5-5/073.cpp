#include <bits/stdc++.h>
using namespace std;
int R,C;
vector<string> g;
static int D[100][100][4];
const int DR[4]={-1,0,1,0}, DC[4]={0,1,0,-1};
int comp[100][100];
// rotate vector clockwise k times
inline void rotv(int k,int&a,int&b){ for(int i=0;i<k;i++){int t=a;a=b;b=-t;} }
struct Cand{int r0,c0,d0;};
vector<Cand> S;
int px=0,py=0,pd=0; // relative pose
const int OFF=100,W=201;
inline int pid(int x,int y,int e){return ((x+OFF)*W+(y+OFF))*4+e;}
vector<int> agreed, distv, par, stamp; int curStamp=0;
int rounds=0;
int readD(){int d; if(!(cin>>d)) exit(0); return d;}
inline void absOf(const Cand&c,int x,int y,int e,int&ar,int&ac,int&ad){
    int a=x,b=y; rotv(c.d0,a,b); ar=c.r0+a; ac=c.c0+b; ad=(e+c.d0)&3;
}
inline int valOf(const Cand&c,int x,int y,int e){int ar,ac,ad;absOf(c,x,y,e,ar,ac,ad);return D[ar][ac][ad];}
map<array<int,4>,int> classMap; // (shapeid, x, y,?) -> class id
vector<int> shapeIdOf; // comp*4+rot
vector<long long> csr,csc; vector<int> csz;
vector<int> compGood; // center good
vector<vector<pair<int,int>>> compCells;
map<vector<pair<int,int>>,int> shapeMap;
vector<array<int,2>> compMinRot; // per comp*4+rot : min row/col after rotation
int classOf(int r,int c,int d){
    int cp=comp[r][c];
    int k=(4-d)&3; // rotate so d becomes N: rotate clockwise k times
    int a=r,b=c; rotv(k,a,b);
    auto &mn=compMinRot[cp*4+k];
    array<int,4> key={shapeIdOf[cp*4+k],a-mn[0],b-mn[1],0};
    auto it=classMap.find(key);
    if(it!=classMap.end()) return it->second;
    int id=classMap.size(); classMap[key]=id; return id;
}
int main(){
    cin>>R>>C; g.resize(R); for(auto&s:g)cin>>s;
    for(int r=0;r<R;r++)for(int c=0;c<C;c++)if(g[r][c]=='.')for(int d=0;d<4;d++){
        int k=0,rr=r+DR[d],cc=c+DC[d];
        while(rr>=0&&rr<R&&cc>=0&&cc<C&&g[rr][cc]=='.'){k++;rr+=DR[d];cc+=DC[d];}
        D[r][c][d]=k;
    }
    memset(comp,-1,sizeof comp);
    int nc=0;
    for(int r=0;r<R;r++)for(int c=0;c<C;c++)if(g[r][c]=='.'&&comp[r][c]<0){
        vector<pair<int,int>> cells; queue<pair<int,int>> q; q.push({r,c}); comp[r][c]=nc;
        while(!q.empty()){auto [a,b]=q.front();q.pop();cells.push_back({a,b});
            for(int d=0;d<4;d++){int x=a+DR[d],y=b+DC[d]; if(x>=0&&x<R&&y>=0&&y<C&&g[x][y]=='.'&&comp[x][y]<0){comp[x][y]=nc;q.push({x,y});}}}
        compCells.push_back(cells); nc++;
    }
    shapeIdOf.assign(nc*4,0); compMinRot.assign(nc*4,{0,0}); compGood.assign(nc,0);
    for(int i=0;i<nc;i++){
        long long sr=0,sc=0; int n=compCells[i].size();
        for(auto [a,b]:compCells[i]){sr+=a;sc+=b;}
        if(sr%n==0&&sc%n==0){int a=sr/n,b=sc/n; if(g[a][b]=='.'&&comp[a][b]==i) compGood[i]=1;}
        for(int k=0;k<4;k++){
            vector<pair<int,int>> v; int mr=INT_MAX,mc=INT_MAX;
            for(auto [a,b]:compCells[i]){int x=a,y=b;rotv(k,x,y);v.push_back({x,y});mr=min(mr,x);mc=min(mc,y);}
            for(auto&p:v){p.first-=mr;p.second-=mc;}
            sort(v.begin(),v.end());
            auto it=shapeMap.find(v); int id;
            if(it==shapeMap.end()){id=shapeMap.size();shapeMap[v]=id;} else id=it->second;
            shapeIdOf[i*4+k]=id; compMinRot[i*4+k]={mr,mc};
        }
    }
    // class info lazily: class -> set of comps, size
    int NP=W*W*4;
    agreed.assign(NP,-1); distv.assign(NP,0); par.assign(NP,-1); stamp.assign(NP,0);
    int d=readD();
    for(int r=0;r<R;r++)for(int c=0;c<C;c++)if(g[r][c]=='.')for(int e=0;e<4;e++) if(D[r][c][e]==d) S.push_back({r,c,e});
    auto out=[&](const string&s){ cout<<s<<"\n"<<flush; rounds++; };
    // precompute class sizes and multi-comp flags over all states
    // classes of state (cell,dir); compute for all states
    vector<int> clsSize, clsComp; // clsComp=-2 means multiple
    for(int r=0;r<R;r++)for(int c=0;c<C;c++)if(g[r][c]=='.')for(int e=0;e<4;e++){
        int id=classOf(r,c,e);
        if((int)clsSize.size()<=id){clsSize.push_back(0);clsComp.push_back(comp[r][c]);}
        clsSize[id]++; if(clsComp[id]!=comp[r][c]) clsComp[id]=-2;
    }
    auto good=[&](int id){ if(clsSize[id]==1) return true; if(clsComp[id]==-2) return false; return (bool)compGood[clsComp[id]]; };
    auto doAction=[&](int a){ // 0 left 1 right 2 step
        if(a==0){out("left");pd=(pd+3)&3;}
        else if(a==1){out("right");pd=(pd+1)&3;}
        else {out("step");px+=DR[pd];py+=DC[pd];}
        return readD();
    };
    while(true){
        // check termination
        {
            int ar0=-1,ac0=-1; bool same=true; bool anyGood=false; int cls0=-1; bool single=true;
            for(auto&cd:S){int ar,ac,ad;absOf(cd,px,py,pd,ar,ac,ad);
                if(ar0<0){ar0=ar;ac0=ac;} else if(ar!=ar0||ac!=ac0) same=false;
                int id=classOf(ar,ac,ad); if(good(id)) anyGood=true;
                if(cls0<0)cls0=id; else if(id!=cls0) single=false;
            }
            if(same){ out("yes "+to_string(ar0+1)+" "+to_string(ac0+1)); return 0;}
            if(!anyGood){ out("no"); return 0;}
            (void)single;
        }
        // BFS
        curStamp++;
        deque<int> dq; int s0=pid(px,py,pd); stamp[s0]=curStamp; par[s0]=-1; dq.push_back(s0);
        int target=-1; bool collapse=false;
        while(!dq.empty()){
            int u=dq.front();dq.pop_front();
            int e=u&3, y=(u>>2)%W-OFF, x=(u>>2)/W-OFF;
            int v=agreed[u];
            if(v<0){
                int v0=valOf(S[0],x,y,e); bool ok=true;
                for(size_t i=1;i<S.size();i++) if(valOf(S[i],x,y,e)!=v0){ok=false;break;}
                if(!ok){target=u;break;}
                agreed[u]=v=v0;
            }
            // collapse check: all same cell
            {
                int ar0,ac0,ad; absOf(S[0],x,y,e,ar0,ac0,ad); bool same=true;
                for(size_t i=1;i<S.size()&&same;i++){int ar,ac;absOf(S[i],x,y,e,ar,ac,ad); if(ar!=ar0||ac!=ac0) same=false;}
                if(same){target=u;collapse=true;break;}
            }
            int nb[3]; int cnt=0;
            nb[cnt++]=pid(x,y,(e+3)&3); nb[cnt++]=pid(x,y,(e+1)&3);
            if(v>0) nb[cnt++]=pid(x+DR[e],y+DC[e],e);
            for(int i=0;i<cnt;i++){int w=nb[i]; if(stamp[w]!=curStamp){stamp[w]=curStamp;par[w]=u;dq.push_back(w);}}
        }
        if(target<0){ out("no"); return 0;} // should not happen
        vector<int> path; for(int u=target;u!=-1;u=par[u]) path.push_back(u);
        reverse(path.begin(),path.end());
        for(size_t i=1;i<path.size();i++){
            int a=path[i-1],b=path[i]; int a_e=a&3,b_e=b&3; int act;
            if((a>>2)==(b>>2)) act=(b_e==((a_e+1)&3))?1:0; else act=2;
            d=doAction(act);
        }
        if(!collapse){
            vector<Cand> ns; ns.reserve(S.size());
            for(auto&cd:S) if(valOf(cd,px,py,pd)==d) ns.push_back(cd);
            S.swap(ns);
        }
    }
}
