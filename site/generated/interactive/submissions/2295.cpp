#include <bits/stdc++.h>
using namespace std;
using u64 = unsigned long long;

static int N, W, k;
static vector<u64> S, F;
static vector<unsigned char> filt, dn;
static vector<int> cnt, doorSw, swDoor, bitv, dcnt;
static vector<int> pendD, pendS;

static inline bool gb(const u64* b,int i){return (b[i>>6]>>(i&63))&1ULL;}
static inline void sb(u64* b,int i){b[i>>6]|=1ULL<<(i&63);}
static inline void cb(u64* b,int i){b[i>>6]&=~(1ULL<<(i&63));}
static u64 rs;
static inline u64 nrand(){rs^=rs<<13;rs^=rs>>7;rs^=rs<<17;return rs;}

static void assignSwitch(int j,int i){
    doorSw[j]=i; swDoor[i]=j;
    u64* Sj=&S[(size_t)j*W];
    int iw=i>>6; u64 keep=1ULL<<(i&63);
    for(int w=0;w<W;w++){
        u64 x=Sj[w];
        if(w==iw) x&=~keep;
        if(x){
            Sj[w]&=~x; u64 y=x;
            while(y){int b=__builtin_ctzll(y);y&=y-1;int ii=(w<<6)+b;--cnt[j];--dcnt[ii];
                if(dcnt[ii]==1&&swDoor[ii]<0) pendS.push_back(ii);}
        }
    }
    for(int jj=k;jj<N;jj++){
        if(jj==j||dn[jj])continue;
        u64* T=&S[(size_t)jj*W];
        if(gb(T,i)){cb(T,i);--cnt[jj];if(cnt[jj]<=1)pendD.push_back(jj);}
    }
    dcnt[i]=0;
}
static void checkDoor(int j){
    if(dn[j]||cnt[j]!=1)return;
    const u64* Sj=&S[(size_t)j*W]; int i=-1;
    for(int w=0;w<W;w++) if(Sj[w]){i=(w<<6)+__builtin_ctzll(Sj[w]);break;}
    if(i<0)return;
    if(doorSw[j]<0) assignSwitch(j,i);
    if(filt[j]){bitv[i]=gb(&F[(size_t)j*W],i)?1:0;dn[j]=1;}
}
static void process(){
    while(!pendD.empty()||!pendS.empty()){
        while(!pendS.empty()){
            int i=pendS.back();pendS.pop_back();
            if(swDoor[i]>=0||dcnt[i]!=1)continue;
            int jj=-1;
            for(int j=k;j<N;j++) if(!dn[j]&&gb(&S[(size_t)j*W],i)){jj=j;break;}
            if(jj<0||doorSw[jj]>=0)continue;
            assignSwitch(jj,i); pendD.push_back(jj);
        }
        while(!pendD.empty()){int j=pendD.back();pendD.pop_back();checkDoor(j);}
    }
}
static void filterDoor(int j,bool cor,const u64* P){
    if(dn[j])return;
    u64* Sj=&S[(size_t)j*W]; u64* Fj=&F[(size_t)j*W];
    if(!filt[j]){
        if(cor){for(int w=0;w<W;w++)Fj[w]=P[w];}
        else{for(int w=0;w<W;w++)Fj[w]=~P[w];}
        filt[j]=1;
    }else{
        int c=0;
        for(int w=0;w<W;w++){
            u64 d=P[w]^Fj[w];
            u64 mask=cor?~d:d;
            u64 old=Sj[w],nw=old&mask,rem=old&~mask;
            Sj[w]=nw; Fj[w]=cor?P[w]:~P[w];
            c+=__builtin_popcountll(nw);
            while(rem){int b=__builtin_ctzll(rem);rem&=rem-1;int ii=(w<<6)+b;--dcnt[ii];
                if(dcnt[ii]==1&&swDoor[ii]<0)pendS.push_back(ii);}
        }
        cnt[j]=c;
    }
    if(cnt[j]<=1)pendD.push_back(j);
}
int main(){
    if(scanf("%d",&N)!=1)return 0;
    W=(N+63)/64;
    S.assign((size_t)N*W,~0ULL); F.assign((size_t)N*W,0ULL);
    u64 lm=(N&63)?((1ULL<<(N&63))-1):~0ULL;
    for(int j=0;j<N;j++) S[(size_t)j*W+W-1]=lm;
    filt.assign(N,0);dn.assign(N,0);cnt.assign(N,N);
    doorSw.assign(N,-1);swDoor.assign(N,-1);bitv.assign(N,-1);dcnt.assign(N,N);
    k=0; bool allBits=false;
    string s(N,'0'); vector<u64> P(W); vector<int> lst; lst.reserve(N);
    rs=(u64)chrono::steady_clock::now().time_since_epoch().count()*1000003ULL+12345ULL;
    if(!rs)rs=1;
    while(true){
        if(k<N)pendD.push_back(k);
        process();
        while(k<N&&dn[k])++k;
        if(k==N)break;
        int pick=-1;
        if(allBits){
            int rem=0;
            for(int i=0;i<N;i++) if(swDoor[i]<0){++rem;if(pick<0)pick=i;}
            if(rem<=0)break;
            if(rem==1){
                int jj=-1;
                for(int j=k;j<N;j++) if(!dn[j]){jj=j;break;}
                if(jj<0)break;
                assignSwitch(jj,pick);dn[jj]=1;process();continue;
            }
            for(int w=0;w<W;w++)P[w]=0;
            for(int i=0;i<N;i++) if(bitv[i]==1)sb(P.data(),i);
            if(bitv[pick]==1)cb(P.data(),pick);else sb(P.data(),pick);
        }else{
            for(int w=0;w<W;w++)P[w]=nrand();
            P[W-1]&=lm;
            for(int i=0;i<N;i++) if(swDoor[i]>=0&&bitv[i]>=0){if(bitv[i])sb(P.data(),i);else cb(P.data(),i);}
            if(filt[k]){
                lst.clear();
                const u64* Sk=&S[(size_t)k*W];
                for(int w=0;w<W;w++){u64 x=Sk[w];while(x){int b=__builtin_ctzll(x);x&=x-1;lst.push_back((w<<6)+b);}}
                int sz=(int)lst.size(),half=sz>>1;
                for(int t=0;t<half;t++){int u=t+(int)(nrand()%(u64)(sz-t));swap(lst[t],lst[u]);}
                const u64* Fk=&F[(size_t)k*W];
                for(int t=0;t<sz;t++){
                    int i=lst[t];bool f=gb(Fk,i);bool want=(t<half)?f:!f;
                    if(want)sb(P.data(),i);else cb(P.data(),i);
                }
            }
        }
        for(int i=0;i<N;i++) s[i]=gb(P.data(),i)?'1':'0';
        printf("? %s\n",s.c_str()); fflush(stdout);
        int r;
        if(scanf("%d",&r)!=1)return 0;
        if(allBits){
            if(r<0||r>=N)return 0;
            if(doorSw[r]<0)assignSwitch(r,pick);
            dn[r]=1; process();
        }else{
            if(r==-1){
                for(int j=k;j<N;j++)filterDoor(j,true,P.data());
                for(int i=0;i<N;i++) if(bitv[i]<0)bitv[i]=gb(P.data(),i)?1:0;
                allBits=true; process();
            }else if(r>=k&&r<N){
                for(int j=k;j<r;j++)filterDoor(j,true,P.data());
                filterDoor(r,false,P.data());
                process();
            }else return 0;
        }
    }
    string ans(N,'0');
    for(int i=0;i<N;i++) ans[i]=(bitv[i]==1)?'1':'0';
    printf("! %s",ans.c_str());
    for(int i=0;i<N;i++) printf(" %d",swDoor[i]);
    printf("\n"); fflush(stdout);
    return 0;
}
