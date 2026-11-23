#include <bits/stdc++.h>
using namespace std;
int N;
map<pair<int,int>,long long> cache_;
long long ask(int i,int j){
    if(i==j) return 0;
    if(i>j) swap(i,j);
    auto it=cache_.find({i,j}); if(it!=cache_.end()) return it->second;
    printf("? %d %d\n",i,j); fflush(stdout);
    long long d; if(scanf("%lld",&d)!=1 || d<0) exit(0);
    cache_[{i,j}]=d; return d;
}

struct Ep{int rep; vector<int> mem, dis;};
template<class F> int majority(const vector<int>& E,int T,F same){
    int m=E.size(); vector<Ep> eps; int cur=-1;
    for(int k=0;k<m;k++){
        int e=E[k]; int rem=m-k;
        long long ub=rem; for(auto&p:eps) ub+=max(p.mem.size(),p.dis.size());
        if(ub<T) return 0;
        if(cur<0 || eps[cur].mem.size()==eps[cur].dis.size()){eps.push_back({e,{e},{}}); cur=eps.size()-1; if(1>=T) return 1; continue;}
        if(same(e,eps[cur].rep)){eps[cur].mem.push_back(e); if((int)eps[cur].mem.size()>=T) return 1;}
        else eps[cur].dis.push_back(e);
    }
    if(cur<0 || eps[cur].mem.size()==eps[cur].dis.size()) return 0;
    int c=eps[cur].rep; int cnt=eps[cur].mem.size();
    int ne=cur; vector<int> st(ne,0); vector<vector<int>> ud(ne);
    for(int i=0;i<ne;i++) ud[i]=eps[i].dis;
    auto UB=[&](){long long u=cnt; for(int i=0;i<ne;i++){ if(st[i]==0) u+=max(eps[i].mem.size(),ud[i].size()); else if(st[i]==2) u+=ud[i].size();} return u;};
    vector<int> order(ne); iota(order.begin(),order.end(),0);
    stable_sort(order.begin(),order.end(),[&](int a,int b){return eps[a].mem.size()>eps[b].mem.size();});
    for(int i:order){
        if(cnt>=T) return 1; if(UB()<T) return 0;
        if(eps[i].mem.size()==1 && ud[i].size()==1){
            int d=ud[i][0]; ud[i].clear();
            if(same(c,d)){cnt++; st[i]=2; continue;}
        }
        if(same(c,eps[i].rep)){st[i]=1;cnt+=eps[i].mem.size();ud[i].clear();} else st[i]=2;
    }
    for(int i:order) while(!ud[i].empty()){
        if(cnt>=T) return 1; if(UB()<T) return 0;
        int d=ud[i].back(); ud[i].pop_back(); if(same(c,d)) cnt++;
    }
    return cnt>=T;
}
int main(){
    if(scanf("%d",&N)!=1) return 0;
    vector<long long> d0(N,0), du(N,-1);
    for(int i=1;i<N;i++) d0[i]=ask(0,i);
    int u=0; for(int i=1;i<N;i++) if(d0[i]>d0[u]) u=i;
    long long L0=d0[u];
    du[u]=0; du[0]=L0;
    // order remaining by decreasing d0
    vector<int> ord; for(int i=1;i<N;i++) if(i!=u) ord.push_back(i);
    sort(ord.begin(),ord.end(),[&](int a,int b){return d0[a]>d0[b];});
    // a_i doubled: A_i = du+L0-d0 = 2*a_i
    auto computeRS=[&](long long M)->long long{
        // best r among known cities (doubled positions), D=M
        long long best=LLONG_MAX;
        for(int i=0;i<N;i++) if(du[i]>=0 && i!=u && i!=0){
            long long A=du[i]+L0-d0[i]; // 2a
            long long r2=max(A,2*M-A);
            best=min(best,r2);
        }
        return best; // doubled
    };
    vector<char> skipped(N,0);
    long long M=L0;
    auto hubsAt=[&](long long Dd, long long &best, vector<long long>&H){
        best=LLONG_MAX; H.clear();
        for(int i=0;i<N;i++) if(du[i]>=0 && i!=u && i!=0){
            long long A=du[i]+L0-d0[i];
            long long r2=max(A,2*Dd-A);
            if(r2<best){best=r2;H.clear();}
            if(r2==best) H.push_back(A);
        }
        sort(H.begin(),H.end()); H.erase(unique(H.begin(),H.end()),H.end());
    };
    long long finalR2=-1; vector<long long> finalH;
    for(size_t k=0;k<ord.size();k++){
        int i=ord[k];
        long long Ls2=2*(L0-d0[i]);
        long long Dub=max(M,L0+d0[i]);
        long long b1,b2; vector<long long> H1,H2;
        hubsAt(M,b1,H1);
        if(!H1.empty() && b1<Ls2){
            bool ok=true;
            if(Dub>M){ hubsAt(Dub,b2,H2); ok=(b1==b2 && H1==H2); }
            if(ok){
                finalR2=b1; finalH=H1;
                for(size_t t=k;t<ord.size();t++) skipped[ord[t]]=1;
                break;
            }
        }
        du[i]=ask(u,i);
        M=max(M,du[i]);
    }
    if(finalR2<0){ hubsAt(M,finalR2,finalH); }
    vector<long long> A(N,-1);
    for(int i=0;i<N;i++) if(!skipped[i] && i!=u && i!=0) A[i]=du[i]+L0-d0[i];
    long long R=finalR2/2;
    vector<long long> hubs=finalH;
    int half=N/2, T=half+1;
    struct Cand{long long x; vector<int> mid;};
    vector<Cand> need;
    for(long long x:hubs){
        int left=1, right=0; vector<int> mid; // u is left
        for(int i=0;i<N;i++){
            if(i==u) continue;
            if(i==0||skipped[i]){right++;continue;}
            if(A[i]<x) left++; else if(A[i]>x) right++; else mid.push_back(i);
        }
        if(left>half||right>half) continue;
        if((int)mid.size()<T){ printf("! %lld\n",R); fflush(stdout); return 0;}
        need.push_back({x,mid});
    }
    for(auto &c:need){
        // h doubled: 2*h_i = 2*du - x
        auto same=[&](int i,int j)->bool{
            long long d=ask(i,j);
            long long hi2=2*du[i]-c.x, hj2=2*du[j]-c.x;
            return 2*d < hi2+hj2;
        };
        vector<int> E=c.mid; int m=E.size();
        { mt19937 rng(712367u); shuffle(E.begin(),E.end(),rng); }
        long long q0=cache_.size();
        int result=majority(E,T,same);
#ifdef DBG
        fprintf(stderr,"N=%d m=%d T=%d res=%d majq=%lld total=%zu\n",N,m,T,result,(long long)cache_.size()-q0,cache_.size());
#endif
        if(result==0){ printf("! %lld\n",R); fflush(stdout); return 0;}
    }
    printf("! %lld\n",-R); fflush(stdout); return 0;
}
