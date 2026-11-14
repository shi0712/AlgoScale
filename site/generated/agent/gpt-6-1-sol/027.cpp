#include <bits/stdc++.h>
using namespace std;

/*
An alive player represents the interval of players merged into its kill chain.
An unacted player is still a singleton, so a shot attaches this singleton to
the adjacent interval. Every final component is therefore an interval built
by adding R's on its left and L's on its right. No component crosses an LR.

For an internal component with a R's and b L's, m=a+b>=2, the number of
orders is (m-1)*binom(m,a). The unique player killed before firing can be
either of the two players at the RL boundary; all other firing orders are
interleavings of the left and right chains. Its own turn can be inserted in
any of the m-1 positions after the first shot. For a pure component touching
an outward-facing end, this becomes m (and m=1 is also allowed).

Use component weights divided by m!, and let x mark final components:
  A(t)=(t-1)e^t+1, B(t)=t e^t,
  H_m(x)=[t^m] 1/(1-x A(t)),
  J_m(x)=[t^m] B(t)/(1-x A(t)).
A pure end block contributes x J_m. An R^a L^b block contributes
  x J_a J_b + (x-1) H_a H_b / x.
Multiply all block polynomials and finally multiply by n!.

To obtain H_m and J_m fast, let
  D_m(z)=m! [t^m] exp(-zt)(1-t)^(-z).
These are cycle-count polynomials for permutations without fixed points:
  D_m=(m-1)(D_{m-1}+z D_{m-2}).
Writing D_m=(m-1)! z T_{m-2} gives the recurrence used below. Matrix
products compute consecutive T's. Evaluate them at consecutive negative
integers, then apply a factorial convolution (finite differences) to obtain
H_m and J_m. All polynomial operations use NTT. The total time is
O(n log^2 n), and the space bound is O(n log n).
*/

static constexpr int MOD = 998244353;
using Poly = vector<int>;
int addmod(int a,int b){a+=b;if(a>=MOD)a-=MOD;return a;}
int submod(int a,int b){a-=b;if(a<0)a+=MOD;return a;}
int power(int a,int b){int r=1;for(;b;b>>=1,a=(long long)a*a%MOD)if(b&1)r=(long long)r*a%MOD;return r;}
void trim(Poly &a){while(!a.empty()&&a.back()==0)a.pop_back();}
void ntt(Poly &a){
    int n=a.size();
    static Poly roots{0,1},rev;
    if((int)rev.size()!=n){
        rev.resize(n);
        for(int i=0;i<n;i++)rev[i]=(rev[i>>1]>>1)|((i&1)*(n>>1));
    }
    if((int)roots.size()<n){
        int k=__builtin_ctz((unsigned)roots.size()); roots.resize(n);
        while((1<<k)<n){
            int e=power(3,(MOD-1)>>(k+1));
            for(int i=1<<(k-1);i<(1<<k);i++){
                roots[2*i]=roots[i]; roots[2*i+1]=(long long)roots[i]*e%MOD;
            }
            k++;
        }
    }
    for(int i=0;i<n;i++)if(i<rev[i])swap(a[i],a[rev[i]]);
    for(int len=1;len<n;len<<=1)
        for(int i=0;i<n;i+=len*2)
            for(int j=0;j<len;j++){
                int z=(long long)a[i+j+len]*roots[len+j]%MOD;
                int x=a[i+j];a[i+j]=addmod(x,z);a[i+j+len]=submod(x,z);
            }
}
void intt(Poly &a){
    reverse(a.begin()+1,a.end());ntt(a);int z=power(a.size(),MOD-2);
    for(int &x:a)x=(long long)x*z%MOD;
}
Poly multiply(const Poly &a,const Poly &b){
    if(a.empty()||b.empty())return {};
    int sz=a.size()+b.size()-1;
    if(min(a.size(),b.size())<=20){
        Poly c(sz);
        for(int i=0;i<(int)a.size();i++)for(int j=0;j<(int)b.size();j++)
            c[i+j]=(c[i+j]+(long long)a[i]*b[j])%MOD;
        return c;
    }
    int n=1;while(n<sz)n<<=1;
    Poly x(a),y(b);x.resize(n);y.resize(n);ntt(x);ntt(y);
    for(int i=0;i<n;i++)x[i]=(long long)x[i]*y[i]%MOD;
    intt(x);x.resize(sz);return x;
}
Poly inverse_series(const Poly &a,int n){
    Poly b{power(a[0],MOD-2)};
    while((int)b.size()<n){
        int m=min(n,(int)b.size()*2);
        Poly cut(a.begin(),a.begin()+min((int)a.size(),m));
        Poly ab=multiply(cut,b);ab.resize(m);
        for(int &x:ab)if(x)x=MOD-x;
        ab[0]=addmod(ab[0],2);
        b=multiply(b,ab);b.resize(m);
    }
    return b;
}
vector<int> fac,ifac,invnum;

// T_0=T_1=1, T_i=T_{i-1}+(z/i)T_{i-2}.
struct Matrix{array<Poly,4>a;};
Matrix matrix_product(const Matrix &x,const Matrix &y){
    Matrix z;
    int degree=0;
    for(int i=0;i<2;i++)for(int j=0;j<2;j++)for(int k=0;k<2;k++)
        if(!x.a[2*i+k].empty()&&!y.a[2*k+j].empty())
            degree=max(degree,(int)x.a[2*i+k].size()+(int)y.a[2*k+j].size()-2);
    if(degree<=40){
        for(int i=0;i<2;i++)for(int j=0;j<2;j++){
            auto &p=z.a[2*i+j];
            for(int k=0;k<2;k++){
                Poly q=multiply(x.a[2*i+k],y.a[2*k+j]);
                if(p.size()<q.size())p.resize(q.size());
                for(int t=0;t<(int)q.size();t++)p[t]=addmod(p[t],q[t]);
            }
            trim(p);
        }
        return z;
    }
    int n=1;while(n<=degree)n<<=1;
    array<Poly,4> a=x.a,b=y.a;
    for(auto &p:a){p.resize(n);ntt(p);}
    for(auto &p:b){p.resize(n);ntt(p);}
    for(int i=0;i<2;i++)for(int j=0;j<2;j++){
        Poly &p=z.a[2*i+j];p.resize(n);
        for(int t=0;t<n;t++)p[t]=((long long)a[2*i][t]*b[j][t]+(long long)a[2*i+1][t]*b[2+j][t])%MOD;
        intt(p);p.resize(degree+1);trim(p);
    }
    return z;
}
Matrix recurrence_product(int l,int r){
    if(l==r){Matrix m;m.a={Poly{1},Poly{0,invnum[l]},Poly{1},Poly{}};return m;}
    int mid=(l+r)/2;
    Matrix left=recurrence_product(l,mid),right=recurrence_product(mid+1,r);
    return matrix_product(right,left);
}
pair<Poly,Poly> consecutive_T(int m){
    if(m<=100){
        Poly prev{1},cur{1};
        for(int i=2;i<=m-1;i++){
            Poly next=cur;next.resize(max(cur.size(),prev.size()+1));
            for(int j=0;j<(int)prev.size();j++)next[j+1]=(next[j+1]+(long long)prev[j]*invnum[i])%MOD;
            prev=move(cur);cur=move(next);
        }
        return {cur,prev};
    }
    Matrix p=recurrence_product(2,m-1);
    Poly cur=p.a[0],prev=p.a[2];
    cur.resize(max(cur.size(),p.a[1].size()));prev.resize(max(prev.size(),p.a[3].size()));
    for(int i=0;i<(int)p.a[1].size();i++)cur[i]=addmod(cur[i],p.a[1][i]);
    for(int i=0;i<(int)p.a[3].size();i++)prev[i]=addmod(prev[i],p.a[3][i]);
    trim(cur);trim(prev);return {move(cur),move(prev)};
}

// A transposed remainder tree evaluates two polynomials at 0,-1,-2,... .
struct Evaluator{
    int n;
    vector<Poly> tree;
    Evaluator(int n):n(n),tree(4*n){build(1,0,n);}
    void build(int v,int l,int r){
        if(r-l==1){tree[v]={l,1};return;}
        int mid=(l+r)/2;build(v*2,l,mid);build(v*2+1,mid,r);
        tree[v]=multiply(tree[v*2],tree[v*2+1]);
    }
    void descend(int v,int l,int r,Poly a,Poly b,Poly &va,Poly &vb){
        int m=r-l;
        if(m==1){va[l]=a[0];if(!b.empty())vb[l]=b[0];return;}
        int mid=(l+r)/2,nl=mid-l,nr=r-mid;
        Poly al(nl),ar(nr),bl,br;
        bool two=!b.empty();if(two){bl.resize(nl);br.resize(nr);}
        if(m<=64){
            for(int i=0;i<nl;i++)for(int j=0;j<=nr;j++){
                al[i]=(al[i]+(long long)a[i+j]*tree[v*2+1][j])%MOD;
                if(two)bl[i]=(bl[i]+(long long)b[i+j]*tree[v*2+1][j])%MOD;
            }
            for(int i=0;i<nr;i++)for(int j=0;j<=nl;j++){
                ar[i]=(ar[i]+(long long)a[i+j]*tree[v*2][j])%MOD;
                if(two)br[i]=(br[i]+(long long)b[i+j]*tree[v*2][j])%MOD;
            }
        }else{
            int len=1;while(len<m)len<<=1;
            a.resize(len);ntt(a);if(two){b.resize(len);ntt(b);}
            Poly work(len),out(len);
            for(int side=0;side<2;side++){
                const Poly &sibling=tree[v*2+(side==0)];
                fill(work.begin(),work.end(),0);
                copy(sibling.rbegin(),sibling.rend(),work.begin());ntt(work);
                int offset=sibling.size()-1,count=side==0?nl:nr;
                for(int i=0;i<len;i++)out[i]=(long long)a[i]*work[i]%MOD;
                intt(out);
                Poly &dest=side==0?al:ar;copy(out.begin()+offset,out.begin()+offset+count,dest.begin());
                if(two){
                    for(int i=0;i<len;i++)out[i]=(long long)b[i]*work[i]%MOD;
                    intt(out);
                    Poly &destb=side==0?bl:br;copy(out.begin()+offset,out.begin()+offset+count,destb.begin());
                }
            }
        }
        descend(v*2,l,mid,move(al),move(bl),va,vb);
        descend(v*2+1,mid,r,move(ar),move(br),va,vb);
    }
    pair<Poly,Poly> evaluate(const Poly &a,const Poly &b){
        Poly va(n),vb;if(!b.empty())vb.resize(n);
        Poly rp(tree[1].rbegin(),tree[1].rend());
        Poly iv=inverse_series(rp,n);
        auto initial=[&](const Poly &f){
            Poly x(n);
            for(int i=0;i<(int)f.size();i++)x[n-1-i]=f[i];
            x=multiply(x,iv);x.resize(n);return x;
        };
        Poly x=initial(a),y;if(!b.empty())y=initial(b);
        descend(1,0,n,move(x),move(y),va,vb);
        return {move(va),move(vb)};
    }
};

Poly binomial_transform(Poly values){
    int n=values.size();Poly b(n);
    for(int i=0;i<n;i++){
        values[i]=(long long)values[i]*ifac[i]%MOD;
        if(i&1)if(values[i])values[i]=MOD-values[i];
        b[i]=ifac[i];
    }
    Poly c=multiply(values,b);c.resize(n);
    for(int i=0;i<n;i++)c[i]=(long long)c[i]*fac[i]%MOD;
    trim(c);return c;
}
struct Run{Poly h,j;};
map<int,Run> cache;
const Run &get_run(int m){
    auto found=cache.find(m);if(found!=cache.end())return found->second;
    Run result;
    if(m==1){result.h={};result.j={1};return cache.emplace(m,move(result)).first->second;}
    auto [cur,prev]=consecutive_T(m);
    int dj=(m-1)/2,dh=m/2;
    int count=max(dj+2,dh+1);
    Poly vcur(count),vprev(count);
    if(m<=100){
        for(int i=0;i<count;i++){
            int x=i?MOD-i:0;
            for(int j=(int)cur.size()-1;j>=0;j--)vcur[i]=((long long)vcur[i]*x+cur[j])%MOD;
            for(int j=(int)prev.size()-1;j>=0;j--)vprev[i]=((long long)vprev[i]*x+prev[j])%MOD;
        }
    }else{
        Evaluator eval(count);
        tie(vcur,vprev)=eval.evaluate(cur,prev);
    }
    Poly jvalues(dj+1),hvalues(dh+1);
    for(int i=0;i<=dj;i++)jvalues[i]=vcur[i+1];
    for(int i=1;i<=dh;i++)hvalues[i]=(long long)(MOD-i)*invnum[m]%MOD*vprev[i]%MOD;
    result.j=binomial_transform(move(jvalues));result.h=binomial_transform(move(hvalues));
    return cache.emplace(m,move(result)).first->second;
}

int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int n;string s;if(!(cin>>n>>s))return 0;
    fac.resize(n+3);ifac.resize(n+3);invnum.resize(n+3);
    fac[0]=1;for(int i=1;i<=n+2;i++)fac[i]=(long long)fac[i-1]*i%MOD;
    ifac[n+2]=power(fac[n+2],MOD-2);for(int i=n+2;i>=1;i--)ifac[i-1]=(long long)ifac[i]*i%MOD;
    for(int i=1;i<=n+2;i++)invnum[i]=(long long)fac[i-1]*ifac[i]%MOD;
    vector<Poly> polys;
    priority_queue<pair<int,int>,vector<pair<int,int>>,greater<pair<int,int>>> pq;
    int blocks=0,scalar=1;
    for(int l=0;l<n;){
        int r=l+1;while(r<n&&!(s[r-1]=='L'&&s[r]=='R'))r++;
        int a=0;while(l+a<r&&s[l+a]=='R')a++;
        int b=r-l-a;Poly q;
        if(!a||!b)q=get_run(r-l).j;
        else{
            const Run &u=get_run(a),&v=get_run(b);
            q=multiply(u.j,v.j);
            Poly hh=multiply(u.h,v.h);
            if(hh.size()>=2){
                if(q.size()<hh.size()-1)q.resize(hh.size()-1);
                // P/x = J_a J_b + H_a H_b/x - H_a H_b/x^2.
                for(int i=1;i<(int)hh.size();i++)q[i-1]=addmod(q[i-1],hh[i]);
                for(int i=2;i<(int)hh.size();i++)q[i-2]=submod(q[i-2],hh[i]);
            }
            trim(q);
        }
        if(q.size()==1)scalar=(long long)scalar*q[0]%MOD;
        else{int id=polys.size();pq.push({q.size(),id});polys.push_back(move(q));}
        blocks++;l=r;
    }
    while(pq.size()>1){
        int a=pq.top().second;pq.pop();int b=pq.top().second;pq.pop();
        Poly q=multiply(polys[a],polys[b]);Poly().swap(polys[a]);Poly().swap(polys[b]);
        int id=polys.size();pq.push({q.size(),id});polys.push_back(move(q));
    }
    Poly finalpoly=pq.empty()?Poly{1}:move(polys[pq.top().second]);
    scalar=(long long)scalar*fac[n]%MOD;
    for(int k=1;k<=n;k++){
        int index=k-blocks,ans=0;
        if(index>=0&&index<(int)finalpoly.size())ans=(long long)finalpoly[index]*scalar%MOD;
        cout<<ans<<(k==n?'\n':' ');
    }
    return 0;
}
