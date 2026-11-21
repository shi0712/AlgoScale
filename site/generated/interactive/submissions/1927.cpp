#include <bits/stdc++.h>
using namespace std;

using I = long long;
constexpr I LIMIT = 100000000000000LL;
constexpr int D = 64, S = 12;

I cnt[D + 1][D + 1], pref[D + 1][D + 1];

// Model a local search in which every query costs or earns one credit.
// Splitting a leaf at depth h increases the external path length by h+2,
// or by h+4 if a recharge is necessary first. These marginal costs are
// nondecreasing along each path, so the cheapest n-1 splits are optimal.
void init() {
    for (int d = 0; d <= D; ++d) {
        for (int k = 0; k <= D; ++k) {
            I v = (d == (k ? 2 : 4));
            if (k && d >= 1)
                v += cnt[k-1][d-1] + cnt[min(D,k+1)][d-1];
            if (!k && d >= 2)
                v += cnt[0][d-2] + cnt[2][d-2];
            cnt[k][d] = min(v, LIMIT);
        }
    }
    for (int k = 0; k <= D; ++k) {
        I total = 0, sum = 0;
        for (int d = 0; d <= D; ++d) {
            I take = min(cnt[k][d], LIMIT-total);
            total += take;
            sum += take*d;
            cnt[k][d] = total;
            pref[k][d] = sum;
        }
    }
}

I model(I n, int k) {
    if (n <= 1) return 0;
    I splits = n-1;
    int d = lower_bound(cnt[k], cnt[k]+D+1, splits) - cnt[k];
    return pref[k][d-1] + (splits-cnt[k][d-1])*d;
}

I estimate(I l, I r, I b) {
    I n = r-l+1;
    if (n <= 1) return 0;
    I ans = 0;
    for (int j = 0; j < S; ++j) {
        I denom = 2*S*l + (2*j+1)*(r-l);
        int k = int(min<I>(D, (2*S*b)/denom));
        ans += model(n,k);
    }
    return ans;
}

map<tuple<I,I,I>,pair<I,I>> memo;

// Exact minimum total remaining path length on short intervals.
// Before a chosen split, any necessary certain gains are best made at l.
pair<I,I> exact(I l, I r, I b) {
    I n = r-l+1;
    if (n == 1) return {0,0};
    int h = bit_width((unsigned long long)(n-1));
    if (b >= r*h)
        return {n*(h+1)-(1LL<<h), l+n/2};
    auto key = make_tuple(l,r,b);
    auto it = memo.find(key);
    if (it != memo.end()) return it->second;
    I best = LLONG_MAX, op = l;
    for (I q=l+1; q<=r; ++q) {
        I t = b>=q ? 0 : (q-b+l-1)/l;
        I cash = b+t*l;
        I cost = n*(1+t) + exact(l,q-1,cash-q).first
                           + exact(q,r,cash+q).first;
        if (cost<best) {
            best=cost;
            op=t ? l : q;
        }
    }
    return memo[key]={best,op};
}

I choose(I l, I r, I b) {
    if (!l) return 1;
    if (r > 4*l) return min(r,b);
    if (b <= l) return l;
    I n = r-l+1;
    if (n <= 16) return exact(l,r,b).second;

    I bestQ=l+1, bestOp=l;
    pair<I,long double> best={LLONG_MAX,0};
    auto test = [&](I q) {
        if (q<=l || q>r) return;
        I t = b>=q ? 0 : (q-b+l-1)/l;
        I cash=b+t*l;
        I a=estimate(l,q-1,cash-q), c=estimate(q,r,cash+q);
        pair<I,long double> score={
            n*S*(1+t)+a+c,
            max((long double)a/(q-l),(long double)c/(r-q+1))
        };
        if (score<best) {
            best=score;
            bestQ=q;
            bestOp=t ? l : q;
        }
    };
    I step=max(1LL,n/128);
    for (I q=l+1; q<=r; q+=step) test(q);
    test(r);
    for (int j=0;j<=4;++j) {
        test(b+j*l);
        test(b+j*l+1);
    }
    while (step>1) {
        I center=bestQ, old=step;
        step=max(1LL,step/8);
        for (I q=max(l+1,center-old);q<=min(r,center+old);q+=step)
            test(q);
    }
    return bestOp;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    init();
    I l=0,r=LIMIT,b=1;
    while (l<r) {
        I x=choose(l,r,b);
        cout << "? " << x << endl;
        string s;
        if (!(cin>>s)) return 0;
        if (s=="Lucky!") {
            l=max(l,x);
            b+=x;
        } else if (s=="Fraudster!") {
            r=min(r,x-1);
            b-=x;
        } else return 0;
    }
    cout << "! " << l << endl;
    return 0;
}
