#include <bits/stdc++.h>
using namespace std;

/*
Let M=n(n+1)/2 and D=M-1. Give every element a real coordinate
in [0,1], in the order in which it occurs. After integrating out the
pairs, fixing singleton k+1 at x gives

 f_k(x) = integral_{0<t_1<...<t_k<x<t_{k+1}<...<t_N<1}
             |Vandermonde(t_1,...,t_N,x)| dt,    N=n-1.

Thus f_k(x) = sum_j answer[k][j+1] x^j(1-x)^(D-j)/(j!(D-j)!).
We compute Q_k(z)=(1+z)^D f_k(z/(1+z)), so its coefficients give
all answers immediately.

The Selberg integration-by-parts recurrence supplies an n-dimensional
space containing these polynomials. For auxiliary integrals J_p with
an additional normalized elementary symmetric polynomial of degree p
in (x-t_i), put K_p(z)=(1+z)^(D+p)J_p(z/(1+z)). Then

 C_p K_{p+1} = z(1+z)K_p' + [B_p+(A_p+B_p-D-p)z]K_p
                - E_p z K_{p-1},
 B_p=-(N-p)(N-p+3)/2, A_p=-2B_p,
 C_p=(N-p)(2N+4-p)/2, E_p=p(N-p+4)/2.

For a fixed coefficient r, this is bidiagonal in p. It has a free
coefficient precisely at r=e(k)=k(k+3)/2. Construct a basis whose
K_0 coefficients at these n indices form the identity matrix.

The desired Q_k has lowest degree e(k) and highest degree D-e(N-k).
Eliminating the highest degrees from this basis, in descending k,
therefore recovers Q_k up to its lowest coefficient. That coefficient
is a product of two ordinary Selberg integrals, evaluated below.

Time O(n^4), memory O(n^3), including the output polynomials.
*/

class Solver {
    int n, N, D, mod;
    vector<int> fact, ifact, inv, gammaValue, invGamma;

    int mul(int a, int b) const { return (long long)a*b%mod; }
    int power(int a, int b) const {
        int ans=1;
        for (; b; b>>=1, a=mul(a,a)) if (b&1) ans=mul(ans,a);
        return ans;
    }
    static int e(int k) { return k*(k+3)/2; }

    // Ordered Selberg integral with weight x^(a-1)(1-x)^(b-1), beta=1.
    int selberg(int r, int a, int b) const {
        int ans=ifact[r];
        for (int j=0;j<r;++j) {
            ans=mul(ans,gammaValue[2*a+j]);
            ans=mul(ans,gammaValue[2*b+j]);
            ans=mul(ans,gammaValue[j+3]);
            ans=mul(ans,invGamma[2*a+2*b+r+j-1]);
            ans=mul(ans,invGamma[3]);
        }
        return ans;
    }

public:
    void run() {
        cin>>n>>mod;
        N=n-1; D=n*(n+1)/2-1;
        int lim=max(D,4*n+10);
        fact.resize(lim+1); ifact.resize(lim+1); inv.resize(lim+1);
        fact[0]=1;
        for (int i=1;i<=lim;++i) fact[i]=mul(fact[i-1],i);
        ifact[lim]=power(fact[lim],mod-2);
        for (int i=lim;i>0;--i) ifact[i-1]=mul(ifact[i],i);
        for (int i=1;i<=lim;++i) inv[i]=mul(fact[i-1],ifact[i]);
        int half=(mod+1)/2;
        gammaValue.resize(2*n+10); invGamma.resize(2*n+10);
        int inv4=mul(half,half), inv4power=1;
        for (int s=0;2*s+1<(int)gammaValue.size();++s) {
            gammaValue[2*s+1]=mul(mul(fact[2*s],ifact[s]),inv4power);
            inv4power=mul(inv4power,inv4);
        }
        for (int s=1;2*s<(int)gammaValue.size();++s)
            gammaValue[2*s]=fact[s-1];
        // Odd indices omit a factor sqrt(pi), which cancels in Selberg's formula.
        for (int i=1;i<(int)gammaValue.size();++i)
            invGamma[i]=power(gammaValue[i],mod-2);

        vector<int> B(n), C(n), E(n), invC(n), resonance(D+1,-1);
        for (int p=0;p<n;++p) {
            B[p]=-e(N-p);
            C[p]=mul((N-p)*(2*N+4-p),half);
            E[p]=mul(p*(N-p+4),half);
            if (C[p]) invC[p]=power(C[p],mod-2);
            resonance[e(p)]=p;
        }
        vector<vector<int>> poly(n,vector<int>(D+1));
        vector<int> prev(n*n), now(n*n);
        int active=0;
        for (int r=0;r<=D;++r) {
            int k=resonance[r];
            if (k>=0) active=k+1;
            int singular=(k<0 ? -1 : N-k);
            for (int p=N;p>singular;--p) {
                int denominator=r+B[p];
                int inverse=denominator>0 ? inv[denominator] : mod-inv[-denominator];
                long long h=r-1-B[p]-D-p;
                int *dst=now.data()+p*n;
                const int *old=prev.data()+p*n;
                const int *oldAbove=p ? old-n : nullptr;
                const int *newBelow=p<N ? dst+n : nullptr;
                for (int b=0;b<active;++b) {
                    long long value=-h*old[b];
                    if (p<N) value+=(long long)C[p]*newBelow[b];
                    if (p) value+=(long long)E[p]*oldAbove[b];
                    value%=mod;
                    if (value<0) value+=mod;
                    dst[b]=value*inverse%mod;
                }
            }
            if (k>=0) {
                for (int b=0;b<active;++b) now[b]=(b==k);
                for (int p=0;p<singular;++p) {
                    long long h=r-1-B[p]-D-p;
                    int diag=r+B[p];
                    const int *cur=now.data()+p*n;
                    const int *old=prev.data()+p*n;
                    const int *oldAbove=p ? old-n : nullptr;
                    int *dst=now.data()+(p+1)*n;
                    for (int b=0;b<active;++b) {
                        long long value=(long long)diag*cur[b]+h*old[b];
                        if (p) value-=(long long)E[p]*oldAbove[b];
                        value%=mod;
                        if (value<0) value+=mod;
                        dst[b]=value*invC[p]%mod;
                    }
                }
            }
            for (int b=0;b<active;++b) poly[b][r]=now[b];
            prev.swap(now);
        }

        vector<int> degree(n), invPivot(n);
        for (int k=N;k>=0;--k) {
            degree[k]=D-e(N-k);
            for (int l=N;l>k;--l) {
                int factor=mul(poly[k][degree[l]],invPivot[l]);
                if (!factor) continue;
                for (int r=e(l);r<=degree[l];++r) {
                    int value=poly[k][r]-mul(factor,poly[l][r]);
                    if (value<0) value+=mod;
                    poly[k][r]=value;
                }
            }
            invPivot[k]=power(poly[k][degree[k]],mod-2);
        }

        vector<int> factorialProduct(D+1);
        for (int r=0;r<=D;++r) factorialProduct[r]=mul(fact[r],fact[D-r]);
        string output;
        output.reserve((size_t)n*(D+1)*10);
        for (int k=0;k<n;++k) {
            int scale=mul(selberg(k,1,2),selberg(N-k,k+2,1));
            for (int r=0;r<=D;++r) {
                int value=0;
                if (r>=e(k) && r<=degree[k])
                    value=mul(mul(poly[k][r],scale),factorialProduct[r]);
                char buf[16];
                auto result=to_chars(buf,buf+16,value);
                output.append(buf,result.ptr);
                output.push_back(r==D ? '\n' : ' ');
            }
        }
        cout<<output;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver().run();
}
