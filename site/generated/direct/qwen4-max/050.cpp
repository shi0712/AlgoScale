#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull;
static char ibuf[1<<16]; static int ilen=0,ipos=0;
static inline int gc(){ if(ipos==ilen){ilen=(int)fread(ibuf,1,sizeof(ibuf),stdin);ipos=0;if(ilen<=0)return -1;} return (unsigned char)ibuf[ipos++]; }
static inline int readInt(){ int c=gc(); while(c!=-1&&(c<'0'||c>'9')&&c!='-')c=gc(); int s=1; if(c=='-'){s=-1;c=gc();} int x=0; while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();} return x*s; }
static const int KC=1000000000; static const ll INFL=(ll)4e18;
int n,m,N; vector<int> st,ato,acap,acost,arev,cap0; vector<ll> h_,dist_; vector<int> pe_;
vector<int> eSrc,eBuy,eFree,eP; int S,T,U,eUcheap,eUexp; vector<int> tp,sp,Mv,cc,ord; vector<ull> heapv;
ll evaluate(int W){
    acap=cap0; for(int j=0;j<m;j++) acap[eP[j]]=W;
    acap[eUexp]=n-W; acap[eUcheap]=0; acap[arev[eUcheap]]=W;
    ll total=0,hU=0;
    for(int k=0;k<W;k++){int i=ord[k];acap[eSrc[i]]=0;acap[arev[eSrc[i]]]=1;acap[eBuy[i]]=0;acap[arev[eBuy[i]]]=1;total+=cc[i];if((ll)cc[i]>hU)hU=cc[i];}
    int flow=W; fill(h_.begin(),h_.end(),0LL); h_[U]=hU; h_[T]=hU;
    while(flow<n){
        fill(dist_.begin(),dist_.end(),INFL); dist_[S]=0; heapv.clear(); ll dT=INFL;
        for(int e=st[S];e<st[S+1];e++){ if(acap[e]<=0)continue; int c=ato[e]; dist_[c]=0; pe_[c]=e; ll hc=h_[c];
            for(int e2=st[c];e2<st[c+1];e2++){ if(acap[e2]<=0)continue; int u=ato[e2]; ll nd=(ll)acost[e2]+hc-h_[u];
                if(nd<dist_[u]&&nd<dT){ dist_[u]=nd;pe_[u]=e2; if(u==T)dT=nd; else{heapv.push_back(((ull)nd<<11)|(ull)u);push_heap(heapv.begin(),heapv.end(),greater<ull>());} } } }
        while(!heapv.empty()){
            pop_heap(heapv.begin(),heapv.end(),greater<ull>()); ull top=heapv.back(); heapv.pop_back();
            int v=(int)(top&2047ULL); ll d=(ll)(top>>11);
            if(d>=dT)break; if(d!=dist_[v])continue; ll hv=h_[v];
            for(int e=st[v];e<st[v+1];e++){ if(acap[e]<=0)continue; int u=ato[e]; ll nd=d+(ll)acost[e]+hv-h_[u];
                if(nd<dist_[u]&&nd<dT){ dist_[u]=nd;pe_[u]=e; if(u==T)dT=nd; else{heapv.push_back(((ull)nd<<11)|(ull)u);push_heap(heapv.begin(),heapv.end(),greater<ull>());} } } }
        if(dist_[T]==INFL)break; ll D=dist_[T];
        for(int v=0;v<N;v++){ ll dv=dist_[v]; h_[v]+=(dv<D?dv:D); }
        int v=T; while(v!=S){int e=pe_[v];acap[e]--;acap[arev[e]]++;v=ato[arev[e]];}
        total+=h_[T]; flow++;
    }
    return total-(ll)KC*(ll)(n-W);
}
int main(){
    n=readInt(); m=readInt(); tp.assign(n,-1); sp.assign(n,-1); Mv.assign(n,0); cc.assign(n,0);
    { vector<pair<int,int>> vp(m);
      for(int i=0;i<n;i++){
        for(int j=0;j<m;j++){int x=readInt(); vp[j]=make_pair(x,j);} sort(vp.begin(),vp.end());
        int idx=m-1,found=0;
        while(idx>=0&&found<2){ int j=idx; while(j>0&&vp[j-1].first==vp[idx].first)j--;
            if(j==idx&&vp[idx].first>0){ if(found==0){tp[i]=vp[idx].second;Mv[i]=vp[idx].first;} else sp[i]=vp[idx].second; found++; }
            idx=j-1; }
        int c=Mv[i]+1;
        while(true){ auto it=lower_bound(vp.begin(),vp.end(),make_pair(c,-1)); if(it!=vp.end()&&it->first==c)++c; else break; }
        cc[i]=c; } }
    S=0;U=1+n+m;T=2+n+m;N=3+n+m;
    vector<int> tu,tv,tc,tw; int cap=8*n+2*m+8; tu.reserve(cap);tv.reserve(cap);tc.reserve(cap);tw.reserve(cap);
    auto add=[&](int u,int v,int c,int w)->int{ int id=(int)tu.size();
        tu.push_back(u);tv.push_back(v);tc.push_back(c);tw.push_back(w);
        tu.push_back(v);tv.push_back(u);tc.push_back(0);tw.push_back(-w); return id; };
    eSrc.resize(n);eBuy.resize(n);eFree.resize(n);
    for(int i=0;i<n;i++){ int Ci=1+i; eSrc[i]=add(S,Ci,1,0); eBuy[i]=add(Ci,U,1,cc[i]);
        if(tp[i]>=0){ eFree[i]=add(Ci,1+n+tp[i],1,0);
            if(sp[i]>=0) add(Ci,1+n+sp[i],1,Mv[i]); else add(Ci,T,1,Mv[i]+KC);
        } else eFree[i]=add(Ci,T,1,KC); }
    eP.resize(m); for(int j=0;j<m;j++) eP[j]=add(1+n+j,T,0,KC);
    eUcheap=add(U,T,0,0); eUexp=add(U,T,0,KC);
    int E=(int)tu.size(); st.assign(N+1,0);
    for(int e=0;e<E;e++) st[tu[e]+1]++;
    for(int v=0;v<N;v++) st[v+1]+=st[v];
    vector<int> pos(st.begin(),st.end()),nid(E);
    for(int e=0;e<E;e++) nid[e]=pos[tu[e]]++;
    ato.resize(E);acap.resize(E);acost.resize(E);arev.resize(E);
    for(int e=0;e<E;e++){int p=nid[e];ato[p]=tv[e];acap[p]=tc[e];acost[p]=tw[e];arev[p]=nid[e^1];}
    for(int i=0;i<n;i++){eSrc[i]=nid[eSrc[i]];eBuy[i]=nid[eBuy[i]];eFree[i]=nid[eFree[i]];}
    for(int j=0;j<m;j++) eP[j]=nid[eP[j]];
    eUcheap=nid[eUcheap]; eUexp=nid[eUexp]; cap0=acap;
    h_.assign(N,0);dist_.assign(N,0);pe_.assign(N,0); heapv.reserve(4*N);
    ord.resize(n); for(int i=0;i<n;i++)ord[i]=i;
    sort(ord.begin(),ord.end(),[](int x,int y){return cc[x]<cc[y];});
    vector<ll> memo(n+2,-1);
    auto F=[&](int W)->ll{ if(memo[W]<0) memo[W]=evaluate(W); return memo[W]; };
    int lo=0,hi=n;
    while(lo<hi){ int mid=(lo+hi)>>1; if(F(mid)<=F(mid+1)) hi=mid; else lo=mid+1; }
    evaluate(lo);
    string out;
    for(int i=0;i<n;i++){ int b;
        if(acap[eBuy[i]]==0) b=cc[i]; else if(acap[eFree[i]]==0) b=0; else b=Mv[i];
        out+=to_string(b); out+=(i+1<n?' ':'\n'); }
    fwrite(out.data(),1,out.size(),stdout);
    return 0;
}
