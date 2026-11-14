#include <bits/stdc++.h>
using namespace std;
using i64 = long long;

/*
Reverse time: a monster has deadline d=T-S. For any subset of monsters,
the minimum number of unprocessed hit points is
    g(x) = max(0, max_d(x * sum_{deadline<=d} H - d)).
The unit attacks form a scheduling matroid. Processing strengths in descending
order therefore gives the minimum penalty as sum (P_j-P_{j+1}) * g_j(x),
where g_j uses the first j monsters in that order.

Store these prefix-maximum functions in a persistent segment tree over deadlines.
For adjacent segments A,B, its function is
    max(g_A(x), x*H_A-D_A+g_B(x)).
All slopes in the second expression are at least all slopes in the first,
so one integer cutoff describes which child wins. Point additions can only
move this cutoff to the left. Queries also return an interval on which their
chosen line is valid, allowing the cutoff search to skip whole intervals.

Sweep x=1..L. Each persistent node changes its chosen child at most once.
Route the coefficients of all roots through the currently chosen children;
a change reroutes its coefficient along two paths, taking O(log N) time.
Maintain the resulting penalty as slope*x-timeCost, and answer the sorted
thresholds in the same sweep.

Time: O(N log^2(N) log(L) + L + Q). Space: O(N log(N) + L + Q).
*/

struct Monster { i64 d, h, p; int pos; };
struct Node {
    i64 h, d, weight;
    int left, right, cut, next;
};
struct Line {
    i64 h, d;
    int low, high;
};

class Solver {
public:
    int L;
    vector<Node> tree;
    vector<i64> deadlines;
    i64 slope = 0, timeCost = 0;

    explicit Solver(int limit) : L(limit) {}

    int build(int l, int r) {
        Node v{0, deadlines[r] - (l ? deadlines[l-1] : 0), 0, 0, 0, L+1, 0};
        if (l != r) {
            int m = (l+r)/2;
            v.left = build(l,m);
            v.right = build(m+1,r);
        }
        int id = (int)tree.size();
        tree.push_back(v);
        return id;
    }

    // The selected prefix gives h*x-d throughout [low,high].
    Line getLine(int id, int x) const {
        Line z{0,0,1,L};
        while (tree[id].h != 0) {
            const Node &v = tree[id];
            if (x < v.cut) {
                z.high = min(z.high, v.cut-1);
                if (!v.left) break;
                id = v.left;
            } else {
                z.low = max(z.low, v.cut);
                if (!v.left) {
                    z.h += v.h;
                    z.d += v.d;
                    break;
                }
                const Node &a = tree[v.left];
                z.h += a.h;
                z.d += a.d;
                id = v.right;
            }
        }
        return z;
    }

    int findCut(int a, int b, int oldCut) const {
        // An empty right part never improves on the left part.
        if (!tree[b].h) return L+1;
        int low = 1, high = oldCut;
        if (high == 1) return 1;
        int x = high-1;
        while (low < high) {
            Line u = getLine(a,x), v = getLine(b,x);
            int lo = max(low, max(u.low,v.low));
            int hi = min(high-1, min(u.high,v.high));
            i64 A = tree[a].h + v.h - u.h;
            i64 B = tree[a].d + v.d - u.d;
            i64 crossing;
            if (!A) crossing = B ? (i64)L+1 : 1;
            else crossing = max<i64>(1, B/A + (B%A != 0));
            // At the interval's first integer, an earlier piece might
            // already be positive; only an interior crossing settles it.
            if (crossing > lo && crossing <= hi) return (int)crossing;
            if (crossing <= lo) high = lo;
            else low = hi+1;
            if (low == high) break;
            x = low + (high-low)/2;
        }
        return low;
    }

    int update(int old, int l, int r, int pos, i64 h) {
        Node v = tree[old];
        v.h += h;
        if (l == r) {
            i64 c = v.d / v.h + (v.d % v.h != 0);
            v.cut = (int)min<i64>(L+1, max<i64>(1,c));
        } else {
            int m = (l+r)/2;
            if (pos <= m) v.left = update(v.left,l,m,pos,h);
            else v.right = update(v.right,m+1,r,pos,h);
            v.cut = findCut(v.left,v.right,tree[old].cut);
        }
        int id = (int)tree.size();
        tree.push_back(v);
        return id;
    }

    // Send a coefficient along the prefix currently attaining the maximum.
    void send(int id, i64 w, int x) {
        while (true) {
            Node &v = tree[id];
            v.weight += w;
            if (x < v.cut) {
                if (!v.left) break;
                id = v.left;
            } else {
                if (!v.left) {
                    slope += w*v.h;
                    timeCost += w*v.d;
                    break;
                }
                const Node &a = tree[v.left];
                slope += w*a.h;
                timeCost += w*a.d;
                id = v.right;
            }
        }
    }

    void change(int id, int x) {
        Node &v = tree[id];
        i64 w = v.weight;
        if (!w) return;
        if (!v.left) {
            slope += w*v.h;
            timeCost += w*v.d;
        } else {
            send(v.left,-w,x);
            slope += w*tree[v.left].h;
            timeCost += w*tree[v.left].d;
            send(v.right,w,x);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int N,L;
    i64 T;
    cin >> N >> L >> T;
    vector<Monster> monsters(N);
    Solver solver(L);
    for (auto &v : monsters) {
        i64 s;
        cin >> s >> v.h >> v.p;
        v.d = T-s;
        solver.deadlines.push_back(v.d);
    }
    auto &ds = solver.deadlines;
    sort(ds.begin(),ds.end());
    ds.erase(unique(ds.begin(),ds.end()),ds.end());
    int K = (int)ds.size();
    for (auto &v : monsters) v.pos = lower_bound(ds.begin(),ds.end(),v.d)-ds.begin();
    sort(monsters.begin(),monsters.end(),[](const Monster &a,const Monster &b){return a.p>b.p;});
    int depth = 0;
    while ((1<<depth) < K) ++depth;
    solver.tree.reserve(2*K + (size_t)N*(depth+1)+1);
    // Index zero is the null child.
    solver.tree.push_back({});
    int root = solver.build(0,K-1);
    vector<pair<int,i64>> roots;
    for (int i=0;i<N;++i) {
        root = solver.update(root,0,K-1,monsters[i].pos,monsters[i].h);
        i64 nextP = i+1<N ? monsters[i+1].p : 0;
        if (monsters[i].p != nextP) roots.emplace_back(root,monsters[i].p-nextP);
    }
    int Q;
    cin >> Q;
    vector<i64> queries(Q);
    for (auto &m : queries) cin >> m;

    vector<int> head(L+2,0);
    // Every child has a smaller ID, so simultaneous events run bottom-up.
    for (int i=(int)solver.tree.size()-1;i>0;--i) {
        int c = solver.tree[i].cut;
        if (c>=2 && c<=L) {
            solver.tree[i].next = head[c];
            head[c] = i;
        }
    }
    for (auto [id,w] : roots) solver.send(id,w,1);
    string output;
    output.reserve((size_t)Q*9);
    int q=0;
    for (int x=1;x<=L && q<Q;++x) {
        if (x>1) {
            for (int id=head[x];id;id=solver.tree[id].next) solver.change(id,x);
        }
        i64 penalty = solver.slope*x-solver.timeCost;
        while (q<Q && queries[q]<penalty) {
            output += to_string(x-1);
            output += '\n';
            ++q;
        }
    }
    while (q<Q) {
        output += to_string(L);
        output += '\n';
        ++q;
    }
    cout << output;
}
