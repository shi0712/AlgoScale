#include <bits/stdc++.h>
using namespace std;
using u64 = unsigned long long;
using i128 = __int128_t;
using u128 = __uint128_t;
using ld = long double;

u64 n, k, a, b;
ld harmonicTable[256];

ld harmonic(u64 x) {
    if (x < 256) return harmonicTable[x];
    ld z = 1.0L / x, z2 = z*z;
    return logl((ld)x) + 0.577215664901532860606512090082402431L
        + z/2 - z2/12 + z2*z2/120 - z2*z2*z2/252;
}
ld harmonicDiff(u64 l, u64 r) {
    if (l == r) return 0;
    if (l < 256) return harmonic(r) - harmonicTable[l];
    ld x = 1.0L/l, y = 1.0L/r;
    ld x2=x*x, y2=y*y;
    return log1pl((ld)(r-l)/l) + (y-x)/2 - (y2-x2)/12
        + (y2*y2-x2*x2)/120 - (y2*y2*y2-x2*x2*x2)/252;
}

// Sum over d of clamp(x/(2*a*d) - d/2 - b/a - 1/2, 0, n-d).
ld model(ld x) {
    ld A=a, B=(ld)a+2*(ld)b;
    ld rd=2*x/(B+sqrtl(B*B+4*A*x));
    u64 d=min(n-1, (u64)max(0.0L,rd));
    if (!d) return 0;
    ld C=(2*(ld)n+1)*A+2*(ld)b;
    u64 t;
    if (x >= (ld)(n-1)*(C-A*(n-1))) t=n-1;
    else {
        ld rt=2*x/(C+sqrtl(max(0.0L,C*C-4*A*x)));
        t=min(d,(u64)max(0.0L,rt));
    }
    u64 m=d-t;
    return (ld)t*n-(ld)t*(t+1)/2 + x/(2*A)*harmonicDiff(t,d)
        -(ld)m*(d+t+1)/4 - ((ld)b/A+0.5L)*m;
}

ld invertModel(ld target, ld low, ld high) {
    for (int it=0; it<80; ++it) {
        ld mid=(low+high)/2;
        if (mid==low || mid==high) break;
        if (model(mid)<target) low=mid;
        else high=mid;
    }
    return (low+high)/2;
}

u128 first(u64 d) { return (u128)d*((u128)a*(d+2)+2*b); }
u128 last(u64 d) { return (u128)d*((u128)a*(2*n-d)+2*b); }

struct Result {
    u64 count, prevGap, nextGap, prevFreq, nextFreq;
    u64 partialBegin, partialEnd;
};
Result countAt(u128 x) {
    ld X=(ld)x, A=a, B=2*(ld)(a+b);
    u64 d=min(n-1,(u64)(2*X/(B+sqrtl(B*B+4*A*X))));
    while(d<n-1 && first(d+1)<=x) ++d;
    while(d && first(d)>x) --d;
    u64 t;
    ld C=2*(ld)a*n+2*(ld)b;
    if (last(n-1)<=x) t=n-1;
    else {
        t=min(d,(u64)(2*X/(C+sqrtl(max(0.0L,C*C-4*A*X)))));
        while(t<d && last(t+1)<=x) ++t;
        while(t && last(t)>x) --t;
    }
    Result res{};
    res.count=t*n-t*(t+1)/2;
    res.prevGap=res.nextGap=ULLONG_MAX;
    if(t) {res.prevGap=(u64)min(x-last(t),(u128)ULLONG_MAX); res.prevFreq=1;}
    if(d<n-1) {res.nextGap=(u64)min(first(d+1)-x,(u128)ULLONG_MAX); res.nextFreq=1;}
    res.partialBegin=t; res.partialEnd=d;
    double xd=(double)x, beta=(double)b/a;
    u64 step=2*a*(t+1);
    u128 base=(u128)(t+1)*((u128)a*(t+1)+2*b);
    u64 baseIncrement=a*(2*(t+1)+1)+2*b;
    for(u64 j=t+1;j<=d;++j) {
        // The floating estimate is within 1; all decisions are corrected exactly.
        u64 q=(u64)(xd/(double)step-0.5*(double)j-beta);
        i128 rem=(i128)(x-base)-(i128)((u128)step*q);
        if(rem<0) {--q;rem+=step;}
        else if((u128)rem>=step) {++q;rem-=step;}
#ifdef LOCAL
        assert(q>=1 && q<n-j && rem>=0 && (u128)rem<step);
#endif
        res.count+=q;
        u64 r=(u64)rem;
        if(r<res.prevGap) {res.prevGap=r;res.prevFreq=1;}
        else if(r==res.prevGap) ++res.prevFreq;
        u64 s=step-r;
        if(s<res.nextGap) {res.nextGap=s;res.nextFreq=1;}
        else if(s==res.nextGap) ++res.nextFreq;
        base+=baseIncrement;
        baseIncrement+=2*a;
        step+=2*a;
    }
    return res;
}

void print128(u128 x) {
    if(x>=10) print128(x/10);
    cout << char('0'+x%10);
}
void output(u128 x) {
    u64 g=gcd((u64)(x%a),a);
    print128(x/g);
    cout << ' ' << a/g << '\n';
}
int main() {
    ios::sync_with_stdio(false);cin.tie(nullptr);
    cin>>n>>k>>a>>b;
    if(k==1) {output(first(1));return 0;}
    if((u128)n*(n-1)/2==k) {output(last(n-1));return 0;}
    for(int i=1;i<256;++i) harmonicTable[i]=harmonicTable[i-1]+1.0L/i;
    u64 m=(u64)sqrtl(2*(ld)k)+1;
    while((u128)m*(m-1)/2<k) ++m;
    while(m>2 && (u128)(m-1)*(m-2)/2>=k) --m;
    // The first m indices already supply at least k pairs.
    u128 low=0, high=first(m-1);
    u64 lowCount=0, highCount=0;
    ld target=k;
    u128 x=(u128)invertModel(target,0,(ld)high);
    for(int it=0;;++it) {
        x=max(low+1,min(x,high));
        Result r=countAt(x);
#ifdef LOCAL
        cerr << "it=" << it << " count=" << r.count << " rows=" << r.partialEnd-r.partialBegin << '\n';
#endif
        if(r.count>=k) {
            if(r.count-r.prevFreq<k) {output(x-r.prevGap);return 0;}
            high=x-r.prevGap-1;
            highCount=r.count-r.prevFreq;
        } else {
            if(r.count+r.nextFreq>=k) {output(x+r.nextGap);return 0;}
            low=x+r.nextGap;
            lowCount=r.count+r.nextFreq;
        }
        if(low+1==high) {output(high);return 0;}
        // After a bounded number of interpolation steps, use binary search.
        // Thus the worst-case number of exact counts remains logarithmic.
        if (it>=12) {
            x=low+(high-low+1)/2;
        } else if(lowCount && highCount) {
            u64 rankSpan=highCount-lowCount;
            x=low+((high-low)*(k-lowCount)+rankSpan-1)/rankSpan;
        } else {
            // Correct the smooth approximation by its exact error here.
            target=model((ld)x)+(ld)k-(ld)r.count;
            u128 next=(u128)max(0.0L,invertModel(target,(ld)low,(ld)high));
            x=max(low+1,min(next,high));
        }
    }
}
