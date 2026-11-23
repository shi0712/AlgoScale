#include <bits/stdc++.h>
using namespace std;

static const int MAXN = 30005;
static int n;
static vector<int> adj[MAXN], ch[MAXN];
static int par0[MAXN], sz_[MAXN], par_[MAXN], cntSub[MAXN], dst[MAXN];
static bool removedV[MAXN], inS[MAXN];
struct Entry { int offA, offB, dist, lens; };
static vector<Entry> cpTmp[MAXN];
static vector<Entry> flatE;
static int cpStart[MAXN], cpCnt[MAXN];
static vector<int> poolv;
static vector<pair<int,int> > arrList;
static int* POOL; static Entry* FE;

static void buildCentroid(int root){
    vector<int> stk; stk.push_back(root);
    vector<int> order; order.reserve(n);
    vector<int> compV, compD, compB, branchMax;
    while(!stk.empty()){
        int rt=stk.back(); stk.pop_back();
        order.clear(); order.push_back(rt); par_[rt]=-1;
        for(size_t i=0;i<order.size();++i){int u=order[i];for(int v:adj[u])if(!removedV[v]&&v!=par_[u]){par_[v]=u;order.push_back(v);} }
        int total=(int)order.size();
        for(int i=total-1;i>=0;--i){int u=order[i];int s=1;for(int v:adj[u])if(!removedV[v]&&v!=par_[u])s+=sz_[v];sz_[u]=s;}
        int c=rt;
        for(;;){int nx=-1;for(int v:adj[c])if(!removedV[v]&&v!=par_[c]&&2*sz_[v]>total){nx=v;break;}if(nx<0)break;c=nx;}
        removedV[c]=true;
        compV.clear();compD.clear();compB.clear();branchMax.clear();
        compV.push_back(c);compD.push_back(0);compB.push_back(-1);par_[c]=-1;
        for(size_t i=0;i<compV.size();++i){int u=compV[i],d=compD[i],b=compB[i];
            for(int v:adj[u])if(!removedV[v]&&v!=par_[u]){par_[v]=u;int nb;
                if(u==c){nb=(int)branchMax.size();branchMax.push_back(d+1);}else{nb=b;if(d+1>branchMax[nb])branchMax[nb]=d+1;}
                compV.push_back(v);compD.push_back(d+1);compB.push_back(nb);} }
        int maxA=0;for(int d:compD)maxA=max(maxA,d);
        int offA=(int)poolv.size(),lenA=maxA+1;
        poolv.resize(offA+lenA,0);arrList.push_back(make_pair(offA,lenA));
        int nb=(int)branchMax.size();vector<int> offB(nb),lenB(nb);
        for(int b=0;b<nb;++b){offB[b]=(int)poolv.size();lenB[b]=branchMax[b]+1;poolv.resize(offB[b]+lenB[b],0);arrList.push_back(make_pair(offB[b],lenB[b]));}
        int la=lenA-1;
        for(size_t i=0;i<compV.size();++i){int u=compV[i],d=compD[i],b=compB[i];int ob,lb;
            if(b<0){ob=0;lb=0;}else{ob=offB[b];lb=lenB[b]-1;}
            Entry en;en.offA=offA;en.offB=ob;en.dist=d;en.lens=la|(lb<<16);cpTmp[u].push_back(en);}
        for(int v:adj[c])if(!removedV[v])stk.push_back(v);
    }
}
static inline int cntQ(int u,int k){
    if(k<0)return 0;int res=0;const Entry*e=FE+cpStart[u];int c=cpCnt[u];
    for(int i=0;i<c;++i,++e){int t=k-e->dist;if(t<0)break;int la=e->lens&0xFFFF,lb=(e->lens>>16)&0xFFFF;
        res+=POOL[e->offA+(t<la?t:la)]-POOL[e->offB+(t<lb?t:lb)];}
    return res;
}
static inline void cnt4(int u,int k,int*res){
    res[0]=res[1]=res[2]=res[3]=0;const Entry*e=FE+cpStart[u];int c=cpCnt[u];
    for(int i=0;i<c;++i,++e){int t=k-e->dist;if(t<-1)break;int la=e->lens&0xFFFF,lb=(e->lens>>16)&0xFFFF;
        const int*A=POOL+e->offA;const int*B=POOL+e->offB;int t1=t+1;
        res[3]+=A[t1<la?t1:la]-B[t1<lb?t1:lb];
        if(t>=0)res[2]+=A[t<la?t:la]-B[t<lb?t:lb];
        if(t>=1){int t2=t-1;res[1]+=A[t2<la?t2:la]-B[t2<lb?t2:lb];}
        if(t>=2){int t3=t-2;res[0]+=A[t3<la?t3:la]-B[t3<lb?t3:lb];}}
}
int main(){
    if(scanf("%d",&n)!=1)return 0;
    for(int i=2;i<=n;++i){int f;if(scanf("%d",&f)!=1)return 0;par0[i]=f;adj[i].push_back(f);adj[f].push_back(i);ch[f].push_back(i);}
    vector<int> ord;ord.reserve(n);ord.push_back(1);
    for(size_t i=0;i<ord.size();++i){int u=ord[i];for(size_t j=0;j<ch[u].size();++j)ord.push_back(ch[u][j]);}
    poolv.push_back(0);arrList.push_back(make_pair(0,1));
    buildCentroid(1);
    for(int u=1;u<=n;++u){
        sort(cpTmp[u].begin(),cpTmp[u].end(),[](const Entry&a,const Entry&b){return a.dist<b.dist;});
        cpStart[u]=(int)flatE.size();cpCnt[u]=(int)cpTmp[u].size();
        for(size_t i=0;i<cpTmp[u].size();++i)flatE.push_back(cpTmp[u][i]);
        vector<Entry>().swap(cpTmp[u]);
    }
    POOL=poolv.data();FE=flatE.data();
    vector<int> S(n);for(int i=0;i<n;++i)S[i]=i+1;
    vector<int> bq;bq.reserve(n);vector<pair<int,int> > stk;stk.reserve(n);vector<int> NS;NS.reserve(n);
    while(true){
        int m=(int)S.size();
        if(m==0)return 0;
        if(m==1){printf("! %d\n",S[0]);fflush(stdout);return 0;}
        fill(poolv.begin(),poolv.end(),0);
        for(size_t idx=0;idx<S.size();++idx){int x=S[idx];const Entry*e=FE+cpStart[x];int c=cpCnt[x];
            for(int i=0;i<c;++i,++e){POOL[e->offA+e->dist]++;if((e->lens>>16)>0)POOL[e->offB+e->dist]++;}}
        for(size_t a=0;a<arrList.size();++a){int off=arrList[a].first,len=arrList[a].second;for(int i=1;i<len;++i)POOL[off+i]+=POOL[off+i-1];}
        for(int i=1;i<=n;++i)inS[i]=false;
        for(size_t idx=0;idx<S.size();++idx)inS[S[idx]]=true;
        for(int i=1;i<=n;++i)cntSub[i]=inS[i]?1:0;
        for(int i=n-1;i>=1;--i){int u=ord[i];cntSub[par0[u]]+=cntSub[u];}
        int r=1;
        for(;;){int nx=-1;for(size_t j=0;j<ch[r].size();++j)if(cntSub[ch[r][j]]==m){nx=ch[r][j];break;}if(nx<0)break;r=nx;}
        int half=(m+1)/2;int bestQ=INT_MAX,bu=-1,bk=-1;
        auto consider=[&](int u,int k,int c){if(k<0||k>n)return;if(c<=0||c>=m)return;int q=c>m-c?c:m-c;if(q<bestQ){bestQ=q;bu=u;bk=k;}};
        int lo=0,hi=n;
        while(lo<hi){int mid=(lo+hi)>>1;if(cntQ(r,mid)>=half)hi=mid;else lo=mid+1;}
        int kr=lo;
        consider(r,kr,cntQ(r,kr));
        if(kr>=1)consider(r,kr-1,cntQ(r,kr-1));
        stk.clear();stk.push_back(make_pair(r,kr));
        while(!stk.empty()){
            pair<int,int> pr=stk.back();stk.pop_back();int u=pr.first,ku=pr.second;
            for(size_t j=0;j<ch[u].size();++j){int v=ch[u][j];if(cntSub[v]<=0)continue;
                int res[4];cnt4(v,ku,res);
                consider(v,ku-2,res[0]);consider(v,ku-1,res[1]);consider(v,ku,res[2]);consider(v,ku+1,res[3]);
                int kv;if(res[1]>=half)kv=ku-1;else if(res[2]>=half)kv=ku;else kv=ku+1;
                stk.push_back(make_pair(v,kv));}
        }
        if(bu<0){bu=S[0];bk=0;}
        printf("? %d %d\n",bu,bk);fflush(stdout);
        int o;if(scanf("%d",&o)!=1)return 0;if(o!=0&&o!=1)return 0;
        for(int i=1;i<=n;++i)dst[i]=-1;
        bq.clear();bq.push_back(bu);dst[bu]=0;
        for(size_t i=0;i<bq.size();++i){int u=bq[i];for(size_t j=0;j<adj[u].size();++j){int v=adj[u][j];if(dst[v]<0){dst[v]=dst[u]+1;bq.push_back(v);} }}
        NS.clear();
        for(size_t idx=0;idx<S.size();++idx){int x=S[idx];int in=(dst[x]<=bk)?1:0;if(in==o)NS.push_back(x);}
        S.swap(NS);
    }
    return 0;
}
