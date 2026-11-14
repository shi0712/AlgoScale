#include <bits/stdc++.h>
using namespace std;

// After replacing b by b XOR k, the highest differing bit determines
// whether the two trie branches are completely joined or disconnected.
// For a graph on A,B vertices, its complement's rook numbers satisfy
//   r'[t] = sum_j (-1)^j r[j] (A-j)! (B-j)! / (t-j)!
//                         / ((A-t)! (B-t)!).
// Thus both disjoint union and complementation use polynomial convolution.
// The trie has at most 60 levels: O(60*n*log n) time and O(n) memory.
static constexpr uint32_t MOD = 998244353;
struct Mint {
    uint32_t x;
    static constexpr uint32_t INV = 998244351; // -MOD^{-1} modulo 2^32
    static constexpr uint32_t R2 = 932051910; // 2^64 modulo MOD
    static uint32_t reduce(uint64_t z) {
        uint32_t q = uint32_t(z) * INV;
        uint32_t v = uint32_t((z + uint64_t(q) * MOD) >> 32);
        return v >= MOD ? v - MOD : v;
    }
    Mint(): x(0) {}
    Mint(uint32_t v): x(reduce(uint64_t(v) * R2)) {}
    static Mint raw(uint32_t v) { Mint a; a.x=v; return a; }
    uint32_t val() const { return reduce(x); }
    Mint& operator+=(Mint b) { x += b.x; if(x>=MOD) x-=MOD; return *this; }
    Mint& operator-=(Mint b) { x = x>=b.x ? x-b.x : x+MOD-b.x; return *this; }
    Mint& operator*=(Mint b) { x=reduce(uint64_t(x)*b.x); return *this; }
    friend Mint operator+(Mint a,Mint b) { return a+=b; }
    friend Mint operator-(Mint a,Mint b) { return a-=b; }
    friend Mint operator*(Mint a,Mint b) { return a*=b; }
    Mint pow(uint32_t e) const { Mint r(1),a=*this; while(e){if(e&1)r*=a;a*=a;e>>=1;}return r; }
};
using Poly=vector<Mint>;
vector<Mint> roots{Mint(0),Mint(1)};
vector<vector<int>> reversals(20);
void ntt(Poly &a) {
    int n=a.size(), lg=__builtin_ctz((unsigned)n);
    if((int)roots.size()<n){
        int k=__builtin_ctz((unsigned)roots.size());
        roots.resize(n);
        while((1<<k)<n){
            Mint z=Mint(3).pow((MOD-1)>>(k+1));
            for(int i=1<<(k-1);i<(1<<k);++i){roots[2*i]=roots[i];roots[2*i+1]=roots[i]*z;}
            ++k;
        }
    }
    auto &rev=reversals[lg];
    if(rev.empty()){
        rev.resize(n);
        for(int i=1;i<n;++i) rev[i]=(rev[i>>1]>>1)|((i&1)<<(lg-1));
    }
    for(int i=0;i<n;++i) if(i<rev[i]) swap(a[i],a[rev[i]]);
    for(int len=1;len<n;len<<=1)
        for(int i=0;i<n;i+=2*len)
            for(int j=0;j<len;++j){
                Mint z=a[i+j+len]*roots[len+j],u=a[i+j];
                a[i+j]=u+z; a[i+j+len]=u-z;
            }
}
Poly multiply(Poly a,Poly b,int limit=INT_MAX){
    if(a.empty()||b.empty())return {};
    int full=a.size()+b.size()-1, need=min(full,limit);
    if(a.size()<b.size())swap(a,b);
    if(b.size()==1){for(auto &v:a)v*=b[0];a.resize(min((int)a.size(),need));return a;}
    if(b.size()<=8 || (long long)a.size()*b.size()<=4096){
        Poly c(need);
        for(int j=0;j<(int)b.size();++j)
            for(int i=0;i<(int)a.size() && i+j<need;++i)c[i+j]+=a[i]*b[j];
        return c;
    }
    // Overlap-add avoids a large transform when one operand is short.
    if(a.size()>=8*b.size()){
        int sz=1;while(sz<2*(int)b.size()-1)sz<<=1;
        int block=sz-(int)b.size()+1;
        b.resize(sz);ntt(b);
        Mint iz=Mint(sz).pow(MOD-2);
        for(auto &v:b)v*=iz;
        Poly c(need),buf(sz);
        for(int start=0;start<(int)a.size() && start<need;start+=block){
            fill(buf.begin(),buf.end(),Mint(0));
            int take=min(block,(int)a.size()-start);
            copy_n(a.begin()+start,take,buf.begin());
            ntt(buf);
            for(int i=0;i<sz;++i)buf[i]*=b[i];
            reverse(buf.begin()+1,buf.end());ntt(buf);
            int out=min({sz,need-start,take+sz-block});
            for(int i=0;i<out;++i)c[start+i]+=buf[i];
        }
        return c;
    }
    int sz=1;while(sz<full)sz<<=1;
    a.resize(sz);b.resize(sz);
    ntt(a);ntt(b);
    for(int i=0;i<sz;++i)a[i]*=b[i];
    reverse(a.begin()+1,a.end());ntt(a);
    a.resize(need);Mint iz=Mint(sz).pow(MOD-2);
    for(auto &v:a)v*=iz;
    return a;
}
vector<Mint> fac,ifac,invs;
Poly completePoly(int a,int b){
    int m=min(a,b);Poly p(m+1);p[0]=Mint(1);
    for(int i=1;i<=m;++i)p[i]=p[i-1]*Mint(a-i+1)*Mint(b-i+1)*invs[i];
    return p;
}
void trim(Poly &p){while(p.size()>1 && p.back().x==0)p.pop_back();}
Poly complement(Poly p,int a,int b){
    int m=min(a,b);
    if(p.size()==1)return completePoly(a,b);
    for(int i=0;i<(int)p.size();++i){p[i]*=fac[a-i]*fac[b-i];if(i&1)p[i]=Mint(0)-p[i];}
    Poly q(ifac.begin(),ifac.begin()+m+1);
    p=multiply(move(p),move(q),m+1);
    for(int i=0;i<=m;++i)p[i]*=ifac[a-i]*ifac[b-i];
    trim(p);return p;
}
// Add c vertices on the left, adjacent to every existing right vertex.
Poly addLeft(Poly p,int a,int b,int c){
    if(c==0 || b==0)return p;
    for(int i=0;i<(int)p.size();++i)p[i]*=fac[b-i];
    int s=min(c,b);Poly q(s+1);
    for(int i=0;i<=s;++i)q[i]=fac[c]*ifac[i]*ifac[c-i];
    int m=min(a+c,b);
    p=multiply(move(p),move(q),m+1);
    for(int i=0;i<(int)p.size();++i)p[i]*=ifac[b-i];
    trim(p);return p;
}

struct Node {int a,b,l=-1,r=-1;bool join=false;int kind=-1;}; // kind: 0 empty, 1 complete, -1 general
vector<Node> tr;
vector<uint64_t> values;
vector<int> pa,pb;
uint64_t threshold;
int build(int lo,int hi){
    int a=pa[hi]-pa[lo],b=pb[hi]-pb[lo];
    int id=tr.size();tr.push_back({a,b});
    if(a==0||b==0){tr[id].kind=0;return id;}
    if(hi-lo==1){tr[id].kind=1;return id;}
    int bit=63-__builtin_clzll(values[lo]^values[hi-1]);
    uint64_t split=(values[lo]>>(bit+1)<<(bit+1))|(1ULL<<bit);
    int mid=lower_bound(values.begin()+lo,values.begin()+hi,split)-values.begin();
    int l=build(lo,mid),r=build(mid,hi);
    bool join=((threshold>>bit)&1)==0;
    tr[id].l=l;tr[id].r=r;tr[id].join=join;
    if(join && tr[l].kind==1 && tr[r].kind==1)tr[id].kind=1;
    if(!join && tr[l].kind==0 && tr[r].kind==0)tr[id].kind=0;
    // A single-side graph is both empty and complete.
    bool lc=tr[l].kind==1 || min(tr[l].a,tr[l].b)==0;
    bool rc=tr[r].kind==1 || min(tr[r].a,tr[r].b)==0;
    if(join && lc && rc)tr[id].kind=1;
    return id;
}
Poly solve(int id,bool flip);
void gather(int id,bool flip,vector<Poly>& polys){
    const auto &v=tr[id];
    if(v.kind!=-1){
        if(min(v.a,v.b)>0 && (v.kind^flip))polys.push_back(completePoly(v.a,v.b));
    }else if(v.join==flip){
        gather(v.l,flip,polys);gather(v.r,flip,polys);
    }else{
        Poly p=solve(id,flip);
        if(p.size()>1)polys.push_back(move(p));
    }
}
Poly product(vector<Poly> polys){
    if(polys.empty())return {Mint(1)};
    priority_queue<pair<int,int>,vector<pair<int,int>>,greater<pair<int,int>>> pq;
    for(int i=0;i<(int)polys.size();++i)pq.emplace(polys[i].size(),i);
    while(pq.size()>1){
        int a=pq.top().second;pq.pop();int b=pq.top().second;pq.pop();
        polys[a]=multiply(move(polys[a]),move(polys[b]));
        pq.emplace(polys[a].size(),a);
    }
    return move(polys[pq.top().second]);
}
Poly solve(int id,bool flip){
    const auto &v=tr[id];
    if(min(v.a,v.b)==0)return {Mint(1)};
    if(v.kind!=-1)return (v.kind^flip)?completePoly(v.a,v.b):Poly{Mint(1)};
    if(v.join==flip){
        vector<Poly> polys;gather(v.l,flip,polys);gather(v.r,flip,polys);
        return product(move(polys));
    }
    int l=v.l,r=v.r;
    auto isComplete=[&](int x){return min(tr[x].a,tr[x].b)==0 || (tr[x].kind!=-1 && (tr[x].kind^flip));};
    if(isComplete(l)||isComplete(r)){
        if(!isComplete(r))swap(l,r);
        Poly p=solve(l,flip);
        p=addLeft(move(p),tr[l].a,tr[l].b,tr[r].a);
        p=addLeft(move(p),tr[l].b,tr[l].a+tr[r].a,tr[r].b);
        return p;
    }
    vector<Poly> polys;gather(l,!flip,polys);gather(r,!flip,polys);
    return complement(product(move(polys)),v.a,v.b);
}
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int n;cin>>n>>threshold;
    vector<pair<uint64_t,int>> data;data.reserve(2*n);
    for(int i=0;i<n;++i){uint64_t v;cin>>v;data.emplace_back(v,0);}
    for(int i=0;i<n;++i){uint64_t v;cin>>v;data.emplace_back(v^threshold,1);}
    sort(data.begin(),data.end());pa.push_back(0);pb.push_back(0);
    for(int i=0;i<2*n;){int j=i,ca=0,cb=0;uint64_t v=data[i].first;
        while(j<2*n&&data[j].first==v){(data[j].second?cb:ca)++;++j;}
        values.push_back(v);pa.push_back(pa.back()+ca);pb.push_back(pb.back()+cb);i=j;
    }
    fac.resize(n+1);ifac.resize(n+1);invs.resize(n+1);fac[0]=Mint(1);
    for(int i=1;i<=n;++i)fac[i]=fac[i-1]*Mint(i);
    ifac[n]=fac[n].pow(MOD-2);for(int i=n;i>0;--i)ifac[i-1]=ifac[i]*Mint(i);
    for(int i=1;i<=n;++i)invs[i]=fac[i-1]*ifac[i];
    tr.reserve(values.size()*2);int root=build(0,values.size());
    Poly ans=solve(root,false);ans.resize(n+1);
    for(int i=1;i<=n;++i){if(i>1)cout<<' ';cout<<ans[i].val();}cout<<'\n';
    return 0;
}
