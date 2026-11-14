#include <bits/stdc++.h>
#include <immintrin.h>
using namespace std;
#pragma GCC target("avx2,fma")

/*
For a threshold t, call an entry high when it is >= t. Two high entries at
distance at most two can fill a length-three interval, which can then expand
to fill the array. If all high entries are at least three apart, every allowed
interval has a low majority, so no operation can produce any new high entry.
Thus the answer for one array is max(min(a[i],a[j])) over 1 <= j-i <= 2.

Let F(t) count arrays whose high entries are at least three apart. At position
i there are low_i(t)=clamp(t-l_i,0,r_i-l_i+1) low choices and len_i-low_i(t)
high choices. A three-state automaton counts F(t). Between consecutive input
endpoints its transition matrices are linear polynomials, and F is a polynomial
of degree at most the number of currently nonconstant low_i.

Sweep the endpoints, maintaining matrix products evaluated at 0,1,...,degree.
A segment tree updates those evaluations in O(n log n) per endpoint. Interpolate
the prefix sums of F to sum each polynomial over its entire integer interval.
There are O(n) endpoints: O(n^2 log n) time and O(n^2) memory. Short intervals
use the direct automaton instead. Independent evaluations are batched in fours.
*/
constexpr int MOD=1000000007;
using ll=long long;
int addmod(int a,int b){int s=a+b;return s>=MOD?s-MOD:s;}
int mulmod(int a,int b){return (ll)a*b%MOD;}
int power(int a,int e){int r=1;for(;e;e>>=1,a=mulmod(a,a))if(e&1)r=mulmod(r,a);return r;}

// Four independent modular computations. A floating approximation of the
// quotient suffices: the exact remainder is recovered with 32-bit arithmetic.
struct V { __m128i i; __m256d d; };
inline V makev(__m128i x){return {x,_mm256_cvtepi32_pd(x)};}
inline V constantv(int x){return makev(_mm_set1_epi32(x));}
inline __m128i addi(__m128i a,__m128i b){
    __m128i s=_mm_add_epi32(a,b),p=_mm_set1_epi32(MOD);
    return _mm_sub_epi32(s,_mm_and_si128(_mm_cmpgt_epi32(s,_mm_set1_epi32(MOD-1)),p));
}
inline V plusv(V a,V b){return makev(addi(a.i,b.i));}
inline __m128i reduce4(__m128i low,__m256d approximate){
    __m256d q=_mm256_fmadd_pd(approximate,_mm256_set1_pd(1.0/MOD),_mm256_set1_pd(4503599627370496.0));
    __m256i qi=_mm256_permutevar8x32_epi32(_mm256_castpd_si256(q),_mm256_setr_epi32(0,2,4,6,0,0,0,0));
    __m128i r=_mm_sub_epi32(low,_mm_mullo_epi32(_mm256_castsi256_si128(qi),_mm_set1_epi32(MOD)));
    return _mm_add_epi32(r,_mm_and_si128(_mm_srai_epi32(r,31),_mm_set1_epi32(MOD)));
}
inline V timesv(V a,V b){return makev(reduce4(_mm_mullo_epi32(a.i,b.i),_mm256_mul_pd(a.d,b.d)));}
inline __m128i dot3(V a,V b,V c,V x,V y,V z){
    __m128i low=_mm_add_epi32(_mm_add_epi32(_mm_mullo_epi32(a.i,x.i),_mm_mullo_epi32(b.i,y.i)),_mm_mullo_epi32(c.i,z.i));
    __m256d val=_mm256_fmadd_pd(c.d,z.d,_mm256_fmadd_pd(b.d,y.d,_mm256_mul_pd(a.d,x.d)));
    return reduce4(low,val);
}

struct Node {
    int left=-1,right=-1,begin,end,type; // 0: matrix, 1: column, 2: row, 3: scalar
    vector<int> v;
    bool dirty=false;
};
struct Solver {
    int n,stride,root,previous=0;
    vector<int> l,r,len,offset,slope,leaf;
    vector<Node> tree;
    vector<int> fact,invfact;
    Solver(int n,vector<int> l,vector<int> r):n(n),l(move(l)),r(move(r)){
        stride=(n+4)&~3;
        len.resize(n);offset.resize(n);slope.resize(n);leaf.resize((n+3)/4);
        for(int i=0;i<n;i++)len[i]=this->r[i]-this->l[i]+1;
        tree.reserve(2*leaf.size());
        root=build(0,leaf.size());
        fact.resize(n+3);invfact.resize(n+3);fact[0]=1;
        for(int i=1;i<n+3;i++)fact[i]=mulmod(fact[i-1],i);
        invfact[n+2]=power(fact[n+2],MOD-2);
        for(int i=n+2;i;i--)invfact[i-1]=mulmod(invfact[i],i);
    }
    int build(int a,int b){
        int id=tree.size();tree.emplace_back();
        tree[id].begin=4*a;tree[id].end=min(n,4*b);
        tree[id].type=(a==0?1:0)+(b==(int)leaf.size()?2:0);
        int count=tree[id].type==0?9:(tree[id].type==3?1:3);
        tree[id].v.resize(count*stride);
        if(b-a==1)leaf[a]=id;
        else {int m=(a+b)/2;int x=build(a,m),y=build(m,b);tree[id].left=x;tree[id].right=y;}
        return id;
    }
    inline V low(int i,int j){
        if(!slope[i])return constantv(offset[i]);
        __m128i x=_mm_add_epi32(_mm_set1_epi32(offset[i]),_mm_setr_epi32(j,j+1,j+2,j+3));
        x=_mm_sub_epi32(x,_mm_and_si128(_mm_cmpgt_epi32(x,_mm_set1_epi32(MOD-1)),_mm_set1_epi32(MOD)));
        return makev(x);
    }
    inline V high(int i,V lo){
        __m128i x=_mm_sub_epi32(_mm_set1_epi32(len[i]),lo.i);
        return makev(_mm_add_epi32(x,_mm_and_si128(_mm_srai_epi32(x,31),_mm_set1_epi32(MOD))));
    }
    void makeleaf(Node &u,int from,int to){
        int a=u.begin,b=u.end;
        for(int j=from;j<to;j+=4){
            V out[9];
            if(u.type==1 || u.type==3){
                V x=low(a,j),y=high(a,x),z=constantv(0);
                for(int i=a+1;i<b;i++){
                    V lo=low(i,j),hi=high(i,lo);
                    V nx=timesv(lo,plusv(x,z)),ny=timesv(hi,x),nz=timesv(lo,y);
                    x=nx;y=ny;z=nz;
                }
                if(u.type==3)out[0]=plusv(plusv(x,y),z);
                else {out[0]=x;out[1]=y;out[2]=z;}
            } else if(u.type==2){
                V lo=low(b-1,j);
                V x=constantv(len[b-1]),y=lo,z=lo;
                for(int i=b-2;i>=a;i--){
                    V lowi=low(i,j),hi=high(i,lowi);
                    V t=timesv(x,lowi),nx=plusv(t,timesv(y,hi)),ny=timesv(z,lowi);
                    x=nx;y=ny;z=t;
                }
                out[0]=x;out[1]=y;out[2]=z;
            } else {
                V la=low(a,j),ha=high(a,la),zero=constantv(0);
                if(b-a==1){out[0]=la;out[1]=zero;out[2]=la;out[3]=ha;out[4]=zero;out[5]=zero;out[6]=zero;out[7]=la;out[8]=zero;}
                else {
                    V lb=low(a+1,j),hb=high(a+1,lb);
                    V p=timesv(lb,la),q=timesv(hb,la),s=timesv(lb,ha);
                    out[0]=p;out[1]=p;out[2]=p;out[3]=q;out[4]=zero;out[5]=q;out[6]=s;out[7]=zero;out[8]=zero;
                    if(b-a>=3){
                        V lc=low(a+2,j),hc=high(a+2,lc);
                        V t=timesv(lc,plusv(p,s)),u2=timesv(lc,p),v=timesv(hc,p),w=timesv(lc,q);
                        out[0]=t;out[1]=u2;out[2]=u2;out[3]=v;out[4]=v;out[5]=v;out[6]=w;out[7]=zero;out[8]=w;
                        if(b-a==4){
                            V ld=low(a+3,j),hd=high(a+3,ld);
                            out[0]=timesv(ld,plusv(t,w));out[1]=timesv(ld,u2);out[2]=timesv(ld,plusv(u2,w));
                            out[3]=timesv(hd,t);out[4]=timesv(hd,u2);out[5]=out[4];
                            out[6]=timesv(ld,v);out[7]=out[6];out[8]=out[6];
                        }
                    }
                }
            }
            int count=u.type==0?9:(u.type==3?1:3);
            for(int k=0;k<count;k++)_mm_storeu_si128((__m128i*)&u.v[k*stride+j],out[k].i);
        }
    }
    void combine(Node &u,int from,int to){
        Node &a=tree[u.left],&b=tree[u.right];
        int ca=a.type==0?9:3,cb=b.type==0?9:3;
        for(int j=from;j<to;j+=4){
            V A[9],B[9];
            for(int k=0;k<ca;k++)A[k]=makev(_mm_loadu_si128((__m128i*)&a.v[k*stride+j]));
            for(int k=0;k<cb;k++)B[k]=makev(_mm_loadu_si128((__m128i*)&b.v[k*stride+j]));
            if(u.type==3){
                _mm_storeu_si128((__m128i*)&u.v[j],dot3(B[0],B[1],B[2],A[0],A[1],A[2]));
            }else if(u.type==1){
                for(int i=0;i<3;i++)_mm_storeu_si128((__m128i*)&u.v[i*stride+j],dot3(B[3*i],B[3*i+1],B[3*i+2],A[0],A[1],A[2]));
            }else if(u.type==2){
                for(int k=0;k<3;k++)_mm_storeu_si128((__m128i*)&u.v[k*stride+j],dot3(B[0],B[1],B[2],A[k],A[3+k],A[6+k]));
            }else{
                for(int i=0;i<3;i++)for(int k=0;k<3;k++)_mm_storeu_si128((__m128i*)&u.v[(3*i+k)*stride+j],dot3(B[3*i],B[3*i+1],B[3*i+2],A[k],A[3+k],A[6+k]));
            }
        }
    }
    void recompute(int id,int from,int to,bool all){
        Node &u=tree[id];if(!all&&!u.dirty)return;
        if(u.left==-1)makeleaf(u,from,to);
        else {recompute(u.left,from,to,all);recompute(u.right,from,to,all);combine(u,from,to);}
    }
    bool mark(int id,const vector<char>&changed){
        Node &u=tree[id];
        if(u.left==-1)u.dirty=changed[u.begin/4];
        else {bool a=mark(u.left,changed),b=mark(u.right,changed);u.dirty=a||b;}
        return u.dirty;
    }
    int cdf(int t){
        // x: ready for a high; y: just placed a high; z: one low after a high.
        // The transition matrix is [[lo,0,lo],[hi,0,0],[0,lo,0]].
        int x=1,y=0,z=0;
        for(int i=0;i<n;i++){
            int lo=max(0,min(len[i],t-l[i])),hi=len[i]-lo;
            int nx=mulmod(lo,addmod(x,z)),ny=mulmod(hi,x),nz=mulmod(lo,y);
            x=nx;y=ny;z=nz;
        }
        return addmod(addmod(x,y),z);
    }
    int interpolate(const vector<int>&h,int t){
        int d=(int)h.size()-1;if(t<=d)return h[t];
        vector<int> pre(d+2);pre[0]=1;
        for(int i=0;i<=d;i++)pre[i+1]=mulmod(pre[i],t-i);
        int suffix=1,ans=0;
        for(int i=d;i>=0;i--){
            int term=mulmod(mulmod(h[i],pre[i]),mulmod(suffix,mulmod(invfact[i],invfact[d-i])));
            if((d-i)&1){ans-=term;if(ans<0)ans+=MOD;}
            else ans=addmod(ans,term);
            suffix=mulmod(suffix,t-i);
        }
        return ans;
    }
    int run(){
        int lower=0,upper=0,total=1;
        for(int i=0;i<n;i++){
            total=mulmod(total,len[i]);
            for(int j=max(0,i-2);j<i;j++){
                lower=max(lower,min(l[i],l[j]));upper=max(upper,min(r[i],r[j]));
            }
        }
        if(lower==upper)return mulmod(total,lower);
        // F(t)=0 for t<=lower; no array can have value greater than upper.
        // The required sum is upper*total - sum_{t=lower+1}^{upper} F(t).
        if(upper-lower<=64){
            int ans=mulmod(total,upper);
            for(int t=lower+1;t<=upper;t++){ans-=cdf(t);if(ans<0)ans+=MOD;}
            return ans;
        }
        vector<pair<int,int>>events;
        vector<int>points={lower+1,upper+1};
        int degree=0;
        for(int i=0;i<n;i++){
            int t=lower+1;
            if(t<l[i])offset[i]=0,slope[i]=0;
            else if(t<=r[i])offset[i]=MOD-l[i],slope[i]=1,degree++;
            else offset[i]=len[i],slope[i]=0;
            if(l[i]>t&&l[i]<=upper){events.emplace_back(l[i],i);points.push_back(l[i]);}
            if(r[i]+1>t&&r[i]+1<=upper){events.emplace_back(r[i]+1,i);points.push_back(r[i]+1);}
        }
        sort(events.begin(),events.end());sort(points.begin(),points.end());points.erase(unique(points.begin(),points.end()),points.end());
        vector<char>changed(leaf.size(),true);
        int ei=0,answer=mulmod(total,upper);
        for(int pi=0;pi+1<(int)points.size();pi++){
            int a=points[pi],b=points[pi+1];
            while(ei<(int)events.size()&&events[ei].first==a){
                int i=events[ei++].second;
                if(a==l[i]){slope[i]=1;offset[i]=MOD-l[i];degree++;}
                else {slope[i]=0;offset[i]=len[i];degree--;}
                changed[i/4]=true;
            }
            int sum=0;
            if(b-a<=32){for(int t=a;t<b;t++)sum=addmod(sum,cdf(t));}
            else {
                int needed=(degree+4)&~3,common=min(needed,previous);
                mark(root,changed);
                if(common)recompute(root,0,common,false);
                if(needed>previous)recompute(root,previous,needed,true);
                previous=needed;fill(changed.begin(),changed.end(),false);
                vector<int>h(degree+2);h[0]=0;
                for(int j=0;j<=degree;j++)h[j+1]=addmod(h[j],tree[root].v[j]);
                sum=interpolate(h,b)-interpolate(h,a);if(sum<0)sum+=MOD;
            }
            answer-=sum;if(answer<0)answer+=MOD;
        }
        return answer;
    }
};
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int T;cin>>T;
    while(T--){int n;cin>>n;vector<int>l(n),r(n);for(int &x:l)cin>>x;for(int &x:r)cin>>x;
        Solver solver(n,move(l),move(r));cout<<solver.run()<<'\n';}
}
