#include <bits/stdc++.h>
using namespace std;

// For even n we use n-3 perfect matchings. Split the vertices into equal
// halves, equitably color the two induced graphs with m+8 colors, and
// complete these matchings using cross edges. Randomize and retry if one
// completion fails. Every remaining edge belongs to a regular bipartite
// graph, which is factored by Euler-tour splitting and perfect matchings.
// The eight spare colors keep each completion small while avoiding the
// tight matching constraints of using only m colors.
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

// Edmonds' algorithm is used only for the small instances.
struct Blossom {
    int n;
    vector<vector<int>> g;
    vector<int> match, p, base, q;
    vector<char> used, blossom;
    Blossom(vector<vector<int>> gg): n(gg.size()), g(move(gg)), match(n,-1), p(n), base(n), q(n), used(n), blossom(n) {}
    int lca(int a,int b) {
        vector<char> seen(n,0);
        while(true) { a=base[a]; seen[a]=1; if(match[a]==-1) break; a=p[match[a]]; }
        while(true) { b=base[b]; if(seen[b]) return b; b=p[match[b]]; }
    }
    void markPath(int v,int b,int child) {
        while(base[v]!=b) {
            blossom[base[v]]=blossom[base[match[v]]]=1;
            p[v]=child; child=match[v]; v=p[match[v]];
        }
    }
    int findPath(int root) {
        fill(used.begin(),used.end(),0); fill(p.begin(),p.end(),-1);
        iota(base.begin(),base.end(),0);
        int head=0,tail=0; q[tail++]=root; used[root]=1;
        while(head<tail) {
            int v=q[head++];
            for(int u:g[v]) {
                if(base[v]==base[u] || match[v]==u) continue;
                if(u==root || (match[u]!=-1 && p[match[u]]!=-1)) {
                    int b=lca(v,u); fill(blossom.begin(),blossom.end(),0);
                    markPath(v,b,u); markPath(u,b,v);
                    for(int i=0;i<n;i++) if(blossom[base[i]]) {
                        base[i]=b; if(!used[i]) {used[i]=1; q[tail++]=i;}
                    }
                } else if(p[u]==-1) {
                    p[u]=v;
                    if(match[u]==-1) return u;
                    u=match[u]; used[u]=1; q[tail++]=u;
                }
            }
        }
        return -1;
    }
    bool solve() {
        vector<int> order(n); iota(order.begin(),order.end(),0); shuffle(order.begin(),order.end(),rng);
        for(auto &v:g) shuffle(v.begin(),v.end(),rng);
        for(int v:order) if(match[v]==-1) for(int u:g[v]) if(match[u]==-1) {match[v]=u;match[u]=v;break;}
        for(int v:order) if(match[v]==-1) {
            int u=findPath(v); if(u==-1) continue;
            while(u!=-1) {int pv=p[u], nv=(pv==-1?-1:match[pv]); match[u]=pv; if(pv!=-1)match[pv]=u; u=nv;}
        }
        return count(match.begin(),match.end(),-1)==0;
    }
};

struct Solver {
    int n,m;
    vector<string> g;
    vector<int> ans, order;
    Solver(vector<string> gg): n(gg.size()),m(n/2),g(move(gg)),ans(n*n,-1) {}
    void setColor(int u,int v,int c) {ans[u*n+v]=ans[v*n+u]=c;}
    void reset() {
        for(int i=0;i<n;i++) for(int j=0;j<n;j++) ans[i*n+j]=(g[i][j]=='1'?0:-1);
    }
    void small() {
        int d=n-3;
        while(true) {
            reset(); bool ok=true;
            for(int c=1;c<=d-2;c++) {
                vector<vector<int>> adj(n);
                for(int i=0;i<n;i++) for(int j=0;j<n;j++) if(ans[i*n+j]==0) adj[i].push_back(j);
                Blossom b(move(adj));
                if(!b.solve()) {ok=false;break;}
                for(int i=0;i<n;i++) if(i<b.match[i]) setColor(i,b.match[i],c);
            }
            if(!ok) continue;
            if(d==1) {for(int i=0;i<n;i++)for(int j=i+1;j<n;j++)if(ans[i*n+j]==0)setColor(i,j,1);return;}
            vector<char> seen(n,0);
            for(int st=0;st<n && ok;st++) if(!seen[st]) {
                vector<int> cyc; int v=st,prev=-1;
                do {
                    seen[v]=1;cyc.push_back(v);int next=-1;
                    for(int u=0;u<n;u++)if(ans[v*n+u]==0 && u!=prev){next=u;break;}
                    prev=v;v=next;
                }while(v!=st);
                if(cyc.size()%2) {ok=false;break;}
                for(int j=0;j<(int)cyc.size();j++)setColor(cyc[j],cyc[(j+1)%cyc.size()],d-1+j%2);
            }
            if(ok) return;
        }
    }

    // Make the matching sizes exactly target[c] by exchanging two colors
    // on alternating paths.
    void equalize(vector<vector<int>>& p,vector<int>& count,const vector<int>& target) {
        int k=p.size();
        vector<int> surplus,deficit;
        for(int c=0;c<k;c++) {
            if(count[c]>target[c])surplus.push_back(c);
            if(count[c]<target[c])deficit.push_back(c);
        }
        vector<int> seen(m,0),path; int timer=0;
        while(!surplus.empty()) {
            int a=surplus.back(),b=deficit.back();
            ++timer; bool found=false;
            for(int st=0;st<m && !found;st++) if(p[b][st]==-1 && p[a][st]!=-1 && seen[st]!=timer) {
                path.clear(); int v=st,c=a;
                while(true) {
                    path.push_back(v);seen[v]=timer;
                    int u=p[c][v];if(u==-1)break;
                    v=u;c=(c==a?b:a);
                }
                if((path.size()-1)%2==0)continue;
                for(int vtx:path) p[a][vtx]=p[b][vtx]=-1;
                for(int j=0;j+1<(int)path.size();j++) {
                    int nc=(j%2==0?b:a),u=path[j],v2=path[j+1];
                    p[nc][u]=v2;p[nc][v2]=u;
                }
                count[a]--;count[b]++;found=true;
            }
            assert(found);
            if(count[a]==target[a])surplus.pop_back();
            if(count[b]==target[b])deficit.pop_back();
        }
    }

    bool prepare(vector<int>& residual,int& k) {
        reset();order.resize(n);iota(order.begin(),order.end(),0);shuffle(order.begin(),order.end(),rng);
        k=m+8;
        vector<vector<int>> p[2]; vector<int> cnt[2];
        for(int h=0;h<2;h++) {
            p[h].assign(k,vector<int>(m,-1));cnt[h].assign(k,0);
            for(int u=0;u<m;u++)for(int v=u+1;v<m;v++)if(g[order[h*m+u]][order[h*m+v]]=='1') {
                int c;
                if(m%2)c=(u+v)%m;
                else if(v==m-1)c=2*u%(m-1);
                else c=(u+v)%(m-1);
                p[h][c][u]=v;p[h][c][v]=u;cnt[h][c]++;
            }
        }
        int e=accumulate(cnt[0].begin(),cnt[0].end(),0);
        assert(e==accumulate(cnt[1].begin(),cnt[1].end(),0));
        vector<int> target(k,e/k);for(int c=0;c<e%k;c++)target[c]++;
        equalize(p[0],cnt[0],target);equalize(p[1],cnt[1],target);
        vector<int> mapB(k);iota(mapB.begin(),mapB.end(),0);
        int r=e%k;
        shuffle(mapB.begin(),mapB.begin()+r,rng);shuffle(mapB.begin()+r,mapB.end(),rng);
        vector<int> colors(k);iota(colors.begin(),colors.end(),0);shuffle(colors.begin(),colors.end(),rng);
        for(int c:colors) {
            vector<int> a,b;
            for(int v=0;v<m;v++) {
                if(p[0][c][v]==-1)a.push_back(v);
                if(p[1][mapB[c]][v]==-1)b.push_back(v);
            }
            int s=a.size();assert(s==(int)b.size());
            shuffle(a.begin(),a.end(),rng);shuffle(b.begin(),b.end(),rng);
            vector<vector<int>> adj(s);
            for(int i=0;i<s;i++)for(int j=0;j<s;j++)if(ans[order[a[i]]*n+order[m+b[j]]]==0)adj[i].push_back(j);
            vector<int> match(s,-1),seen(s,0);int stamp=0;
            auto dfs=[&](auto&& self,int v)->bool {
                for(int u:adj[v])if(seen[u]!=stamp) {
                    seen[u]=stamp;
                    if(match[u]==-1 || self(self,match[u])){match[u]=v;return true;}
                }
                return false;
            };
            for(int i=0;i<s;i++){++stamp;if(!dfs(dfs,i))return false;}
            for(int j=0;j<s;j++)setColor(order[a[match[j]]],order[m+b[j]],c+1);
        }
        for(int c=0;c<k;c++)for(int u=0;u<m;u++) {
            int v=p[0][c][u];if(v>u)setColor(order[u],order[v],c+1);
            v=p[1][mapB[c]][u];if(v>u)setColor(order[m+u],order[m+v],c+1);
        }
        residual.clear();residual.reserve(m*(n-3-k));
        for(int u=0;u<m;u++)for(int v=0;v<m;v++)if(ans[order[u]*n+order[m+v]]==0)residual.push_back((u<<12)|v);
        assert((int)residual.size()==m*(n-3-k));
        return true;
    }

    pair<vector<int>,vector<int>> split(const vector<int>& edges) {
        int E=edges.size();
        vector<int> head(2*m,-1),next(2*E);
        for(int i=0;i<E;i++) {
            int u=edges[i]>>12,v=(edges[i]&4095)+m;
            next[2*i]=head[u];head[u]=2*i;
            next[2*i+1]=head[v];head[v]=2*i+1;
        }
        vector<char> used(E,0);
        vector<int> a,b,vertices,enter;
        a.reserve(E/2);b.reserve(E/2);vertices.reserve(E+1);enter.reserve(E+1);
        for(int st=0;st<2*m;st++)if(head[st]!=-1) {
            vertices.push_back(st);enter.push_back(-1);int parity=0;
            while(!vertices.empty()) {
                int v=vertices.back();int &h=head[v];
                while(h!=-1 && used[h/2])h=next[h];
                if(h==-1) {
                    int eid=enter.back();vertices.pop_back();enter.pop_back();
                    if(eid!=-1){(parity?a:b).push_back(edges[eid]);parity^=1;}
                }else {
                    int eid=h/2;h=next[h];if(used[eid])continue;
                    used[eid]=1;int x=edges[eid];
                    int to=(v<m?(x&4095)+m:x>>12);
                    vertices.push_back(to);enter.push_back(eid);
                }
            }
        }
        assert(a.size()==b.size());
        return {move(a),move(b)};
    }

    void factor(vector<int> edges,int d,int first) {
        if(d==0)return;
        if(d==1) {
            for(int x:edges)setColor(order[x>>12],order[m+(x&4095)],first);
            return;
        }
        if(d%2==0) {
            auto parts=split(edges);vector<int>().swap(edges);
            factor(move(parts.first),d/2,first);factor(move(parts.second),d/2,first+d/2);
            return;
        }
        vector<vector<int>> adj(m);
        for(auto& a:adj)a.reserve(d);
        for(int x:edges)adj[x>>12].push_back(x&4095);
        vector<int> pu(m,-1),pv(m,-1),dist(m),ptr(m),q(m);
        int matched=0;
        for(int u=0;u<m;u++)for(int v:adj[u])if(pv[v]==-1){pu[u]=v;pv[v]=u;matched++;break;}
        while(matched<m) {
            int qh=0,qt=0;fill(dist.begin(),dist.end(),-1);fill(ptr.begin(),ptr.end(),0);
            for(int u=0;u<m;u++)if(pu[u]==-1){dist[u]=0;q[qt++]=u;}
            while(qh<qt) {
                int u=q[qh++];
                for(int v:adj[u])if(pv[v]!=-1 && dist[pv[v]]==-1){dist[pv[v]]=dist[u]+1;q[qt++]=pv[v];}
            }
            auto dfs=[&](auto&& self,int u)->bool {
                for(int &j=ptr[u];j<(int)adj[u].size();j++) {
                    int v=adj[u][j];
                    if(pv[v]==-1 || (dist[pv[v]]==dist[u]+1 && self(self,pv[v]))) {pu[u]=v;pv[v]=u;return true;}
                }
                dist[u]=-1;return false;
            };
            int before=matched;
            for(int u=0;u<m;u++)if(pu[u]==-1 && dfs(dfs,u))matched++;
            assert(before<matched);
        }
        for(int u=0;u<m;u++)setColor(order[u],order[m+pu[u]],first);
        int out=0;for(int x:edges)if(pu[x>>12]!=(x&4095))edges[out++]=x;
        edges.resize(out);
        vector<vector<int>>().swap(adj);
        factor(move(edges),d-1,first+1);
    }
    void solve() {
        if(n<80){small();return;}
        vector<int> residual;int k;
        while(!prepare(residual,k)){}
        factor(move(residual),n-3-k,k+1);
    }
};

int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int original;cin>>original;
    vector<string> g(original);for(auto& s:g)cin>>s;
    int aa=-1,bb=-1;
    if(original%2) {
        // Subdivide one edge of the complementary 2-factor. The resulting
        // even-order graph is regular of degree original-2. Restricting
        // its coloring gives the optimal coloring of the original graph.
        for(int j=1;j<original;j++)if(g[0][j]=='0'){aa=0;bb=j;break;}
        for(int i=0;i<original;i++)g[i].push_back(i==aa || i==bb?'0':'1');
        g.push_back(string(original+1,'1'));g.back()[aa]=g.back()[bb]=g.back()[original]='0';
        g[aa][bb]=g[bb][aa]='1';
    }
    Solver solver(move(g));solver.solve();
    if(aa!=-1)solver.setColor(aa,bb,-1);
    cout<<solver.n-3<<'\n';
    for(int i=0;i<original;i++){
        for(int j=0;j<original;j++){if(j)cout<<' ';cout<<solver.ans[i*solver.n+j];}
        cout<<'\n';
    }
    return 0;
}
