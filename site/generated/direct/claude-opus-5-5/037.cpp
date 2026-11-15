#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;

static inline ull rnd(ull x){
    x += 0x9E3779B97F4A7C15ULL + 12345;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

struct Node { int l, r; ull h; };
static vector<Node> pool;
static int poolCnt;
static int K;

static inline ull getH(int node, int lo, int hi){
    if(!node) return 0;
    if(lo == hi) return pool[node].h * rnd(lo);
    return pool[node].h;
}

static int insertNode(int node, int lo, int hi, int pos){
    int id = poolCnt++;
    if(lo == hi){
        pool[id].l = pool[id].r = 0;
        pool[id].h = (node ? pool[node].h : 0) + 1;
        return id;
    }
    int mid = (lo + hi) >> 1;
    int L = node ? pool[node].l : 0;
    int R = node ? pool[node].r : 0;
    if(pos <= mid) L = insertNode(L, lo, mid, pos);
    else R = insertNode(R, mid + 1, hi, pos);
    pool[id].l = L; pool[id].r = R;
    pool[id].h = getH(L, lo, mid) + getH(R, mid + 1, hi);
    return id;
}

// >0 if (a + ia) better than (b + ib)
static int cmpSets(int a, int ia, int b, int ib){
    int lo = 0, hi = K - 1;
    ull ha = getH(a, lo, hi) + (ia >= 0 ? rnd(ia) : 0);
    ull hb = getH(b, lo, hi) + (ib >= 0 ? rnd(ib) : 0);
    if(ha == hb) return 0;
    while(lo < hi){
        int mid = (lo + hi) >> 1;
        int ar = a ? pool[a].r : 0, br = b ? pool[b].r : 0;
        ull xa = getH(ar, mid + 1, hi) + ((ia >= mid + 1 && ia <= hi) ? rnd(ia) : 0);
        ull xb = getH(br, mid + 1, hi) + ((ib >= mid + 1 && ib <= hi) ? rnd(ib) : 0);
        if(xa != xb){
            a = ar; b = br; lo = mid + 1;
        } else {
            a = a ? pool[a].l : 0;
            b = b ? pool[b].l : 0;
            hi = mid;
        }
    }
    ll ca = (a ? (ll)pool[a].h : 0) + (ia == lo ? 1 : 0);
    ll cb = (b ? (ll)pool[b].h : 0) + (ib == lo ? 1 : 0);
    if(ca > cb) return 1;
    if(ca < cb) return -1;
    return 0;
}

int main(){
    int t;
    scanf("%d", &t);
    while(t--){
        int n, m;
        scanf("%d %d", &n, &m);
        vector<pair<ll,ll>> v;
        v.reserve(n + m);
        for(int i = 0; i < n + m; i++){
            ll l, r;
            scanf("%lld %lld", &l, &r);
            v.push_back({l, r});
        }
        sort(v.begin(), v.end(), [](const pair<ll,ll>& a, const pair<ll,ll>& b){
            if(a.first != b.first) return a.first < b.first;
            return a.second > b.second;
        });
        vector<ll> L, R;
        ll maxR = 0;
        for(auto &p : v){
            if(p.second <= maxR) continue;
            L.push_back(p.first);
            R.push_back(p.second);
            maxR = p.second;
        }
        int k = L.size();
        // 1-indexed vertices
        vector<ll> w(k + 2, 0), mm(k + 2, 0);
        for(int i = 1; i < k; i++){
            ll o = R[i - 1] - L[i] + 1; // between vertex i and i+1 (0-indexed i-1, i)
            w[i] = o > 0 ? o : 0;
        }
        for(int i = 1; i <= k; i++){
            mm[i] = (R[i - 1] - L[i - 1] + 1) - w[i - 1] - w[i];
        }
        auto lenOf = [&](int i, int p, int q) -> ll {
            return mm[i] + (p == 1 ? w[i - 1] : 0) + (q == 0 ? w[i] : 0);
        };
        vector<ll> vals;
        vals.reserve(4 * k);
        for(int i = 1; i <= k; i++)
            for(int p = 0; p < 2; p++)
                for(int q = 0; q < 2; q++){
                    ll x = lenOf(i, p, q);
                    if(x > 0) vals.push_back(x);
                }
        sort(vals.begin(), vals.end());
        vals.erase(unique(vals.begin(), vals.end()), vals.end());
        K = max<int>(1, vals.size());
        int depth = 1;
        while((1 << (depth - 1)) < K) depth++;
        size_t need = (size_t)2 * k * (depth + 1) + 5;
        if(pool.size() < need) pool.resize(need);
        poolCnt = 1;
        pool[0].l = pool[0].r = 0; pool[0].h = 0;

        vector<array<char,2>> par(k + 1);
        int root[2] = {0, 0};
        for(int i = 1; i <= k; i++){
            int nr[2];
            for(int q = 0; q < 2; q++){
                ll l0 = lenOf(i, 0, q), l1 = lenOf(i, 1, q);
                int i0 = l0 > 0 ? (int)(lower_bound(vals.begin(), vals.end(), l0) - vals.begin()) : -1;
                int i1 = l1 > 0 ? (int)(lower_bound(vals.begin(), vals.end(), l1) - vals.begin()) : -1;
                int p = (cmpSets(root[0], i0, root[1], i1) >= 0) ? 0 : 1;
                par[i][q] = p;
                int idx = p == 0 ? i0 : i1;
                nr[q] = idx >= 0 ? insertNode(root[p], 0, K - 1, idx) : root[p];
            }
            root[0] = nr[0]; root[1] = nr[1];
        }
        int q = (cmpSets(root[0], -1, root[1], -1) >= 0) ? 0 : 1;
        vector<ll> res;
        for(int i = k; i >= 1; i--){
            int p = par[i][q];
            ll x = lenOf(i, p, q);
            if(x > 0) res.push_back(x);
            q = p;
        }
        sort(res.begin(), res.end(), greater<ll>());
        printf("%d\n", (int)res.size());
        string out;
        for(size_t i = 0; i < res.size(); i++){
            if(i) out += ' ';
            out += to_string(res[i]);
        }
        puts(out.c_str());
    }
    return 0;
}
