#include <bits/stdc++.h>
using namespace std;
static constexpr int MOD = 998244353;
struct FastInput {
    static constexpr int SZ = 1 << 16;
    char buf[SZ]; int pos=0, len=0;
    inline char get() { if(pos==len){len=fread(buf,1,SZ,stdin);pos=0;if(!len)return 0;} return buf[pos++]; }
    int read(){char c; do {c=get();} while(c && c<'0'); int x=0; while(c>='0'&&c<='9'){x=x*10+c-'0';c=get();} return x;}
};
int modpow(int a,int b){int r=1;while(b){if(b&1)r=(long long)r*a%MOD;a=(long long)a*a%MOD;b>>=1;}return r;}
int main(){
    FastInput in; int t=in.read();
    const int N=1000000;
    vector<int> fac(N+1), invfac(N+1); fac[0]=1;
    for(int i=1;i<=N;i++) fac[i]=(long long)fac[i-1]*i%MOD;
    invfac[N]=modpow(fac[N],MOD-2);
    for(int i=N;i;i--) invfac[i-1]=(long long)invfac[i]*i%MOD;
    string output; output.reserve(t*11);
    while(t--){
        int n=in.read(), k=in.read();
        vector<int> head(n+1,0), next(n+1), dep(n+1);
        for(int v=2;v<=n;v++){int p=in.read(); next[v]=head[p];head[p]=v;dep[v]=dep[p]+1;}
        if(k==1){output+=to_string(n);output+='\n';continue;}
        if(k==2){output+=to_string((long long)n*(n-1)%MOD);output+='\n';continue;}
        int r=k-1;
        vector<vector<int>> sub(n+1);
        for(int v=n;v>=1;--v){
            int h=0;
            for(int c=head[v];c;c=next[c]) h=max(h,(int)sub[c].size());
            sub[v].assign(h+1,0);sub[v][0]=1;
            for(int c=head[v];c;c=next[c])
                for(int j=0;j<(int)sub[c].size();j++) sub[v][j+1]+=sub[c][j];
        }
        int maxdep=*max_element(dep.begin(),dep.end());
        int offset=maxdep+2;
        vector<int> cnt(2*maxdep+5,0);
        auto choose=[&](int x)->int { if(x<r)return 0; return (long long)fac[x]*invfac[r]%MOD*invfac[x-r]%MOD; };
        int cur=0;
        for(int j=0;j<(int)sub[1].size();j++) {cnt[offset+j]=sub[1][j];cur+=choose(sub[1][j]);if(cur>=MOD)cur-=MOD;}
        int ans=cur;
        auto change=[&](int idx,int delta){
            int &x=cnt[offset+idx];
            cur-=choose(x); if(cur<0)cur+=MOD;
            x+=delta;
            cur+=choose(x); if(cur>=MOD)cur-=MOD;
        };
        auto move_down=[&](int v){
            int base=-dep[v];
            for(int j=0;j<(int)sub[v].size();j++) change(base+j+2,-sub[v][j]);
            for(int j=0;j<(int)sub[v].size();j++) change(base+j,sub[v][j]);
        };
        auto move_up=[&](int v){
            int base=-dep[v];
            for(int j=0;j<(int)sub[v].size();j++) change(base+j,-sub[v][j]);
            for(int j=0;j<(int)sub[v].size();j++) change(base+j+2,sub[v][j]);
        };
        vector<pair<int,int>> st; st.reserve(n); st.emplace_back(1,head[1]);
        while(!st.empty()){
            int &c=st.back().second;
            if(c){int v=c;c=next[c];move_down(v);ans+=cur;if(ans>=MOD)ans-=MOD;st.emplace_back(v,head[v]);}
            else {int v=st.back().first;st.pop_back();if(v!=1)move_up(v);}
        }
        output+=to_string(ans);output+='\n';
    }
    fwrite(output.data(),1,output.size(),stdout);
}
