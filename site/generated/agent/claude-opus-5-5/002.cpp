#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64; typedef uint32_t u32;
static const u32 MOD=1000000007u;
static u32 pw2[2000], pinv2[2000], invn[64];
static inline u32 mul(u32 a,u32 b){return (u64)a*b%MOD;}
static inline u32 addm(u32 a,u32 b){u32 c=a+b; return c>=MOD?c-MOD:c;}
static inline u32 subm(u32 a,u32 b){return a>=b?a-b:a+MOD-b;}
static u32 pw(u32 a,u64 e){u32 r=1;while(e){if(e&1)r=mul(r,a);a=mul(a,a);e>>=1;}return r;}

struct Comp{ vector<int> verts; vector<u32> P; };
static int forceMode=0;

static void zetaL(vector<u32>&F,int N,int K){
    size_t M=(size_t)1<<N;
    for(int b=0;b<N;b++){ size_t bit=(size_t)1<<b;
        for(size_t X=0;X<M;X++) if(X&bit){
            u32* d=&F[X*K]; const u32* s=&F[(X^bit)*K];
            for(int r=0;r<K;r++){u32 c=d[r]+s[r]; d[r]=c>=MOD?c-MOD:c;}
        }
    }
}
static void mobL(vector<u32>&F,int N,int K){
    size_t M=(size_t)1<<N;
    for(int b=0;b<N;b++){ size_t bit=(size_t)1<<b;
        for(size_t X=0;X<M;X++) if(X&bit){
            u32* d=&F[X*K]; const u32* s=&F[(X^bit)*K];
            for(int r=0;r<K;r++){u32 c=d[r]+MOD-s[r]; d[r]=c>=MOD?c-MOD:c;}
        }
    }
}

Comp processLevel(vector<Comp*>& ch, vector<pair<int,int>>& es){
    stable_sort(ch.begin(),ch.end(),[](Comp*x,Comp*y){return x->verts.size()<y->verts.size();});
    int k=ch.size(); vector<int> off(k), sz(k); int N=0;
    int loc[32]; int compOf[20]; u32 compMask[20];
    Comp res;
    for(int i=0;i<k;i++){ off[i]=N; sz[i]=ch[i]->verts.size();
        for(int j=0;j<sz[i];j++){ loc[ch[i]->verts[j]]=N+j; compOf[N+j]=i; res.verts.push_back(ch[i]->verts[j]); }
        compMask[i]=((1u<<sz[i])-1)<<off[i]; N+=sz[i]; }
    u32 adj[20]={0};
    for(auto&e:es){int a=loc[e.first],b=loc[e.second]; if(compOf[a]!=compOf[b]){adj[a]|=1u<<b; adj[b]|=1u<<a;}}
    u32 M=1u<<N;
    vector<u32> cm(M), A(M), lam(M);
    cm[0]=0; lam[0]=1;
    for(u32 R=1;R<M;R++){int y=__builtin_ctz(R); cm[R]=cm[R&(R-1)]|(1u<<compOf[y]); lam[R]=mul(lam[R&(R-1)],pinv2[__builtin_popcount(adj[y])]);}
    A[0]=1;
    for(int i=0;i<k;i++){ u32 lo=1u<<off[i];
        for(u32 hi=1;hi<(1u<<sz[i]);hi++){ u32 p=ch[i]->P[hi]; u32 base=hi<<off[i];
            for(u32 l=0;l<lo;l++) A[base|l]=mul(A[l],p); } }
    int sCnt=0; for(int i=0;i<k;i++) if(sz[i]==1) sCnt++;
    // cost estimates
    double costDirect=1, pairsB=1;
    for(int i=0;i<k;i++){ costDirect*=(double)((2u<<sz[i])-1); if(sz[i]>1) pairsB*=(double)((2u<<sz[i])-1); }
    costDirect*=1.0;
    double Kd=sCnt+1;
    double costB=pairsB*(double)(1u<<sCnt)*(Kd*sCnt*0.5*0.35+Kd*Kd*0.5*0.6+6)+ (double)(1u<<(N-sCnt))*(double)(1u<<sCnt)*Kd*(sCnt*0.5*0.35+Kd*0.6);
    vector<u32> L(M,0);
    if(forceMode==1 || (forceMode==0 && costB<costDirect)){
        int s=sCnt, bb=N-s, K=s+1; u32 S=1u<<s, BM=1u<<bb;
        // comps 0..s-1 are singletons (local bits 0..s-1), big comps s..k-1
        vector<uint16_t> es(S); es[0]=0;
        for(u32 X=1;X<S;X++){int y=__builtin_ctz(X); u32 r=X&(X-1); es[X]=es[r]+__builtin_popcount(adj[y]&r);}
        vector<vector<u32>> betaHat(BM);
        vector<int> ex(S);
        auto buildEx=[&](u32 Ub,u32 QbFull){ int v[20]; for(int y=0;y<s;y++) v[y]=__builtin_popcount(adj[y]&Ub)+__builtin_popcount(adj[y]&QbFull);
            ex[0]=0; for(u32 X=1;X<S;X++) ex[X]=ex[X&(X-1)]+v[__builtin_ctz(X)]; };
        auto getBeta=[&](u32 Qb)->vector<u32>&{
            vector<u32>& bh=betaHat[Qb];
            if(bh.empty()){ bh.assign((size_t)S*K,0); u32 QF=Qb<<s; int v[20];
                for(int y=0;y<s;y++) v[y]=__builtin_popcount(adj[y]&QF);
                ex[0]=0; for(u32 X=1;X<S;X++) ex[X]=ex[X&(X-1)]+v[__builtin_ctz(X)];
                for(u32 X=0;X<S;X++) bh[(size_t)X*K+__builtin_popcount(X)]=mul(pw2[es[X]],pinv2[ex[X]]);
                zetaL(bh,s,K); }
            return bh; };
        vector<u32> Fa, Acc((size_t)S*K);
        if(bb>0) Fa.resize((size_t)S*K);
        const u64 LIM=8ull*MOD*MOD;
        vector<u32> inv(K), tmp(K);
        for(u32 Rb=0;Rb<BM;Rb++){
            u32 RF=Rb<<s;
            u32 Yb=0; for(u32 t=RF;t;t&=t-1) Yb|=1u<<compOf[__builtin_ctz(t)];
            u32 Ub=0; for(u32 t=Yb;t;t&=t-1) Ub|=compMask[__builtin_ctz(t)];
            // Acc := zeta(rhs) ; then subtract products
            fill(Acc.begin(),Acc.end(),0u);
            for(u32 X=0;X<S;X++) Acc[(size_t)X*K+__builtin_popcount(X)]=pw2[es[X]];
            if(Rb==0) Acc[0]=subm(Acc[0],1);
            zetaL(Acc,s,K);
            u32 qp[20]; int kb=0; for(u32 t=Yb;t;t&=t-1) qp[kb++]=RF&compMask[__builtin_ctz(t)];
            for(u32 Zb=0;Zb+1<(1u<<kb);Zb++){
                u32 QF=0; for(u32 t=Zb;t;t&=t-1) QF|=qp[__builtin_ctz(t)];
                int cQ=0; for(u32 t=QF;t;t&=t-1) cQ+=__builtin_popcount(adj[__builtin_ctz(t)]&Ub);
                buildEx(Ub,QF);
                fill(Fa.begin(),Fa.end(),0u);
                for(u32 X=0;X<S;X++){ u32 val = (QF==0&&X==0)?0u:mul(L[QF|X],pinv2[cQ+ex[X]+es[X]]); Fa[(size_t)X*K+__builtin_popcount(X)]=val; }
                zetaL(Fa,s,K);
                vector<u32>& bh=getBeta(QF>>s);
                for(u32 x=0;x<S;x++){ const u32* fa=&Fa[(size_t)x*K]; const u32* fb=&bh[(size_t)x*K]; u32* ac=&Acc[(size_t)x*K];
                    for(int n=0;n<K;n++){ u64 sum=0; for(int i=0;i<=n;i++){ sum+=(u64)fa[i]*fb[n-i]; if(sum>=LIM) sum-=LIM; } ac[n]=subm(ac[n],(u32)(sum%MOD)); } }
            }
            vector<u32> fbv(K);
            vector<u32>* bRp = bb==0? nullptr : &getBeta(Rb);
            for(u32 x=0;x<S;x++){ u32* ac=&Acc[(size_t)x*K];
                for(int n=0;n<K;n++) tmp[n]=ac[n];
                const u32* fb;
                if(bb==0){ for(int n=0;n<K;n++) fbv[n]=tmp[n]; fbv[0]=addm(fbv[0],1); fb=fbv.data(); }
                else fb=&(*bRp)[(size_t)x*K];
                // inverse of fb (fb[0] invertible)
                u32 i0=pw(fb[0],MOD-2); inv[0]=i0;
                for(int n=1;n<K;n++){ u64 sum=0; for(int j=1;j<=n;j++){ sum+=(u64)fb[j]*inv[n-j]; if(sum>=LIM) sum-=LIM; } inv[n]=mul((MOD-(u32)(sum%MOD))%MOD,i0); }
                for(int n=0;n<K;n++){ u64 sum=0; for(int i=0;i<=n;i++){ sum+=(u64)tmp[i]*inv[n-i]; if(sum>=LIM) sum-=LIM; } ac[n]=(u32)(sum%MOD); }
            }
            mobL(Acc,s,K);
            int cR=0; for(u32 t=RF;t;t&=t-1) cR+=__builtin_popcount(adj[__builtin_ctz(t)]&Ub);
            buildEx(Ub,RF);
            for(u32 X=0;X<S;X++){ if(RF==0&&X==0) continue; u32 al=Acc[(size_t)X*K+__builtin_popcount(X)]; L[RF|X]=mul(al,pw2[cR+ex[X]+es[X]]); }
        }
        vector<vector<u32>>().swap(betaHat);
    } else {
        u32 q[20]; int dd[20]; static u32 Q1[1<<11],Q2[1<<11]; static int D1[1<<11],D2[1<<11];
        const u64 LIM=8ull*MOD*MOD;
        for(u32 R=1;R<M;R++){
            u32 Y=cm[R];
            if(__builtin_popcount(Y)==1){L[R]=1;continue;}
            u32 U=0; for(u32 t=Y;t;t&=t-1) U|=compMask[__builtin_ctz(t)];
            int kk=0, Df=0;
            for(u32 t=Y;t;t&=t-1){ int i=__builtin_ctz(t); u32 qi=R&compMask[i]; int d=0;
                for(u32 s=qi;s;s&=s-1) d+=__builtin_popcount(adj[__builtin_ctz(s)]&U);
                q[kk]=qi; dd[kk]=d; Df+=d; kk++; }
            u32 Q=0; int D=0; u64 acc=0; u32 lim=1u<<kk;
            for(u32 g=1;g<lim;g++){
                int b=__builtin_ctz(g); Q^=q[b]; D+=(Q&q[b])?dd[b]:-dd[b];
                acc+=(u64)L[Q]*pinv2[D]; if(acc>=LIM) acc-=LIM;
            }
            u32 s=(u32)(acc%MOD);
            L[R]=mul(subm(1,s),pw2[Df]);
        }
    }
    // ell = L*A*lam ; a = -log(1-ell) ranked by comps
    int K=k+1;
    vector<u32> ell(M,0);
    for(u32 R=1;R<M;R++) ell[R]=mul(mul(L[R],A[R]),lam[R]);
    vector<u32>().swap(L);
    vector<u32> a(M,0);
    {
        vector<u32> F((size_t)M*K,0);
        for(u32 R=1;R<M;R++) F[(size_t)R*K+__builtin_popcount(cm[R])]=ell[R];
        zetaL(F,N,K);
        vector<u32> p(K), g(K);
        for(u32 X=1;X<M;X++){ u32* f=&F[(size_t)X*K]; int d=K-1;
            for(int n=1;n<=d;n++) p[n]=(MOD-f[n])%MOD;
            for(int n=1;n<=d;n++){ u64 s=0; for(int j=1;j<n;j++){ s+=(u64)mul(j,g[j])*p[n-j]; if(s>=(8ull*MOD*MOD)) s-=8ull*MOD*MOD;}
                u32 sv=(u32)(s%MOD); g[n]=subm(p[n],mul(sv,invn[n])); }
            f[0]=0; for(int n=1;n<=d;n++) f[n]=(MOD-g[n])%MOD; for(int n=d+1;n<K;n++) f[n]=0;
        }
        mobL(F,N,K);
        for(u32 R=1;R<M;R++) a[R]=F[(size_t)R*K+__builtin_popcount(cm[R])];
    }
    u32 KM=1u<<k;
    vector<u32> B(KM,0); B[0]=1;
    for(u32 R=1;R<M;R++) B[cm[R]]=subm(B[cm[R]],ell[R]);
    for(int i=0;i<k;i++){ u32 np=(MOD-ch[i]->P[0])%MOD; u32 bit=1u<<i;
        if(np) for(u32 T=0;T<KM;T++) if(T&bit) B[T]=addm(B[T],mul(B[T^bit],np)); }
    for(int i=0;i<k;i++){ u32 bit=1u<<i; for(u32 T=0;T<KM;T++) if(T&bit) B[T]=addm(B[T],B[T^bit]); }
    res.P.assign(M,0); u32 tot=0, full=KM-1;
    for(u32 R=1;R<M;R++){ res.P[R]=mul(a[R],B[full^cm[R]]); tot=addm(tot,res.P[R]); }
    res.P[0]=subm(1,tot);
    if(getenv("DBG")){ for(u32 R=0;R<M;R++) fprintf(stderr,"R=%u ell=%u a=%u P=%u\n",R,ell[R],a[R],res.P[R]); }
    return res;
}

int par[32];
int fnd(int x){return par[x]==x?x:par[x]=fnd(par[x]);}

int main(){
    pw2[0]=1; for(int i=1;i<2000;i++) pw2[i]=addm(pw2[i-1],pw2[i-1]);
    u32 i2=(MOD+1)/2; pinv2[0]=1; for(int i=1;i<2000;i++) pinv2[i]=mul(pinv2[i-1],i2);
    for(int i=1;i<64;i++) invn[i]=pw(i,MOD-2);
    if(getenv("FM")) forceMode=atoi(getenv("FM"));
    long long c; int T; if(scanf("%lld %d",&c,&T)!=2) return 0;
    while(T--){
        int n,m; scanf("%d %d",&n,&m);
        vector<array<int,3>> E(m);
        for(auto&e:E){scanf("%d %d %d",&e[1],&e[2],&e[0]); e[1]--; e[2]--;}
        sort(E.begin(),E.end());
        vector<Comp> comps(n);
        for(int i=0;i<n;i++){par[i]=i; comps[i].verts={i}; comps[i].P={0,1};}
        for(int s=0;s<m;){
            int t=s; while(t<m&&E[t][0]==E[s][0]) t++;
            // group
            vector<pair<int,int>> es; 
            map<int,int> tp; // temp dsu over comp roots
            function<int(int)> tf=[&](int x)->int{ return tp[x]==x?x:tp[x]=tf(tp[x]); };
            for(int j=s;j<t;j++){int a=fnd(E[j][1]),b=fnd(E[j][2]); if(a!=b){ es.push_back({E[j][1],E[j][2]}); if(!tp.count(a)) tp[a]=a; if(!tp.count(b)) tp[b]=b; int x=tf(a),y=tf(b); if(x!=y) tp[x]=y; }}
            map<int,vector<int>> groups;
            for(auto&kv:tp) groups[tf(kv.first)].push_back(kv.first);
            for(auto&gv:groups){
                vector<int>& roots=gv.second;
                set<int> rs(roots.begin(),roots.end());
                vector<pair<int,int>> ge;
                for(auto&e:es) if(rs.count(fnd(e.first))) ge.push_back(e);
                vector<Comp*> ch; for(int r:roots) ch.push_back(&comps[r]);
                Comp nc=processLevel(ch,ge);
                int nr=roots[0];
                for(int r:roots) par[r]=nr;
                for(int r:roots) if(r!=nr){ comps[r].P.clear(); comps[r].verts.clear(); }
                comps[nr]=move(nc);
            }
            s=t;
        }
        int r=fnd(0);
        printf("%u\n",subm(1,comps[r].P[0]));
    }
}
