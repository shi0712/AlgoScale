// Relic Conveyor: O(n log(n+1)) time, O(n) auxiliary space.
// Based on the weighted-expression dynamic forest algorithm in the conversation.
// n <= 1,000,000; m <= 2500; T <= 10, sum(n) <= 1,000,000 per input file.
// C++17. No precomputed answer table. See admin/editorial.md for proofs.
#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
int addmod(int a, int b) { int s=a+b; return s>=MOD ? s-MOD:s; }
int submod(int a, int b) { int s=a-b; return s<0?s+MOD:s; }
int mulmod(long long a, long long b) { return int(a*b%MOD); }
int modpow(int a, int e) {
    int r=1;
    for (;e;e>>=1,a=mulmod(a,a)) if(e&1) r=mulmod(r,a);
    return r;
}

// 计数矩阵：[[g,z],[0,f]]。
struct Word {
    int g=1,z=0,f=1;
};
Word concat(const Word& a,const Word& b) {
    return {mulmod(a.g,b.g),addmod(mulmod(a.g,b.z),mulmod(a.z,b.f)),mulmod(a.f,b.f)};
}

// 仿射变换：g'=a*g+b，z'=c*g+d*z+e*f+t，f'=h*f+i。
struct Affine {
    int a=1,b=0,c=0,d=1,e=0,t=0,h=1,i=0;
};
Affine compose(const Affine& x,const Affine& y) {
    Affine r;
    r.a=mulmod(x.a,y.a);
    r.b=addmod(mulmod(x.a,y.b),x.b);
    r.c=addmod(mulmod(x.c,y.a),mulmod(x.d,y.c));
    r.d=mulmod(x.d,y.d);
    r.e=addmod(mulmod(x.d,y.e),mulmod(x.e,y.h));
    r.t=(1LL*x.c*y.b+1LL*x.d*y.t+1LL*x.e*y.i+x.t)%MOD;
    r.h=mulmod(x.h,y.h);
    r.i=addmod(mulmod(x.h,y.i),x.i);
    return r;
}
Word applyIdentity(const Affine& x) {
    return {addmod(x.a,x.b),addmod(addmod(x.c,x.e),x.t),addmod(x.h,x.i)};
}

struct PrefixInfo { int sum,last,finish; };
// sum = H(a)，last = J(a)，finish = 再处理一个 anchor 后的方案数。
PrefixInfo prefixInfo(const vector<int>& a,int anchor) {
    int n=(int)a.size();
    vector<int> ps(n+1),st;
    ps[0]=1;
    int last=1,total=1;
    for(int i=1;i<=n;i++) {
        while(!st.empty() && a[st.back()-1]<a[i-1]) st.pop_back();
        int L=st.empty()?0:st.back();
        int dp=0;
        if(a[i-1]>1) {
            dp=submod(ps[i-1],L?ps[L-1]:0);
            last=dp;
        }
        ps[i]=addmod(ps[i-1],dp);
        total=addmod(total,last);
        st.push_back(i);
    }
    while(!st.empty() && a[st.back()-1]<anchor) st.pop_back();
    int L=st.empty()?0:st.back();
    int finish=submod(ps[n],L?ps[L-1]:0);
    return {total,last,finish};
}

struct Forest {
    int n;
    vector<int> dep,sz,heavy,first,nxt;
    explicit Forest(const vector<int>& d):n((int)d.size()),dep(n+1),sz(n+1,1),heavy(n+1,-1),first(n+1,-1),nxt(n+1,-1) {
        dep[0]=-1;
        for(int i=1;i<=n;i++) dep[i]=d[i-1];
        vector<int> st{0},tail(n+1,-1);
        for(int i=1;i<=n;i++) {
            while(dep[st.back()]>dep[i]) st.pop_back();
            int pa=st.back();
            if(first[pa]==-1) first[pa]=i;
            else nxt[tail[pa]]=i;
            tail[pa]=i;
            st.push_back(i);
        }
        for(int u=n;u>=0;u--) {
            for(int v=first[u];v!=-1;v=nxt[v]) {
                sz[u]+=sz[v];
                if(heavy[u]==-1 || sz[v]>sz[heavy[u]]) heavy[u]=v;
            }
        }
    }
    // 第二层的特殊统计：只给原前缀 P 中的合法切分位置计权。
    int prefixMarks(int k,int suffixLength) const {
        vector<Word> dp(n+1);
        for(int u=n;u>=0;u--) {
            if(u && dep[u]>=k) { dp[u]=Word{}; continue; }
            Word x;
            if(u) {
                bool good=dep[u]<=k-2;
                x={int(good),int(good && u>suffixLength),1};
            }
            for(int v=first[u];v!=-1;v=nxt[v]) x=concat(x,dp[v]);
            if(u && dep[u]!=0) {
                x.g=addmod(x.g,1);
                x.f=addmod(x.f,1);
            }
            dp[u]=x;
        }
        return dp[0].z;
    }
};

// 全局加权平衡表达式树：每次修改沿父指针更新 O(log n) 个结点。
class DynamicDP {
    enum { LOCAL, COMPOSE, EXTRACT, CONCAT };
    struct Node {
        int type,l=-1,r=-1,par=-1,u=-1;
        // For CONCAT/EXTRACT only the first three fields store (g,z,f).
        // Other types use all eight fields as an Affine; avoids union lifetime issues.
        Affine a;
    };
    const Forest& F;
    vector<Node> tr;
    vector<int> state,loc;
    int root;

    Word word(int x) const {return x==-1?Word{}:Word{tr[x].a.a,tr[x].a.b,tr[x].a.c};}
    void pull(int x) {
        Node &v=tr[x];
        if(v.type==CONCAT) {
            Word w=concat(word(v.l),word(v.r));
            v.a.a=w.g; v.a.b=w.z; v.a.c=w.f;
        } else if(v.type==COMPOSE) {
            v.a=compose(tr[v.l].a,tr[v.r].a);
        } else if(v.type==EXTRACT) {
            Word w=applyIdentity(tr[v.l].a);
            v.a.a=w.g; v.a.b=w.z; v.a.c=w.f;
        } else {
            int u=v.u,eps;
            Word single;
            if(u==0) {
                single={1,0,1}; eps=0;
            } else if(state[u]==0) {
                single={0,0,0}; eps=1;
            } else {
                bool good=state[u]==2;
                single={int(good),int(good),1};
                eps=(F.dep[u]==0?0:1);
            }
            Word p=concat(single,word(v.l)),q=word(v.r);
            v.a={mulmod(p.g,q.g),eps,
                 mulmod(p.g,q.z),mulmod(p.g,q.f),mulmod(p.z,q.f),0,
                 mulmod(p.f,q.f),eps};
        }
    }
    int make(int type,int l=-1,int r=-1,int u=-1) {
        int x=(int)tr.size();
        tr.push_back(Node{type,l,r,-1,u,{}});
        if(l!=-1) tr[l].par=x;
        if(r!=-1) tr[r].par=x;
        pull(x);
        return x;
    }
    int buildRange(const vector<int>& ids,const vector<int>& ps,int l,int r,int type) {
        if(l>r) return -1;
        if(l==r) return ids[l];
        int target=ps[l]+(ps[r+1]-ps[l]+1)/2;
        int m=int(lower_bound(ps.begin()+l+1,ps.begin()+r+2,target)-ps.begin())-1;
        int a=buildRange(ids,ps,l,m-1,type);
        int b=ids[m];
        if(a!=-1) b=make(type,a,b);
        int c=buildRange(ids,ps,m+1,r,type);
        if(c!=-1) b=make(type,b,c);
        return b;
    }
    int balanced(const vector<int>& ids,const vector<int>& weights,int type) {
        vector<int> ps(ids.size()+1);
        for(int i=0;i<(int)ids.size();i++) ps[i+1]=ps[i]+weights[i];
        return buildRange(ids,ps,0,(int)ids.size()-1,type);
    }
    int buildChain(int head) {
        vector<int> ids,weights;
        for(int u=head;u!=-1;u=F.heavy[u]) {
            vector<int> left,lw,right,rw;
            bool after=false;
            for(int v=F.first[u];v!=-1;v=F.nxt[v]) {
                if(v==F.heavy[u]) {after=true;continue;}
                int x=buildChain(v);
                if(!after) {left.push_back(x);lw.push_back(F.sz[v]);}
                else {right.push_back(x);rw.push_back(F.sz[v]);}
            }
            int l=balanced(left,lw,CONCAT),r=balanced(right,rw,CONCAT);
            int x=make(LOCAL,l,r,u);
            loc[u]=x;
            ids.push_back(x);
            weights.push_back(F.sz[u]-(F.heavy[u]==-1?0:F.sz[F.heavy[u]]));
        }
        return make(EXTRACT,balanced(ids,weights,COMPOSE));
    }
public:
    explicit DynamicDP(const Forest& f):F(f),state(f.n+1),loc(f.n+1) {
        // N LOCAL + (N-H) COMPOSE + H EXTRACT + at most H-1 CONCAT.
        // H is the number of heavy paths, so fewer than 3*N nodes suffice.
        tr.reserve(3*(f.n+1));
        for(int u=1;u<=F.n;u++) {
            state[u]=(F.dep[u]==0?2:F.dep[u]==1?1:0);
        }
        root=buildChain(0);
    }
    void change(int u,int s) {
        state[u]=s;
        for(int x=loc[u];x!=-1;x=tr[x].par) pull(x);
    }
    int value() const {Word w=word(root); return addmod(w.f,w.z);}
};

int solve(const vector<int>& a) {
    int M=*max_element(a.begin(),a.end());
    int n=(int)a.size(),p=n-1;
    while(a[p]!=M) --p;
    if(M==1) return n%MOD;
    vector<int> pre(a.begin(),a.begin()+p),suf(a.begin()+p+1,a.end());
    PrefixInfo P=prefixInfo(pre,M);
    vector<int> d;
    d.reserve(n-1);
    for(int x:suf) d.push_back(M-1-x);
    for(int x:pre) d.push_back(M-x);
    int ones=1;
    for(int x:d) if(x==0) ++ones;
    int ans=addmod(P.sum,ones%MOD);
    if(M==2) return ans;

    Forest F(d);
    int K=P.finish;
    int r=0;
    for(int x:pre) {
        if(x>=3) break;
        if(x==2) ++r;
    }
    int K0=mulmod(K,modpow(r+1,MOD-2));
    int q=0;
    for(int i=(int)suf.size()-1;i>=0 && suf[i]==1;i--) ++q;
    PrefixInfo C=prefixInfo(suf,M);
    int B=submod(mulmod(K,C.sum),mulmod(mulmod(C.last,q),submod(K,K0)));
    int A=F.prefixMarks(M-1,(int)suf.size());
    ans=addmod(ans,addmod(A,B));

    int limit=M-2;
    if(limit<2) return ans;
    // Release rotation buffers before allocating the expression tree.
    vector<int>().swap(pre); vector<int>().swap(suf); vector<int>().swap(d);
    DynamicDP dp(F);
    struct Event {int k,u,s;};
    vector<Event> ev;
    ev.reserve(2*F.n);
    for(int u=1;u<=F.n;u++) {
        int k=F.dep[u]+1;
        if(3<=k && k<=limit) ev.push_back({k,u,1});
        ++k;
        if(3<=k && k<=limit) ev.push_back({k,u,2});
    }
    sort(ev.begin(),ev.end(),[](const Event& x,const Event& y) {return x.k<y.k;});
    int now=2;
    for(int i=0;i<(int)ev.size();) {
        int k=ev[i].k;
        ans=addmod(ans,mulmod(dp.value(),k-now));
        while(i<(int)ev.size() && ev[i].k==k) {
            dp.change(ev[i].u,ev[i].s); ++i;
        }
        now=k;
    }
    ans=addmod(ans,mulmod(dp.value(),limit-now+1));
    return ans;
}

#ifndef RELIC_LIBRARY
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int c,T;
    if(!(cin>>c>>T)) return 0;
    while(T--) {
        int n,m;cin>>n>>m;
        vector<int> a(n);
        for(int &x:a) cin>>x;
        cout<<solve(a)<<'\n';
    }
}

#endif
