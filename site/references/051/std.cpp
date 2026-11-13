#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;
static constexpr int P1 = 998244353;
static constexpr int P2 = 1004535809;
static constexpr int P3 = 469762049;
static constexpr int G = 3;

using Poly = vector<int>;

static int mod_pow(long long a, long long e, int mod) {
    long long r = 1;
    while (e) { if (e & 1) r = r * a % mod; a = a * a % mod; e >>= 1; }
    return (int)r;
}

static void ntt(vector<int> &a, bool invert, int mod) {
    int n = (int)a.size();
    for (int i=1,j=0;i<n;++i) {
        int bit=n>>1; for(;j&bit;bit>>=1) j^=bit; j^=bit;
        if(i<j) swap(a[i],a[j]);
    }
    for (int len=2;len<=n;len<<=1) {
        int wlen=mod_pow(G,(mod-1)/len,mod); if(invert) wlen=mod_pow(wlen,mod-2,mod);
        for(int i=0;i<n;i+=len) {
            long long w=1;
            for(int j=0;j<len/2;++j) {
                int u=a[i+j], v=(int)(a[i+j+len/2]*w%mod);
                int x=u+v; if(x>=mod)x-=mod; int y=u-v; if(y<0)y+=mod;
                a[i+j]=x; a[i+j+len/2]=y; w=w*wlen%mod;
            }
        }
    }
    if(invert) { long long ni=mod_pow(n,mod-2,mod); for(int &x:a)x=(int)(x*ni%mod); }
}

static vector<int> conv_mod(const vector<int> &a, const vector<int> &b, int mod) {
    if (a.empty() || b.empty()) return {};
    vector<int> x(a.begin(),a.end()), y(b.begin(),b.end());
    int need=(int)a.size()+(int)b.size()-1, n=1; while(n<need)n<<=1;
    x.resize(n); y.resize(n); ntt(x,false,mod); ntt(y,false,mod);
    for(int i=0;i<n;++i)x[i]=(int)((long long)x[i]*y[i]%mod);
    ntt(x,true,mod); x.resize(need); return x;
}

static Poly multiply_sparse(const Poly &a, int da, const Poly &b, int db) {
    int d=da+db, B=d+1;
    size_t maxA=(size_t)da*B+da+1, maxB=(size_t)db*B+db+1;
    long long cells=0;
    for(int x=0;x<=da;++x) for(int y=0;x+y<=da;++y) if(a[(size_t)x*(da+1)+y]) ++cells;
    long long cellsB=0;
    for(int x=0;x<=db;++x) for(int y=0;x+y<=db;++y) if(b[(size_t)x*(db+1)+y]) ++cellsB;
    if (cells && cellsB && cells*cellsB <= 250000) {
        Poly out((size_t)(d+1)*(d+1),0);
        for(int ax=0;ax<=da;++ax) for(int ay=0;ax+ay<=da;++ay) {
            int va=a[(size_t)ax*(da+1)+ay]; if(!va)continue;
            for(int bx=0;bx<=db;++bx) for(int by=0;bx+by<=db;++by) {
                int vb=b[(size_t)bx*(db+1)+by]; if(!vb)continue;
                int &z=out[(size_t)(ax+bx)*(d+1)+(ay+by)];
                z=(z+(long long)va*vb)%MOD;
            }
        }
        return out;
    }
    vector<int> ea(maxA,0), eb(maxB,0);
    for(int x=0;x<=da;++x) for(int y=0;x+y<=da;++y) ea[(size_t)x*B+y]=a[(size_t)x*(da+1)+y];
    for(int x=0;x<=db;++x) for(int y=0;x+y<=db;++y) eb[(size_t)x*B+y]=b[(size_t)x*(db+1)+y];
    vector<int> c1=conv_mod(ea,eb,P1), c2=conv_mod(ea,eb,P2), c3=conv_mod(ea,eb,P3);
    const long long invP1P2_modP3=mod_pow((long long)P1*P2%P3,P3-2,P3);
    Poly out((size_t)(d+1)*(d+1),0);
    long long p1mod= P1%MOD, p12mod=(long long)P1*P2%MOD;
    for(int x=0;x<=d;++x) for(int y=0;x+y<=d;++y) {
        size_t id=(size_t)x*B+y;
        long long r1=c1[id], r2=c2[id], r3=c3[id];
        long long t=(r2-r1)%P2; if(t<0)t+=P2; t=t*mod_pow(P1%P2,P2-2,P2)%P2;
        long long x12mod3=(r1+(long long)(P1%P3)*(t%P3))%P3;
        long long t2=(r3-x12mod3)%P3; if(t2<0)t2+=P3; t2=t2*invP1P2_modP3%P3;
        out[(size_t)x*(d+1)+y]=(int)((r1%MOD+p1mod*(t%MOD)+p12mod*(t2%MOD))%MOD);
    }
    return out;
}

static Poly add_shifted(const Poly &a,int da,const Poly &b,int db,int d) {
    Poly c((size_t)(d+1)*(d+1),0);
    for(int x=0;x<=da;++x)for(int y=0;x+y<=da;++y)c[(size_t)x*(d+1)+y]=a[(size_t)x*(da+1)+y];
    for(int x=0;x<=db;++x)for(int y=0;x+y<=db;++y){int &v=c[(size_t)x*(d+1)+y];v+=b[(size_t)x*(db+1)+y];if(v>=MOD)v-=MOD;}
    return c;
}

static vector<int> mul_small(const vector<int>&a,const vector<int>&b){vector<int>c(a.size()+b.size()-1);for(size_t i=0;i<a.size();++i)for(size_t j=0;j<b.size();++j)c[i+j]=(c[i+j]+(long long)a[i]*b[j])%MOD;return c;}
static void add_poly(vector<int>&a,const vector<int>&b,int shift,int scale=1){if(a.size()<b.size()+shift)a.resize(b.size()+shift);for(size_t i=0;i<b.size();++i){long long v=a[i+shift]+(long long)scale*b[i];v%=MOD;if(v<0)v+=MOD;a[i+shift]=(int)v;}}

// Coefficients of H_s, the degree-s part of 1/(1-xy-xz-yz-2xyz).
static Poly build_H(int s) {
    Poly h((size_t)(s+1)*(s+1),0);
    const int inv2=mod_pow(2,MOD-2,MOD);
    for(int c=0;c<=s/2;++c){
        int d=s-2*c, e=d&1, hh=(d-e)/2, L=c-e; if(L<0)continue;
        long long b0=1;
        for(int i=1;i<=e;++i)b0=b0*(c-i+1)%MOD*mod_pow(i,MOD-2,MOD)%MOD;
        b0=b0*mod_pow(2,e,MOD)%MOD;
        for(int i=1;i<=hh;++i)b0=b0*(c+i)%MOD*mod_pow(i,MOD-2,MOD)%MOD;
        vector<vector<vector<int>>> Q(4,vector<vector<int>>(4)); Q[0][0]={1};
        for(int k=0;k<3;++k)for(int j=0;j<=k;++j){
            const auto &q=Q[k][j]; if(q.empty())continue;
            vector<int> der(q.size()>1?q.size()-1:1,0); for(size_t t=1;t<q.size();++t)der[t-1]=(long long)t*q[t]%MOD;
            vector<int> w=der; add_poly(w,der,1,MOD-1); add_poly(w,q,0,2*k%MOD); // q*Q' + 2kQ
            vector<int> term; add_poly(term,w,1); add_poly(term,w,2); // x(1+x)w
            vector<int> lq; add_poly(lq,q,1); add_poly(lq,q,2,MOD-1); add_poly(term,lq,0,MOD-(L%MOD));
            add_poly(Q[k+1][j],term,0);
            vector<int> term2; add_poly(term2,q,1); add_poly(term2,q,3,MOD-1); add_poly(Q[k+1][j+1],term2,0);
        }
        vector<int> alpha={0,(int)((2*e-1LL)*(c+hh+1)%MOD),(int)((2LL*(c+hh+1)-2*e+1)%MOD),MOD-2};
        vector<int> beta={(int)((long long)hh*L*(L-1)%MOD),(int)((-2LL*hh*(2*L-1)-1LL*L*(L-1))%MOD+MOD)%MOD,(int)((4LL*hh+4LL*L-2)%MOD),MOD-4};
        vector<vector<int>> A(4);
        for(int j=0;j<4;++j)for(int k=j;k<4;++k){
            vector<int> pk={alpha[k],(int)((2LL*alpha[k]-2LL*beta[k])%MOD),alpha[k]};
            int powq=6-2*k; for(int z=0;z<powq;++z)pk=mul_small(pk,{1,MOD-1});
            add_poly(A[j],mul_small(pk,Q[k][j]),0);
        }
        vector<int> inv(s+5); for(int i=1;i<(int)inv.size();++i)inv[i]=(long long)(MOD-MOD/i)*inv[MOD%i]%MOD;
        vector<int> B(L+1,0); B[0]=(int)b0;
        for(int r=1;r<=L;++r){
            long long rhs=0;
            for(int j=0;j<4;++j)for(int u=j+1;u<(int)A[j].size();++u){
                int t=r-u+j; if(t<0||t>=r||t> L)continue;
                long long fall=1; for(int z=0;z<j;++z)fall=fall*(t-z)%MOD;
                rhs=(rhs+(long long)A[j][u]*fall%MOD*B[t])%MOD;
            }
            long long delta=(long long)r*(2LL*r+2*e-1)%MOD*(c+hh+1-r)%MOD;
            B[r]=(int)((MOD-rhs%MOD)*mod_pow(delta,MOD-2,MOD)%MOD);
        }
        for(int r=0;r<=L;++r){int x=hh+e+r,y=s-c-x;if(x>=0&&y>=0)h[(size_t)x*(s+1)+y]=B[r];}
    }
    return h;
}

static Poly run_poly(int len,int left,int right) {
    vector<Poly> f(4,Poly());
    for(int j=0;j<=3;++j){f[j].assign((size_t)(j+1)*(j+1),0);int total=1;for(int t=0;t<j;++t)total*=3;for(int mask=0;mask<total;++mask){int v=mask;int last=-1,a=0,b=0;bool ok=!(j==0&&left>=0&&right>=0&&left==right);for(int t=0;t<j;++t){int z=v%3;v/=3;if(z==last||(t==0&&left>=0&&z==left))ok=false;last=z;if(z==0)++a;else if(z==1)++b;}if(right>=0&&last==right)ok=false;if(ok){int &q=f[j][(size_t)a*(j+1)+b];if(++q==MOD)q=0;}}}
    vector<Poly> N(4);
    for(int j=0;j<=3;++j){N[j]=f[j];if(j>=2){for(int a=0;a<=j-2;++a)for(int b=0;a+b<=j-2;++b){int v=f[j-2][(size_t)a*(j-1)+b];if(!v)continue;auto sub=[&](int x,int y){int &q=N[j][(size_t)x*(j+1)+y];q=(q-(long long)v+MOD)%MOD;};sub(a+1,b+1);sub(a+1,b);sub(a,b+1);}}if(j>=3){for(int a=0;a<=j-3;++a)for(int b=0;a+b<=j-3;++b){int v=2LL*f[j-3][(size_t)a*(j-2)+b]%MOD;N[j][(size_t)(a+1)*(j+1)+b+1]=(N[j][(size_t)(a+1)*(j+1)+b+1]-(long long)v+MOD)%MOD;}}}
    Poly out((size_t)(len+1)*(len+1),0); vector<Poly> hs(4); for(int j=0;j<=3;++j)if(len>=j)hs[j]=build_H(len-j);
    for(int j=0;j<=3;++j)if(len>=j)for(int a=0;a<=j;++a)for(int b=0;a+b<=j;++b){int v=N[j][(size_t)a*(j+1)+b];if(!v)continue;int s=len-j;for(int x=0;x<=s;++x)for(int y=0;x+y<=s;++y){int z=hs[j][(size_t)x*(s+1)+y];if(z)out[(size_t)(x+a)*(len+1)+(y+b)]=(out[(size_t)(x+a)*(len+1)+(y+b)]+(long long)v*z)%MOD;}}
    return out;
}

struct Item{int deg;Poly p;bool operator>(const Item&o)const{return deg>o.deg;}};

int main(){ios::sync_with_stdio(false);cin.tie(nullptr);int n,Q;if(!(cin>>n>>Q))return 0;string s;cin>>s;int m=count(s.begin(),s.end(),'?');
    priority_queue<Item,vector<Item>,greater<Item>> pq; bool bad=false; int i=0;
    while(i<n){if(s[i]!='?'){if(i&&s[i]==s[i-1]&&s[i-1]!='?')bad=true;++i;continue;}int st=i;while(i<n&&s[i]=='?')++i;int len=i-st,left=(st? s[st-1]-'a':-1),right=(i<n?s[i]-'a':-1);pq.push({len,run_poly(len,left,right)});}
    if(bad){for(int k=0;k<Q;++k){int x,y,z;cin>>x>>y>>z;cout<<0<<'\n';}return 0;}
    if(pq.empty()){for(int k=0;k<Q;++k){int x,y,z;cin>>x>>y>>z;cout<<(s.empty()?0:1)<<'\n';}return 0;}
    while(pq.size()>1){Item a=pq.top();pq.pop();Item b=pq.top();pq.pop();pq.push({a.deg+b.deg,multiply_sparse(a.p,a.deg,b.p,b.deg)});}Item all=pq.top();
    const int d=all.deg, side=d+2; auto id=[side](int a,int b)->size_t{return (size_t)a*side+b;};
    vector<int> exact((size_t)(d+1)*(d+1),0), sa(d+2,0), sb(d+2,0), sc(d+2,0);
    for(int a=0;a<=d;++a)for(int b=0;a+b<=d;++b){int v=all.p[(size_t)a*(d+1)+b];exact[(size_t)a*(d+1)+b]=v;sa[a]=(sa[a]+v)%MOD;sb[b]=(sb[b]+v)%MOD;sc[d-a-b]=(sc[d-a-b]+v)%MOD;}
    for(int i=d-1;i>=0;--i){sa[i]=(sa[i]+sa[i+1])%MOD;sb[i]=(sb[i]+sb[i+1])%MOD;sc[i]=(sc[i]+sc[i+1])%MOD;}
    vector<int> ab((size_t)side*side,0),ac((size_t)side*side,0),bc((size_t)side*side,0);
    for(int a=0;a<=d;++a)for(int b=0;a+b<=d;++b){int v=exact[(size_t)a*(d+1)+b];int c=d-a-b;ab[id(a,b)]=(ab[id(a,b)]+v)%MOD;ac[id(a,c)]=(ac[id(a,c)]+v)%MOD;bc[id(b,c)]=(bc[id(b,c)]+v)%MOD;}
    auto suffix=[&](vector<int>&mat){for(int a=d;a>=0;--a)for(int b=d;b>=0;--b){long long v=(long long)mat[id(a,b)]+mat[id(a+1,b)]+mat[id(a,b+1)]-mat[id(a+1,b+1)];v%=MOD;if(v<0)v+=MOD;mat[id(a,b)]=(int)v;}};suffix(ab);suffix(ac);suffix(bc);
    int total=sa[0];
    for(int k=0;k<Q;++k){int x,y,z;cin>>x>>y>>z;x=min(x,d);y=min(y,d);z=min(z,d);int A=sa[x+1],Bv=sb[y+1],C=sc[z+1],AB=ab[id(x+1,y+1)],AC=ac[id(x+1,z+1)],BC=bc[id(y+1,z+1)];long long ans=(long long)total-A-Bv-C+AB+AC+BC;ans%=MOD;if(ans<0)ans+=MOD;cout<<ans<<'\n';}
}
