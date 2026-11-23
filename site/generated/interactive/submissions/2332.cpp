#include <bits/stdc++.h>
using namespace std;

int N, M;
vector<int> par, pr, s0, s1;

int findr(int x){
    if(par[x]==x) return x;
    int r=findr(par[x]);
    pr[x]^=pr[par[x]];
    par[x]=r;
    return r;
}
void unite(int x,int y,int d){
    int rx=findr(x), ry=findr(y);
    if(rx==ry) return;
    int t=pr[x]^pr[y]^d;
    if(s0[rx]+s1[rx] < s0[ry]+s1[ry]) swap(rx,ry);
    par[ry]=rx; pr[ry]=t;
    if(t==0){ s0[rx]+=s0[ry]; s1[rx]+=s1[ry]; }
    else { s0[rx]+=s1[ry]; s1[rx]+=s0[ry]; }
}
int ask(int x,int y){
    printf("? %d %d\n",x,y);
    fflush(stdout);
    int r;
    if(scanf("%d",&r)!=1) exit(0);
    if(r==-1) exit(0);
    return r;
}

vector<int> answerCoins;
// returns undetermined roots; if empty, answerCoins filled
vector<int> analyze(){
    vector<int> roots;
    for(int i=1;i<=N;i++) if(findr(i)==i) roots.push_back(i);
    int K=roots.size();
    vector<array<int,11>> pre(K+1), suf(K+2);
    for(auto&a:pre) a.fill(0);
    for(auto&a:suf) a.fill(0);
    pre[0][0]=1;
    for(int i=0;i<K;i++){
        int p=s0[roots[i]], q=s1[roots[i]];
        for(int s=0;s<=M;s++) if(pre[i][s]){
            if(s+p<=M) pre[i+1][s+p]=min(2,pre[i+1][s+p]+pre[i][s]);
            if(s+q<=M) pre[i+1][s+q]=min(2,pre[i+1][s+q]+pre[i][s]);
        }
    }
    suf[K][0]=1;
    for(int i=K-1;i>=0;i--){
        int p=s0[roots[i]], q=s1[roots[i]];
        for(int s=0;s<=M;s++) if(suf[i+1][s]){
            if(s+p<=M) suf[i][s+p]=min(2,suf[i][s+p]+suf[i+1][s]);
            if(s+q<=M) suf[i][s+q]=min(2,suf[i][s+q]+suf[i+1][s]);
        }
    }
    vector<int> und;
    vector<int> choice(K,-1);
    for(int i=0;i<K;i++){
        int p=s0[roots[i]], q=s1[roots[i]];
        bool fa=false, fb=false;
        for(int s=0;s<=M;s++) if(pre[i][s]){
            if(s+p<=M && suf[i+1][M-s-p]) fa=true;
            if(s+q<=M && suf[i+1][M-s-q]) fb=true;
        }
        if(fa&&fb) und.push_back(roots[i]);
        else if(fa) choice[i]=0;
        else choice[i]=1;
    }
    if(und.empty()){
        answerCoins.clear();
        map<int,int> ch;
        for(int i=0;i<K;i++) ch[roots[i]]=choice[i];
        for(int c=1;c<=N;c++){
            int r=findr(c);
            if(pr[c]==ch[r]) answerCoins.push_back(c);
        }
    }
    return und;
}

int main(){
    if(scanf("%d %d",&N,&M)!=2) return 0;
    par.resize(N+1); pr.assign(N+1,0); s0.assign(N+1,1); s1.assign(N+1,0);
    for(int i=0;i<=N;i++) par[i]=i;
    vector<int> order(N);
    iota(order.begin(),order.end(),1);
    mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count());
    shuffle(order.begin(),order.end(),rng);
    int idx=0;
    int pending=-1;
    while(true){
        auto und=analyze();
        if(und.empty()){
            printf("!");
            for(int c:answerCoins) printf(" %d",c);
            printf("\n");
            fflush(stdout);
            return 0;
        }
        if(pending!=-1 && idx<N){
            int x=order[idx++];
            int r=ask(pending,x);
            unite(pending,x,r);
            pending=-1;
            continue;
        }
        if(idx+1<N){
            int x=order[idx++], y=order[idx++];
            int r=ask(x,y);
            unite(x,y,r);
            if(r==1) pending=x;
            continue;
        }
        pending=-1;
        if(und.size()>=2){
            sort(und.begin(),und.end(),[&](int a,int b){
                return s0[a]+s1[a] < s0[b]+s1[b];
            });
            int a=und[0], b=und[1];
            int r=ask(a,b);
            unite(a,b,r);
        } else {
            int a=und[0];
            int b=-1;
            for(int i=1;i<=N;i++) if(findr(i)==i && i!=a){ b=i; break; }
            if(b==-1) return 0;
            int r=ask(a,b);
            unite(a,b,r);
        }
    }
}
