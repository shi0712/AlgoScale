#include <bits/stdc++.h>
using namespace std;
using ll=long long; using ull=unsigned long long;
static char ibuf[1<<22]; static size_t ipos=0,ilen=0;
static inline int gc(){ if(ipos==ilen){ilen=fread(ibuf,1,sizeof(ibuf),stdin);ipos=0;if(ilen==0)return -1;} return (unsigned char)ibuf[ipos++]; }
static inline int readInt(){ int c=gc(); while(c!=-1&&(c<'0'||c>'9'))c=gc(); int x=0; while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();} return x; }
static int dsuPar[100005];
static inline int findp(int x){ while(dsuPar[x]!=x){dsuPar[x]=dsuPar[dsuPar[x]];x=dsuPar[x];} return x; }
int main(){
    int n=readInt(),m=readInt(),k=readInt();
    vector<ull> E(m);
    for(int i=0;i<m;i++){ ull u=(ull)(readInt()-1),v=(ull)(readInt()-1),w=(ull)readInt(); E[i]=(w<<34)|(u<<17)|v; }
    sort(E.begin(),E.end());
    int maxNodes=2*n-1;
    vector<int> tw(maxNodes,0),tp(maxNodes,-1),tl(maxNodes,-1),tr(maxNodes,-1);
    vector<int> sz(n,1),rootOf(n);
    for(int i=0;i<n;i++){dsuPar[i]=i;rootOf[i]=i;}
    int tot=n; ll W=0;
    for(int i=0;i<m&&tot<maxNodes;i++){
        ull e=E[i]; int w=(int)(e>>34); int u=(int)((e>>17)&0x1FFFFull),v=(int)(e&0x1FFFFull);
        int ru=findp(u),rv=findp(v); if(ru==rv)continue;
        int id=tot++; tw[id]=w; tl[id]=rootOf[ru]; tr[id]=rootOf[rv]; tp[tl[id]]=id; tp[tr[id]]=id;
        if(sz[ru]<sz[rv]){int t=ru;ru=rv;rv=t;} dsuPar[rv]=ru; sz[ru]+=sz[rv]; rootOf[ru]=id; W+=w;
    }
    {vector<ull> t;E.swap(t);} {vector<int> t;sz.swap(t);} {vector<int> t;rootOf.swap(t);}
    if(k==0){ printf("%lld\n",W); return 0; }
    vector<int> c(k),a((size_t)k*n);
    for(int j=0;j<k;j++){ c[j]=readInt(); int* aj=&a[(size_t)j*n]; for(int i=0;i<n;i++)aj[i]=readInt(); }
    vector<int> A((size_t)tot*k);
    for(int i=0;i<n;i++){ int* d=&A[(size_t)i*k]; for(int j=0;j<k;j++)d[j]=a[(size_t)j*n+i]; }
    {vector<int> t;a.swap(t);}
    for(int X=n;X<tot;X++){ const int*L=&A[(size_t)tl[X]*k];const int*R=&A[(size_t)tr[X]*k];int*Cu=&A[(size_t)X*k];
        for(int j=0;j<k;j++)Cu[j]=L[j]<R[j]?L[j]:R[j]; }
    const int KS=1<<k;
    vector<ll> val((size_t)k*KS,0);
    int tau[16][16]; for(int j=0;j<k;j++)for(int l=0;l<k;l++)tau[j][l]=INT_MAX;
    int tauMax=INT_MAX; bool tauDone=(k<2); ull keys[16];
    for(int X=0;X<tot;X++){
        int wx=tw[X]; int px=(tp[X]<0)?INT_MAX:tw[tp[X]]; const int* Ax=&A[(size_t)X*k];
        if(px>wx){
            for(int j=0;j<k;j++)keys[j]=((ull)(unsigned)Ax[j]<<4)|(unsigned)j;
            for(int i=1;i<k;i++){ ull key=keys[i]; int t=i-1; while(t>=0&&keys[t]>key){keys[t+1]=keys[t];--t;} keys[t+1]=key; }
            unsigned U=0;
            for(int r=0;r<k;r++){ int j=(int)(keys[r]&15u); int aj=(int)(keys[r]>>4);
                int contrib=(aj<px?aj:px)-wx; if(contrib>0)val[(size_t)j*KS+U]+=contrib; U|=1u<<j; }
        }
        if(!tauDone){
            if(wx>=tauMax)tauDone=true;
            else{ bool ch=false;
                for(int j=0;j<k;j++){ int aj=Ax[j]; int* tj=tau[j];
                    for(int l=j+1;l<k;l++){ int al=Ax[l]; int cand=aj>al?aj:al; if(wx>cand)cand=wx;
                        if(cand<tj[l]){tj[l]=cand;tau[l][j]=cand;ch=true;} } }
                if(ch){ tauMax=0; for(int j=0;j<k;j++)for(int l=j+1;l<k;l++)if(tau[j][l]>tauMax)tauMax=tau[j][l]; }
            }
        }
    }
    for(int j=0;j<k;j++){ ll* Z=&val[(size_t)j*KS];
        for(int b=0;b<k;b++){ int bit=1<<b; for(int U=0;U<KS;U++) if(U&bit) Z[U]+=Z[U^bit]; } }
    vector<ll> csum(KS,0);
    for(int S=1;S<KS;S++){ int b=__builtin_ctz((unsigned)S); csum[S]=csum[S^(1<<b)]+c[b]; }
    ll ans=W; const int FULL=KS-1;
    for(int S=1;S<KS;S++){
        int comp=FULL^S; ll hub=0;
        for(int t=S;t;t&=t-1){ int j=__builtin_ctz((unsigned)t); hub+=val[(size_t)j*KS+comp]; }
        ll link=0; int rem=S; int v=__builtin_ctz((unsigned)rem); rem^=1<<v; int d[16];
        for(int t=rem;t;t&=t-1){ int u=__builtin_ctz((unsigned)t); d[u]=tau[v][u]; }
        while(rem){
            int best=-1,bd=INT_MAX;
            for(int t=rem;t;t&=t-1){ int u=__builtin_ctz((unsigned)t); if(d[u]<bd){bd=d[u];best=u;} }
            link+=bd; rem^=1<<best; const int* tb=tau[best];
            for(int t=rem;t;t&=t-1){ int u=__builtin_ctz((unsigned)t); if(tb[u]<d[u])d[u]=tb[u]; }
        }
        ll total=csum[S]+hub+link; if(total<ans)ans=total;
    }
    printf("%lld\n",ans);
    return 0;
}
