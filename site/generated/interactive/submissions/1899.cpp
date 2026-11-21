#include <bits/stdc++.h>
using namespace std;

using ll = long long;
constexpr ll CAP = 1LL << 30;

ll ask(int i) {
    cout << "? 1 " << i << endl;
    ll x;
    if (!(cin >> x) || x == -1) exit(0);
    return x;
}

ll powdown(ll x) {
    return 1LL << (63 - __builtin_clzll(x));
}

ll total;
map<ll,int> obs; // mass coordinate -> number of elements preceding it

struct Info {
    bool feasible = true;
    ll lo[3]{}, hi[3]{}; // index ranges for the three internal cuts
    ll dmin = 0, dmax = 0; // length(left child) - length(right child)
    ll lb[2]{}, ub[2]{}; // bounds on the maximum in each child
};

// All elements are aligned dyadic intervals in mass coordinates.
// Impose three known boundaries s,m,t (possibly coincident).
Info inspect(ll s, ll m, ll t) {
    Info z;
    ll cut[] = {0,s,m,t,total};
    for (auto it = obs.begin(), jt = next(it); jt != obs.end(); ++it, ++jt) {
        ll p = it->first, q = jt->first, cnt = jt->second - it->second;
        ll low[4]{}, high[4]{}, largest[4]{};
        ll pieces[4][100];
        int num[4]{};
        ll sumlow = 0, sumhigh = 0;
        for (int j=0;j<4;++j) {
            ll a=max(p,cut[j]), b=min(q,cut[j+1]);
            if (b<=a) continue;
            high[j]=b-a;
            // Maximal aligned dyadic pieces. A piece bigger than CAP
            // stands for multiple CAP-sized pieces.
            while (a<b) {
                ll v=powdown(b-a);
                if (a) v=min(v,a&-a);
                pieces[j][num[j]++]=v;
                low[j]+=max(1LL,v/CAP);
                largest[j]=max(largest[j],min(v,CAP));
                a+=v;
            }
            sumlow+=low[j];
            sumhigh+=high[j];
        }
        if (sumlow>cnt || sumhigh<cnt) {
            z.feasible=false;
            return z;
        }
        ll pl=0, ph=0;
        for (int j=0;j<3;++j) {
            pl+=low[j]; ph+=high[j];
            z.lo[j]+=max(pl,cnt-(sumhigh-ph));
            z.hi[j]+=min(ph,cnt-(sumlow-pl));
        }
        ll extra=cnt-sumlow;
        ll A=high[1]-low[1], B=high[2]-low[2];
        ll other=high[0]-low[0]+high[3]-low[3];
        z.dmin+=low[1]-low[2]-min(B,extra)+max(0LL,extra-B-other);
        z.dmax+=low[1]-low[2]+min(A,extra)-max(0LL,extra-A-other);

        for (int j=1;j<=2;++j) if (high[j]) {
            ll few=max(low[j],cnt-(sumhigh-high[j]));
            ll many=min(high[j],cnt-(sumlow-low[j]));
            ll u=min(largest[j],powdown(high[j]-few+1));
            z.ub[j-1]=max(z.ub[j-1],u);
            // Least possible maximum: subdivide all canonical pieces
            // to the trial cap and test the required number of leaves.
            int l=0,r=__builtin_ctzll(u);
            while (l<r) {
                int h=(l+r)/2;
                ll need=0;
                for (int k=0;k<num[j];++k)
                    need+=max(1LL,pieces[j][k]>>h);
                if (need<=many) r=h;
                else l=h+1;
            }
            z.lb[j-1]=max(z.lb[j-1],1LL<<l);
        }
    }
    return z;
}

ll solve(int n) {
    if (n==1) return ask(1);
    // The last element is at most half the total. Thus the first
    // query gives both the power-of-two total and one internal cut.
    ll p=ask(n-1);
    total=2*powdown(p);
    obs[0]=0;
    obs[p]=n-1;
    obs[total]=n;
    ll s=0,t=total,best=1;
    while (true) {
        Info whole=inspect(s,t,t);
        best=max(best,whole.lb[0]);
        if (whole.ub[0]<=best) return best;
        ll m=(s+t)/2;
        Info h=inspect(s,m,t);
        if (!h.feasible) return t-s; // the active node is a single leaf
        bool maybeLeaf=(whole.ub[0]==t-s);
        int probe=1;
        ll i=(h.lo[1]+h.hi[1])/2;
        if (!maybeLeaf) {
            best=max({best,h.lb[0],h.lb[1]});
            if (max(h.ub[0],h.ub[1])<=best) return best;

            // At the creation of a concatenation the two halves were
            // obtained from identical arrays by maximal-element splits.
            // Later splits restricted to a half also split its maximum.
            // Hence the shorter half has the greater (or equal) maximum:
            // the multiset after a fixed number of splits is unique.
            // A half descended from a singleton split has the same
            // property. This argument applies recursively to either half.
            bool left=h.dmax<=0 || h.ub[1]<=best;
            bool right=h.dmin>=0 || h.ub[0]<=best;
            if (left || right) {
                if (left && right) {
                    if (h.ub[0]<=best && h.ub[1]<=best) return best;
                    if (h.ub[0]<=best) left=false;
                    else if (h.ub[1]>best &&
                             h.hi[2]-h.lo[2]<h.hi[0]-h.lo[0]) left=false;
                }
                if (left) t=m;
                else s=m;
                continue;
            }

            // Refine only a boundary whose uncertainty affects the
            // comparison; exact endpoints are not needed to descend.
            ll width=-1;
            for (int j=0;j<3;++j) {
                ll w=(h.hi[j]-h.lo[j])*(j==1?2:1);
                if (w>width) width=w,probe=j;
            }
            ll a=h.lo[probe], b=h.hi[probe];
            i=(a+b)/2;
            if (probe==1) {
                ll pivot=(h.lo[0]+h.hi[0]+h.lo[2]+h.hi[2])/4;
                ll margin=(h.lo[0]==h.hi[0] && h.lo[2]==h.hi[2])
                           ? 0 : (b-a)/8;
                i=clamp(pivot,a+margin,b-margin);
            }
        }
        ll mass=ask((int)i);
        obs[mass]=(int)i;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin>>n) || n<1) return 0;
    ll ans=solve(n);
    cout<<"! "<<ans<<endl;
    return 0;
}
