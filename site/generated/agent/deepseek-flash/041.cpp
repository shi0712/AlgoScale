// 四点换枝  (four-point branch exchange)
//
// Operation: pick a path a-b-c-d, delete its 3 edges and add any spanning tree
// of K4 on {a,b,c,d}.  In particular a single edge swap {x,y} -> {x2,y2} can be
// performed whenever a suitable length-3 path through the four involved
// vertices exists; swapEdge() below searches for such a path.
//
// Answer: NO iff S is a star (a star admits no operation at all) and S != T.
// Otherwise the construction below always succeeds:
//
//   phase 1  flatten S into   hub + leaves   plus the chain   hub - r - f,
//            where r is a leaf of T, f its T-neighbour, hub another leaf of T.
//
//   phase 2  root T at r (thus f is a child of r).  Walk a DFS of T starting at
//            f.  Invariant: the already placed part equals T restricted to it,
//            every unplaced vertex is a leaf attached to the hub, and the hub
//            itself sits on the vertex currently visited.  At vertex p we place
//            all free T-children of p (one op each), then descend into a child
//            (one op) and recurse.  The walk is ordered to end on qq=par_T(hub)
//            so that the hub finishes in its final position.  If qq has no
//            child other than the hub, the walk ends on par_T(qq) leaving qq
//            free and one last op puts both qq and the hub into place.
//
//   ops <= (n-1) + (n-3) + 2(n-2) + 2 <= 4n-5 <= 395  for n <= 100.
#include <bits/stdc++.h>
using namespace std;

int n;
bool Sin[105][105];      // the first tree, as read
bool Tg[105][105];       // the target tree
bool cur[105][105];      // current tree
struct Op { int a,b,c,d,u1,v1,u2,v2,u3,v3; };
vector<Op> ops;

bool verify = (getenv("VERIFY")!=nullptr);   // extra internal consistency checks

int hub=-1;
vector<char> freeNow;
vector<int> parT;
vector<vector<int>> chT;
int qq=-1, endV=-1;
bool caseB=false;
int nextOnPath[105];

// ------------------------------------------------------------------ helpers
static void applyOp(int a,int b,int c,int d,
                    int u1,int v1,int u2,int v2,int u3,int v3){
    if(verify){
        int vs[4]={a,b,c,d};
        assert(a>=1&&a<=n&&b>=1&&b<=n&&c>=1&&c<=n&&d>=1&&d<=n);
        assert(a!=b&&a!=c&&a!=d&&b!=c&&b!=d&&c!=d);
        assert(cur[a][b]&&cur[b][c]&&cur[c][d]);
        int e[3][2]={{u1,v1},{u2,v2},{u3,v3}};
        for(int i=0;i<3;i++){
            assert(e[i][0]!=e[i][1]);
            bool f1=false,f2=false;
            for(int q=0;q<4;q++){ if(vs[q]==e[i][0]) f1=true; if(vs[q]==e[i][1]) f2=true; }
            assert(f1&&f2);
        }
        for(int i=0;i<3;i++)for(int j=i+1;j<3;j++)
            assert(!((e[i][0]==e[j][0]&&e[i][1]==e[j][1])||(e[i][0]==e[j][1]&&e[i][1]==e[j][0])));
    }
    cur[a][b]=cur[b][a]=false; cur[b][c]=cur[c][b]=false; cur[c][d]=cur[d][c]=false;
    cur[u1][v1]=cur[v1][u1]=true; cur[u2][v2]=cur[v2][u2]=true; cur[u3][v3]=cur[v3][u3]=true;
    if(verify){
        vector<pair<int,int>> es;
        for(int i=1;i<=n;i++)for(int j=i+1;j<=n;j++) if(cur[i][j]) es.push_back({i,j});
        assert((int)es.size()==n-1);
        vector<int> p(n+1); for(int i=1;i<=n;i++) p[i]=i;
        function<int(int)> fnd=[&](int z){ while(p[z]!=z) z=p[z]=p[p[z]]; return z; };
        for(auto&e:es){ int x=fnd(e.first), y=fnd(e.second); assert(x!=y); p[x]=y; }
        int rt=fnd(1); for(int i=2;i<=n;i++) assert(fnd(i)==rt);
    }
    ops.push_back({a,b,c,d,u1,v1,u2,v2,u3,v3});
}

// replace edge {x,y} by {x2,y2} with a single operation; false if impossible
static bool swapEdge(int x,int y,int x2,int y2){
    if(x>y) swap(x,y);
    if(x2>y2) swap(x2,y2);
    if(x==x2&&y==y2) return true;         // nothing to do
    if(cur[x2][y2]) return false;         // target edge already present
    vector<int> b{x,y,x2,y2};
    sort(b.begin(),b.end()); b.erase(unique(b.begin(),b.end()),b.end());
    vector<vector<int>> sets;
    if((int)b.size()==4) sets.push_back(b);
    else for(int w=1;w<=n;w++){
        bool in=false; for(int z:b) if(z==w) in=true;
        if(!in){ auto s=b; s.push_back(w); sets.push_back(s); }
    }
    for(auto s:sets){
        vector<pair<int,int>> want;
        for(int p=0;p<4;p++)for(int q=p+1;q<4;q++){
            int a=s[p], c=s[q];
            if(a>c) swap(a,c);
            if(a==x&&c==y) continue;
            if(cur[s[p]][s[q]]) want.push_back({a,c});
        }
        want.push_back({x2,y2});
        sort(want.begin(),want.end());
        want.erase(unique(want.begin(),want.end()),want.end());
        if((int)want.size()!=3) continue;
        sort(s.begin(),s.end());
        do{                                  // find a length-3 path of the 4-set
            int P1=s[0],P2=s[1],P3=s[2],P4=s[3];
            if(!cur[P1][P2]||!cur[P2][P3]||!cur[P3][P4]) continue;
            int par[4]={0,1,2,3};
            function<int(int)> fnd=[&](int z){ while(par[z]!=z) z=par[z]=par[par[z]]; return z; };
            bool ok=true;
            for(int i=0;i<3&&ok;i++){
                int u=-1,v=-1;
                for(int j=0;j<4;j++){
                    if(s[j]==want[i].first) u=j;
                    if(s[j]==want[i].second) v=j;
                }
                if(u<0||v<0||u==v){ ok=false; break; }
                int ru=fnd(u), rv=fnd(v);
                if(ru==rv) ok=false;
                par[ru]=rv;
            }
            if(!ok) continue;
            int rt=fnd(0);
            for(int i=1;i<4&&ok;i++) if(fnd(i)!=rt) ok=false;
            if(!ok) continue;
            applyOp(P1,P2,P3,P4,
                    want[0].first,want[0].second,
                    want[1].first,want[1].second,
                    want[2].first,want[2].second);
            return true;
        } while(next_permutation(s.begin(),s.end()));
    }
    return false;
}

static int degCur(int v){ int d=0; for(int i=1;i<=n;i++) if(cur[v][i]) d++; return d; }
static void rootAt(int root, vector<int>& par, vector<int>& dep){
    par.assign(n+1,-1); dep.assign(n+1,0);
    vector<char> vis(n+1,0); vis[root]=1; queue<int> q; q.push(root);
    while(!q.empty()){
        int u=q.front(); q.pop();
        for(int v=1;v<=n;v++) if(cur[u][v]&&!vis[v]){ vis[v]=1; par[v]=u; dep[v]=dep[u]+1; q.push(v); }
    }
}

// ---------------------------------------------------------------- phase 1
// current tree ->  hub attached to every vertex except r,f  plus chain hub-r-f
static bool phase1(int H,int r,int f){
    vector<int> par,dep;
    while(true){
        rootAt(H,par,dep);
        int D=0;
        for(int v=1;v<=n;v++) if(v!=H && dep[v]>=2) D++;
        if(D<=1) break;
        if(degCur(H)==1){
            int b=-1; for(int v=1;v<=n;v++) if(cur[H][v]){ b=v; break; }
            if(b<0) return false;
            int x=-1;
            for(int v=1;v<=n;v++) if(cur[b][v]&&v!=H&&degCur(v)>=2){ x=v; break; }
            if(x<0) return false;
            int y=-1; for(int v=1;v<=n;v++) if(cur[x][v]&&v!=b){ y=v; break; }
            if(y<0) return false;
            applyOp(H,b,x,y, H,b, H,x, x,y);          // lift x to depth 1
        } else {
            int v=-1; for(int q=1;q<=n;q++) if(dep[q]==2){ v=q; break; }
            if(v<0) return false;
            int b=par[v];
            if(b==H||dep[b]!=1) return false;
            int d=-1; for(int q=1;q<=n;q++) if(cur[H][q]&&q!=b){ d=q; break; }
            if(d<0) return false;
            applyOp(v,b,H,d, H,v, H,b, H,d);          // promote v to depth 1
        }
    }
    rootAt(H,par,dep);
    int y=-1;
    for(int v=1;v<=n;v++) if(v!=H && dep[v]>=2){ y=v; break; }
    if(y<0) return false;
    int x=par[y];
    if(dep[x]!=1) return false;
    if(x==r && y==f){
        /* already hub - r - f */
    } else if(r==x){
        applyOp(y,r,H,f, r,f, r,H, H,y);
    } else if(r==y && f==x){
        int d=-1; for(int q=1;q<=n;q++) if(cur[H][q]&&q!=f){ d=q; break; }
        if(d<0) return false;
        applyOp(r,f,H,d, r,H, r,f, H,d);
    } else if(r==y){
        applyOp(r,x,H,f, r,f, r,H, H,x);
    } else if(f==x){
        applyOp(y,f,H,r, r,f, H,r, H,y);
    } else if(f==y){
        applyOp(f,x,H,r, f,r, r,H, H,x);
    } else {
        applyOp(y,x,H,r, r,y, r,H, H,x);          // -> chain hub - r - y
        applyOp(y,r,H,f, r,f, r,H, H,y);          // -> hub - r - f
    }
    if(!cur[H][r]||!cur[r][f]) return false;
    int hd=0; for(int v=1;v<=n;v++) if(cur[H][v]) hd++;
    if(hd!=n-2) return false;
    return true;
}

// ---------------------------------------------------------------- phase 2
static bool refreshAnchor(){
    int cnt=0;
    for(int v=1;v<=n;v++) if(cur[hub][v]&&!freeNow[v]) cnt++;
    return cnt==1;
}
static bool placeAt(int g,int c){             // free g becomes a child of c
    if(!swapEdge(g,hub,g,c)) return false;
    freeNow[g]=0;
    return refreshAnchor();
}
static bool hopH(int a,int s){                 // the hub moves from a to s
    if(!swapEdge(hub,a,hub,s)) return false;
    return refreshAnchor();
}
static bool hasPlaceableChild(int c){
    for(int d: chT[c]) if(d!=hub && !(caseB && d==qq)) return true;
    return false;
}
static bool dfs(int p){
    int pc = nextOnPath[p];
    for(int c: chT[p]){
        if(c==hub || (caseB && c==qq) || c==pc) continue;
        if(!placeAt(c,p)) return false;
        if(hasPlaceableChild(c)){
            if(!hopH(p,c)) return false;
            if(!dfs(c)) return false;
            if(!hopH(c,p)) return false;
        }
    }
    if(pc){
        if(!placeAt(pc,p)) return false;
        if(!hopH(p,pc)) return false;
        if(!dfs(pc)) return false;
    }
    return true;
}

// ------------------------------------------------------------------- build
// try to transform Sin into T using hub=H, r, f
static bool build(int H,int r,int f){
    memcpy(cur,Sin,sizeof(cur));
    ops.clear();
    hub=H;
    memset(nextOnPath,0,sizeof(nextOnPath));
    if(!phase1(H,r,f)) return false;

    parT.assign(n+1,-1); chT.assign(n+1,{});
    {
        vector<char> vis(n+1,0); vector<int> st{r}; vis[r]=1;
        while(!st.empty()){
            int u=st.back(); st.pop_back();
            for(int v=1;v<=n;v++) if(Tg[u][v]&&!vis[v]){ vis[v]=1; parT[v]=u; chT[u].push_back(v); st.push_back(v); }
        }
    }
    qq=parT[H];
    if(qq<0||qq==H||qq==r) return false;

    freeNow.assign(n+1,0);
    for(int v=1;v<=n;v++) if(v!=r&&v!=f&&v!=H) freeNow[v]=1;

    bool qqOtherChild=false;
    for(int c: chT[qq]) if(c!=H) qqOtherChild=true;
    if(qq==f){ caseB=false; endV=f; }
    else if(qqOtherChild){ caseB=false; endV=qq; }
    else { caseB=true; endV=parT[qq]; }
    if(endV<0) return false;

    for(int x=endV; x!=f && x>=0; x=parT[x]) nextOnPath[parT[x]]=x;

    if(!hopH(r,f)) return false;
    if(!dfs(f)) return false;
    if(caseB){
        if(!swapEdge(hub,endV,qq,endV)) return false;
        freeNow[qq]=0;
        if(!refreshAnchor()) return false;
    }
    for(int i=1;i<=n;i++)for(int j=1;j<=n;j++) if(cur[i][j]!=Tg[i][j]) return false;
    return true;
}

int main(){
    if(scanf("%d",&n)!=1) return 0;
    for(int i=0;i<n-1;i++){ int u,v; if(scanf("%d %d",&u,&v)!=2) return 0; Sin[u][v]=Sin[v][u]=true; }
    for(int i=0;i<n-1;i++){ int u,v; if(scanf("%d %d",&u,&v)!=2) return 0; Tg[u][v]=Tg[v][u]=true; }

    {
        bool same=true;
        for(int i=1;i<=n&&same;i++)for(int j=1;j<=n;j++) if(Sin[i][j]!=Tg[i][j]){ same=false; break; }
        if(same){ printf("YES\n0\n"); return 0; }
    }
    for(int v=1;v<=n;v++){
        int d=0; for(int i=1;i<=n;i++) if(Sin[v][i]) d++;
        if(d==n-1){ printf("NO\n"); return 0; }     // star: no operation exists
    }

    vector<int> tdeg(n+1,0);
    for(int i=1;i<=n;i++)for(int j=i+1;j<=n;j++) if(Tg[i][j]){ tdeg[i]++; tdeg[j]++; }
    vector<int> leaves;
    for(int v=1;v<=n;v++) if(tdeg[v]==1) leaves.push_back(v);

    struct Cand { int score,r,f,H; };
    vector<Cand> cand;
    for(int r: leaves){
        int f=-1; for(int v=1;v<=n;v++) if(Tg[r][v]){ f=v; break; }
        if(f<0) continue;
        for(int H: leaves){
            if(H==r||H==f) continue;
            int d=0; for(int i=1;i<=n;i++) if(Sin[H][i]) d++;
            cand.push_back({d,r,f,H});
        }
    }
    sort(cand.begin(),cand.end(),[](const Cand&a,const Cand&b){ return a.score>b.score; });

    vector<Op> best;
    bool have=false;
    int tried=0;
    for(auto&cd: cand){
        if(have && best.size()<=400u && tried>=24) break;   // cheap enough answer
        if(!have && tried>=40) break;
        tried++;
        if(build(cd.H,cd.r,cd.f)){
            if(!have || ops.size()<best.size()){ best=ops; have=true; }
            if(best.size()<=3u*(unsigned)n) break;          // good enough
        }
    }
    if(!have){ printf("NO\n"); return 0; }

    printf("YES\n%d\n",(int)best.size());
    for(auto&o:best){
        printf("%d %d %d %d\n",o.a,o.b,o.c,o.d);
        printf("%d %d %d %d %d %d\n",o.u1,o.v1,o.u2,o.v2,o.u3,o.v3);
    }
    return 0;
}
