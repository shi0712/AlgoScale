#include <bits/stdc++.h>
using namespace std;

struct Mint {
    static constexpr uint32_t mod=998244353, mod2=mod*2;
    static constexpr uint32_t inv=998244351; // -mod^{-1} modulo 2^32
    static constexpr uint32_t r2=932051910; // 2^64 modulo mod
    uint32_t a;
    Mint():a(0){}
    Mint(long long v){v%=mod;if(v<0)v+=mod;a=reduce(uint64_t(v)*r2);}
    static uint32_t reduce(uint64_t v){return (v+uint64_t(uint32_t(v)*inv)*mod)>>32;}
    static Mint raw(uint32_t x){Mint t;t.a=x;return t;}
    uint32_t val()const{uint32_t v=reduce(a);return v>=mod?v-mod:v;}
    Mint& operator+=(Mint b){a+=b.a;if(a>=mod2)a-=mod2;return *this;}
    Mint& operator-=(Mint b){a+=mod2-b.a;if(a>=mod2)a-=mod2;return *this;}
    Mint& operator*=(Mint b){a=reduce(uint64_t(a)*b.a);return *this;}
    friend Mint operator+(Mint a,Mint b){return a+=b;}
    friend Mint operator-(Mint a,Mint b){return a-=b;}
    friend Mint operator*(Mint a,Mint b){return a*=b;}
    Mint pow(long long n)const{Mint a=*this,r=1;while(n){if(n&1)r*=a;a*=a;n>>=1;}return r;}
    Mint inverse()const{return pow(mod-2);}
};

// Forward transform and its inverse, using roots shared by every transform.
vector<Mint> roots{Mint(0),Mint(1)};
void ensure_roots(int n){
    int old=roots.size(); if(old>=n)return;
    roots.resize(n);
    for(int k=old;k<n;k*=2){
        Mint z=Mint(3).pow((Mint::mod-1)/(2*k));
        for(int i=k/2;i<k;i++){roots[2*i]=roots[i];roots[2*i+1]=roots[i]*z;}
    }
}
void ntt(vector<Mint>& a,bool inverse=false){
    int n=a.size();ensure_roots(n);
    for(int i=1,j=0;i<n;i++){
        int b=n>>1;for(;j&b;b>>=1)j^=b;j^=b;
        if(i<j)swap(a[i],a[j]);
    }
    for(int k=1;k<n;k*=2)
        for(int i=0;i<n;i+=2*k)
            for(int j=0;j<k;j++){
                Mint z=a[i+j+k]*roots[k+j];
                a[i+j+k]=a[i+j]-z;a[i+j]+=z;
            }
    if(inverse){reverse(a.begin()+1,a.end());Mint z=Mint(n).inverse();for(auto &v:a)v*=z;}
}
vector<Mint> multiply(const vector<Mint>& a,const vector<Mint>& b){
    if(a.empty()||b.empty())return {};
    int sz=a.size()+b.size()-1;
    if(min(a.size(),b.size())<=20){
        vector<Mint> c(sz);
        for(int i=0;i<(int)a.size();i++)for(int j=0;j<(int)b.size();j++)c[i+j]+=a[i]*b[j];
        return c;
    }
    int z=1;while(z<sz)z*=2;
    vector<Mint>x(a),y(b);x.resize(z);y.resize(z);ntt(x);ntt(y);
    for(int i=0;i<z;i++)x[i]*=y[i];ntt(x,true);x.resize(sz);return x;
}

/*
Let f_n(m) be the answer, including f_0(m)=1, and put
A_n(u) = sum_{m>=0} f_n(m) u^m. The counting recurrence is

 A_n = u A'_{n-1} - u sum_{i=0}^{n-2} A_i A_{n-2-i}.

Write A_n=P_n/(1-u)^{n+1}. Consequently,
 P_n = u(1-u)P'_{n-1} + n*u*P_{n-1}
       - u(1-u) sum_{i=0}^{n-2} P_i P_{n-2-i}.

The quadratic term is computed by divide and conquer in n. An NTT in u
turns its polynomial products into independent scalar convolutions. The
precomputation costs O(N^2 log^2 N) time and O(N^2) memory.

Finally P_n(u)=sum_k d_k*k!*u^k*(1-u)^{n-k}, where
 f_n(m)=sum_k d_k*(m)_k. A convolution gives the d_k; a shared product tree
converts the falling factorial basis to ordinary powers for Horner evaluation.
*/
int N,W;
vector<Mint> transformed, pending;
vector<vector<Mint>> numerator;

// Transform along the row coordinate; each row contains W independent values.
void row_ntt(vector<Mint>& a,int z,bool inverse){
    ensure_roots(z);
    for(int i=1,j=0;i<z;i++){
        int b=z>>1;for(;j&b;b>>=1)j^=b;j^=b;
        if(i<j)for(int f=0;f<W;f++)swap(a[size_t(i)*W+f],a[size_t(j)*W+f]);
    }
    for(int k=1;k<z;k*=2)for(int i=0;i<z;i+=2*k)for(int j=0;j<k;j++){
        Mint rt=roots[k+j];
        Mint *p=a.data()+size_t(i+j)*W,*q=p+size_t(k)*W;
        for(int f=0;f<W;f++){
            Mint v=q[f]*rt;q[f]=p[f]-v;p[f]+=v;
        }
    }
    if(inverse){
        Mint iz=Mint(z).inverse();
        for(int i=1;i<(z+1)/2;i++)for(int f=0;f<W;f++)swap(a[size_t(i)*W+f],a[size_t(z-i)*W+f]);
        for(auto &v:a)v*=iz;
    }
}
void finish_row(int n){
    vector<Mint> h(W);
    if(n>=2){copy_n(pending.data()+size_t(n)*W,W,h.data());ntt(h,true);}
    auto &p=numerator[n];p.resize(n+1);
    if(n==0)p[0]=1;
    else for(int k=0;k<=n;k++){
        if(k<n)p[k]+=numerator[n-1][k]*Mint(k);
        if(k)p[k]+=numerator[n-1][k-1]*Mint(n-k+1);
        if(k>=1&&k-1<=n-2)p[k]-=h[k-1];
        if(k>=2)p[k]+=h[k-2];
    }
    fill(h.begin(),h.end(),Mint(0));copy(p.begin(),p.end(),h.begin());ntt(h);
    copy(h.begin(),h.end(),transformed.begin()+size_t(n)*W);
}
void solve_rows(int l,int r){
    if(r-l<=16){
        for(int i=l;i<r;i++){
            finish_row(i);
            for(int j=0;j<=i&&i+j+2<r;j++){
                Mint *dst=pending.data()+size_t(i+j+2)*W;
                const Mint *a=transformed.data()+size_t(i)*W,*b=transformed.data()+size_t(j)*W;
                if(i==j)for(int f=0;f<W;f++)dst[f]+=a[f]*b[f];
                else for(int f=0;f<W;f++){Mint t=a[f]*b[f];dst[f]+=t+t;}
            }
        }
        return;
    }
    int mid=(l+r)/2;solve_rows(l,mid);
    int alen=mid-l,blen=min(mid,r-l-2);
    if(blen>0){
        int z=1;while(z<alen+blen-1)z*=2;
        vector<Mint>a(size_t(z)*W),b;
        bool square=l==0;
        for(int i=0;i<alen;i++)copy_n(transformed.data()+size_t(l+i)*W,W,a.data()+size_t(i)*W);
        if(!square){
            b.resize(size_t(z)*W);
            for(int j=0;j<blen;j++){
                const Mint *src=transformed.data()+size_t(j)*W;Mint *dst=b.data()+size_t(j)*W;
                if(j<l)for(int f=0;f<W;f++)dst[f]=src[f]+src[f];
                else copy_n(src,W,dst);
            }
        }
        row_ntt(a,z,false);
        if(square)for(auto &v:a)v*=v;
        else{row_ntt(b,z,false);for(size_t i=0;i<a.size();i++)a[i]*=b[i];}
        row_ntt(a,z,true);
        for(int n=mid;n<r;n++){
            int k=n-l-2;if(k<0||k>=alen+blen-1)continue;
            Mint *dst=pending.data()+size_t(n)*W,*src=a.data()+size_t(k)*W;
            for(int f=0;f<W;f++)dst[f]+=src[f];
        }
    }
    solve_rows(mid,r);
}

// Shared products and transforms for conversion from falling factorials.
struct Node { vector<Mint> prod,ft;int l,r,left=-1,right=-1;};
vector<Node> tree;
int build_tree(int l,int r){
    int id=tree.size();tree.push_back({});tree[id].l=l;tree[id].r=r;
    if(r-l==1){tree[id].prod={Mint(-l),Mint(1)};return id;}
    int mid=(l+r)/2,lc=build_tree(l,mid),rc=build_tree(mid,r);
    tree[id].left=lc;tree[id].right=rc;
    tree[id].prod=multiply(tree[lc].prod,tree[rc].prod);
    int z=1;while(z<r-l)z*=2;
    tree[id].ft=tree[lc].prod;tree[id].ft.resize(z);ntt(tree[id].ft);
    return id;
}
vector<Mint> convert_basis(int id,const vector<Mint>& d,int end){
    auto &v=tree[id];
    if(v.l>=end)return {};
    if(v.r-v.l==1)return {d[v.l]};
    auto a=convert_basis(v.left,d,end),b=convert_basis(v.right,d,end);
    if(b.empty())return a;
    vector<Mint> c;
    int sz=tree[v.left].prod.size()+b.size()-1;
    if(b.size()<=16||tree[v.left].prod.size()<=16)c=multiply(tree[v.left].prod,b);
    else{
        int z=v.ft.size();b.resize(z);ntt(b);
        for(int i=0;i<z;i++)b[i]*=v.ft[i];ntt(b,true);b.resize(sz);c=move(b);
    }
    if(c.size()<a.size())c.resize(a.size());for(int i=0;i<(int)a.size();i++)c[i]+=a[i];return c;
}

struct FastInput{
    static constexpr int S=1<<16;char b[S];int p=0,n=0;
    char gc(){if(p==n){n=fread(b,1,S,stdin);p=0;if(!n)return 0;}return b[p++];}
    int read(){char c;do{c=gc();}while(c<=32&&c);int x=0;while(c>32){x=x*10+c-'0';c=gc();}return x;}
};
int main(){
    FastInput in;int T=in.read();
    vector<vector<pair<uint32_t,int>>> qs(1001);N=0;
    for(int i=0;i<T;i++){int n=in.read(),m=in.read();qs[n].push_back({uint32_t(m),i});N=max(N,n);}
    W=1;while(W<=N)W*=2;
    transformed.resize(size_t(N+1)*W);pending.resize(size_t(N+1)*W);numerator.resize(N+1);
    solve_rows(0,N+1);
    vector<Mint>().swap(transformed);vector<Mint>().swap(pending);
    vector<Mint> fact(N+1),ifact(N+1);fact[0]=1;
    for(int i=1;i<=N;i++)fact[i]=fact[i-1]*Mint(i);
    ifact[N]=fact[N].inverse();for(int i=N;i>0;i--)ifact[i-1]=ifact[i]*Mint(i);
    build_tree(0,W);
    vector<vector<Mint>> invft(12);
    vector<uint32_t> ans(T);
    for(int n=1;n<=N;n++)if(!qs[n].empty()){
        int z=1;while(z<2*n+1)z*=2;int lg=__builtin_ctz(unsigned(z));
        if(invft[lg].empty()){
            invft[lg].resize(z);for(int i=0;i<=min(N,z/2-1);i++)invft[lg][i]=ifact[i];ntt(invft[lg]);
        }
        vector<Mint>d(z);
        for(int j=0;j<=n;j++)d[j]=numerator[n][j]*fact[n-j];
        ntt(d);for(int j=0;j<z;j++)d[j]*=invft[lg][j];ntt(d,true);d.resize(n+1);
        for(int j=0;j<=n;j++)d[j]*=ifact[n-j]*ifact[j];
        auto coeff=convert_basis(0,d,n+1);coeff.resize(n+1);
        auto &q=qs[n];
        // Interleave independent Horner evaluations to hide multiplication latency.
        for(size_t j=0;j<q.size();j+=8){
            int cnt=min(size_t(8),q.size()-j);Mint x[8],v[8];
            for(int k=0;k<cnt;k++){x[k]=Mint(q[j+k].first);v[k]=coeff[n];}
            for(int t=n-1;t>=0;t--)for(int k=0;k<cnt;k++)v[k]=v[k]*x[k]+coeff[t];
            for(int k=0;k<cnt;k++)ans[q[j+k].second]=v[k].val();
        }
    }
    string output;output.reserve(size_t(T)*11);
    for(auto v:ans){char buf[16];auto [end,ec]=to_chars(buf,buf+16,v);output.append(buf,end);output+='\n';}
    fwrite(output.data(),1,output.size(),stdout);
}
