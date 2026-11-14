#include <bits/stdc++.h>
using namespace std;
int M,P;
vector<int16_t> colm, atm; // colm[u*M+v], atm[u*M+c]
inline int16_t& COL(int u,int v){return colm[(size_t)u*M+v];}
inline int16_t& AT(int u,int c){return atm[(size_t)u*M+c];}
mt19937 rng(12345);
vector<int> pathv;
// walk chain from s starting with color a, alternating a,b; fill pathv
void walk(int s,int a,int b){
    pathv.clear(); pathv.push_back(s);
    int cur=s,c=a;
    while(true){int nx=AT(cur,c); if(nx<0)break; pathv.push_back(nx); cur=nx; c=(c==a?b:a);}
}
void flipPath(int a,int b){
    int k=pathv.size();
    for(int i=0;i+1<k;i++){int x=pathv[i],y=pathv[i+1];int c=COL(x,y);AT(x,c)=-1;AT(y,c)=-1;}
    for(int i=0;i+1<k;i++){int x=pathv[i],y=pathv[i+1];int c=COL(x,y);int nc=(c==a?b:a);COL(x,y)=COL(y,x)=nc;}
    for(int i=0;i+1<k;i++){int x=pathv[i],y=pathv[i+1];int c=COL(x,y);AT(x,c)=y;AT(y,c)=x;}
}
void setc(int u,int v,int c){COL(u,v)=COL(v,u)=c;AT(u,c)=v;AT(v,c)=u;}
int main(){
    int N; if(scanf("%d",&N)!=1)return 0;
    vector<string> S(N);
    {static char buf[4000]; for(int i=0;i<N;i++){scanf("%s",buf);S[i]=buf;}}
    M = N + (N%2);
    vector<array<int,2>> hn(M,{-1,-1});
    for(int i=0;i<N;i++){int k=0;for(int j=0;j<N;j++) if(j!=i&&S[i][j]=='0') hn[i][k++]=j;}
    int ou=-1,ow=-1;
    if(N%2){
        int x=N; ou=0; ow=hn[0][0];
        hn[ou][0]=x; // replace w by x
        if(hn[ow][0]==ou) hn[ow][0]=x; else hn[ow][1]=x;
        hn[x]={ou,ow};
    }
    // cycles order
    vector<int> order; vector<char> vis(M,0); vector<pair<int,int>> chords;
    for(int s=0;s<M;s++) if(!vis[s]){
        int prev=-1,cur=s; int st=order.size();
        while(!vis[cur]){vis[cur]=1;order.push_back(cur);int nx=(hn[cur][0]!=prev?hn[cur][0]:hn[cur][1]); if(hn[cur][0]==hn[cur][1]) nx=hn[cur][0]; prev=cur;cur=nx;}
    }
    // GK on n' = M-1 plus inf = M-1
    int np=M-1, INF=M-1;
    auto gk=[&](int a,int b)->int{ if(a==INF) return (2*b)%np; if(b==INF) return (2*a)%np; return (a+b)%np;};
    // Ham cycle of colors 0 and 1
    vector<int> g; g.reserve(M);
    {
        int cur=INF; int c=0; int inv2=(np+1)/2;
        for(int i=0;i<M;i++){
            g.push_back(cur);
            int nx;
            if(c==0){ nx = (cur==INF?0:(cur==0?INF:(np-cur)%np)); }
            else { if(cur==INF) nx=inv2; else if(cur==inv2) nx=INF; else nx=((1-cur)%np+np)%np; }
            cur=nx; c^=1;
        }
    }
    vector<int> lab(M);
    for(int i=0;i<M;i++) lab[order[i]]=g[i];
    P=M-3;
    colm.assign((size_t)M*M,-1); atm.assign((size_t)M*M,-1);
    vector<pair<int,int>> unc;
    for(int i=0;i<M;i++) for(int j=i+1;j<M;j++){
        if(hn[i][0]==j||hn[i][1]==j) continue;
        int c=gk(lab[i],lab[j]);
        if(c<2){unc.push_back({i,j});continue;}
        setc(i,j,c-2);
    }
    vector<int> mu,mv;
    auto miss=[&](int u,vector<int>&r){r.clear();for(int c=0;c<P;c++) if(AT(u,c)<0) r.push_back(c);};
    while(!unc.empty()){
        auto [u,v]=unc.back();
        miss(u,mu); miss(v,mv);
        bool done=false;
        for(int a:mu){ for(int b:mv) if(a==b){setc(u,v,a);done=true;break;} if(done)break;}
        if(!done){
            for(int a:mu){ for(int b:mv){
                walk(v,a,b);
                if(pathv.back()==u) continue;
                flipPath(a,b); setc(u,v,a); done=true; break;
            } if(done)break;}
        }
        if(done){unc.pop_back();continue;}
        // perturb
        int w = (rng()&1)?u:v;
        vector<int>& mw = (w==u)?mu:mv;
        int a=mw[rng()%mw.size()];
        int c; do{c=rng()%P;}while(AT(w,c)<0);
        walk(w,c,a); flipPath(c,a);
    }
    // output
    string out; out.reserve((size_t)N*N*5+64);
    out+=to_string(P); out+='\n';
    char tmp[16];
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
            int c;
            if(i==j||S[i][j]=='0') c=-1; else c=COL(i,j)+1;
            if(c<0){out+="-1";} else {int l=0;while(c){tmp[l++]='0'+c%10;c/=10;} while(l)out+=tmp[--l];}
            out+=(j+1<N?' ':'\n');
        }
    }
    fwrite(out.data(),1,out.size(),stdout);
}
