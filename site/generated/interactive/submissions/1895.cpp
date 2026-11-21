#include <bits/stdc++.h>
using namespace std;

int n, A, L, H;
int cacheD[111][111], dep[111], lowT[111], highT[111];
const int INF = 1000000000;

int distq(int u, int v) {
    if (u == v) return 0;
    int &z = cacheD[u][v];
    if (z) return z;
    cout << "? " << u << ' ' << v << endl;
    if (!(cin >> z) || z <= 0) exit(0);
    cacheD[v][u] = z;
    return z;
}

void finish(int v) {
    cout << "! " << v << endl;
    exit(0);
}

// The coordinate of i is where its path meets the spine 0--A.
void resolve(int i) {
    lowT[i] = highT[i] = (L + dep[i] - distq(A, i)) / 2;
}

pair<int,int> mBounds() {
    int a = 0, b = 0;
    for (int i = 1; i < n; ++i) {
        a = max(a, dep[i] - 2 * highT[i]);
        b = max(b, dep[i] - 2 * lowT[i]);
    }
    return {a,b};
}

int rad(int t, int m) {
    return max(L - t, m + t);
}

int minRad(int i, int m) {
    int t = clamp((L-m)/2, lowT[i], highT[i]);
    return min(rad(t,m), rad(min(t+1,highT[i]),m));
}

void refineM(int mhi) {
    for (int i = 1; i < n; ++i)
        if (lowT[i] < highT[i] && dep[i]-2*lowT[i] == mhi) {
            resolve(i);
            return;
        }
}

int radius() {
    while (true) {
        auto [a,b] = mBounds();
        int lb = INF, ub = INF, optimisticUB = INF, who = -1;
        for (int i = 1; i < n; ++i) if (i != A) {
            int v = minRad(i,a);
            if (v < lb || (v == lb && dep[i] > dep[who])) {
                lb = v;
                who = i;
            }
            ub = min(ub, max(L-lowT[i], b+highT[i]));
            optimisticUB = min(optimisticUB, max(L-lowT[i], a+highT[i]));
        }
        if (lb == ub) return lb;
        if (a < b && (optimisticUB < ub || lowT[who] == highT[who]))
            refineM(b);
        else
            resolve(who);
    }
}

// Only off-spine towns that could share a component are supplied.
// Equal-sized monochromatic blocks cancel on a mismatch. Any strict
// majority survives these cancellations and owns the largest leftover.
bool heavy(int x, const vector<int>& towns) {
    if ((int)towns.size() <= H) return false;
    auto same = [&](int u, int v) {
        return distq(u,v) < dep[u] + dep[v] - 2*x;
    };
    map<int, vector<int>> bins;
    bins[1] = towns;
    vector<array<int,3>> pairs;
    vector<pair<int,int>> leftovers;
    int cap = towns.size();
    for (auto it = bins.begin(); it != bins.end(); ++it) {
        int w = it->first;
        auto &v = it->second;
        while (v.size() >= 2) {
            int u = v.back(); v.pop_back();
            int z = v.back(); v.pop_back();
            if (same(u,z)) {
                if (2*w > H) return true;
                bins[2*w].push_back(u);
            } else {
                pairs.push_back({w,u,z});
                cap -= w;
                if (cap <= H) return false;
            }
        }
        if (!v.empty()) leftovers.emplace_back(w,v.back());
    }
    if (leftovers.empty()) return false;
    auto [cnt,c] = leftovers.back();
    leftovers.pop_back();
    // For each cancelled pair at most one of its blocks can match c.
    vector<array<int,3>> tests = pairs;
    for (auto [w,u] : leftovers) tests.push_back({w,u,-1});
    sort(tests.rbegin(), tests.rend());
    for (auto [w,u,v] : tests) {
        if (same(c,u) || (v >= 0 && same(c,v))) cnt += w;
        else cap -= w;
        if (cnt > H) return true;
        if (cap <= H) return false;
    }
    return cnt > H;
}

// Inspect a coordinate whose radius, if present, is known to be R.
bool balanced(int x) {
    while (true) {
        int left = 0, right = 0, ml = 0, mr = 0, deep = 0;
        bool exists = false;
        vector<int> group, uncertain;
        for (int i = 0; i < n; ++i) {
            left += highT[i] < x;
            right += lowT[i] > x;
            ml += lowT[i] < x;
            mr += highT[i] > x;
            if (lowT[i] <= x && x <= highT[i]) {
                bool fixed = lowT[i] == highT[i];
                exists |= fixed;
                if (!fixed) uncertain.push_back(i);
                if (dep[i] >= x+2) {
                    ++deep;
                    if (fixed) group.push_back(i);
                }
            }
        }
        if (left > H || right > H || (!exists && uncertain.empty())) return false;
        int best = 0, who = -1;
        for (int i : uncertain) {
            int score = (!exists) + (ml > H && lowT[i] < x)
                      + (mr > H && highT[i] > x)
                      + (deep > H && dep[i] >= x+2);
            if (score > best) best = score, who = i;
        }
        if (who < 0) return !heavy(x,group);
        resolve(who);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n < 6 || n > 110) return 0;
    H = n/2;
    for (int i = 1; i < n; ++i) {
        dep[i] = distq(0,i);
        if (dep[i] > L) A = i, L = dep[i];
    }
    lowT[A] = highT[A] = L;
    for (int i = 1; i < n; ++i) if (i != A) {
        lowT[i] = 1;
        highT[i] = min(dep[i],L)-1;
    }

    // Every spine city is represented by a projection, since its degree
    // is at least three. At t its eccentricity is max(L-t,M+t), where
    // M=max_i(dep[i]-2*t_i). When M+t>L-t a maximizing town projects
    // strictly to the left of t. Thus an off-spine city is strictly
    // worse than its projection (one of the two spine directions already
    // attains the projection's eccentricity).
    int R = radius();
    set<int> rejected;
    while (true) {
        auto [a,b] = mBounds();
        vector<int> lo(lowT,lowT+n), hi(highT,highT+n);
        sort(lo.begin(),lo.end());
        sort(hi.begin(),hi.end());
        int s = max({1,lo[n-1-H],L-R});
        int e = min({L-1,hi[H],R-a});
        // A balanced hub must be a median projection. Intersect its
        // possible coordinates with the two arms of the radius formula.
        auto possible = [&](int l,int r) {
            l = max(l,s); r = min(r,e);
            for (int i = 1; i < n; ++i) if (i != A) {
                int u = max(l,lowT[i]), v = min(r,highT[i]);
                while (u <= v && rejected.count(u)) ++u;
                if (u <= v) return true;
            }
            return false;
        };
        int x = L-R;
        bool p = possible(x,x), q = possible(R-b,R-a);
        if (!p && !q) finish(-R);
        vector<int> cand;
        if (p && b+x <= R) cand.push_back(x);
        if (q && a == b && (cand.empty() || cand[0] != R-a))
            cand.push_back(R-a);
        if (cand.empty()) {
            refineM(b);
            continue;
        }
        auto cost = [&](int t) {
            int k = 0, g = 0;
            for (int i = 1; i < n; ++i)
                if (lowT[i] <= t && highT[i] >= t) {
                    k += lowT[i] != highT[i];
                    g += dep[i] >= t+2;
                }
            return k + (g > H ? g : 0);
        };
        int t = cand.front();
        for (int z : cand) if (cost(z) < cost(t)) t = z;
        if (balanced(t)) finish(R);
        rejected.insert(t);
    }
}
