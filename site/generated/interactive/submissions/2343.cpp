#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
int n,m,W,D;
vector<vector<int>> G;
vector<int> px,py;
vector<vector<int>> svals, srow;
vector<size_t> poff;
vector<u64> pref;
vector<u64> Fw,Bw;
vector<vector<int>> U,forcedR,mnA,mxA,valA,LBA;
vector<int> minLB;
set<pair<int,int>> gs;
vector<int> fen;int FN;
void fadd(int i){for(;i<=FN;i+=i&-i)fen[i]++;}
int ftot=0;
int fkth(int k){int p=0;int LOG=1;while((1<<LOG)<=FN)LOG++;for(int b=LOG;b>=0;b--){int np=p+(1<<b);if(np<=FN&&fen[np]<k){p=np;k-=fen[np];}}return p+1;}
inline u64* FW(int l,int k){return &Fw[((size_t)l*(D+1)+k)*W];}
inline u64* BW(int l,int k){return &Bw[((size_t)l*(D+1)+k)*W];}
inline u64* PR(int k,int r){return &pref[poff[k]+(size_t)r*W];}
void getA(int l,int k,u64*out){
    if(k==1){for(int i=0;i<W;i++)out[i]=0;out[0]=1;return;}
    int f=forcedR[l][k];
    if(f){for(int i=0;i<W;i++)out[i]=0;out[(f-1)>>6]|=1ULL<<((f-1)&63);return;}
    int r=upper_bound(svals[k].begin(),svals[k].end(),U[l][k])-svals[k].begin();
    u64*p=PR(k,r);for(int i=0;i<W;i++)out[i]=p[i];
}
void computeMM(int l,int k){
    u64 F[8];u64*a=FW(l,k),*b=BW(l,k);int pc=0;
    for(int i=0;i<W;i++){F[i]=a[i]&b[i];pc+=__builtin_popcountll(F[i]);}
    if(pc==0){mnA[l][k]=mxA[l][k]=0;return;}
    if(pc<=12){
        int mn=INT_MAX,mx=0;
        for(int i=0;i<W;i++){u64 x=F[i];while(x){int bt=__builtin_ctzll(x);x&=x-1;int row=i*64+bt+1;int v=G[row][k+1-row];mn=min(mn,v);mx=max(mx,v);}}
        mnA[l][k]=mn;mxA[l][k]=mx;return;
    }
    int len=svals[k].size();
    int lo=1,hi=len;
    while(lo<hi){int md=(lo+hi)/2;u64*p=PR(k,md);bool nz=false;for(int i=0;i<W;i++)if(F[i]&p[i]){nz=true;break;}if(nz)hi=md;else lo=md+1;}
    mnA[l][k]=svals[k][lo-1];
    lo=1;hi=len;
    while(lo<hi){int md=(lo+hi)/2;u64*p=PR(k,md);bool sub=true;for(int i=0;i<W;i++)if(F[i]&~p[i]){sub=false;break;}if(sub)hi=md;else lo=md+1;}
    mxA[l][k]=svals[k][lo-1];
}
void windows(int l){
    int L=2*n-2+l;
    vector<int> amn(L),amx(L);
    for(int j=0;j<L;j++){int k=j+2-l;if(k<=0){amn[j]=amx[j]=G[1][2-k];}else if(k==1){amn[j]=amx[j]=G[1][1];}else{amn[j]=mnA[l][k];amx[j]=mxA[l][k];}}
    // window for T: j in [T-1, T+l-2]
    deque<int> d1,d2;int ml=INT_MAX;
    int nextj=0;
    for(int T=1;T<=D;T++){
        int hiJ=T+l-2;
        while(nextj<=hiJ){while(!d1.empty()&&amn[d1.back()]<=amn[nextj])d1.pop_back();d1.push_back(nextj);
            while(!d2.empty()&&amx[d2.back()]<=amx[nextj])d2.pop_back();d2.push_back(nextj);nextj++;}
        while(d1.front()<T-1)d1.pop_front();
        while(d2.front()<T-1)d2.pop_front();
        int lb=amn[d1.front()],ub=amx[d2.front()];
        if(valA[l][T]==0){
            if(lb==ub){valA[l][T]=lb;fadd(lb);ftot++;}
            else{LBA[l][T]=lb;ml=min(ml,lb);}
        }
    }
    if(minLB[l]!=INT_MAX)gs.erase({minLB[l],l});
    minLB[l]=ml;
    if(ml!=INT_MAX)gs.insert({ml,l});
}
int main(){
    if(scanf("%d %d",&n,&m)!=2)return 0;
    G.assign(n+2,vector<int>(n+2,0));px.assign(n*n+1,0);py.assign(n*n+1,0);
    for(int i=1;i<=n;i++)for(int j=1;j<=n;j++){scanf("%d",&G[i][j]);px[G[i][j]]=i;py[G[i][j]]=j;}
    W=(n+63)/64;D=2*n-1;
    svals.assign(D+1,{});srow.assign(D+1,{});poff.assign(D+1,0);
    size_t tot=0;
    for(int k=1;k<=D;k++){int lo=max(1,k+1-n),hi=min(n,k);poff[k]=tot;tot+=(size_t)(hi-lo+2)*W;}
    pref.assign(tot,0);
    for(int k=1;k<=D;k++){
        int lo=max(1,k+1-n),hi=min(n,k);
        vector<pair<int,int>> c;
        for(int x=lo;x<=hi;x++)c.push_back({G[x][k+1-x],x});
        sort(c.begin(),c.end());
        for(int r=0;r<(int)c.size();r++){
            svals[k].push_back(c[r].first);srow[k].push_back(c[r].second);
            u64*p=PR(k,r),*q=PR(k,r+1);
            for(int i=0;i<W;i++)q[i]=p[i];
            int x=c[r].second;q[(x-1)>>6]|=1ULL<<((x-1)&63);
        }
    }
    Fw.assign((size_t)(n+1)*(D+1)*W,0);Bw.assign((size_t)(n+1)*(D+1)*W,0);
    U.assign(n+1,vector<int>(D+1,INT_MAX));forcedR.assign(n+1,vector<int>(D+1,0));
    mnA.assign(n+1,vector<int>(D+1,0));mxA=mnA;valA.assign(n+1,vector<int>(D+1,0));LBA=valA;
    minLB.assign(n+1,INT_MAX);
    FN=n*n;fen.assign(FN+1,0);
    u64 A[8],tmp[8];
    for(int l=1;l<=n;l++){
        forcedR[l][2]=2;
        getA(l,1,FW(l,1));
        for(int k=2;k<=D;k++){getA(l,k,A);u64*p=FW(l,k-1),*q=FW(l,k);
            for(int i=0;i<W;i++){u64 s=(p[i]<<1)|(i>0?p[i-1]>>63:0);q[i]=(p[i]|s)&A[i];}}
        getA(l,D,BW(l,D));
        for(int k=D-1;k>=1;k--){getA(l,k,A);u64*p=BW(l,k+1),*q=BW(l,k);
            for(int i=0;i<W;i++){u64 s=(p[i]>>1)|(i+1<W?p[i+1]<<63:0);q[i]=(p[i]|s)&A[i];}}
        for(int k=1;k<=D;k++)computeMM(l,k);
        windows(l);
    }
    vector<char> changed(D+2,0);
    while(true){
        int th=ftot>=m?fkth(m):INT_MAX;
        if(gs.empty()||gs.begin()->first>=th)break;
        int l=gs.begin()->second;int lbv=gs.begin()->first;
        int T=-1;
        for(int t=1;t<=D;t++)if(valA[l][t]==0&&LBA[l][t]==lbv){T=t;break;}
        printf("? %d %d\n",l,T);fflush(stdout);
        int v;if(scanf("%d",&v)!=1)return 0;if(v==-1)return 0;
        valA[l][T]=v;fadd(v);ftot++;
        int x=px[v],y=py[v];int kc;
        if(x==1&&y>=2)kc=2-y;else kc=x+y-1;
        int a=max(2,T-l+1),b=T;
        if(b>=2){
            for(int k=a;k<=b;k++)U[l][k]=min(U[l][k],v);
            if(kc>=2)forcedR[l][kc]=x;
            fill(changed.begin(),changed.end(),0);
            for(int k=a;k<=D;k++){
                getA(l,k,A);u64*p=FW(l,k-1),*q=FW(l,k);bool diff=false;
                for(int i=0;i<W;i++){u64 s=(p[i]<<1)|(i>0?p[i-1]>>63:0);tmp[i]=(p[i]|s)&A[i];if(tmp[i]!=q[i])diff=true;}
                if(diff){changed[k]=1;for(int i=0;i<W;i++)q[i]=tmp[i];}
                else if(k>b)break;
            }
            for(int k=b;k>=1;k--){
                getA(l,k,A);u64*q=BW(l,k);bool diff=false;
                if(k==D){for(int i=0;i<W;i++){tmp[i]=A[i];if(tmp[i]!=q[i])diff=true;}}
                else{u64*p=BW(l,k+1);
                for(int i=0;i<W;i++){u64 s=(p[i]>>1)|(i+1<W?p[i+1]<<63:0);tmp[i]=(p[i]|s)&A[i];if(tmp[i]!=q[i])diff=true;}}
                if(diff){changed[k]=1;for(int i=0;i<W;i++)q[i]=tmp[i];}
                else if(k<a)break;
            }
            for(int k=1;k<=D;k++)if(changed[k])computeMM(l,k);
        }
        windows(l);
    }
    vector<int> all;
    for(int l=1;l<=n;l++)for(int t=1;t<=D;t++)if(valA[l][t])all.push_back(valA[l][t]);
    sort(all.begin(),all.end());
    string out="!";
    for(int i=0;i<m&&i<(int)all.size();i++){out+=' ';out+=to_string(all[i]);}
    printf("%s\n",out.c_str());fflush(stdout);
    return 0;
}
