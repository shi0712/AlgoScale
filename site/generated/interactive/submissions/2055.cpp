#include <bits/stdc++.h>
using namespace std;
using U = uint64_t;

int n, nb, visible;
U allBits, parity;
int cover[55];
U incident[10], touched[1024];
unordered_set<U> visited[10];
unordered_set<U> queried;
uint64_t rngState = 0x28b5dc7ae148319bULL;

int countBits(U x) { return __builtin_popcountll(x); }
int firstBit(U x) { return __builtin_ctzll(x); }
U bit(int i) { return U(1) << i; }

double randomUnit() {
    rngState ^= rngState >> 12;
    rngState ^= rngState << 25;
    rngState ^= rngState >> 27;
    return ((rngState * 2685821657736338717ULL) >> 11) * 0x1p-53;
}

void flip(int i) {
    int cells = cover[i];
    cout << __builtin_ctz(cells) + 1 << ' ' << 32 - __builtin_clz(cells) << endl;
    if (!(cin >> visible) || visible < 0 || visible >= n) exit(0);
    parity ^= bit(i);
    queried.insert(parity);
}

void remember() {
    for (int c = 0; c < n; ++c) visited[c].insert(parity & incident[c]);
}

// In one possible visibility history, original stores the initial bits
// of every interval touching a cell that has ever been seen.
struct World {
    int seen, lit;
    U original;
};

// A cell which first becomes visible must have a previously untested
// local signature. All bits incident to it are then determined forever.
vector<World> observe(const vector<World>& before) {
    vector<World> after;
    int fresh = 0;
    for (int c = 0; c < n; ++c)
        if (!visited[c].count(parity & incident[c])) fresh |= 1 << c;
    for (const auto& w : before) {
        U active = (w.original ^ parity) & touched[w.seen];
        int possible = fresh & ~w.seen, lit = w.seen;
        for (int c = 0; c < n; ++c) {
            if (incident[c] & active) {
                lit &= ~(1 << c);
                possible &= ~(1 << c);
            }
        }
        int need = visible - countBits(lit);
        if (need < 0 || need > countBits(possible)) continue;
        if (need == 0) {
            after.push_back({w.seen, lit, w.original});
            continue;
        }
        for (int s = possible; s; s = (s - 1) & possible)
            if (countBits(s) == need)
                after.push_back({w.seen | s, lit | s, w.original | (parity & touched[s])});
    }
    remember();
    return after;
}

// Mixture of exchangeable priors on bits. These weights only order probes.
struct Group {
    U mask;
    double a, b;
    int type;
};
struct Model {
    double weight;
    vector<Group> groups;
    U center = 0;
};
vector<Model> models;
double mass[12][56][56];
vector<pair<double,double>> types;
Group group(U mask, double a, double b) {
    auto ab = make_pair(a,b);
    int t = find(types.begin(),types.end(),ab) - types.begin();
    if (t == (int)types.size()) {
        types.push_back(ab);
        mass[t][0][0] = 1;
        for (int k = 1; k <= nb; ++k) {
            mass[t][k][0] = mass[t][k-1][0] * (b+k-1)/(a+b+k-1);
            for (int i = 1; i <= k; ++i)
                mass[t][k][i] = mass[t][k-1][i-1] * (a+i-1)/(a+b+k-1);
        }
    }
    return {mask,a,b,t};
}

void initModels() {
    vector<U> length(n), left(n), right(n);
    for (int i = 0; i < nb; ++i) {
        length[countBits(cover[i])-1] |= bit(i);
        left[__builtin_ctz(cover[i])] |= bit(i);
        right[31-__builtin_clz(cover[i])] |= bit(i);
    }
    models.push_back({.43,{group(allBits,.55,.78)}});
    models.push_back({.19,{group(allBits,30,30)}});
    models.push_back({.085,{group(length[0],.55,.55),group(allBits ^ length[0],.14,4)}});
    for (auto [partition, weight] : vector<pair<vector<U>,double>>{
            {length,.105},{left,.05},{right,.05}}) {
        Model m{weight,{}};
        for (U g : partition) m.groups.push_back(group(g,.24,.32));
        models.push_back(m);
    }
    U prefix = 0;
    for (int l = 0; l + 1 < n; ++l) {
        prefix |= length[l];
        models.push_back({.09 / (2*(n-1)),
            {group(prefix,.65,.60),group(allBits ^ prefix,.09,3.5)}});
        models.push_back({.09 / (2*(n-1)),
            {group(allBits ^ prefix,.65,.60),group(prefix,.09,3.5)}});
    }
    auto nearPattern = [&](U center, double weight) {
        models.push_back({weight,{group(allBits,.22,9)},center});
    };
    U alternateLength=0, alternateStart=0, alternateEnd=0;
    for (int j=0;j<n;++j) {
        nearPattern(incident[j], .025/n);
        nearPattern(length[j], .028/n);
        if (j%2==0) {
            alternateLength |= length[j];
            alternateStart |= left[j];
            alternateEnd |= right[j];
        }
    }
    for (U mask : {alternateLength,alternateStart,alternateEnd}) {
        nearPattern(mask,.004);
        nearPattern(allBits^mask,.004);
    }
}

double likelihood(const Model& m, U known, U values) {
    values ^= m.center & known;
    double p = m.weight;
    for (auto& g : m.groups)
        p *= mass[g.type][countBits(known & g.mask)][countBits(values & g.mask)];
    return p;
}
double prior(U known, U values) {
    double p = 0;
    for (auto& m : models) p += likelihood(m,known,values);
    return p;
}

struct Sampler {
    U known, values;
    double total = 0;
    vector<double> cumulative;
    Sampler(U known, U values) : known(known), values(values) {
        for (auto& m : models) {
            total += likelihood(m,known,values);
            cumulative.push_back(total);
        }
    }
    U draw() const {
        int j = lower_bound(cumulative.begin(), cumulative.end(), randomUnit()*total)
            - cumulative.begin();
        U center=models[j].center;
        U result = values;
        U errors=values^(center&known);
        for (auto& g : models[j].groups) {
            double ones = g.a + countBits(g.mask & errors);
            double denom = g.a + g.b + countBits(g.mask & known);
            U rest = g.mask & ~known;
            while (rest) {
                U b = rest & -rest;
                if (randomUnit()*denom < ones) {
                    result |= b;
                    ++ones;
                }
                ++denom;
                rest ^= b;
            }
        }
        return result ^ (center & ~known);
    }
};

bool consistent(U values, int unseen) {
    while (unseen) {
        int c = __builtin_ctz(unseen);
        if (visited[c].count(incident[c] & values)) return false;
        unseen &= unseen-1;
    }
    return true;
}

// Measure information in the reply per expected extra activation.
double queryValue(const double* bucket, int last, double total, double wrong) {
    double h = 0;
    for (int i = 0; i <= last; ++i) {
        if (bucket[i] <= 0) continue;
        double p = min(1.0,bucket[i]/total);
        h -= p * log2(p);
    }
    return h / (.085 + wrong);
}

struct LocalCandidate { int value; double weight; };

// For a short candidate list, optimize the remaining expected flip count
// exactly. At the first informative configuration, the reply partitions
// the candidates. Traveling there costs its Hamming distance.
int exactMove(int x, int m, const vector<LocalCandidate>& candidates,
              const vector<uint8_t>& result) {
    int k=candidates.size(), size=1<<m, sets=1<<k;
    vector<double> weights(sets);
    vector<double> cost((size_t)sets*size);
    vector<array<int,6>> groups(size);
    for (int q=0;q<size;++q)
        for (int i=0;i<k;++i)
            groups[q][result[q ^ candidates[i].value]] |= 1<<i;
    for (int s=1;s<sets;++s) {
        int i=__builtin_ctz(s), rest=s&(s-1);
        weights[s]=weights[rest]+candidates[i].weight;
        auto f=cost.data()+(size_t)s*size;
        if (!rest) {
            for (int q=0;q<size;++q) f[q]=countBits(q ^ candidates[i].value);
            continue;
        }
        for (int q=0;q<size;++q) {
            double value=0;
            for (int g:groups[q]) {
                int part=g&s;
                if (part==s) { value=1e9; break; }
                if (part) value+=weights[part]/weights[s]*cost[(size_t)part*size+q];
            }
            f[q]=value;
        }
        for (int b=1;b<size;b*=2) for (int start=0;start<size;start+=2*b) {
            for (int a=start;a<start+b;++a) {
                f[a]=min(f[a],1+f[a+b]);
                f[a+b]=min(f[a+b],1+f[a]);
            }
        }
    }
    int move=0;
    auto f=cost.data()+(size_t)(sets-1)*size;
    for (int j=1;j<m;++j)
        if (f[x^(1<<j)]<f[x^(1<<move)]-1e-10) move=j;
    return move;
}

// All moves in this run leave the exterior visibility unchanged.
void solveSmall(int lo, int hi, int exterior, const World& w) {
    int cells = ((1 << (hi-lo+1))-1) << lo;
    vector<int> ids;
    for (int i = 0; i < nb; ++i)
        if ((cover[i] & ~cells) == 0) ids.push_back(i);
    int m = ids.size(), size = 1 << m;
    vector<U> expand(size);
    vector<int> covered(size);
    vector<uint8_t> result(size);
    int x = 0;
    for (int j = 0; j < m; ++j)
        if (parity & bit(ids[j])) x |= 1 << j;
    vector<LocalCandidate> candidates;
    U known = touched[w.seen];
    for (int s = 0; s < size; ++s) {
        if (s) {
            int j = __builtin_ctz(s), rest = s & (s-1);
            expand[s] = expand[rest] | bit(ids[j]);
            covered[s] = covered[rest] | cover[ids[j]];
        }
        result[s] = hi-lo+1-countBits(covered[s]);
    }
    U full = known | expand.back();
    for (int s = 0; s < size; ++s) {
        if (result[s^x] != visible-exterior) continue;
        U values = w.original | expand[s];
        if (!consistent(values, cells & ~w.seen)) continue;
        candidates.push_back({s,prior(full,values)});
    }
    // Choose an informative destination, taking one step at a time. Keep
    // following the route through uninformative intermediate observations.
    int goal = -1;
    while (visible-exterior < hi-lo+1) {
        double total = 0;
        array<double,16> wrong{};
        int common = size-1;
        for (auto& a : candidates) {
            total += a.weight;
            int active = a.value ^ x;
            common &= active;
            int off = (size-1) ^ active;
            while (off) {
                wrong[__builtin_ctz(off)] += a.weight;
                off &= off-1;
            }
        }
        int next = -1;
        if (candidates.empty()) abort();
        for (int j = 0; j < m; ++j) wrong[j] /= total;
        if (candidates.size() >= 2 && candidates.size()+m <= 20 && candidates.size()<=13) {
            next=exactMove(x,m,candidates,result);
            goal=-1;
        } else if (common && goal < 0) {
            // Every candidate needs these removals. Prefer one whose
            // response separates the possibilities.
            double best = -1;
            for (int j = 0; j < m; ++j) if (common & (1<<j)) {
                double bucket[11]{};
                for (auto& a : candidates) bucket[result[a.value^x^(1<<j)]] += a.weight;
                double score = queryValue(bucket, hi-lo+1, total,0);
                if (score > best) { best=score; next=j; }
            }
            goal = -1;
        } else if (goal < 0 || goal == x) {
            vector<int> options;
            if (m <= 6) {
                for (int s = 0; s < size; ++s) if (s != x) options.push_back(s);
            } else {
                for (int j = 0; j < m; ++j)
                    options.push_back(x^(1<<j));
                for (int j = 0; j < m; ++j)
                    for (int k = 0; k < j; ++k) options.push_back(x^(1<<j)^(1<<k));
                // Endpoints can be exposed by matching just their bits.
                for (int c : {lo,hi}) {
                    int incidentLocal=0;
                    for (int j=0;j<m;++j) if (cover[ids[j]] & (1<<c)) incidentLocal |= 1<<j;
                    for (int t=incidentLocal;t;t=(t-1)&incidentLocal) options.push_back(x^t);
                }
                if (candidates.size() <= 128)
                    for (auto& a : candidates) options.push_back(a.value);
                else {
                    auto mode = max_element(candidates.begin(), candidates.end(),
                        [](auto& a, auto& b){return a.weight < b.weight;});
                    options.push_back(mode->value);
                    for (int i=0;i<12;++i)
                        options.push_back(candidates[(size_t)(randomUnit()*candidates.size())].value);
                }
                sort(options.begin(),options.end());
                options.erase(unique(options.begin(),options.end()),options.end());
            }
            double best = -1;
            for (int q : options) {
                int moves = q^x;
                if (!moves) continue;
                double cost = 0, bucket[11]{};
                for (int t=moves; t; t &= t-1) cost += wrong[__builtin_ctz(t)];
                for (auto& a : candidates) bucket[result[a.value^q]] += a.weight;
                double value = queryValue(bucket,hi-lo+1,total,cost + .035*(countBits(moves)-1));
                if (value > best) { best = value; goal = q; }
            }
        }
        if (next < 0) {
            double best = -1;
            for (int t = goal^x; t; t &= t-1) {
                int j = __builtin_ctz(t);
                double bucket[11]{};
                for (auto& a : candidates) bucket[result[a.value^x^(1<<j)]] += a.weight;
                double value = queryValue(bucket,hi-lo+1,total,wrong[j]) + .05*(1-wrong[j]);
                if (value > best) { best=value; next=j; }
            }
        }
        flip(ids[next]);
        x ^= 1<<next;
        size_t oldSize = candidates.size();
        erase_if(candidates,[&](auto& a){ return result[a.value^x] != visible-exterior; });
        if (candidates.size() < oldSize) goal=-1;
    }
}

int chooseAmbiguous(const vector<World>& worlds, bool forceSingleton) {
    double reply[55][21]{}, activeMass[55]{}, total=0;
    auto add = [&](U values, double weight) {
        U active = values ^ parity;
        int lit=0;
        int gain[55]{};
        for (int c=0;c<n;++c) {
            U z=active & incident[c];
            if (!z) lit |= 1<<c;
            else if ((z & (z-1)) == 0) ++gain[firstBit(z)];
        }
        for (int j=0;j<nb;++j) {
            if (active & bit(j)) {
                activeMass[j] += weight;
                reply[j][n+gain[j]] += weight;
            } else reply[j][n-countBits(lit & cover[j])] += weight;
        }
        total += weight;
    };
    int fullCells = (1<<n)-1;
    vector<Sampler> generators;
    vector<double> allocation;
    double sum=0;
    for (auto& w : worlds) {
        generators.emplace_back(touched[w.seen],w.original);
        double h=sqrt(generators.back().total);
        allocation.push_back(h);
        sum += h;
    }
    for (int index=0; index<(int)worlds.size(); ++index) {
        auto& w=worlds[index];
        auto& gen=generators[index];
        int unseen=fullCells ^ w.seen;
        U unknown=allBits ^ gen.known;
        int budget = 12 + int(3600*allocation[index]/sum);
        if (countBits(unknown) <= 14 && (1<<countBits(unknown)) <= 4*budget) {
            for (U s=unknown;;s=(s-1)&unknown) {
                U values=w.original|s;
                if (consistent(values,unseen)) add(values,prior(allBits,values));
                if (!s) break;
            }
        } else {
            double weight=gen.total/budget;
            for (int k=0; k<budget;++k) {
                U values=gen.draw();
                if (consistent(values,unseen)) add(values,weight);
            }
        }
    }
    double best=-1;
    int answer=-1;
    for (int j=0; j<nb; ++j) {
        int intersects=0;
        for (auto& w : worlds) intersects += bool(w.lit & cover[j]);
        bool disputed=countBits(cover[j])==1 && intersects>0 && intersects<(int)worlds.size();
        if (forceSingleton && !disputed) continue;
        double score = disputed ? .00001 : 0;
        if (total>0) {
            double p=min(1.,activeMass[j]/total);
            score += queryValue(reply[j],2*n,total,1-p) + .28*p;
        }
        if (queried.count(parity ^ bit(j)) && !disputed) score *= .03;
        if (score>best) {best=score;answer=j;}
    }
    return answer;
}

struct Endpoint {
    vector<int> ids;
    vector<U> expand;
    vector<double> prob;
    int c, x=0;
    Endpoint(int c, const World& w) : c(c) {
        U known=touched[w.seen];
        for (U s=incident[c] & ~known;s;s &= s-1) ids.push_back(firstBit(s));
        int m=ids.size();
        for (int j=0;j<m;++j) if (parity & bit(ids[j])) x |= 1<<j;
        expand.resize(1<<m);
        prob.resize(1<<m);
        for (int s=0;s<(1<<m);++s) {
            if (s) expand[s]=expand[s&(s-1)] | bit(ids[__builtin_ctz(s)]);
            prob[s]=prior(known|incident[c],w.original|expand[s]);
        }
        double sum=accumulate(prob.begin(),prob.end(),0.);
        for (double& p:prob) p /= sum;
    }
    int pack(U value) const {
        int s=0;
        for (int j=0;j<(int)ids.size();++j) if (bit(ids[j]) & value) s |= 1<<j;
        return s;
    }
};

vector<World> searchEndpoint(int lo,int hi,const World& w) {
    vector<Endpoint> ends;
    ends.emplace_back(lo,w);
    ends.emplace_back(hi,w);
    Sampler gen(touched[w.seen],w.original);
    vector<U> samples;
    int unseen=((1<<n)-1)^w.seen;
    for (int k=0;k<15000;++k) {
        U s=gen.draw();
        if (consistent(s,unseen)) samples.push_back(s);
    }
    // Try concentrated signatures on either side, or at an interior cell,
    // before spending a long sequence of probes on a single endpoint.
    vector<World> initialWorld{w};
    vector<unordered_map<U,int>> frequencies(n);
    for (U value : samples)
        for (int c=lo; c<=hi; ++c) ++frequencies[c][value & incident[c]];
    int runCells=(2<<hi)-(1<<lo);
    U localMask=0;
    for (int j=0;j<nb;++j)
        if ((cover[j] & ~runCells)==0) localMask |= bit(j);
    int length=hi-lo+1;
    int initialCount=visible;
    for (int step=0;step<6*length && samples.size()>80;) {
        // Give sparse and dense endpoint patterns a short early tour.
        bool tryShells = !w.seen && length >= 7 && step >= length+3;
        vector<U> options;
        for (U t=localMask;t;t &= t-1) options.push_back(t & -t);
        for (int c=lo;c<=hi;++c) {
            array<pair<double,U>,5> top{};
            U current=parity & incident[c];
            for (auto [value,number] : frequencies[c]) {
                if (number<4) continue;
                U change=value^current;
                int d=countBits(change);
                if (d==0 || d>length+1) continue;
                double gain=double(number)/d;
                if (gain>top.back().first) {
                    int k=(int)top.size()-1;
                    while (k && gain>top[k-1].first) {top[k]=top[k-1];--k;}
                    top[k]={gain,change};
                }
            }
            for (auto [gain,change]:top) if (gain>0) options.push_back(change);
        }
        if (tryShells) {
            for (auto& e:ends) {
                U bits=incident[e.c];
                for (U t=bits;t;t &= t-1) {
                    U b=t & -t;
                    for (U signature:{b,bits^b})
                        if (!visited[e.c].count(signature))
                            options.push_back(signature^(parity&bits));
                }
            }
        }
        auto reward = [&](U change) {
            double sum=0, largest=0;
            for (int c=lo;c<=hi;++c) {
                auto it=frequencies[c].find((parity^change)&incident[c]);
                double v=it==frequencies[c].end() ? 0 : it->second;
                if (tryShells && (c==lo || c==hi)) {
                    U signature=(parity^change)&incident[c];
                    int k=countBits(signature);
                    if ((k==1 || k==length-1) && !visited[c].count(signature))
                        v+=.028*samples.size();
                }
                sum+=v;
                largest=max(largest,v);
            }
            return (.55*sum + .45*largest)/countBits(change);
        };
        U goal=0;
        double bestReward=0;
        for (U change:options) {
            double r=reward(change);
            if (r>bestReward) {bestReward=r;goal=change;}
        }
        if (bestReward < max(6.,2.4*samples.size()/(1<<length))) break;
        while (goal) {
            U move=0;
            double bestOne=-1;
            for (U t=goal;t;t &= t-1) {
                U b=t & -t;
                double r=reward(b);
                if (r>bestOne) {bestOne=r;move=b;}
            }
            flip(firstBit(move));
            ++step;
            if (visible!=initialCount) return observe(initialWorld);
            remember();
            goal ^= move;
            int cells=cover[firstBit(move)];
            erase_if(samples,[&](U value){
                for (int t=cells;t;t &= t-1) {
                    int c=__builtin_ctz(t);
                    if (((value^parity) & incident[c])==0) {
                        for (int k=lo;k<=hi;++k) --frequencies[k][value & incident[k]];
                        return true;
                    }
                }
                return false;
            });
        }
    }
    vector<vector<double>> baseline;
    for (auto& e:ends) baseline.push_back(e.prob);
    int previous=-1;
    while (true) {
        double best=numeric_limits<double>::infinity();
        int choice=0;
        for (int side=0;side<2;++side) {
            auto& e=ends[side];
            e.x=e.pack(parity);
            e.prob=baseline[side];
            for (double& p:e.prob) p *= 2200.;
            for (U value:samples) ++e.prob[e.pack(value)];
            vector<pair<double,int>> live;
            double sum=0;
            for (int s=0;s<(int)e.prob.size();++s) {
                if (visited[e.c].count((w.original & incident[e.c])|e.expand[s])) continue;
                live.emplace_back(e.prob[s],s);
                sum+=e.prob[s];
            }
            sort(live.begin(),live.end(),greater<>());
            double cost=0, hit=0, index=1e100;
            for (int j=0;j<(int)live.size();++j) {
                cost+=1-hit/sum;
                hit+=live[j].first;
                // A short excursion has to pay the distance to its probes.
                double overhead=countBits(e.x^live[0].second)-1;
                double effective=(cost+overhead)*sum/hit;
                index=min(index,effective);
            }
            if (side==previous) index *= .80;
            if (index<best) {best=index;choice=side;}
        }
        previous=choice;
        auto& e=ends[choice];
        int m=e.ids.size(), size=1<<m;
        U varying=e.expand.back(), fixed=w.original & incident[e.c];
        vector<vector<double>> massOther(n, vector<double>(size));
        vector<int> projection(n);
        for (int c=lo;c<=hi;++c) if (c != e.c) {
            projection[c]=e.pack(incident[c]);
            for (U value:samples)
                if (((parity^value) & incident[c] & ~varying)==0)
                    ++massOther[c][e.pack(value & incident[c])];
        }
        int x=e.x, goal=-1;
        vector<bool> tested(size);
        for (int s=0;s<size;++s) tested[s]=visited[e.c].count(fixed|e.expand[s]);
        // Finish the current route before reconsidering the other side.
        // Each route visits at least one new signature on the selected side.
        for (int steps=0;steps<32 || goal>=0;++steps) {
            if (goal<0) {
                double top=-1;
                vector<double> score=e.prob;
                for (int c=lo;c<=hi;++c) if (c!=e.c) {
                    int mask=projection[c];
                    U outside=parity & incident[c] & ~varying;
                    for (int s=mask;;s=(s-1)&mask) {
                        if (visited[c].count(outside|e.expand[s])) massOther[c][s]=0;
                        if (!s) break;
                    }
                    for (int s=0;s<size;++s) score[s]+=massOther[c][s&mask];
                }
                for (int s=0;s<size;++s) if (!tested[s]) {
                    int distance=countBits(s^x);
                    double value=score[s]/distance;
                    if (distance>1) {
                        double first=0, second=0, third=0;
                        for (int j=0;j<m;++j) {
                            int neighbor=s^(1<<j);
                            if (tested[neighbor]) continue;
                            double p=e.prob[neighbor];
                            if (p>first) {third=second;second=first;first=p;}
                            else if (p>second) {third=second;second=p;}
                            else third=max(third,p);
                        }
                        value=max(value,(score[s]+first)/(distance+1));
                        value=max(value,(score[s]+first+second+third)/(distance+5));
                    }
                    if (value>top) {top=value;goal=s;}
                }
            }
            int next=-1;
            double top=-1;
            for (int t=x^goal;t;t &= t-1) {
                int j=__builtin_ctz(t), s=x^(1<<j);
                double val=tested[s] ? 0 : e.prob[s];
                for (int c=lo;c<=hi;++c) if (c!=e.c && (projection[c] & (1<<j)))
                    val+=massOther[c][s & projection[c]];
                if (val>top) {top=val;next=j;}
            }
            flip(e.ids[next]);
            x ^= 1<<next;
            if (visible!=initialCount) return observe(initialWorld);
            if (!tested[x]) goal=-1;
            tested[x]=true;
            remember();
        }
        erase_if(samples,[&](U value){return !consistent(value,unseen);});
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin>>n) || n<1 || n>10) return 0;
    if (!(cin>>visible) || visible<0 || visible>=n) return 0;
    for (int l=0;l<n;++l) for (int r=l;r<n;++r) {
        int cells=((2<<r)-(1<<l));
        cover[nb]=cells;
        for (int c=l;c<=r;++c) incident[c] |= bit(nb);
        ++nb;
    }
    allBits=bit(nb)-1;
    for (int s=1;s<(1<<n);++s)
        touched[s]=touched[s&(s-1)] | incident[__builtin_ctz(s)];
    initModels();
    if (n<=5) {
        solveSmall(0,n-1,0,{0,0,0});
        return 0;
    }
    queried.insert(parity);
    vector<World> worlds=observe({{0,0,0}});
    int ambiguousSteps=0;
    while (true) {
        if (worlds.size()>1) {
            // A singleton with disputed visibility always eliminates at
            // least one history, and cannot split a surviving history.
            int move=chooseAmbiguous(worlds,ambiguousSteps>=n);
            int old=visible;
            flip(move);
            worlds=observe(worlds);
            // Keep the worlds distinguishable by their current visible
            // masks. Restoring this known active interval costs one flip.
            if (visible<old) {
                bool used[1024]{}, duplicate=false;
                for (auto& w:worlds) {
                    duplicate |= used[w.lit];
                    used[w.lit]=true;
                }
                if (duplicate) {
                    flip(move);
                    worlds=observe(worlds);
                }
            }
            ++ambiguousSteps;
            continue;
        }
        ambiguousSteps=0;
        World w=worlds.at(0);
        U active=(parity^w.original) & touched[w.seen];
        if (active) {
            int best=-1,move=-1;
            for (U s=active;s;s &= s-1) {
                int j=firstBit(s),score=0;
                for (int c=0;c<n;++c) if ((incident[c]&active)==bit(j))
                    score += (w.seen & (1<<c)) ? 1 : 4;
                if (score>best) {best=score;move=j;}
            }
            flip(move);
            worlds=observe(worlds);
            continue;
        }
        int lo=0, hi=n;
        for (int i=0;i<n;) {
            if (w.seen & (1<<i)) {++i;continue;}
            int l=i;
            while (i<n && !(w.seen & (1<<i))) ++i;
            if (i-l < hi-lo+1) {lo=l; hi=i-1;}
        }
        if (hi-lo+1<=5) {
            solveSmall(lo,hi,visible,w);
            int cells=((2<<hi)-(1<<lo));
            w.seen |= cells;
            w.lit |= cells;
            w.original=parity & touched[w.seen];
            worlds={w};
            remember();
        } else worlds=searchEndpoint(lo,hi,w);
    }
}
