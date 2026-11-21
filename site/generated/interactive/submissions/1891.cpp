#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using ld = long double;
const ll INF = 1LL << 50;

ll N, basepos[2];
mt19937_64 rng(72318451);

bool ask(ll x, ll y) {
    cout << "? " << x << ' ' << y << endl;
    string s;
    if (!(cin >> s) || (s != "true" && s != "false")) exit(0);
    return s == "true";
}
[[noreturn]] void finish(ll x, ll y) {
    cout << "! " << x << ' ' << y << endl;
    exit(0);
}

// M = 2k+1. The offsets of the given cell from the board origin
// range independently between affine bounds on each k-interval.
struct Line {
    ll a, b;
    ll operator()(ll k) const { return a*k+b; }
};
struct Box {
    ll u,v;
    int parity;
    Line l[2], h[2];
};

ll floordiv(ll x,ll y) { // y positive
    return x/y - (x%y < 0);
}
// Restrict [u,v] to A(k) <= B(k) (strict if requested).
void restrictTo(ll &u,ll &v,Line A,Line B,bool strict=false) {
    ll a=A.a-B.a, b=B.b-A.b-strict;
    if (a>0) v=min(v,floordiv(b,a));
    else if (a<0) u=max(u,-floordiv(b,-a));
    else if (b<0) v=u-1;
}

void clip(const Box &s,int d,Line L,Line H,vector<Box>& out) {
    for(int i=0;i<2;i++) for(int j=0;j<2;j++) {
        ll u=s.u,v=s.v;
        Line lo=i?L:s.l[d], hi=j?H:s.h[d];
        if(i) restrictTo(u,v,s.l[d],L,true);
        else restrictTo(u,v,L,s.l[d]);
        if(j) restrictTo(u,v,H,s.h[d],true);
        else restrictTo(u,v,s.h[d],H);
        restrictTo(u,v,lo,hi);
        if(u<=v) {
            Box t=s;
            t.u=u;t.v=v;t.l[d]=lo;t.h[d]=hi;
            out.push_back(t);
        }
    }
}

Line edge(int tile,ll shift) {
    return {2*tile,tile-shift};
}

struct Board {
    ll k,r[2];
    ll start(int d) const { return basepos[d]-r[d]; }
    ll center(int d) const { return start(d)+5*k+2; }
    bool at(ll x,ll y) const {
        ll a=x-start(0),b=y-start(1),m=2*k+1;
        return a>=0 && b>=0 && a<5*m && b<5*m && (a/m+b/m)%2==0;
    }
};

// For a small remaining hypothesis set, optimize the whole decision tree.
// Grid membership is constant between consecutive tile boundaries.
void small(vector<Board> b) {
    int n=b.size(), full=(1<<n)-1;
    vector<ll> coords[2];
    for(int d=0;d<2;d++) {
        for(auto z:b) for(int j=0;j<=5;j++) {
            ll x=z.start(d)+j*(2*z.k+1);
            if(x<=N) coords[d].push_back(x);
        }
        sort(coords[d].begin(),coords[d].end());
        coords[d].erase(unique(coords[d].begin(),coords[d].end()),coords[d].end());
    }
    struct Q { int m; ll x,y; };
    vector<Q> qs;
    vector<int> seen(full+1);
    for(ll x:coords[0]) for(ll y:coords[1]) {
        int m=0;
        for(int j=0;j<n;j++) if(b[j].at(x,y)) m|=1<<j;
        if(m && m!=full && !seen[m]) {
            seen[m]=seen[full^m]=1;
            qs.push_back({m,x,y});
        }
    }
    vector<int> group(n);
    for(int i=0;i<n;i++) for(int j=0;j<n;j++)
        if(b[i].center(0)==b[j].center(0) && b[i].center(1)==b[j].center(1))
            group[i]|=1<<j;
    vector<int> cost(full+1,-1),choice(full+1,-1),depth(full+1);
    auto solve=[&](auto&& self,int m)->int {
        int &res=cost[m];
        if(res!=-1) return res;
        if(!(m & ~group[__builtin_ctz(m)])) return res=0;
        res=100000;
        for(int i=0;i<(int)qs.size();i++) {
            int a=m&qs[i].m,c=m^a;
            if(!a || !c) continue;
            int v=self(self,a)+self(self,c)+__builtin_popcount((unsigned)m);
            int h=1+max(depth[a],depth[c]);
            if(v<res || (v==res && h<depth[m])) res=v,choice[m]=i,depth[m]=h;
        }
        return res;
    };
    solve(solve,full);
    int m=full;
    while(choice[m]>=0) {
        auto q=qs[choice[m]];
        m &= ask(q.x,q.y)?q.m:(full^q.m);
    }
    auto z=b[__builtin_ctz(m)];
    finish(z.center(0),z.center(1));
}

ll pick(ll a,ll b) {
    return a+(ll)(rng()%(b-a+1));
}
ll width(const Box &s,int d,ll k) {
    return s.h[d](k)-s.l[d](k)+1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>N>>basepos[0]>>basepos[1])) return 0;
    ll K=(N/5-1)/2;
    vector<Box> boxes;
    for(ll u=1;u<=K;u*=2) for(int i=0;i<5;i++) for(int j=i%2;j<5;j+=2) {
        Box s{};
        s.u=u;s.v=min(K,2*u-1);s.parity=i%2;
        for(int d=0;d<2;d++) {
            int t=d?j:i;
            s.l[d]=edge(t,0);s.h[d]=edge(t+1,1);
        }
        vector<Box> tmp;
        clip(s,0,{10,basepos[0]+4-N},{0,basepos[0]-1},tmp);
        for(auto &t:tmp) clip(t,1,{10,basepos[1]+4-N},{0,basepos[1]-1},boxes);
    }

    while(!boxes.empty()) {
        ll ext[4]={INF,-INF,INF,-INF};
        Board witness[4]{};
        vector<ld> sums;
        ld total=0,count=0;
        for(auto &s:boxes) {
            for(ll k:{s.u,s.v}) for(int i=0;i<4;i++) {
                int d=i/2;
                Board z{k,{s.l[0](k),s.l[1](k)}};
                if(!(i&1)) z.r[d]=s.h[d](k);
                ll x=z.center(d);
                if((i&1)?x>ext[i]:x<ext[i]) ext[i]=x,witness[i]=z;
            }
            // Exact sum of width_x * width_y (a quadratic) over the slab.
            ld n=s.v-s.u+1;
            ld a=width(s,0,s.u),b=width(s,0,s.v);
            ld c=width(s,1,s.u),d=width(s,1,s.v);
            ld t=n==1?0:(n-2)/(6*(n-1));
            ld area=n*((0.5L-t)*(a*c+b*d)+t*(a*d+b*c));
            count+=area;
            // Approximately uniform prior per logarithmic size scale.
            ld scale=sqrtl((ld)(2*s.u+1)*(2*s.v+1));
            total+=area/(scale*scale*scale);
            sums.push_back(total);
        }
        if(ext[0]==ext[1] && ext[2]==ext[3]) finish(ext[0],ext[2]);
        if(count<12.5L) {
            vector<Board> b;
            for(auto &s:boxes) for(ll k=s.u;k<=s.v;k++)
                for(ll x=s.l[0](k);x<=s.h[0](k);x++)
                    for(ll y=s.l[1](k);y<=s.h[1](k);y++)
                        b.push_back({k,{x,y}});
            small(b);
        }

        // Sample only to choose a balanced query. Elimination and the
        // stopping test always use the exact feasible set.
        vector<Board> samples(witness,witness+4);
        constexpr int S=768;
        size_t idx=0;
        for(int i=0;i<S;i++) {
            ld p=(i+generate_canonical<ld,60>(rng))*total/S;
            while(idx+1<boxes.size() && sums[idx]<=p) ++idx;
            auto &s=boxes[idx];
            ll A=max(width(s,0,s.u),width(s,0,s.v));
            ll B=max(width(s,1,s.u),width(s,1,s.v));
            ll k;
            do {
                k=pick(s.u,s.v);
            } while(pick(1,A*B)>width(s,0,k)*width(s,1,k));
            samples.push_back({k,{pick(s.l[0](k),s.h[0](k)),pick(s.l[1](k),s.h[1](k))}});
        }

        int best=-1,dir=0;
        ll pos=0;
        for(int d=0;d<2;d++) {
            vector<pair<ll,int>> ev;
            for(auto z:samples) {
                ll m=2*z.k+1;
                for(int h=(z.r[d]/m)%2;h<5;h+=2) {
                    ll x=z.start(d)+h*m;
                    ev.emplace_back(x,1);
                    ev.emplace_back(x+m,-1);
                }
            }
            sort(ev.begin(),ev.end());
            int cur=0;
            for(size_t i=0;i<ev.size();) {
                ll x=ev[i].first;
                do {cur+=ev[i++].second;} while(i<ev.size() && ev[i].first==x);
                int score=min(cur,(int)samples.size()-cur);
                if(i<ev.size() && score>best) {
                    best=score;dir=d;pos=(x+ev[i].first-1)/2;
                }
            }
        }
        bool ans=ask(dir?basepos[0]:pos,dir?pos:basepos[1]);
        ll shift=pos-basepos[dir];
        vector<Box> next;
        for(auto &s:boxes) {
            Line prev{0,-INF};
            for(int h=s.parity;h<5;h+=2) {
                Line L=edge(h,shift),R=edge(h+1,shift);
                if(ans) clip(s,dir,L,{R.a,R.b-1},next);
                else clip(s,dir,prev,{L.a,L.b-1},next);
                prev=R;
            }
            if(!ans) clip(s,dir,prev,{0,INF},next);
        }
        boxes.swap(next);
    }
}
