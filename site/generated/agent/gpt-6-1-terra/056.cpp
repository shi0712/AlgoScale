#include <bits/stdc++.h>
using namespace std;

struct Blossom {
    int n, timer=0, lcatimer=0;
    vector<vector<int>> adj;
    vector<int> match, par, base, seen, used, mark, lcaMark, q;
    Blossom(int n):n(n),adj(n),match(n,-1),par(n),base(n),seen(n),used(n),mark(n),lcaMark(n) {q.reserve(n);}
    void add(int a,int b){adj[a].push_back(b);adj[b].push_back(a);}
    inline void touch(int x){if(seen[x]!=timer){seen[x]=timer;base[x]=x;par[x]=-1;used[x]=0;}}
    int lca(int a,int b){
        ++lcatimer;
        while(true){a=base[a];lcaMark[a]=lcatimer;if(match[a]==-1)break;a=par[match[a]];}
        while(true){b=base[b];if(lcaMark[b]==lcatimer)return b;b=par[match[b]];}
    }
    void markPath(int v,int b,int child){
        while(base[v]!=b){
            mark[base[v]]=mark[base[match[v]]]=timer;
            par[v]=child;child=match[v];v=par[match[v]];
        }
    }
    bool augment(int root){
        ++timer;q.clear();touch(root);used[root]=1;q.push_back(root);
        for(size_t qi=0;qi<q.size();++qi){
            int v=q[qi];
            for(int u:adj[v]){
                touch(u);
                if(base[v]==base[u] || match[v]==u)continue;
                if(u==root || (match[u]!=-1 && (touch(match[u]),par[match[u]]!=-1))){
                    int b=lca(v,u);
                    markPath(v,b,u);markPath(u,b,v);
                    // All vertices in the current alternating forest are in q or
                    // are matched to a vertex in q.
                    size_t old=q.size();
                    for(size_t i=0;i<old;i++){
                        int x=q[i];
                        if(mark[base[x]]==timer){base[x]=b;}
                        int y=match[x];
                        if(y!=-1 && mark[base[y]]==timer){
                            base[y]=b;
                            if(!used[y]){used[y]=1;q.push_back(y);}
                        }
                    }
                }else if(par[u]==-1){
                    par[u]=v;
                    if(match[u]==-1){
                        int x=u;
                        while(x!=-1){int pv=par[x];int next=pv==-1?-1:match[pv];match[x]=pv;if(pv!=-1)match[pv]=x;x=next;}
                        return true;
                    }
                    int w=match[u];touch(w);used[w]=1;q.push_back(w);
                }
            }
        }
        return false;
    }
};
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int n,m;if(!(cin>>n>>m))return 0;
    vector<string>a(n);for(auto &s:a)cin>>s;
    vector<int> id(n*m,-1), centers;
    int N=0;
    for(int i=0;i<n*m;i++)if(a[i/m][i%m]=='.')id[i]=N++;
    int dots=N;
    for(int i=0;i<n*m;i++)if(a[i/m][i%m]!='.'){
        int r=i/m,c=i%m,vertical=0,horizontal=0;
        if(r>0 && a[r-1][c]=='.')++vertical;
        if(r+1<n && a[r+1][c]=='.')++vertical;
        if(c>0 && a[r][c-1]=='.')++horizontal;
        if(c+1<m && a[r][c+1]=='.')++horizontal;
        if((a[r][c]=='*' && vertical && horizontal) || (a[r][c]=='+' && vertical+horizontal>=2)){
            id[i]=N;N+=2;centers.push_back(i);
        }
    }
    Blossom bm(N);
    int dr[4]={-1,1,0,0},dc[4]={0,0,-1,1};
    for(int c:centers){int r=c/m,col=c%m,x=id[c];bm.add(x,x+1);
        for(int k=0;k<4;k++){int rr=r+dr[k],cc=col+dc[k];if(rr<0||rr>=n||cc<0||cc>=m||a[rr][cc]!='.')continue;int d=id[rr*m+cc];
            if(a[r][col]=='+'){bm.add(x,d);bm.add(x+1,d);}else bm.add(x+(k>=2),d);
        }
    }
    // Start with complete trominoes, keeping all center vertices matched.
    for(int c:centers){int x=id[c],p=-1,z=-1;
        for(int v:bm.adj[x])if(v<dots && bm.match[v]==-1){
            for(int w:bm.adj[x+1])if(w<dots && w!=v && bm.match[w]==-1){p=v;z=w;break;}
            if(p!=-1)break;
        }
        if(p!=-1){bm.match[x]=p;bm.match[p]=x;bm.match[x+1]=z;bm.match[z]=x+1;}
        else {bm.match[x]=x+1;bm.match[x+1]=x;}
    }
    vector<int> component(N,-1), total, filled, queue;
    for(int v=0;v<N;v++) if(component[v]<0){
        int k=total.size();total.push_back(0);filled.push_back(0);
        queue.clear();queue.push_back(v);component[v]=k;
        for(size_t i=0;i<queue.size();i++){
            int u=queue[i];
            for(int w:bm.adj[u])if(component[w]<0){component[w]=k;queue.push_back(w);}
        }
    }
    for(int c:centers){int x=id[c],k=component[x];++total[k];if(bm.match[x]!=x+1)++filled[k];}
    for(int v=0;v<dots;v++)if(bm.match[v]==-1){
        int k=component[v];
        if(filled[k]<total[k] && bm.augment(v))++filled[k];
    }
    vector<int> pos(dots);for(int i=0;i<n*m;i++)if(a[i/m][i%m]=='.')pos[id[i]]=i;
    for(int c:centers){int x=id[c];if(bm.match[x]==x+1)continue;
        int cells[3]={c,pos[bm.match[x]],pos[bm.match[x+1]]};bool forbidden[26]={};
        for(int t:cells){int r=t/m,col=t%m;for(int k=0;k<4;k++){int rr=r+dr[k],cc=col+dc[k];if(rr>=0&&rr<n&&cc>=0&&cc<m&&a[rr][cc]>='a'&&a[rr][cc]<='z')forbidden[a[rr][cc]-'a']=true;}}
        int color=0;while(forbidden[color])++color;
        for(int t:cells)a[t/m][t%m]=char('a'+color);
    }
    for(auto&s:a)cout<<s<<'\n';
}
