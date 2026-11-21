#include <bits/stdc++.h>
using namespace std;

namespace {
// Known labels serve as references. Fresh labels have not been counted.
// Every other label belongs to a measured pool with an exact species total.
// Probability estimates only select the next measurement.
int n, lengthUsed;
vector<int> species, known[2], fresh;
int nextFresh;
mt19937 rng(19260817);

int ask(const vector<int>& path) {
    cout << "? " << path.size();
    for (int i : path) cout << ' ' << i;
    cout << endl;
    int r;
    if (!(cin >> r) || r < 0 || r >= (int)path.size()) exit(0);
    lengthUsed += path.size();
    return r;
}
void finish(int countA) {
    cout << "! " << countA << endl;
    exit(0);
}
void identify(int label, int b) {
    if (species[label] == -1) {
        species[label] = b;
        known[b].push_back(label);
    }
}
int remaining() { return (int)fresh.size() - nextFresh; }
int take() { return fresh[nextFresh++]; }

// Small decision trees obtained by enumerating paths and partitioning the
// possible assignments by their response. Negative children encode answers.
void smallTree() {
    struct Node { const char* path; array<int,8> child; };
    static const vector<Node> tree[] = {
        {
            {"01", {-2, -1}},
        },
        {
            {"102", {-3, -2, -1}},
        },
        {
            {"0123", {-4, 1, 2, -2}},
            {"021", {-3, -1, -2}},
            {"0132", {0, -3, -3, -2}},
        },
        {
            {"01234", {-5, 1, 2, 3, -3}},
            {"0312", {-4, -1, -3, -2}},
            {"10243", {0, -4, -4, -3, -2}},
            {"01342", {0, -3, -2, -2, -3}},
        },
        {
            {"102345", {-6, 1, 2, 5, 7, -3}},
            {"014253", {0, -2, -5, -3, -4, 0}},
            {"012534", {0, 3, -5, 4, -4, -3}},
            {"01235", {-5, -1, -4, 0, 0}},
            {"01", {-3, -2}},
            {"205143", {0, -3, 6, -4, -3, -3}},
            {"012", {0, -2, -4}},
            {"013542", {0, -4, -3, -2, 0, -3}},
        },
        {
            {"0123456", {-7, 1, 2, 4, 7, 9, -4}},
            {"041523", {-6, -1, -5, -2, -4, -3}},
            {"2036514", {0, -6, -6, -5, 3, -4, -3}},
            {"015", {-4, -2, -3}},
            {"1602534", {0, -5, -2, 5, 6, -3, -4}},
            {"102453", {0, 0, -5, -4, -2, -3}},
            {"2154603", {0, -2, -3, -5, -2, -4, -3}},
            {"2503614", {0, -5, 8, -4, -5, -3, 0}},
            {"013245", {0, -4, -5, -3, -4, 0}},
            {"0136245", {0, 0, -4, -3, -3, 0, -4}},
        },
        {
            {"01234567", {-8, 1, 2, 4, 9, 13, 16, -4}},
            {"0415263", {-7, -1, -6, -2, -5, -3, -4}},
            {"25037614", {0, -7, -7, -6, 3, -5, -3, -4}},
            {"01756", {-5, -6, -4, -2, -3}},
            {"2314605", {0, 5, 6, 7, 8, -4, -3}},
            {"0152634", {0, 0, -6, -2, -5, -3, -4}},
            {"01234576", {0, 0, -2, -5, -6, 0, 0, 0}},
            {"0246713", {0, -2, -3, -3, -5, -4, 0}},
            {"02174635", {0, 0, -4, -2, -5, -3, 0, -4}},
            {"01367245", {0, -5, -6, 10, 11, 12, -5, -4}},
            {"0326714", {0, -5, -6, -4, -5, -3, 0}},
            {"1025374", {0, -6, -6, -5, -5, -4, -3}},
            {"312654", {0, -3, -5, -4, -5, -5}},
            {"02473561", {0, -3, -5, 14, -4, 15, -5, -4}},
            {"325604", {0, -5, -4, -4, -3, -3}},
            {"012534", {0, 0, -3, -3, -5, -4}},
            {"01346725", {0, 0, -4, -5, -5, -4, -4, 0}},
        },
    };
    const auto& nodes=tree[n-2];
    int node=0;
    do {
        vector<int> path;
        for(const char* p=nodes[node].path;*p;++p) path.push_back(*p-'0');
        node=nodes[node].child[ask(path)];
        if(node==0) exit(0);
    } while(node>0);
    finish(-node);
}

// Search small decision trees by comparing the partitions made by possible paths.
void tiny(vector<int> vars, bool countOnly, int references, int changes=-1) {
    int d = vars.size(), limit = 1 << d;
    vector<int> labels = vars, fixed;
    for (int b = 0; b < 2; ++b)
        for (int j = 0; j < min(references, (int)known[b].size()); ++j) {
            labels.push_back(known[b][j]);
            fixed.push_back(b);
        }
    int N = labels.size();
    vector<int> states(limit), order(N);
    iota(states.begin(), states.end(), 0);
    iota(order.begin(), order.end(), 0);
    if(changes>=0) {
        erase_if(states,[&](int x) {
            unsigned pattern=(x<<1)|((changes&1)<<(n-1));
            return __builtin_popcount((pattern^(pattern>>1))&((1u<<(n-1))-1))!=changes;
        });
    }
    vector<int> weight(limit);
    for (int x = 1; x < limit; ++x) weight[x] = weight[x >> 1] + (x & 1);
    for (int round=0;;++round) {
        bool resolved = true;
        for (int x : states)
            if (countOnly ? weight[x] != weight[states[0]] : x != states[0]) {
                resolved = false;
                break;
            }
        if (resolved) {
            if (countOnly) finish(n - (int)known[1].size() - weight[states[0]]);
            for (int j = 0; j < d; ++j) identify(vars[j], (states[0] >> j) & 1);
            return;
        }
        if(round>=12) {
            // Each such probe fixes a new coordinate, even if an adaptive
            // sequence of replies left a very uneven candidate set.
            int difference=0;
            for(int x:states) difference|=x^states.front();
            int j=__builtin_ctz((unsigned)difference);
            int b=ask({known[0][0],vars[j]});
            erase_if(states,[&](int x){return ((x>>j)&1)!=b;});
            continue;
        }
        vector<int> sample;
        if(states.size()>700) {
            sample=states;
            shuffle(sample.begin(),sample.end(),rng);
            sample.resize(700);
            // Keep a witness for every possible answer in the sample.
            int represented=0;
            for(int x:states) if(!(represented&(1<<weight[x]))) {
                sample.push_back(x); represented|=1<<weight[x];
            }
        }
        const vector<int>& evaluate=sample.empty()?states:sample;
        double bestScore = 1e100;
        vector<int> bestPath;
        vector<unsigned char> outcomes(limit);
        auto consider = [&](int k) {
            if (order[0] > order[k-1]) return;
            unsigned pos[24]{}, constant = 0;
            for (int i = 0; i < k; ++i) {
                int v = order[i];
                if (v < d) pos[v] = 1u << i;
                else if (fixed[v-d]) constant |= 1u << i;
            }
            unsigned lo[512], hi[512];
            lo[0]=hi[0]=0;
            int h = d / 2;
            for (int i = 1; i < (1 << h); ++i)
                lo[i] = lo[i & (i-1)] | pos[__builtin_ctz(i)];
            for (int i = 1; i < (1 << (d-h)); ++i)
                hi[i] = hi[i & (i-1)] | pos[h + __builtin_ctz(i)];
            int sizes[32]{}, first[32];
            bool mixed[32]{};
            auto result = [&](int x) {
                unsigned pattern = lo[x & ((1 << h)-1)] | hi[x >> h] | constant;
                return __builtin_popcount((pattern ^ (pattern >> 1)) & ((1u << (k-1))-1));
            };
            for (int x : evaluate) {
                int r = result(x);
                int key = countOnly ? weight[x] : x;
                if (!sizes[r]++) first[r] = key;
                else if (first[r] != key) mixed[r] = true;
            }
            double score = k * 0.000001;
            int worst = 0;
            for (int i = 0; i < k; ++i) if (mixed[i]) {
                score += sizes[i] * (1 + log2(sizes[i]) * 0.45);
                worst = max(worst, sizes[i]);
            }
            score += 0.4 * worst;
            if (score < bestScore) {
                bestScore = score;
                bestPath.clear();
                for (int i = 0; i < k; ++i) bestPath.push_back(labels[order[i]]);
                for (int x : states) outcomes[x] = result(x);
            }
        };
        for (int j = 0; j < d; ++j) {
            iota(order.begin(), order.end(), 0);
            swap(order[0], order[j]);
            swap(order[1], order[d]);
            consider(2);
        }
        for (int t = 0; t < 3200; ++t) {
            shuffle(order.begin(), order.end(), rng);
            int k = t % 3 == 0 ? N : 2 + rng() % (N-1);
            consider(k);
        }
        int r = ask(bestPath);
        erase_if(states, [&](int x) { return outcomes[x] != r; });
    }
}

struct Interval { int l, r, changes; };

// With one change, visiting small and large indices alternately makes
// the cut size unique. Left-half cuts are odd and right-half cuts even.
int singleCut(const vector<int>& path) {
    int h = path.size()/2;
    vector<int> q;
    for (int i = 0; i < h; ++i) {
        q.push_back(path[i]);
        q.push_back(path[2*h-1-i]);
    }
    int t = ask(q);
    return t % 2 ? (t+1)/2 : 2*h-t/2;
}
void exploitRuns(int initial) {
    vector<Interval> work;
    auto add = [&](int l, int r, int t) {
        int w = r-l, start = species[l];
        identify(r, start ^ (t&1));
        if (t == 0 || t == w) {
            for (int j=l+1; j<r; ++j) identify(j, start ^ (t != 0 && ((j-l)&1)));
        } else work.push_back({l,r,t});
    };
    add(0,n-1,initial);
    while (!work.empty()) {
        int K = max(known[0].size(),known[1].size());
        int F = n-known[0].size()-known[1].size();
        if (F <= K) return;
        int chosen=-1, split=0;
        double best=0;
        for (int i=0; i<(int)work.size(); ++i) {
            auto [l,r,t] = work[i];
            int w=r-l, d=min(t,w-t);
            int s = d <= 3 ? w/2 : max(2, w/d);
            double expected = d == 1 ? (w-1.0)/(t == 1 ? 1 : 2) :
                (s+1)*pow(1.0-(double)d/w,s) + (d<=3 ? w*0.13 : 0);
            // Compare the references earned here with an ordinary training
            // query, and account for the number of counting queries left.
            double rarer=(min(known[0].size(),known[1].size())+1.0)/(n-F+2);
            double alternative = K>=7 ? min(K*0.5,0.40/rarer) : 0;
            double needed = max({3.0,alternative,1.1*K*(double)K/max(1,F)});
            double gain = expected / needed;
            int length = d == 1 ? w+1 : s+1;
            if (lengthUsed+length+2*F <= 100000 && gain > best && gain > 1) {
                best=gain; chosen=i; split=s;
            }
        }
        if (chosen<0) break;
        auto [l,r,t]=work[chosen];
        work.erase(work.begin()+chosen);
        int w=r-l;
        if (t==1 || t==w-1) {
            int cut;
            if (t==1) {
                vector<int> q(w+1);
                iota(q.begin(),q.end(),l);
                cut=singleCut(q);
            } else {
                // On even offsets the single exceptional edge looks like
                // a single change. Two neighboring edge locations remain.
                vector<int> even;
                for (int j=l;j<=r;j+=2) even.push_back(j);
                int m=2*(even.size()/2);
                int index=singleCut(even);
                int lo=2*index-1, hi=index==m ? w : min(w,2*index);
                while(lo<hi) {
                    int mid=(lo+hi)/2;
                    if(ask({l,l+mid}) != (mid&1)) hi=mid;
                    else lo=mid+1;
                }
                cut=lo;
            }
            for (int j=1;j<w;++j)
                identify(l+j,species[l] ^ ((t!=1) && (j&1)) ^ (j>=cut));
        } else {
            vector<int> q(split+1);
            iota(q.begin(),q.end(),l);
            int v=ask(q);
            add(l,l+split,v);
            add(l+split,r,t-v);
        }
    }
}

// Matrices with entries in {-1,0,1}. Two child measurements U and V
// become U+V+z and U-V, where z is a new bit recoverable by parity.
struct Code {
    int m, cols;
    vector<vector<int>> coeff;
    vector<int> pairRow, leaves, bits, flip;
    vector<vector<int>> digits;
    vector<int> leadingSign;
};
vector<Code> codes;
void buildCodes() {
    Code base;
    base.m=base.cols=1;
    base.coeff={{1}}; base.leaves={0}; base.flip={1};
    codes.push_back(base);
    while (codes.back().m<128) {
        const Code &a=codes.back();
        int h=a.m, c=a.cols;
        Code b;
        b.m=2*h; b.cols=2*c+h;
        b.coeff.assign(2*h,vector<int>(b.cols));
        vector<int> order(h), wt(h);
        iota(order.begin(),order.end(),0);
        for(int i=0;i<h;++i)
            for(int x:a.coeff[i]) wt[i] += abs(x);
        sort(order.begin(),order.end(),[&](int i,int j){return wt[i]<wt[j];});
        b.pairRow.resize(h);
        for (int j=0;j<h;++j) b.pairRow[order[j]]=order[h-1-j];
        for(int i=0;i<h;++i) {
            for(int j=0;j<c;++j) {
                b.coeff[i][j]=b.coeff[h+i][j]=a.coeff[i][j];
                int x=a.coeff[b.pairRow[i]][j];
                b.coeff[i][c+j]=x; b.coeff[h+i][c+j]=-x;
            }
            b.coeff[i][2*c+i]=1;
        }
        for(int k=0;k<2;++k) for(int j:a.leaves) b.leaves.push_back(k*c+j);
        for(int k=0;k<2;++k) for(int j:a.bits) b.bits.push_back(k*c+j);
        for(int j=2*c;j<b.cols;++j) b.bits.push_back(j);
        b.flip.resize(b.cols);
        for(int& x:b.flip) x=(rng()&1)?1:-1;
        codes.push_back(move(b));
    }
    // With predominantly one species, use 0/1 measurements. On the
    // Boolean cube a column depending only on S has zero finite difference
    // on every set not contained in S. Its difference on S can be set to
    // any power of two up to 2^(|S|-1) by selecting parity-matched inputs.
    for(int m=2;m<=128;m*=2) {
        Code c;
        c.m=m;
        c.coeff.resize(m); c.digits.resize(m);
        for(int mask=0;mask<m;++mask) {
            int d=__builtin_popcount((unsigned)mask);
            for(int bit=0;bit<max(1,d);++bit) {
                int id=c.leadingSign.size();
                int sign=(mask && (rng()&1)) ? -1 : 1;
                c.leadingSign.push_back(sign);
                vector<int> atoms;
                for(int sub=mask;;sub=(sub-1)&mask) {
                    int parity=__builtin_parity((unsigned)(mask^sub));
                    if((parity ? -1:1)==sign) atoms.push_back(sub);
                    if(!sub) break;
                }
                shuffle(atoms.begin(),atoms.end(),rng);
                vector<int> used(m);
                for(int j=0;j<(1<<bit);++j) used[atoms[j]]=1;
                for(int r=0;r<m;++r) c.coeff[r].push_back(used[r&mask]);
                if(bit+1==max(1,d)) c.leaves.push_back(id);
                else c.bits.push_back(id);
                c.digits[mask].push_back(id);
            }
        }
        c.cols=c.leadingSign.size();
        c.flip.assign(c.cols,1);
        codes.push_back(move(c));
    }
}
void decode(int depth, const vector<int>& y, vector<int>& out, int offset) {
    if (depth==0) {out[offset]=y[0]; return;}
    const Code& a=codes[depth-1];
    const Code& b=codes[depth];
    vector<int> u(a.m),v(a.m);
    for(int i=0;i<a.m;++i) {
        int col=offset+2*a.cols+i;
        // out initially holds the sign of each column.
        int z=((y[i]+y[a.m+i])%2 != 0) ? out[col] : 0;
        out[col]=z;
        u[i]=(y[i]+y[a.m+i]-z)/2;
        v[b.pairRow[i]]=u[i]-y[a.m+i];
    }
    decode(depth-1,u,out,offset);
    decode(depth-1,v,out,offset+a.cols);
}
void decodePositive(const Code& c,vector<int> y,vector<int>& out) {
    for(int mask=c.m-1;mask>=0;--mask) {
        int v=0;
        for(int sub=mask;;sub=(sub-1)&mask) {
            v+=(__builtin_parity((unsigned)(mask^sub)) ? -1:1)*y[sub];
            if(!sub) break;
        }
        for(int j:c.digits[mask]) {
            int sign=c.leadingSign[j];
            out[j]=j==c.leaves[mask] ? v*sign : int(v%2!=0);
            v=(v-sign*out[j])/2;
            for(int i=0;i<c.m;++i) y[i]-=c.coeff[i][j]*out[j];
        }
    }
}
struct Pool { vector<int> ids; int ones; };
vector<Pool> pools;
vector<pair<int,int>> pairsLeft;
int hiddenOnes;
void measured(vector<int> ids, int ones) {
    if (ids.empty()) return;
    if (ones==0 || ones==(int)ids.size()) {
        for (int i:ids) identify(i,ones!=0);
    } else {
        hiddenOnes+=ones;
        if (ids.size()==2) pairsLeft.emplace_back(ids[0],ids[1]);
        else pools.push_back({move(ids),ones});
    }
}
// A measured group can supply endpoint bits until its remaining labels
// are all forced. This uses fewer bits than learning its labels separately.
struct Reserve {
    double turns=0, a=0, b=0;
    void add(double t,double x,double y) { turns+=t; a+=x; b+=y; }
};
Reserve poolReserve(int s,int b) {
    int a=s-b;
    if(s>16 || !a || !b || (s>=5 && min(a,b)==1)) return {};
    return {s-a/(b+1.0)-b/(a+1.0),double(a),double(b)};
}
struct Source { int index; double gainA,gainB,priority; };
vector<Source> sources;
vector<int> rarePools;
vector<double> prefixA,prefixB;
int sourceCursor;
Reserve prepareSources() {
    sources.clear(); rarePools.clear(); sourceCursor=0;
    double pairs=pairsLeft.size();
    Reserve reserve{pairs,pairs,pairs};
    for(int i=0;i<(int)pools.size();++i) {
        int s=pools[i].ids.size(),b=pools[i].ones,a=s-b;
        if(s>=5 && min(a,b)==1) rarePools.push_back(i);
        Reserve r=poolReserve(s,b);
        if(r.turns==0) continue;
        reserve.add(r.turns,a,b);
        double ga=double(a)/s*(1+(b==1)), gb=double(b)/s*(1+(a==1));
        // Prefer small groups and groups rich in the current majority.
        int majority=known[1].size()>known[0].size();
        double priority=(s+(majority?b:a)*0.35)/r.turns;
        sources.push_back({i,ga,gb,priority});
    }
    sort(rarePools.begin(),rarePools.end(),[](int x,int y){return pools[x].ids.size()>pools[y].ids.size();});
    sort(sources.begin(),sources.end(),[](auto &x,auto &y){return x.priority>y.priority;});
    prefixA.assign(sources.size()+1,0); prefixB=prefixA;
    for(int i=0;i<(int)sources.size();++i) {
        prefixA[i+1]=prefixA[i]+sources[i].gainA;
        prefixB[i+1]=prefixB[i]+sources[i].gainB;
    }
    return reserve;
}
struct Bit { int id, opposite=-1, pool=-1; };
Bit getBit() {
    if (!pairsLeft.empty()) {
        auto [u,v]=pairsLeft.back(); pairsLeft.pop_back();
        --hiddenOnes;
        return {u,v};
    }
    if(sourceCursor<(int)sources.size()) {
        int i=sources[sourceCursor++].index;
        int label=pools[i].ids.back();
        pools[i].ids.pop_back();
        return {label,-1,i};
    }
    return {take()};
}
void saveBit(Bit x,int b) {
    identify(x.id,b);
    if(x.opposite>=0) identify(x.opposite,1-b);
    if(x.pool>=0) {
        int count=pools[x.pool].ones;
        hiddenOnes-=count;
        vector<int> rest;
        rest.swap(pools[x.pool].ids);
        measured(move(rest),count-b);
    }
}

// Estimate how many interleaving queries remain. Each query also learns
// its first unknown, increasing the supply of references for later queries.
double bulkTime(double a,double b,double f,double p,Reserve reserve) {
    double time=0;
    auto advance = [&](double turns, double gainA,double gainB,double endpoints) {
        // Integrate the size of the larger reference set, allowing it to change.
        auto capacity=[&](double q) {
            double A=a-0.5*gainA, B=b-0.5*gainB;
            double capA=q*(A+q*0.5*gainA), capB=q*(B+q*0.5*gainB);
            double v=max(capA,capB);
            if ((A-B)*(A-B+(gainA-gainB)*q)<0) {
                double x=(B-A)/(gainA-gainB);
                v=(A>B?capB:capA)+abs(A-B)*x*0.5;
            }
            return v-q*endpoints;
        };
        if(capacity(turns)>=f) {
            double l=0,r=turns;
            for(int i=0;i<22;++i) {
                double mid=(l+r)*0.5;
                if(capacity(mid)<f) l=mid; else r=mid;
            }
            time+=r; f=0;
        } else {
            f-=capacity(turns); time+=turns; a+=gainA*turns; b+=gainB*turns;
        }
    };
    if(reserve.turns>1e-7) advance(reserve.turns,reserve.a/reserve.turns,reserve.b/reserve.turns,1);
    if(f>0) advance(2*sqrt(2*f)+5,1-p,p,0);
    return time;
}
double probabilityB() {
    // Includes previously measured groups, so the estimate also reflects
    // groups whose individual labels have not been decoded.
    return (known[1].size()+hiddenOnes+0.8)/(n-remaining()+1.6);
}

struct Layout {
    int depth=0, width=0, cost=0, splits=0;
    vector<int> sign;
};
bool shape(int depth,int width,int splits,Layout& out) {
    const Code& c=codes[depth];
    int a=known[0].size(), b=known[1].size(),m=c.m;
    if(c.digits.empty() && width+m/2 > a+b) return false;
    vector<int> count(c.cols,1);
    for(int i=0;i<m;++i) count[c.leaves[i]]=width/m+(i<width%m);
    for(int i=0;i<splits;++i) count[c.bits[i]]=0;
    out={depth,width,0,splits,vector<int>(m)};
    for(int i=0;i<m;++i) {
        int plus=0,minus=0;
        for(int j=0;j<c.cols;++j) {
            int x=c.coeff[i][j]*c.flip[j];
            if(x>0) plus+=count[j];
            if(x<0) minus+=count[j];
        }
        int extra[2]{}, items[2]{};
        for(int k=0;k<splits;++k) {
            int j=c.bits[k],x=c.coeff[i][j]*c.flip[j];
            if(!x) continue;
            const Pool &q=pools[rarePools[k]];
            int size=q.ids.size();
            ++extra[q.ones>1];
            items[0]+=x>0 ? size/2 : size-size/2;
            items[1]+=x<0 ? size/2 : size-size/2;
        }
        int best=INT_MAX;
        for(int reverse=0;reverse<2;++reverse) {
            int A=(reverse?minus:plus)+extra[0],B=(reverse?plus:minus)+extra[1];
            A+=(A>0); B+=(B>0);
            int length=1+A+B+plus+minus+items[reverse]+(A+B==0);
            if(A<=a && B<=b && length<best) {
                out.sign[i]=reverse?-1:1;
                best=length;
            }
        }
        if(best==INT_MAX) return false;
        out.cost+=best;
    }
    return true;
}
void runCode(const Layout& layout) {
    const Code& c=codes[layout.depth];
    int m=c.m;
    vector<vector<int>> ids(c.cols);
    vector<Bit> bits(c.cols),ends(m);
    vector<bool> binary(c.cols);
    vector<int> split(c.cols,-1);
    for(int k=0;k<(int)c.bits.size();++k) {
        int j=c.bits[k];
        if(k<layout.splits) split[j]=rarePools[k];
        else { bits[j]=getBit(); ids[j]={bits[j].id}; binary[j]=true; }
    }
    for(Bit& e:ends) e=getBit();
    for(int i=0;i<m;++i) {
        int j=c.leaves[i], size=layout.width/m+(i<layout.width%m);
        if(size==1) {bits[j]=getBit(); binary[j]=true; ids[j]={bits[j].id};}
        else for(int k=0;k<size;++k) ids[j].push_back(take());
    }
    vector<int> y(m);
    for(int i=0;i<m;++i) {
        vector<int> run[2];
        int refs[2]{},offset=0;
        for(int j=0;j<c.cols;++j) {
            int x=c.coeff[i][j]*c.flip[j]*layout.sign[i];
            if(!x) continue;
            if(split[j]>=0) {
                // With a single exception, a whole subpath contributes
                // either zero or two transitions between majority anchors.
                // Its complementary subpath realizes the negative sign.
                const Pool& q=pools[split[j]];
                int b=q.ones>1;
                auto mid=q.ids.begin()+q.ids.size()/2;
                run[b].push_back(known[b][refs[b]++]);
                run[b].insert(run[b].end(),x>0?q.ids.begin():mid,x>0?mid:q.ids.end());
                offset+=x<0;
            } else {
                int b=x<0;
                for(int id:ids[j]) {
                    run[b].push_back(known[b][refs[b]++]);
                    run[b].push_back(id);
                }
                if(b) offset+=ids[j].size();
            }
        }
        vector<int> path{ends[i].id};
        int anchor=run[0].empty() && (!run[1].empty() || known[0].empty());
        for(int b=0;b<2;++b) if(refs[b]) {
            path.insert(path.end(),run[b].begin(),run[b].end());
            path.push_back(known[b][refs[b]]);
        }
        if(path.size()==1) path.push_back(known[anchor][0]);
        int v=ask(path)-(refs[0]>0 && refs[1]>0);
        saveBit(ends[i],anchor^(v&1));
        y[i]=(v/2-offset)*layout.sign[i];
    }
    vector<int> values=c.flip;
    if(c.digits.empty()) decode(layout.depth,y,values,0);
    else decodePositive(c,y,values);
    for(int j=0;j<c.cols;++j) {
        int count=values[j]*c.flip[j];
        if(split[j]>=0) {
            Pool q=move(pools[split[j]]);
            pools[split[j]].ids.clear();
            hiddenOnes-=q.ones;
            int h=q.ids.size()/2;
            int firstOnes=q.ones==1 ? count : h-count;
            vector<int> tail(q.ids.begin()+h,q.ids.end());
            q.ids.resize(h);
            measured(move(q.ids),firstOnes);
            measured(move(tail),q.ones-firstOnes);
        } else if(binary[j]) saveBit(bits[j],count);
        else measured(move(ids[j]),count);
    }
}

int weigh(const vector<int>& ids,Bit e) {
    int b=known[1].size()>known[0].size();
    vector<int> path{e.id,known[b][0]};
    for(int i=0;i<(int)ids.size();++i) {
        path.push_back(ids[i]); path.push_back(known[b][i+1]);
    }
    int v=ask(path);
    saveBit(e,b^(v&1));
    return b ? (int)ids.size()-v/2 : v/2;
}

double pureChance(int size,int same,int draw) {
    if(draw>same) return 0;
    if(draw==0 || same==size) return 1;
    int other=size-same;
    // Avoid multiplying a long sequence when a group has few exceptions.
    double v=1;
    if(draw<=other) {
        for(int i=0;i<draw && v>1e-12;++i) v*=double(same-i)/(size-i);
    } else {
        for(int i=0;i<other && v>1e-12;++i) v*=double(size-draw-i)/(size-i);
    }
    return v;
}
void solveBatches() {
    buildCodes();
    while(remaining()>0) {
        int f=remaining(), a=known[0].size(), b=known[1].size();
        int M=max(a,b), np=pairsLeft.size();
        if(f<=M) {
            Bit end={take(),-1};
            vector<int> ids;
            while(remaining()) ids.push_back(take());
            int ones=weigh(ids,end);
            finish(n-(int)known[1].size()-hiddenOnes-ones);
        }
        if (f==2) {
            int t=ask({take(),known[0][0],take()});
            finish(n-b-hiddenOnes-t);
        }
        erase_if(pools,[](const Pool& q){return q.ids.empty();});
        Reserve reserve=prepareSources();
        int supply=np+sources.size();
        double p=probabilityB();
        double baseline=bulkTime(a,b,f,p,reserve);
        double best=0.015;
        Layout layout;
        int poolIndex=-1, splitSize=0;
        for(int d=0; d<(int)codes.size(); ++d) {
            const Code& c=codes[d];
            int m=c.m, bits=m+c.bits.size();
            if (bits > f+supply+(int)min(rarePools.size(),c.bits.size()) || m>=baseline || m/2>a+b) continue;
            vector<int> widths{0,1,m/4,m/2,3*m/4,m,m+m/2,2*m,3*m,4*m,8*m,12*m,16*m,24*m,32*m};
            int rareWidth=min(60,(int)round(1.0/min(p,1-p)));
            widths.push_back(rareWidth*m);
            if(m==1) widths.push_back(M-1);
            sort(widths.begin(),widths.end());
            widths.erase(unique(widths.begin(),widths.end()),widths.end());
            for(int w:widths) {
                if((c.digits.empty() ? w+m/2>a+b : w>2*M) || w>f+supply) continue;
                int low=w/m,highCount=w%m;
                int single=(low==1 ? m-highCount : (low==0 ? highCount : 0));
                int most=min(rarePools.size(),c.bits.size());
                for(int option=0;option<=int(most>0);++option) {
                    int splits=option*most;
                    int slots=bits+single-splits;
                    int usePairs=min(np,slots), useGroup=min((int)sources.size(),slots-usePairs);
                    int use=usePairs+useGroup;
                    int draw=w+bits-splits-use;
                    if(draw>f) continue;
                    double ga=usePairs+prefixA[useGroup], gb=usePairs+prefixB[useGroup];
                    double A=a+ga+(bits-splits-use)*(1-p), B=b+gb+(bits-splits-use)*p;
                    Reserve future=reserve;
                    future.add(-use,-ga,-gb);
                    for(int k=0;k<splits;++k) {
                        const Pool& q=pools[rarePools[k]];
                        int size=q.ids.size(), half=size/2;
                        double gain=2.0*half*(size-half)/size;
                        (q.ones==1 ? A:B)+=gain;
                        for(int part:{half,size-half}) {
                            Reserve r=poolReserve(part,q.ones==1 ? 1:part-1);
                            double chance=double(part)/size;
                            future.add(chance*r.turns,chance*r.a,chance*r.b);
                        }
                    }
                    for(int j=0;j<2;++j) {
                        int s=low+j, reps=j==0 ? m-highCount : highCount;
                        if(!s || !reps) continue;
                        double pureA=pow(1-p,s),pureB=pow(p,s);
                        A+=reps*s*pureA; B+=reps*s*pureB;
                        if(s>=2 && s<=16) {
                            // The expected trailing run is the sum of the
                            // probabilities that the last i labels all agree.
                            double run=(1-p)*(1-pureA)/p+p*(1-pureB)/(1-p);
                            future.add(reps*(s-run),reps*s*(1-p-pureA),reps*s*(p-pureB));
                        }
                    }
                    double after=bulkTime(A,B,f-draw,p,future);
                    double score=((baseline-after)/m-1)*baseline/(after+m);
                    if(score<=best) continue;
                    Layout candidate;
                    if(shape(d,w,splits,candidate) && lengthUsed+candidate.cost+2*(f-draw)<=100000) {
                        best=score; layout=move(candidate);
                    }
                }
            }
        }
        // A measured group with few exceptions can yield many references
        // by measuring a subset and inferring the count of its complement.
        for(int i=0;i<(int)pools.size();++i) {
            int s=pools[i].ids.size(), k=pools[i].ones;
            int cap=min(M-1,s/2);
            if(cap<=0) continue;
            int typical=min(cap,max(1,s/(1+min(k,s-k))));
            for(int h:{typical,cap,max(1,typical/2)}) {
                int pairUsed=np>0;
                if(lengthUsed+2*(h+1)+2*(f-1+pairUsed)>100000) continue;
                double A=a+(pairUsed?1:1-p), B=b+(pairUsed?1:p);
                Reserve future=reserve;
                future.add(-pairUsed,-pairUsed,-pairUsed);
                Reserve old=poolReserve(s,k);
                future.add(-old.turns,-old.a,-old.b);
                for(int j:{h,s-h}) {
                    double za=pureChance(s,s-k,j),zb=pureChance(s,k,j);
                    A+=za*j; B+=zb*j;
                    if(j==2) future.add(1-za-zb,1-za-zb,1-za-zb);
                }
                double after=bulkTime(A,B,f-1+pairUsed,p,future);
                double score=(baseline-after-1)*baseline/(after+1);
                if(score>best) {best=score; poolIndex=i; splitSize=h;}
            }
        }
        if(poolIndex>=0) {
            Pool src=move(pools[poolIndex]);
            if(poolIndex+1!=(int)pools.size()) pools[poolIndex]=move(pools.back());
            pools.pop_back();
            hiddenOnes-=src.ones;
            vector<int> part(src.ids.end()-splitSize,src.ids.end());
            src.ids.resize(src.ids.size()-splitSize);
            // The selected pool has moved. Only use a separate pair or a
            // fresh label as the endpoint in this query.
            sourceCursor=sources.size();
            int v=weigh(part,getBit());
            measured(move(part),v);
            measured(move(src.ids),src.ones-v);
        } else if (!layout.sign.empty()) {
            runCode(layout);
        } else {
            Bit e=(lengthUsed+2*f+2<=100000 && M>1) ? getBit() : Bit{take(),-1};
            vector<int> ids;
            int h=min(remaining(),M-1);
            for(int i=0;i<h;++i) ids.push_back(take());
            int v=weigh(ids,e);
            measured(move(ids),v);
        }
    }
    finish(n-(int)known[1].size()-hiddenOnes);
}

// Confirm a periodic hypothesis by alternating its two predicted species.
// A maximal reply certifies every label on the path, including label zero.
bool checkPattern(const vector<int>& pattern) {
    vector<int> group[2];
    for(int j=0;j<n;++j) group[pattern[j%pattern.size()]].push_back(j);
    vector<int> path;
    int first=(group[1].size()>group[0].size());
    size_t index[2]{};
    for(int b=first; index[b]<group[b].size(); b^=1)
        path.push_back(group[b][index[b]++]);
    int unknown=n-known[0].size()-known[1].size();
    if(path.size()<2 || lengthUsed+(int)path.size()+2*unknown>100000) return false;
    int v=ask(path);
    if(v+1!=(int)path.size()) return false;
    for(int j=0;j<(int)path.size();++j) identify(path[j],first^(j&1));
    return true;
}
void tryPeriod(int changes,int testedRun) {
    int prefix=0;
    while(prefix<n && species[prefix]>=0) ++prefix;
    if(prefix<5 || remaining()<=16) return;
    for(int period=1; period<=min(24,prefix); ++period) {
        bool ok=true;
        int expected=0;
        for(int j=0;j<n && ok;++j) {
            int prediction=species[j%period];
            if(species[j]>=0 && prediction!=species[j]) ok=false;
            if(j && prediction!=species[(j-1)%period]) ++expected;
        }
        if(!ok || expected!=changes) continue;
        if(testedRun>0 && period==2*testedRun) continue;
        checkPattern(vector<int>(species.begin(),species.begin()+period));
        return;
    }
}

} // namespace
int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    if(!(cin>>n)) return 0;
    species.assign(n,-1);
    identify(0,0);
    if(n<=8) smallTree();
    vector<int> all(n);
    iota(all.begin(),all.end(),0);
    int changes=ask(all);
    identify(n-1,changes&1);
    if(changes==n-2) {
        // Exactly one edge breaks the alternating pattern. Swap each
        // adjacent pair in an even-length prefix. All new edges have odd
        // length. An interior odd cut crosses three edges (two at the
        // boundary), whereas an even cut crosses at most one.
        vector<int> swapped;
        for(int i=0;i+1<n;i+=2) {
            swapped.push_back(i+1); swapped.push_back(i);
        }
        int lost=(int)swapped.size()-1-ask(swapped);
        finish(n/2+(lost>=2));
    }
    if(changes==0 || changes==n-1) finish(changes==0 ? n : (n+1)/2);
    if(changes==1) finish(singleCut(all));
    if(n<=20) {
        vector<int> u(n-2);
        iota(u.begin(),u.end(),1);
        tiny(u,true,2,changes);
    }
    int testedRun=0;
    bool patternSolved=false;
    if(changes>=4 && changes<n-2) {
        int run=(n-1)/changes;
        if(run>=2 && (n-1)/run==changes && (n-1)/(run+1)!=changes) {
            vector<int> pattern(2*run,0);
            fill(pattern.begin()+run,pattern.end(),1);
            testedRun=run;
            patternSolved=checkPattern(pattern);
        }
    }
    if(!patternSolved) exploitRuns(changes);
    for(int i=1;i<n;++i) if(species[i]<0) fresh.push_back(i);
    if(!remaining()) finish(known[0].size());
    if(remaining()>(int)max(known[0].size(),known[1].size()) &&
       known[0].size()+known[1].size()<6 && lengthUsed+4096+2*remaining()<=100000) {
        int count=min(8,remaining());
        vector<int> u;
        for(int j=0;j<count;++j) u.push_back(take());
        tiny(u,false,3);
        tryPeriod(changes,testedRun);
        fresh.clear(); nextFresh=0;
        for(int i=1;i<n;++i) if(species[i]<0) fresh.push_back(i);
    }
    solveBatches();
}
