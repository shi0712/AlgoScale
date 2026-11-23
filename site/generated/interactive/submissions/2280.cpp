#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
static int n;
static int ask(int i,int j){printf("? %d %d\n",i,j);fflush(stdout);int x;if(scanf("%d",&x)!=1)exit(0);if(x<0)exit(0);return x;}
struct Rep{int k,last,w;};
int main(){
    if(scanf("%d",&n)!=1)return 0;
    if(n<=2){printf("!\n");fflush(stdout);
        if(n==1){printf("1\n");fflush(stdout);printf("0\n");fflush(stdout);}
        else{printf("2\n");fflush(stdout);printf("0 1\n");fflush(stdout);}return 0;}
    const int m=n-1; vector<int> a(m);
    for(int i=0;i<m;++i)a[i]=ask(i,0);
    int LIM=1;while(LIM<n)LIM<<=1;
    vector<int> posA(LIM,-1); for(int i=0;i<m;++i)posA[a[i]]=i;
    int Xall=0;for(int v=0;v<n;++v)Xall^=v; int Xa=0;for(int i=0;i<m;++i)Xa^=a[i];
    const int a0=a[0];
    vector<Rep> reps;
    { mt19937_64 rng((ull)chrono::steady_clock::now().time_since_epoch().count()^0x9e3779b97f4a7c15ULL);
      vector<ull> R1(n),R2(n); for(int j=0;j<n;++j){R1[j]=rng();R2[j]=rng();}
      ull RL1=rng(),RL2=rng(); map<pair<ull,ull>,int> gid; vector<int> p(n),b(n);
      for(int k=0;k<n;++k){ bool ok=true;
        for(int i=0;i<m;++i){if((a[i]^k)>=n){ok=false;break;}} if(!ok)continue;
        int last=Xall^Xa^((m&1)?k:0);
        if(k<m){if(a[k]!=k)continue;} else if(last!=0)continue;
        for(int i=0;i<m;++i)p[i]=a[i]^k; p[n-1]=last;
        for(int i=0;i<n;++i)b[p[i]]=i;
        ull h1=RL1*(ull)(last^k),h2=RL2*(ull)(last^k); int p0=p[0];
        for(int j=0;j<n;++j){ull c=(ull)(p0^b[j]);h1+=R1[j]*c;h2+=R2[j]*c;}
        pair<ull,ull> key(h1,h2); auto it=gid.find(key);
        if(it==gid.end()){gid.emplace(key,(int)reps.size());reps.push_back(Rep{k,last,1});}
        else reps[it->second].w++; } }
    if(reps.empty())return 0;
    vector<int> bw1(LIM,0),bc1(LIM,0),bw2(LIM,0),bc2(LIM,0),t1,t2;
    t1.reserve(LIM);t2.reserve(LIM);
    vector<double> cost(2,0.0); long long work=0; const long long WLIM=60000000LL;
    auto answerOf=[&](const Rep&r,int type,int j)->int{
        int idx=posA[j^r.k]; int bj=(idx>=0?idx:n-1);
        return (type==0)?((a0^r.k)^bj):(r.last^bj);};
    while(reps.size()>1){
        int R=(int)reps.size();
        while((int)cost.size()<=R){int c=(int)cost.size();cost.push_back(1.0+log2((double)c));}
        int bestType=-1,bestJ=-1,bestMax=INT_MAX; double bestScore=1e18; bool finish=false;
        if(work<=WLIM){
            for(int j=0;j<n&&!finish;++j){
                t1.clear();t2.clear();
                for(const Rep&r:reps){
                    int idx=posA[j^r.k]; int bj=(idx>=0?idx:n-1);
                    int v1=(a0^r.k)^bj, v2=r.last^bj;
                    if(bw1[v1]==0)t1.push_back(v1); bw1[v1]+=r.w; ++bc1[v1];
                    if(bw2[v2]==0)t2.push_back(v2); bw2[v2]+=r.w; ++bc2[v2]; }
                work+=2LL*R;
                if((int)t1.size()>=2){ double sc=0;int mx=0;
                    for(int v:t1){sc+=(double)bw1[v]*cost[bc1[v]];if(bw1[v]>mx)mx=bw1[v];}
                    if(sc<bestScore-1e-9||(sc<=bestScore+1e-9&&mx<bestMax)){bestScore=sc;bestMax=mx;bestType=0;bestJ=j;}
                    if((int)t1.size()==R)finish=true; }
                if(!finish&&(int)t2.size()>=2){ double sc=0;int mx=0;
                    for(int v:t2){sc+=(double)bw2[v]*cost[bc2[v]];if(bw2[v]>mx)mx=bw2[v];}
                    if(sc<bestScore-1e-9||(sc<=bestScore+1e-9&&mx<bestMax)){bestScore=sc;bestMax=mx;bestType=1;bestJ=j;}
                    if((int)t2.size()==R)finish=true; }
                for(int v:t1){bw1[v]=0;bc1[v]=0;} for(int v:t2){bw2[v]=0;bc2[v]=0;} } }
        if(bestType<0){
            for(int j=0;j<n&&bestType<0;++j){
                if(answerOf(reps[0],0,j)!=answerOf(reps[1],0,j)){bestType=0;bestJ=j;}
                else if(answerOf(reps[0],1,j)!=answerOf(reps[1],1,j)){bestType=1;bestJ=j;} }
            if(bestType<0)break; }
        int qi=(bestType==0?0:n-1);
        int ans=ask(qi,bestJ);
        vector<Rep> nr; nr.reserve(reps.size());
        for(const Rep&r:reps) if(answerOf(r,bestType,bestJ)==ans) nr.push_back(r);
        reps.swap(nr);
        if(reps.empty())return 0; }
    const Rep r=reps[0]; vector<int> p(n);
    for(int i=0;i<m;++i)p[i]=a[i]^r.k; p[n-1]=r.last;
    printf("!\n");fflush(stdout);
    printf("%d\n",r.w);fflush(stdout);
    for(int i=0;i<n;++i)printf("%d%c",p[i],(i+1<n)?' ':'\n');
    fflush(stdout);
    return 0;
}
