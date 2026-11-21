#include <bits/stdc++.h>
using namespace std;

constexpr int MAXV = 600;
using Set = bitset<MAXV>;
int n, m;
vector<int> order, leaves;
int coreEdges;
Set universe;
unordered_map<Set, int> memo;
struct Independent { Set vertices; int size; };
vector<Independent> zeros;

Set one(int v) {
    Set s;
    s.set(v);
    return s;
}
Set interval(int lo, int hi) {
    Set s;
    for (int i = lo; i < hi; ++i) s.set(i);
    return s;
}
vector<int> elements(const Set& s) {
    vector<int> a;
    for (size_t i = s._Find_first(); i < MAXV; i = s._Find_next(i)) a.push_back(i);
    return a;
}
void printList(const vector<int>& a) {
    for (size_t i = 0; i < a.size(); ++i) {
        if (i) cout << ' ';
        cout << a[i] + 1;
    }
    cout << endl;
}
[[noreturn]] void cycle(const vector<int>& c);
int edges(const Set& s) {
    int k = s.count();
    if (k < 2) return 0;
    auto it = memo.find(s);
    if (it != memo.end()) return it->second;
    for (auto z=zeros.rbegin();z!=zeros.rend();++z)
        if (k<=z->size && (s & ~z->vertices).none()) return 0;
    cout << "? " << k << endl;
    printList(elements(s));
    int e;
    if (!(cin >> e) || e < 0 || e > k * (k-1)/2) exit(0);
    if (k>=3 && e==k*(k-1)/2) {
        auto a=elements(s);
        a.resize(3);
        cycle(a);
    }
    if (!e) {
        erase_if(zeros,[&](const Independent& z) {
            return z.size<=k && (z.vertices & ~s).none();
        });
        zeros.push_back({s,k});
    }
    memo.emplace(s, e);
    return e;
}
void restoreLeaves(Set& a);
[[noreturn]] void partition(Set a) {
    restoreLeaves(a);
    Set b = universe ^ a;
    auto side = elements((b.none() || a.count() <= b.count()) ? a : b);
    cout << "Y " << side.size() << endl;
    printList(side);
    exit(0);
}
[[noreturn]] void cycle(const vector<int>& c) {
    cout << "N " << c.size() << endl;
    printList(c);
    exit(0);
}
void verify(const Set& a) {
    if (edges(a) || edges(universe ^ a)) return;
    partition(a);
}
Set firstK(const Set& s, int k) {
    Set a;
    for (size_t i = s._Find_first(); k-- > 0; i = s._Find_next(i)) a.set(i);
    return a;
}

// On a complete bipartite graph the count for k vertices is x*(k-x).
int smallRoot(int k, int e) {
    int d = k*k - 4*e;
    if (d < 0) return -1;
    int r = (int)sqrt(d);
    if (r*r != d || (k-r)%2) return -1;
    return (k-r)/2;
}

struct Matrix {
    vector<vector<int>> row;
    int cols = 0, weight = 0;
};
vector<Matrix> codes;
void makeCodes() {
    Matrix base;
    base.row = {{1,1,0,1}, {1,0,1,1}, {0,1,1,1}};
    base.cols = 4;
    base.weight = 3;
    codes.push_back(base);
    while (codes.back().cols < n) {
        const auto& a = codes.back();
        int r = a.row.size(), k = a.cols;
        Matrix b;
        b.cols = 2*k+r;
        b.row.assign(2*r+1, vector<int>(b.cols));
        for (int i=0; i<r; ++i) {
            for (int j=0; j<k; ++j) {
                b.row[i][j] = b.row[i][k+j] = b.row[r+i][j] = a.row[i][j];
                b.row[r+i][k+j] = 1-a.row[i][j];
            }
            b.row[i][2*k+i] = 1;
        }
        fill(b.row.back().begin()+k, b.row.back().begin()+2*k, 1);
        for (const auto& row : b.row)
            b.weight = max(b.weight, accumulate(row.begin(), row.end(), 0));
        codes.push_back(move(b));
    }
}
// Decode the binary vector from its subset sums.
bool decode(int level, const vector<int>& t, vector<int>& result) {
    if (!level) {
        for (int mask=0; mask<16; ++mask) {
            bool ok = true;
            for (int i=0; i<3; ++i) {
                int v=0;
                for (int j=0; j<4; ++j) v += codes[0].row[i][j] * ((mask>>j)&1);
                if (v != t[i]) ok = false;
            }
            if (ok) {
                for (int j=0; j<4; ++j) result.push_back((mask>>j)&1);
                return true;
            }
        }
        return false;
    }
    int r = codes[level-1].row.size();
    vector<int> x(r), y(r), z(r);
    for (int i=0; i<r; ++i) {
        int v = t[i]+t[r+i]-t.back();
        if (v<0) return false;
        z[i] = v%2;
        x[i] = v/2;
        y[i] = t[i]-x[i]-z[i];
    }
    if (!decode(level-1, x, result) || !decode(level-1, y, result)) return false;
    result.insert(result.end(), z.begin(), z.end());
    return true;
}
int missingU=-1, missingV=-1;
int modelEdges(const Set& s) {
    return edges(s) + (missingU>=0 && s[missingU] && s[missingV]);
}
bool minorityColors(const Set& s, int t, bool pivotMinor, Set& minority) {
    int k = s.count();
    if (t < 0 || t > k) return false;
    if (!t) return true;
    if (t==k) { minority |= s; return true; }
    Set l = firstK(s, k/2);
    int h=l.count(), x=smallRoot(h, modelEdges(l));
    if (x<0) return false;
    auto possible = [&](int u) { return u<=t && t-u<=k-h; };
    if (!possible(x)) x=h-x;
    if (!possible(x)) return false;
    if (x!=h-x && possible(h-x)) {
        int diff = modelEdges(l | one(0)) - modelEdges(l);
        x = pivotMinor ? h-diff : diff;
        if (x<0 || x>h || !possible(x) || x*(h-x)!=modelEdges(l)) return false;
    }
    return minorityColors(l,x,pivotMinor,minority) &&
           minorityColors(s^l,t-x,pivotMinor,minority);
}
void tryComplete(int a, const Set& odds) {
    int h = odds.count(), r = smallRoot(h, modelEdges(odds));
    if (r<0 || r>a) return;
    if (a==1) {
        Set candidates=r ? odds : universe^odds;
        while (candidates.count()>1) {
            Set half=firstK(candidates,candidates.count()/2);
            Set q=half;
            if (q.count()==1) q|=firstK(universe^candidates,1);
            int t=modelEdges(q);
            if (t==(int)q.count()-1) candidates=half;
            else if (!t) candidates^=half;
            else return;
        }
        verify(candidates);
        return;
    }
    if (a*8 <= n) {
        int d = modelEdges(universe)-modelEdges(universe ^ one(0));
        if (d!=a && d!=n-a) return;
        bool pivotMinor = d==n-a;
        Set minor;
        if (pivotMinor) minor.set(0);
        if (minorityColors(universe ^ one(0), a-pivotMinor, pivotMinor, minor))
            verify(minor);
        return;
    }
    if (codes.empty()) makeCodes();
    Set sides[2];
    sides[0].set(0);
    for (int i=1; i<n;) {
        int ref = sides[1].count() > sides[0].count();
        int level = (int)codes.size()-1;
        while (level>=0 && (codes[level].cols>n-i ||
               codes[level].weight>(int)sides[ref].count())) --level;
        if (level<0) {
            int p=sides[ref]._Find_first();
            sides[ref ^ modelEdges(one(i)|one(p))].set(i);
            ++i;
            continue;
        }
        const Matrix& code=codes[level];
        vector<int> sums, bits;
        for (const auto& row : code.row) {
            Set q;
            for (int j=0; j<code.cols; ++j) if (row[j]) q.set(i+j);
            int k=q.count();
            int t=smallRoot(2*k, modelEdges(q | firstK(sides[ref],k)));
            if (t<0) return;
            sums.push_back(t);
        }
        if (!decode(level,sums,bits)) return;
        for (int bit : bits) sides[ref^bit].set(i++);
    }
    if ((int)min(sides[0].count(),sides[1].count())==a) verify(sides[0]);
}

// On odd subsets every complete bipartite graph has an even edge
// count. With one exceptional edge and a pivot outside it, the parity
// test is a group test for the pair of exceptional endpoints.
void trySingleDefect(int delta, int a, const Set& odds) {
    int k=odds.count(), e=edges(odds);
    auto fits=[&](int t) {
        if (t<0) return false;
        int r=smallRoot(k,t);
        return r>=0 && r<=a;
    };
    if (!fits(e) && !fits(e-delta)) return;
    for (int pivot : {0,n/2,n-1}) {
        auto odd=[&](Set s) {
            if (!(s.count()%2)) s.set(pivot);
            return edges(s)%2!=0;
        };
        Set s=universe^one(pivot);
        if (!odd(s)) continue;
        int u=-1,v=-1;
        while (s.count()>=2) {
            if (s.count()==2) {
                u=s._Find_first();v=s._Find_next(u);
                break;
            }
            Set l=firstK(s,s.count()/2), r=s^l;
            if (odd(l)) {s=l;continue;}
            if (odd(r)) {s=r;continue;}
            while (l.count()>1) {
                Set half=firstK(l,l.count()/2);
                if (odd(half|r)) l=half;
                else l^=half;
            }
            while (r.count()>1) {
                Set half=firstK(r,r.count()/2);
                if (odd(l|half)) r=half;
                else r^=half;
            }
            u=l._Find_first();v=r._Find_first();
            break;
        }
        if (u<0) continue;
        Set uv=one(u)|one(v);
        if (edges(uv|one(pivot))==3) cycle({pivot,u,v});
        if (delta==-1 && !edges(uv)) {
            missingU=u;missingV=v;
            tryComplete(a,odds);
            missingU=missingV=-1;
        } else if (delta==1 && edges(uv)) {
            // For a single extra edge its endpoints have the same
            // neighbors outside the pair. Verify the resulting triangle.
            s=universe^uv;
            int t=m-edges(s)-1;
            while (s.count()>1 && t>0) {
                Set half=firstK(s,s.count()/2);
                int x=edges(half|uv)-edges(half)-1;
                if (x) {s=half;t=x;}
                else s^=half;
            }
            if (s.count()==1 && t==2) cycle({u,v,(int)s._Find_first()});
        }
    }
}

// In the natural odd cycle, the two odd-indexed neighbors of an even
// vertex have consecutive Gray codes. Every forbidden neighbor differs
// from both in a common bit, so these queries cover all forbidden edges.
void tryOrderedCycle(const Set& odds) {
    if (m!=n || n%2==0 || edges(odds)!=1 || edges(universe^odds)!=0 ||
        !edges(one(0)|one(n-1))) return;
    int h=(n+1)/2;
    for (int b=1; b<h; b*=2) {
        Set group[2];
        for (int i=0; i<h; ++i) {
            int c=((i^(i>>1)) & b)!=0;
            group[c].set(2*i);
            if (i+1<h) {
                int d=(((i+1)^((i+1)>>1)) & b)!=0;
                if (c==d) group[1-c].set(2*i+1);
            }
        }
        for (const auto& s : group)
            if (edges(s) != (int)(s[0] && s[n-1])) return;
    }
    cycle(elements(universe));
}

// A component is a tree of joins. The edge from a join vertex to a
// child is known to enter 'targets'. Its endpoint is located only if
// that particular bridge is needed in the final odd cycle.
struct Bridge { int child, count; Set targets; };
struct Component { Set side[2], all; vector<Bridge> child; int edges=0; };
int owner[MAXV];
vector<Component> node;
vector<int> roots;
int hits[MAXV][2];
int prefixEdges=0, surplus;

int neighbor(int v, Set s, int t, int prefer=-1) {
    if (t==(int)s.count()) return prefer>=0 && s[prefer] ? prefer : (int)s._Find_first();
    if (prefer>=0 && s[prefer] && (s.count()<=3 || abs(v-prefer)==1 ||
                                      t*3>=(int)s.count())) {
        if (edges(one(v)|one(prefer))) return prefer;
        s.reset(prefer);
    }
    while (t<(int)s.count()) {
        int k=s.count();
        // A smaller sample is usually sufficient when there are many neighbors.
        int h=max(1,k/(t+1));
        Set l=firstK(s,h);
        int found=edges(one(v)|l);
        if (found) { s=l; t=found; }
        else s ^= l;
    }
    return (int)s._Find_first();
}
vector<int> path(int r, int u, int v) {
    if (u==v) return {u};
    const Bridge *a=nullptr, *b=nullptr;
    for (const Bridge& e : node[r].child) {
        if (node[e.child].all[u]) a=&e;
        if (node[e.child].all[v]) b=&e;
    }
    if (a && a==b) return path(a->child,u,v);
    vector<int> p;
    if (a) {
        int z=neighbor(r,a->targets,a->count,a->targets[u]?u:a->child);
        p=path(a->child,u,z);
    }
    p.push_back(r);
    if (b) {
        int z=neighbor(r,b->targets,b->count,b->targets[v]?v:b->child);
        auto q=path(b->child,z,v);
        p.insert(p.end(),q.begin(),q.end());
    }
    return p;
}
struct Basket { int root; Set s; int capacity; };
void distribute(int v, int side, const vector<Basket>& b, int lo, int hi,
                int count, int capacity) {
    if (!count) return;
    if (hi-lo==1) {
        hits[b[lo].root][side]=count;
        return;
    }
    if (count==capacity) {
        for (int i=lo; i<hi; ++i) hits[b[i].root][side]=b[i].capacity;
        return;
    }
    int mid=lo, left=0;
    Set s;
    do {
        left+=b[mid].capacity;
        s|=b[mid++].s;
    } while (mid+1<hi && 2*left+b[mid].capacity<capacity);
    int t=edges(one(v)|s);
    distribute(v,side,b,lo,mid,t,left);
    distribute(v,side,b,mid,hi,count-t,capacity-left);
}
void mergeVertex(int v,int degree) {
    int biggest=-1, color=0;
    for (int r : roots) {
        if (hits[r][0] && hits[r][1]) {
            int u=neighbor(v,node[r].side[0],hits[r][0],r);
            int w=neighbor(v,node[r].side[1],hits[r][1],r);
            auto p=path(r,u,w);
            p.push_back(v);
            cycle(p);
        }
        if ((hits[r][0] || hits[r][1]) && (int)node[r].all.count()>biggest) {
            biggest=node[r].all.count();
            color=hits[r][0] ? 1 : 0;
        }
    }
    vector<int> next;
    auto& c=node[v];
    c.side[color].set(v);
    for (int r : roots) {
        int side=hits[r][1] ? 1 : 0, t=hits[r][side];
        if (!t) { next.push_back(r); continue; }
        surplus-=t-1;
        c.child.push_back({r,t,node[r].side[side]});
        c.side[color^1] |= node[r].side[side];
        c.side[color] |= node[r].side[side^1];
    }
    c.all=c.side[0]|c.side[1];
    for (int x : elements(c.all)) owner[x]=v;
    c.edges=degree;
    for (const Bridge& e : c.child) c.edges+=node[e.child].edges;
    next.push_back(v);
    roots.swap(next);
    prefixEdges+=degree;
}

void insertVertex(int index) {
    int v=order[index];
    memset(hits,0,sizeof hits);
    int degree=0;
    for (int side=0; side<2; ++side) {
        Set s;
        vector<Basket> b;
        int cap=0;
        for (int r : roots) {
            const Set& part=node[r].side[side];
            if (part.none() || (!surplus && side && hits[r][0])) continue;
            int c=surplus ? part.count() : 1;
            cap+=c;
            s|=part;
            b.push_back({r,part,c});
        }
        int t=0;
        if (!b.empty()) {
            if (index+1==(int)order.size() && (side || (int)s.count()==index))
                t=coreEdges-prefixEdges-degree;
            else t=edges(one(v)|s);
            distribute(v,side,b,0,b.size(),t,cap);
        }
        degree+=t;
    }
    mergeVertex(v,degree);
}

// Pool an independent block of incoming vertices. At a split of the
// old components, each row with one remaining edge contributes a bit.
// A detecting matrix resolves several such rows per group of queries.
struct RowCount { int v, total; };
struct Incidence { int root, side, count; };
vector<Incidence> incident[MAXV];
struct Target { Set s; int inside=0, cap=0; };
int crossCount(const Set& rows,const Target& target) {
    if (rows.none() || target.s.none()) return 0;
    return edges(rows|target.s)-target.inside;
}
vector<int> splitCounts(const vector<RowCount>& rows, const Target& left,int rightCap) {
    vector<int> result(rows.size());
    vector<int> binary;
    for (int i=0;i<(int)rows.size();++i) {
        int lo=max(0,rows[i].total-rightCap), hi=min(rows[i].total,left.cap);
        if (lo==hi) result[i]=lo;
        else if (lo==0 && hi==1) binary.push_back(i);
        else result[i]=crossCount(one(rows[i].v),left);
    }
    int pos=0, count=binary.size();
    for (int level=(int)codes.size()-1;level>=0;--level) {
        const Matrix& code=codes[level];
        while (count-pos>=code.cols) {
            vector<int> sums,bits;
            for (const auto& row : code.row) {
                Set q;
                for (int j=0;j<code.cols;++j)
                    if (row[j]) q.set(rows[binary[pos+j]].v);
                sums.push_back(crossCount(q,left));
            }
            if (!decode(level,sums,bits)) exit(0);
            for (int bit : bits) result[binary[pos++]]=bit;
        }
    }
    for (;pos<count;++pos) {
        int i=binary[pos];
        result[i]=crossCount(one(rows[i].v),left);
    }
    return result;
}
void batchSearch(const vector<RowCount>& rows,int lo,int hi,const Target& target) {
    if (rows.empty()) return;
    if (hi-lo==1) {
        int r=roots[lo];
        int side=node[r].side[0].count()<=node[r].side[1].count() ? 0 : 1;
        Target left{node[r].side[side],0,(int)node[r].side[side].count()};
        auto part=splitCounts(rows,left,node[r].side[1-side].count());
        for (int i=0;i<(int)rows.size();++i) {
            if (part[i]) incident[rows[i].v].push_back({r,side,part[i]});
            if (rows[i].total>part[i])
                incident[rows[i].v].push_back({r,1-side,rows[i].total-part[i]});
        }
        return;
    }
    // A component is one possible attachment location in a forest.
    // Near the leaves of this search, the vertex counts also give caps.
    int mid=(lo+hi)/2;
    Target left;
    for (int i=lo;i<mid;++i) {
        const Component& c=node[roots[i]];
        left.s|=c.all;left.inside+=c.edges;
        left.cap+=surplus?(int)c.all.count():1;
    }
    Target right{target.s^left.s,target.inside-left.inside,target.cap-left.cap};
    auto counts=splitCounts(rows,left,right.cap);
    vector<RowCount> a,b;
    for (int i=0;i<(int)rows.size();++i) {
        if (counts[i]) a.push_back({rows[i].v,counts[i]});
        if (rows[i].total>counts[i]) b.push_back({rows[i].v,rows[i].total-counts[i]});
    }
    batchSearch(a,lo,mid,left);
    batchSearch(b,mid,hi,right);
}
void rowTotals(vector<RowCount>& rows,const vector<int>& vs,int lo,int hi,
               int total,const Target& target) {
    if (!total) return;
    if (hi-lo==1) {rows.push_back({vs[lo],total});return;}
    int mid=(lo+hi)/2;
    Set a;
    for (int i=lo;i<mid;++i) a.set(vs[i]);
    int t=crossCount(a,target);
    rowTotals(rows,vs,lo,mid,t,target);
    rowTotals(rows,vs,mid,hi,total-t,target);
}
void insertBatch(int lo,int hi) {
    if (codes.empty()) makeCodes();
    Set block;
    vector<int> vs(order.begin()+lo,order.begin()+hi);
    for (int v : vs) block.set(v);
    if (!roots.empty()) {
        Target target;
        for (int r : roots) target.s|=node[r].all;
        target.inside=prefixEdges;
        target.cap=surplus?lo:(int)roots.size();
        int total=hi==(int)order.size()?coreEdges-prefixEdges:crossCount(block,target);
        vector<RowCount> rows;
        rowTotals(rows,vs,0,vs.size(),total,target);
        batchSearch(rows,0,roots.size(),target);
    }
    for (int v : vs) {
        memset(hits,0,sizeof hits);
        int degree=0;
        for (const Incidence& e : incident[v]) {
            int r=owner[e.root];
            int sample=node[e.root].side[e.side]._Find_first();
            hits[r][node[r].side[1][sample]]+=e.count;
            degree+=e.count;
        }
        mergeVertex(v,degree);
    }
}

pair<int,int> internalEdge(Set s) {
    while (s.count()>2) {
        Set l=firstK(s,s.count()/2), r=s^l;
        if (edges(l)) { s=l; continue; }
        if (edges(r)) { s=r; continue; }
        int t=edges(s);
        while (l.count()>1) {
            Set half=firstK(l,l.count()/2);
            int x=edges(half|r);
            if (x) { l=half; t=x; }
            else l^=half;
        }
        int u=l._Find_first();
        return {u,neighbor(u,r,t)};
    }
    int u=s._Find_first();
    return {u,(int)s._Find_next(u)};
}
void tryTriangle(const Set& a, const Set& bad) {
    auto [u,v]=internalEdge(bad);
    Set s=a, uv=one(u)|one(v);
    int t=edges(s|uv)-1;
    if (t<=(int)s.count()) {
        // Sample common neighbors even when the degree sum alone
        // does not force an intersection in the whole set.
        int du=edges(s|one(u)), dv=t-du;
        if (du>dv) {swap(du,dv);swap(u,v);}
        if (du<=3) {
            while (du>0) {
                int w=neighbor(u,s,du);
                if (edges(one(w)|one(v))) cycle({u,v,w});
                s.reset(w);--du;
            }
            return;
        }
        if (t*3<(int)s.count()) return;
        int tried=0;
        for (int w : elements(s)) {
            if (edges(uv|one(w))==3) cycle({u,v,w});
            if (++tried==12) break;
        }
        return;
    }
    while (s.count()>1) {
        Set l=firstK(s,s.count()/2);
        int x=edges(l|uv)-1;
        if (x>(int)l.count()) { s=l; t=x; }
        else { s^=l; t-=x; }
    }
    cycle({u,v,(int)s._Find_first()});
}

// In dense graphs the number of edges to a large reference side can
// estimate the number of opposite colors in a small sample. The two
// independence checks certify every decoded batch, even for irregular
// graphs where the estimate can be inaccurate.
int denseBatch(Set& a, Set& b, int pos, int limit, double density) {
    bool flip=b.count()>a.count();
    const Set& ref=flip?b:a;
    int anchors=ref.count();
    if (codes.empty()) makeCodes();
    int level=(int)codes.size()-1;
    while (level>=0) {
        const Matrix& code=codes[level];
        double noise=(1.0-density)*code.weight/max(0.001,density);
        if (code.cols<=limit-pos && code.cols>=11 &&
            anchors >= 1.3*code.weight + 18*noise) break;
        --level;
    }
    if (level<0) return 0;
    const Matrix& code=codes[level];
    vector<int> sums, bits;
    for (const auto& row : code.row) {
        Set s=ref;
        int h=0;
        for (int j=0;j<code.cols;++j) if (row[j]) {s.set(order[pos+j]);++h;}
        int e=edges(s), best=0;
        double distance=e;
        for (int t=1;t<=h;++t) {
            double d=abs(density*t*(anchors+h-t)-e);
            if (d<distance) {distance=d;best=t;}
        }
        sums.push_back(best);
    }
    if (!decode(level,sums,bits)) return -1;
    Set aa=a,bb=b;
    for (int j=0;j<code.cols;++j)
        ((bits[j]^flip)?bb:aa).set(order[pos+j]);
    if (edges(aa)) {tryTriangle(b,aa);return -1;}
    if (edges(bb)) {tryTriangle(a,bb);return -1;}
    a=aa;b=bb;
    return code.cols;
}

// In a tree a new vertex has at most one neighbor in a connected
// prefix. Independent pieces of an insertion order can therefore share
// their attachment queries. Checking the enlarged A side also makes
// this shortcut valid when the prefix has several components.
int sparseBatch(Set& a,Set& b,int pos,int limit) {
    if (codes.empty()) makeCodes();
    for (int level=(int)codes.size()-1;level>=1;--level) {
        const Matrix& code=codes[level];
        if (code.cols>limit-pos || code.cols*code.cols>3*(pos+30)) continue;
        Set block;
        for (int j=0;j<code.cols;++j) block.set(order[pos+j]);
        if (edges(block)) continue;
        vector<int> sums,bits;
        for (const auto& row : code.row) {
            Set s=a;
            for (int j=0;j<code.cols;++j) if (row[j]) s.set(order[pos+j]);
            sums.push_back(edges(s));
        }
        if (!decode(level,sums,bits)) return 0;
        Set aa=a,bb=b;
        for (int j=0;j<code.cols;++j)
            (bits[j]?bb:aa).set(order[pos+j]);
        if (edges(aa)) return 0;
        a=aa;b=bb;
        return code.cols;
    }
    return 0;
}

// Try extending the current coloring greedily. One side stays
// independent by construction. Certify the other side in batches.
int greedy(int start) {
    Set a,b;
    for (int r : roots) { a|=node[r].side[0]; b|=node[r].side[1]; }
    int end=start;
    int batch=start ? max(8,start/2) : 4;
    int size=order.size();
    int tryBatch=24;
    while (end<size) {
        if (end>=tryBatch && size-end>=11 && m*14>n*n && a.any() && b.any()) {
            double density=edges(a|b)/(double)(a.count()*b.count());
            if (density>=0.42 && density<=1.0) {
                int step=denseBatch(a,b,end,size,density);
                if (step>0) {end+=step;continue;}
                tryBatch=step<0 ? max(end+64,2*end) : end+16;
            } else tryBatch=2*end;
        }
        int next=min(size,end+batch);
        Set block;
        for (int i=end;i<next;++i) block.set(order[i]);
        if (batch>=4 && (a.none() || (int)a.count()*m < n*n/2)
                && !edges(a|block)) a|=block;
        else {
            for (int i=end; i<next;) {
                if (m==n-1 && next-i>=11) {
                    int step=sparseBatch(a,b,i,next);
                    if (step) {i+=step;continue;}
                }
                int stop=min(next,i+8);
                while (i<stop) {
                    int v=order[i++];
                    Set s=a|one(v);
                    if (edges(s)) b.set(v);
                    else a=s;
                }
            }
        }
        if (edges(b)) {
            if (m*16>n*n && a.any()) tryTriangle(a,b);
            return next;
        }
        end=next;
        batch=min(batch*2,64);
    }
    partition(a);
}

// A degree-one vertex can be colored after its unique remaining neighbor.
// Probe a short run first so that paths and graphs with few leaves skip
// the rest of the sweep.
void peelLeaves() {
    Set core=universe;
    while (core.count()>20) {
        int tested=0, removed=0;
        for (int v=n-1; v>=0 && core.count()>12; --v) if (core[v]) {
            core.reset(v);
            if (edges(core)==coreEdges-1) {
                leaves.push_back(v);
                --coreEdges;
                ++removed;
            } else core.set(v);
            ++tested;
            if ((tested==16 || tested==48 || tested==96) && removed*6<tested) break;
        }
        if (!removed || removed*6<tested) break;
    }
    order=elements(core);
}
void restoreBlock(Set& a, int lo, int hi) {
    if (lo==hi) return;
    if (hi-lo<=3) {
        for (int i=lo; i<hi; ++i) {
            int v=leaves[i];
            if (!edges(a|one(v))) a.set(v);
        }
        return;
    }
    Set block;
    for (int i=lo;i<hi;++i) block.set(leaves[i]);
    // Each leaf of an independent block has exactly one edge into the
    // restored core. A pooled query counts leaves touching its A side.
    if (edges(block)) {
        int mid=(lo+hi)/2;
        restoreBlock(a,lo,mid);
        restoreBlock(a,mid,hi);
        return;
    }
    int i=lo;
    for (int level=(int)codes.size()-1;level>=0;--level) {
        const auto& code=codes[level];
        while (hi-i>=code.cols) {
            vector<int> sums, bits;
            for (const auto& row : code.row) {
                Set s=a;
                for (int j=0;j<code.cols;++j) if (row[j]) s.set(leaves[i+j]);
                sums.push_back(edges(s));
            }
            if (!decode(level,sums,bits)) exit(0);
            for (int hit : bits) {
                if (!hit) a.set(leaves[i]);
                ++i;
            }
        }
    }
    for (;i<hi;++i) if (!edges(a|one(leaves[i]))) a.set(leaves[i]);
}
void restoreLeaves(Set& a) {
    if (leaves.empty()) return;
    if (codes.empty()) makeCodes();
    reverse(leaves.begin(),leaves.end());
    restoreBlock(a,0,leaves.size());
}

// On very small instances enumerate the possible connected graphs.
// A common certificate exists if their union is bipartite or their
// intersection contains an odd cycle.
void smallGraph() {
    int pairs=0, bit[6][6]={};
    for (int i=0;i<n;++i) for (int j=0;j<i;++j)
        bit[i][j]=bit[j][i]=1<<pairs++;
    int masks=1<<pairs, full=masks-1, all=(1<<n)-1;
    vector<int> induced(1<<n), bip(masks,-1);
    for (int s=0;s<=all;++s)
        for (int i=0;i<n;++i) if (s>>i&1)
            for (int j=0;j<i;++j) if (s>>j&1) induced[s]|=bit[i][j];
    vector<bool> disconnected(masks);
    for (int side=1;side<all;++side) {
        int internal=induced[side]|induced[all^side];
        int cross=full^internal;
        for (int g=cross;;g=(g-1)&cross) {
            bip[g]=side;
            if (!g) break;
        }
        for (int g=internal;;g=(g-1)&internal) {
            disconnected[g]=true;
            if (!g) break;
        }
    }
    vector<int> possible;
    for (int g=0;g<masks;++g) if (!disconnected[g]) possible.push_back(g);
    for (;;) {
        int common=full, combined=0;
        for (int g : possible) {common&=g; combined|=g;}
        if (bip[combined]>=0) partition(Set(bip[combined]));
        if (bip[common]<0) {
            for (int l=3;l<=n;l+=2) {
                vector<int> a(n);
                iota(a.begin(),a.end(),0);
                do {
                    bool ok=true;
                    for (int i=0;i<l;++i) if (!(common&bit[a[i]][a[(i+1)%l]])) ok=false;
                    if (ok) {a.resize(l);cycle(a);}
                } while (next_permutation(a.begin(),a.end()));
            }
        }
        double best=1e99;
        int choice=0;
        for (int s=1;s<=all;++s) if (induced[s] && (induced[s]&(combined^common))) {
            int cnt[16]={}, meet[16], join[16]={};
            fill(begin(meet),end(meet),full);
            for (int g : possible) {
                int k=__builtin_popcount(g&induced[s]);
                ++cnt[k]; meet[k]&=g; join[k]|=g;
            }
            double cost=0;
            for (int k=0;k<=pairs;++k) if (cnt[k] && bip[meet[k]]>=0 && bip[join[k]]<0)
                cost+=cnt[k]*(1.0+0.48*log2(cnt[k]));
            if (cost<best) {best=cost;choice=s;}
        }
        int t=edges(Set(choice));
        erase_if(possible,[&](int g){return __builtin_popcount(g&induced[choice])!=t;});
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin>>n) || n<1 || n>MAXV) return 0;
    universe=interval(0,n);
    if (n<=2) partition(one(0));
    if (n<=6) smallGraph();
    Set odds;
    for (int i=0;i<n;i+=2) odds.set(i);
    verify(odds);
    m=edges(universe);
    if (m==n*(n-1)/2) cycle({0,1,2});
    if (m>n*n/4 && edges(interval(0,3))==3) cycle({0,1,2});
    tryOrderedCycle(odds);
    for (int delta : {1,-1}) {
        int a=smallRoot(n,m-delta);
        if (a>0 && n>=16) trySingleDefect(delta,a,odds);
    }
    if (m<=n*n/4) {
        int a=smallRoot(n,m);
        int oddRoot=smallRoot(odds.count(),edges(odds));
        if (a>0 && oddRoot>=0 && oddRoot<=a) {
            verify(interval(0,a));
            verify(interval(n-a,n));
            tryComplete(a,odds);
        }
        if (has_single_bit((unsigned)n) && 2*m==n*(int)bit_width((unsigned)n-1)) {
            Set side;
            for (int i=0;i<n;++i) if (popcount((unsigned)i)%2) side.set(i);
            verify(side);
        }
        for (int h=2; h*h<=n; ++h) if (n%h==0 && m==2*n-h-n/h) {
            for (int w : {h,n/h}) {
                Set board;
                for (int i=0; i<n; ++i) if ((i/w+i%w)%2) board.set(i);
                verify(board);
            }
        }
        int mid=n/2;
        Set left=interval(0,mid);
        bool l=!edges(left), r=!edges(universe^left);
        if (l && r) partition(left);
        if (l || r) {
            int lo=l?mid:1, hi=l?n:mid;
            while (hi-lo>1) {
                int k=(lo+hi)/2;
                if (!edges(interval(0,k))) lo=k;
                else hi=k;
            }
            verify(interval(0,lo));
        }
    }
    node.resize(n);
    surplus=m-(n-1);
    order=elements(universe);
    coreEdges=m;
    int retry=greedy(0);
    if (n>=50 && m*5<=n*7) {
        peelLeaves();
        if (!leaves.empty()) retry=greedy(0);
    }
    int size=order.size();
    for (int i=0; i<size;) {
        int batch=1;
        if (size>=64 && coreEdges<2*size && (i==0 || roots.size()>=4)) {
            batch=min(64,size-i);
            while (batch>=4) {
                Set block;
                for (int j=0;j<batch;++j) block.set(order[i+j]);
                if (!edges(block)) break;
                batch/=2;
            }
        }
        if (batch>=4) insertBatch(i,i+batch);
        else {insertVertex(i);batch=1;}
        i+=batch;
        if (i>=retry && i<size && roots.size()==1 && coreEdges<=size*size/4) {
            int fail=greedy(i);
            retry=max(fail,2*i);
        }
    }
    partition(node[roots[0]].side[0]);
}
