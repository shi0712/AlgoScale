#include <bits/stdc++.h>
using namespace std;
// Online algorithm: Kruskal hierarchy + root-set DP + grouped fast subset
// convolutions. No precomputed answers. See admin/editorial.md for proof.
using U = uint32_t;
using W = uint64_t;
static constexpr U MOD=1000000007;
static constexpr int SMALL=5; // only the constant-size convolution kernel is quadratic
inline U addm(U a,U b){ U c=a+b; return c>=MOD?c-MOD:c; }
inline U subm(U a,U b){ return a>=b?a-b:a+MOD-b; }
inline U negm(U a){return a?MOD-a:0;}
inline U mulm(U a,U b){return W(a)*b%MOD;}
U modpow(U x,int y){U r=1;for(;y;y>>=1,x=mulm(x,x))if(y&1)r=mulm(r,x);return r;}
static U po[801],qp[801],iv[21];
static uint64_t grouped_batches=0, fast_kernels=0, small_kernels=0;

// A ranked zeta transform. Storage is state-major; rank is the number of blocks.
struct Ranked {
    int a,M,L;
    vector<U> v;
    vector<unsigned char> pc;
    explicit Ranked(int n):a(n),M(1<<n),L(n+1),v(size_t(M)*L),pc(M){
        for(int s=1;s<M;++s)pc[s]=pc[s>>1]+(s&1);
    }
    U* row(int s){return v.data()+size_t(s)*L;}
    const U* row(int s)const{return v.data()+size_t(s)*L;}
    void load(const vector<U>& f){
        fill(v.begin(),v.end(),0);
        for(int s=0;s<M;++s)row(s)[pc[s]]=f[s];
    }
    void zeta(){
        for(int b=1;b<M;b<<=1)for(int base=0;base<M;base+=b*2)
            for(int s=0;s<b;++s){
                U *x=row(base+b+s); const U*y=row(base+s);
                for(int r=0;r<=pc[base+s];++r)x[r]=addm(x[r],y[r]);
            }
    }
    void mobius(){
        for(int b=1;b<M;b<<=1)for(int base=0;base<M;base+=b*2)
            for(int s=0;s<b;++s){
                int t=base+b+s; U*x=row(t);const U*y=row(base+s);
                for(int r=pc[t];r<=a;++r)x[r]=subm(x[r],y[r]);
            }
    }
    vector<U> diagonal()const{
        vector<U> r(M);for(int s=0;s<M;++s)r[s]=row(s)[pc[s]];return r;
    }
};
// Dot product of at most 21 residues; reduce every 8 terms to prevent overflow.
U dot(const U*x,const U*y,int r,int lo,int hi){
    W t=0;int c=0;
    for(int j=lo;j<=hi;++j){t+=W(x[j])*y[r-j];if(++c==8){t%=MOD;c=0;}}
    return t%MOD;
}
vector<U> convolution(const vector<U>&f,const vector<U>&g,int a){
    int M=1<<a;
    if(a<=SMALL){
        ++small_kernels;vector<U> h(M);
        for(int s=0;s<M;++s){U z=0;for(int t=s;;t=(t-1)&s){z=addm(z,mulm(f[t],g[s^t]));if(!t)break;}h[s]=z;}
        return h;
    }
    ++fast_kernels;Ranked F(a);F.load(f);F.zeta();
    {
        Ranked G(a);G.load(g);G.zeta();
        for(int s=0;s<M;++s){
            U tmp[21]{};const U*x=F.row(s),*y=G.row(s);int p=F.pc[s];
            for(int r=0;r<=a;++r)tmp[r]=dot(x,y,r,max(0,r-p),min(r,p));
            copy(tmp,tmp+a+1,F.row(s));
        }
    }
    F.mobius();return F.diagonal();
}
// Divide one transformed series by another (denominator constant coefficient 1).
void poly_div(U*num,const U*den,int a,int den_degree){
    U out[21]{};
    for(int r=0;r<=a;++r){U t=dot(den,out,r,1,min(r,den_degree));out[r]=subm(num[r],t);}
    copy(out,out+a+1,num);
}

struct Edge{int u,v,w;};
struct DSU{
    vector<int> p;
    explicit DSU(int n):p(n){iota(p.begin(),p.end(),0);}
    int find(int x){return p[x]==x?x:p[x]=find(p[x]);}
    void unite(int a,int b){a=find(a);b=find(b);if(a!=b)p[b]=a;}
};
struct Node{vector<int> children,vertices;vector<Edge> edges;};
struct Distribution{vector<int> vertices;vector<U> p;};

class Merge {
    int k,N,M,K;
    vector<Distribution> ch;
    vector<int> sz,off,owner;
    vector<U> cm,adj,expand,support,P,H;
    vector<U> pfull,ptotal;
    int ec(U A,U B)const{int z=0;while(B){int v=__builtin_ctz(B);B&=B-1;z+=__builtin_popcount(adj[v]&A);}return z;}
    void compute_h(){
        H.assign(M,0);H[0]=1;
        vector<int> nonsingle;
        for(int i=0;i<k;++i)if(sz[i]>1)nonsingle.push_back(i);
        int nb=nonsingle.size();
        // Proper submasks precede their supersets in numerical order.
        for(int bs=0;bs<(1<<nb);++bs){
            vector<int>B,A;U bm=0;
            for(int j=0;j<nb;++j)if(bs>>j&1)B.push_back(nonsingle[j]),bm|=1u<<nonsingle[j];
            for(int i=0;i<k;++i)if(!(bm>>i&1))A.push_back(i);
            int b=B.size(),a=A.size(),Q=1<<a,ZM=1<<b;
            vector<U> full(Q),base(Q);vector<int>d(Q);
            for(int x=1;x<Q;++x){int j=__builtin_ctz((unsigned)x),y=x&(x-1);U block=cm[A[j]];
                full[x]=full[y]|block;d[x]=d[y]+ec(block,full[y]);}
            for(int x=0;x<Q;++x)base[x]=mulm(P[full[x]],po[d[x]]);
            vector<U> chosen(b),rz(ZM),uz(ZM);
            for(int z=1;z<ZM;++z){int j=__builtin_ctz((unsigned)z);uz[z]=uz[z&(z-1)]|cm[B[j]];}
            function<void(int)> enumerate=[&](int j){
                if(j<b){
                    int i=B[j];for(int r=1;r<(1<<sz[i])-1;++r)if(ch[i].p[r]){
                        chosen[j]=U(r)<<off[i];enumerate(j+1);
                    }
                    return;
                }
                ++grouped_batches;
                for(int z=1;z<ZM;++z){int t=__builtin_ctz((unsigned)z);rz[z]=rz[z&(z-1)]|chosen[t];}
                U root=rz.back(),allB=uz.back();
                vector<U> ker(Q),fg(Q),gg(Q),rhs(Q);
                auto setup=[&](int z){
                    U rem=allB^uz[z];
                    U c=mulm(P[root^rz[z]],qp[ec(rem,rz[z])]);
                    vector<int> rdeg(a),ldeg(a),re(Q),le(Q);
                    for(int t=0;t<a;++t){rdeg[t]=ec(cm[A[t]],rz[z]);ldeg[t]=ec(rem,cm[A[t]]);}
                    for(int x=1;x<Q;++x){int t=__builtin_ctz((unsigned)x),y=x&(x-1);re[x]=re[y]+rdeg[t];le[x]=le[y]+ldeg[t];}
                    for(int x=0;x<Q;++x){
                        fg[x]=mulm(mulm(H[rz[z]|full[x]],po[d[x]]),qp[le[x]]);
                        gg[x]=mulm(base[x],qp[re[x]]);
                    }
                    return c;
                };
                // The unknown batch has a unit-constant convolution denominator.
                setup(ZM-1);ker=gg;
                vector<U> ans(Q);
                if(a<=SMALL){
                    ++small_kernels;
                    if(!b)rhs[0]=1;
                    for(int z=0;z<ZM-1;++z){U c=setup(z);if(!c)continue;
                        for(int x=0;x<Q;++x){U sum=0;for(int y=x;;y=(y-1)&x){sum=addm(sum,mulm(fg[y],gg[x^y]));if(!y)break;}
                            rhs[x]=subm(rhs[x],mulm(c,sum));}
                    }
                    for(int x=0;x<Q;++x){U t=rhs[x];
                        // sum_{y proper subset of x} ans[y] * ker[x\y]
                        if(x)for(int y=(x-1)&x;;y=(y-1)&x){t=subm(t,mulm(ans[y],ker[x^y]));if(!y)break;}
                        ans[x]=t;
                    }
                }else if(!b){
                    ++fast_kernels;Ranked F(a);F.load(ker);F.zeta();
                    for(int x=0;x<Q;++x){U den[21];copy(F.row(x),F.row(x)+a+1,den);fill(F.row(x),F.row(x)+a+1,0);F.row(x)[0]=1;poly_div(F.row(x),den,a,F.pc[x]);}
                    F.mobius();ans=F.diagonal();
                }else{
                    ++fast_kernels;Ranked Acc(a),F(a),G(a);
                    for(int z=0;z<ZM-1;++z){U c=setup(z);if(!c)continue;
                        F.load(fg);G.load(gg);F.zeta();G.zeta();
                        for(int x=0;x<Q;++x){U*r=Acc.row(x);int p=F.pc[x];
                            for(int t=0;t<=a;++t){U v=dot(F.row(x),G.row(x),t,max(0,t-p),min(t,p));r[t]=subm(r[t],mulm(c,v));}}
                    }
                    G.load(ker);G.zeta();
                    for(int x=0;x<Q;++x)poly_div(Acc.row(x),G.row(x),a,G.pc[x]);
                    Acc.mobius();ans=Acc.diagonal();
                }
                for(int x=0;x<Q;++x)H[root|full[x]]=mulm(ans[x],qp[d[x]]);
            };
            enumerate(0);
        }
    }
    vector<U> recover(){
        // c[S] is the number of potential effective arcs within the selected blocks.
        vector<uint16_t> c(M),allin(M);
        vector<unsigned char> rank(M);
        vector<U> aa(K),bb(K),F(M);
        for(int s=1;s<M;++s){int v=__builtin_ctz((unsigned)s);allin[s]=allin[s&(s-1)]+__builtin_popcount(adj[v]);}
        for(int s=0;s<M;++s){
            rank[s]=__builtin_popcount(support[s]);c[s]=ec(expand[support[s]],s);
            aa[support[s]]=addm(aa[support[s]],mulm(H[s],qp[allin[s]-c[s]]));
        }
        {
            int L=k+1;vector<U> mat(size_t(M)*L);
            auto row=[&](int s){return mat.data()+size_t(s)*L;};
            for(int s=0;s<M;++s)row(s)[rank[s]]=mulm(H[s],po[c[s]]);
            // Zeta on a product of stars: empty state below each nonempty root set.
            for(int i=0;i<k;++i){int low=1<<off[i],stride=1<<(off[i]+sz[i]);
                for(int hi=0;hi<M;hi+=stride)for(int part=low;part<stride;part+=low)
                    for(int lo=0;lo<low;++lo){int t=hi+part+lo,s=hi+lo;U*x=row(t),*y=row(s);
                        for(int r=0;r<=rank[s];++r)x[r]=addm(x[r],y[r]);}}
            for(int s=0;s<M;++s){
                U in[21],der[21]{};copy(row(s),row(s)+L,in);
                for(int r=1;r<=k;++r){U z=0;W sum=0;int cnt=0;
                    for(int j=max(1,r-int(rank[s]));j<r;++j){sum+=W(der[j])*in[r-j];if(++cnt==8){sum%=MOD;cnt=0;}}
                    z=sum%MOD;der[r]=subm(mulm(r,in[r]),z);}
                row(s)[0]=0;for(int r=1;r<=k;++r)row(s)[r]=negm(mulm(der[r],iv[r]));
            }
            for(int i=0;i<k;++i){int low=1<<off[i],stride=1<<(off[i]+sz[i]);
                for(int hi=0;hi<M;hi+=stride)for(int part=low;part<stride;part+=low)
                    for(int lo=0;lo<low;++lo){int t=hi+part+lo,s=hi+lo;U*x=row(t),*y=row(s);
                        for(int r=rank[t];r<=k;++r)x[r]=subm(x[r],y[r]);}}
            for(int s=1;s<M;++s)F[s]=mulm(row(s)[rank[s]],qp[c[s]]);
        }
        bb[0]=1;for(int t=1;t<K;++t){int i=__builtin_ctz((unsigned)t);bb[t]=mulm(bb[t&(t-1)],ptotal[i]);}
        // Release arrays not needed by the two ranked convolution buffers.
        vector<U>().swap(H);vector<U>().swap(P);vector<Distribution>().swap(ch);
        vector<U> reachable=convolution(aa,bb,k);
        for(int s=1;s<M;++s)F[s]=mulm(mulm(F[s],qp[allin[s]-c[s]]),reachable[(K-1)^support[s]]);
        return F;
    }
public:
    Merge(vector<Distribution> children,const vector<Edge>&edges):ch(move(children)){
        k=ch.size();N=0;sz.resize(k);off.resize(k);cm.resize(k);pfull.resize(k);ptotal.resize(k);
        int pos[20];fill(pos,pos+20,-1);
        for(int i=0;i<k;++i){sz[i]=ch[i].vertices.size();off[i]=N;cm[i]=((1u<<sz[i])-1)<<N;
            for(int v:ch[i].vertices){pos[v]=N++;owner.push_back(i);}
            pfull[i]=ch[i].p.back();ptotal[i]=0;for(U p:ch[i].p)ptotal[i]=addm(ptotal[i],p);}
        M=1<<N;K=1<<k;adj.assign(N,0);
        for(auto e:edges){int u=pos[e.u],v=pos[e.v];assert(u>=0&&v>=0&&owner[u]!=owner[v]);adj[u]|=1u<<v;adj[v]|=1u<<u;}
        expand.resize(K);for(int s=1;s<K;++s){int j=__builtin_ctz((unsigned)s);expand[s]=expand[s&(s-1)]|cm[j];}
        support.resize(M);P.resize(M);P[0]=1;
        for(int s=1;s<M;++s){int v=__builtin_ctz((unsigned)s),i=owner[v];support[s]=support[s&(s-1)]|(1u<<i);
            int r=(s&cm[i])>>off[i];P[s]=mulm(P[s&~cm[i]],ch[i].p[r]);}
    }
    Distribution run(){
        Distribution out;for(auto &c:ch)out.vertices.insert(out.vertices.end(),c.vertices.begin(),c.vertices.end());
        compute_h();out.p=recover();return out;
    }
};
U solve(int n,vector<Edge> edges){
    sort(edges.begin(),edges.end(),[](Edge a,Edge b){return a.w<b.w;});
    vector<Node> nd(n);for(int i=0;i<n;++i)nd[i].vertices={i};
    DSU d(n);vector<int> top(n);iota(top.begin(),top.end(),0);
    for(int l=0;l<int(edges.size());){
        int r=l;while(r<int(edges.size())&&edges[r].w==edges[l].w)++r;
        vector<Edge> active;map<int,int> old;
        for(int i=l;i<r;++i){int a=d.find(edges[i].u),b=d.find(edges[i].v);if(a!=b){active.push_back(edges[i]);old[a]=top[a];old[b]=top[b];}}
        for(auto e:active)d.unite(e.u,e.v);
        map<int,vector<int>> groups;for(auto [rep,id]:old)groups[d.find(rep)].push_back(id);
        map<int,int> nid;
        for(auto &[rep,children]:groups){int id=nd.size();Node x;x.children=children;nd.push_back(move(x));top[rep]=id;nid[rep]=id;}
        for(auto e:active)nd[nid[d.find(e.u)]].edges.push_back(e);
        l=r;
    }
    function<Distribution(int)> dfs=[&](int id)->Distribution{
        if(id<n)return {{id},{0,1}};
        vector<Distribution> ch;for(int c:nd[id].children)ch.push_back(dfs(c));
        return Merge(move(ch),nd[id].edges).run();
    };
    auto ans=dfs(top[d.find(0)]);U res=0;for(U p:ans.p)res=addm(res,p);return res;
}
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    po[0]=qp[0]=1;for(int i=1;i<=800;++i){po[i]=addm(po[i-1],po[i-1]);qp[i]=mulm(qp[i-1],500000004);}
    for(int i=1;i<=20;++i)iv[i]=modpow(i,MOD-2);
    int c,T;if(!(cin>>c>>T))return 0;
    while(T--){int n,m;cin>>n>>m;vector<Edge>e(m);for(auto &x:e){cin>>x.u>>x.v>>x.w;--x.u;--x.v;}cout<<solve(n,move(e))<<'\n';}
#ifdef AUDIT_COUNTERS
    cerr<<"grouped_batches="<<grouped_batches<<" fast_kernels="<<fast_kernels<<" small_kernels="<<small_kernels<<'\n';
#endif
    return 0;
}
