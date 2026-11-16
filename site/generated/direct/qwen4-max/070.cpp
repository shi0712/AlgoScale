#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
static int n; static ull FULL;
static inline int pc(ull x){ return __builtin_popcountll(x); }
static ull rot(ull m,int t){ t%=n; if(t<0)t+=n; if(t==0)return m&FULL; return ((m<<t)|(m>>(n-t)))&FULL; }
static int ask(int i,long long j){ printf("%d %lld\n",i,j); fflush(stdout); int k; if(scanf("%d",&k)!=1) exit(0); if(k==1) exit(0); return k; }
static bool isPer(ull V){ for(int p=1;p<n;p++) if(rot(V,p)==V) return true; return false; }
int main(){
    if(scanf("%d",&n)!=1) return 0;
    FULL=(1ULL<<n)-1ULL;
    int k; if(scanf("%d",&k)!=1) return 0; if(k==1) return 0;
    vector<int> ks(n); ks[0]=k;
    for(int t=1;t<n;t++) ks[t]=ask(1,1);
    int d=*min_element(ks.begin(),ks.end());
    ull S=0; for(int t=0;t<n;t++) if(ks[t]==d) S|=(1ULL<<t);
    bool per=isPer(S); int tf=-1;
    if(!per){ if((S>>(n-1))&1ULL) tf=n-1; else { for(int t=0;t<n;t++) if((S>>t)&1ULL){tf=t;break;} } }
    else { if(!((S>>(n-1))&1ULL)) tf=n-1; else { for(int t=0;t<n;t++) if(!((S>>t)&1ULL)){tf=t;break;} } }
    int curk=ks[n-1];
    if(tf!=n-1) curk=ask(1,(long long)tf-(long long)(n-1));
    ull V=rot(S,-tf)|1ULL; int k0=curk;
    vector<int> val(n+1,-1); val[1]=0; int g=1;
    while(true){
        int u=-1; for(int i=1;i<=n;i++) if(val[i]==-1){u=i;break;}
        if(u==-1){
            int w=-1; for(int i=1;i<=n;i++) if(val[i]!=0){w=i;break;}
            if(w==-1) return 0;
            int v=val[w]; int kk=ask(w,(long long)(-v));
            if(k0-kk==1) V&=~(1ULL<<v);
            val[w]=0; g++; k0=kk; continue;
        }
        if(isPer(V)){
            ull holes=FULL&~V; int h=-1;
            for(int t=0;t<n;t++) if((holes>>t)&1ULL){h=t;break;}
            int w=-1; for(int i=1;i<=n;i++) if(val[i]==0){w=i;break;}
            if(h<0||w<0) return 0;
            if(g>=2){ int kk=ask(w,(long long)h); V|=(1ULL<<h); val[w]=h; g--; k0=kk; }
            else{
                int kk=ask(w,(long long)h); ull V2;
                if(kk==k0+1) V2=V|(1ULL<<h); else V2=(V&~1ULL)|(1ULL<<h);
                V=rot(V2,-h);
                for(int i=1;i<=n;i++) if(val[i]!=-1) val[i]=((val[i]-h)%n+n)%n;
                k0=kk;
            }
            continue;
        }
        int nu=0; ull km=0;
        for(int i=1;i<=n;i++){ if(val[i]==-1) nu++; else km|=(1ULL<<val[i]); }
        ull unc=V&~km; ull C1,C0;
        if(pc(unc)==nu){ C1=unc; C0=0; }
        else{ C1=V&~1ULL; C0=1ULL; if(n-g-(pc(V)-1)>=1) C0|=(V&~1ULL); }
        int off=0,lastk=k0;
        while(pc(C1|C0)>1){
            int bT=-1; long long bS=LLONG_MAX;
            for(int t=1;t<n;t++){
                if(t==off) continue;
                ull In=rot(V,-t),Out=FULL&~In;
                int a=pc(C1&In), b=pc((C1&Out)|(C0&In)), c=pc(C0&Out);
                if((a==0&&b==0)||(a==0&&c==0)||(b==0&&c==0)) continue;
                int mx=max(a,max(b,c));
                long long sc=(long long)mx*1000000LL+(long long)(a*a+b*b+c*c);
                if(sc<bS){bS=sc;bT=t;}
            }
            if(bT<0) break;
            int t=bT; int kk=ask(u,(long long)(t-off)); off=t; lastk=kk;
            ull In=rot(V,-t),Out=FULL&~In;
            if(kk==k0-1){ C1&=In; C0=0; }
            else if(kk==k0){ C1&=Out; C0&=In; }
            else if(kk==k0+1){ C1=0; C0&=Out; }
            else break;
            if((C1|C0)==0) break;
        }
        ull C=C1|C0; if(C==0) C=V;
        int delta=__builtin_ctzll(C);
        int pos=(delta+off)%n; int kk;
        if(pos!=0) kk=ask(u,(long long)(-pos)); else kk=lastk;
        if(k0-kk==1&&delta!=0) V&=~(1ULL<<delta);
        val[u]=0; g++; k0=kk;
    }
    return 0;
}
