#include <bits/stdc++.h>
using namespace std;
using u32 = uint32_t;
using u64 = uint64_t;
const u32 MOD=1000000007;
u32 addm(u32 a,u32 b){u32 c=a+b;return c>=MOD?c-MOD:c;}
u32 subm(u32 a,u32 b){return a>=b?a-b:a+MOD-b;}
u32 mulm(u32 a,u32 b){return (u64)a*b%MOD;}
u32 power(u32 a,int b){u32 r=1;for(;b;b>>=1,a=mulm(a,a))if(b&1)r=mulm(r,a);return r;}
u32 invSmall[21];

// Ordinary ranked subset zeta transform, with rank-major storage.
void zeta(vector<u32>& a,int bits,bool inverse=false){
    int N=1<<bits;
    for(int r=0;r<=bits;r++){
        u32* f=a.data()+(size_t)r*N;
        for(int b=1;b<N;b<<=1)
            for(int base=0;base<N;base+=b*2)
                for(int j=0;j<b;j++){
                    u32 &x=f[base+b+j],y=f[base+j];
                    x=inverse?subm(x,y):addm(x,y);
                }
    }
}
void zeta_ptr(u32* a,int bits,bool inverse=false){
    int N=1<<bits;
    for(int r=0;r<=bits;r++){
        u32* f=a+(size_t)r*N;
        for(int b=1;b<N;b<<=1)
            for(int base=0;base<N;base+=b*2)
                for(int j=0;j<b;j++){
                    u32 &x=f[base+b+j],y=f[base+j];
                    x=inverse?subm(x,y):addm(x,y);
                }
    }
}
// Dot products have at most 21 terms. Reduce after 16 to avoid overflow.
u32 dot(const u32* a,const u32* b,int len){
    u64 v=0;int i=0;
    for(;i<min(len,16);i++)v+=(u64)a[i]*b[i];
    if(len<=16)return v%MOD;
    u32 r=v%MOD;v=0;
    for(;i<len;i++)v+=(u64)a[i]*b[i];
    return addm(r,v%MOD);
}

/*
Kruskal's equal-weight merges form a hierarchy. In an MST, each child
component is itself a tree and the children are joined by a tree. Therefore
an optimal outward arborescence must enter a child at one of its feasible
roots. For each component we store the probability of every nonempty set
of feasible roots; the remaining probability represents failure.

After fixing the children's root sets, retain an auxiliary arc i -> j
when some current-weight arc goes from child i into a feasible root of j.
The parent succeeds exactly when this graph has a unique source SCC. Its
root set is the union of the root sets in that source SCC.

Source-SCC inclusion-exclusion gives a = exp(-s), where s counts one
source SCC, with the appropriate probabilities of absent incoming arcs
factored out. Hence s = -log(a) in the algebra of disjoint child sets.
Ranked subset transforms evaluate this logarithm efficiently.

The asymmetric part of the inclusion-exclusion recurrence involves only
children with at least two vertices. Enumerating their active sets, and
using ranked subset convolution on singleton children, takes
O(n^2 * 2^l * product(2^size + 1)) time for a component, where the product
is over its nonsingleton children and l counts its singleton children.
This is at most O(n^2 * 5^(n/2)); memory is O(n * 2^n).
*/

struct Edge{int u,v,w;};
struct Node{vector<int> verts,child;vector<pair<int,int>> edges;vector<u32> p;};
vector<Node> nodes;

// Each coordinate is either absent or one nonempty root subset of a child.
// Products using the same child twice are zero. Ranked zeta + formal log
// computes the logarithm in this algebra.
vector<u32> negativeLog(const vector<u32>& a,const vector<int>& sizes,
                        const vector<unsigned char>& rank){
    int n=0;for(int s:sizes)n+=s;
    int N=1<<n,k=sizes.size();
    vector<u32> f((size_t)(k+1)*N);
    for(int x=0;x<N;x++)f[(size_t)rank[x]*N+x]=a[x];
    auto transform=[&](bool inverse){
        int off=0;
        for(int s:sizes){
            int low=1<<off,block=1<<(off+s),choices=1<<s;
            for(int r=0;r<=k;r++){
                u32* v=f.data()+(size_t)r*N;
                for(int base=0;base<N;base+=block)
                    for(int c=1;c<choices;c++){
                        u32* dst=v+base+c*low;u32* src=v+base;
                        for(int j=0;j<low;j++)dst[j]=inverse?subm(dst[j],src[j]):addm(dst[j],src[j]);
                    }
            }
            off+=s;
        }
    };
    transform(false);
    u32 orig[21],der[21],aa[21],bb[21];
    for(int x=0;x<N;x++){
        int degree=rank[x];
        for(int r=0;r<=k;r++)orig[r]=f[(size_t)r*N+x];
        f[x]=0;der[0]=0;
        for(int r=1;r<=k;r++){
            int len=min(r-1,degree);
            for(int j=1;j<=len;j++){aa[j-1]=orig[j];bb[j-1]=der[r-j];}
            u32 v=subm(mulm(r,orig[r]),dot(aa,bb,len));
            der[r]=v;f[(size_t)r*N+x]=mulm(v,invSmall[r]);
        }
    }
    transform(true);
    vector<u32> out(N);
    for(int x=1;x<N;x++){
        u32 v=f[(size_t)rank[x]*N+x];out[x]=v?MOD-v:0;
    }
    return out;
}

void solveNode(int id){
    if(nodes[id].child.empty()){nodes[id].p={0,1};return;}
    vector<int> children=nodes[id].child;
    for(int c:children)solveNode(c);
    // Put one-vertex children first; the remaining children are the exceptions
    // to symmetry in the auxiliary directed graph.
    stable_sort(children.begin(),children.end(),[](int a,int b){return nodes[a].verts.size()<nodes[b].verts.size();});
    int l=0;while(l<(int)children.size()&&nodes[children[l]].verts.size()==1)l++;
    int k=children.size(),h=k-l,n=nodes[id].verts.size(),q=n-l;
    int M=1<<l,Q=1<<q,N=1<<n,H=1<<h;
    vector<int> sizes,verts,offs;
    int pos=0;
    for(int c:children){offs.push_back(pos);sizes.push_back(nodes[c].verts.size());for(int v:nodes[c].verts)verts.push_back(v);pos+=nodes[c].verts.size();}
    nodes[id].verts=verts;
    int loc[20];for(int i=0;i<n;i++)loc[verts[i]]=i;
    vector<u32> adj(n);
    for(auto [u,v]:nodes[id].edges){int a=loc[u],b=loc[v];adj[a]|=1u<<b;adj[b]|=1u<<a;}
    vector<u32> pw(2*n*n+1),ipw(2*n*n+1);pw[0]=ipw[0]=1;
    for(int i=1;i<(int)pw.size();i++){pw[i]=addm(pw[i-1],pw[i-1]);ipw[i]=mulm(ipw[i-1],500000004);}
    auto signedpow=[&](int e){return e>=0?pw[e]:ipw[-e];};
    vector<unsigned char> pc(M),singleE(M);
    for(int s=1;s<M;s++){int b=__builtin_ctz((unsigned)s),t=s&(s-1);pc[s]=pc[t]+1;singleE[s]=singleE[t]+__builtin_popcount(adj[b]&t);}
    vector<int> spOff(h),spSize(h),which(q);
    for(int i=0;i<h;i++){
        spOff[i]=offs[l+i]-l;spSize[i]=sizes[l+i];
        for(int j=0;j<spSize[i];j++)which[spOff[i]+j]=i;
    }
    vector<u32> closure(H);
    for(int a=1;a<H;a++){int i=__builtin_ctz((unsigned)a);closure[a]=closure[a&(a-1)]|(((1u<<spSize[i])-1)<<(spOff[i]+l));}
    vector<int> active(Q);
    vector<unsigned short> internal(Q);
    vector<u32> tprob(Q,1);
    for(int b=1;b<Q;b++){
        int v=__builtin_ctz((unsigned)b);active[b]=active[b&(b-1)]|(1<<which[v]);
        int val=0;u32 z=(u32)b<<l,cl=closure[active[b]];
        while(z){int vtx=__builtin_ctz(z);z&=z-1;val+=__builtin_popcount(adj[vtx]&cl);}
        internal[b]=val;
        u32 prob=1;
        for(int i=0;i<h;i++){int r=(b>>spOff[i])&((1<<spSize[i])-1);if(r)prob=mulm(prob,nodes[children[l+i]].p[r]);}
        tprob[b]=prob;
    }
    // Number of arcs from all vertices of special components C to singletons S.
    vector<unsigned short> cross((size_t)H*M);
    for(int c=0;c<H;c++)for(int s=1;s<M;s++){
        int b=__builtin_ctz((unsigned)s);
        cross[(size_t)c*M+s]=cross[(size_t)c*M+(s&(s-1))]+__builtin_popcount(adj[b]&closure[c]);
    }
    vector<u32> a(N);
    {
        size_t R=(size_t)(l+1)*M;
        // D_B is the ranked transform of 2^{e(S)} times the probability
        // that no arc goes from singleton S into the chosen roots B.
        vector<u32> denom((size_t)Q*R),d(N);
        vector<unsigned short> ey(M);
        for(int b=0;b<Q;b++){
            u32* dst=denom.data()+(size_t)b*R;ey[0]=0;
            for(int s=1;s<M;s++){
                int v=__builtin_ctz((unsigned)s);
                ey[s]=ey[s&(s-1)]+__builtin_popcount(adj[v]&((u32)b<<l));
            }
            for(int s=0;s<M;s++)dst[(size_t)pc[s]*M+s]=signedpow((int)singleE[s]-ey[s]);
            zeta_ptr(dst,l);
        }
        vector<u32> scratch(h ? R : 0);
        u32 aa[21],bb[21],rev[21],res[21];
        for(int jset=0;jset<H;jset++){
            vector<int> incl,bitPositions,localOff;
            int qb=0;
            for(int i=0;i<h;i++)if(jset>>i&1){
                incl.push_back(i);localOff.push_back(qb);
                for(int j=0;j<spSize[i];j++)bitPositions.push_back(spOff[i]+j);
                qb+=spSize[i];
            }
            int Bcount=1<<qb;
            vector<int> mapping(Bcount);
            vector<unsigned short> degsum(Bcount);
            int deg[20]={};
            for(int v=0;v<q;v++)deg[v]=__builtin_popcount(adj[l+v]&closure[jset]);
            for(int b=1;b<Bcount;b++){
                int v=__builtin_ctz((unsigned)b),prev=b&(b-1);
                mapping[b]=mapping[prev]|(1<<bitPositions[v]);
                degsum[b]=degsum[prev]+deg[bitPositions[v]];
            }
            vector<u32> f((size_t)Bcount*R);
            if(jset==0){for(int s=0;s<M;s++)f[s]=1;}
            else {
                for(int b=0;b<Bcount;b++){
                    int gb=mapping[b],act=active[gb];
                    if(act==jset)continue;
                    int c=jset^act;
                    fill(scratch.begin(),scratch.end(),0);
                    for(int s=0;s<M;s++)scratch[(size_t)pc[s]*M+s]=mulm(d[(size_t)gb*M+s],ipw[cross[(size_t)c*M+s]]);
                    zeta(scratch,l);
                    const u32* den=denom.data()+(size_t)gb*R;
                    u32* dst=f.data()+(size_t)b*R;
                    u32 factor=ipw[degsum[b]-internal[gb]];
                    for(int s=0;s<M;s++){
                        int p=pc[s];
                        for(int r=0;r<=p;r++){aa[r]=scratch[(size_t)r*M+s];bb[r]=den[(size_t)r*M+s];}
                        for(int r=0;r<=min(l,2*p);r++){
                            int lo=max(0,r-p),hi=min(p,r),len=hi-lo+1;
                            for(int v=0;v<len;v++)rev[v]=bb[r-lo-v];
                            dst[(size_t)r*M+s]=mulm(dot(aa+lo,rev,len),factor);
                        }
                    }
                }
                // Sum over dropping whole exceptional children. Their root
                // choices are alternatives, rather than subsets of one another.
                for(int it=0;it<(int)incl.size();it++){
                    int low=1<<localOff[it],choices=1<<spSize[incl[it]],block=low*choices;
                    for(int base=0;base<Bcount;base+=block)
                        for(int choice=1;choice<choices;choice++)
                            for(int lowmask=0;lowmask<low;lowmask++){
                                u32* dst=f.data()+((size_t)base+choice*low+lowmask)*R;
                                const u32* src=f.data()+((size_t)base+lowmask)*R;
                                for(size_t z=0;z<R;z++)dst[z]=addm(dst[z],src[z]);
                            }
                }
            }
            for(int b=0;b<Bcount;b++){
                int gb=mapping[b];if(active[gb]!=jset)continue;
                u32* num=f.data()+(size_t)b*R;
                const u32* den=denom.data()+(size_t)gb*R;
                for(int s=0;s<M;s++){
                    int p=pc[s];
                    for(int r=0;r<=p;r++)aa[r]=den[(size_t)r*M+s];
                    for(int r=0;r<=l;r++){
                        int len=min(r,p);
                        for(int v=0;v<len;v++)rev[v]=res[r-1-v];
                        u32 val=num[(size_t)r*M+s];
                        if(jset&&val)val=MOD-val;
                        res[r]=subm(val,dot(aa+1,rev,len));
                        num[(size_t)r*M+s]=res[r];
                    }
                }
                zeta_ptr(num,l,true);
                for(int s=0;s<M;s++)d[(size_t)gb*M+s]=num[(size_t)pc[s]*M+s];
            }
        }
        for(int b=0;b<Q;b++){
            ey[0]=0;
            for(int s=1;s<M;s++){
                int v=__builtin_ctz((unsigned)s);
                ey[s]=ey[s&(s-1)]+__builtin_popcount(adj[v]&((u32)b<<l));
            }
            for(int s=0;s<M;s++){
                int exponent=singleE[s]+ey[s]+internal[b]+cross[(size_t)active[b]*M+s];
                a[(size_t)b*M+s]=mulm(mulm(d[(size_t)b*M+s],tprob[b]),pw[exponent]);
            }
        }
    }
    vector<int> childMask(N);
    vector<unsigned char> rank(N);
    vector<unsigned short> rootDegree(N);
    vector<u32> coeff(1<<k),success(k);
    for(int i=0;i<k;i++)for(u32 p:nodes[children[i]].p)success[i]=addm(success[i],p);
    int degree[20];for(int i=0;i<n;i++)degree[i]=__builtin_popcount(adj[i]);
    for(int x=0;x<N;x++){
        int s=x&(M-1),b=x>>l;
        childMask[x]=s|(active[b]<<l);rank[x]=__builtin_popcount((unsigned)childMask[x]);
        if(x){int bit=__builtin_ctz((unsigned)x);rootDegree[x]=rootDegree[x&(x-1)]+degree[bit];}
        coeff[childMask[x]]=addm(coeff[childMask[x]],mulm(a[x],ipw[rootDegree[x]]));
    }
    for(int bit=0;bit<k;bit++)for(int mask=0;mask<(1<<k);mask++)if(mask>>bit&1)
        coeff[mask]=addm(coeff[mask],mulm(success[bit],coeff[mask^(1<<bit)]));
    vector<u32> strong=negativeLog(a,sizes,rank);
    vector<u32> prob(N);int full=(1<<k)-1;
    for(int x=1;x<N;x++)prob[x]=mulm(mulm(strong[x],ipw[rootDegree[x]]),coeff[full^childMask[x]]);
    nodes[id].p=move(prob);
    // Only a component's root distribution is needed by its parent.
    for(int c:children)vector<u32>().swap(nodes[c].p);
}

struct DSU{vector<int> p;DSU(int n):p(n){iota(p.begin(),p.end(),0);}int find(int x){return p[x]==x?x:p[x]=find(p[x]);}void unite(int a,int b){a=find(a);b=find(b);if(a!=b)p[a]=b;}};
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    invSmall[1]=1;for(int i=2;i<=20;i++)invSmall[i]=MOD-(u64)(MOD/i)*invSmall[MOD%i]%MOD;
    int c,T;if(!(cin>>c>>T))return 0;
    while(T--){
        int n,m;cin>>n>>m;vector<Edge> e(m);
        for(auto& x:e){cin>>x.u>>x.v>>x.w;--x.u;--x.v;}
        sort(e.begin(),e.end(),[](auto a,auto b){return a.w<b.w;});
        nodes.clear();nodes.resize(n);for(int i=0;i<n;i++)nodes[i].verts={i};
        DSU ds(n);vector<int> current(n);iota(current.begin(),current.end(),0);
        for(int at=0;at<m;){
            int to=at;while(to<m&&e[to].w==e[at].w)to++;
            vector<tuple<int,int,int,int>> useful;
            for(int i=at;i<to;i++){
                int a=ds.find(e[i].u),b=ds.find(e[i].v);
                if(a!=b)useful.emplace_back(a,b,e[i].u,e[i].v);
            }
            for(auto [a,b,u,v]:useful)ds.unite(a,b);
            map<int,vector<int>> parts;map<int,vector<pair<int,int>>> edges;
            for(auto [a,b,u,v]:useful){int r=ds.find(a);parts[r].push_back(current[a]);parts[r].push_back(current[b]);edges[r].push_back({u,v});}
            for(auto& [r,ch]:parts){
                sort(ch.begin(),ch.end());ch.erase(unique(ch.begin(),ch.end()),ch.end());
                Node node;node.child=ch;node.edges=move(edges[r]);
                for(int x:ch)for(int v:nodes[x].verts)node.verts.push_back(v);
                current[r]=nodes.size();nodes.push_back(move(node));
            }
            at=to;
        }
        int root=current[ds.find(0)];solveNode(root);u32 ans=0;for(u32 p:nodes[root].p)ans=addm(ans,p);cout<<ans<<'\n';
    }
}
