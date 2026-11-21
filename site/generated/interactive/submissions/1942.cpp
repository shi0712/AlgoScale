#include <bits/stdc++.h>
using namespace std;

using ll = long long;
ll n, h, s, k;

ll modn(ll a) {
    a %= n;
    return a < 0 ? a + n : a;
}
ll dist1(ll a, ll b) {
    ll z = abs(a - b);
    return min(z, n - z);
}
ll ask(ll x) {
    cout << "? " << x + 1 << endl;
    ll v;
    if (!(cin >> v) || v < 0) exit(0);
    return v;
}

// After measuring s at 0, the families in signed coordinates are
// (t,t+s), (t,s-t), and (-t,t-s).
struct Seg { int f; ll l, r; };
using State = vector<Seg>;
const int A[] = {1, 1, -1}, B[] = {1, -1, 1};

pair<ll,ll> pts(int f, ll t) {
    return {modn(A[f]*t), modn(B[f]*t + (f == 2 ? -s : s))};
}
ll sizeOf(const State& st) {
    ll m = 0;
    for (auto e : st) m += e.r-e.l+1;
    return m;
}

// On each returned piece the answer is constant or has slope +/-2.
template<class F>
void pieces(Seg e, ll x, F fn) {
    ll c = modn(A[e.f]*x);
    ll b = modn(B[e.f]*(x - (e.f == 2 ? -s : s)));
    ll cut[6] = {e.l, e.r+1};
    int z = 2;
    for (ll v : {c, (c+h+1)%n, b, (b+h+1)%n}) {
        if (v > h) v -= n;
        if (v > e.l && v <= e.r) cut[z++] = v;
    }
    sort(cut, cut+z);
    for (int i=1; i<z; i++) if (cut[i-1] < cut[i]) {
        ll l=cut[i-1], r=cut[i]-1;
        auto value = [&](ll t) {
            ll u = t < 0 ? t+n : t;
            return dist1(u,c)+dist1(u,b);
        };
        fn(l, r, value(l), value(r));
    }
}

State filtered(const State& st, ll x, ll ans) {
    State res;
    for (auto e : st) {
        pieces(e,x,[&](ll l,ll r,ll a,ll b) {
            if (a==b) {
                if (a!=ans) return;
            } else {
                if (ans < min(a,b) || ans > max(a,b) || (abs(a-ans)&1)) return;
                l = r = l + abs(a-ans)/2;
            }
            if (!res.empty() && res.back().f==e.f && res.back().r+1==l)
                res.back().r=r;
            else res.push_back({e.f,l,r});
        });
    }
    return res;
}

// For a contiguous interval of rotations there are k intermediate
// readings along a ramp, with a plateau to either side. A query can
// place this ramp inside the interval and recurse on its two ends.
// Expand a binary tree of these queries, filling shallow levels first.
ll rotCost(ll m) {
    ll cost=0, nodes=1;
    for (int dep=1; m>1; dep++,nodes*=2) {
        ll g=min(m-1,nodes*(k+1));
        cost+=dep*g+(g+k)/(k+1);
        m-=g;
    }
    return cost;
}
ll leftSize(ll m) {
    if (m<=k+2) return max(0LL,m-k-1);
    ll rem=m-(k+2), left=1;
    for (ll nodes=2; rem; nodes*=2) {
        ll g=min(rem,nodes*(k+1));
        left+=min(g,nodes/2*(k+1));
        rem-=g;
    }
    return left;
}
ll rotationQuery(Seg e,ll left) {
    return modn(e.l+left-1+(s<=h ? 0 : s));
}

struct Event {
    ll key;
    int f;
    ll delta;
    bool operator<(const Event& o) const {return key<o.key;}
};
struct Rank {
    ll cost;
    double tie;
    ll x;
    bool operator<(const Rank& o) const {
        return tie_tuple()<o.tie_tuple();
    }
    tuple<ll,double,ll> tie_tuple() const { return {cost,tie,x}; }
};
const ll INF = (1LL<<60);

// Sweep the answer histogram in runs (separately for each parity).
// A nonsingleton bucket needs at least another query; also account
// for the search still needed among rotations within that bucket.
Rank evaluate(const State& st,ll x,ll m) {
    vector<Event> ev;
    ev.reserve(st.size()*8);
    for (auto e:st) pieces(e,x,[&](ll l,ll r,ll a,ll b) {
        ll cnt=a==b ? r-l+1 : 1;
        ll base=(a&1)*(n+3);
        ev.push_back({base+min(a,b)/2,e.f,cnt});
        ev.push_back({base+max(a,b)/2+1,e.f,-cnt});
    });
    sort(ev.begin(),ev.end());
    ll v[3]={}, prev=0, score=0;
    double tie=0;
    for (size_t i=0;i<ev.size();) {
        ll at=ev[i].key, w=at-prev;
        ll c=v[0]+v[1]+v[2];
        if (w && c>1) {
            if (c==m) return {INF,0,x};
            score+=w*(c+(v[0]>1 ? rotCost(v[0])-v[0] : 0));
            ll minority=c-max({v[0],v[1],v[2]});
            tie+=double(w)*(minority*4.0+c*log2(double(c)));
        }
        do {
            v[ev[i].f]+=ev[i].delta;
            i++;
        } while(i<ev.size() && ev[i].key==at);
        prev=at;
    }
    return {score,tie,x};
}

// Exact minimum total path length for a small remaining hypothesis set.
// Between distance breakpoints two answer functions are affine, so
// their equality can change only at a breakpoint or an intersection.
ll exactQuery(const State& st) {
    vector<pair<ll,ll>> p;
    for (auto e:st) for (ll t=e.l;t<=e.r;t++) p.push_back(pts(e.f,t));
    int m=p.size(), full=(1<<m)-1;
    vector<ll> cand={0};
    auto value=[&](int i,ll x) {
        return dist1(x,p[i].first)+dist1(x,p[i].second);
    };
    for (int i=0;i<m;i++) for(int j=0;j<i;j++) {
        vector<ll> b={0,n};
        for (ll u:{p[i].first,p[i].second,p[j].first,p[j].second}) {
            b.push_back(u); b.push_back((u+h+1)%n);
        }
        sort(b.begin(),b.end());
        for(int a=1;a<(int)b.size();a++) {
            ll l=b[a-1],r=b[a]-1;
            if(l>r) continue;
            cand.push_back(l);
            if(l==r) continue;
            ll d=value(i,l)-value(j,l);
            ll slope=value(i,l+1)-value(j,l+1)-d;
            if(slope && d%slope==0) {
                ll x=l-d/slope;
                if(x>=l && x<=r) {
                    cand.push_back(x);
                    if(x<r) cand.push_back(x+1);
                }
            }
        }
    }
    sort(cand.begin(),cand.end());
    cand.erase(unique(cand.begin(),cand.end()),cand.end());
    map<vector<int>,ll> partitions;
    for(ll x:cand) {
        map<ll,int> groups;
        for(int i=0;i<m;i++) groups[value(i,x)] |= 1<<i;
        vector<int> g;
        for(auto [y,mask]:groups) if(mask&(mask-1)) g.push_back(mask);
        if(g.empty()) return x;
        sort(g.begin(),g.end());
        partitions.emplace(g,x);
    }
    vector<int> dp(1<<m,-1);
    ll answer=0;
    function<int(int)> solve=[&](int mask) -> int {
        if(!(mask&(mask-1))) return 0;
        if(dp[mask]>=0) return dp[mask];
        int cnt=__builtin_popcount((unsigned)mask), best=10000;
        for(auto& [g,x]:partitions) {
            int cost=cnt;
            for(int b:g) {
                int sub=b&mask;
                if(sub==mask) {cost=10000;break;}
                cost+=solve(sub);
                if(cost>=best) break;
            }
            if(cost<best) {
                best=cost;
                if(mask==full) answer=x;
                if(best==cnt) break;
            }
        }
        return dp[mask]=best;
    };
    solve(full);
    return answer;
}

vector<Rank> options(const State& st,bool thorough) {
    vector<ll> xs;
    auto add=[&](ll x){ xs.push_back(modn(x)); };
    auto around=[&](ll x) {
        add(x-1);add(x);add(x+1);
    };
    int grid=thorough?64:12;
    for(int i=0;i<grid;i++) add(i*n/grid);
    around(s/2); around(-s/2);
    for(auto e:st) {
        ll m=e.r-e.l+1;
        if(!e.f) {
            ll left=leftSize(m);
            for(ll a:{left,m-min(k,m)-left,0LL,m-min(k,m),1LL})
                around(rotationQuery(e,a));
        }
        int steps=thorough?12:4;
        for(int j=0;j<=steps;j++) {
            auto [p,q]=pts(e.f,e.l+(m-1)*j/steps);
            for(ll v:{p,q}) {
                around(v); around(v+h); add(v+h+2);
            }
        }
    }
    sort(xs.begin(),xs.end());
    xs.erase(unique(xs.begin(),xs.end()),xs.end());
    ll m=sizeOf(st);
    vector<Rank> ranks;
    for(ll x:xs) ranks.push_back(evaluate(st,x,m));
    sort(ranks.begin(),ranks.end());
    if(thorough && ranks.front().cost>0) {
        Rank best=ranks.front();
        for(ll step=max(1LL,n/64);step;step/=2) {
            ll x=best.x;
            for(int j:{-2,-1,1,2}) {
                Rank z=evaluate(st,modn(x+j*step),m);
                if(z<best) best=z;
                ranks.push_back(z);
            }
        }
        sort(ranks.begin(),ranks.end());
        ranks.erase(unique(ranks.begin(),ranks.end(),
                    [](Rank a,Rank b){return a.x==b.x;}),ranks.end());
    }
    return ranks;
}

ll choose(const State& st) {
    if(st.size()==1) {
        Seg e=st[0];
        if(e.f) return modn(A[e.f]*(s/2));
        return rotationQuery(e,leftSize(e.r-e.l+1));
    }
    ll m=sizeOf(st);
    if(m<=12) return exactQuery(st);
    auto ranks=options(st,true);
    Rank best={INF,0,0};
    int tried=0;
    for(Rank z:ranks) {
        if(z.cost>best.cost || tried++==12) break;
        if(!z.cost) return z.x;
        // Refine the estimate where a plateau and candidates from a
        // different family give the same reply.
        set<ll> plateau;
        for(auto e:st) pieces(e,z.x,[&](ll l,ll r,ll a,ll b){
            if(l<r && a==b) plateau.insert(a);
        });
        for(ll y:plateau) {
            auto child=filtered(st,z.x,y);
            if(child.size()<2) continue;
            ll rot=0;
            for(auto e:child) if(!e.f) rot+=e.r-e.l+1;
            ll extra=rot>1 ? rotCost(rot)-rot : 0;
            auto nxt=options(child,false);
            if(nxt[0].cost<INF)
                z.cost+=max(0LL,nxt[0].cost-extra);
        }
        if(z<best) best=z;
        if(best.cost==ranks.front().cost) break;
    }
    return best.x;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>n) || n<3) return 0;
    h=n/2;
    s=ask(0);
    k=h-min(s,n-s);
    State st;
    auto put=[&](int f,ll l,ll r){
        if(l<=r) st.push_back({f,l,r});
    };
    put(0,max(-h,-s),min(0LL,h-s));
    for(int f:{1,2}) put(f,max(1LL,s-h),(s-1)/2);

    while(sizeOf(st)>1) {
        ll x=choose(st);
        ll y=ask(x);
        st=filtered(st,x,y);
    }
    if(!st.empty()) {
        auto [a,b]=pts(st[0].f,st[0].l);
        cout<<"! "<<a+1<<" "<<b+1<<endl;
    }
    return 0;
}
