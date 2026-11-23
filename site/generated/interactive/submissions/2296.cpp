#include <bits/stdc++.h>
using namespace std;

static deque<pair<int,int>> belt;
static int curCnt=0;
static int K=0;
static vector<int> reps, cls;

static int opPush(int id,int cl){
    printf("+ %d\n",id); fflush(stdout);
    int c; if(scanf("%d",&c)!=1) exit(0); if(c<0) exit(0);
    belt.push_back(make_pair(id,cl)); curCnt=c; return c;
}
static int opPop(){
    printf("-\n"); fflush(stdout);
    int c; if(scanf("%d",&c)!=1) exit(0); if(c<0) exit(0);
    if(!belt.empty()) belt.pop_front(); curCnt=c; return c;
}
static void classifyBatch(const vector<int>& items){
    int m=(int)items.size(); if(m==0) return;
    vector<int> lo(m,0),hi(m,K),act(m); iota(act.begin(),act.end(),0);
    auto computeT=[&](){
        vector<char> inT(K,0);
        for(int i:act){int mid=(lo[i]+hi[i])/2; for(int j=lo[i];j<mid;j++) inT[j]=1;}
        vector<int> T; for(int j=0;j<K;j++) if(inT[j]) T.push_back(j); return T;
    };
    vector<int> T=computeT();
    for(int j:T) opPush(reps[j],j);
    while(true){
        vector<int> cur=act;
        for(int i:cur){int pc=curCnt; opPush(items[i],-1); int mid=(lo[i]+hi[i])/2;
            if(curCnt>pc) lo[i]=mid; else hi[i]=mid;}
        vector<int> na; for(int i:cur) if(hi[i]-lo[i]>1) na.push_back(i); act=na;
        if(act.empty()){ while(!belt.empty()) opPop(); break; }
        vector<int> T2=computeT();
        for(int j:T2) opPush(reps[j],j);
        for(size_t k=0;k<T.size();k++) opPop();
        for(int i:cur){int pc=curCnt; opPop();
            if(hi[i]-lo[i]>1){int mid=(lo[i]+hi[i])/2; if(curCnt==pc) hi[i]=mid; else lo[i]=mid;}}
        vector<int> na2; for(int i:act) if(hi[i]-lo[i]>1) na2.push_back(i); act=na2;
        if(act.empty()){ while(!belt.empty()) opPop(); break; }
        vector<int> T3=computeT();
        for(size_t k=0;k<T2.size();k++) opPop();
        for(int j:T3) opPush(reps[j],j);
        T=T3;
    }
    for(int i=0;i<m;i++) cls[items[i]]=lo[i];
}
static void runScan(const vector<int>& todo){
    vector<int> cnt(K,0),freq(K,1),rep=reps,missing(K);
    iota(missing.begin(),missing.end(),0);
    for(int x:todo){
        int before=curCnt; opPush(x,-1); int found=-1;
        if(curCnt>before){
            sort(missing.begin(),missing.end(),[&](int a,int b){return freq[a]>freq[b];});
            while(missing.size()>1){
                int cl=missing.front(); int pc=curCnt;
                opPush(rep[cl],cl); cnt[cl]++; missing.erase(missing.begin());
                if(curCnt==pc){found=cl;break;}
            }
            if(found<0){found=missing.front(); missing.erase(missing.begin());}
        } else {
            while(found<0 && !belt.empty()){
                pair<int,int> p=belt.front(); int pc=curCnt; opPop();
                int c=p.second; if(c<0) continue;
                cnt[c]--;
                if(cnt[c]==0){ if(curCnt==pc) found=c; else {missing.push_back(c); rep[c]=p.first;} }
            }
            if(found<0) found=0;
        }
        cls[x]=found;
        for(auto&q:belt) if(q.first==x&&q.second==-1) q.second=found;
        cnt[found]++; freq[found]++; rep[found]=x;
    }
    while(!belt.empty()) opPop();
}
int main(){
    int N; if(scanf("%d",&N)!=1) return 0;
    cls.assign(N+1,-1);
    vector<char> isLast(N+1,0);
    int prev=0;
    for(int i=1;i<=N;i++){int c=opPush(i,-1); if(c>prev){reps.push_back(i); cls[i]=(int)reps.size()-1;} prev=c;}
    K=(int)reps.size();
    if(K!=1&&K!=N){
        prev=K;
        for(int i=1;i<=N;i++){int c=opPop(); if(c<prev) isLast[i]=1; prev=c;}
    }
    if(K==1){ for(int i=1;i<=N;i++) cls[i]=0; }
    else if(K!=N){
        vector<int> todo; for(int i=1;i<=N;i++) if(cls[i]<0) todo.push_back(i);
        double t=(double)todo.size();
        double scanEst=(double)K/3.0*t;
        double batchEst=(double)N*(double)N/(2.0*K)+2.0*t*log2((double)K);
        if(scanEst<=batchEst) runScan(todo);
        else{
            vector<int> b0,pool;
            for(int i=1;i<=N;i++) if(cls[i]<0){ if(isLast[i]) b0.push_back(i); else pool.push_back(i);}
            classifyBatch(b0);
            while(!pool.empty()){
                int sz=(int)pool.size();
                vector<char> fi(sz,0),la(sz,0);
                int pc=curCnt;
                for(int i=0;i<sz;i++){int c=opPush(pool[i],-1); if(c>pc) fi[i]=1; pc=c;}
                for(int i=0;i<sz;i++){int c=opPop(); if(c<pc) la[i]=1; pc=c;}
                vector<int> bA,bB,rest;
                for(int i=0;i<sz;i++){ if(fi[i]) bA.push_back(pool[i]); else if(la[i]) bB.push_back(pool[i]); else rest.push_back(pool[i]); }
                classifyBatch(bA); classifyBatch(bB);
                pool=rest;
            }
        }
    }
    vector<vector<int>> groups(K);
    for(int i=1;i<=N;i++){int c=cls[i]; if(c<0)c=0; if(c>=K)c=K-1; groups[c].push_back(i);}
    vector<vector<int>> ans; for(auto&g:groups) if(!g.empty()) ans.push_back(g);
    printf("! %d\n",(int)ans.size());
    for(auto&g:ans){ printf("%d",(int)g.size()); for(int x:g) printf(" %d",x); printf("\n"); }
    fflush(stdout);
    return 0;
}
