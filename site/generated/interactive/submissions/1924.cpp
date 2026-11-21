#include <bits/stdc++.h>
using namespace std;

using Real = long double;

int n, groups;
Real binom[1025][1025], fac[1025];

int ask(int l, int r, int k) {
    cout << "? " << l << ' ' << r << ' ' << k << endl;
    int v;
    if (!(cin >> v) || v < 1) exit(0);
    return v;
}

// For k=2, mark the earlier member of each pair L and the later R.
// The other positions are singletons: 1, and also n if n is even.
struct State {
    int L, R, s, h; // h = unmatched L's before this interval
    int size() const { return L + R + s; }
};

struct Row {
    int obs[3]; // L count, R count, presence of singleton n in left part
    bool left;
    State child;
    Real weight;
};

// Number of local arrangements, counting choices of partners for R's.
// With j internal pairs, R-j partners come from the h open pairs.
Real ways(State z) {
    Real sum = 0;
    for (int j = max(0, z.R - z.h); j <= min(z.L, z.R); ++j)
        sum += ldexpl(binom[z.L][j] * binom[z.h][z.R-j], -j);
    return (fac[z.size()] / fac[z.s] / fac[z.L]) * sum;
}

vector<Row> rows(State z, int t) {
    vector<Row> v;
    for (int s = 0; s <= z.s; ++s) {
        for (int a = max(0, t-s-z.R); a <= min(z.L, t-s); ++a) {
            int b = t-s-a, mid = z.h+a-b;
            if (mid < 0 || z.s-s > z.size()-t) continue;
            State u{a,b,s,z.h}, w{z.L-a,z.R-b,z.s-s,mid};
            Real prob = ways(u)*ways(w);
            if (s) {
                v.push_back({{a,b,int(z.s==2 && s==2)},true,u,
                             prob * s / z.s});
            }
            if (s != z.s) {
                v.push_back({{a,b,int(z.s==2 && s==1)},false,w,
                             prob * (z.s-s) / z.s});
            }
        }
    }
    return v;
}

// Minimum weighted cost of resolving the cut, skipping all observations
// already forced by earlier responses or the interval counts.
pair<Real,int> tests(const vector<Row>& v, const vector<int>& ids, int mask) {
    if (ids.size() <= 1) return {0,-1};
    Real mass = 0;
    for (int i : ids) mass += v[i].weight;
    Real best = numeric_limits<Real>::infinity();
    int action = -1;
    for (int k = 0; k < 3; ++k) if (mask >> k & 1) {
        bool differs = false;
        for (int i : ids)
            if (v[i].obs[k] != v[ids[0]].obs[k]) { differs = true; break; }
        if (!differs) continue;
        map<int,vector<int>> buckets;
        for (int i : ids) buckets[v[i].obs[k]].push_back(i);
        Real cost = mass;
        for (auto& [key, subset] : buckets)
            cost += tests(v,subset,mask^(1<<k)).first;
        if (cost < best) best = cost, action = k;
    }
    return {best,action};
}

using Key = array<int,4>;
map<Key,pair<Real,int>> memo;
constexpr int EXACT = 14;

pair<Real,int> plan(State z) {
    int m = z.size();
    if (m == 1) return {0,0};
    Key key{z.L,z.R,z.s,z.h};
    if (auto it = memo.find(key); it != memo.end()) return it->second;

    Real best = numeric_limits<Real>::infinity();
    int cut = 1;
    int lo = m <= EXACT ? 1 : m/2;
    int hi = m <= EXACT ? m-1 : (m+1)/2;
    for (int t = lo; t <= hi; ++t) {
        auto v = rows(z,t);
        vector<int> ids(v.size());
        iota(ids.begin(),ids.end(),0);
        Real cost = tests(v,ids,7).first, mass = 0;
        for (const auto& r : v) {
            State c = r.child;
            Real future = c.size() <= EXACT ? plan(c).first :
                (1 + int(c.L>0 && c.R>0))*log2l(c.size()) + (c.s==2);
            cost += r.weight * future;
            mass += r.weight;
        }
        cost /= mass;
        if (cost < best) best = cost, cut = t;
    }
    return memo[key] = {best,cut};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n < 3 || n > 1024) return 0;
    fac[0] = 1;
    for (int i = 0; i <= n; ++i) {
        binom[i][0] = binom[i][i] = 1;
        for (int j = 1; j < i; ++j)
            binom[i][j] = binom[i-1][j-1]+binom[i-1][j];
        if (i) fac[i] = fac[i-1]*i;
    }

    groups = n/2+1;
    int pairs = n-groups;
    State z{pairs,pairs,n-2*pairs,0};
    int start = 0, preL = 0, preR = 0;
    while (z.size() > 1) {
        int t = plan(z).second, x = start+t;
        auto v = rows(z,t);
        vector<int> ids(v.size());
        iota(ids.begin(),ids.end(),0);
        int mask = 7;
        while (ids.size() > 1) {
            int k = tests(v,ids,mask).second;
            if (k < 0) return 0;
            int ans;
            if (k == 0) {
                // Exactly the L positions repeat a quotient to their right.
                ans = x - groups + ask(x+1,n,2) - preL;
            } else if (k == 1) {
                // Exactly the R positions repeat a quotient to their left.
                ans = x - ask(1,x,2) - preR;
            } else {
                // Only used while both singletons are in the interval.
                ans = x >= 2 ? int(ask(1,x,n)==2) : int(ask(x+1,n,n)==1);
            }
            mask ^= 1<<k;
            ids.erase(remove_if(ids.begin(),ids.end(),[&](int i) {
                return v[i].obs[k] != ans;
            }),ids.end());
            if (ids.empty()) return 0;
        }
        const Row& r = v[ids[0]];
        if (!r.left) {
            start = x;
            preL += r.obs[0];
            preR += r.obs[1];
        }
        z = r.child;
    }
    cout << "! " << start+1 << endl;
}
