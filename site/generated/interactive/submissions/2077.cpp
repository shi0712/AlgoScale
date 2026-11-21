#include <bits/stdc++.h>
using namespace std;

using U = uint64_t;
using Wide = __uint128_t;
using Triple = array<U, 3>;
using Range = pair<U, U>;                 // half open
using Domain = vector<Range>;
using Query = tuple<U, U, U>;
static U n, total;
static vector<Query> history;
static mt19937_64 rng(213779321);

static int high(U x) { return 63 - __builtin_clzll(x); }
static U reply(const Triple &t, U l, U r) {
    U v = 0;
    for (U x : t) if (l <= x && x < r) v ^= x;
    return v;
}
static U ask(U l, U r) {
    cout << "xor " << l << ' ' << r - 1 << endl;
    long long v;
    if (!(cin >> v) || v < 0) exit(0);
    history.emplace_back(l, r, v);
    return v;
}
static void answer(Triple t) {
    sort(t.begin(), t.end());
    cout << "ans " << t[0] << ' ' << t[1] << ' ' << t[2] << endl;
    exit(0);
}
static bool valid(Triple t) {
    sort(t.begin(), t.end());
    if (t[0] == 0 || t[2] > n || t[0] == t[1] || t[1] == t[2]) return false;
    for (auto [l,r,v] : history) if (reply(t,l,r) != v) return false;
    return true;
}
static Domain restrictTo(const Domain &d, U l, U r, bool inside) {
    Domain res;
    for (auto [a,b] : d) {
        if (inside) {
            if (max(l,a) < min(r,b)) res.emplace_back(max(l,a),min(r,b));
        } else {
            if (a < min(l,b)) res.emplace_back(a,min(l,b));
            if (max(r,a) < b) res.emplace_back(max(r,a),b);
        }
    }
    return res;
}
struct Block { U lo, size; };
static vector<Block> decompose(const Domain &d) {
    vector<Block> blocks;
    for (auto [a,b] : d) {
        while (a < b) {
            U size = min(a & -a, U(1) << high(b-a));
            blocks.push_back({a,size});
            a += size;
        }
    }
    return blocks;
}
static int containing(const vector<Block> &b, U x) {
    auto it = upper_bound(b.begin(),b.end(),x,[](U v,Block t){return v<t.lo;});
    if (it == b.begin()) return -1;
    --it;
    return x-it->lo < it->size ? int(it-b.begin()) : -1;
}

// The two independent coordinates are a and b. c is determined by the total.
// When the last two blocks coincide, b uses just the lower endpoint of the pair.
struct Cell {
    U a, amask, b, bmask, skip;
    int group;
    Wide count() const { return Wide(amask+1) * ((bmask+1) >> (skip != 0)); }
    Triple make(U i, U j) const {
        if (skip) j = ((j & ~(skip-1)) << 1) | (j & (skip-1));
        U x = a | i, y = b | j;
        return {x,y,total^x^y};
    }
    Triple sample() const {
        return make(rng() & amask, rng() & (bmask >> (skip != 0)));
    }
};
// Order the blocks by size. Once the two smaller blocks are chosen, XOR fixes
// all the bits above their widths in the third coordinate. A compatible
// third block therefore contains every combination of the two free offsets.
// The first two blocks cannot coincide: the third would then contain total.
static void addTriples(const Domain &d, int group, vector<Cell> &cells) {
    auto b = decompose(d);
    vector<int> order(b.size()), rank(b.size());
    iota(order.begin(),order.end(),0);
    sort(order.begin(),order.end(),[&](int i,int j) {
        return tie(b[i].size,b[i].lo) < tie(b[j].size,b[j].lo);
    });
    for (int i=0; i<(int)b.size(); ++i) rank[order[i]]=i;
    for (int j=0; j<(int)b.size(); ++j) {
        Block y=b[order[j]];
        for (int i=0; i<j; ++i) {
            Block x=b[order[i]];
            int k=containing(b,total^x.lo^y.lo);
            if (k<0 || rank[k]<j) continue;
            U skip = k==order[j] ? U(1)<<high(x.lo^total) : 0;
            cells.push_back({x.lo,x.size-1,y.lo,y.size-1,skip,group});
        }
    }
}
static void addPairs(U s, int group, vector<Cell> &cells) {
    if (!s || s>n || s==total) return;
    U diff=s^total;
    Domain d={{1,n+1}};
    d=restrictTo(d,s,s+1,false);
    d=restrictTo(d,total,total+1,false);
    for (auto [l,r,v] : history) {
        U p = v ^ (l<=s && s<r ? s : 0);
        if (p != 0 && p != diff) {
            if (valid({s,p,p^diff})) cells.push_back({s,0,p,0,0,group});
            return;
        }
        d=restrictTo(d,l,r,p==diff);
        if (d.empty()) return;
    }
    auto b=decompose(d);
    for (int i=0; i<(int)b.size(); ++i) {
        int j=containing(b,b[i].lo^diff);
        if (j<0 || tie(b[j].size,b[j].lo)<tie(b[i].size,b[i].lo)) continue;
        U skip = i==j ? U(1)<<high(diff) : 0;
        cells.push_back({s,0,b[i].lo,b[i].size-1,skip,group});
    }
}

// Every possible pair straddles the center of one block of size 2*half.
// Probing two centers recovers either endpoint pair immediately. All other
// blocks are divided into the blocks between the centers and their complement.
static Range pairQuery(const vector<Cell> &cells) {
    U s=cells[0].a, half=U(1)<<high(s^total), size=2*half;
    struct Run { U start,len,weight,rank,mass; };
    vector<Run> input, runs;
    for (auto &c:cells) {
        U len=max(U(1),(c.bmask+1)/size);
        input.push_back({c.b/size,len,U(c.count())/len,0,0});
    }
    sort(input.begin(),input.end(),[](auto &a,auto &b){return a.start<b.start;});
    for (auto run:input) {
        if (!runs.empty() && runs.back().start==run.start)
            runs.back().weight+=run.weight;
        else runs.push_back(run);
    }
    U m=0, sum=0;
    for (auto &run:runs) {
        run.rank=m; run.mass=sum;
        m+=run.len; sum+=run.len*run.weight;
    }
    auto center=[&](U block){return block*size+half;};
    if (m==1) return {1,center(runs[0].start)};
    // A distance prior also represents pairs close to the singleton or an
    // endpoint. Its discrete mass telescopes, even over very long runs.
    auto radial=[](U start,U end,U anchor) {
        auto tail=[](U distance,U len) {
            return double(len)/((1.0+distance)*(1.0+distance+len));
        };
        double mass=0;
        if (start<anchor) {
            U stop=min(end,anchor);
            mass+=tail(anchor-stop+1,stop-start);
        }
        if (end>anchor) {
            U left=max(start,anchor);
            mass+=tail(left-anchor,end-left);
        }
        return mass;
    };
    auto proximity=[&](const Run &r,U len) {
        U end=r.start+len;
        return double(r.weight)/half*(0.32*radial(r.start,end,s/size)+
                    0.055*radial(r.start,end,0)+0.055*radial(r.start,end,n/size));
    };
    vector<double> cumulative(1,0);
    for (auto &r:runs) cumulative.push_back(cumulative.back()+proximity(r,r.len));
    struct Pivot { U center, mass, weight; double nearBefore, nearWeight; };
    auto pivot=[&](U rank) {
        auto it=prev(upper_bound(runs.begin(),runs.end(),rank,
                    [](U v,const Run &r){return v<r.rank;}));
        U off=rank-it->rank;
        double nearBefore=cumulative[it-runs.begin()]+proximity(*it,off);
        Run point={it->start+off,1,it->weight,0,0};
        return Pivot{center(point.start),it->mass+off*it->weight,it->weight,
                     nearBefore,proximity(point,1)};
    };
    // Keep extrema and simple subdivisions as possible probes. Probing one
    // particular block still permits a balanced split with the other endpoint.
    map<U,double> preferred;
    auto anchor=[&](U x,double w) {
        U key=x/size;
        auto it=upper_bound(runs.begin(),runs.end(),key,
                           [](U v,const Run &r){return v<r.start;});
        if (it!=runs.begin()) {
            const Run &r=*prev(it);
            if (key<r.start+r.len) {
                U z=r.rank+key-r.start;
                preferred[z]+=w;
                if (z>0) preferred[z-1]+=0.20*w;
                if (z+1<m) preferred[z+1]+=0.20*w;
                return;
            }
            U gap=key-(r.start+r.len-1);
            preferred[r.rank+r.len-1]+=w/pow(1+0.45*log2(1.0+gap),2);
        }
        if (it!=runs.end()) {
            U gap=it->start-key;
            preferred[it->rank]+=w/pow(1+0.45*log2(1.0+gap),2);
        }
    };
    anchor(1,0.42); anchor(n,0.42);
    anchor(s,0.70); anchor(total,0.28);
    anchor(n/2,0.16); anchor(n/3,0.08); anchor(n*2/3,0.08);
    anchor(n/4,0.07); anchor(n*3/4,0.07);
    for (U x=U(1)<<high(n); x>=size && x; x>>=1) {
        double w=0.12/(1+high(n)-high(x));
        anchor(x,w);
        anchor(x-1,w);
    }
    vector<U> ranks;
    if (m<=90) {
        for (U j=0;j<m;++j) ranks.push_back(j);
    } else {
        for (int j=0;j<16;++j) ranks.push_back((m/16)*j+(m%16)*j/16);
        for (auto [j,w]:preferred) ranks.push_back(j);
        for (auto &r:runs) {
            if (r.weight>half/4 && r.len>m/8) ranks.push_back(r.rank+r.len/2);
        }
        for (int j=1;j<16;++j) {
            double target=cumulative.back()*j/16;
            U lo=0, hi=m-1;
            while (lo<hi) {
                U mid=(lo+hi)/2;
                auto p=pivot(mid);
                if (p.nearBefore+p.nearWeight<target) lo=mid+1;
                else hi=mid;
            }
            ranks.push_back(lo);
        }
        size_t k=ranks.size();
        for (size_t j=0;j<k;++j) ranks.push_back((ranks[j]+m/2)%m);
        sort(ranks.begin(),ranks.end());
        ranks.erase(unique(ranks.begin(),ranks.end()),ranks.end());
    }
    vector<Pivot> piv;
    vector<double> bonus;
    for (U j:ranks) {
        piv.push_back(pivot(j));
        auto it=preferred.find(j);
        bonus.push_back(it==preferred.end()?0:it->second);
    }
    double uniformUnit=1.25/n;
    double massTotal=cumulative.back()+uniformUnit*sum;
    auto entropyTerm=[](double p){return p>0 ? p*log2(p) : 0.0;};
    int depth=high(m+1);
    U capacity=(U(1)<<depth)-2;
    double bestScore=1e100;
    Range best{};
    for (int a=0;a<(int)ranks.size();++a) for (int b=a+1;b<(int)ranks.size();++b) {
        U between=ranks[b]-ranks[a]-1, outside=m-2-between;
        if (max(between,outside)>capacity) continue;
        U inner=piv[b].mass-piv[a].mass-piv[a].weight;
        U outer=sum-piv[a].weight-piv[b].weight-inner;
        double innerNear=piv[b].nearBefore-piv[a].nearBefore-piv[a].nearWeight;
        double p=(uniformUnit*inner+innerNear)/massTotal;
        double x=(uniformUnit*piv[a].weight+piv[a].nearWeight)/massTotal;
        double y=(uniformUnit*piv[b].weight+piv[b].nearWeight)/massTotal;
        double q=1-p-x-y;
        double score=entropyTerm(p)+entropyTerm(q)+entropyTerm(x)+entropyTerm(y)-x-y
                    -0.11*(bonus[a]+bonus[b])*min(1.0,double(m-2)/12);
        if (inner==1) score-=p;
        if (outer==1) score-=q;
        if (score<bestScore) {
            bestScore=score; best={piv[a].center,piv[b].center};
        }
    }
    return best;
}

struct Example { Triple t; int group; };
static vector<Example> examples(const vector<Cell> &cells, Wide count, bool pairMode,
                                const vector<Domain> &domains) {
    vector<Example> out;
    if (count<=768) {
        for (const Cell &c : cells)
            for (U i=0; i<=c.amask; ++i)
                for (U j=0; j<=(c.bmask>>(c.skip!=0)); ++j)
                    out.push_back({c.make(i,j),c.group});
        return out;
    }
    vector<array<double,4>> raw;
    array<double,4> sum{};
    for (auto &c : cells) {
        Triple t=c.make(0,0);
        int h=high(max({t[0]^total,t[1]^total,t[2]^total}));
        int l=high(min({t[0]^total,t[1]^total,t[2]^total}));
        double w=double(c.count());
        double perShell=ldexp(w,-2*h);
        double perTwoShells=ldexp(w,-h-l)/max(h,1);
        array<double,4> component={w,perShell,perTwoShells,perShell/pow(h+0.5,2)};
        raw.push_back(component);
        for (int j=0;j<4;++j) sum[j]+=component[j];
    }
    // Condition a mixture of uniform and distance-scale distributions on
    // every pure reply so far. Tiny clusters retain representation.
    static array<double,4> initial{};
    if (!initial[0]) initial=sum;
    vector<double> weight;
    for (auto w:raw) weight.push_back(pairMode ? w[0] :
        0.65*w[0]/initial[0]+0.10*w[1]/initial[1]+
        0.13*w[2]/initial[2]+0.12*w[3]/initial[3]);
    vector<Cell> biased;
    vector<double> prior;
    if (!pairMode) {
        auto groupOf=[&](U s) {
            for (int g=0;g<(int)domains.size();++g)
                for (auto [a,b]:domains[g]) if (a<=s && s<b) return g;
            return -1;
        };
        static map<U,double> initialPairs;
        for (U s:{U(1),n,total^1}) {
            int g=groupOf(s);
            if (g<0) continue;
            size_t start=biased.size();
            addPairs(s,g,biased);
            double mass=0;
            for (size_t i=start;i<biased.size();++i) mass+=double(biased[i].count());
            if (!mass) continue;
            double initialMass=initialPairs.emplace(s,mass).first->second;
            for (size_t i=start;i<biased.size();++i)
                prior.push_back(0.025*double(biased[i].count())/initialMass);
        }
        // Give short runs and library endpoints representation even in a
        // universe too large for uniform sampling to include them.
        set<Triple> specific;
        auto suggest=[&](U a,U b) {
            Triple t={a,b,total^a^b};
            sort(t.begin(),t.end());
            if (valid(t)) specific.insert(t);
        };
        suggest(1,2); suggest(n-1,n); suggest(1,n);
        suggest(total^1,total^2);
        U v=total^1;
        if (v>=3) suggest(v,v-2);
        suggest(v,v+2);
        for (auto t:specific) {
            int g=groupOf(t[0]);
            if (g>=0) {
                biased.push_back({t[0],0,t[1],0,0,g});
                prior.push_back(0.015);
            }
        }
        double biasMass=accumulate(prior.begin(),prior.end(),0.0);
        double baseMass=accumulate(weight.begin(),weight.end(),0.0);
        double scale=biasMass>0 ? min(1.0,baseMass/biasMass) : 0;
        for (double w:prior) weight.push_back(scale*w);
    }
    discrete_distribution<int> dist(weight.begin(),weight.end());
    for (int i=0; i<480; ++i) {
        int idx=dist(rng);
        auto &c=idx<(int)cells.size()?cells[idx]:biased[idx-cells.size()];
        out.push_back({c.sample(),c.group});
    }
    // Include other singleton hypotheses producing the same split-pair replies.
    if (pairMode) {
        map<int,U> singles;
        for (auto &c : cells) singles[c.group]=c.a;
        if (singles.size()==2) {
            set<Triple> seen;
            vector<Example> closure;
            for (auto e : out) for (auto [g,s] : singles) {
                for (U mask : {U(0), total, singles.begin()->second, singles.rbegin()->second}) {
                    U p=e.t[1]^mask;
                    Triple t={s,min(p,p^s^total),max(p,p^s^total)};
                    if (seen.insert(t).second && valid(t)) closure.push_back({t,g});
                }
            }
            out.swap(closure);
        }
    }
    return out;
}

static void quantiles(vector<U> vals, int parts, set<U> &cuts) {
    if (vals.empty()) return;
    sort(vals.begin(),vals.end());
    for (int i=0; i<=parts; ++i) cuts.insert(vals[(vals.size()-1)*i/parts]);
}
static vector<U> cutList(const vector<Example> &ex, bool exact, bool pairMode) {
    set<U> cuts={1,n+1};
    vector<U> vals, mids;
    map<U,int> popularity;
    for (auto &e : ex) {
        for (U v : e.t) vals.push_back(v);
        for (int j=0; j<3; ++j) {
            if (pairMode && j!=0) continue;
            U a=e.t[(j+1)%3], b=e.t[(j+2)%3];
            U half=U(1)<<high(a^b);
            U mid=(a & ~(2*half-1)) + half;
            mids.push_back(mid);
            ++popularity[mid];
        }
    }
    if (exact && (ex.size()<=40 || n<=64)) {
        cuts.insert(vals.begin(),vals.end());
    } else {
        quantiles(vals,16,cuts);
        quantiles(mids,16,cuts);
        vector<pair<int,U>> freq;
        for (auto [x,k]:popularity) freq.emplace_back(-k,x);
        sort(freq.begin(),freq.end());
        for (int i=0;i<min(12,(int)freq.size());++i) cuts.insert(freq[i].second);
        if (!pairMode) {
            for (int k=1;k<8;++k) cuts.insert(1+n*k/8);
            cuts.insert(2); cuts.insert(3); cuts.insert(n); cuts.insert(n-1);
            for (U v: {total^1,total^2,total^3}) if (v && v<=n) {cuts.insert(v);cuts.insert(v+1);}
            if (total && total<=n) {cuts.insert(total);cuts.insert(total+1);}
        }
    }
    return {cuts.begin(),cuts.end()};
}

// Exact finite candidate sets, and the sampled pair model, share this query scorer.
// The lookup table gives each unsolved response a cost based on its multiplicity.
static Range finiteQuery(const vector<Example> &ex, const vector<U> &cuts,
                         bool exact, const vector<Cell> &cells) {
    int m=ex.size(), k=cuts.size();
    map<U,int> ids;
    vector<array<int,8>> id(m);
    for (int i=0;i<m;++i) {
        for (int mask=0;mask<8;++mask) {
            U v=0;
            for (int j=0;j<3;++j) if (mask>>j&1) v^=ex[i].t[j];
            auto it=ids.emplace(v,(int)ids.size()).first;
            id[i][mask]=it->second;
        }
    }
    vector<vector<unsigned char>> pre(k,vector<unsigned char>(m));
    for (int p=0;p<k;++p) for (int i=0;i<m;++i)
        for (int j=0;j<3;++j) pre[p][i]|=(ex[i].t[j]<cuts[p])<<j;
    vector<int> hist(ids.size()), touched(m);
    vector<double> cost(m+1);
    for (int j=2;j<=m;++j) cost[j]=j*(0.8+0.45*log2(j));
    // Non-split pair responses can leave many more candidates than the sample.
    vector<int> heavy;
    double blocks=0;
    if (!exact) {
        for (auto &c:cells) {
            U size=U(1)<<(high(c.a^total)+1);
            blocks+=double(c.count())/(size/2);
            for (U v : {U(0),total,c.a,c.a^total}) {
                auto it=ids.find(v);
                if (it!=ids.end()) heavy.push_back(it->second);
            }
        }
        sort(heavy.begin(),heavy.end()); heavy.erase(unique(heavy.begin(),heavy.end()),heavy.end());
    }
    vector<double> extra(m+1);
    for (int j=1;j<=m && !exact;++j) {
        double b=max(1.0,blocks*j/m);
        extra[j]=j*(1+max(0.0,log2(b)-1.1)*0.93)-cost[j];
    }
    Range best={1,n+1};
    double bestScore=1e100;
    for (int a=0;a<k;++a) for (int b=a+1;b<k;++b) {
        int used=0;
        for (int i=0;i<m;++i) {
            int v=id[i][pre[a][i]^pre[b][i]];
            if (!hist[v]++) touched[used++]=v;
        }
        double score=0;
        for (int j=0;j<used;++j) score+=cost[hist[touched[j]]];
        for (int v:heavy) score+=extra[hist[v]];
        if (used==1) score+=1e9;
        if (score<bestScore) {bestScore=score;best={cuts[a],cuts[b]};}
        for (int j=0;j<used;++j) hist[touched[j]]=0;
        if (exact && used==m) return best;
    }
    return best;
}

struct Side {
    U size=0, first=0, last=0;
    void add(U a,U b) {
        if (a>=b) return;
        if (!size) first=a;
        last=b-1;
        size+=b-a;
    }
};
static Range tripleQuery(const vector<Example> &ex, const vector<U> &cuts,
                         const vector<Domain> &domains) {
    const int groups=domains.size(), k=cuts.size(), m=ex.size();
    struct Info { int h[3]; double nearCost[3]; bool alt[3]; };
    vector<Info> info(m);
    vector<vector<unsigned char>> pre(k,vector<unsigned char>(m)), altPre=pre;
    for (int i=0;i<m;++i) for (int j=0;j<3;++j) {
        U s=ex[i].t[j], d=total^s;
        info[i].h[j]=high(d);
        U half=U(1)<<high(d), block=ex[i].t[(j+1)%3]/(2*half);
        U near=min(block, n/(2*half)-block);
        U anchor=s/(2*half);
        near=min(near,max(block,anchor)-min(block,anchor));
        info[i].nearCost[j]=1.25+min(2.5,0.5*log2(1.0+near));
        info[i].alt[j]=false;
        if (total && d<=n) for (auto [a,b]:domains[ex[i].group])
            if (a<=d && d<b) {info[i].alt[j]=true;break;}
        for (int p=0;p<k;++p) {
            pre[p][i]|=(s<cuts[p])<<j;
            altPre[p][i]|=(d<cuts[p])<<j;
        }
    }
    struct SideCost { U maxXor; double pair[60]; };
    vector<array<SideCost,2>> sc(groups);
    double bestScore=1e100;
    Range best={1,n+1};
    for (int a=0;a<k;++a) for (int b=a+1;b<k;++b) {
        U l=cuts[a], r=cuts[b];
        for (int g=0;g<groups;++g) {
            Side side[2];
            for (auto [u,v]:domains[g]) {
                side[0].add(u,min(v,l)); side[0].add(max(u,r),v);
                side[1].add(max(u,l),min(v,r));
            }
            for (int p=0;p<2;++p) {
                U bits=side[p].first^side[p].last;
                sc[g][p].maxXor=bits ? (U(2)<<high(bits))-1 : 0;
                double lg=log2(double(max(U(1),side[p].size)));
                for (int h=0;h<60;++h)
                    sc[g][p].pair[h]=1 + clamp((lg-h-1.9)*0.83,0.0,3.0);
            }
        }
        double score=0;
        int pure[2]={};
        for (int i=0;i<m;++i) {
            int mask=pre[a][i]^pre[b][i];
            if (mask==0 || mask==7) {
                ++pure[total && mask==7];
                continue;
            }
            bool in=!(mask&(mask-1)); // exactly the singleton lies inside
            int j=__builtin_ctz(in?mask:7^mask);
            int g=ex[i].group;
            score+=min(info[i].nearCost[j],sc[g][!in].pair[info[i].h[j]]);
            if (info[i].alt[j] && ex[i].t[j]<=sc[g][in].maxXor &&
                (((altPre[a][i]^altPre[b][i])>>j&1)!=in)) score+=0.55;
        }
        for (int cnt:pure) if (cnt)
            score+=cnt*(3.85+0.38*log2(double(cnt)/m));
        if (score<bestScore) {bestScore=score;best={l,r};}
    }
    return best;
}

// Relative to total, the three coordinates XOR to zero. Two share the
// highest occupied bit, and the third has a lower highest bit. Each bit
// class is a contiguous interval. A cut between the two classes separates
// the singleton from the pair.
static Range shellQuery(const vector<Cell> &cells,const vector<Example> &ex) {
    auto start=[](U x) {
        U bit=U(1)<<high(x^total);
        return max(U(1),x & ~(bit-1));
    };
    // For each possible pair of classes, at least one cut must fall between
    // them. Greedy interval stabbing finds a minimum set of such cuts.
    map<U,U> left;
    for (const auto &c:cells) {
        Triple t=c.make(0,0);
        for (U &x:t) x=start(x);
        auto [a,b]=minmax_element(t.begin(),t.end());
        left[*b]=max(left[*b],*a);
    }
    vector<U> cut;
    for (auto [right,lower]:left)
        if (cut.empty() || cut.back()<=lower) cut.push_back(right);
    if (cut.size()==1) return {1,cut[0]};
    vector<Range> options;
    if (cut.size()==2) {
        options={{1,cut[0]},{1,cut[1]},{cut[0],cut[1]}};
    } else {
        // With a third cut between the two probes, every class pair not
        // separated by this query is still hit by some unprobed cut.
        for (int i=0;i<(int)cut.size();++i)
            for (int j=i+2;j<(int)cut.size();++j) options.emplace_back(cut[i],cut[j]);
    }
    Range best=options.front();
    double bestScore=-1;
    for (auto [l,r]:options) {
        int mixed=0,inside=0;
        for (auto &e:ex) {
            U v=reply(e.t,l,r);
            if (v!=0 && v!=total) ++mixed;
            else if (total && v==total) ++inside;
        }
        double score=mixed+0.45*min(inside,(int)ex.size()-mixed-inside);
        if (score>bestScore) {bestScore=score;best={l,r};}
    }
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin>>n) || n<3 || n>1000000000000000000ULL) return 0;
    if (n<=32) {
        vector<Example> ex;
        for (U a=1;a<=n;++a) for (U b=a+1;b<=n;++b) for (U c=b+1;c<=n;++c)
            ex.push_back({{a,b,c},0});
        while (ex.size()>1) {
            auto cuts=cutList(ex,true,false);
            auto [l,r]=finiteQuery(ex,cuts,true,{});
            U v=ask(l,r);
            erase_if(ex,[&](const Example &e){return reply(e.t,l,r)!=v;});
            if (ex.empty()) return 0;
        }
        answer(ex[0].t);
    }
    // A stolen type cannot equal the total: the other two would be equal.
    total=ask(1,n+1);
    vector<Domain> domains={restrictTo({{1,n+1}},total,total+1,false)};
    vector<U> singles;
    int ambiguousQueries=0;
    while (true) {
        vector<Cell> cells;
        if (singles.empty()) {
            vector<Domain> live;
            for (auto &d:domains) {
                size_t before=cells.size();
                addTriples(d,live.size(),cells);
                if (cells.size()!=before) live.push_back(move(d));
            }
            domains=move(live);
        } else {
            for (int i=0;i<(int)singles.size();++i) addPairs(singles[i],i,cells);
        }
        if (cells.empty()) return 0;
        Wide count=0;
        for (auto &c:cells) count+=c.count();
        if (count==1) answer(cells.front().make(0,0));
        if (singles.empty()) {
            for (U s:cells[0].make(0,0)) {
                bool common=all_of(cells.begin(),cells.end(),[&](const Cell &c){
                    Triple t=c.make(0,0);
                    return find(t.begin(),t.end(),s)!=t.end();
                });
                if (!common) continue;
                vector<Cell> with;
                addPairs(s,0,with);
                Wide matches=0;
                for (const auto &c:with) matches+=c.count();
                if (matches==count) {
                    singles={s}; cells=move(with); break;
                }
            }
        }
        bool pairMode=!singles.empty(), exact=count<=768;
        if (pairMode && cells.front().group==cells.back().group) {
            auto [l,r]=pairQuery(cells);
            ask(l,r);
            continue;
        }
        if (pairMode && ++ambiguousQueries>=4) {
            U s=singles[0];
            ask(s,s+1);
            continue;
        }
        auto ex=examples(cells,count,pairMode,domains);
        U l=0,r=0;
        if (!pairMode && history.size()>=10) {
            tie(l,r)=shellQuery(cells,ex);
        } else {
            auto cuts=cutList(ex,exact,pairMode);
            tie(l,r)=exact || pairMode ? finiteQuery(ex,cuts,exact,cells)
                                      : tripleQuery(ex,cuts,domains);
        }
        // Any two different witnesses can be separated by a singleton query.
        // This also handles samples concentrated in a single response bucket.
        auto first=ex.front().t;
        if ((pairMode || history.size()<10) && all_of(ex.begin(),ex.end(),[&](const Example &e){
            return reply(e.t,l,r)==reply(first,l,r);
        })) {
            Triple second=cells.back().make(cells.back().amask,
                                            cells.back().bmask>>(cells.back().skip!=0));
            first=cells.front().make(0,0);
            for (U x:first) if (find(second.begin(),second.end(),x)==second.end()) {
                l=x;r=x+1;break;
            }
        }
        U v=ask(l,r);
        if (pairMode) continue;
        // A split has a singleton on one side. Its label is the XOR
        // of that side. An unsplit triple lies wholly in one new domain.
        if (v!=0 && v!=total) {
            if (l<=v && v<r) singles.push_back(v);
            U d=v^total;
            if (d && d<=n && (d<l || d>=r)) singles.push_back(d);
            if (singles.empty()) return 0;
        } else {
            vector<Domain> next;
            for (const auto &d:domains) {
                if (v==0) next.push_back(restrictTo(d,l,r,false));
                if (v==total) next.push_back(restrictTo(d,l,r,true));
            }
            domains=move(next);
        }
    }
}
