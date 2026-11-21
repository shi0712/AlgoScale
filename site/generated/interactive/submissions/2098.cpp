#include <bits/stdc++.h>
using namespace std;

constexpr int MAXN = 500;
struct Mask {
    array<uint64_t, 8> w{};
    void set(int e) { w[e / 64] |= 1ULL << (e % 64); }
    bool has(int e) const { return w[e / 64] >> (e % 64) & 1; }
    Mask operator|(const Mask& b) const {
        Mask r;
        for (int i = 0; i < 8; ++i) r.w[i] = w[i] | b.w[i];
        return r;
    }
    Mask operator^(const Mask& b) const {
        Mask r;
        for (int i = 0; i < 8; ++i) r.w[i] = w[i] ^ b.w[i];
        return r;
    }
};
int n, m, words;
int preferredEdge=-1;
Mask allEdges, toward;
mt19937_64 gen(19283671);
vector<unordered_map<string,int>> cacheReply;

int ask(int v, const Mask& b) {
    string s(m, '0');
    bool flip = b.has(0);
    for (int i = 0; i < m; ++i) if (b.has(i) != flip) s[i] = '1';
    auto& cache = cacheReply[v];
    auto it = cache.find(s);
    if (it != cache.end()) return flip ? m - it->second : it->second;
    string message = s;
    if (flip) for (char& c : message) c = c == '0' ? '1' : '0';
    cout << "? " << v + 1 << ' ' << message << endl;
    int a;
    if (!(cin >> a) || a < 0 || a > m) exit(0);
    cache.emplace(move(s), flip ? m - a : a);
    return a;
}
int common(const Mask& a, const Mask& b) {
    int ans = 0;
    for (int i = 0; i < words; ++i) ans += __builtin_popcountll(a.w[i] & b.w[i]);
    return ans;
}
Mask maskOf(const vector<int>& ids) {
    Mask r;
    for (int i : ids) r.set(i);
    return r;
}

using Probe = function<int(const Mask&)>;
struct Possibility {
    Mask path;
    double weight;
    int id;
};

// Choose a probe by the entropy of its exact answers on the remaining choices.
// Whole segment masks preserve long-distance information on partly known paths.
template<class MakeProbe>
int distinguish(const vector<Possibility>& choices, const Probe& read, MakeProbe makeProbe,
                const vector<Mask>* cellForEdge = nullptr, bool distance = false) {
    vector<int> depths(choices.size());
    if (distance) for (int i=0; i<(int)choices.size(); ++i) depths[i]=common(allEdges,choices[i].path);
    vector<int> live(choices.size());
    iota(live.begin(), live.end(), 0);
    while (live.size() > 1) {
        vector<int> sample;
        if (live.size()>560) {
            sample={live[0],live[1]};
            for (int j=2; j<400; ++j) sample.push_back(live[gen()%live.size()]);
        }
        const auto& scored=sample.empty() ? live : sample;
        Mask best;
        double bestScore = -1e300;
        bool found = false;
        for (int trial = 0; trial < 32; ++trial) {
            Mask q = makeProbe(trial);
            if (!cellForEdge && trial>0 && trial%6==0)
                q=choices[scored[gen()%scored.size()]].path;
            if (trial == 0) {
                Mask delta = choices[live[0]].path ^ choices[live[1]].path;
                q = {};
                for (int i = 0; depths[live[0]]==depths[live[1]] && i < words; ++i) if (delta.w[i]) {
                    int e = 64*i + __builtin_ctzll(delta.w[i]);
                    if (cellForEdge) q = (*cellForEdge)[e];
                    else q.set(e);
                    break;
                }
            }
            double hist[MAXN+1]{};
            int used[MAXN+1], buckets = 0;
            int qsize=distance ? common(allEdges,q) : 0;
            for (int i : scored) {
                int x = common(q, choices[i].path);
                if (distance) x=depths[i]+qsize-2*x;
                if (hist[x] == 0) used[buckets++] = x;
                hist[x] += choices[i].weight;
            }
            if (buckets < 2) continue;
            double score = 0;
            for (int j = 0; j < buckets; ++j) {
                double h = hist[used[j]];
                score -= h * log(h);
            }
            if (!found || score > bestScore) {
                found = true;
                bestScore = score;
                best = q;
            }
            if (buckets == (int)scored.size()) break;
        }
        int result = read(best);
        int size=distance ? common(allEdges,best) : 0;
        erase_if(live, [&](int i){
            int x=common(best,choices[i].path);
            return (distance ? depths[i]+size-2*x : x) != result;
        });
    }
    if (live.empty()) exit(0);
    return live[0];
}

double binom[MAXN+1][MAXN+1], cost[MAXN+1][MAXN+1];
int action[MAXN+1][MAXN+1];
int capacity(int k) { return 1 + k*(1 << (k-1)); }
void prepare(int limit) {
    for (int s = 0; s <= limit; ++s) {
        binom[s][0] = binom[s][s] = 1;
        for (int k = 1; k < s; ++k) binom[s][k] = binom[s-1][k-1]+binom[s-1][k];
    }
    for (int s = 2; s <= limit; ++s) {
        int order = 1;
        while (capacity(order) < s) ++order;
        vector<int> splits{s / 2, max(1,s/3)};
        if (order > 1) splits.push_back(min(capacity(order-1), s-capacity(order-1)));
        sort(splits.begin(), splits.end());
        splits.erase(unique(splits.begin(), splits.end()), splits.end());
        for (int k = 1; k <= s/2; ++k) {
            double best = (1 << order) - 1;
            int pick = -order;
            for (int h : splits) {
                double v = 0;
                for (int j = max(0,k-(s-h)); j <= min(h,k); ++j)
                    v += binom[h][j]*binom[s-h][k-j]*(cost[h][j]+cost[s-h][k-j]);
                v = 1 + v/binom[s][k];
                if (v < best + 1e-10) best = v, pick = h;
            }
            cost[s][k] = cost[s][s-k] = best;
            action[s][k] = action[s][s-k] = pick;
        }
    }
}

// On the Boolean cube of dimension k, a column supported on odd-parity
// assignments of T has a controllable top Walsh coefficient. Powers of two
// encode |T| independent bits there. Proper supersets are decoded first.
void cubeDecode(const vector<int>& ids, int total, int k, const Probe& read, Mask& answer) {
    int rows = 1 << k;
    vector<Mask> probes(rows);
    struct Column { int id, subset; vector<int> rows; };
    vector<Column> cols;
    int pos = 0;
    for (int t = 1; t < rows && pos + 1 < (int)ids.size(); ++t) {
        vector<int> odd;
        for (int a = 0; a < rows; ++a)
            if ((a & t) == a && __builtin_parity((unsigned)a)) odd.push_back(a);
        for (int j = 0; (1 << j) <= (int)odd.size() && pos + 1 < (int)ids.size(); ++j) {
            Column c{ids[pos++],t,{}};
            for (int r = 0; r < rows; ++r)
                if (binary_search(odd.begin(),odd.begin()+(1<<j),r&t)) {
                    probes[r].set(c.id);
                    c.rows.push_back(r);
                }
            cols.push_back(move(c));
        }
    }
    vector<int> y(rows);
    for (int r = 1; r < rows; ++r) y[r] = read(probes[r]);
    for (int end = cols.size(); end > 0;) {
        int t = cols[end-1].subset, start = end-1;
        while (start && cols[start-1].subset == t) --start;
        int coefficient = 0;
        for (int r = 0; r < rows; ++r)
            coefficient += __builtin_parity((unsigned)(r&t)) ? y[r] : -y[r];
        int bits = coefficient / (rows >> __builtin_popcount((unsigned)t));
        for (int i = start; i < end; ++i) if (bits >> (i-start) & 1) {
            answer.set(cols[i].id);
            --total;
            for (int r : cols[i].rows) --y[r];
        }
        end = start;
    }
    if (total) answer.set(ids.back());
}

struct Group { vector<int> ids; int count; };
void subsets(const vector<int>& ids, int left, int at, Mask mask, vector<Mask>& out) {
    if (left == 0) { out.push_back(mask); return; }
    if (left == (int)ids.size()-at) {
        for (int j = at; j < (int)ids.size(); ++j) mask.set(ids[j]);
        out.push_back(mask);
        return;
    }
    for (int j = at; j + left <= (int)ids.size(); ++j) {
        Mask next = mask;
        next.set(ids[j]);
        subsets(ids, left-1, j+1, next, out);
    }
}

Mask recoverGroups(vector<Group> todo, const Probe& read) {
    vector<Group> singles;
    Mask answer;
    while (!todo.empty()) {
        Group g = move(todo.back());
        todo.pop_back();
        int s = g.ids.size(), k = g.count;
        if (k == 0) continue;
        if (s == k) { answer = answer | maskOf(g.ids); continue; }
        if (k == 1 || k == s-1) { singles.push_back(move(g)); continue; }
        int h = action[s][k];
        if (binom[s][k] <= 32000) {
            vector<Mask> ways;
            subsets(g.ids,min(k,s-k),0,{},ways);
            Mask allowed=maskOf(g.ids);
            if (k>s-k) for (auto& b:ways) b=b ^ allowed;
            vector<Possibility> opts;
            for (auto& b : ways) opts.push_back({b,1,0});
            auto randomMask = [&](int) {
                Mask q;
                for (int i=0; i<words; ++i) q.w[i]=gen() & allowed.w[i];
                return q;
            };
            answer = answer | ways[distinguish(opts,read,randomMask)];
        } else if (h<0) {
            cubeDecode(g.ids,k,-h,read,answer);
        } else {
            vector<int> part(g.ids.begin(), g.ids.begin()+h);
            int j = read(maskOf(part));
            g.ids.erase(g.ids.begin(),g.ids.begin()+h);
            todo.push_back({move(g.ids),k-j});
            todo.push_back({move(part),j});
        }
    }
    // Several independent one-of-many choices can share additive probes.
    while (singles.size() >= 2) {
        int a = singles.size()-1, b = a-1;
        Group &x = singles[a], &y = singles[b];
        bool invX=x.count!=1, invY=y.count!=1;
        Mask allowed=maskOf(x.ids) | maskOf(y.ids), background;
        if (invX) background=maskOf(x.ids);
        if (invY) background=background | maskOf(y.ids);
        vector<Possibility> opts;
        opts.reserve(x.ids.size()*y.ids.size());
        for (int i:x.ids) for (int j:y.ids) {
            Mask delta;
            delta.set(i);
            delta.set(j);
            opts.push_back({background ^ delta,1,0});
        }
        auto randomMask=[&](int) {
            Mask q;
            for (int i=0; i<words; ++i) q.w[i]=gen() & allowed.w[i];
            return q;
        };
        answer=answer | opts[distinguish(opts,read,randomMask)].path;
        singles.pop_back();
        singles.pop_back();
    }
    if (!singles.empty()) {
        Group& g = singles[0];
        bool inv = g.count != 1;
        int lo=0, hi=g.ids.size();
        if (hi>2 && !inv && preferredEdge>=0) {
            auto it=find(g.ids.begin(),g.ids.end(),preferredEdge);
            if (it!=g.ids.end()) {
                Mask q;
                q.set(preferredEdge);
                if (read(q)==1) return answer | q;
                g.ids.erase(it);
                --hi;
            }
        }
        while (hi-lo>1) {
            int mid=(lo+hi)/2;
            Mask q;
            for (int i=lo; i<mid; ++i) q.set(g.ids[i]);
            int x=read(q);
            if ((inv ? mid-lo-x : x) == 1) hi=mid;
            else lo=mid;
        }
        for (int i=0; i<(int)g.ids.size(); ++i)
            if ((i==lo) != inv) answer.set(g.ids[i]);
    }
    return answer;
}

struct Node {
    int parent, label, depth;
    vector<int> block;
    Mask path;
};
struct Site { int end, prefix, extra; double weight; };
struct Cell {
    vector<int> ids;
    Mask mask;
    explicit Cell(vector<int> a) : ids(move(a)), mask(maskOf(ids)) {}
};
vector<Node> tree;
vector<int> unusedEdges, depthV, width, assigned;
vector<vector<int>> pending;

vector<Mask> prefixes(const vector<int>& ids) {
    vector<Mask> p(ids.size()+1);
    for (int i=0; i<(int)ids.size(); ++i) {
        p[i+1]=p[i];
        p[i+1].set(ids[i]);
    }
    return p;
}

// Within a compressed segment only the number of selected edges is known.
// A canonical prefix represents this count as long as probes take whole cells.
Mask initialSplit;
vector<int> signedLevel;
bool splitMode;
pair<int,Mask> resolve(const vector<Site>& sites, const Probe& read, int splitCount) {
    if (sites.size() == 1 && splitCount<0) {
        const auto& s = sites[0];
        const Node& u = tree[s.end];
        Mask base = s.end ? tree[u.parent].path : Mask{};
        Mask extra = recoverGroups({{u.block,s.prefix},{unusedEdges,s.extra}},read);
        return {0,base | extra};
    }
    vector<Cell> cells;
    vector<array<vector<Mask>,2>> pref(tree.size()+1);
    auto partition = [&](int i, const vector<int>& ids) {
        array<vector<int>,2> bins;
        for (int e:ids) bins[splitCount>=0 && initialSplit.has(e)].push_back(e);
        for (int j=0; j<2; ++j) {
            pref[i][j]=prefixes(bins[j]);
            if (!bins[j].empty()) cells.emplace_back(move(bins[j]));
        }
    };
    for (int i=0; i<(int)tree.size(); ++i) partition(i,tree[i].block);
    int outside=tree.size();
    partition(outside,unusedEdges);
    vector<Possibility> opts;
    for (int i=0; i<(int)sites.size(); ++i) {
        const Site& s=sites[i];
        Mask base=s.end ? tree[tree[s.end].parent].path : Mask{};
        auto& seg=pref[s.end];
        auto& ext=pref[outside];
        int a=seg[0].size()-1, b=seg[1].size()-1;
        int c=ext[0].size()-1, e=ext[1].size()-1;
        int need=splitCount<0 ? 0 : splitCount-common(initialSplit,base);
        for (int j=max({0,s.prefix-a,need-min(s.extra,e)});
             j<=min({b,s.prefix,need-max(0,s.extra-c)}); ++j) {
            int h=need-j;
            double prob=(binom[a][s.prefix-j]*binom[b][j]/binom[a+b][s.prefix])*
                        (binom[c][s.extra-h]*binom[e][h]/binom[c+e][s.extra]);
            Mask path=base | seg[0][s.prefix-j] | seg[1][j] | ext[0][s.extra-h] | ext[1][h];
            opts.push_back({path,max(1e-120,s.weight*prob),i});
        }
    }
    // If the explicit set is affordable, retain the correlation between
    // the attachment and the individual edge IDs until both are known.
    double combinations=0;
    if (opts.size()>1) for (auto& o:opts) {
        double multiplicity=1;
        for (auto& c:cells) {
            multiplicity*=binom[c.ids.size()][common(c.mask,o.path)];
            if (multiplicity>18000) break;
        }
        combinations+=multiplicity;
        if (combinations>18000) break;
    }
    if (opts.size()>1 && combinations<=18000) {
        vector<Possibility> full;
        full.reserve((size_t)(combinations+.5));
        for (auto& o:opts) {
            Mask base;
            vector<vector<Mask>> parts;
            int count=1;
            for (auto& c:cells) {
                int len=c.ids.size(), k=common(c.mask,o.path);
                if (k==len) base=base | c.mask;
                else if (k) {
                    auto& ways=parts.emplace_back();
                    subsets(c.ids,min(k,len-k),0,{},ways);
                    if (2*k>len) for (auto& x:ways) x=x ^ c.mask;
                    count*=ways.size();
                }
            }
            int start=full.size();
            full.push_back({base,o.weight/count,o.id});
            for (auto& ways:parts) {
                int size=full.size()-start;
                for (int j=1; j<(int)ways.size(); ++j)
                    for (int i=start; i<start+size; ++i)
                        full.push_back({full[i].path | ways[j],full[i].weight,o.id});
                for (int i=start; i<start+size; ++i) full[i].path=full[i].path | ways[0];
            }
        }
        auto randomMask=[&](int trial){
            Mask q;
            for (int i=0; i<words; ++i) q.w[i]=gen();
            if (trial%3==0) for (auto& c:cells) if (c.ids.size()>1 && (gen()&1)) {
                bool take=gen()&1;
                for (int i=0; i<words; ++i) {
                    if (take) q.w[i]|=c.mask.w[i];
                    else q.w[i]&=~c.mask.w[i];
                }
            }
            return q;
        };
        auto& best=full[distinguish(full,read,randomMask)];
        return {best.id,best.path};
    }
    // Include partial information about the unknown edge IDs in the same
    // probes that locate the attachment point.
    for (int round=0; round<14 && opts.size()>1; ++round) {
        int best = -1;
        double bestScore = 0;
        for (int c=0; c<(int)cells.size(); ++c) {
            int len=cells[c].ids.size(), h=len/2;
            if (!h) continue;
            double gain=0;
            int size=opts.size();
            for (auto& o : opts) {
                int k=common(o.path,cells[c].mask);
                int spread=min({h,k,len-k});
                size += spread;
                if (size>640) break;
                if (spread) gain += o.weight * (min(k,len-k)==1 ? 1.0 : 0.5);
            }
            if (size<=640 && gain>bestScore) best=c, bestScore=gain;
        }
        if (best<0) break;
        auto old = move(cells[best]);
        int len=old.ids.size(), h=len/2;
        cells[best] = Cell(vector<int>(old.ids.begin(),old.ids.begin()+h));
        cells.emplace_back(vector<int>(old.ids.begin()+h,old.ids.end()));
        auto left=prefixes(cells[best].ids), right=prefixes(cells.back().ids);
        vector<Possibility> next;
        for (auto& o : opts) {
            int k=common(o.path,old.mask);
            if (!k || k==len) { next.push_back(o); continue; }
            Mask base=o.path;
            for (int i=0; i<words; ++i) base.w[i] &= ~old.mask.w[i];
            for (int j=max(0,k-(len-h)); j<=min(h,k); ++j) {
                double prob=binom[h][j]*binom[len-h][k-j]/binom[len][k];
                next.push_back({base | left[j] | right[k-j],max(1e-120,o.weight*prob),o.id});
            }
        }
        opts=move(next);
    }
    vector<Mask> cellForEdge(m);
    for (auto& c : cells) for (int e:c.ids) cellForEdge[e]=c.mask;
    auto randomMask = [&](int) {
        Mask q;
        for (const Cell& c : cells) if (gen() & 1) q=q | c.mask;
        return q;
    };
    int result=distinguish(opts,read,randomMask,&cellForEdge);
    vector<Group> groups;
    Mask base;
    for (auto& c : cells) {
        int k=common(opts[result].path,c.mask);
        if (k==(int)c.ids.size()) base=base | c.mask;
        else if (k) groups.push_back({move(c.ids),k});
    }
    return {opts[result].id,base | recoverGroups(move(groups),read)};
}

int chooseDepth(const vector<int>& occupied, int height) {
    // First span long paths. At a narrow level preceding a much wider
    // level, resolving the common ancestors early saves repeated searches.
    int deepest=0;
    for (int d=1; d<=height; ++d)
        if (!pending[d].empty() && occupied[d]<width[d]) deepest=d;
    if (deepest) {
        for (int d=1; d<deepest; ++d) {
            if (!pending[d].empty() && occupied[d]<width[d] &&
                width[d+1]>=4*width[d] && (d==1 || width[d]==1)) return d;
        }
        return deepest;
    }
    int best=-1, value=-1;
    for (int i=1; i<(int)tree.size(); ++i) {
        int lo=tree[tree[i].parent].depth, hi=tree[i].depth;
        for (int d=lo+1; d<=hi; ++d) if (!pending[d].empty()) {
            int score=min(d-lo,hi-d);
            if (score>value) best=d, value=score;
        }
    }
    return best;
}

void insertPath(int v,int d,const Site& s,const Mask& path) {
    int attach=s.end;
    if (s.prefix<(int)tree[attach].block.size()) {
        int parent=tree[attach].parent;
        vector<int> prefix, suffix;
        for (int e:tree[attach].block)
            (path.has(e) ? prefix : suffix).push_back(e);
        int newId=tree.size();
        Mask p=tree[parent].path | maskOf(prefix);
        tree.push_back({parent,-1,d-s.extra,move(prefix),p});
        tree[attach].parent=newId;
        tree[attach].block=move(suffix);
        attach=newId;
    }
    if (s.extra) {
        vector<int> branch;
        erase_if(unusedEdges,[&](int e) {
            if (!path.has(e)) return false;
            branch.push_back(e);
            return true;
        });
        tree.push_back({attach,v,d,move(branch),path});
    } else tree[attach].label=v;
}

// Some input numberings assign each new vertex its incoming edge index.
// Use this only to propose probes, after several recovered edges agree.
int edgeHint(int v) {
    int matches=0, known=0;
    for (const auto& u:tree) if (u.label>0 && u.block.size()==1) {
        ++known;
        matches+=u.block[0]==u.label-1;
    }
    return matches>=3 && matches*10>=known*9 ? v-1 : -1;
}

// After one depth parity has been spanned, every exterior edge leads to a
// single remaining leaf. Interior positions are either known nodes or the
// middle of a two-edge segment.
void finishFringe(int v,int coreParity) {
    int h=signedLevel[v];
    preferredEdge=edgeHint(v);
    bool colored=depthV[v]<0;
    auto fits=[&](const Mask& p,int d){
        return (colored ? d-2*common(p,initialSplit) : d)==h;
    };
    vector<Site> sites;
    vector<Possibility> opts;
    auto position=[&](int i,int t,Mask p,int d,double wt) {
        if (!fits(p,d)) return;
        opts.push_back({p,wt,(int)sites.size()});
        sites.push_back({i,t,0,0});
    };
    for (int i=1; i<(int)tree.size(); ++i) {
        const auto& u=tree[i];
        if (u.label<0) position(i,u.block.size(),u.path,u.depth,1);
        if (u.block.size()==2) for (int e:u.block) {
            Mask p=tree[u.parent].path;
            p.set(e);
            position(i,1,p,u.depth-1,.5);
        }
    }
    array<vector<int>,2> byColor, parents;
    for (int e:unusedEdges) byColor[colored && initialSplit.has(e)].push_back(e);
    int coreCount=0;
    for (int i=0; i<(int)tree.size(); ++i) {
        const auto& u=tree[i];
        if ((u.depth&1)!=coreParity) continue;
        ++coreCount;
        for (int c=0; c<2; ++c) if (!byColor[c].empty()) {
            Mask p=u.path;
            p.set(byColor[c][0]);
            if (fits(p,u.depth+1)) parents[c].push_back(i);
        }
    }
    int numParents=parents[0].size()+parents[1].size();
    if (numParents==1 && opts.empty()) {
        int c=parents[0].empty(), i=parents[c][0], d=tree[i].depth+1;
        auto read=[&](const Mask& q){
            return (d+common(q,allEdges)-m+ask(v,toward ^ q))/2;
        };
        Mask p=tree[i].path | recoverGroups({{byColor[c],1}},read);
        insertPath(v,d,{i,(int)tree[i].block.size(),1,0},p);
        return;
    }
    size_t total=opts.size();
    for (int c=0; c<2; ++c) total+=parents[c].size()*byColor[c].size();
    opts.reserve(total);
    for (int c=0; c<2; ++c) for (int i:parents[c]) {
        for (int e:byColor[c]) {
            Mask p=tree[i].path;
            p.set(e);
            opts.push_back({p,(e==preferredEdge ? 1000.0 : 1.0)/coreCount,(int)sites.size()});
        }
        sites.push_back({i,(int)tree[i].block.size(),1,0});
    }
    auto query=[&](const Mask& q){return m-ask(v,toward ^ q);};
    Mask outsideMask=maskOf(unusedEdges);
    auto randomMask=[&](int trial){
        Mask q;
        for (int j=0; j<words; ++j) q.w[j]=gen();
        if (preferredEdge>=0 && trial%4==1) {
            for (int j=0; j<words; ++j) q.w[j]&=~outsideMask.w[j];
            q.set(preferredEdge);
        }
        return q;
    };
    int result=distinguish(opts,query,randomMask,nullptr,true);
    Mask p=opts[result].path;
    int d=common(p,allEdges), which=opts[result].id;
    insertPath(v,d,sites[which],p);
}

void solveTree() {
    prepare(m);
    int rootWeight=m-ask(0,{});
    unusedEdges.resize(m);
    iota(unusedEdges.begin(),unusedEdges.end(),0);
    auto rootProbe = [&](const Mask& q) {
        return (ask(0,q)-m+rootWeight+common(q,allEdges))/2;
    };
    toward=recoverGroups({{unusedEdges,rootWeight}},rootProbe);
    depthV.assign(n,-1);
    depthV[0]=0;
    signedLevel.assign(n,0);
    width.assign(n,0);
    assigned.assign(n,0);
    pending.resize(n);
    width[0]=assigned[0]=1;
    int height=0, sample=min(8,max(4,n/14)), oddSample=0;
    vector<int> order(m);
    iota(order.begin(),order.end(),1);
    shuffle(order.begin(),order.end(),gen);
    for (int j=0; j<sample; ++j) {
        int v=order[j];
        signedLevel[v]=depthV[v]=m-ask(v,toward);
        height=max(height,depthV[v]);
        oddSample+=depthV[v]&1;
    }
    // A level-order numbering with a fixed branching factor predicts both
    // endpoints when edge IDs follow the insertion order. A perfect reply
    // proves the entire path, independently of this numbering hypothesis.
    vector<Mask> predicted(n);
    vector<bool> certified(n);
    int hits=0;
    if (n>=18) {
        int first=height==1 ? m : 1, degree=0;
        for (int k=first; k<=m && !degree; ++k) {
            bool fits=true;
            for (int j=0; j<sample && fits; ++j) {
                int d=0;
                for (int u=order[j]; u; u=(u-1)/k) ++d;
                fits=d==depthV[order[j]];
            }
            if (fits) degree=k;
        }
        if (degree) {
            vector<int> modelDepth(n);
            for (int v=1; v<n; ++v) {
                int p=(v-1)/degree;
                predicted[v]=predicted[p];
                predicted[v].set(v-1);
                modelDepth[v]=modelDepth[p]+1;
            }
            int misses=0;
            for (int v:order) {
                if (ask(v,toward ^ predicted[v])==m) {
                    certified[v]=true;
                    signedLevel[v]=depthV[v]=modelDepth[v];
                    ++hits;
                } else if (++misses>hits/5) break;
            }
        }
    }
    splitMode=hits==0 && n>=15 && height<=min(16,n/8+3) &&
              (height<=4 || 3*max(oddSample,sample-oddSample)>=2*sample);
    for (int i=0; i<m; ++i) if (gen()&1) initialSplit.set(i);
    array<int,2> parity{1,0};
    for (int v=1; v<n; ++v) {
        if (depthV[v]<0) {
            if (splitMode) signedLevel[v]=m-ask(v,toward ^ initialSplit)-common(allEdges,initialSplit);
            else signedLevel[v]=depthV[v]=m-ask(v,toward);
        }
        ++parity[abs(signedLevel[v])%2];
    }
    int coreParity=parity[0]<=parity[1] ? 0 : 1;
    int count=0;
    vector<int> fringe;
    // Keep the depth for sampled vertices, even when they are in the fringe.
    vector<bool> measuredSplit(n);
    for (int v=1; v<n; ++v) {
        if (splitMode && abs(signedLevel[v])%2!=coreParity) {
            fringe.push_back(v);
            continue;
        }
        if (depthV[v]<0) {
            depthV[v]=m-ask(v,toward);
            measuredSplit[v]=true;
        }
        int d=depthV[v];
        ++width[d];
        ++count;
        pending[d].push_back(v);
        height=max(height,d);
    }
    // In preorder-numbered trees the depths also predict the ancestry.
    // Every prediction is certified by an exact Hamming-distance reply.
    vector<Mask> preorder(n);
    bool useOrder=!splitMode && n>=20;
    vector<int> ancestors{0};
    for (int v=1; v<n && useOrder; ++v) {
        int d=depthV[v];
        if (d>(int)ancestors.size()) { useOrder=false; break; }
        ancestors.resize(d);
        preorder[v]=preorder[ancestors.back()];
        preorder[v].set(v-1);
        ancestors.push_back(v);
    }
    tree.push_back({-1,0,0,{},{}});
    for (int step=0; step<count; ++step) {
        vector<int> occupied(n);
        occupied[0]=1;
        for (int i=1; i<(int)tree.size(); ++i)
            for (int d=tree[tree[i].parent].depth+1; d<=tree[i].depth; ++d)
                ++occupied[d];
        int d=chooseDepth(occupied,height);
        int v=pending[d].back();
        pending[d].pop_back();
        auto read = [&](const Mask& q) {
            return (ask(v,toward ^ q)-m+d+common(allEdges,q))/2;
        };
        vector<double> prior(d+1);
        double mass=1;
        for (int a=d; a>=0; --a) {
            int w=width[a];
            if (splitMode && a && (a&1)!=coreParity)
                w=max(occupied[a]+1,(width[a-1]+width[a+1]+1)/2);
            int available=w-(a==d ? assigned[a] : 0);
            prior[a]=mass/available;
            mass=prior[a]*(w-occupied[a]);
        }
        vector<Site> sites;
        if (prior[0]>0 && d<=(int)unusedEdges.size()) sites.push_back({0,0,d,prior[0]});
        for (int i=1; i<(int)tree.size(); ++i) {
            const auto& u=tree[i];
            int lo=tree[u.parent].depth;
            for (int a=lo+1; a<=min(d,u.depth); ++a) {
                if (a==d && a==u.depth && u.label>=0) continue;
                if (prior[a]>0 && d-a<=(int)unusedEdges.size()) sites.push_back({i,a-lo,d-a,prior[a]});
            }
        }
        preferredEdge=edgeHint(v);
        int idx=-1;
        Mask path;
        if (certified[v] || useOrder) {
            path=certified[v] ? predicted[v] : preorder[v];
            for (int i=0; i<(int)sites.size(); ++i) {
                Site s=sites[i];
                const Node& u=tree[s.end];
                int base=u.depth-(int)u.block.size();
                if (s.end && common(tree[u.parent].path,path)!=base) continue;
                if (common(u.path,path)!=base+s.prefix ||
                    common(maskOf(unusedEdges),path)!=s.extra) continue;
                bool forced=sites.size()==1 &&
                    binom[u.block.size()][s.prefix]*binom[unusedEdges.size()][s.extra] == 1;
                if (certified[v] || forced || ask(v,toward ^ path)==m) idx=i;
                break;
            }
            if (idx<0) useOrder=false;
        }
        if (idx<0) tie(idx,path)=resolve(sites,read,measuredSplit[v] ? (d-signedLevel[v])/2 : -1);
        Site s=sites[idx];
        insertPath(v,d,s,path);
        assigned[d]++;
    }
    stable_sort(fringe.begin(),fringe.end(),[&](int v,int u){
        if ((depthV[v]<0)!=(depthV[u]<0)) return depthV[v]<0;
        return abs(signedLevel[v])>abs(signedLevel[u]);
    });
    for (int v:fringe) finishFringe(v,coreParity);
    vector<pair<int,int>> edges(m);
    for (int i=1; i<(int)tree.size(); ++i) {
        int e=tree[i].block[0], p=tree[tree[i].parent].label, c=tree[i].label;
        edges[e]=toward.has(e) ? make_pair(c,p) : make_pair(p,c);
    }
    cout << "!";
    for (auto [u,v]:edges) cout << ' ' << u+1 << ' ' << v+1;
    cout << endl;
}

// For the smallest instances, retain the complete set of possible labeled
// trees, so that a probe at any vertex can finish the reconstruction.
void smallTree() {
    using State=array<unsigned char,8>;
    int weight=m-ask(0,{});
    vector<State> shapes, live;
    vector<int> par(n), ids(m);
    iota(ids.begin(),ids.end(),0);
    int limit=1<<m, trees=1;
    for (int i=1; i<n; ++i) trees*=n;
    for (int code=0; code<trees; ++code) {
        int rem=code;
        for (int v=1; v<n; ++v) par[v]=rem%n,rem/=n;
        State path{};
        bool ok=true;
        for (int v=1; v<n && ok; ++v) {
            for (int u=v; u; u=par[u]) {
                int bit=1<<(u-1);
                if (path[v]&bit) { ok=false; break; }
                path[v]|=bit;
            }
        }
        if (ok) shapes.push_back(path);
    }
    vector<int> orientations;
    if (n<=6) {
        for (int t=0; t<limit; ++t)
            if (__builtin_popcount((unsigned)t)==weight) orientations.push_back(t);
    } else {
        prepare(m);
        auto read=[&](const Mask& q){return (ask(0,q)-m+weight+common(q,allEdges))/2;};
        toward=recoverGroups({{ids,weight}},read);
        orientations.push_back((int)toward.w[0]);
        int orders=1;
        for (int j=2; j<=m; ++j) orders*=j;
        while (shapes.size()*orders>500000) {
            int bestVertex=0;
            double best=1e300;
            for (int v=1; v<n; ++v) {
                int hist[8]{};
                for (const auto& st:shapes) ++hist[__builtin_popcount((unsigned)st[v])];
                double entropy=0;
                for (int z:hist) if (z) entropy+=z*log(double(z));
                if (entropy<best) best=entropy,bestVertex=v;
            }
            int depth=m-ask(bestVertex,toward);
            erase_if(shapes,[&](const State& st){
                return __builtin_popcount((unsigned)st[bestVertex])!=depth;
            });
        }
    }
    do {
        int permuted[128]{};
        for (int x=1; x<limit; ++x)
            permuted[x]=permuted[x&(x-1)] | (1<<ids[__builtin_ctz((unsigned)x)]);
        for (const auto& st:shapes) {
            State p{};
            for (int v=1; v<n; ++v) p[v]=permuted[st[v]];
            for (int t:orientations) {
                State oriented=p;
                for (int v=0; v<n; ++v) oriented[v]^=t;
                live.push_back(oriented);
            }
        }
    } while (next_permutation(ids.begin(),ids.end()));
    while (live.size()>1) {
        double best=1e300;
        int vBest=0, qBest=0;
        for (int v=0; v<n; ++v) {
            int patterns[128]{};
            for (auto& st:live) ++patterns[st[v]];
            for (int q=0; q<limit/2; ++q) {
                int hist[8]{};
                for (int p=0; p<limit; ++p) hist[__builtin_popcount((unsigned)(p^q))]+=patterns[p];
                double score=0;
                for (int h:hist) if (h) score+=h*log(double(h));
                if (score<best) best=score,vBest=v,qBest=q;
            }
        }
        Mask q;
        q.w[0]=qBest;
        int diff=m-ask(vBest,q);
        erase_if(live,[&](const State& s){return __builtin_popcount((unsigned)(s[vBest]^qBest))!=diff;});
    }
    vector<pair<int,int>> edges(m);
    auto& s=live.front();
    for (int v=0; v<n; ++v) for (int u=0; u<v; ++u) {
        int diff=s[v]^s[u];
        if (__builtin_popcount((unsigned)diff)==1) {
            int e=__builtin_ctz((unsigned)diff);
            edges[e]=s[v]&diff ? make_pair(u,v) : make_pair(v,u);
        }
    }
    cout << "!";
    for (auto [a,b]:edges) cout << ' ' << a+1 << ' ' << b+1;
    cout << endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n<2 || n>MAXN) return 0;
    m=n-1;
    words=(m+63)/64;
    for (int i=0; i<m; ++i) allEdges.set(i);
    cacheReply.resize(n);
    if (n<=8) smallTree();
    else solveTree();
}
