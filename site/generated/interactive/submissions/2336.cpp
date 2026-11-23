#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
typedef array<u64,4> Bits;
struct BH{ size_t operator()(const Bits&b)const{ u64 h=0x9e3779b97f4a7c15ULL; for(int i=0;i<4;i++){ u64 x=b[i]+h+0x9e3779b97f4a7c15ULL*(i+1); x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL; x=(x^(x>>27))*0x94d049bb133111ebULL; x^=x>>31; h^=x+(h<<6)+(h>>2);} return h; } };
struct Entry{ int failD=-1; int succD=1000; int q=-1; };
static inline int pc(const Bits&b){return __builtin_popcountll(b[0])+__builtin_popcountll(b[1])+__builtin_popcountll(b[2])+__builtin_popcountll(b[3]);}
static inline Bits band(const Bits&a,const Bits&b){return {a[0]&b[0],a[1]&b[1],a[2]&b[2],a[3]&b[3]};}
static inline Bits bandn(const Bits&a,const Bits&b){return {a[0]&~b[0],a[1]&~b[1],a[2]&~b[2],a[3]&~b[3]};}
struct Sig{
    int k,m,C; vector<vector<int>> orders; vector<pair<int,int>> qs; vector<Bits> Q;
    unordered_map<Bits,Entry,BH> memo; long long nodes,budget; bool aborted; Bits full;
    void build(int k_,int Lsize){
        k=k_; m=min(Lsize,2);
        vector<int> lst; for(int i=0;i<m;i++) lst.push_back(k+i);
        function<void(int,int)> rec=[&](int t,int inserted){
            if(t<0){orders.push_back(lst);return;}
            int maxs=min(2,Lsize+inserted);
            for(int s=0;s<=maxs;s++){
                lst.insert(lst.begin()+s,t);
                rec(t-1,inserted+1);
                lst.erase(lst.begin()+s);
            }
        };
        rec(k-1,0);
        C=orders.size();
        int T=k+m;
        for(int u=0;u<T;u++)for(int v=u+1;v<T;v++){ if(u>=k&&v>=k)continue; qs.push_back({u,v}); }
        Q.assign(qs.size(),Bits{0,0,0,0});
        full={0,0,0,0};
        for(int c=0;c<C;c++){
            full[c>>6]|=1ULL<<(c&63);
            vector<int> rk(T);
            for(int i=0;i<T;i++) rk[orders[c][i]]=i;
            for(size_t q=0;q<qs.size();q++) if(rk[qs[q].first]<rk[qs[q].second]) Q[q][c>>6]|=1ULL<<(c&63);
        }
    }
    bool dfs(const Bits&s,int d){
        int c=pc(s); if(c<=1) return true; if(d>=40) return true;
        if((long long)c>(1LL<<d)) return false;
        auto it=memo.find(s);
        if(it!=memo.end()){ if(it->second.succD<=d) return true; if(it->second.failD>=d) return false; }
        if(++nodes>budget){aborted=true;return false;}
        int half=1<<(d-1);
        vector<pair<int,int>> cand;
        for(size_t q=0;q<qs.size();q++){
            int c0=pc(band(s,Q[q])); int c1=c-c0;
            if(c0==0||c1==0) continue;
            int mx=max(c0,c1); if(mx>half) continue;
            cand.push_back({mx,(int)q});
        }
        sort(cand.begin(),cand.end());
        for(auto&pr:cand){
            int q=pr.second;
            Bits a=band(s,Q[q]); Bits b=bandn(s,Q[q]);
            if(dfs(a,d-1)&&dfs(b,d-1)){
                Entry&e=memo[s];
                if(d<e.succD){e.succD=d;e.q=q;}
                return true;
            }
            if(aborted) return false;
        }
        Entry&e=memo[s]; e.failD=max(e.failD,d);
        return false;
    }
    int find(int maxD,long long bud){
        int lb=0; while((1LL<<lb)<C) lb++;
        for(int d=lb;d<=maxD;d++){
            aborted=false; nodes=0; budget=bud;
            if(dfs(full,d)) return d;
        }
        return -1;
    }
};
int main(){
    int n; if(!(cin>>n)) return 0;
    if(n==1){ printf("! 1\n"); fflush(stdout); return 0; }
    // choose main block size
    int B; Sig* mainSig=nullptr;
    {
        Sig* s5=new Sig(); s5->build(5,2);
        if(n>=7 && s5->find(8,2000000)>0){ B=5; mainSig=s5; }
        else {
            delete s5;
            Sig* s3=new Sig(); s3->build(3,2);
            if(s3->find(5,3000000)>0){ B=3; mainSig=s3; }
            else { delete s3; Sig* s1=new Sig(); s1->build(1,2); s1->find(10,100000000); B=1; mainSig=s1; }
        }
    }
    int k0=-1;
    for(int x=2;x<=B+1;x++){ if(x<=n && (n-x)%B==0){k0=x;break;} }
    Sig* firstSig=new Sig(); firstSig->build(k0,0); firstSig->find(60,100000000);
    vector<int> Lrev; // back = smallest
    auto runBlock=[&](Sig* sg,int start)->bool{
        int k=sg->k,m=sg->m;
        auto posOf=[&](int t)->int{ if(t<k) return start+t; return Lrev[Lrev.size()-1-(t-k)]; };
        Bits s=sg->full;
        while(pc(s)>1){
            auto it=sg->memo.find(s);
            int q=it->second.q;
            int u=sg->qs[q].first,v=sg->qs[q].second;
            printf("? %d %d\n",posOf(u),posOf(v)); fflush(stdout);
            string r; if(!(cin>>r)) return false;
            if(r=="<") s=band(s,sg->Q[q]);
            else if(r==">") s=bandn(s,sg->Q[q]);
            else return false;
        }
        int c=0; for(int i=0;i<4;i++) if(s[i]){ c=i*64+__builtin_ctzll(s[i]); break; }
        vector<int> ord; for(int t:sg->orders[c]) ord.push_back(posOf(t));
        for(int i=0;i<m;i++) Lrev.pop_back();
        for(int i=(int)ord.size()-1;i>=0;i--) Lrev.push_back(ord[i]);
        return true;
    };
    int cur=n;
    if(!runBlock(firstSig,cur-k0+1)) return 0;
    cur-=k0;
    while(cur>0){
        if(!runBlock(mainSig,cur-B+1)) return 0;
        cur-=B;
    }
    vector<int> a(n+1);
    int sz=Lrev.size();
    for(int r=0;r<sz;r++){ a[Lrev[sz-1-r]]=r+1; }
    string out="!";
    for(int i=1;i<=n;i++){ out+=' '; out+=to_string(a[i]); }
    out+='\n';
    fputs(out.c_str(),stdout); fflush(stdout);
    return 0;
}
