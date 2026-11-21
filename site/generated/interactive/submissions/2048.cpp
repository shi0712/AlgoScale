#include <bits/stdc++.h>
using namespace std;

int n;
int ask(int i) {
    cout << "? " << i + 1 << endl;
    int r;
    if (!(cin >> r) || (r != 0 && r != 1)) exit(0);
    return r;
}

int distTo(int from, int to) { return (to - from + n) % n; }

void finish(const vector<int>& pos) {
    int target = 0, best = INT_MAX;
    for (int t : pos) {
        int cost = 0;
        for (int p : pos) cost += distTo(p,t);
        if (cost < best) best = cost, target = t;
    }
    for (int i = 0; i < n; ++i)
        for (int k = distTo(pos[i],target); k; --k) ask(i);
    cout << "!" << endl;
}

// Among small locks every relative state fits in a table. Store the successor
// and the reply together, so lookahead can partition the belief set cheaply.
class Small {
    using Row = array<int,8>;
    vector<Row> next;
    vector<int> cost;
    vector<double> uncertainty;
    vector<int> pos(int id) const {
        vector<int> a(n);
        for (int i = 1; i < n; ++i) a[i] = id % n, id /= n;
        return a;
    }
    // Expected cost multiplied by the number of equiprobable states.
    double search(const vector<int>& b, int depth, int& choice) {
        if (b.empty()) return 0;
        if (b.size() == 1) return cost[b[0]];
        if (!depth) {
            double s = uncertainty[b.size()];
            for (int id : b) s += cost[id];
            return s;
        }
        double best = 1e100;
        vector<int> part[2];
        for (auto& v : part) v.reserve(b.size());
        for (int p = 0; p < n; ++p) {
            for (auto& v : part) v.clear();
            for (int id : b) {
                int s = next[id][p];
                part[s & 1].push_back(s >> 1);
            }
            int tmp = 0;
            double score = b.size() + search(part[0],depth-1,tmp) + search(part[1],depth-1,tmp);
            // Resolve exact ties in favor of earlier information.
            score -= 0.00001 * min(part[0].size(),part[1].size());
            if (score < best - 1e-8) best = score, choice = p;
        }
        return best;
    }
    // Separate two hypotheses by arranging a collision in one and a vacant
    // destination in the other. All preparation moves also provide feedback.
    vector<int> separate(int u, int v) {
        auto a = pos(u), b = pos(v);
        vector<int> best;
        for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
            if (distTo(a[i],a[j]) == distTo(b[i],b[j])) continue;
            for (int d = 1; d <= n; ++d) {
                int t = (a[i]+d)%n;
                vector<int> seq;
                for (int k = 0; k < n; ++k)
                    if (k!=i && k!=j && a[k]==t) seq.push_back(k);
                seq.insert(seq.end(),distTo(b[j],(b[i]+d)%n),j);
                seq.insert(seq.end(),d,i);
                if (best.empty() || seq.size()<best.size()) best=seq;
            }
        }
        return best;
    }
public:
    void solve() {
        int count = 1;
        for (int i=1; i<n; ++i) count *= n;
        next.resize(count); cost.resize(count); uncertainty.resize(count+1);
        for (int i=1; i<=count; ++i) uncertainty[i]=0.66*i*log2(i);
        for (int id=0; id<count; ++id) {
            array<int,8> a{},cnt{};
            int number=id,power=1,shifted=0,fuel=0;
            ++cnt[0];
            for (int i=1;i<n;++i) {
                a[i]=number%n; number/=n;
                ++cnt[a[i]];
                fuel+=a[i]?n-a[i]:0;
                shifted+=(a[i]?a[i]-1:n-1)*power;
                power*=n;
            }
            cost[id]=fuel;
            for (int t=1;t<n;++t) {
                fuel+=n*(1-cnt[t]);
                cost[id]=min(cost[id],fuel);
            }
            next[id][0]=2*shifted+(cnt[1]!=0);
            power=1;
            for (int i=1;i<n;++i) {
                int dest=a[i]+1;
                int successor=id+power;
                if (dest==n) dest=0,successor-=n*power;
                next[id][i]=2*successor+(cnt[dest]!=0);
                power*=n;
            }
        }
        vector<int> belief(count-1);
        iota(belief.begin(),belief.end(),1);
        int stale=0;
        vector<int> plan;
        while (belief.size()>1) {
            int p=0;
            if (stale>=n && plan.empty()) {
                plan=separate(belief.front(),belief.back());
                reverse(plan.begin(),plan.end());
            }
            if (!plan.empty()) {
                p=plan.back(); plan.pop_back();
            } else {
                int depth=1;
                long long work=(long long)belief.size()*n;
                while (work*n<=400000 && depth<7) ++depth, work*=n;
                search(belief,depth,p);
            }
            int r=ask(p);
            size_t sz=0;
            for (int id:belief) {
                int s=next[id][p];
                if ((s&1)==r) belief[sz++]=s>>1;
            }
            if (sz<belief.size()) stale=0,plan.clear();
            else ++stale;
            belief.resize(sz);
        }
        finish(pos(belief.front()));
    }
};

using Mask=__uint128_t;
Mask full;
Mask bit(int k) { return Mask(1)<<k; }
int firstBit(Mask b) {
    return (uint64_t)b ? __builtin_ctzll((uint64_t)b) : 64+__builtin_ctzll((uint64_t)(b>>64));
}
int sizeOf(Mask b) { return __builtin_popcountll((uint64_t)b)+__builtin_popcountll((uint64_t)(b>>64)); }
bool single(Mask b) { return b && !(b&(b-1)); }
Mask rot(Mask b,int d) {
    return ((b<<d)&full)|(b>>(n-d));
}

// A compact set of complete hypotheses can be searched directly. Evaluate
// probes by the information in their replies and by their physical cost.
using World=vector<uint8_t>;

class ProbeSearch {
    struct Sample {
        array<uint8_t,12> pos{},count{},cost{};
    };
    vector<Sample> samples;
    double search(const vector<int>& ids,int depth,int& pick) {
        if (ids.empty()) return 0;
        if (ids.size()==1 || !depth) {
            double value=0.9*ids.size()*log2(ids.size());
            for (int id:ids) {
                auto& c=samples[id].cost;
                value+=n*(*min_element(c.begin(),c.begin()+n));
            }
            return value;
        }
        double best=1e100;
        vector<int> parts[2];
        for (int i=0;i<n;++i) {
            parts[0].clear();parts[1].clear();
            for (int id:ids) {
                auto& w=samples[id];
                int x=w.pos[i],y=(x+1==n?0:x+1);
                parts[w.count[y]!=0].push_back(id);
                --w.count[x];++w.count[y];++w.cost[x];w.pos[i]=y;
            }
            int ignored=0;
            double value=0.06*ids.size()+search(parts[0],depth-1,ignored)
                                        +search(parts[1],depth-1,ignored);
            value-=0.0001*min(parts[0].size(),parts[1].size());
            if (value<best-1e-8) best=value,pick=i;
            for (int id:ids) {
                auto& w=samples[id];
                int y=w.pos[i],x=(y?y-1:n-1);
                ++w.count[x];--w.count[y];--w.cost[x];w.pos[i]=x;
            }
        }
        return best;
    }
public:
    int choose(const vector<World>& worlds,const vector<int>& active,const vector<int>& shift) {
        int size=min(144,(int)active.size());
        samples.resize(size);
        vector<int> ids(size);
        iota(ids.begin(),ids.end(),0);
        for (int z=0;z<size;++z) {
            auto& w=samples[z];
            int fuel=0;
            for (int i=0;i<n;++i) {
                int x=(worlds[active[z]][i]+shift[i])%n;
                w.pos[i]=x;++w.count[x];fuel+=distTo(x,0);
            }
            for (int t=0;t<n;++t) {
                if (t) fuel+=n*(1-w.count[t]);
                w.cost[t]=fuel/n;
            }
        }
        int p=0,depth=3;
        if (size<=10) depth=4;
        search(ids,depth,p);
        return p;
    }
};

void exactWorlds(vector<World> worlds,const vector<int>& offsets) {
    vector<int> shift=offsets;
    vector<int> active(worlds.size());
    iota(active.begin(),active.end(),0);
    mt19937 rng(89321);
    shuffle(active.begin(),active.end(),rng);
    vector<int> plan;
    int stale=0;
    while (active.size()>1) {
        int p=0;
        auto at=[&](int id,int i) {return (worlds[id][i]+shift[i])%n;};
        if (stale>=n && plan.empty()) {
            int u=active.front(),v=active.back(),j=1;
            while (at(u,j)==at(v,j)) ++j;
            // Dial zero has the same origin in every hypothesis.
            int dest=(at(u,0)+1)%n;
            for (int i=1;i<n;++i) if (i!=j && at(u,i)==dest) plan.push_back(i);
            plan.insert(plan.end(),distTo(at(v,j),dest),j);
            plan.push_back(0);
            reverse(plan.begin(),plan.end());
        }
        if (!plan.empty()) {
            p=plan.back();plan.pop_back();
        } else if (n<=12) {
            p=ProbeSearch().choose(worlds,active,shift);
        } else {
            int kmax=min(n-1,6), patterns=1<<kmax;
            vector<int> hist(n*patterns),risk(n*kmax);
            int sample=min((int)active.size(),max(100,5000/n));
            for (int z=0;z<sample;++z) {
                int id=active[z];
                array<int,100> cnt{},pos{},pattern{},safe{};
                int fuel=0;
                for (int i=0;i<n;++i) {
                    pos[i]=at(id,i);++cnt[pos[i]];
                    fuel+=distTo(pos[i],0);
                }
                Mask targets=1;
                int best=fuel;
                for (int t=1;t<n;++t) {
                    fuel+=n*(1-cnt[t]);
                    if (fuel<best) best=fuel,targets=0;
                    if (fuel==best) targets|=bit(t);
                }
                for (int x=0;x<n;++x) if (cnt[x]) {
                    int word=0;
                    for (int k=1;k<=kmax;++k)
                        word=2*word+(cnt[(x+k)%n]>0);
                    pattern[x]=word;
                    Mask rotated=rot(targets,n-x);
                    // Only distances below kmax can incur a full extra turn.
                    int d=kmax;
                    if (rotated<bit(kmax)) d=63-__builtin_clzll((uint64_t)rotated);
                    safe[x]=d;
                }
                for (int i=0;i<n;++i) {
                    int x=pos[i];
                    ++hist[i*patterns+pattern[x]];
                    for (int k=safe[x];k<kmax;++k) ++risk[i*kmax+k];
                }
            }
            double best=1e100;
            for (int i=0;i<n;++i) {
                int* counts=&hist[i*patterns];
                for (int k=kmax;k;--k) {
                    double info=0;
                    for (int j=0;j<(1<<k);++j) if (counts[j]) {
                        double prob=(double)counts[j]/sample;
                        info-=prob*log2(prob);
                    }
                    double score=(n*(double)risk[i*kmax+k-1]/sample-1.05*info)/sqrt(k);
                    if (score<best) best=score,p=i;
                    for (int j=0;j<(1<<(k-1));++j) counts[j]=counts[2*j]+counts[2*j+1];
                }
            }
        }
        shift[p]=(shift[p]+1)%n;
        int r=ask(p);
        size_t keep=0;
        for (int id:active) {
            int x=at(id,p);
            bool hit=false;
            for (int j=0;j<n && !hit;++j) hit=j!=p && at(id,j)==x;
            if (hit==r) active[keep++]=id;
        }
        if (keep<active.size()) stale=0,plan.clear();
        else ++stale;
        active.resize(keep);
    }
    vector<int> pos(n);
    for (int i=0;i<n;++i) pos[i]=(worlds[active.front()][i]+shift[i])%n;
    finish(pos);
}

struct Run {
    int p;
    vector<int> before;
    Mask zeros=0, ones=0;
    Mask certain=0; // occupied at the start, possibly by this mover
    Mask appliedZeros=0, appliedOnes=0;
};
struct Literal { uint8_t dial, origin; };
using Clause=vector<Literal>;

// Coordinates are relative to the initial value of dial zero. domain[i]
// contains every still possible initial coordinate, and shift[i] records
// the moves made since then. Inferences only remove impossible origins.
// Sampling and expected costs affect the choice of query, never the proof
// of alignment.
class Tracker {
    vector<Mask> domain;
    vector<int> shift;
    vector<Run> runs;
    vector<Clause> clauses;
    vector<int> effort;
    int queries=0;
    int position(int i) const { return (firstBit(domain[i])+shift[i])%n; }

    // Project a disjunction of possible witnesses into the current coordinates.
    // If all witnesses would now be at x, the site is certainly occupied.
    Mask occupied() const {
        Mask must=0;
        for (int i=0; i<n; ++i) if (single(domain[i])) must |= bit(position(i));
        for (auto& c:clauses) {
            int site=-1;
            for (auto l:c) {
                if (!(domain[l.dial]&bit(l.origin))) continue;
                int x=(l.origin+shift[l.dial])%n;
                if (site==-1) site=x;
                else if (site!=x) {site=-2; break;}
            }
            if (site>=0) must|=bit(site);
        }
        return must;
    }
    bool narrow(int i, Mask accepted) {
        Mask a=domain[i]&accepted;
        if (a==domain[i]) return false;
        assert(a);
        domain[i]=a; return true;
    }
    void addClause(int except, const vector<int>& at, int x) {
        Clause c;
        for (int j=0;j<n;++j) if (j!=except) {
            int y=distTo(at[j],x);
            if (!(domain[j]&bit(y))) continue;
            if (single(domain[j])) return;
            c.push_back({(uint8_t)j,(uint8_t)y});
        }
        assert(!c.empty());
        clauses.push_back(move(c));
    }
    // All observations within a run refer to the same stationary background.
    // Intersect their absolute locations over all possible mover origins.
    bool analyze(Run& r) {
        int p=r.p;
        Mask known=0, possible=0;
        int unknown=0;
        for (int j=0;j<n;++j) if (j!=p) {
            Mask s=rot(domain[j],r.before[j]);
            possible|=s;
            if (single(s)) known|=s;
            else ++unknown;
        }
        Mask yes=full,no=full,valid=0;
        for (Mask b=domain[p]; b; b&=b-1) {
            int a=firstBit(b);
            Mask z=rot(r.zeros,a), o=rot(r.ones,a);
            Mask others=known|(r.certain&~bit((a+r.before[p])%n));
            if ((z&others) || (o&~possible)) continue;
            valid|=bit(a); yes&=o; no&=z;
        }
        bool changed=narrow(p,valid);
        Mask newNo=no&~r.appliedZeros;
        if (newNo) {
            for (int j=0;j<n;++j) if (j!=p)
                changed |= narrow(j,~rot(newNo,n-r.before[j]));
            r.appliedZeros|=newNo;
        }
        Mask newYes=yes&~r.appliedOnes;
        for (Mask b=newYes; b; b&=b-1) addClause(p,r.before,firstBit(b));
        r.appliedOnes|=newYes;
        Mask required=yes&~known;
        if (unknown && sizeOf(required)==unknown)
            for (int j=0;j<n;++j) if (j!=p && !single(domain[j]))
                changed |= narrow(j,rot(required,n-r.before[j]));
        return changed;
    }
    void propagate() {
        bool changed;
        do {
            changed=false;
            size_t keep=0;
            for (auto& c:clauses) {
                size_t sz=0;
                bool done=false;
                for (auto l:c) if (domain[l.dial]&bit(l.origin)) {
                    if (single(domain[l.dial])) { done=true; break; }
                    c[sz++]=l;
                }
                if (done) continue;
                c.resize(sz);
                assert(sz);
                if (sz==1) changed |= narrow(c[0].dial,bit(c[0].origin));
                else {
                    if (&clauses[keep]!=&c) clauses[keep]=move(c);
                    ++keep;
                }
            }
            clauses.resize(keep);
            for (auto& r:runs) changed |= analyze(r);
        } while (changed);
    }

    void advance(Run& r) {
        int i=r.p;
        shift[i]=(shift[i]+1)%n;
        int reply=ask(i);
        (reply?r.ones:r.zeros)|=bit(shift[i]);
        ++queries; ++effort[i];
    }
    Run makeRun(int i) {return {i,shift,0,0,occupied()};}
    void scan(int i,int d) {
        Run r=makeRun(i);
        for (int k=0;k<d;++k) advance(r);
        runs.push_back(move(r));
        propagate();
    }
    int countUnknown() const {
        return count_if(domain.begin(),domain.end(),[](Mask b){return !single(b);});
    }

    // Estimate how many steps a probe takes before its response sequence
    // isolates its origin, using both bounds on the stationary background.
    double probeCost(Mask starts, Mask must, Mask may) const {
        int limit=min(n-1,12), sz=sizeOf(starts);
        array<Mask,13> hit,miss;
        for (int k=1;k<=limit;++k) {
            hit[k]=rot(may,n-k);
            miss[k]=rot(full^must,n-k);
        }
        double total=0;
        for (Mask b=starts;b;b&=b-1) {
            int a=firstBit(b);
            Mask low=starts,high=starts;
            for (int k=1;k<=limit;++k) {
                Mask at=bit((a+k)%n);
                if (!single(low)) {
                    low &= (at&must)?hit[k]:miss[k];
                    total+=1;
                }
                if (!single(high)) {
                    high &= (at&may)?hit[k]:miss[k];
                    total+=1;
                }
                if (single(low)&&single(high)) break;
            }
            total+=1.5*(log2(sizeOf(low))+log2(sizeOf(high)));
        }
        return total/(2*sz);
    }

    // Compare two complete surveys with only p moving in between. A zero
    // exposes its destination. If every one of n-1 steps hits, the other
    // n-1 dials occupy all sites except its origin, which it has vacated.
    void identifyOne(int p) {
        int scout=0;
        Run a=makeRun(scout);
        for (int k=0;k<n;++k) advance(a);
        Mask vacancies=rot(a.zeros,firstBit(domain[scout]));
        runs.push_back(move(a)); propagate();
        if (single(domain[p])) return;
        Run m=makeRun(p);
        // A zero before a full turn must be a site vacant in the first survey
        // (the scout can mask one vacancy, but there are n-2 other fixed dials).
        for (int k=0;k<n-1;++k) {
            advance(m);
            if (m.zeros) break;
        }
        runs.push_back(move(m)); propagate();
        if (single(domain[p])) return;
        Run b=makeRun(scout);
        for (int k=0;k<n;++k) advance(b);
        Mask added=rot(b.ones,firstBit(domain[scout]))&vacancies;
        if (added) narrow(p,rot(added,n-shift[p]));
        else {
            Mask lost=rot(b.zeros,firstBit(domain[scout]))&~vacancies;
            narrow(p,rot(lost,n-runs.back().before[p]));
        }
        runs.push_back(move(b)); propagate();
    }

    // If the first survey has n-1 occupied sites, the unprobed dials
    // occupy them bijectively. Their complete multiset stays known after
    // each identification, including the holes created by earlier moves.
    void distinct(Mask remaining) {
        vector<int> pos(n),id(n);
        iota(id.begin(),id.end(),0);
        int step=-1;
        bool affine=!(remaining&1);
        mt19937 rng(432195);
        pos[0]=position(0);
        for (int p=1;p<n;++p) {
            if (single(remaining)) {pos[p]=firstBit(remaining);break;}
            vector<int> counts(n),cost(n);
            for (int j=0;j<p;++j) ++counts[pos[j]];
            vector<int> starts;
            for (Mask b=remaining;b;b&=b-1) {
                starts.push_back(firstBit(b)); ++counts[starts.back()];
            }
            Mask board=0;
            for (int x=0;x<n;++x) if (counts[x]) {
                board|=bit(x);
                for (int t=0;t<n;++t) cost[t]+=counts[x]*distTo(x,t);
            }
            int leader=0,length=0,target=0;
            Mask selected=board;
            double best=1e100;
            for (int j:{p-1,0}) {
                int x=pos[j];
                Mask rest=counts[x]==1?board^bit(x):board;
                for (int len=0;len<n;++len) {
                    Mask state=rest|bit((x+len)%n);
                    if (len && state==board) continue;
                    array<Mask,100> word;
                    array<int,100> depth{},delta{};
                    for (int a:starts) word[a]=rot(state,n-(a+1)%n);
                    bool periodic=false;
                    for (int a:starts) {
                        for (int b:starts) {
                            if (b>=a) break;
                            Mask diff=word[a]^word[b];
                            if (!diff) {periodic=true;break;}
                            int need=firstBit(diff)+1;
                            depth[a]=max(depth[a],need);depth[b]=max(depth[b],need);
                        }
                        if (periodic) break;
                    }
                    if (periodic) continue;
                    int overlap=0,totalDepth=0;
                    for (int a:starts) {
                        int end=(a+depth[a])%n;
                        ++delta[a];--delta[end];
                        if (end<a) ++overlap;
                        totalDepth+=depth[a];
                    }
                    double extra=0.012*totalDepth+0.025*len;
                    for (int t=0;t<n;++t) {
                        overlap+=delta[t];
                        double score=cost[t]+(distTo(x,t)<len?n:0)
                                    +min(0.60,4.0/starts.size())*n*overlap+extra;
                        if (score<best) best=score,leader=j,length=len,selected=state,target=t;
                    }
                }
                if (!j) break;
            }
            assert(best<1e99);
            if (affine && step>=0) {
                double highest=-1e100;
                int choice=p;
                for (int j=p;j<n;++j) {
                    int a=id[j]*step%n;
                    int depth=0;
                    Mask left=remaining;
                    while (!single(left)) {
                        ++depth;
                        Mask yes=rot(selected,n-depth);
                        left &= (bit(a)&yes)?yes:~yes;
                    }
                    int distance=distTo(a,target);
                    double score=(depth<=distance?2*n:0)-distance+0.35*depth;
                    if (score>highest) highest=score,choice=j;
                }
                swap(id[p],id[choice]);
            }
            for (int k=0;k<length;++k) ask(id[leader]);
            pos[leader]=(pos[leader]+length)%n;
            Mask possible=remaining;
            int steps=0;
            while (!single(possible)) {
                ++steps;
                Mask hit=rot(selected,n-steps);
                possible &= ask(id[p])?hit:~hit;
            }
            pos[p]=(firstBit(possible)+steps)%n;
            remaining^=possible;
            if (p==1) step=firstBit(possible);
            if (affine && (gcd(step,n)!=1 || id[p]*step%n!=firstBit(possible))) {
                affine=false; shuffle(id.begin()+p+1,id.end(),rng);
            }
        }
        vector<int> actual(n);
        for (int i=0;i<n;++i) actual[id[i]]=pos[i];
        finish(actual);
    }

    // Enumerate a short survey's remaining hypotheses when they fit cheaply.
    bool surveyBeliefs(const Run& r) const {
        const int limit=150000;
        int allowed=n-sizeOf(r.zeros), required=sizeOf(r.ones);
        array<int,16> ways{};
        ways[0]=1;
        for (int k=1;k<n;++k) {
            for (int j=required;j>=0;--j) {
                int w=(allowed-j)*ways[j]+(j?j*ways[j-1]:0);
                ways[j]=min(limit+1,w);
            }
        }
        if (ways[required]>limit) return false;
        vector<World> worlds;
        worlds.reserve(ways[required]);
        World a(n,0);
        auto enumerate=[&](auto&& self,int i,Mask need) -> void {
            if (i==n) {if (any_of(a.begin(),a.end(),[](int x){return x!=0;})) worlds.push_back(a);return;}
            Mask choices=(sizeOf(need)==n-i)?need:full^r.zeros;
            for (Mask b=choices;b;b&=b-1) {
                a[i]=firstBit(b);
                self(self,i+1,need&~bit(a[i]));
            }
        };
        enumerate(enumerate,1,r.ones);
        exactWorlds(move(worlds),shift);
        return true;
    }

    bool smallRemainder() const {
        int limit=min(1800,2800000/(1+(int)runs.size()*n));
        int total=1;
        for (Mask d:domain) {
            if (total>limit/sizeOf(d)) return false;
            total*=sizeOf(d);
        }
        vector<World> worlds;
        World a(n);
        auto enumerate=[&](auto&& self,int i) -> void {
            if (i<n) {
                for (Mask b=domain[i];b;b&=b-1) {
                    a[i]=firstBit(b);
                    self(self,i+1);
                }
                return;
            }
            for (auto& r:runs) {
                Mask occ=0;
                for (int j=0;j<n;++j) if (j!=r.p)
                    occ|=bit((a[j]+r.before[j])%n);
                occ=rot(occ,n-a[r.p]);
                if ((occ&r.zeros) || (r.ones&~occ)) return;
            }
            worlds.push_back(a);
        };
        enumerate(enumerate,0);
        exactWorlds(move(worlds),shift);
        return true;
    }

    bool resolvePair() {
        Mask sites=0;
        for (int i=0;i<n;++i) if (!single(domain[i]))
            sites|=rot(domain[i],shift[i]);
        if (sizeOf(sites)!=2) return false;
        int a=firstBit(sites), b=firstBit(sites&(sites-1));
        int match=-1,emptySite=-1,anchor=-1;
        double best=1e100;
        for (int start:{a,b}) {
            int m=(start+1)%n, e=((start==a?b:a)+1)%n;
            if (sites&bit(e)) continue;
            int leader=-1;
            for (int i=0;i<n;++i) if (single(domain[i]) &&
                (leader<0 || distTo(position(i),m)<distTo(position(leader),m))) leader=i;
            for (int t:{m,(e+1)%n}) {
                double c=0;
                for (int i=0;i<n;++i) {
                    if (!single(domain[i])) c+=1.5+0.5*(distTo(m,t)+distTo((e+1)%n,t));
                    else {
                        int x=position(i);
                        if (i==leader) c+=distTo(x,m)+distTo(m,t);
                        else if (x==e) c+=1+distTo((e+1)%n,t);
                        else c+=distTo(x,t);
                    }
                }
                if (c<best) best=c,match=m,emptySite=e,anchor=leader;
            }
        }
        if (position(anchor)!=match) scan(anchor,distTo(position(anchor),match));
        for (int i=0;i<n && countUnknown();++i) if (single(domain[i]) && position(i)==emptySite)
            scan(i,1);
        for (int i=0;i<n;++i) if (!single(domain[i])) {
            Run r=makeRun(i);
            advance(r);
            bool hit=r.ones!=0;
            narrow(i,bit(distTo(shift[i],hit?match:emptySite)));
            if (!hit && countUnknown()) advance(r);
            runs.push_back(move(r));
            propagate();
        }
        return true;
    }

    // Try to gather without ever advancing a dial past the chosen site.
    void gather(int t) {
        if (!countUnknown()) return;
        int scout=0;
        if (position(scout)!=t) scan(scout,distTo(position(scout),t));
        while (countUnknown()) {
            int p=-1;
            for (int i=0;i<n;++i) {
                Mask here=rot(domain[i],shift[i]);
                if (here&bit(t)) continue;
                if (p<0 || sizeOf(here)<sizeOf(domain[p])) p=i;
            }
            if (p<0) return;
            Run r=makeRun(p);
            Mask may=0;
            for (int i=0;i<n;++i) if (i!=p)
                may|=rot(domain[i],shift[i]);
            bool last=countUnknown()==1 && !single(domain[p]);
            while (!(rot(domain[p],shift[p])&bit(t))) {
                advance(r);
                int k=shift[p];
                bool hit=r.ones&bit(k);
                narrow(p,rot(hit?may:full^r.certain,n-k));
                if (last && single(domain[p])) break;
            }
            runs.push_back(move(r));
            propagate();
        }
    }

    int gatheringSite(int pass) const {
        Mask occ=0;
        for (int i=1;i<n;++i) occ|=rot(domain[i],shift[i]);
        int home=position(0);
        vector<int> learned(n);
        for (Mask d:domain) if (single(d)) ++learned[firstBit(d)];
        vector<double> prior(n);
        int nsites=0;
        for (Mask d:domain) if (!single(d)) ++nsites;
        Mask support=0;
        for (Mask d:domain) if (!single(d)) support|=d;
        int m=sizeOf(support);
        int need=0;
        for (Mask b=support;b;b&=b-1) need+=learned[firstBit(b)]==0;
        double alpha=m<0.45*n?2.5:0.8*m;
        double sum=0;
        for (Mask b=support;b;b&=b-1) sum+=alpha/m+learned[firstBit(b)];
        for (int x=0;x<n;++x) prior[x]=0.1+(learned[x]==0)
                           +max(0,nsites-need)*(alpha/m+learned[x])/sum;
        map<Mask,vector<double>> kinds;
        for (int i=1;i<n;++i) {
            Mask d=rot(domain[i],shift[i]);
            auto& freq=kinds[d];
            if (freq.empty()) freq.resize(n);
            double total=0;
            for (Mask b=d;b;b&=b-1) total+=prior[distTo(shift[i],firstBit(b))];
            for (Mask b=d;b;b&=b-1) {
                int x=firstBit(b);
                freq[x]+=prior[distTo(shift[i],x)]/total;
            }
        }
        int bestSite=home;
        double best=1e100;
        for (int t=0;t<n;++t) {
            if (pass && t==home) continue;
            Mask map=occ|bit(t);
            double score=distTo(home,t);
            for (auto& [d,weights]:kinds) {
                for (Mask b=d;b;b&=b-1) {
                    int x=firstBit(b),h=distTo(x,t);
                    // Weight the previous gathering site more heavily if this
                    // dial's earlier probe left several possible positions.
                    double wt=weights[x];
                    if (pass && (d&bit(home)))
                        wt*=x==home?(sizeOf(d)+1)*0.5:0.5;
                    int k=0;
                    Mask left=d;
                    while (!(left&bit((t-k+n)%n)) && !single(left)) {
                        ++k;
                        Mask hit=rot(map,n-k);
                        left &= (hit&bit(x))?hit:~hit;
                    }
                    double ambiguity=(k<h && !single(left))?1:0;
                    score+=wt*(h + ambiguity*(0.5*n+0.25*(h-k))
                                    + (single(d)?0:0.02*k));
                }
            }
            if (score<best) best=score,bestSite=t;
        }
        return bestSite;
    }

public:
    void solve();
};

void Tracker::solve() {
    full=bit(n)-1;
    shift.assign(n,0);
    effort.assign(n,0);
    domain.assign(n,full);
    domain[0]=1;
    Run first=makeRun(0);
    for (int k=1;k<=n;++k) {
        advance(first);
        if (n<=12 && surveyBeliefs(first)) return;
        if (sizeOf(first.ones)==n-1) break;
    }
    runs.push_back(move(first));
    propagate();
    if (sizeOf(domain[1])==n-1) {distinct(domain[1]);return;}
    Mask initial=domain[1];
    int largestGap=0;
    for (Mask b=initial;b;b&=b-1) {
        int p=firstBit(b),d=1;
        while (!(initial&bit((p+d)%n))) ++d;
        largestGap=max(largestGap,d);
    }
    bool regular=false;
    for (int k=1;k<n;++k) if (rot(initial,k)==initial) regular=true;
    int sampling=(largestGap>n/2 || regular)?0:min(n/3,3+(int)sqrt(n));
    bool gathered=false;
    int meeting=position(0);
    int lastScan=queries, lastProgress=queries;
    while (int u=countUnknown()) {
        if (resolvePair()) break;
        if (u<=5 && smallRemainder()) return;
        if (!gathered && (n-1-u>=sampling || queries>4*n)) {
            for (int pass=0;pass<3 && countUnknown();++pass) {
                if (resolvePair()) break;
                if (countUnknown()<=5 && smallRemainder()) return;
                int t=gatheringSite(pass);
                int before=countUnknown();
                gather(t);
                if (!countUnknown()) break;
                scan(0,n);
                gather(t);
                if (!countUnknown() || countUnknown()==before) break;
            }
            gathered=true;
            lastProgress=lastScan=queries; meeting=position(0);
            continue;
        }
        Mask must=occupied();
        vector<int> covers(n);
        vector<double> mass(n);
        vector<int> learned(n);
        for (Mask d:domain) if (single(d)) ++learned[firstBit(d)];
        vector<double> forecast(n);
        for (int i=0;i<n;++i) {
            Mask all=rot(domain[i],shift[i]);
            double total=0;
            for (Mask b=all;b;b&=b-1) {
                int x=firstBit(b);
                ++covers[x];
                total+=1+0.7*learned[distTo(shift[i],x)];
            }
            for (Mask b=all;b;b&=b-1) {
                int x=firstBit(b);
                double w=(1+0.7*learned[distTo(shift[i],x)])/total;
                mass[x]+=w;
                if (single(all)) { forecast[x]+=w; continue; }
                Mask left=all;
                int steps=0;
                while (!single(left) && steps<min(7,n-1)) {
                    ++steps;
                    Mask hit=rot(must,n-steps);
                    left &= (hit&bit(x))?hit:~hit;
                }
                // Some origins are resolved passively, through later queries.
                forecast[(x+steps)%n]+=0.8*w;
                forecast[x]+=0.2*w;
            }
        }
        vector<double> estimate(n),later(n);
        for (int t=0;t<n;++t) {
            for (int x=0;x<n;++x) {
                estimate[t]+=mass[x]*distTo(x,t);
                later[t]+=forecast[x]*distTo(x,t);
            }
        }
        int candidate=min_element(later.begin(),later.end())-later.begin();
        if (later[candidate]+0.6*n<later[meeting]) meeting=candidate;
        int target=min_element(estimate.begin(),estimate.end())-estimate.begin();
        // The uncertainty about the eventual cheapest target smooths the
        // penalty for crossing it during an exploratory run.
        vector<double> weight(n);
        double norm=0;
        for (int t=0;t<n;++t)
            norm += weight[t]=exp((estimate[target]-estimate[t])/max(1.0,0.22*n*sqrt(u)));
        for (double& w:weight) w/=norm;

        int p=-1;
        double top=-1e100;
        Mask background=0;
        for (int i=0;i<n;++i) if (!single(domain[i])) {
            Mask d=rot(domain[i],shift[i]);
            Mask may=0;
            for (int x=0;x<n;++x) if (covers[x]>bool(d&bit(x))) may|=bit(x);
            int sz=sizeOf(d);
            double value=0,risk=0;
            int horizon=min(n-1,8);
            for (int k=1;k<=horizon;++k) {
                Mask dest=rot(d,k);
                int hits=sizeOf(dest&must), miss=sizeOf(dest&~may);
                value += (double)(hits*miss+0.22*(hits+miss)*(sz-hits-miss))/(sz*sz*k);
            }
            for (Mask b=d;b;b&=b-1) risk+=weight[firstBit(b)]/sz;
            double score=5.0*value - ((d&bit(meeting)) && effort[i] ? 4 : 0) + 0.32/sqrt(sz) - 0.32*n*risk
                        - 0.032*effort[i];
            if (score>top) top=score,p=i,background=may;
        }
        Mask starts=rot(domain[p],shift[p]);
        // Put a located dial where its signal helps distinguish the remaining
        // origins. Also consider vacating a site when a known dial masks a zero.
        vector<vector<int>> groups(n);
        Mask fixed=0,unknownMay=0;
        for (int i=0;i<n;++i) {
            if (single(domain[i])) {
                groups[position(i)].push_back(i);
                fixed|=bit(position(i));
            } else if (i!=p) unknownMay|=rot(domain[i],shift[i]);
        }
        Mask clauseMust=0;
        for (auto& c:clauses) {
            int x=-1;
            for (auto l:c) {
                int y=(l.origin+shift[l.dial])%n;
                if (x==-1) x=y;
                else if (x!=y) {x=-2;break;}
            }
            if (x>=0) clauseMust|=bit(x);
        }
        double baseCost=probeCost(starts,must,background);
        int from=-1,to=-1,number=0;
        double improvement=0.28;
        if (baseCost>2.5 && queries-lastProgress<=max(55,3*n)) {
            for (int x=0;x<n;++x) if (!groups[x].empty()) {
                for (int k=1;k<n;++k) {
                    int y=(x+k)%n;
                    if (k>5 && y!=target && (k>12 || sizeOf(starts)>12)) continue;
                    for (int g:{1,(int)groups[x].size()}) {
                        Mask moved=fixed|bit(y);
                        if (g==(int)groups[x].size()) moved &= ~bit(x);
                        Mask lo=moved|clauseMust, hi=moved|unknownMay;
                        if (lo==must && hi==background) continue;
                        double expense=1e100;
                        for (int t=0;t<n;++t) {
                            double e=estimate[t]-estimate[target];
                            if (distTo(x,t)<k) e+=g*n;
                            expense=min(expense,e);
                        }
                        double price=(0.8*expense+0.21*g*k)/max(1.0,0.55*u);
                        if (price+improvement>=baseCost-1) continue;
                        double gain=baseCost-probeCost(starts,lo,hi)-price;
                        if (gain>improvement) improvement=gain,from=x,to=y,number=g;
                        if (groups[x].size()==1) break;
                    }
                }
            }
        }
        if (from!=-1) {
            int oldCount=u;
            for (int k=0;k<number;++k) scan(groups[from][k],distTo(from,to));
            if (countUnknown()<oldCount) lastProgress=queries;
            if (single(domain[p])) continue;
            must=occupied(); background=0;
            for (int j=0;j<n;++j) if (j!=p)
                background|=rot(domain[j],shift[j]);
        }
        if (queries-lastProgress>max(55,3*n)) {
            identifyOne(p);
            lastProgress=lastScan=queries;
            continue;
        }
        // Refresh absolute occupancy when too much of the map has become
        // ambiguous. Use a located dial whose survey ends near the target.
        if (from==-1 && queries-lastScan>=max(10,n/2) &&
            (sizeOf(background&~must)>n*0.32 || queries-lastProgress>n/2)) {
            int scout=0, length=n;
            double best=-1e100;
            for (int j=0;j<n;++j) if (single(domain[j])) {
                int x=position(j);
                double gain=0, penalty=0;
                for (int k=1;k<=n;++k) {
                    int v=(x+k)%n;
                    gain+=bool((background&~must)&bit(v));
                    penalty+=weight[(x+k-1)%n];
                    if (k<max(3,n/8)) continue;
                    double s=gain/(3.0+0.15*n+k*0.28) - penalty*0.85;
                    if (s>best) best=s,scout=j,length=k;
                }
            }
            scan(scout,length);
            lastScan=queries;
        } else {
            Run r=makeRun(p);
            int since=0;
            for (int k=1;k<n;++k) {
                int old=sizeOf(domain[p]);
                advance(r);
                Mask valid=0;
                bool hit=r.ones&bit(shift[p]);
                for (Mask b=domain[p];b;b&=b-1) {
                    int a=firstBit(b), x=(a+shift[p])%n;
                    if (hit ? bool(background&bit(x)) : !(must&bit(x))) valid|=bit(a);
                }
                narrow(p,valid);
                if (single(domain[p])) break;
                since=(old==sizeOf(domain[p]))?since+1:0;
                if (rot(domain[p],shift[p])&bit(meeting)) break;
                if (since>=5 && k>=6) break;
            }
            if (gathered && single(domain[p]) && countUnknown() && distTo(position(p),meeting)<n-max(3,n/6)) {
                int steps=distTo(position(p),meeting);
                for (int k=0;k<steps;++k) advance(r);
            }
            runs.push_back(move(r));
            propagate();
        }
        if (countUnknown()<u) lastProgress=queries;
    }
    vector<int> a(n);
    for (int i=0;i<n;++i) a[i]=position(i);
    finish(a);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin>>n) || n<3 || n>100) return 0;
    if (n<=8) Small().solve();
    else Tracker().solve();
}
