#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using u64 = uint64_t;
using u128 = __uint128_t;
int n;
int origin=1, direction=1;
int physical(int x) { return origin+direction*(x-1); }
mt19937_64 rng(54892761);

ll tri(ll x) { return x * (x - 1) / 2; }
ll root(ll x) {
    ll s = sqrt((double)x);
    if (s*s > x) --s;
    else if ((s+1)*(s+1) <= x) ++s;
    return s;
}
// For zero we use the positive inverse, 1.
int untri(ll y) {
    if (y < 0) return -1;
    ll t = (1 + (ll)(sqrt(8.0 * y + 1) + 0.5)) / 2;
    return tri(t) == y ? (int)t : -1;
}
ll ask(int l, int r) {
    int left=physical(l), right=physical(r);
    if (left>right) swap(left,right);
    cout << "? " << left << ' ' << right << endl;
    ll y;
    if (!(cin >> y) || y < 0 || y > tri(r-l+1)) exit(0);
    return y;
}

u64 mulmod(u64 a, u64 b, u64 m) { return (u128)a*b % m; }
u64 power(u64 a, u64 k, u64 m) {
    u64 v=1;
    for (; k; k>>=1, a=mulmod(a,a,m))
        if (k&1) v=mulmod(v,a,m);
    return v;
}
bool prime(u64 m) {
    if (m<2) return false;
    for (unsigned p : {2,3,5,7,11,13,17,19,23,29,31,37})
        if (m%p==0) return m==p;
    u64 d=m-1; int s=__builtin_ctzll(d); d>>=s;
    for (u64 a : {2ULL,325ULL,9375ULL,28178ULL,450775ULL,9780504ULL,1795265022ULL}) {
        if (a%m==0) continue;
        u64 x=power(a%m,d,m);
        if (x==1 || x==m-1) continue;
        int h=1;
        for (; h<s; ++h) {
            x=mulmod(x,x,m);
            if (x==m-1) break;
        }
        if (h==s) return false;
    }
    return true;
}
u64 divisor(u64 m) {
    for (int p : {2,3,5,7,11,13,17,19,23,29,31,37})
        if (m%p==0) return p;
    for (;;) {
        u64 x = 2+rng()%(m-2), y=x, c=1+rng()%(m-1);
        auto f=[&](u64 z){ return (mulmod(z,z,m)+c)%m; };
        for (int k=0;k<6000;++k) {
            u64 acc=1, prevX=x, prevY=y;
            for (int t=0;t<48;++t) {
                x=f(x); y=f(f(y));
                acc=mulmod(acc, x>y ? x-y : y-x,m);
            }
            u64 g=gcd(acc,m);
            if (g>1) {
                if (g<m) return g;
                for (int t=0;t<48;++t) {
                    prevX=f(prevX); prevY=f(f(prevY));
                    g=gcd(prevX>prevY ? prevX-prevY : prevY-prevX,m);
                    if (g>1) {
                        if (g<m) return g;
                        break;
                    }
                }
                break;
            }
        }
    }
}
void factor(u64 m,map<u64,int>& fac) {
    if (m==1) return;
    if (prime(m)) { ++fac[m]; return; }
    u64 d=divisor(m); factor(d,fac); factor(m/d,fac);
}
using Pair=pair<int,int>;
using Gauss=pair<ll,ll>;
Gauss product(Gauss x,Gauss y) {
    return {x.first*y.first-x.second*y.second,x.first*y.second+x.second*y.first};
}
// All ordered positive x,y with tri(x)+tri(y)=v.
// 4v+1 = (x+y-1)^2 + (x-y)^2. Factor it in the Gaussian integers.
vector<Pair> decompositions(ll v) {
    set<Pair> ans;
    auto add=[&](int x,int y){ ans.emplace(x,y); ans.emplace(y,x); };
    if (v<4000000) {
        for (int y=1; 2*tri(y)<=v; ++y) {
            int x=untri(v-tri(y));
            if (x>=y) add(x,y);
        }
    } else {
        map<u64,int> fac;
        factor(4*v+1,fac);
        vector<Gauss> ways{{1,0}};
        ll scalar=1;
        for (auto [p,e] : fac) {
            if (p%4==3) {
                if (e%2) return {};
                for (int t=0;t<e/2;++t) scalar*=p;
                continue;
            }
            u64 h=2;
            while (power(h,(p-1)/2,p)==1) ++h;
            u64 a=p, b=power(h,(p-1)/4,p);
            while (b>p/b) { u64 r=a%b; a=b; b=r; }
            Gauss z{b,root(p-b*b)}, conj{z.first,-z.second};
            vector<Gauss> pos(e+1,{1,0}), neg=pos;
            for (int t=1;t<=e;++t) {
                pos[t]=product(pos[t-1],z); neg[t]=product(neg[t-1],conj);
            }
            vector<Gauss> next;
            for (int t=0;t<=e;++t) {
                Gauss w=product(pos[t],neg[e-t]);
                for (auto old:ways) next.push_back(product(w,old));
            }
            ways.swap(next);
        }
        for (auto [u,w]:ways) {
            u=abs(u)*scalar; w=abs(w)*scalar;
            if (u<w) swap(u,w);
            add((u+1+w)/2,(u+1-w)/2);
        }
    }
    return {ans.begin(),ans.end()};
}

struct Triple { int i,j,k; };
ll value(Triple a,int l,int r) {
    return tri(max(0,min(a.j-1,r)-max(a.i,l)+1))
        + tri(max(0,min(a.k,r)-max(a.j,l)+1));
}
void answer(Triple a) {
    if (direction>0) a={physical(a.i),physical(a.j),physical(a.k)};
    else a={physical(a.k),physical(a.j-1),physical(a.i)};
    cout << "! " << a.i << ' ' << a.j << ' ' << a.k << endl;
    exit(0);
}

// A translated shape varies its start. An extension fixes i,j and varies k.
struct Family {
    bool extension;
    int a,b,lo,hi;
    Triple at(int t) const {
        if (extension) return {a,b,t};
        return {t,t+a,t+(a+b-1)};
    }
    ll size() const { return hi-lo+1; }
};

struct Cut { bool suffix; int edge; };
ll observe(Triple t,Cut c) {
    return value(t,c.suffix?c.edge:1,c.suffix?n:c.edge);
}

// Enumerate (or just count) the parameter intervals with this answer.
template<class Emit>
void matching(const Family& f,Cut c,ll y,Emit emit) {
    if (f.lo==f.hi) {
        if (observe(f.at(f.lo),c)==y) emit(f.lo,f.hi);
        return;
    }
    if (f.extension) { // Only present before the second, suffix query.
        int fixed=max(0,f.b-max(f.a,c.edge));
        int b=untri(y-tri(fixed));
        if (b>=1) {
            ll t=(ll)max(f.b,c.edge)+b-1;
            if (t>=f.lo && t<=f.hi) emit((int)t,(int)t);
        }
        return;
    }
    int a=f.a,b=f.b,r=c.edge;
    int flip=n-(a+b)+2;
    if (c.suffix) { swap(a,b); r=n-c.edge+1; }
    auto interval=[&](int low,int high) {
        if (c.suffix) {
            int u=flip-high;
            high=flip-low; low=u;
        }
        low=max(low,f.lo); high=min(high,f.hi);
        if (low<=high) emit(low,high);
    };
    if (y==0) { interval(r,n); return; }
    ll total=tri(a)+tri(b);
    if (y==total) { interval(1,r-(a+b)+1); return; }
    if (y>total) return;
    int u=untri(y);
    if (u>=2 && u<=a) interval(r-u+1,r-u+1);
    u=untri(y-tri(a));
    if (u>=1 && u<b) interval(r-u+1-a,r-u+1-a);
}

vector<Family> retain(const vector<Family>& fs,Cut cut,ll y) {
    vector<Family> out;
    for (auto f:fs)
        matching(f,cut,y,[&](int l,int h){out.push_back({f.extension,f.a,f.b,l,h});});
    return out;
}

vector<Triple> sample(const vector<Family>& fs,int count) {
    ll total=0;
    for (auto f:fs) total+=f.size();
    count=min<ll>(count,total);
    vector<Triple> res;
    size_t idx=0;
    ll offset=0;
    for (int s=0;s<count;++s) {
        ll lo=total/count*s+(total%count*s)/count;
        ll hi=total/count*(s+1)+(total%count*(s+1))/count;
        ll t=lo+rng()%(hi-lo);
        while (t>=offset+fs[idx].size()) offset+=fs[idx++].size();
        res.push_back(fs[idx].at(fs[idx].lo+(int)(t-offset)));
    }
    return res;
}

// Value a query by the sizes of its actual answer classes, with an extra
// step charged whenever a class still contains more than one possibility.
double cost(ll s) { return s<=1 ? 0.0 : 0.9+0.35*log2((double)s); }
Cut chooseCut(const vector<Family>& fs,int top,bool suffixOnly) {
    auto probes=sample(fs,clamp(22000/(int)fs.size(),18,100));
    set<pair<bool,int>> choices;
    for (bool suffix:{false,true}) {
        if (!suffix && suffixOnly) continue;
        vector<int> ends;
        for (auto p:probes) {
            ends.push_back(p.i); ends.push_back(p.j); ends.push_back(p.k);
            ends.push_back((p.i+p.j)/2); ends.push_back((p.j+p.k)/2);
        }
        sort(ends.begin(),ends.end());
        for (int t=1;t<24;++t) {
            choices.emplace(suffix,clamp(ends[t*ends.size()/24],1,top));
            choices.emplace(suffix,1+(ll)(top-1)*t/24);
        }
        choices.emplace(suffix,1); choices.emplace(suffix,top);
    }
    if (!suffixOnly) {
        auto widest=max_element(fs.begin(),fs.end(),[](auto x,auto y){return x.size()<y.size();});
        if (widest->lo<widest->hi) choices.emplace(false,(widest->lo+widest->hi)/2+1);
    }
    Cut best{}; double bestCost=1e100;
    for (auto [s,e]:choices) {
        Cut c{s,e};
        map<ll,int> freq;
        for (auto p:probes) ++freq[observe(p,c)];
        double score=0;
        for (auto [y,w]:freq) {
            ll cnt=0;
            for (auto& f:fs) matching(f,c,y,[&](int l,int h){cnt+=h-l+1;});
            score+=w*cost(cnt);
            if (score>=bestCost) break;
        }
        if (score<bestCost) { bestCost=score; best=c; }
    }
    return best;
}

int uniform(int lo,int hi) { return lo+(int)(rng()%(hi-lo+1)); }

// Choose the second query independently in each answer class of the first.
int lookAhead(const vector<vector<unsigned short>>& replies,const vector<int>& firstChoices,int limit) {
    int m=replies.front().size();
    // Count an additional query for each unresolved candidate. A small
    // further penalty favors smaller classes beyond the lookahead.
    vector<double> increment(m+1);
    auto penalty=[](int k){ return k<=1 ? 0.0 : k*(1.0+0.08*log2(k)); };
    for (int k=1;k<=m;++k) increment[k]=penalty(k)-penalty(k-1);
    double bestScore=1e100;
    int best=-1;
    vector<int> counts(limit);
    for (int q:firstChoices) {
        vector<vector<int>> groups(limit);
        for (int h=0;h<m;++h) groups[replies[q][h]].push_back(h);
        double score=0;
        for (auto& g:groups) if (g.size()>1) score+=g.size();
        if ((int)groups[replies[q][0]].size()==m) continue;
        for (auto& g:groups) {
            if (g.size()<=1) continue;
            if (score>=bestScore) break;
            double least=1e100;
            for (auto& row:replies) {
                fill(counts.begin(),counts.end(),0);
                double extra=0;
                for (int h:g) {
                    extra+=increment[++counts[row[h]]];
                    if (extra>=least) break;
                }
                least=min(least,extra);
                if (least==0) break;
            }
            score+=least;
        }
        if (score<bestScore) { bestScore=score; best=q; }
        if (score==0) break;
    }
    return best;
}

Pair smallBoardQuery(const vector<Triple>& states,int low,int high) {
    vector<Pair> queries;
    vector<vector<unsigned short>> replies;
    vector<int> firstChoices;
    for (int l=low;l<high;++l) for (int r=l+1;r<=high;++r) {
        if (high-low<24 || states.size()<=400 || l==low || r==high)
            firstChoices.push_back(queries.size());
        queries.emplace_back(l,r);
        auto& row=replies.emplace_back(states.size());
        for (size_t h=0;h<states.size();++h) row[h]=value(states[h],l,r);
    }
    return queries[lookAhead(replies,firstChoices,tri(high-low+1)+1)];
}

void explicitSolve(vector<Triple> states) {
    while (states.size()>1) {
        int low=n,high=1;
        vector<int> bounds;
        for (auto p:states) { low=min(low,p.i); high=max(high,p.k); }
        if (high-low<=39) {
            auto [l,r]=smallBoardQuery(states,low,high);
            ll y=ask(l,r);
            erase_if(states,[&](Triple p){return value(p,l,r)!=y;});
            continue;
        }
        set<Pair> qs;
        auto add=[&](int l,int r) { if (low<=l && l<r && r<=high) qs.emplace(l,r); };
        int samples=min(160,(int)states.size());
        for (int t=0;t<samples;++t) {
            auto p=states[t*(ll)states.size()/samples];
            for (int x:{p.i,p.j,p.k}) for (int d:{-1,0,1})
                bounds.push_back(clamp(x+d,low,high));
            bounds.push_back((p.i+p.j)/2); bounds.push_back((p.j+p.k)/2);
        }
        sort(bounds.begin(),bounds.end());
        bounds.erase(unique(bounds.begin(),bounds.end()),bounds.end());
        if (bounds.size()<80 && states.size()<1000) {
            for (int x:bounds) for (int z:bounds) add(x,z);
        } else {
            for (int t=0;t<90;++t) {
                int x=bounds[t*bounds.size()/90];
                add(low,x); add(x,high); add(low+1,x); add(x,high-1);
            }
            for (int t=0;t<200;++t) {
                int l=bounds[rng()%bounds.size()], r=bounds[rng()%bounds.size()];
                if (l>r) swap(l,r);
                add(l,r);
            }
        }
        if (high-low<=250) {
            for (int x=low;x<=high;++x) { add(low,x); add(x,high); }
        }
        // Prefixes at these positions distinguish any two different triples.
        for (auto p:{states.front(),states.back()})
            for (int x:{p.i,p.j,p.k}) for (int d:{-1,0,1,2}) add(low,x+d);
        vector<Triple> probes;
        int num=min(1600,(int)states.size());
        for (int t=0;t<num;++t) {
            int lo=t*(ll)states.size()/num, hi=(t+1)*(ll)states.size()/num-1;
            probes.push_back(states[uniform(lo,hi)]);
        }
        // Always include the endpoints used for the guaranteed split.
        if (num<(int)states.size()) {
            probes.push_back(states.front());
            probes.push_back(states.back());
            auto left=*min_element(states.begin(),states.end(),[](auto p,auto q){return p.i<q.i;});
            auto right=*max_element(states.begin(),states.end(),[](auto p,auto q){return p.k<q.k;});
            probes.push_back(left);
            probes.push_back(right);
        }
        vector<double> weight(probes.size()+1);
        for (int s=1;s<(int)weight.size();++s) weight[s]=s*cost(s);
        Pair best{}; double bestCost=1e100;
        vector<Pair> queries(qs.begin(),qs.end());
        vector<vector<unsigned short>> replies;
        vector<pair<double,int>> rated;
        unordered_map<ll,int> ids;
        vector<int> counts(probes.size());
        for (int q=0;q<(int)queries.size();++q) {
            auto [l,r]=queries[q];
            ids.clear();
            auto& row=replies.emplace_back(probes.size());
            for (int h=0;h<(int)probes.size();++h) {
                auto [it,added]=ids.emplace(value(probes[h],l,r),ids.size());
                if (added) counts[it->second]=0;
                ++counts[row[h]=it->second];
            }
            double score=0;
            for (int id=0;id<(int)ids.size();++id) score+=weight[counts[id]];
            rated.emplace_back(score,q);
            if (score<bestCost) { bestCost=score; best=queries[q]; }
            if (score==0) break;
        }
        if (bestCost>0 && num==(int)states.size()) {
            sort(rated.begin(),rated.end());
            vector<int> choices;
            for (int h=0;h<min(16,(int)rated.size());++h) choices.push_back(rated[h].second);
            best=queries[lookAhead(replies,choices,probes.size())];
        }
        auto [l,r]=best;
        ll y=ask(l,r);
        erase_if(states,[&](Triple p){return value(p,l,r)!=y;});
    }
    if (states.empty()) exit(0);
    answer(states.front());
}

void solveIfSmall(const vector<Family>& fs,ll limit) {
    ll count=0;
    for (auto f:fs) count+=f.size();
    if (count>limit) return;
    vector<Triple> states;
    states.reserve(count);
    for (auto f:fs) for (int x=f.lo;x<=f.hi;++x) states.push_back(f.at(x));
    explicitSolve(move(states));
}

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    if (!(cin>>n) || n<4 || n>1000000000) return 0;
    int r;
    ll prefixCount;
    for (;;) {
        if (n<=40) {
            vector<Triple> v;
            for (int i=1;i<=n-3;++i) for (int j=i+2;j<n;++j)
                for (int k=j+1;k<=n;++k) v.push_back({i,j,k});
            explicitSolve(move(v));
        }
        r=1+(ll)(n-1)*77/100;
        prefixCount=ask(1,r);
        if (prefixCount>0) break;
        // The first reversed pair starts at or beyond r. Search from the
        // other end of the remaining interval on the next iteration.
        origin=physical(n);
        direction=-direction;
        n=n-r+1;
    }
    vector<Family> fs;
    for (auto [a,b]:decompositions(prefixCount)) {
        if (a<2) continue;
        // Both blocks fully in the prefix.
        int last=r-(a+b)+1;
        if (b>=2 && 1<=last) fs.push_back({false,a,b,1,last});
        // The first block is complete and the second continues past r.
        int j=r-b+1, i=j-a;
        if (i>=1) fs.push_back({true,i,j,r+1,n});
    }
    int t=untri(prefixCount);
    bool plane=t>=2 && r-t+1>=1;
    if (fs.empty() && !plane) return 0;
    ll initialCount=plane ? tri(n-r) : 0;
    for (auto f:fs) initialCount+=f.size();
    if (initialCount<=1800) {
        if (plane) for (int j=r+1;j<n;++j)
            fs.push_back({true,r-t+1,j,j+1,n});
        solveIfSmall(fs,initialCount);
    }
    Cut cut;
    if (plane) cut={true,r-t+1};
    else cut=chooseCut(fs,r,true);
    ll suffixCount=ask(cut.edge,n);
    fs=retain(fs,cut,suffixCount);
    for (auto& f:fs) if (f.extension) {
        Triple p=f.at(f.lo); // A suffix reaching the end uniquely fixes k.
        f={false,p.j-p.i,p.k-p.j+1,p.i,p.i};
    }
    if (plane) {
        // The first prefix ended inside block one. Its start is known.
        int i=r-t+1;
        for (auto [a,b]:decompositions(suffixCount)) {
            ll j=(ll)i+a, k=j+b-1;
            if (j>r && b>=2 && k<=n)
                fs.push_back({false,(int)j-i,b,i,i});
        }
    }
    while (!fs.empty()) {
        solveIfSmall(fs,40000);
        // One large surviving run of translations always offers a split.
        cut=chooseCut(fs,n,false);
        int left=n,right=1;
        for (auto f:fs) {
            left=min(left,f.at(f.lo).i);
            right=max(right,f.at(f.hi).k);
        }
        int l=cut.suffix ? cut.edge : left+1;
        int h=cut.suffix ? right-1 : cut.edge;
        // Clipping one element at the outside end also tests the extreme
        // placement. Only one translation per shape can touch that end.
        bool trim=l<h;
        vector<Triple> extremes;
        vector<Family> regular;
        if (trim) {
            for (auto f:fs) {
                int x=cut.suffix ? f.hi : f.lo;
                auto p=f.at(x);
                if ((!cut.suffix && p.i<l) || (cut.suffix && p.k>h)) {
                    extremes.push_back(p);
                    if (cut.suffix) --f.hi; else ++f.lo;
                }
                if (f.lo<=f.hi) regular.push_back(f);
            }
            // Compare the two partitions exactly for the answer classes
            // affected by clipping. All other classes have identical cost.
            map<ll,int> change;
            for (auto p:extremes) {
                --change[observe(p,cut)];
                ++change[value(p,l,h)];
            }
            double difference=0;
            for (auto [y,delta]:change) if (delta) {
                ll old=0;
                for (auto f:fs) matching(f,cut,y,[&](int a,int b){old+=b-a+1;});
                difference+=(old+delta)*cost(old+delta)-old*cost(old);
            }
            trim=difference<=0;
        }
        if (trim) {
            ll v=ask(l,h);
            fs=retain(regular,cut,v);
            for (auto p:extremes) if (value(p,l,h)==v)
                fs.push_back({false,p.j-p.i,p.k-p.j+1,p.i,p.i});
        } else {
            ll v=ask(cut.suffix?cut.edge:1,cut.suffix?n:cut.edge);
            fs=retain(fs,cut,v);
        }
    }
    return 0;
}
