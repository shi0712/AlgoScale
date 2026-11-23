// core solver; ASK(l,r) must be provided as a callable
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
struct Solver {
    int n;
    function<ll(int,int)> rawask;
    map<pair<int,int>,ll> cache;
    ll ask(int l,int r){auto it=cache.find({l,r});if(it!=cache.end())return it->second;ll v=rawask(l,r);cache[{l,r}]=v;return v;}
    vector<int> st; // 1 pos known, 0 nonpos, -1 unknown
    vector<ll> val, b;
    int policy=100; int probe=1; int useRF=1; double pendTh=0.3; double singTh=0.35; int dbl=1; int grp=1; long long gq=0,gm=0,gp=0,nb=0;
    // discovery
    void resolve(int l,int r,ll M){
        int len=r-l+1;
        if(len==1){st[l]=1;val[l]=M;return;}
        int mid=l+len/2-1;
        ll s=ask(l,mid);
        if(s==0){for(int i=l;i<=mid;i++)st[i]=0;resolve(mid+1,r,M);return;}
        resolve(l,mid,s);
        bool allpos=true;for(int i=l;i<=mid;i++)if(st[i]!=1)allpos=false;
        if(allpos && r==mid+1){
            if(M>s){st[r]=1;val[r]=M-s;}else st[r]=0;
            return;
        }
        test(mid+1,r);
    }
    void test(int l,int r){
        ll R=ask(l,r);
        if(R==0){for(int i=l;i<=r;i++)st[i]=0;return;}
        resolve(l,r,R);
    }
    double th[11]={0.70,0.80,0.86,0.9,0.93,0.95,0.97,0.98,0.99,0.995,2};
    int ms[11]={2,3,4,5,6,8,12,16,24,48,96};
    int split=0;
    int resolveFirst(int l,int r,ll M){
        int lo=l,hi=r;ll Mh=M;
        while(lo<hi){
            int mid=(split==0)?(lo+(hi-lo+1)/2-1):lo;
            ll s=ask(lo,mid);
            if(s==0){for(int k=lo;k<=mid;k++)st[k]=0;lo=mid+1;}
            else{hi=mid;Mh=s;}
        }
        st[lo]=1;val[lo]=Mh;
        if(lo==r-1){ if(M>Mh){st[r]=1;val[r]=M-Mh;}else st[r]=0; return r+1;}
        return lo+1;
    }
    void discover2(){
        if(probe && n>=12){ll R0=ask(1,n);if(R0==0){for(int k=1;k<=n;k++)st[k]=0;return;}}
        int i=1; int prev=0; double T[2][2]={{1,1},{1,1}}; int lastS=-1; int cntUpto=1; int lastZero=0;
        auto upd=[&](){while(cntUpto<i){int sg=st[cntUpto];if(sg==1||sg==0){if(lastS>=0)T[lastS][sg==0]+=1;lastS=(sg==1?1:0);}cntUpto++;}};
        while(i<=n){
            upd();
            int ps=(lastS<0?0:lastS);
            double q=T[ps][1]/(T[ps][0]+T[ps][1]);
            double qmm=T[0][1]/(T[0][0]+T[0][1]);
            if(prev==1 && i+1<=n && q*qmm<pendTh){
                ll t=ask(i+1,i+1);
                if(t>0){st[i+1]=1;val[i+1]=t;st[i]=-2;i+=2;prev=1;}
                else{st[i+1]=0;test(i,i);i+=2;prev=0;}
                continue;
            }
            int m=2;for(int k=0;k<11;k++)if(q<th[k]){m=ms[k];break;}
            if(prev==0 && lastS==0 && q<singTh) m=1;
            if(lastZero>0 && dbl && q>0.99) m=max(m,2*lastZero);
            m=min(m,n-i+1);
            ll R=ask(i,i+m-1);
            if(R==0){for(int k=i;k<i+m;k++)st[k]=0;lastZero=m;prev=0;i+=m;}
            else if(useRF){int ni=resolveFirst(i,i+m-1,R);lastZero=0;prev=(st[ni-1]==1);i=ni;}
            else{resolve(i,i+m-1,R);lastZero=0;prev=(st[i+m-1]==1);i+=m;}
        }
    }
    void discover(){
        if(policy==100){discover2();return;}
        int i=1,m=(policy==0?1:(policy>=10?policy-10:2));
        while(i<=n){
            int len=min(m,n-i+1);
            ll R=ask(i,i+len-1);
            if(R==0){for(int j=i;j<i+len;j++)st[j]=0;if(policy==2)m=min(m*2,n);}
            else{resolve(i,i+len-1,R);if(policy==2)m=2;}
            i+=len;
        }
    }
    // regions
    struct Node{int type,l,r,A,B;ll z;};
    vector<Node> nodes;
    int leaf(int l,int r){nodes.push_back({0,l,r,-1,-1,0});return nodes.size()-1;}
    int comp(int A,ll z,int B){nodes.push_back({1,0,0,A,B,z});return nodes.size()-1;}
    bool empty(int x){return nodes[x].type==0&&nodes[x].l>nodes[x].r;}
    void assign(int x,ll T){
        // iterative over B-chain
        while(true){
            Node&nd=nodes[x];
            if(nd.type==0){ if(nd.l<=nd.r){b[nd.l]=T;for(int i=nd.l+1;i<=nd.r;i++)b[i]=0;} return;}
            ll z=nd.z;int A=nd.A,B=nd.B;
            if(!empty(A)){assign(A,-z);}
            else T=T-z;
            x=B;
        }
    }
    struct It{int L,R;ll v;int reg;};
    vector<ll> run(){
        st.assign(n+2,-1);val.assign(n+2,0);b.assign(n+2,0);
        discover();
        for(int i=1;i<=n;i++)if(st[i]==1)b[i]=val[i];else b[i]=0;
        for(int i=1;i<=n;i++)assert(st[i]!=-1);
        const ll BIG=100000000000000LL;
        // build blocks, resolving pending
        vector<It> blocks;
        {
            int i=1;
            while(i<=n){
                if(st[i]!=1){i++;continue;}
                int j=i;ll s=0;while(j<=n&&st[j]==1){s+=val[j];j++;}
                It y{i,j-1,s,-1};
                if(!blocks.empty() && blocks.back().R==i-2 && st[i-1]==-2){
                    It x=blocks.back();
                    ll q=ask(x.L,y.R);
                    if(q>max(x.v,y.v)){
                        ll g=q-x.v-y.v; b[i-1]=g; st[i-1]=3; // known
                        blocks.back()=It{x.L,y.R,q,-1};
                    }else{st[i-1]=0;blocks.push_back(y);}
                }else blocks.push_back(y);
                i=j;
            }
        }
        vector<It> stk;
        int prevEnd=0;
        for(auto y:blocks){
            nb++;
            int g=leaf(prevEnd+1,y.L-1);
            int yR=y.R;
            while(true){
                if(stk.empty()){y.reg=g;stk.push_back(y);break;}
                if(grp){
                    int c=0;while(c<(int)stk.size()&&stk[stk.size()-1-c].v<=y.v)c++;
                    if(c>=2){
                        ll q=ask(stk[stk.size()-c].L,y.R);gq++;
                        if(q==y.v){
                            for(int k=0;k<c;k++){It x=stk.back();stk.pop_back();g=comp(x.reg,x.v,g);gp++;}
                            continue;
                        }
                    }
                }
                It x=stk.back();
                ll q=ask(x.L,y.R);gq++;
                if(q>max(x.v,y.v)){
                    assign(g,q-x.v-y.v);gm++;
                    stk.pop_back();
                    y=It{x.L,y.R,q,-1};g=x.reg;
                }else if(x.v<=y.v){
                    stk.pop_back();
                    g=comp(x.reg,x.v,g);gp++;
                }else{y.reg=g;stk.push_back(y);break;}
            }
            prevEnd=yR;
        }
        for(auto&e:stk)assign(e.reg,-BIG);
        return vector<ll>(b.begin()+1,b.begin()+n+1);
    }
};
int main(){
    int n; if(scanf("%d",&n)!=1) return 0;
    Solver S; S.n=n;
    S.rawask=[&](int l,int r)->ll{
        printf("? %d %d\n",l,r); fflush(stdout);
        ll v; if(scanf("%lld",&v)!=1) exit(0);
        if(v<0) exit(0);
        return v;
    };
    auto b=S.run();
    printf("!");
    for(int i=0;i<n;i++) printf(" %lld",b[i]);
    printf("\n"); fflush(stdout);
    return 0;
}
