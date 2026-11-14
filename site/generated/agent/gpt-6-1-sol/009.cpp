#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
constexpr int LIM = 1000000000;
constexpr int64 INF = (1LL << 60);

struct Event {
    int value, id, kind; // 0: ordinary, 1: selected-only, 2: excluded-only
};

// Supports subtracting a weight from a prefix, removing a constraint,
// and finding the smallest remaining constraint.
struct SegTree {
    int n;
    vector<int64> mn, lazy;
    SegTree(const vector<int64>& a) : n((int)a.size()), mn(4*n), lazy(4*n) {
        build(1, 0, n, a);
    }
    void build(int p, int l, int r, const vector<int64>& a) {
        if (r-l == 1) { mn[p] = a[l]; return; }
        int mid = (l+r)/2;
        build(p*2,l,mid,a); build(p*2+1,mid,r,a);
        mn[p] = min(mn[p*2],mn[p*2+1]);
    }
    void apply(int p, int64 v) { mn[p] += v; lazy[p] += v; }
    void push(int p) {
        if (lazy[p]) {
            apply(p*2,lazy[p]); apply(p*2+1,lazy[p]); lazy[p]=0;
        }
    }
    void addPrefix(int p, int l, int r, int end, int64 v) {
        if (r <= end) { apply(p,v); return; }
        push(p);
        int mid=(l+r)/2;
        addPrefix(p*2,l,mid,end,v);
        if (end > mid) addPrefix(p*2+1,mid,r,end,v);
        mn[p]=min(mn[p*2],mn[p*2+1]);
    }
    void erase(int p, int l, int r, int pos) {
        if (r-l == 1) { mn[p]=INF; lazy[p]=0; return; }
        push(p);
        int mid=(l+r)/2;
        if (pos < mid) erase(p*2,l,mid,pos);
        else erase(p*2+1,mid,r,pos);
        mn[p]=min(mn[p*2],mn[p*2+1]);
    }
    void addPrefix(int end, int64 v) {
        if (end) addPrefix(1,0,n,end,v);
    }
    void erase(int pos) { erase(1,0,n,pos); }
    int64 minimum() const { return mn[1]; }
};

void solve() {
    int n, W;
    cin >> n >> W;
    vector<int> w(n), r(n), fixedItems, order(n,-1);
    for (int& x : w) cin >> x;
    for (int& x : r) cin >> x;
    vector<Event> events;
    events.reserve(2*n);
    int firstUnknown = n;
    for (int i=0;i<n;i++) {
        if (w[i]) fixedItems.push_back(i);
        else firstUnknown=min(firstUnknown,i);
        if (r[i]) events.push_back({r[i],i,0});
        else {
            events.push_back({LIM,i,1});
            events.push_back({1,i,2});
        }
    }
    sort(fixedItems.begin(),fixedItems.end(),[&](int a,int b) {
        return w[a]!=w[b] ? w[a]<w[b] : a<b;
    });
    int m=fixedItems.size();
    vector<int64> sum(m+1);
    for (int j=0;j<m;j++) {
        order[fixedItems[j]]=j;
        sum[j+1]=sum[j]+w[fixedItems[j]];
    }
    sort(events.begin(),events.end(),[](const Event& a,const Event& b) {
        return a.value!=b.value ? a.value>b.value : a.id<b.id;
    });
    int E=events.size();
    vector<int> selectRank(n,-1), excludeRank(n,-1);
    for (int j=0;j<E;j++) {
        const auto& e=events[j];
        if (w[e.id]) {
            if (e.kind!=2) selectRank[e.id]=j;
            if (e.kind!=1) excludeRank[e.id]=j;
        }
    }
    int maxK=int(upper_bound(sum.begin(),sum.end(),(int64)W)-sum.begin())-1;

    // For a boundary fixed item p, a selected unknown weight may be at
    // most w[p] - (index > p). Unselected unknown weights are LIM.
    // At W=LIM, weights LIM-1 suffice for selected unknown items.
    auto capacity = [&](int id,int k) -> int {
        int u=(k==m ? LIM : w[fixedItems[k]]-(id>fixedItems[k]));
        if (W==LIM) u=min(u,LIM-1);
        return u;
    };

    // With B=W-F weight available to unknown items, filling their
    // capacities in Bob order gives prefix weight min(B,U).
    // An excluded fixed item needs P+min(B,U)>W-w, equivalent to:
    //   P+U>W-w  and  F-P<w.
    // The first group of inequalities is monotone in k.
    auto enoughCapacity = [&](int k) -> bool {
        int64 P=0,U=0;
        for (const auto& e:events) {
            int id=e.id;
            if (!w[id]) {
                U+=capacity(id,k);
                // An excluded unknown item of weight W must also be
                // preceded by a selection; including its own capacity
                // allows it to be selected instead.
                if (W==LIM && P+U==0) return false;
            } else if (order[id]<k) {
                if (e.kind!=2) P+=w[id];
            } else if (e.kind!=1) {
                if (P+U <= (int64)W-w[id]) return false;
            }
        }
        return k==m || sum[k]+U > (int64)W-w[fixedItems[k]];
    };

    if (!enoughCapacity(maxK)) { cout << "No\n"; return; }
    int lo=0,hi=maxK;
    while (lo<hi) {
        int mid=(lo+hi)/2;
        if (enoughCapacity(mid)) hi=mid;
        else lo=mid+1;
    }
    int k=lo;

    // Maintain the second group, w-(F-P)>0, for excluded items.
    // Adding a fixed selected item subtracts its weight from all
    // constraints preceding it in Bob's order.
    vector<int64> initial(E,INF);
    int64 P=0;
    for (int j=0;j<E;j++) {
        const auto& e=events[j];
        int id=e.id;
        if (!w[id]) {
            if (W==LIM) initial[j]=(int64)LIM-sum[k]+P;
        } else if (order[id]<k) {
            if (e.kind!=2) P+=w[id];
        } else if (e.kind!=1) {
            initial[j]=(int64)w[id]-sum[k]+P;
        }
    }
    SegTree seg(initial);
    while (seg.minimum()<=0 && k<maxK) {
        int id=fixedItems[k++];
        seg.erase(excludeRank[id]);
        seg.addPrefix(selectRank[id],-w[id]);
    }
    if (seg.minimum()<=0) { cout << "No\n"; return; }

    // A selected fixed item of weight LIM is necessarily the only
    // selected item. Unknown weights cannot be placed after it if
    // they have smaller original indices.
    if (k && w[fixedItems[k-1]]==LIM && firstUnknown<fixedItems[k-1]) {
        cout << "No\n";
        return;
    }

    vector<int> answerW=w, answerR=r;
    int64 budget=(int64)W-sum[k];
    for (const auto& e:events) {
        int id=e.id;
        if (!w[id]) {
            int take=(int)min(budget,(int64)capacity(id,k));
            answerW[id]=take ? take : LIM;
            budget-=take;
        }
    }
    for (int i=0;i<n;i++) {
        if (!r[i]) answerR[i]=(order[i]<k ? LIM : 1);
    }
    cout << "Yes\n";
    for (int i=0;i<n;i++) cout << answerW[i] << (i+1==n?'\n':' ');
    for (int i=0;i<n;i++) cout << answerR[i] << (i+1==n?'\n':' ');
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    cin >> T;
    while (T--) solve();
}
