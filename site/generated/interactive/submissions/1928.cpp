#include <bits/stdc++.h>
using namespace std;

using U = uint64_t;

// State bits: low bit = last reply was a lie; high bit = the last
// two replies were both lies or both truths.
constexpr int SMALL = 14;
constexpr int A = 1597, B = 987;
int fibs[32];
unordered_map<U, pair<int,int>> memo;
int budget;
bool stopped;

struct Query { int l, r; }; // Half-open, in the ordered list of survivors.
struct Node {
    U s = 0;
    int n = 0, forced = 0;
    void add(int x) {
        s |= U(x) << (2*n++);
        forced += x >> 1;
    }
    int weight() const { return A*(n-forced) + B*forced; }
};

bool fits(const Node& v, int d) {
    return 1LL*(v.n-v.forced)*fibs[d+2] + 1LL*v.forced*fibs[d+1]
           <= (2LL << d);
}

// On a circle every arc or its complement is an allowed interval.
// Complementing a question merely interchanges its two replies.
U key(const Node& v) {
    int bits = 2*v.n;
    U mark = U(1)<<bits, mask = (mark-1)/3;
    U best = mark, x = v.s;
    for (int i=0; i<v.n; ++i) {
        best = min(best, min(x, x^mask));
        x = (x>>2) | ((x&3)<<(bits-2));
    }
    return mark | best;
}

struct Option { Query q; Node c[2]; int rank; };

bool feasible(const Node& v, int d, Query* result = nullptr) {
    if (v.n <= 2) return true;
    if (d == 0 || !fits(v,d)) return false;
    U k = key(v);
    auto it = memo.find(k);
    if (it != memo.end()) {
        if (it->second.first >= d) return false;
        if (!result && it->second.second <= d) return true;
    }
    if (--budget < 0) { stopped = true; return false; }

    array<Option, 1+SMALL*(SMALL-1)/2> opts;
    int cnt = 0;
    auto consider = [&](int l, int r) {
        Option o{};
        o.q = {l,r};
        for (int i=0; i<v.n; ++i) {
            int t = (v.s>>(2*i))&3;
            int in = (l<=i && i<r);
            int repeatReply = in ^ (t&1);
            if (t<2) o.c[repeatReply].add(t+2);
            o.c[1-repeatReply].add(1-(t&1));
        }
        if (!fits(o.c[0],d-1) || !fits(o.c[1],d-1)) return;
        if (o.c[0].weight()<o.c[1].weight()) swap(o.c[0],o.c[1]);
        o.rank = o.c[0].weight()*16 + min(o.c[0].n,o.c[1].n);
        opts[cnt++] = o;
    };
    consider(0,v.n);
    for (int l=0;l<v.n-1;++l)
        for (int r=l+1;r<v.n;++r) consider(l,r);
    sort(opts.begin(),opts.begin()+cnt,[](const Option& a,const Option& b){
        return a.rank<b.rank;
    });
    for (int j=0;j<cnt;++j) {
        auto& o = opts[j];
        if (feasible(o.c[0],d-1) && feasible(o.c[1],d-1)) {
            auto p = memo.try_emplace(k, -1, 100).first;
            p->second.second = min(p->second.second,d);
            if (result) *result = o.q;
            return true;
        }
        if (stopped) return false;
    }
    auto p = memo.try_emplace(k, -1, 100).first;
    p->second.first = max(p->second.first,d);
    return false;
}

struct Student { int id, t; };

Query balanced(const vector<Student>& v) {
    // Minimize the difference of the two child weights. For a free
    // candidate the children are free and forced; for a forced one
    // they are free and eliminated.
    int n = v.size();
    vector<long long> ps(n+1);
    for (int i=0;i<n;++i) {
        int t=v[i].t, w=t>=2 ? A : A-B;
        ps[i+1]=ps[i]+((t&1)?w:-w);
    }
    long long best=llabs(ps[n]);
    Query q{0,n};
    map<long long,int> prior;
    for (int r=1;r<=n && best;++r) {
        prior.emplace(2*ps[r-1],r-1);
        long long target=2*ps[r]-ps[n];
        auto it=prior.lower_bound(target);
        auto use=[&](auto p){
            long long diff=llabs(p->first-target);
            if(diff<best) { best=diff; q={p->second,r}; }
        };
        if(it!=prior.end()) use(it);
        if(it!=prior.begin()) use(prev(it));
    }
    return q;
}

Query choose(const vector<Student>& v) {
    int n=v.size();
    if(n<=SMALL) {
        Node root;
        for(auto e:v) root.add(e.t);
        budget=2200; stopped=false;
        Query q{};
        for(int d=1;d<=24 && !stopped;++d)
            if(feasible(root,d,&q)) return q;
    }
    if(n<=4) {
        // Focus on three candidates. With two forced, make their
        // required replies conflict. With one forced, its required
        // reply should make both others forced. With none, split the
        // repeat replies so at least one candidate becomes forced.
        int f=0;
        for(int i=0;i<3;++i) f+=(v[i].t>=2);
        for(int l=0;l<3;++l) for(int r=l+1;r<=3;++r) {
            bool ok=true;
            for(int a=0;a<2;++a) {
                bool lost=false; int nf=0;
                for(int i=0;i<3;++i) {
                    if((a^(l<=i && i<r))==(v[i].t&1)) {
                        if(v[i].t>=2) lost=true;
                        ++nf;
                    }
                }
                if(!lost && nf<=f) ok=false;
            }
            if(ok) return {l,r};
        }
    }
    return balanced(v);
}

int readReply() {
    int x;
    if(!(cin>>x) || x<0) exit(0);
    return x;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if(!(cin>>n) || n<3) return 0;
    fibs[1]=1;
    for(int i=2;i<32;++i) fibs[i]=fibs[i-1]+fibs[i-2];
    vector<Student> v;
    for(int i=1;i<=n;++i) v.push_back({i,-1});
    while(v.size()>2) {
        Query q;
        if(v[0].t<0) {
            q={0,n/2};
            if(n<=SMALL) {
                // The first reply is unconstrained; its two outcomes
                // differ only by a global interchange of truth and lie.
                budget=2500; stopped=false;
                bool found=false;
                for(int d=1;d<=24 && !found && !stopped;++d)
                    for(int k=n/2;k>=0 && !stopped;--k) {
                        Node x;
                        for(int i=0;i<n;++i) x.add(i<k);
                        if(feasible(x,d)) {
                            q={0,k?k:n}; found=true; break;
                        }
                    }
            }
        } else q=choose(v);
        int l=v[q.l].id, r=v[q.r-1].id;
        cout<<"? "<<l<<' '<<r<<endl;
        int hands=readReply();
        int a=(hands==r-l);
        int m=0;
        for(auto e:v) {
            int bit=a^(l<=e.id && e.id<=r);
            bool rep=e.t>=0 && bit==(e.t&1);
            if(rep && e.t>=2) continue;
            v[m++]={e.id,bit+2*rep};
        }
        v.resize(m);
    }
    for(auto e:v) {
        cout<<"! "<<e.id<<endl;
        if(readReply()==1) break;
    }
    cout<<"#"<<endl;
    return 0;
}
