#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
int n;
int curK;
#ifdef LOCAL
vector<int> hid; long long actions=0; int nsTotal=0;
int countDistinct(){ set<int> s(hid.begin()+1,hid.end()); return s.size(); }
#endif
int act(int i,int j){
#ifdef LOCAL
    actions++;
    hid[i]=((hid[i]+j)%n+n)%n;
    curK=countDistinct();
    if(curK==1) throw 1;
    return curK;
#else
    printf("%d %d\n",i,j); fflush(stdout);
    if(scanf("%d",&curK)!=1) exit(0);
    if(curK==1) exit(0);
    return curK;
#endif
}
static inline u64 B(int p){return 1ULL<<p;}
mt19937 rng(12345);
void solve(){
    vector<int> obs(n);
    obs[0]=curK;
    for(int x=1;x<n;x++) obs[x]=act(1,1);
    int lo=*min_element(obs.begin(),obs.end());
    u64 O=0;
    for(int x=0;x<n;x++) if(obs[x]==lo) O|=B(x);
    vector<int> kp(n+1,-1), kc(n,0);
    u64 K=0;
    auto setKnown=[&](){K=0; for(int p=0;p<n;p++) if(kc[p]) K|=B(p);};
    auto randFree=[&](int ex){ vector<int> f; for(int x=0;x<n;x++) if(x!=ex && !(O>>x&1)) f.push_back(x);
        if(f.empty()) for(int x=0;x<n;x++) if(x!=ex && kc[x]==0) f.push_back(x);
        return f[rng()%f.size()]; };
    // invariant: known wheels never on positions in O
    {
        int p=n-1;
        
        kp[1]=p; kc[p]++; setKnown();
    }
    for(int j=2;j<=n;j++){
        vector<pair<int,int>> H;
        for(int c=0;c<n;c++) if(O>>c&1){if(j<n)H.push_back({c,0});H.push_back({c,1});}
        int D=0;
        while(true){
            bool same=true;
            for(auto&h:H) if(h!=H[0]) same=false;
            if(same) break;
            int m=H.size();
            vector<u64> Oc(m);
            for(int t=0;t<m;t++) Oc[t]=H[t].second? (O&~B(H[t].first)):O;
            long long best=LLONG_MAX; int bestType=-1,bp=0,bq=0; int ties=0;
            int cnt[64];
            vector<array<int,3>> allOpts;
            auto consider=[&](long long sc,int type,int p,int q){
                allOpts.push_back({type,p,q});
                if(sc<best){best=sc;bestType=type;bp=p;bq=q;ties=1;}
                else if(sc==best){ties++; if(rng()%ties==0){bestType=type;bp=p;bq=q;}}
            };
            long long minRaw=LLONG_MAX;
            for(int d=0;d<n;d++){
                if(d==D) continue;
                memset(cnt,0,sizeof cnt);
                for(int t=0;t<m;t++) cnt[__builtin_popcountll(Oc[t]|K|B((H[t].first+d)%n))]++;
                long long sc=0; for(int v=0;v<=n;v++) sc+=(long long)cnt[v]*cnt[v];
                minRaw=min(minRaw,sc);
                consider(2*sc,0,d,0);
            }
            for(int p=0;p<n;p++) if(kc[p]) for(int q=0;q<n;q++) if(q!=p && (!(O>>q&1) || kc[q]==0)){
                u64 K2=(kc[p]==1? (K&~B(p)):K)|B(q);
                memset(cnt,0,sizeof cnt);
                for(int t=0;t<m;t++) cnt[__builtin_popcountll(Oc[t]|K2|B((H[t].first+D)%n))]++;
                long long sc=0; for(int v=0;v<=n;v++) sc+=(long long)cnt[v]*cnt[v];
                minRaw=min(minRaw,sc);
                consider(2*sc+1,1,p,q); // slight preference to moving j
            }
            bool samec=true; for(auto&h:H) if(h.first!=H[0].first) samec=false;
            if(minRaw>=(long long)m*m && samec && kc[H[0].first]>0){
                int c=H[0].first; bestType=1; bp=c; bq=randFree(c);
                if(bq==(c+D)%n) bq=randFree(c);
            } else if(minRaw>=(long long)m*m){
                // no single action splits: random symmetry-breaking action
#ifdef LOCAL
                nsTotal++;
#endif
                auto o=allOpts[rng()%allOpts.size()];
                bestType=o[0]; bp=o[1]; bq=o[2];
            }
#ifdef LOCAL
            if(getenv("DBG")){fprintf(stderr,"j=%d m=%d best=%lld type=%d p=%d q=%d D=%d O=%llx K=%llx",j,m,best,bestType,bp,bq,D,O,K);for(auto&h:H)fprintf(stderr," (%d,%d)",h.first,h.second);fprintf(stderr,"\n");}
#endif
            int k;
            vector<int> pred(m);
            if(bestType==0){
                int d=bp;
                for(int t=0;t<m;t++) pred[t]=__builtin_popcountll(Oc[t]|K|B((H[t].first+d)%n));
                k=act(j,((d-D)%n+n)%n);
                D=d;
            }else{
                int p=bp,q=bq;
                int w=-1; for(int i=1;i<=n;i++) if(kp[i]==p){w=i;break;}
                kc[p]--; kc[q]++; kp[w]=q; setKnown();
                for(int t=0;t<m;t++) pred[t]=__builtin_popcountll(Oc[t]|K|B((H[t].first+D)%n));
                k=act(w,((q-p)%n+n)%n);
            }
            vector<pair<int,int>> H2;
            for(int t=0;t<m;t++) if(pred[t]==k) H2.push_back(H[t]);
            if(H2.empty()){
#ifdef LOCAL
                fprintf(stderr,"EMPTY\n");exit(1);
#endif
                H2=H; // should not happen
            }
            H=H2;
        }
        int c=H[0].first,a=H[0].second;
        if(a) O&=~B(c);
        int p=(c+D)%n;
        if(j<n && (O>>p&1) && kc[p]>0){ int q=randFree(p); act(j,((q-p)%n+n)%n); p=q; }
        kp[j]=p; kc[p]++; setKnown();
    }
    int T=max_element(kc.begin(),kc.end())-kc.begin();
    for(int i=1;i<=n;i++) if(kp[i]!=T) act(i,((T-kp[i])%n+n)%n);
}
int main(){
#ifdef LOCAL
    int seed,mode; scanf("%d %d %d",&n,&seed,&mode);
    mt19937 g(seed);
    hid.assign(n+1,0);
    while(true){
        int per=1+g()%n, vals=1+g()%n;
        for(int i=1;i<=n;i++){
            if(mode==0) hid[i]=g()%n;
            else if(mode==1) hid[i]=(i-1)%n;              // all distinct
            else if(mode==2) hid[i]=(g()%((n+per-1)/per))*per%n; // periodic
            else if(mode==3) hid[i]=g()%vals;
            else if(mode==4){ int dv[]={1,2,5,10,25}; int p=dv[g()%5]; if(n%p) p=1; hid[i]=((i-1)%(n/p))*p%n; }
            else if(mode==5) hid[i]= (i==1+(int)(seed%n))? 1+g()%(n-1):0;
            else if(mode==6) hid[i]=((i-1)/2*2)%n;
            else if(mode==7) hid[i]=(g()%2)*(n/2);
            else hid[i]=(g()%vals)*(n/max(1,vals))%n;
        }
        if(countDistinct()>1) break;
    }
    curK=countDistinct();
    try{ solve(); fprintf(stderr,"FAIL not solved\n"); return 1;}catch(int){}
    printf("%lld\n",actions); fprintf(stderr,"%d\n",nsTotal);
#else
    if(scanf("%d %d",&n,&curK)!=2) return 0;
    if(curK==1) return 0;
    solve();
#endif
}
