#include <bits/stdc++.h>
using namespace std;

struct Set {
    array<uint64_t, 4> w{};
    void add(int i) { w[i/64] |= uint64_t(1) << (i%64); }
    bool has(int i) const { return (w[i/64] >> (i%64)) & 1; }
    int size() const {
        int s = 0;
        for (auto x : w) s += __builtin_popcountll(x);
        return s;
    }
    Set operator|(const Set &b) const {
        Set c;
        for (int j = 0; j < 4; ++j) c.w[j] = w[j] | b.w[j];
        return c;
    }
    Set operator-(const Set &b) const {
        Set c;
        for (int j = 0; j < 4; ++j) c.w[j] = w[j] & ~b.w[j];
        return c;
    }
};
struct Hash {
    size_t operator()(const Set &s) const {
        uint64_t h = 0;
        for (auto x : s.w) h = (h ^ (x + (h >> 29))) * 0x9e3779b97f4a7c15ULL;
        return h ^ (h >> 31);
    }
};
bool operator==(const Set &a, const Set &b) { return a.w == b.w; }

int n;
Set full;
unordered_map<Set,int,Hash> cache;
[[noreturn]] void answer(bool yes) {
    cout << "! " << int(yes) << endl;
    exit(0);
}
Set one(int v) { Set s; s.add(v); return s; }
int cover(const Set &s) {
    auto it = cache.find(s);
    if (it != cache.end()) return it->second;
    string bits(n, '0');
    for (int v = 0; v < n; ++v) if (s.has(v)) bits[v] = '1';
    cout << "? " << bits << endl;
    int q;
    if (!(cin >> q) || q < 0 || q > n-s.size()) exit(0);
    if (q == 0) answer(false); // Empty and full are always cached.
    int value = s.size()+q;
    if (s.size()==1 && value==n) answer(true);
    cache.emplace(s,value);
    return value;
}
Set take(const vector<int> &v, int l, int r) {
    Set s;
    for (int i=l; i<r; ++i) s.add(v[i]);
    return s;
}
vector<int> labels(const Set &s) {
    vector<int> v;
    for (int i=0; i<n; ++i) if (s.has(i)) v.push_back(i);
    return v;
}

// A group is contained in one graph component. Different groups have
// disjoint closed neighborhoods of their seeds. Every assigned member's
// entire closed neighborhood is covered by its group's seeds.
struct Group {
    Set seeds, members;
    int size;
};
vector<Group> groups;
Set assigned;
int total;
Set seeds() {
    Set s;
    for (auto &g:groups) s=s|g.seeds;
    return s;
}
void verify(const Group &g) {
    if (g.size==n) answer(true);
    if (g.size==g.members.size()) answer(false);
}

// Exactly one vertex is outside the connected group's coverage. A query
// with extra labels tests whether they hit that vertex's closed neighborhood.
void last_vertex(const Group &g) {
    vector<int> vs=labels(full-g.members);
    int l=0, r=vs.size();
    while (r-l>1) {
        int m=(l+r)/2;
        if (cover(g.seeds|take(vs,l,m))==n-1) l=m;
        else if (cover(g.seeds|take(vs,m,r))==n-1) r=m;
        else answer(true); // Two distinct labels touch it, so it has a neighbor.
    }
    answer(false);
}

struct Part {
    vector<int> ids;
    Set seeds;
    int weight=0;
    void add(int i) {
        ids.push_back(i);
        seeds=seeds|groups[i].seeds;
        weight+=groups[i].size;
    }
};

// Counted intersections with disjoint supports are additive. Only one
// side of each split needs a query.
void locate(const Set &x, int sx, const Part &p, int hit, vector<int> &out) {
    if (!hit) return;
    int smallest=n;
    for (int i:p.ids) smallest=min(smallest,groups[i].size);
    if (p.ids.size()==1 || hit>p.weight-smallest) {
        out.insert(out.end(),p.ids.begin(),p.ids.end());
        return;
    }
    Part a,b;
    for (int i:p.ids) {
        if (a.weight<=b.weight) a.add(i);
        else b.add(i);
    }
    int ha=sx+a.weight-cover(x|a.seeds);
    locate(x,sx,a,ha,out);
    locate(x,sx,b,hit-ha,out);
}

void merge_group(Group x, vector<int> ids, int hit) {
    // When only one old support contains the new one, keep the old seeds.
    bool contained=(ids.size()==1 && hit==x.size);
    if (contained) x.seeds=groups[ids[0]].seeds;
    sort(ids.rbegin(),ids.rend());
    total+=x.size-hit;
    x.size-=hit;
    for (int i:ids) {
        x.members=x.members|groups[i].members;
        x.seeds=x.seeds|groups[i].seeds;
        x.size+=groups[i].size;
        groups.erase(groups.begin()+i);
    }
    assigned=assigned|x.members;
    verify(x);
    groups.push_back(x);
}
Part all_except(int skip) {
    Part p;
    vector<int> ids;
    for (int i=0;i<(int)groups.size();++i) if (i!=skip) ids.push_back(i);
    stable_sort(ids.begin(),ids.end(),[](int a,int b){return groups[a].size>groups[b].size;});
    for (int i:ids) p.add(i);
    return p;
}
void insert_group(const Set &s, int size, int joint) {
    int hit=size+total-joint;
    vector<int> hits;
    if (hit) locate(s,size,all_except(-1),hit,hits);
    merge_group({s,s,size},hits,hit);
}

// With full coverage, search for a vertex whose closed neighborhood meets
// both a chosen group and its complement. Whole pure batches can be skipped.
bool bridge(int id, const Part &other, const vector<int> &v, int l, int r) {
    Group &g=groups[id];
    Set t=take(v,l,r);
    int a=cover(other.seeds|t)-other.weight;
    if (!a) return false;
    auto known=cache.find(t);
    int b=known!=cache.end() ? known->second-a : cover(g.seeds|t)-g.size;
    if (!b) {
        g.members=g.members|t;
        assigned=assigned|t;
        verify(g);
        return false;
    }
    if (r==l+1) {
        vector<int> ids{id};
        locate(t,a+b,other,b,ids);
        merge_group({t,t,a+b},ids,a+b);
        return true;
    }
    int m=(l+r)/2;
    return bridge(id,other,v,l,m) || bridge(id,other,v,m,r);
}
// For two supports that partition V, every label touching the first support
// is either wholly on that side or bridges the two. Test the second condition
// once for a batch of such labels.
void finish_pair(int id) {
    auto &a=groups[id], &b=groups[1-id];
    auto vs=labels(full-assigned);
    int miss=a.size-a.members.size();
    int block=max(1,int(vs.size())/(2*miss));
    Set touching;
    auto flush=[&]() {
        if (!touching.size()) return;
        if (cover(a.seeds|touching)>a.size) answer(true);
        a.members=a.members|touching;
        verify(a);
        touching=Set{};
    };
    for (int l=0;l<(int)vs.size();l+=block) {
        int r=min(int(vs.size()),l+block);
        Set t=take(vs,l,r);
        if (block>1 && cover(b.seeds|t)==b.size) {
            b.members=b.members|t;
            verify(b);
            continue;
        }
        for (int i=l;i<r;++i) {
            Set x=one(vs[i]);
            int hit=cover(b.seeds|x)-b.size;
            if (hit==0) {
                b.members=b.members|x;
                verify(b);
            } else {
                // An unassigned vertex inside a's support touches both
                // itself and a seed of a. A single hit forces a bridge.
                if (hit==1) answer(true);
                auto it=cache.find(x);
                if (it!=cache.end() && it->second>hit) answer(true);
                touching=touching|x;
                if (hit<a.size && hit==2 && cover(a.seeds|x)>a.size) answer(true);
            }
        }
        if (touching.size()>=12) flush();
    }
    flush();
    answer(false);
}

void resolve_full() {
    int id=0;
    for (int i=1;i<(int)groups.size();++i)
        if (groups[i].size-groups[i].members.size()<groups[id].size-groups[id].members.size())
            id=i;
    if (groups.size()==2) finish_pair(id);
    // Late in the search, assign the remaining vertices once each. This
    // also bounds the cost of repeating bridge searches after mergers.
    static const size_t initial_queries=cache.size();
    if (n-assigned.size()<n/2 || cache.size()-initial_queries>size_t(n)) {
        Set x=one(labels(full-assigned).front());
        insert_group(x,cover(x),n);
        return;
    }
    const auto other=all_except(id);
    auto v=labels(full-assigned);
    // For a small side most blocks miss it. For sides of comparable size,
    // short blocks save the queries spent descending through mixed blocks.
    int missing=groups[id].size-groups[id].members.size();
    int batch=max(1,int(v.size())/max(1,missing*2));
    for (int l=0;l<(int)v.size();l+=batch)
        if (bridge(id,other,v,l,min((int)v.size(),l+batch))) return;
    answer(false);
}

// Try to complete a small component early. A whole block disjoint from the
// current support can be set aside until that support grows.
bool seek(int id, const vector<int> &v, int l, int r, bool must_hit) {
    Group &g=groups[id];
    Set t=take(v,l,r);
    int joint=cover(t|g.seeds);
    if (joint==g.size) {
        g.members=g.members|t;
        assigned=assigned|t;
        verify(g);
        return false;
    }
    if (r==l+1) {
        int st=cover(t);
        if (st+g.size==joint) return false;
        insert_group(t,st,cover(t|seeds()));
        return true;
    }
    if (!must_hit && cover(t)+g.size==joint) return false;
    int m=(l+r)/2;
    if (seek(id,v,l,m,false)) return true;
    return seek(id,v,m,r,false);
}

// Look for a sparse closed neighborhood inside a small block. Splitting
// keeps a witness whenever all its neighbors lie on one side. In particular
// an isolated vertex always survives in the half containing it.
void sparse_witness(vector<int> v) {
    while (v.size()>1) {
        int m=v.size()/2;
        Set left=take(v,0,m),right=take(v,m,v.size());
        if (cover(full-left)<n) v.resize(m);
        else if (cover(full-right)<n) v.erase(v.begin(),v.begin()+m);
        else {
            // Every surviving witness meets both halves. A two-label block
            // is therefore a connected pair.
            if (v.size()==2) cover(left|right);
            return;
        }
    }
    answer(false);
}

void probe_sparse() {
    int half=n/2;
    vector<int> vs=labels(full);
    Set l=take(vs,0,half),r=full-l;
    int fl=cover(l),fr=cover(r);
    // A dominating half often contains a hub. Try to narrow it to a single
    // vertex, keeping a dominating block throughout.
    if (fl==n || fr==n) {
        vector<int> w=labels(fl==n ? l : r);
        while (w.size()>1) {
            int mid=w.size()/2;
            if (cover(take(w,0,mid))==n) w.resize(mid);
            else if (cover(take(w,mid,w.size()))==n) w.erase(w.begin(),w.begin()+mid);
            else break;
        }
    } else {
        // Small samples contain an isolated vertex much more often than
        // an entire high-degree neighborhood. Test several disjoint samples.
        mt19937 rng(417719+n);
        shuffle(vs.begin(),vs.end(),rng);
        int len=max(2,n/9);
        for (int i=0;i<n;i+=len) {
            Set block=take(vs,i,min(i+len,n));
            if (cover(full-block)<n) {
                sparse_witness(labels(block));
            }
        }
    }
}

// The only neighbor of a leaf u is in T exactly when T already covers
// the whole closed neighborhood of u (with u outside T).
int leaf_neighbor(int leaf) {
    Set u=one(leaf);
    vector<int> vs=labels(full-u);
    int l=0,r=vs.size();
    while (r-l>1) {
        int mid=(l+r)/2;
        Set half=take(vs,l,mid);
        int f=cover(half);
        if (f==n || cover(half|u)==f) r=mid;
        else l=mid;
    }
    return vs[l];
}

// A component of size h outside g must meet every batch that leaves
// fewer than h vertices uncovered. Disjoint batches need distinct labels.
bool residual_tried[201]{};
void residual(const Group &g) {
    int m=n-g.size;
    const auto vs=labels(full-g.members);
    int k=m+1;
    vector<int> deficits;
    for (int i=0;i<k;++i) {
        Set t=take(vs,i*vs.size()/k,(i+1)*vs.size()/k);
        deficits.push_back(n-cover(g.seeds|t));
        // If this batch missed every residual vertex, even the last size
        // m cannot be excluded by k batches.
        if (deficits.back()==m) return;
    }
    sort(deficits.begin(),deficits.end());
    for (int h=1;h<=m;++h) if (deficits[h]>=h) return;
    answer(true);
}

// Outside the component of the initial seeds, a vertex needs a distinct
// closed neighbor in each disjoint color class that dominates together with
// those seeds. This gives a lower bound shared by many future insertions.
int outside_minimum=1;
int exception_vertex=-1;

void color_bound(const Group &g, int classes, bool check_endpoint) {
    bool checked=false, end_leaf=false;
    int good=0;
    for (int j=0;j<classes;++j) {
        Set s=g.seeds;
        for (int i=j;i<n;i+=classes) s.add(i);
        int f=cover(s);
        if (f==n) {
            ++good;
        } else if (check_endpoint && f==n-1 && !s.has(n-1) && !s.has(n-2)) {
            if (!checked) {
                checked=true;
                Set end=one(n-1),prev=one(n-2);
                end_leaf=cover(end)==2 && cover(prev|end)==cover(prev);
            }
            if (end_leaf) {
                // The endpoint is the unique missed vertex.
                ++good;
                exception_vertex=n-1;
            }
        }
    }
    outside_minimum=max(outside_minimum,good);
}

// On very small inputs the complete set of graphs fits in memory. Discard
// candidates after each answer and stop as soon as their decisions agree.
void small_graph() {
    const int limit=1<<n;
    vector<pair<int,int>> edges;
    for (int i=0;i<n;++i)
        for (int j=0;j<i;++j) edges.emplace_back(i,j);
    struct Model {
        array<unsigned char,64> size{};
        bool connected;
    };
    vector<Model> models(1<<edges.size());
    vector<int> alive(models.size());
    iota(alive.begin(),alive.end(),0);
    for (int graph:alive) {
        int adj[6], reaches[64]{};
        for (int i=0;i<n;++i) adj[i]=1<<i;
        for (int e=0;e<(int)edges.size();++e) if (graph>>e&1) {
            auto [i,j]=edges[e];
            adj[i]|=1<<j;
            adj[j]|=1<<i;
        }
        for (int t=1;t<limit;++t) {
            reaches[t]=reaches[t&(t-1)]|adj[__builtin_ctz(t)];
            models[graph].size[t]=__builtin_popcount(reaches[t]);
        }
        int t=1;
        while (reaches[t]!=t) t=reaches[t];
        models[graph].connected=(t==limit-1);
    }
    int t=1;
    while (true) {
        Set q;
        q.w[0]=t;
        int f=cover(q);
        erase_if(alive,[&](int g){return models[g].size[t]!=f;});
        int yes=count_if(alive.begin(),alive.end(),[&](int g){return models[g].connected;});
        if (!yes || yes==(int)alive.size()) answer(yes!=0);
        // Minimize pairs of remaining graphs with opposite decisions, with
        // the expected number of surviving ambiguous models as a tie breaker.
        int64_t best=LLONG_MAX;
        for (int mask=1;mask<limit-1;++mask) {
            int cnt[7][2]{};
            for (int g:alive) ++cnt[models[g].size[mask]][models[g].connected];
            int64_t pairs=0, ambiguous=0;
            for (auto &c:cnt) if (c[0] && c[1]) {
                pairs+=int64_t(c[0])*c[1];
                ambiguous+=c[0]+c[1];
            }
            int64_t score=pairs*(alive.size()+1)+ambiguous;
            if (score<best) {
                best=score;
                t=mask;
            }
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin>>n) || n<0 || n>200) return 0;
    if (n<=1) answer(true);
    for (int i=0;i<n;++i) full.add(i);
    cache[Set{}]=0;
    cache[full]=n;
    if (n<=6) small_graph();
    Set start=one(0);
    int first=cover(start);
    groups.push_back({start,start,first});
    assigned=start;
    total=first;
    if (n>3 && first<n) {
        Set interval;
        for (int i=0;i<first;++i) interval.add(i);
        cover(interval);
    }

    if (first==2 && n>=8) {
        Set next=one(1);
        int together=cover(start|next);
        int d=cover(next);
        if (d!=together && (d==2 || together==d+2)) {
            int w=leaf_neighbor(0);
            Set x=one(w);
            groups[0]={x,x|start,cover(x)};
            assigned=x|start;
            total=groups[0].size;
            verify(groups[0]);
        } else if (d==together) {
            groups[0]={next,start|next,d};
            assigned=start|next;
            total=d;
            verify(groups[0]);
        }
    }
    bool sparse_checked=false;
    if (n>=20 && first<=3 && assigned.has(1)) {
        Set half;
        for (int i=0;i<n/2;++i) half.add(i);
        cover(half);
    }
    int last_ordered=-1;
    int trial_budget=1;
    int redundant=0;
    bool colors_checked=false;
    while (true) {
        for (auto &g:groups) if (g.size==n-1) last_vertex(g);
        if (total<n && groups.size()==1) {
            int m=n-total;
            if (m<outside_minimum && exception_vertex<0) answer(true);
            bool ordered=first<=3 && last_ordered>=4 && last_ordered<=8 && total>=6;
            bool dense=first>=10 && total>first && total>=n/2 &&
                       groups[0].members.size()>=3 && m>=8;
            if (!colors_checked && n>=30 && (ordered || dense)) {
                colors_checked=true;
                color_bound(groups[0],ordered ? 3 : clamp(first/6,3,9),ordered);
            }
            if (m>=2 && m<=7 && first>=10 && first<=n/2 && !residual_tried[m]) {
                residual_tried[m]=true;
                residual(groups[0]);
            }
        }
        if (total==n) {
            resolve_full();
            continue;
        }
        // On a small component, locating its few remaining labels is faster
        // than scanning the entire vertex list.
        if (!sparse_checked && n>=20 && first<=16 && groups.size()>1 &&
            (first>=4 || trial_budget==0 || groups[0].size>4)) {
            sparse_checked=true;
            probe_sparse();
        }
        if (groups.size()>=2 && trial_budget>0 && groups[0].size<=4) {
            auto v=labels(full-assigned);
            --trial_budget;
            if (!seek(0,v,0,v.size(),true)) answer(false);
            continue;
        }
        Set p=seeds();
        auto v=labels(full-assigned);
        int pick=v.front();
        // Test the vertex just past the covered prefix on ordered instances.
        // Preserve locality when a sequence of neighboring groups develops.
        if (groups.size()==1 && first<=3 && n>2 && last_ordered+2<n &&
            !assigned.has(last_ordered<0 ? 2 : last_ordered+2))
            pick=last_ordered<0 ? 2 : last_ordered+2;
        else if (groups.size()==1 && total<n && !assigned.has(total))
            pick=total;
        else if (assigned.size()==1 && n>2) pick=1;
        if (redundant>=2) {
            // Find a vertex that contributes new coverage. Removed batches
            // are consumed only if their entire support lies in one group.
            int l=0,r=v.size();
            while (r-l>1) {
                int m=(l+r)/2;
                Set t=take(v,l,m);
                if (cover(p|t)>total) r=m;
                else {
                    if (groups.size()==1) {
                        groups[0].members=groups[0].members|t;
                        assigned=assigned|t;
                        verify(groups[0]);
                    }
                    l=m;
                }
            }
            pick=v[l];
        }
        Set t=one(pick);
        int old_total=total;
        if (groups.size()==1 && outside_minimum>=3 && first>=10) {
            int count=min(12, (outside_minimum-1)*n/((n-total)*first));
            if (count>=2) {
                Set batch=t;
                for (int u:v) {
                    if (batch.size()>=count) break;
                    batch.add(u);
                }
                if (cover(p|batch)-total<outside_minimum) t=batch;
            }
        }
        int joint=cover(p|t);
        if (groups.size()==1 && (joint==total ||
            (joint-total<outside_minimum &&
             (exception_vertex<0 || !t.has(exception_vertex))))) {
            if (joint>total) groups[0].seeds=p|t;
            groups[0].size=total=joint;
            groups[0].members=groups[0].members|t;
            assigned=assigned|t;
            verify(groups[0]);
        } else {
            int st=cover(t);
            if (groups.size()==1 && st+total==joint) {
                // A prefix of this size is a cheap candidate cut when the
                // current group has grown in label order.
                Set prefix;
                for (int i=0;i<total;++i) prefix.add(i);
                if ((groups[0].members-prefix).size()==0) cover(prefix);
            }
            insert_group(t,st,joint);
        }
        last_ordered=pick;
        redundant=joint==old_total && joint<n ? redundant+1 : 0;
    }
}
