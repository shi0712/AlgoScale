#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
int n;
#ifdef SIM
vector<vector<char>> FF; long long QC=0;
int ask(int l,int r){QC++;return FF[l][r];}
#else
int ask(int l,int r){printf("? %d %d\n",l,r);fflush(stdout);int x;if(scanf("%d",&x)!=1||x<0)exit(0);return x;}
#endif
int main(){
#ifdef SIM
    unsigned seed; int type; if(scanf("%d %u %d",&n,&seed,&type)!=3)return 0; mt19937 rng(seed);
    vector<int> P(n+1); iota(P.begin()+1,P.end(),1);
    if(type==0) shuffle(P.begin()+1,P.end(),rng); else if(type==2) reverse(P.begin()+1,P.end());
    else if(type==3){for(int i=1;i<=n;i++)P[i]=(i%2? (i+1)/2 : (n+1)/2 + i/2);}
    else if(type==4){ // few random swaps
        for(int t=0;t<5;t++){int a=rng()%n+1,b=rng()%n+1;swap(P[a],P[b]);}}
    else if(type==5){ // blocks
        int B=rng()%50+1; vector<int> Q; for(int s=1;s<=n;s+=B){int e=min(n,s+B-1);for(int v=e;v>=s;v--)Q.push_back(v);} for(int i=1;i<=n;i++)P[i]=Q[i-1];}
    else if(type==6){ // shuffle blocks of sorted
        for(int i=1;i<=n;i++)P[i]=i; int B=rng()%100+1; vector<vector<int>> bl; for(int s=1;s<=n;s+=B){vector<int> b; for(int v=s;v<=min(n,s+B-1);v++)b.push_back(v); bl.push_back(b);} shuffle(bl.begin(),bl.end(),rng); int i=1; for(auto&b:bl)for(int v:b)P[i++]=v;}
    FF.assign(n+2,vector<char>(n+2,0));
    for(int len=2;len<=n;len++)for(int l=1;l+len-1<=n;l++){int r=l+len-1;FF[l][r]=FF[l+1][r]^FF[l][r-1]^(len>2?FF[l+1][r-1]:0)^(P[l]>P[r]);}
#else
    if(scanf("%d",&n)!=1)return 0;
#endif
    vector<int> ord; ord.reserve(n); ord.push_back(1);
    vector<int> rk(n+2,0), F(n+2,0), mark(n+2,0);
    vector<int> cands; vector<ull> bs;
    for(int k=2;k<=n;k++){
        for(int i=0;i<k-1;i++) rk[ord[i]]=i;
        cands.resize(k); iota(cands.begin(),cands.end(),0);
        while(cands.size()>1){
            int c=cands.size(); int W=(c+63)/64;
            bs.assign(W,0);
            int cnt=0; // number of ones (actual = flip? c-cnt : cnt)
            bool gflip=false;
            int bestx=-1,bestv=-1;
            int lo=cands[0], hiR=cands.back();
            for(int x=k-1;x>=1;x--){
                int rho=rk[x];
                if(rho<lo) continue; // toggles nothing
                if(rho>=hiR){gflip=!gflip;}
                else{
                    int L=upper_bound(cands.begin(),cands.end(),rho)-cands.begin();
                    // toggle first L bits
                    int full=L>>6, rem=L&63; int po=0;
                    for(int w=0;w<full;w++){po+=__builtin_popcountll(bs[w]);bs[w]=~bs[w];}
                    if(rem){ull mk=(1ULL<<rem)-1; po+=__builtin_popcountll(bs[full]&mk); bs[full]^=mk;}
                    cnt+=L-2*po;
                }
                int ones=gflip?c-cnt:cnt;
                int v=min(ones,c-ones);
                if(v>bestv){bestv=v;bestx=x; if(2*v>=c-1)break;}
            }
            int x=bestx;
            int s=ask(x,k)^F[x];
            // f_x(r) = parity #{j in [x,k-1]: rk_j >= r}
            for(int r=0;r<k;r++)mark[r]=0;
            for(int j=x;j<k;j++)mark[rk[j]]=1;
            // suffix parity
            vector<int> nc; nc.reserve(c);
            {
                int par=0; int idx=c-1;
                for(int r=k-1;r>=0&&idx>=0;r--){
                    par^=mark[r];
                    if(cands[idx]==r){ if(par==s) nc.push_back(r); idx--; }
                }
                reverse(nc.begin(),nc.end());
            }
            cands.swap(nc);
        }
        int pos=cands[0];
        ord.insert(ord.begin()+pos,k);
        int par=0;
        for(int x=k-1;x>=1;x--){ if(rk[x]>=pos) par^=1; F[x]^=par; }
        F[k]=0;
    }
    vector<int> p(n+1);
    for(int i=0;i<n;i++)p[ord[i]]=i+1;
#ifdef SIM
    bool ok=true; for(int i=1;i<=n;i++) if(p[i]!=P[i]) ok=false;
    printf("%s %lld\n",ok?"OK":"WRONG",QC);
#else
    printf("!");for(int i=1;i<=n;i++)printf(" %d",p[i]);printf("\n");fflush(stdout);
#endif
}
