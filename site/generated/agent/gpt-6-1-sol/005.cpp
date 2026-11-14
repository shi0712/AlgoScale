#include <bits/stdc++.h>
using namespace std;

class FastInput {
    static constexpr int S = 1 << 20;
    char b[S]; int p=0, e=0;
public:
    inline char get() { if(p==e){e=fread(b,1,S,stdin);p=0;if(!e)return 0;}return b[p++]; }
    void ignoreToken(){char c; do{c=get();}while(c && c<=' ');while(c>' ')c=get();}
    int next(){char c;do{c=get();}while(c && c<=' ');int x=0;while(c>='0'&&c<='9'){x=x*10+c-'0';c=get();}return x;}
};
struct Query { int s,t,r; unsigned char ox,oy; };
struct Segment { int chain,lo,hi; };

int main(){
    FastInput in;
    in.ignoreToken();
    int n=in.next();
    vector<int> deg(n), eu(n-1),ev(n-1);
    for(int i=0;i<n-1;++i){int u=in.next()-1,v=in.next()-1;eu[i]=u;ev[i]=v;++deg[u];++deg[v];}
    vector<int> adjStart(n+1);
    for(int i=0;i<n;++i)adjStart[i+1]=adjStart[i]+deg[i];
    vector<int> adj(2*n-2), cur=adjStart;
    for(int i=0;i<n-1;++i){adj[cur[eu[i]]++]=ev[i];adj[cur[ev[i]]++]=eu[i];}
    vector<int>().swap(eu);vector<int>().swap(ev);vector<int>().swap(cur);vector<int>().swap(deg);
    vector<int> par(n),depth(n),ord(n),height(n),heavy(n);
    auto rootTree = [&](int root){
        par[root]=-1;depth[root]=0;ord[0]=root;int sz=1;
        for(int i=0;i<n;++i){int u=ord[i];for(int j=adjStart[u];j<adjStart[u+1];++j){int v=adj[j];if(v==par[u])continue;par[v]=u;depth[v]=depth[u]+1;ord[sz++]=v;}}
        int far=root;
        for(int u:ord)if(depth[u]>depth[far])far=u;
        return far;
    };
    int a=rootTree(0), b=rootTree(a), diameter=depth[b];
    int center=b;
    for(int i=0;i<diameter/2;++i)center=par[center];
    int other=-1;
    if(diameter&1)other=par[center];
    rootTree(center);
    auto setHeights = [&](){
        for(int i=n-1;i>=0;--i){int u=ord[i],mx=-1,hc=-1;for(int j=adjStart[u];j<adjStart[u+1];++j){int v=adj[j];if(par[v]==u && height[v]>mx){mx=height[v];hc=v;}}height[u]=mx+1;heavy[u]=hc;}
    };
    setHeights();

    // Rank rooted components. Lexicographic child multisets give exactly
    // the same order as the multisets of all proper descendants.
    // With a center as root, every component pointing toward the center is
    // larger than every downward component (the central edge alone may tie).
    // For two centers, the smaller half is chosen as the downward half.
    int total=n+(other!=-1);
    vector<int> cstart(total+1);
    for(int u=0;u<n;++u)cstart[u+1]=adjStart[u+1]-adjStart[u]-(u!=center);
    if(other!=-1)cstart[n+1]=adjStart[center+1]-adjStart[center]-1;
    for(int i=0;i<total;++i)cstart[i+1]+=cstart[i];
    vector<int> children(cstart[total]);
    for(int u=0;u<n;++u){int p=cstart[u];for(int j=adjStart[u];j<adjStart[u+1];++j)if(par[adj[j]]==u)children[p++]=adj[j];}
    if(other!=-1){int p=cstart[n];for(int j=adjStart[center];j<adjStart[center+1];++j)if(adj[j]!=other)children[p++]=adj[j];}
    vector<int> allHeight=height;
    if(other!=-1)allHeight.push_back(height[other]);
    int maxH=*max_element(allHeight.begin(),allHeight.end());
    vector<int> hcount(maxH+2);
    for(int h:allHeight)++hcount[h+1];
    for(int i=1;i<(int)hcount.size();++i)hcount[i]+=hcount[i-1];
    vector<int> byH(total),pos=hcount;
    for(int u=0;u<total;++u)byH[pos[allHeight[u]]++]=u;
    vector<int> type(total),signature(children.size());
    auto lessSignature = [&](int u,int v){
        int p=cstart[u],q=cstart[v],pe=cstart[u+1],qe=cstart[v+1];
        while(p<pe && q<qe){if(signature[p]!=signature[q])return signature[p]<signature[q];++p;++q;}
        return pe-p<qe-q;
    };
    int types=0;
    for(int h=0;h<=maxH;++h){
        int st=hcount[h],en=hcount[h+1];
        for(int i=st;i<en;++i){int u=byH[i];for(int j=cstart[u];j<cstart[u+1];++j)signature[j]=type[children[j]];sort(signature.begin()+cstart[u],signature.begin()+cstart[u+1],greater<int>());}
        sort(byH.begin()+st,byH.begin()+en,lessSignature);
        int previous=-1;
        for(int i=st;i<en;++i){int u=byH[i];if(previous==-1 || lessSignature(previous,u))++types;type[u]=types;previous=u;}
    }
    if(other!=-1 && type[other]>type[n]){
        int old=center;center=other;type[old]=type[n];rootTree(center);setHeights();
    }
    type.resize(n);type[center]=types+1;
    vector<int>().swap(children);vector<int>().swap(cstart);vector<int>().swap(signature);
    vector<int>().swap(allHeight);vector<int>().swap(hcount);vector<int>().swap(byH);vector<int>().swap(pos);
    vector<int> cnt(types+2);
    for(int u=0;u<n;++u)if(u!=center)++cnt[type[u]];
    int greaterCount=0;
    for(int i=types;i>=1;--i){int c=cnt[i];cnt[i]=2+greaterCount;greaterCount+=c;}
    // R[v] = 2 + number of strictly larger downward components.
    // If y is an ancestor of x, f(x,y) = distance(x,y)+1. Otherwise,
    // f(x,y) = R[y] + distance(x, {center} union {v: R[v] < R[y]}).
    vector<int> rank(n);
    vector<long long> rankSum(n);
    for(int u:ord){rank[u]=(u==center?1:cnt[type[u]]);rankSum[u]=(u==center?0:rankSum[par[u]]+rank[u]);}
    vector<int>().swap(cnt);

    int LOG=1;while((1<<LOG)<=n)++LOG;
    vector<vector<int>> up(LOG,vector<int>(n));
    for(int u=0;u<n;++u)up[0][u]=(u==center?center:par[u]);
    for(int k=1;k<LOG;++k)for(int u=0;u<n;++u)up[k][u]=up[k-1][up[k-1][u]];
    auto lift = [&](int u,int d){for(int k=0;d;++k,d>>=1)if(d&1)u=up[k][u];return u;};
    auto lca = [&](int u,int v){
        if(depth[u]<depth[v])swap(u,v);
        u=lift(u,depth[u]-depth[v]);if(u==v)return u;
        for(int k=LOG-1;k>=0;--k)if(up[k][u]!=up[k][v]){u=up[k][u];v=up[k][v];}
        return up[0][u];
    };
    // Deepest ancestor whose downward product is strictly greater.
    auto projection = [&](int u,int R){
        if(rank[u]<R)return u;
        for(int k=LOG-1;k>=0;--k){int v=up[k][u];if(rank[v]>=R)u=v;}
        return up[0][u];
    };
    auto lastRank = [&](int u,int r,int floorNode){
        if(rank[u]<=r)return u;
        for(int k=LOG-1;k>=0;--k){int v=up[k][u];if(depth[v]>depth[floorNode] && rank[v]>r)u=v;}
        int v=up[0][u];return depth[v]<depth[floorNode]?floorNode:v;
    };

    // A maximum-height-child decomposition: every chain contains exactly
    // one vertex of each height from zero to its head's height.
    vector<int> chain(n), chainStart,chainLength, chainNodes(n);
    int used=0;
    for(int u:ord)if(u==center || heavy[par[u]]!=u){
        int id=chainStart.size(),len=height[u]+1;
        chainStart.push_back(used);chainLength.push_back(len);
        int v=u;
        while(v!=-1){chain[v]=id;chainNodes[used+height[v]]=v;v=heavy[v];}
        used+=len;
    }
    vector<int>().swap(adj);vector<int>().swap(adjStart);vector<int>().swap(heavy);

    int m=in.next(), both=0;
    vector<Query> queries(m);
    for(auto &q:queries){q.s=in.next()-1;q.t=in.next()-1;q.ox=in.next();q.oy=in.next();q.r=in.next();both+=(q.ox&&q.oy);}

    // Choose a height cutoff, balancing prefix-table work and direct scans.
    // Sum over chain pairs of their common heights above the cutoff is small.
    vector<int> lengths=chainLength;
    sort(lengths.begin(),lengths.end(),greater<int>());
    vector<long long> pref(lengths.size()+1);
    for(int i=0;i<(int)lengths.size();++i)pref[i+1]=pref[i]+1LL*i*(lengths[i]+1);
    int cutoff=maxH+1;
    long long best=LLONG_MAX;
    if(both){
        int k=lengths.size();
        for(int z=1;z<=maxH+1;++z){
            while(k && lengths[k-1]<=z)--k;
            long long pairs=1LL*k*(k-1)/2;
            long long cells=pref[k]-1LL*z*pairs;
            if(pairs>4000000 || cells+pairs>115000000)continue;
            long long cost=cells+2LL*both*z;
            if(cost<best){best=cost;cutoff=z;}
        }
    }
    vector<int>().swap(lengths);vector<long long>().swap(pref);
    vector<int> longId(chainStart.size(),-1), longChains;
    for(int i=0;i<(int)chainStart.size();++i)if(chainLength[i]>cutoff){longId[i]=longChains.size();longChains.push_back(i);}
    int K=longChains.size();
    vector<size_t> row(K);
    for(int i=0;i<K;++i)row[i]=1ULL*i*(2*K-i-1)/2;
    const uint32_t MISSING=UINT32_MAX, WIDE=1u<<31;
    vector<uint32_t> cache(1ULL*K*(K-1)/2,MISSING);
    vector<uint32_t> data32;
    vector<uint64_t> data64;
    size_t cells32=0,cells64=0;
    for(int i=0;i<K;++i)for(int j=i+1;j<K;++j){
        int len=min(chainLength[longChains[i]],chainLength[longChains[j]])-cutoff;
        if(len>65535)cells64+=len+1;else cells32+=len+1;
    }
    data32.reserve(cells32);data64.reserve(cells64);
    // Tables are built only when a query actually uses the chain pair.
    auto equalHeightCount = [&](int ca,int cb,int low,int high)->long long{
        if(low>high || ca==cb)return 0;
        long long answer=0;
        int enda=min(high,cutoff-1);
        int sa=chainStart[ca],sb=chainStart[cb];
        for(int h=max(low,1);h<=enda;++h)answer+=type[chainNodes[sa+h]]>type[chainNodes[sb+h]];
        low=max(low,cutoff);
        if(low>high)return answer;
        int ia=longId[ca],ib=longId[cb];
        bool reverse=ia>ib;
        if(reverse)swap(ia,ib);
        size_t ix=row[ia]+ib-ia-1;
        uint32_t off=cache[ix];
        int L=min(chainLength[ca],chainLength[cb])-cutoff;
        bool wide=L>65535;
        if(off==MISSING){
            int aa=chainStart[longChains[ia]],bb=chainStart[longChains[ib]];
            if(wide){
                off=(uint32_t)data64.size()|WIDE;cache[ix]=off;
                data64.push_back(0);uint64_t g=0,l=0;
                for(int h=cutoff;h<cutoff+L;++h){int x=type[chainNodes[aa+h]],y=type[chainNodes[bb+h]];g+=x>y;l+=x<y;data64.push_back(g|(l<<32));}
            }else{
                off=data32.size();cache[ix]=off;
                data32.push_back(0);uint32_t g=0,l=0;
                for(int h=cutoff;h<cutoff+L;++h){int x=type[chainNodes[aa+h]],y=type[chainNodes[bb+h]];g+=x>y;l+=x<y;data32.push_back(g|(l<<16));}
            }
        }
        int p=low-cutoff,q=high-cutoff+1;
        if(off&WIDE){
            size_t o=off&~WIDE;
            uint64_t v=data64[o+q]-data64[o+p];
            answer+=reverse?(v>>32):(v&0xffffffffULL);
        }else{
            uint32_t v=data32[off+q]-data32[off+p];
            answer+=reverse?(v>>16):(v&65535u);
        }
        return answer;
    };
    // Count product(a)>product(b) between two ancestor-path segments.
    // Endpoints 'stop' are excluded. Heights within a chain are consecutive.
    auto inversionCount = [&](int ua,int stopa,int ub,int stopb)->long long{
        if(ua==stopa || ub==stopb)return 0;
        Segment as[1024],bs[1024];int na=0,nb=0;
        while(depth[ua]>depth[stopa]){
            int c=chain[ua],head=chainNodes[chainStart[c]+chainLength[c]-1];
            int top=depth[head]>depth[stopa]?head:lift(ua,depth[ua]-depth[stopa]-1);
            as[na++]={c,height[ua],height[top]};ua=up[0][top];
        }
        while(depth[ub]>depth[stopb]){
            int c=chain[ub],head=chainNodes[chainStart[c]+chainLength[c]-1];
            int top=depth[head]>depth[stopb]?head:lift(ub,depth[ub]-depth[stopb]-1);
            bs[nb++]={c,height[ub],height[top]};ub=up[0][top];
        }
        int i=0,j=0;long long countB=0,answer=0;
        while(i<na && j<nb){
            auto &a=as[i];auto &b=bs[j];
            if(a.hi<b.lo){answer+=1LL*(a.hi-a.lo+1)*countB;++i;continue;}
            if(b.hi<a.lo){countB+=b.hi-b.lo+1;++j;continue;}
            if(a.lo<b.lo){answer+=1LL*(b.lo-a.lo)*countB;a.lo=b.lo;}
            if(b.lo<a.lo){countB+=a.lo-b.lo;b.lo=a.lo;}
            int h=min(a.hi,b.hi),len=h-a.lo+1;
            answer+=1LL*len*countB+1LL*len*(len-1)/2+equalHeightCount(a.chain,b.chain,a.lo,h);
            countB+=len;a.lo=h+1;b.lo=h+1;
            if(a.lo>a.hi)++i;
            if(b.lo>b.hi)++j;
        }
        while(i<na){answer+=1LL*(as[i].hi-as[i].lo+1)*countB;++i;}
        return answer;
    };
    auto fixedRank = [&](int x,int y,int l)->int{
        if(l==y)return depth[x]-depth[y]+1;
        int v=projection(x,rank[y]);
        return rank[y]+depth[x]-depth[v];
    };
    auto pathVertex = [&](int s,int t,int l,int at){
        int a=depth[s]-depth[l],b=depth[t]-depth[l];
        return at<=a?lift(s,at):lift(t,a+b-at);
    };
    auto sumMin = [&](int len,int r)->long long{
        int k=min(len,r-1);
        return 1LL*k*(k+3)/2+1LL*(len-k)*r;
    };
    string output;output.reserve(m*14);
    for(const auto&q:queries){
        int s=q.s,t=q.t,r=q.r,l=lca(s,t),alen=depth[s]-depth[l],blen=depth[t]-depth[l];
        long long answer=0;
        if(r==n){
            long long len=alen+blen+1;answer=(q.ox?len:1)*(q.oy?len:1);
        }else if(!q.ox && !q.oy){answer=fixedRank(s,t,l)<=r;}
        else if(!q.ox){
            int low=0,high=alen+blen;
            while(low<high){int mid=(low+high+1)/2,v=pathVertex(s,t,l,mid);int lc=mid<=alen?v:l;
                if(fixedRank(s,v,lc)<=r)low=mid;else high=mid-1;
            }
            answer=low+1;
        }else if(!q.oy){
            int low=0,high=alen+blen;
            while(low<high){int mid=(low+high+1)/2,v=pathVertex(t,s,l,mid);int lc=mid<=blen?v:l;
                if(fixedRank(v,t,lc)<=r)low=mid;else high=mid-1;
            }
            answer=low+1;
        }else{
            int aa=lastRank(s,r,l),bb=lastRank(t,r,l);
            int ka=depth[aa]-depth[l],kb=depth[bb]-depth[l];
            answer=1+sumMin(alen,r)+sumMin(blen,r)+1LL*ka*(ka+1)/2+1LL*kb*(kb+1)/2;
            // Along opposite arms, let h(y) count root-arm vertices with
            // a greater downward product. The number of admissible roots is
            // min(xlen, h(y)+r-R[y]), provided R[y]<=r. It decreases along
            // the target arm. Fully covered targets form a prefix; summing
            // the remaining h(y)'s is one comparison count between segments.
            auto cross = [&](int x,int y0,int xlen,int ylen)->long long{
                if(!xlen || !ylen)return 0;
                auto qualifies = [&](int y){return rank[y]+depth[x]-depth[projection(x,rank[y])]<=r;};
                if(qualifies(y0))return 1LL*xlen*ylen;
                int u=y0;
                for(int k=LOG-1;k>=0;--k){int v=up[k][u];if(depth[v]>depth[l] && !qualifies(v))u=v;}
                int good=up[0][u];
                int full=depth[good]-depth[l];
                long long ans=1LL*xlen*full+1LL*(ylen-full)*r-(rankSum[y0]-rankSum[good]);
                ans+=inversionCount(x,l,y0,good);
                return ans;
            };
            answer+=cross(s,bb,alen,kb)+cross(t,aa,blen,ka);
        }
        output+=to_string(answer);output+='\n';
    }
    fwrite(output.data(),1,output.size(),stdout);
    return 0;
}
