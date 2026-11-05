#include <bits/stdc++.h>
using namespace std;
static const int INF = 1000000000;

struct DownSeg {
    struct F { int a; bool g; };
    int n=0; vector<F> tr;
    static F id(){ return {INF,true}; }
    static F mergeF(const F& L,const F& R){ return {min(L.a, L.g?R.a:INF), bool(L.g&&R.g)}; }
    DownSeg(){}
    DownSeg(int n):n(n),tr(4*n+4,id()){}
    void build(int p,int l,int r,const vector<F>& a){ if(l==r){tr[p]=a[l];return;} int m=(l+r)/2; build(p*2,l,m,a); build(p*2+1,m+1,r,a); tr[p]=mergeF(tr[p*2],tr[p*2+1]); }
    void build(const vector<F>& a){ if(n) build(1,0,n-1,a); }
    void modify(int p,int l,int r,int x,F v){ if(l==r){tr[p]=v;return;} int m=(l+r)/2; if(x<=m)modify(p*2,l,m,x,v);else modify(p*2+1,m+1,r,x,v);tr[p]=mergeF(tr[p*2],tr[p*2+1]); }
    void modify(int x,F v){modify(1,0,n-1,x,v);}
    F query(int p,int l,int r,int ql,int qr) const { if(ql<=l&&r<=qr)return tr[p]; int m=(l+r)/2; if(qr<=m)return query(p*2,l,m,ql,qr); if(ql>m)return query(p*2+1,m+1,r,ql,qr); return mergeF(query(p*2,l,m,ql,qr),query(p*2+1,m+1,r,ql,qr)); }
    F query(int l,int r) const { return query(1,0,n-1,l,r); }
};

struct UpSeg {
    int n=0; vector<int> mn; vector<unsigned char> allGo;
    UpSeg(){} UpSeg(int n):n(n),mn(4*n+4,INF),allGo(4*n+4,0){}
    void pull(int p){mn[p]=min(mn[p*2],mn[p*2+1]);allGo[p]=allGo[p*2]&allGo[p*2+1];}
    void build(int p,int l,int r,const vector<int>& b,const vector<unsigned char>& g){if(l==r){mn[p]=b[l];allGo[p]=g[l];return;}int m=(l+r)/2;build(p*2,l,m,b,g);build(p*2+1,m+1,r,b,g);pull(p);}
    void build(const vector<int>& b,const vector<unsigned char>& g){if(n)build(1,0,n-1,b,g);}
    void modify(int p,int l,int r,int x,int b,bool g){if(l==r){mn[p]=b;allGo[p]=g;return;}int m=(l+r)/2;if(x<=m)modify(p*2,l,m,x,b,g);else modify(p*2+1,m+1,r,x,b,g);pull(p);}
    void modify(int x,int b,bool g){modify(1,0,n-1,x,b,g);}
    int queryMin(int p,int l,int r,int ql,int qr)const{if(ql<=l&&r<=qr)return mn[p];int m=(l+r)/2,ans=INF;if(ql<=m)ans=min(ans,queryMin(p*2,l,m,ql,qr));if(qr>m)ans=min(ans,queryMin(p*2+1,m+1,r,ql,qr));return ans;}
    int queryMin(int l,int r)const{return l<=r?queryMin(1,0,n-1,l,r):INF;}
    int rightmostFalse(int p,int l,int r,int ql,int qr)const{if(r<ql||qr<l||allGo[p])return -1;if(l==r)return l;int m=(l+r)/2;int z=rightmostFalse(p*2+1,m+1,r,ql,qr);if(z!=-1)return z;return rightmostFalse(p*2,l,m,ql,qr);}
    int rightmostFalse(int l,int r)const{return rightmostFalse(1,0,n-1,l,r);}
};

struct Solver {
    struct Edge{int u,v,itemU=-1,itemV=-1;};
    int n,root=0,timer=0; vector<Edge> edges; vector<vector<pair<int,int>>> adj; vector<vector<int>> child;
    vector<int> fa,dep,siz,heavy,top,dfn,rev,chainEnd;
    vector<int> selfItem,upPort,parentSidePort,localCnt;
    vector<int> owner,dsu,compSz,compHead,compTail; vector<unsigned char> inUsed,outUsed;
    vector<set<pair<int,int>>> S; vector<int> setVal; vector<unsigned char> inSet;
    DownSeg downSeg; UpSeg upSeg;
    Solver(int n):n(n),adj(n),child(n),fa(n,-1),dep(n),siz(n),heavy(n,-1),top(n),dfn(n),rev(n),chainEnd(n,-1),selfItem(n),upPort(n,-1),parentSidePort(n,-1),localCnt(n),S(n){}
    void addEdge(int u,int v){int id=edges.size();edges.push_back({u,v,-1,-1});adj[u].push_back({v,id});adj[v].push_back({u,id});}
    int newItem(int u){int id=owner.size();owner.push_back(u);dsu.push_back(id);compSz.push_back(1);compHead.push_back(id);compTail.push_back(id);inUsed.push_back(0);outUsed.push_back(0);return id;}
    int findSet(int x){while(dsu[x]!=x){dsu[x]=dsu[dsu[x]];x=dsu[x];}return x;}
    int itemOnEdge(int eid,int u)const{auto&e=edges[eid];return e.u==u?e.itemU:e.itemV;}
    int portBetween(int u,int v)const{if(fa[u]==v)return upPort[u];if(fa[v]==u)return parentSidePort[v];abort();}
    void buildItems(){owner.reserve(3*n);dsu.reserve(3*n);compSz.reserve(3*n);compHead.reserve(3*n);compTail.reserve(3*n);inUsed.reserve(3*n);outUsed.reserve(3*n);for(int u=0;u<n;++u)selfItem[u]=newItem(u);for(auto&e:edges){e.itemU=newItem(e.u);e.itemV=newItem(e.v);}int M=owner.size();setVal.assign(M,INF);inSet.assign(M,0);for(int u=0;u<n;++u)localCnt[u]=1+(int)adj[u].size();}
    void buildTreeIterative(){
        vector<int> order; order.reserve(n); fa[root]=-1; dep[root]=0; order.push_back(root);
        for(size_t qi=0;qi<order.size();++qi){int u=order[qi];for(auto [v,eid]:adj[u])if(v!=fa[u]){fa[v]=u;dep[v]=dep[u]+1;upPort[v]=itemOnEdge(eid,v);parentSidePort[v]=itemOnEdge(eid,u);child[u].push_back(v);order.push_back(v);}}
        for(int ii=n-1;ii>=0;--ii){int u=order[ii];siz[u]=1;int best=0;for(int v:child[u]){siz[u]+=siz[v];if(siz[v]>best){best=siz[v];heavy[u]=v;}}}
        timer=0; vector<pair<int,int>> st; st.reserve(n); st.push_back({root,root});
        while(!st.empty()){
            auto [start,tp]=st.back(); st.pop_back();
            int u=start;
            while(u!=-1){top[u]=tp;dfn[u]=timer;rev[timer++]=u;for(int i=(int)child[u].size()-1;i>=0;--i){int v=child[u][i];if(v!=heavy[u])st.push_back({v,v});}if(heavy[u]==-1){chainEnd[tp]=dfn[u];break;}u=heavy[u];}
        }
    }
    bool legal(int a,int b){if(a<0||b<0||outUsed[a]||inUsed[b])return false;int ra=findSet(a),rb=findSet(b);if(ra!=rb)return true;return compSz[ra]==localCnt[owner[a]];}
    int getDown(int u)const{return downSeg.query(dfn[u],chainEnd[top[u]]).a;}
    int bestLight(int u,int x){if(outUsed[x]||S[u].empty())return INF;int r=findSet(x),forbid=-1;if(compSz[r]!=localCnt[u])forbid=compHead[r];auto it=S[u].begin();if(it->second!=forbid)return it->first;++it;return it==S[u].end()?INF:it->first;}
    DownSeg::F calcDownPoint(int u){if(u==root)return {INF,false};int p=upPort[u];int a=bestLight(u,p);bool g=heavy[u]!=-1&&legal(p,parentSidePort[heavy[u]]);return {a,g};}
    pair<int,bool> calcUpPoint(int u){if(heavy[u]==-1)return {INF,false};int x=parentSidePort[heavy[u]];int branch=bestLight(u,x);bool go=fa[u]!=-1&&legal(x,upPort[u]);return {branch,go};}
    void changeLightKey(int p,int t,int newVal){int item=parentSidePort[t];if(!inSet[item])return;S[p].erase({setVal[item],item});setVal[item]=newVal;S[p].insert({setVal[item],item});}
    void touch(int u){while(u!=-1){int t=top[u];int oldTop=getDown(t);downSeg.modify(dfn[u],calcDownPoint(u));auto [b,go]=calcUpPoint(u);upSeg.modify(dfn[u],b,go);int newTop=getDown(t);if(oldTop==newTop)break;int p=fa[t];if(p==-1)break;changeLightKey(p,t,newTop);u=p;}}
    int genericBranch(int u,int x){int ans=bestLight(u,x);int h=heavy[u];if(h!=-1){int y=parentSidePort[h];if(legal(x,y))ans=min(ans,getDown(h));}return ans;}
    int querySource(int s){int ans=genericBranch(s,selfItem[s]);if(fa[s]==-1||!legal(selfItem[s],upPort[s]))return ans;int c=s,u=fa[s];while(true){if(heavy[u]==c){int l=dfn[top[u]],r=dfn[u];int stop=upSeg.rightmostFalse(l,r);int ql=stop==-1?l:stop;ans=min(ans,upSeg.queryMin(ql,r));if(stop!=-1)break;int t=top[u];if(fa[t]==-1)break;c=t;u=fa[t];}else{int x=parentSidePort[c];ans=min(ans,genericBranch(u,x));if(fa[u]==-1||!legal(x,upPort[u]))break;c=u;u=fa[u];}}return ans;}
    void eraseDestinationItem(int item){if(!inSet[item])return;int u=owner[item];S[u].erase({setVal[item],item});inSet[item]=0;}
    void addLocalArc(int u,int a,int b){if(!legal(a,b)){cerr<<"illegal arc at u="<<u+1<<" a="<<a<<" b="<<b<<"\n";abort();}eraseDestinationItem(b);int ra=findSet(a),rb=findSet(b);int newHead=compHead[ra],newTail=compTail[rb];outUsed[a]=1;inUsed[b]=1;if(ra!=rb){dsu[rb]=ra;compSz[ra]+=compSz[rb];compHead[ra]=newHead;compTail[ra]=newTail;}touch(u);}
    vector<int> getPath(int s,int t)const{vector<int>A,B;int u=s,v=t;while(dep[u]>dep[v]){A.push_back(u);u=fa[u];}while(dep[v]>dep[u]){B.push_back(v);v=fa[v];}while(u!=v){A.push_back(u);B.push_back(v);u=fa[u];v=fa[v];}A.push_back(u);reverse(B.begin(),B.end());A.insert(A.end(),B.begin(),B.end());return A;}
    void commitPath(int s,int t){auto path=getPath(s,t);if(path.size()==1){addLocalArc(s,selfItem[s],selfItem[s]);return;}for(int i=0;i<(int)path.size();++i){int u=path[i],a,b;if(i==0){a=selfItem[u];b=portBetween(u,path[i+1]);}else if(i+1==(int)path.size()){a=portBetween(u,path[i-1]);b=selfItem[u];}else{a=portBetween(u,path[i-1]);b=portBetween(u,path[i+1]);}addLocalArc(u,a,b);}}
    void init(){buildItems();buildTreeIterative();vector<int> initialDown(n,INF);for(int i=n-1;i>=0;--i){int u=rev[i];int val=u;for(int c:child[u])val=min(val,initialDown[c]);initialDown[u]=val;}
        for(int u=0;u<n;++u){int x=selfItem[u];inSet[x]=1;setVal[x]=u;S[u].insert({u,x});for(int c:child[u])if(c!=heavy[u]){int item=parentSidePort[c];inSet[item]=1;setVal[item]=initialDown[c];S[u].insert({setVal[item],item});}}
        vector<DownSeg::F> f(n);vector<int>b(n,INF);vector<unsigned char>go(n,0);
        for(int u=0;u<n;++u){f[dfn[u]]=calcDownPoint(u);auto q=calcUpPoint(u);b[dfn[u]]=q.first;go[dfn[u]]=q.second;}
        downSeg=DownSeg(n);upSeg=UpSeg(n);downSeg.build(f);upSeg.build(b,go);
    }
    vector<int> solve(const vector<int>&pos){if(n==1)return{0};init();vector<int>ans;ans.reserve(n);for(int s:pos){int t=querySource(s);if(t>=INF){cerr<<"no target for source "<<s+1<<"\n";abort();}ans.push_back(t);commitPath(s,t);}return ans;}
};
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);int T;if(!(cin>>T))return 0;while(T--){int n;cin>>n;vector<int>pos(n);for(int&i:pos){cin>>i;--i;}Solver solver(n);for(int i=1;i<n;++i){int u,v;cin>>u>>v;--u;--v;solver.addEdge(u,v);}auto ans=solver.solve(pos);for(int i=0;i<n;++i){if(i)cout<<' ';cout<<ans[i]+1;}cout<<'\n';} }
