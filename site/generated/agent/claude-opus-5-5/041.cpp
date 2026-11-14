#include <bits/stdc++.h>
using namespace std;
int n;
struct Tr{
    bool A[101][101];
    vector<array<int,10>> ops;
    bool check=true;
    void op(int x,int y,int z,int w,int a1,int b1,int a2,int b2,int a3,int b3){
        if(check){ if(!(A[x][y]&&A[y][z]&&A[z][w])) {fprintf(stderr,"BAD PATH\n"); exit(1);} }
        A[x][y]=A[y][x]=A[y][z]=A[z][y]=A[z][w]=A[w][z]=0;
        A[a1][b1]=A[b1][a1]=A[a2][b2]=A[b2][a2]=A[a3][b3]=A[b3][a3]=1;
        ops.push_back({x,y,z,w,a1,b1,a2,b2,a3,b3});
    }
    vector<int> par,dep;
    void bfs(int c){
        par.assign(n+1,0);dep.assign(n+1,-1);
        queue<int> q;q.push(c);dep[c]=0;
        while(!q.empty()){int v=q.front();q.pop();for(int u=1;u<=n;u++)if(A[v][u]&&dep[u]<0){dep[u]=dep[v]+1;par[u]=v;q.push(u);}}
    }
    int deg(int v){int d=0;for(int u=1;u<=n;u++)d+=A[v][u];return d;}
    int child(int v){for(int u=1;u<=n;u++)if(A[v][u]&&u!=par[v])return u;return 0;}
};
Tr S,T;
int main(){
    scanf("%d",&n);
    memset(S.A,0,sizeof S.A);memset(T.A,0,sizeof T.A);
    for(int i=0;i<n-1;i++){int a,b;scanf("%d %d",&a,&b);S.A[a][b]=S.A[b][a]=1;}
    for(int i=0;i<n-1;i++){int a,b;scanf("%d %d",&a,&b);T.A[a][b]=T.A[b][a]=1;}
    bool same=true;
    for(int i=1;i<=n;i++)for(int j=1;j<=n;j++)if(S.A[i][j]!=T.A[i][j])same=false;
    if(same){puts("YES\n0");return 0;}
    int sc=0,tc=0;
    for(int v=1;v<=n;v++){if(S.deg(v)==n-1)sc=v;if(T.deg(v)==n-1)tc=v;}
    if(sc){puts("NO");return 0;}
    int c=tc?tc:1;int target=tc?0:1;
    // pull S
    while(true){
        S.bfs(c);int k=0;for(int v=1;v<=n;v++)if(S.dep[v]>=2)k++;
        if(k<=target)break;
        if(S.deg(c)>=2){
            int v=0;for(int x=1;x<=n;x++)if(S.dep[x]==2){v=x;break;}
            int u=S.par[v];int q=0;for(int x=1;x<=n;x++)if(S.A[c][x]&&x!=u){q=x;break;}
            S.op(v,u,c,q,v,c,u,c,q,c);
        }else{
            int v=0,w=0;for(int x=1;x<=n;x++)if(S.dep[x]==3){w=x;v=S.par[x];break;}
            int u=S.par[v];
            S.op(w,v,u,c,w,v,v,c,u,c);
        }
    }
    if(!tc){
        S.bfs(c);int d0=0;for(int v=1;v<=n;v++)if(S.dep[v]==2)d0=v;
        int e0=S.par[d0];
        // T reverse
        vector<array<int,10>> rev;
        while(true){
            T.bfs(c);int w=0;for(int x=1;x<=n;x++)if(T.dep[x]==3){w=x;break;}
            if(!w)break;
            int v=T.par[w],u=T.par[v];
            T.A[w][v]=T.A[v][w]=T.A[v][u]=T.A[u][v]=T.A[u][c]=T.A[c][u]=0;
            T.A[w][u]=T.A[u][w]=T.A[u][c]=T.A[c][u]=T.A[c][v]=T.A[v][c]=1;
            rev.push_back({w,u,c,v,w,v,v,u,u,c});
        }
        T.bfs(c);
        vector<vector<int>> K(n+1);
        for(int x=1;x<=n;x++)if(T.dep[x]==2)K[T.par[x]].push_back(x);
        vector<int> heads;
        int last=0;
        if(T.dep[d0]==2)last=T.par[d0];
        else if(!K[d0].empty())last=d0;
        for(int y=1;y<=n;y++)if(!K[y].empty()&&y!=last)heads.push_back(y);
        int p=e0;
        auto anyq=[&](int ex){for(int x=1;x<=n;x++)if(S.A[c][x]&&x!=ex)return x;return 0;};
        bool retd0=(last==0);
        auto process=[&](int y,bool fin){
            if(y!=p){S.op(d0,p,c,y,p,c,c,y,y,d0);p=y;}
            vector<int> L;for(int x:K[y])if(x!=d0)L.push_back(x);
            for(int i=0;i<(int)L.size();i++){int x=L[i];
                if(fin&&i+1==(int)L.size())S.op(x,c,y,d0,c,y,y,x,c,d0);
                else S.op(x,c,y,d0,c,y,y,d0,y,x);}
        };
        for(int i=0;i<(int)heads.size();i++)process(heads[i],retd0&&i+1==(int)heads.size());
        if(last && last!=d0)process(last,false);
        else if(last==d0){
            auto &Kd=K[d0];
            int start;
            if(find(Kd.begin(),Kd.end(),p)!=Kd.end()){
                int q=anyq(p);
                S.op(d0,p,c,q,c,d0,d0,p,c,q);start=p;
            }else{
                int x=Kd[0];
                S.op(x,c,p,d0,c,p,c,d0,d0,x);start=x;
            }
            for(int x:Kd)if(x!=start)S.op(x,c,d0,start,c,d0,d0,start,d0,x);
        }
        // now S should equal T (C_T)
        for(int i=(int)rev.size()-1;i>=0;i--){auto&r=rev[i];S.op(r[0],r[1],r[2],r[3],r[4],r[5],r[6],r[7],r[8],r[9]);
            T.A[r[0]][r[1]]=T.A[r[1]][r[0]]=T.A[r[1]][r[2]]=T.A[r[2]][r[1]]=T.A[r[2]][r[3]]=T.A[r[3]][r[2]]=0;
            T.A[r[4]][r[5]]=T.A[r[5]][r[4]]=T.A[r[6]][r[7]]=T.A[r[7]][r[6]]=T.A[r[8]][r[9]]=T.A[r[9]][r[8]]=1;}
    }
    for(int i=1;i<=n;i++)for(int j=1;j<=n;j++)if(S.A[i][j]!=T.A[i][j]){fprintf(stderr,"MISMATCH\n");return 1;}
    string out="YES\n"+to_string(S.ops.size())+"\n";
    for(auto&o:S.ops){for(int i=0;i<4;i++)out+=to_string(o[i])+(i<3?" ":"\n");for(int i=4;i<10;i++)out+=to_string(o[i])+(i<9?" ":"\n");}
    fputs(out.c_str(),stdout);
}
