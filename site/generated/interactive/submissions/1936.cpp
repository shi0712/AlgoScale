#include <bits/stdc++.h>
using namespace std;

using I = long long;
using U = unsigned long long;
using W = __uint128_t;

static mt19937_64 rng(827135924);
static I n;

I ask(I l, I r) {
    cout << "? " << l << ' ' << r << endl;
    I x;
    if (!(cin >> x) || x < 0) exit(0);
    return x;
}

I tri(I x) { return x * (x - 1) / 2; }

struct Hyp {
    I s, a, b;
    I eval(I l, I r) const {
        I j = s + a, k = j + b - 1;
        return tri(max(0LL, min(j - 1, r) - max(s, l) + 1)) +
               tri(max(0LL, min(k, r) - max(j, l) + 1));
    }
};

using Q = pair<I,I>;
Q witness(Hyp x, Hyp y) {
    if (x.s != y.s) return {1, min(x.s, y.s) + 1};
    if (x.a != y.a) return {1, x.s + min(x.a, y.a)};
    return {1, x.s + x.a + min(x.b, y.b)};
}

U mul(U a, U b, U m) { return W(a) * b % m; }
U power(U a, U b, U m) {
    U v = 1;
    for (; b; b >>= 1, a = mul(a, a, m))
        if (b & 1) v = mul(v, a, m);
    return v;
}
bool prime(U x) {
    if (x < 2) return false;
    for (U p : {2,3,5,7,11,13,17,19,23,29,31,37})
        if (x % p == 0) return x == p;
    U d = x - 1;
    int s = 0;
    while (!(d & 1)) d >>= 1, ++s;
    for (U a : {2ULL,325ULL,9375ULL,28178ULL,450775ULL,9780504ULL,1795265022ULL}) {
        if (a % x == 0) continue;
        U t = power(a % x, d, x);
        if (t == 1 || t == x - 1) continue;
        bool ok = false;
        for (int h = 1; h < s; ++h)
            if ((t = mul(t,t,x)) == x - 1) { ok = true; break; }
        if (!ok) return false;
    }
    return true;
}
U divisor(U x) {
    if (!(x & 1)) return 2;
    for (;;) {
        U c = 1 + rng() % (x - 1), y = rng() % x;
        auto f = [&](U v) { return (mul(v,v,x) + c) % x; };
        U g = 1, z = 0, saved = 0;
        for (U len = 1; g == 1; len *= 2) {
            z = y;
            for (U i = 0; i < len; ++i) y = f(y);
            for (U i = 0; i < len && g == 1; i += 64) {
                saved = y;
                U prod = 1;
                for (U h = 0; h < min(64ULL,len-i); ++h) {
                    y = f(y);
                    prod = mul(prod, z > y ? z-y : y-z, x);
                }
                g = gcd(prod, x);
            }
        }
        if (g == x) {
            do {
                saved = f(saved);
                g = gcd(z > saved ? z-saved : saved-z, x);
            } while (g == 1);
        }
        if (g != x) return g;
    }
}
void factor(U x, map<U,int>& fs) {
    if (x == 1) return;
    if (prime(x)) { ++fs[x]; return; }
    U d = divisor(x);
    factor(d, fs); factor(x/d, fs);
}

struct Gaussian {
    I x, y;
    Gaussian operator*(Gaussian v) const {
        return {x*v.x-y*v.y, x*v.y+y*v.x};
    }
};
I isqrt(I x) {
    I y = sqrtl(x);
    while ((y+1)*(y+1) <= x) ++y;
    while (y*y > x) --y;
    return y;
}

struct Group { I a,b,lo,hi; };

// 4*total+1 = (a+b-1)^2 + (a-b)^2.
// Factor in the Gaussian integers to enumerate the possible lengths.
vector<Group> lengths(I total) {
    map<U,int> fs;
    factor(4*total+1,fs);
    vector<Gaussian> reps{{1,0}};
    for (auto [p,e] : fs) {
        if (p % 4 == 3) {
            I scale = 1;
            for (int h=0; h<e/2; ++h) scale *= p;
            for (auto &z: reps) z = z * Gaussian{scale,0};
        } else {
            U t = 2;
            while (power(t,(p-1)/2,p) != p-1) ++t;
            U v = power(t,(p-1)/4,p), u = p;
            while (W(v)*v > p) { U w=u%v; u=v; v=w; }
            Gaussian z{(I)v,isqrt(p-v*v)};
            vector<Gaussian> f(e+1,{1,0}), g=f, out;
            for (int h=1; h<=e; ++h) {
                f[h]=f[h-1]*z;
                g[h]=g[h-1]*Gaussian{z.x,-z.y};
            }
            for (int h=0; h<=e; ++h) {
                auto w=f[h]*g[e-h];
                for (auto q: reps) out.push_back(q*w);
            }
            reps.swap(out);
        }
    }
    set<Q> pairs;
    for (auto z: reps) {
        I x=max(abs(z.x),abs(z.y)), y=min(abs(z.x),abs(z.y));
        I a=(x+1+y)/2, b=x+1-a;
        if (b>=2 && a+b<=n && tri(a)+tri(b)==total) {
            pairs.emplace(a,b); pairs.emplace(b,a);
        }
    }
    vector<Group> out;
    for (auto [a,b]: pairs) out.push_back({a,b,1,n-a-b+1});
    return out;
}

// On each piece both overlap lengths are affine in the start position.
// The inversion count there is a convex quadratic (possibly constant).
vector<Group> filter(const vector<Group>& gs, Q q, I ans) {
    vector<Group> out;
    auto [l,r]=q;
    for (auto [a,b,lo,hi]: gs) {
        auto val=[&](I s){return Hyp{s,a,b}.eval(l,r);};
        auto emit=[&](I x,I y){
            if (x>y) return;
            if (!out.empty() && out.back().a==a && out.back().b==b &&
                out.back().hi+1==x) out.back().hi=y;
            else out.push_back({a,b,x,y});
        };
        if (lo==hi) {
            if (val(lo)==ans) emit(lo,hi);
            continue;
        }
        vector<I> cuts{lo,hi+1};
        for (auto [u,v]: {Q{0,a-1},Q{a,a+b-1}})
            for (I c: {l-v,r-u+1,r-v+1,l-u+1})
                if (lo<c && c<=hi) cuts.push_back(c);
        sort(cuts.begin(),cuts.end());
        auto mono=[&](I x,I y,bool inc){
            if (x>y) return;
            auto f=[&](I s){return inc?val(s):-val(s);};
            I want=inc?ans:-ans;
            if (want<f(x) || want>f(y)) return;
            I L=x,R=y;
            while (L<R) {
                I m=(L+R)/2;
                if (f(m)<want) L=m+1; else R=m;
            }
            if (f(L)!=want) return;
            I first=L; R=y;
            while (L<R) {
                I m=(L+R+1)/2;
                if (f(m)>want) R=m-1; else L=m;
            }
            emit(first,L);
        };
        for (int h=1; h<(int)cuts.size(); ++h) {
            I x=cuts[h-1], y=cuts[h]-1;
            if (x>y) continue;
            if (x==y || val(x)<=val(x+1)) mono(x,y,true);
            else if (val(y)<=val(y-1)) mono(x,y,false);
            else {
                I L=x,R=y;
                while (L<R) {
                    I m=(L+R)/2;
                    if (val(m)<=val(m+1)) R=m; else L=m+1;
                }
                mono(x,L,false); mono(L+1,y,true);
            }
        }
    }
    return out;
}

Q choose(const vector<Hyp>& h) {
    int m=h.size();
    Q best=witness(h.front(),h.back());
    if (m==2) return best;
    vector<Q> qs{best};
    if (n<=65) {
        for (I l=1;l<n;++l) for(I r=l+1;r<=n;++r) qs.emplace_back(l,r);
    } else {
        vector<I> pts{1,n};
        for (auto [s,a,b]: h)
            for (I p: {s,s+a,s+a+b-1})
                for (I d: {-1,0,1}) pts.push_back(clamp(p+d,1LL,n));
        sort(pts.begin(),pts.end());
        pts.erase(unique(pts.begin(),pts.end()),pts.end());
        vector<I> ext=pts;
        for (size_t z=1;z<pts.size();++z)
            if (pts[z]-pts[z-1]>1) ext.push_back((pts[z]+pts[z-1])/2);
        auto pick=[&](){return ext[rng()%ext.size()];};
        qs.emplace_back(1,n/2); qs.emplace_back(n/2+1,n);
        for (int i=0;i<min(800,(int)ext.size());++i) {
            I p=ext.size()<=800?ext[i]:pick();
            qs.emplace_back(1,p); qs.emplace_back(p,n);
        }
        if (ext.size()<=42) {
            for (I x:ext) for (I y:ext) if(x<y) qs.emplace_back(x,y);
        } else {
            for(int i=0;i<800;++i) {
                I x=pick(),y=pick();
                qs.emplace_back(min(x,y),max(x,y));
            }
        }
    }
    // Score the response partition, favoring immediate identifications
    // and otherwise penalizing large remaining buckets.
    vector<double> cost(m+1);
    for(int i=2;i<=m;++i) cost[i]=i*(1+0.22*log2(i));
    int cap=1;
    while(cap<4*m) cap*=2;
    vector<I> key(cap);
    vector<int> stamp(cap),cnt(cap);
    int iter=0;
    double opt=1e100;
    for(auto [l,r]:qs) {
        if(l>=r) continue;
        ++iter;
        double score=0;
        for(auto x:h) {
            I v=x.eval(l,r);
            U z=U(v)*11995408973635179863ULL;
            int p=(z^(z>>32))&(cap-1);
            while(stamp[p]==iter && key[p]!=v) p=(p+1)&(cap-1);
            if(stamp[p]!=iter) {stamp[p]=iter;key[p]=v;cnt[p]=0;}
            int c=cnt[p]++;
            score+=cost[c+1]-cost[c];
            if(score>=opt) break;
        }
        if(score<opt) {opt=score;best={l,r};}
        if(opt==0) break;
    }
    return best;
}

// Exact minimum total decision-tree depth on the smallest instances.
void small(vector<Hyp> h) {
    vector<Q> qs;
    vector<vector<U>> buckets;
    for(I l=1;l<n;++l) for(I r=l+1;r<=n;++r) {
        map<I,U> masks;
        for(int z=0;z<(int)h.size();++z) masks[h[z].eval(l,r)]|=1ULL<<z;
        if(masks.size()<2) continue;
        qs.emplace_back(l,r);
        buckets.emplace_back();
        for(auto [v,mask]:masks) buckets.back().push_back(mask);
    }
    unordered_map<U,pair<int,int>> memo;
    function<int(U)> dp=[&](U st){
        int sz=popcount(st);
        if(sz<=1) return 0;
        if(auto it=memo.find(st);it!=memo.end()) return it->second.first;
        vector<pair<int,int>> order;
        for(int i=0;i<(int)qs.size();++i) {
            int bound=sz; bool splits=true;
            for(U mask:buckets[i]) {
                U t=mask&st;
                if(t==st){splits=false;break;}
                if(t&(t-1)) bound+=popcount(t);
            }
            if(splits) order.emplace_back(bound,i);
        }
        sort(order.begin(),order.end());
        int opt=INT_MAX,idx=-1;
        for(auto [lb,i]:order) {
            if(lb>=opt) break;
            int v=lb;
            for(U mask:buckets[i]) {
                U t=mask&st;
                if(t&(t-1)) v+=dp(t)-popcount(t);
                if(v>=opt) break;
            }
            if(v<opt) opt=v,idx=i;
        }
        memo[st]={opt,idx};
        return opt;
    };
    U st=(1ULL<<h.size())-1;
    while(st&(st-1)) {
        dp(st);
        auto [l,r]=qs[memo[st].second];
        I v=ask(l,r);
        U next=0;
        for(int z=0;z<(int)h.size();++z)
            if(h[z].eval(l,r)==v) next|=1ULL<<z;
        st&=next;
        if(!st) exit(0);
    }
    auto [s,a,b]=h[countr_zero(st)];
    cout<<"! "<<s<<' '<<s+a<<' '<<s+a+b-1<<endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>n) || n<4) return 0;
    vector<Group> gs;
    if(n<=65) {
        for(I a=2;a+2<=n;++a)
            for(I b=2;a+b<=n;++b) gs.push_back({a,b,1,n-a-b+1});
    } else gs=lengths(ask(1,n));
    for(;;) {
        vector<I> ends;
        I count=0;
        for(auto g:gs) ends.push_back(count+=g.hi-g.lo+1);
        if(!count) return 0;
        if(count==1) {
            auto [a,b,s,t]=gs[0];
            cout<<"! "<<s<<' '<<s+a<<' '<<s+a+b-1<<endl;
            return 0;
        }
        vector<Hyp> h;
        int m=count<=1200?(int)count:512;
        for(int i=0;i<m;++i) {
            I l=I(W(count)*i/m),r=I(W(count)*(i+1)/m);
            I idx=l+rng()%(r-l);
            if(i==0) idx=0;
            if(i==m-1) idx=count-1;
            int z=upper_bound(ends.begin(),ends.end(),idx)-ends.begin();
            auto g=gs[z];
            h.push_back({g.hi-(ends[z]-1-idx),g.a,g.b});
        }
        if(n<=9) {small(h);return 0;}
        Q q=choose(h);
        I v=ask(q.first,q.second);
        gs=filter(gs,q,v);
    }
}
