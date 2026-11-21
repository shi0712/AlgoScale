#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using ld = long double;

struct Point {
    int x, y;
};
vector<Point> pt;
vector<int> hull, inner, xs;
int freqX[1001];

ll det(int a, int b) {
    return 1LL * pt[a].x * pt[b].y - 1LL * pt[a].y * pt[b].x;
}
ll turn(int a, int b, int c) {
    return det(a,b) + det(b,c) + det(c,a);
}

using Key = pair<int,int>;
Key key(int p, int q) {
    int g = gcd(p,q);
    return {p/g,q/g};
}
map<Key,ld> cacheQ;
ld ask(Key k) {
    auto it = cacheQ.find(k);
    if (it != cacheQ.end()) return it->second;
    cout << "? " << k.first << ' ' << k.second << endl;
    string r,s;
    if (!(cin >> r) || r[0]=='-' || !(cin >> s) || s[0]=='-') exit(0);
    // The unreduced size of an intersection denominator need not fit in a word.
    int a=min<int>(18,r.size()), b=min<int>(18,s.size());
    ld v=stoll(r.substr(0,a))/(ld)stoll(s.substr(0,b));
    if (v) v *= powl(10.L, (int)r.size()-a-(int)s.size()+b);
    cacheQ[k]=v;
    return v;
}
void finish(ll twice) {
    int g=gcd(twice,2LL);
    cout << "! " << twice/g << ' ' << 2/g << endl;
    exit(0);
}

struct Term {
    Key k;
    int w;
};
vector<Term> plan;

// Within a chain, the two endpoint limits of each open slab determine
// its integral. Assign their trapezoid weights to queries, pairing
// neighboring weights into one barycentric sample whenever possible.
void chainPlan(int l,int r) {
    int m=r-l+1;
    vector<int> x(xs.begin()+l,xs.begin()+r+1), w(m);
    for(int i=0;i+1<m;i++) {
        int d=x[i+1]-x[i];
        w[i]+=d; w[i+1]+=d;
    }
    auto known=[&](int i) {
        return freqX[x[i]]==1 && (x[i]==xs.front() || x[i]==xs.back());
    };
    for(int i=0;i<m;i++) if(known(i)) w[i]=0;
    struct State {
        int cost,prev;
        vector<Term> add;
    };
    vector<map<int,State>> dp(m);
    dp[0][w[0]]={0,-1,{}};
    for(int i=0;i+1<m;i++) {
        int d=x[i+1]-x[i];
        for(auto &[a,st]:dp[i]) for(int b:{0,d,w[i+1]}) {
            if(b>w[i+1]) continue;
            int rem=w[i+1]-b;
            if(i+2==m && rem) continue;
            auto relax=[&](vector<Term> ts) {
                int c=st.cost+(int)ts.size();
                auto &nxt=dp[i+1];
                if(!nxt.count(rem) || c<nxt[rem].cost)
                    nxt[rem]={c,a,move(ts)};
            };
            if(a && b) {
                auto k=key(a*x[i]+b*x[i+1],a+b);
                if(k.second<=1000) relax({{k,a+b}});
            }
            vector<Term> sep;
            bool ok=true;
            auto single=[&](int j,int h,int other) {
                if(!h) return;
                if(freqX[x[j]]==1) sep.push_back({{x[j],1},h});
                else if(known(other))
                    sep.push_back({key(x[j]+x[other],2),2*h});
                else ok=false;
            };
            single(i,a,i+1); single(i+1,b,i);
            if(ok) relax(move(sep));
        }
    }
    int rem=0;
    for(int i=m-1;i>0;i--) {
        auto &st=dp[i].at(rem);
        for(auto t:st.add) plan.push_back(t);
        rem=st.prev;
    }
}

// A small polygonization family can be handled as an exact finite
// hypothesis set. Boundary points occur in hull order; only the
// placements of the strictly interior points have to be enumerated.
struct Edge {int a,b,sg;};
struct Candidate {ll area; vector<Edge> e;};
vector<Candidate> cand;
ll hullArea;

bool onseg(int a,int b,int p) {
    return min(pt[a].x,pt[b].x)<=pt[p].x && pt[p].x<=max(pt[a].x,pt[b].x)
        && min(pt[a].y,pt[b].y)<=pt[p].y && pt[p].y<=max(pt[a].y,pt[b].y);
}
bool conflict(int a,int b,int c,int d) {
    if(b==c) return !turn(a,b,d) && (onseg(a,b,d)||onseg(b,d,a));
    if(a==d) return !turn(c,a,b) && (onseg(c,a,b)||onseg(a,b,c));
    ll u=turn(a,b,c),v=turn(a,b,d),s=turn(c,d,a),t=turn(c,d,b);
    if((!u&&onseg(a,b,c))||(!v&&onseg(a,b,d))||
       (!s&&onseg(c,d,a))||(!t&&onseg(c,d,b))) return true;
    return ((u>0 && v<0)||(u<0 && v>0)) && ((s>0 && t<0)||(s<0 && t>0));
}

void generate() {
    int h=hull.size(), k=inner.size();
    vector<int> gap(k);
    do {
        auto rec=[&](auto &&self,int j,int lo)->void {
            if(j<k) {
                for(int s=lo;s<h;s++) { gap[j]=s; self(self,j+1,s); }
                return;
            }
            vector<Edge> es;
            ll area=hullArea;
            auto add=[&](int a,int b,int sg) {
                if(sg==1)
                    for(auto e:es)
                        if(e.sg==1 && conflict(e.a,e.b,a,b)) return false;
                es.push_back({a,b,sg});
                area+=sg*det(a,b);
                return true;
            };
            for(int i=0;i<k;) {
                int s=gap[i], a=hull[s], b=hull[(s+1)%h];
                add(a,b,-1);
                do {
                    if(!add(a,inner[i],1)) return;
                    a=inner[i++];
                } while(i<k && gap[i]==s);
                if(!add(a,b,1)) return;
            }
            cand.push_back({area,move(es)});
        };
        rec(rec,0,0);
    } while(next_permutation(inner.begin(),inner.end()));
}

// At a vertex abscissa, average the two limiting sections and add
// half the total vertical boundary length there.
ld edgeValue(Edge e,Key k) {
    Point a=pt[e.a], b=pt[e.b];
    int p=k.first,q=k.second;
    if(a.x==b.x) return p==q*a.x ? e.sg*abs(b.y-a.y)*0.5L : 0;
    int sg=e.sg;
    if(a.x>b.x) swap(a,b);
    else sg=-sg;
    int d=p-q*a.x, len=q*(b.x-a.x);
    if(d<0 || d>len) return 0;
    ld y=a.y+(ld)(b.y-a.y)*d/len;
    return sg*y*((d==0 || d==len)?0.5L:1.L);
}
ld delta(const Candidate &c,Key k) {
    ld s=0;
    for(auto e:c.e) s+=edgeValue(e,k);
    return s;
}
void tryFamily() {
    int n=pt.size();
    ll ways=1;
    for(int j=0;j<(int)inner.size();j++) {
        ways*=n-1-j;
        if(ways>40000 || ways*n>2000000) return;
    }
    generate();
    vector<int> live(cand.size());
    iota(live.begin(),live.end(),0);
    vector<Key> tests;
    for(int x:xs) tests.push_back({x,1});
    for(int i=1;i<(int)xs.size();i++)
        tests.push_back(key(498*xs[i-1]+499*xs[i],997));
    mt19937 rng(832746);
    shuffle(tests.begin(),tests.end(),rng);
    const ld eps=1e-8L;
    int rounds=0;
    while(!live.empty()) {
        ll area=cand[live[0]].area;
        if(all_of(live.begin(),live.end(),[&](int i){return cand[i].area==area;}))
            finish(area);
        Key best={-1,1};
        pair<int,ll> score={(int)live.size(),1LL*live.size()*live.size()};
        vector<pair<ld,ll>> vals(live.size());
        for(auto k:tests) {
            if(cacheQ.count(k)) continue;
            for(int j=0;j<(int)live.size();j++) {
                auto &c=cand[live[j]];
                vals[j]={delta(c,k),c.area};
            }
            sort(vals.begin(),vals.end());
            pair<int,ll> cur={0,0};
            for(int i=0;i<(int)vals.size();) {
                int j=i+1; bool mixed=false;
                while(j<(int)vals.size() && vals[j].first-vals[j-1].first<=2*eps) {
                    mixed |= vals[j].second!=vals[i].second;
                    j++;
                }
                if(mixed) {
                    cur.first=max(cur.first,j-i);
                    cur.second+=1LL*(j-i)*(j-i);
                }
                i=j;
            }
            if(cur<score) {score=cur; best=k;}
            if(!score.first) break;
        }
        if(best.first<0 || (score.first && rounds+1>=(int)plan.size())) return;
        ld base=0;
        for(int i=0;i<(int)hull.size();i++)
            base+=edgeValue({hull[i],hull[(i+1)%hull.size()],1},best);
        ld obs=ask(best)-base;
        live.erase(remove_if(live.begin(),live.end(),[&](int i){
            return fabsl(delta(cand[i],best)-obs)>eps;
        }),live.end());
        rounds++;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if(!(cin>>n) || n<3) return 0;
    pt.resize(n);
    for(auto &p:pt) {
        if(!(cin>>p.x>>p.y)) return 0;
        freqX[p.x]++;
    }
    sort(pt.begin(),pt.end(),[](Point a,Point b){
        return tie(a.x,a.y)<tie(b.x,b.y);
    });
    for(int pass=0;pass<2;pass++) {
        vector<int> part;
        for(int j=0;j<n;j++) {
            int i=pass?n-1-j:j;
            while(part.size()>=2 && turn(part[part.size()-2],part.back(),i)<0)
                part.pop_back();
            part.push_back(i);
        }
        part.pop_back();
        hull.insert(hull.end(),part.begin(),part.end());
    }
    vector<bool> on(n);
    for(int i=0;i<(int)hull.size();i++) {
        on[hull[i]]=true;
        hullArea+=det(hull[i],hull[(i+1)%hull.size()]);
    }
    for(int i=0;i<n;i++) if(!on[i]) inner.push_back(i);
    if(inner.empty()) finish(hullArea);
    for(int i=0;i<=1000;i++) if(freqX[i]) xs.push_back(i);
    int l=0;
    for(int i=1;i<(int)xs.size();i++) if(freqX[xs[i]]>1 || i+1==(int)xs.size()) {
        chainPlan(l,i);
        l=i;
    }
    tryFamily();
    ld ans=0;
    for(auto t:plan) ans+=t.w*ask(t.k);
    finish(llroundl(ans));
}
