#include <bits/stdc++.h>
using namespace std;
static int n; static vector<vector<int>> adjK; static set<pair<int,int>> E;
static vector<char> satv; static mt19937 rng(1234567u);
static vector<char> doQuery(const vector<char>& inA){
    string out; out.reserve(2*n+4); out+='?';
    for(int i=0;i<n;i++){ out+=' '; out+=char('0'+(inA[i]?1:0)); }
    out+='\n'; fwrite(out.data(),1,out.size(),stdout); fflush(stdout);
    static char buf[8192];
    if(scanf("%8000s",buf)!=1) exit(0);
    if(buf[0]=='-') exit(0);
    if((int)strlen(buf)!=n) exit(0);
    vector<char> res(n); for(int i=0;i<n;i++) res[i]=(buf[i]=='1'); return res;
}
static void addEdge(int u,int v){ if(u==v)return; if(u>v)swap(u,v);
    if(E.insert(make_pair(u,v)).second){ adjK[u].push_back(v); adjK[v].push_back(u);} }
static long long binom(int a,int b){ if(b<0||b>a)return 0; long long r=1;
    for(int i=0;i<b;i++) r=r*(a-i)/(i+1); return r; }
struct RR{ int y; vector<int> res; };
static RR doRound(const vector<int>& A){
    RR rr; rr.y=0; int m=(int)A.size(); if(m==0) return rr;
    int t=1; while(binom(t,max(1,t/2))<(long long)m) ++t; int w=max(1,t/2);
    vector<int> codes; codes.reserve(m);
    for(int mask=0;mask<(1<<t)&&(int)codes.size()<m;++mask)
        if(__builtin_popcount((unsigned)mask)==w) codes.push_back(mask);
    vector<int> codeOf(n,-1); unordered_map<int,int> ctv; ctv.reserve(m*2+1);
    for(int i=0;i<m;++i){ codeOf[A[i]]=codes[i]; ctv.emplace(codes[i],A[i]); }
    vector<int> z(n,0);
    for(int b=0;b<t;++b){ vector<char> inA(n,0); bool any=false;
        for(int v:A) if((codeOf[v]>>b)&1){ inA[v]=1; any=true; }
        if(!any) continue; vector<char> r=doQuery(inA);
        for(int i=0;i<n;++i) if(r[i]) z[i]|=(1<<b); }
    int before=(int)E.size();
    for(int i=0;i<n;++i){ if(z[i]==0) continue; auto it=ctv.find(z[i]);
        if(it!=ctv.end()){ addEdge(i,it->second); rr.res.push_back(i);} }
    rr.y=(int)E.size()-before; return rr;
}
static void components(vector<int>& comp, vector<vector<int>>& members){
    comp.assign(n,-1); members.clear();
    for(int i=0;i<n;++i) if(comp[i]<0){ int c=(int)members.size(); members.push_back(vector<int>());
        vector<int> st; st.push_back(i); comp[i]=c;
        while(!st.empty()){ int v=st.back(); st.pop_back(); members[c].push_back(v);
            for(int u:adjK[v]) if(comp[u]<0){ comp[u]=c; st.push_back(u);} } }
}
static void endgame(){
    if((int)E.size()==n-1) return;
    vector<int> comp; vector<vector<int>> members; components(comp,members);
    int c=(int)members.size(); if(c<=1) return;
    vector<int> order(c); iota(order.begin(),order.end(),0);
    sort(order.begin(),order.end(),[&](int a,int b){return members[a].size()<members[b].size();});
    vector<vector<int>> f(c, vector<int>(c,-1));
    for(int idx=0; idx<c && (int)E.size()<n-1; ++idx){ int ci=order[idx];
        vector<char> inA(n,0); for(int v:members[ci]) inA[v]=1;
        vector<char> r=doQuery(inA);
        for(int v=0;v<n;++v){ if(!r[v]) continue; int cj=comp[v]; if(cj==ci) continue;
            f[ci][cj]=v; int u=f[cj][ci];
            if(u<0 && members[ci].size()==1) u=members[ci][0];
            if(u>=0) addEdge(u,v); } }
}
static void finish(){ if((int)E.size()!=n-1) endgame();
    string out="!\n";
    for(const auto& e:E){ out+=to_string(e.first+1); out+=' '; out+=to_string(e.second+1); out+='\n'; }
    fwrite(out.data(),1,out.size(),stdout); fflush(stdout); }
int main(){
    if(scanf("%d",&n)!=1) return 0;
    adjK.assign(n,vector<int>()); satv.assign(n,0);
    if(n==2){ printf("!\n1 2\n"); fflush(stdout); return 0; }
    { vector<char> inA(n,0); inA[0]=1; vector<char> r=doQuery(inA);
      vector<int> nb; for(int i=0;i<n;++i) if(r[i]) nb.push_back(i);
      for(int v:nb) addEdge(0,v);
      if((int)E.size()==n-1){ finish(); return 0; }
      if(nb.size()==1){ satv[0]=1;
        if(n==3){ int x=nb[0],y=-1; for(int i=0;i<n;++i) if(i!=0&&i!=x) y=i;
            addEdge(x,y); finish(); return 0; } } }
    { vector<int> A(n); iota(A.begin(),A.end(),0); RR rr=doRound(A);
      for(int v:rr.res) satv[v]=1; }
    int mode=0, rounds=0;
    while((int)E.size()<n-1){
        int c=n-(int)E.size();
        if(c<=26||rounds>=16){ endgame(); break; }
        int remaining=(n-1)-(int)E.size();
        vector<int> cand; for(int i=0;i<n;++i) if(!satv[i]) cand.push_back(i);
        vector<int> A;
        if(mode==0){
            vector<int> kp(n,0);
            for(int v:cand){ int cnt=0; for(int u:adjK[v]) if(!satv[u]) ++cnt; kp[v]=cnt; }
            for(int v:cand){
                if(kp[v]==0){ if(rng()&1u) A.push_back(v); }
                else if(kp[v]==1){ int u=-1; for(int w:adjK[v]) if(!satv[w]){u=w;break;}
                    if(u>=0&&kp[u]>=2) A.push_back(v); } }
        } else {
            vector<int> ord=cand; shuffle(ord.begin(),ord.end(),rng);
            vector<char> inA(n,0);
            for(int v:ord){ if(rng()&1u){ bool ok=true;
                for(int u:adjK[v]) if(inA[u]){ ok=false; break; }
                if(ok){ inA[v]=1; A.push_back(v);} } }
        }
        if(A.empty()){ for(int v:cand) if(rng()&1u) A.push_back(v); }
        if(A.empty()){ endgame(); break; }
        RR rr=doRound(A); ++rounds;
        if(rr.y*10<remaining) mode^=1;
    }
    finish(); return 0;
}
