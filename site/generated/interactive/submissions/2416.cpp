#include <bits/stdc++.h>
using namespace std;

static int n, T;
#ifdef LOCAL
static vector<int> arr; static long long qcnt=0; static vector<int> ks; static int kidx=0;
static vector<int> pre; // not used
static vector<int> bit; static void badd(int i,int v){for(;i<=n;i+=i&-i)bit[i]+=v;} static int bsum(int i){int s=0;for(;i>0;i-=i&-i)s+=bit[i];return s;}
static int askPrefix(int m){ qcnt++; return bsum(m); }
#else
static int askPrefix(int m){
    printf("? 1 %d\n", m); fflush(stdout);
    int r; if(scanf("%d",&r)!=1 || r<0) exit(0); return r;
}
#endif

// segment tree: max with range add, over 0..n
struct Seg{
    int N; vector<int> mx, lz;
    void init(int n_){N=1; while(N<n_+1)N<<=1; mx.assign(2*N, INT_MIN/2); lz.assign(2*N,0);}
    void apply(int x,int v){mx[x]+=v; lz[x]+=v;}
    void push(int x){ if(lz[x]){apply(2*x,lz[x]);apply(2*x+1,lz[x]);lz[x]=0;} }
    void add(int x,int l,int r,int ql,int qr,int v){
        if(qr<l||r<ql) return; if(ql<=l&&r<=qr){apply(x,v);return;}
        push(x); int md=(l+r)/2; add(2*x,l,md,ql,qr,v); add(2*x+1,md+1,r,ql,qr,v); mx[x]=max(mx[2*x],mx[2*x+1]);
    }
    void setv(int x,int l,int r,int p,int v){
        if(l==r){mx[x]=v; lz[x]=0; return;}
        push(x); int md=(l+r)/2; if(p<=md) setv(2*x,l,md,p,v); else setv(2*x+1,md+1,r,p,v); mx[x]=max(mx[2*x],mx[2*x+1]);
    }
    int get(int x,int l,int r,int p){
        if(l==r) return mx[x]; push(x); int md=(l+r)/2; return p<=md?get(2*x,l,md,p):get(2*x+1,md+1,r,p);
    }
    int first(int x,int l,int r,int k){ // first idx with value>=k
        if(mx[x]<k) return -1; if(l==r) return l; push(x); int md=(l+r)/2;
        int res=first(2*x,l,md,k); if(res!=-1) return res; return first(2*x+1,md+1,r,k);
    }
    void add(int l,int r,int v){ if(l<=r) add(1,0,N-1,l,r,v);}
    void setv(int p,int v){setv(1,0,N-1,p,v);}
    int get(int p){return get(1,0,N-1,p);}
    int first(int k){return first(1,0,N-1,k);}
};

static const int LM = 100;
static vector<float> E; static vector<unsigned char> BM; static vector<vector<unsigned char>> BMS;
static const double WS[]={0,0.2,0.4,0.6,0.8,0.9,0.95,0.98}; static const int NWS=8;
static inline size_t idx(int L,int z,int j){ return ((size_t)L*(LM+1)+z)*(LM+1)+j; }
static double Cb[LM+1][LM+1];

static double Fr[LM+1][LM+1];
static double WGT=0, VF=0.5, OV=1.0, BETA=1.5, LOCW=0.95; static int LOCTH=16, HIST=16, CLT=100; static double ALPHA_LN=4.0;
static void buildDP1(){
    for(int i=0;i<=LM;i++){Cb[i][0]=1; for(int j=1;j<=i;j++) Cb[i][j]=Cb[i-1][j-1]+(j<=i-1?Cb[i-1][j]:0);}
    for(int L=1;L<=LM;L++)for(int z=0;z<=L;z++){ if(z==0||z==L){Fr[L][z]=0;continue;}
        double best=1e18; for(int m=1;m<L;m++){double s=0; for(int c=max(0,z-(L-m));c<=min(m,z);c++) s+=Cb[m][c]*Cb[L-m][z-c]*(Fr[m][c]+Fr[L-m][z-c]); s/=Cb[L][z]; best=min(best,s);} Fr[L][z]=1+best;}
    E.assign((size_t)(LM+1)*(LM+1)*(LM+1),0); BM.assign(E.size(),0);
    for(int L=1;L<=LM;L++) for(int z=1;z<=L;z++) for(int j=1;j<=z;j++){
        if(z==L){E[idx(L,z,j)]=0;continue;}
        double best=1e18; int bm=1;
        for(int m=1;m<L;m++){
            double s=0;
            int clo=max(0,z-(L-m)), chi=min(m,z);
            for(int c=clo;c<=chi;c++){
                double p=Cb[m][c]*Cb[L-m][z-c];
                double e = (j<=c)? E[idx(m,c,j)]+WGT*Fr[L-m][z-c] : E[idx(L-m,z-c,j-c)]+WGT*Fr[m][c];
                s+=p*e;
            }
            s/=Cb[L][z];
            if(s<best){best=s;bm=m;}
        }
        E[idx(L,z,j)]=1+best; BM[idx(L,z,j)]=bm;
    }
}
static void buildDP(){
#ifdef LOCAL
    { FILE*f=fopen("/app/work/tables.bin","rb"); if(f){ for(int i=0;i<NWS;i++){ vector<unsigned char> b((size_t)(LM+1)*(LM+1)*(LM+1)); if(fread(b.data(),1,b.size(),f)!=b.size()) exit(3); BMS.push_back(b);} BM=BMS[0]; fclose(f); return; } }
#endif
#ifdef LOCAL
    for(int i=0;i<NWS;i++){WGT=WS[i]; buildDP1(); BMS.push_back(BM);} E.clear(); E.shrink_to_fit();
    { FILE*f=fopen("/app/work/tables.bin","wb"); for(auto&b:BMS) fwrite(b.data(),1,b.size(),f); fclose(f);} 
#else
    BMS.assign(NWS, vector<unsigned char>());
#endif
}
static void ensureTable(int i){ if(BMS[i].empty()){ WGT=WS[i]; buildDP1(); BMS[i]=BM; } }

// choose offset m in [1,L-1] for large closed gap: median of j-th zero position
static int chooseLarge(int L,int z,int j){
    if(L>CLT){ long long m=llround((j-1/3.0)/(z+1/3.0)*L); return (int)min<long long>(max<long long>(m,1),L-1); }
    // P(pos<=m) = P(#zeros in first m >= j) ; find m where ~0.5 using normal approx / log-space
    // use log hypergeometric pmf on position: P(pos=m)=C(m-1,j-1)C(L-m,z-j)/C(L,z)
    double lc = lgamma(L+1.0)-lgamma(z+1.0)-lgamma(L-z+1.0);
    double cum=0; int lo=j, hi=j+(L-z);
    for(int m=lo;m<=hi;m++){
        double lp = lgamma((double)m)-lgamma((double)j)-lgamma((double)(m-j+1)) + lgamma(L-m+1.0)-lgamma(z-j+1.0)-lgamma((double)(L-m-(z-j)+1)) - lc;
        cum+=exp(lp);
        if(cum>=0.5){ return min(max(m,1),L-1); }
    }
    return min(max(hi,1),L-1);
}

int main(){
#ifdef LOCAL
    // input: n t, array, then t ks generated by mode
    int mode; unsigned seed;
    if(scanf("%d %d %d %u",&n,&T,&mode,&seed)!=4) return 0;
    double dens; int atype; scanf("%lf %d",&dens,&atype); scanf("%lf %lf %lf %lf %d %d %lf",&VF,&OV,&BETA,&LOCW,&LOCTH,&HIST,&ALPHA_LN);
    mt19937 rng(seed); arr.assign(n+1,0);
    if(atype==0){ for(int i=1;i<=n;i++) arr[i] = (rng()%1000000 < dens*1e6) ? 1:0; }
    else if(atype==1){ int c=(int)(dens*n); for(int i=1;i<=n;i++) arr[i]= i<=c; }
    else if(atype==3){ int c=(int)((1-dens)*n); for(int i=1;i<=n;i++) arr[i]= i>c; }
    else if(atype==2||atype==5){ int i=1; double ds[5]={0,0.05,0.5,0.95,1}; int ML= atype==2?2000:50; while(i<=n){ int len=rng()%ML+1; double dd=ds[rng()%5]; for(int q=0;q<len&&i<=n;q++,i++) arr[i]=(rng()%1000000<dd*1e6);} }
    else if(atype==4){ for(int i=1;i<=n;i++) arr[i] = (i>1000 && i<=n/2) ? 1 : (rng()%1000000 < dens*1e6); }
    int zeros=0; for(int i=1;i<=n;i++) zeros+=!arr[i];
    while(zeros<T){ int i=rng()%n+1; if(arr[i]){arr[i]=0;zeros++;} }
    bit.assign(n+1,0); for(int i=1;i<=n;i++) if(arr[i]) badd(i,1);
    int zc=zeros;
#else
    if(scanf("%d %d",&n,&T)!=2) return 0;
#endif
    buildDP();
    Seg sg; sg.init(n);
    set<int> known; known.insert(0); sg.setv(0,0);
    // fenwick of flips
    vector<int> fw(n+1,0);
    auto fadd=[&](int i){for(;i<=n;i+=i&-i)fw[i]++;};
    auto fsum=[&](int i){int s=0;for(;i>0;i-=i&-i)s+=fw[i];return s;};
    (void)fsum;
    vector<int> khist; int curTab=0;
    for(int round=0; round<T; round++){
        int k;
#ifdef LOCAL
        if(mode==0) k=rng()%zc+1; else if(mode==1) k=1; else if(mode==2) k=zc; else if(mode==3) k=min(zc,(int)(rng()%10+1)); else k = (zc+1)/2;
#else
        if(scanf("%d",&k)!=1 || k<0) return 0;
#endif
        int x=-1;
        int gdir=0; long long goffL=0, goffR=0; int sPos=-1; double sDen=0, curD=0.5; // streak start boundary & density
        {
            // estimate weight
            int last=*known.rbegin();
            double zce;
            if(last==n) zce=sg.get(n);
            else { double dor=(sg.get(last)+fsum(last)+1.0)/(last+2.0); zce=sg.get(last)+dor*(n-last); }
            int R=T-round-1;
            double w=min(0.9, BETA*R/max(1.0,zce));
            khist.push_back(k);
            int hs=khist.size(); int cntn=0,tot=0;
            for(int i=max(1,hs-HIST);i<hs;i++){ tot++; if(abs(khist[i]-khist[i-1])<=LOCTH) cntn++; }
            if(tot>0) w=max(w, LOCW*cntn/tot);
            int bi=0; for(int i=1;i<NWS;i++) if(fabs(WS[i]-w)<fabs(WS[bi]-w)) bi=i;
            ensureTable(bi);
            BM.swap(BMS[bi]); // use selected
            curTab=bi;
        }
        while(true){
            int b = sg.first(k);
            int a;
            if(b==-1){ a=*known.rbegin(); }
            else { auto it=known.find(b); --it; a=*it; }
            int za = sg.get(a);
            int j = k-za;
            int m;
            if(b!=-1){
                int zb=sg.get(b); int L=b-a, z=zb-za;
                if(z==L){ x=a+j; break; }
                if(L<=LM) m = a + BM[idx(L,z,j)];
                else {
                    double d=(double)z/L;
                    double posL=(j-1/3.0)/(z+1/3.0)*L, posR=L-posL;
                    if((posL<=LM*VF && j<LM) || (posR<=LM*VF && z-j+1<LM)){
                        int zv=(int)llround(d*LM);
                        bool left = (posL<=posR && j<LM) || z-j+1>=LM;
                        int jj = left? j : z-j+1;
                        zv=max(zv,jj); zv=min(zv,LM-1);
                        int off=BM[idx(LM,zv,jj)];
                        m = left? a+off : b-off;
                    } else m = a + chooseLarge(L,z,j);
                }
            } else {
                if(n-a==j){ x=n; break; }
                // open gap: estimate density of zeros (original array)
                double d;
                int last=*known.rbegin(); (void)last;
                if(last==0) d=0.5; else d = (sg.get(last)+fsum(last)+1.0)/(last+2.0);
                curD=d;
                double pos=j/d;
                if(pos<=LM*VF && j<LM && a+LM<=n){
                    int zv=(int)llround(d*LM); zv=max(zv,j); zv=min(zv,LM-1);
                    m = a+BM[idx(LM,zv,j)];
                } else {
                    long long mm = a + (long long)ceil(pos*OV);
                    if(mm>n) mm=n; if(mm<=a) mm=a+1;
                    m=(int)mm;
                }
            }
            {
                int hiB = (b!=-1)? b-1 : n;
                long long bEff = (b!=-1)? b : n+1;
                if(gdir!=0 && sPos>=0){
                    // cut region since streak start
                    long long S; long long Zs;
                    if(gdir==1){ S=a-sPos; Zs=za-sg.get(sPos); }
                    else { S=sPos-b; Zs=sg.get(sPos)-sg.get(b); }
                    bool surprise=false;
                    double q=(double)Zs/max<long long>(S,1);
                    if(S>0 && q<sDen){
                        double p=sDen; double kl = (q>0? q*log(q/p):0) + (1-q)*log((1-q)/(1-p));
                        if(p>=1) kl=1e9;
                        if(S*kl > ALPHA_LN) surprise=true;
                    }
                    if(surprise){
                        if(gdir==1){ long long need=min(2*goffL, (bEff-a)/2); if(m-a<need) m=(int)(a+need); }
                        else { long long need=min(2*goffR, (bEff-a)/2); if(bEff-m<need) m=(int)(bEff-need); }
                    }
                }
                if(m<=a || m>hiB) m = a + (hiB-a+1)/2;
                if(m<=a) m=a+1;
            }
            int ones = askPrefix(m);
            int zm = m-ones;
            {
                long long bEff = (b!=-1)? b : n+1;
                int dir = (zm>=k)? -1 : 1;
                if(dir!=gdir || sPos<0){
                    gdir=dir;
                    double den;
                    if(b!=-1) den=(double)(sg.get(b)-za)/(b-a);
                    else den=curD;
                    if(dir==1){ sPos=a; sDen=den; }
                    else { if(b!=-1){ sPos=b; sDen=den; } else sPos=-1; }
                }
                goffL=m-a; goffR=bEff-m;
            }
            known.insert(m); sg.setv(m,zm);
        }
        BM.swap(BMS[curTab]);
        // record x-1 and x
        if(!known.count(x-1)){known.insert(x-1); sg.setv(x-1,k-1);}
        if(!known.count(x)){known.insert(x); sg.setv(x,k);}
        sg.add(x,n,-1); fadd(x);
#ifdef LOCAL
        // verify
        { if(arr[x]!=0 || x-bsum(x)!=k){fprintf(stderr,"WRONG round %d k=%d got %d\n",round,k,x); return 1;} arr[x]=1; badd(x,1); zc--; }
#else
        printf("! %d\n",x); fflush(stdout);
#endif
    }
#ifdef LOCAL
    printf("%lld\n",qcnt);
#endif
}
