#include <bits/stdc++.h>
using namespace std;
using u64 = unsigned long long;

// The path count is the central trinomial coefficient [x^0](x+1+x^-1)^n.
// Over F_11, Frobenius shows that it is the product of the coefficients
// for the base-11 digits of n. All eleven digit values are nonzero.
// Taking logarithms to primitive root 2 turns prefix products into sums
// modulo 10; we calculate these sums separately modulo 2 and modulo 5.

// Sum binom(A - m*t, k), 0 <= t < L, modulo the small prime P.
// Lucas' theorem makes A periodic modulo the least power q of P > k.
template<int P> struct BinomialProgression {
    static constexpr int MAXD = 32;
    int q[MAXD], kk[MAXD], step[MAXD], base[MAXD];
    vector<int8_t> memo[MAXD];
    int choose[P][P]{};
    int levels, firstMemo;

    BinomialProgression() {
        for (int i=0; i<P; ++i) {
            choose[i][0]=choose[i][i]=1;
            for (int j=1; j<i; ++j)
                choose[i][j]=(choose[i-1][j-1]+choose[i-1][j])%P;
        }
    }

    void init(int k, u64 m, u64 minLength, int roots) {
        int period=1;
        levels=0;
        while (period<=k) { period*=P; ++levels; }
        q[0]=period; kk[0]=k; step[0]=int(m%period);
        base[0]=int(minLength%period);
        u64 branches=roots;
        firstMemo=levels;
        // At depth d, lengths have only two possible values, and A has
        // at most q/P^d values. Before memoization there are at most
        // roots*P^d calls. Switching near their intersection bounds the
        // work and space by O(sqrt(roots*q)), with P a constant.
        for (int d=0; d<levels; ++d) {
            if (firstMemo==levels && u64(q[d])*2<=branches) firstMemo=d;
            if (d>=firstMemo) memo[d].assign(size_t(q[d])*2, int8_t(-1));
            else memo[d].clear();
            q[d+1]=q[d]/P;
            kk[d+1]=kk[d]/P;
            step[d+1]=step[d]%q[d+1];
            base[d+1]=base[d]/P;
            branches*=P;
        }
    }

    int calc(int d, int a, int len) {
        if (!len) return 0;
        if (!kk[d]) return len%P;
        int index=0;
        if (d>=firstMemo) {
            index=a*2+(len>base[d]);
            int old=memo[d][index];
            if (old>=0) return old;
        }
        const int digit=kk[d]%P;
        const int whole=len/P, rem=len%P;
        int result=0, cur=a;
        for (int r=0; r<P && r<len; ++r) {
            int factor=choose[cur%P][digit];
            if (factor) result+=factor*calc(d+1,cur/P,whole+(r<rem));
            cur-=step[d];
            if (cur<0) cur+=q[d];
        }
        result%=P;
        if (d>=firstMemo) memo[d][index]=int8_t(result);
        return result;
    }

    int sum(u64 a, u64 length) {
        // A complete period sums to binom(q,k+1), which is nonzero
        // only when k=q-1.
        int result=(kk[0]==q[0]-1) ? int((length/q[0])%P) : 0;
        result+=calc(0,int(a%q[0]),int(length%q[0]));
        return result%P;
    }
};

template<int P> int exponentMod(u64 n, int c) {
    // Discrete logarithms to base 2 of the central trinomial numbers
    // for digits 0,...,10, reduced modulo 10.
    static constexpr int diff[12]={0,0,8,9,6,4,9,7,4,5,3,5};
    BinomialProgression<P> solver;
    int answer=0;
    // If D(y) is the digit-log polynomial, the contribution of digit
    // position b=11^j to the generating function is
    // D(x^b)(1-x^b) / ((1-x)(1-x^(11b))).
    // After c prefix sums, its shifted coefficients are sums of
    // binom(n-shift-11b*t+c,c). 'diff' is D(y)(1-y).
    for (u64 b=1; b<=n/2; ) {
        const u64 m=b*11;
        int roots=0;
        for (int d=2; d<=11 && u64(d)<=n/b; ++d)
            if (diff[d]%P) ++roots;
        if (roots) {
            solver.init(c,m,n/m,roots);
            for (int d=2; d<=11 && u64(d)<=n/b; ++d) {
                int coefficient=diff[d]%P;
                if (!coefficient) continue;
                u64 offset=n-u64(d)*b;
                answer+=coefficient*solver.sum(offset+u64(c),offset/m+1);
            }
            answer%=P;
        }
        if (b>n/22) break;
        b*=11;
    }
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    cin>>T;
    constexpr int power[10]={1,2,4,8,5,10,9,7,3,6};
    while (T--) {
        u64 n;
        int c;
        cin>>n>>c;
        int a=exponentMod<2>(n,c);
        int b=exponentMod<5>(n,c);
        int exponent=b+5*((a-b%2+2)%2);
        cout<<power[exponent]<<'\n';
    }
    return 0;
}
