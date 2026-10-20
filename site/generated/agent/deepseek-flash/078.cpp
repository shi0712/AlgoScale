#include <bits/stdc++.h>
using namespace std;
using ll = long long;
static const int INF = 100000000;

int n;
vector<int> a;
int Amax;
ll baseCost;

int sz;
vector<int> tr;   // 4 ints per node: m00,m01,m10,m11  (min-plus 2x2)

static inline void pull(int p){
    int* c = &tr[(size_t)p*4];
    const int* L = &tr[(size_t)(2*p)*4];
    const int* R = &tr[(size_t)(2*p+1)*4];
    int l0=L[0],l1=L[1],l2=L[2],l3=L[3],r0=R[0],r1=R[1],r2=R[2],r3=R[3];
    int x0,y0;
    x0=l0+r0; y0=l1+r2; c[0]=x0<y0?x0:y0;
    x0=l0+r1; y0=l1+r3; c[1]=x0<y0?x0:y0;
    x0=l2+r0; y0=l3+r2; c[2]=x0<y0?x0:y0;
    x0=l2+r1; y0=l3+r3; c[3]=x0<y0?x0:y0;
}

int main(){
    int T; if(scanf("%d",&T)!=1) return 0;
    while(T--){
        int m;
        if(scanf("%d %d",&n,&m)!=2) return 0;
        a.assign(n,0);
        for(int i=0;i<n;i++) if(scanf("%d",&a[i])!=1) return 0;
        Amax = 0; for(int x:a) Amax = x>Amax?x:Amax;
        baseCost = 0;
        for(int i=1;i<n;i++) if(a[i-1]>a[i]) baseCost += a[i-1]-a[i];
        if(m==1 || n==1){
            printf("%lld\n", (ll)((Amax>a[0])?(Amax-a[0]):0) + baseCost);
            continue;
        }
        vector<int> vals(a);
        sort(vals.begin(), vals.end());
        vals.erase(unique(vals.begin(), vals.end()), vals.end());
        if((int)vals.size()<=2){ printf("0\n"); continue; }

        ll best = (ll)((Amax>a[0])?(Amax-a[0]):0) + baseCost;   // D=0

        // ---------- family I : q = Amax (D = Amax - V) ----------
        {
            int Vlist[2100]; int cnt=0;
            Vlist[cnt++]=0;
            for(int v: vals) Vlist[cnt++]=v;
            for(int k=0;k<cnt;k++){
                int V = Vlist[k];
                if((ll)V - a[0] >= best) continue;
                int D = Amax - V;
                int dp0 = (a[0]<=V)?0:INF;
                int dp1 = D;
                for(int i=1;i<n;i++){
                    int ai=a[i], r=ai-a[i-1];
                    int al = (D>r)?(D-r):0;
                    int dr = (r<0)?-r:0;
                    int be = (D<dr)?D:dr;
                    int t = dp0+al; int n1 = dp1<t?dp1:t;
                    int n0;
                    if(ai<=V){ t=dp1-be; n0 = dp0<t?dp0:t; }
                    else n0 = INF;
                    dp0=n0; dp1=n1;
                }
                int phi = dp0<dp1?dp0:dp1;
                ll tot = (ll)V - a[0] + baseCost + phi;
                if(tot<best) best=tot;
            }
        }

        // ---------- family II : D in breakpoints ----------
        vector<int> cand;
        cand.push_back(0);
        for(int i=1;i<n;i++){ int d = a[i]-a[i-1]; cand.push_back(d<0?-d:d); }
        cand.push_back(Amax - vals[0]);
        sort(cand.begin(), cand.end());
        cand.erase(unique(cand.begin(),cand.end()),cand.end());

        // lower bounds via unconstrained optimum (V = Amax)
        vector<pair<int,int>> tasks;   // (lowerbound, D)
        tasks.reserve(cand.size());
        for(int D : cand){
            int Tv = Amax - D; if(Tv<0) Tv=0;
            if((ll)Tv - a[0] >= best) continue;
            // unconstrained DP (all positions may be 0)
            int dp0=0, dp1=D;
            for(int i=1;i<n;i++){
                int r=a[i]-a[i-1];
                int al = (D>r)?(D-r):0;
                int dr = (r<0)?-r:0;
                int be = (D<dr)?D:dr;
                int t = dp0+al; int n1 = dp1<t?dp1:t;
                t = dp1-be;     int n0 = dp0<t?dp0:t;
                dp0=n0; dp1=n1;
            }
            int phiM = dp0<dp1?dp0:dp1;
            int lb = (int)((ll)Tv - a[0] + baseCost + phiM);   // valid lower bound of any (V,D) cost
            if((ll)lb < best) tasks.push_back(make_pair(lb, D));
        }
        sort(tasks.begin(), tasks.end());

        vector<int> ord(n);
        for(int i=0;i<n;i++) ord[i]=i;
        sort(ord.begin(), ord.end(), [](int x,int y){ return a[x]<a[y]; });

        int leaves = n-1;
        sz = 1; while(sz<leaves) sz <<= 1;
        tr.assign((size_t)2*sz*4, 0);
        for(int j=0;j<sz;j++){ int* p=&tr[(size_t)(sz+j)*4]; p[0]=0;p[1]=INF;p[2]=INF;p[3]=0; }
        for(int p=sz-1;p>=1;p--) pull(p);

        vector<int> alpha_(n,0), beta_(n,0);

        for(size_t ti=0; ti<tasks.size(); ++ti){
            if((ll)tasks[ti].first >= best) break;   // remaining bounds are >= this one
            int D = tasks[ti].second;
            int T = Amax - D; if(T<0) T=0;
            if((ll)T - a[0] >= best) continue;
            for(int i=1;i<n;i++){
                int r=a[i]-a[i-1];
                alpha_[i] = (D>r)?(D-r):0;
                int dr = (r<0)?-r:0;
                beta_[i]  = (D<dr)?D:dr;
            }
            for(int i=1;i<n;i++){
                int* p=&tr[(size_t)(sz+i-1)*4];
                p[1]=alpha_[i];
                if(a[i]>T){ p[0]=INF; p[2]=INF; p[3]=0; }
                else { p[0]=0; p[2]=-beta_[i]; p[3]=0; }
            }
            for(int p=sz-1;p>=1;p--) pull(p);
            int restricted0 = (a[0] > T);

            int nextord = 0;
            while(nextord<n && a[ord[nextord]]<=T) nextord++;
            int V = T;
            while(true){
                if((ll)V - a[0] >= best) break;
                const int* P = &tr[4];
                int init1 = D;
                int m0 = init1+P[2];
                { int t=init1+P[3]; if(t<m0) m0=t; }
                if(!restricted0){
                    if(P[0]<m0) m0=P[0];
                    { int t=P[1]; if(t<m0) m0=t; }
                }
                ll tot = (ll)V - a[0] + baseCost + m0;
                if(tot<best) best=tot;
                if(nextord>=n) break;
                int nv = a[ord[nextord]];
                if((ll)nv - a[0] >= best) break;
                while(nextord<n && a[ord[nextord]]==nv){
                    int pos = ord[nextord];
                    if(pos==0) restricted0 = 0;
                    else {
                        int idx = sz + pos - 1;
                        int* p=&tr[(size_t)idx*4];
                        p[0]=0; p[2]=-beta_[pos];
                        idx >>= 1;
                        while(idx>=1){ pull(idx); idx >>= 1; }
                    }
                    nextord++;
                }
                V = nv;
            }
        }
        printf("%lld\n", best);
    }
    return 0;
}
