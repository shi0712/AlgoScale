#include <bits/stdc++.h>
using namespace std;

constexpr int N = 100000;
constexpr long double ERR = 0.00000100001L;
using i128 = __int128_t;
using ll = long long;

struct Disk {
    int x, y, r;
};

struct Probe {
    int a, b;
    bool vertical; // Exchange the axes of the segment (0,a)--(N,b).
};

long double ask(Probe p) {
    if (p.vertical)
        cout << "query " << p.a << " 0 " << p.b << ' ' << N << endl;
    else
        cout << "query 0 " << p.a << ' ' << N << ' ' << p.b << endl;

    long double v;
    if (!(cin >> v) || v < 0) exit(0);
    return v;
}

long double chord(const Disk &c, Probe p) {
    ll x = p.vertical ? c.y : c.x;
    ll y = p.vertical ? c.x : c.y;
    ll slope = p.b - p.a;
    ll den = 1LL * N * N + slope * slope;
    ll q = (y - p.a) * N - slope * x;
    i128 h = (i128)c.r * c.r * den - (i128)q * q;
    if (h <= 0) return 0;
    // Both endpoints are on the box boundary, so the whole chord is measured.
    return 2 * sqrtl((long double)h / den);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Interleave the search rows by repeatedly splitting the largest gap.
    // For integer x in the interior, the scan lines are strictly between
    // consecutive integer heights. Spacing 200 therefore leaves no tangencies
    // or gaps for a disk of radius at least 100.
    priority_queue<array<int, 3>> gaps;
    auto push = [&](int l, int r) {
        if (l <= r) gaps.push({r - l + 1, -abs(l + r - 499), l});
    };
    push(0, 499);
    vector<int> missed;
    int row = 0;
    long double hit = 0;
    while (!gaps.empty()) {
        auto g = gaps.top();
        gaps.pop();
        int l = g[2], r = l + g[0] - 1, mid = (l + r) / 2;
        row = 100 + 200 * mid;
        hit = ask({row, row + 1, false});
        if (hit > ERR) break;
        missed.push_back(row);
        push(l, mid - 1);
        push(mid + 1, r);
    }

    int below = -1, above = N;
    for (int s : missed) {
        if (s < row) below = max(below, s);
        else above = min(above, s);
    }

    // q = N*(y-row)-x. For fixed r the response bounds |q| tightly,
    // and each signed q determines x by its residue and then determines y.
    const ll den = 1LL * N * N + 1;
    long double lo = max(0.L, (hit - ERR) / 2);
    long double hi = (hit + ERR) / 2;
    vector<Disk> cand;
    for (int r = max(100, (int)ceill(lo)); r <= N / 2; ++r) {
        ll qlo = max(0LL, (ll)sqrtl(max(0.L, (r-hi)*(r+hi)) * den) - 1);
        ll qhi = (ll)sqrtl(max(0.L, (r-lo)*(r+lo)) * den) + 1;
        for (ll v = qlo; v <= qhi; ++v) {
            for (int sign : {-1, 1}) {
                if (!v && sign == -1) continue;
                ll q = sign * v;
                int x = int((-q) % N);
                if (x < 0) x += N;
                if (x < r || x > N-r) continue;
                int y = row + int((q+x)/N);
                if (y < r || y > N-r) continue;
                Disk c{x,y,r};
                if (fabsl(chord(c,{row,row+1,false}) - hit) > ERR) continue;
                // A more distant parallel scan beyond a missed scan cannot
                // intersect a disk which intersects the hit scan.
                if (below >= 0 && chord(c,{below,below+1,false}) > ERR) continue;
                if (above < N && chord(c,{above,above+1,false}) > ERR) continue;
                cand.push_back(c);
            }
        }
    }

    mt19937 rng(172391);
    auto clip = [](int v) { return clamp(v, 0, N); };

    while (cand.size() > 1) {
        int n = (int)cand.size();
        vector<Probe> options;
        set<tuple<int,int,bool>> seen;
        auto add = [&](int a, int b, bool vert = false) {
            a = clip(a); b = clip(b);
            if (seen.emplace(a,b,vert).second) options.push_back({a,b,vert});
        };

        for (int k : {1, -1, 2, -2, 5, -5, 17, -17}) {
            add(row+k,row+1+k);
            add(row-k,row+1+k);
            add(row,row+1+k);
        }

        Disk big = *max_element(cand.begin(),cand.end(),
                                [](Disk a, Disk b){ return a.r < b.r; });
        // The two axial diameters of a maximum-radius candidate cannot both
        // be reproduced (within twice the error) by any different candidate.
        add(big.y,big.y);
        add(big.x,big.x,true);
        int trials = n < 300 ? 160 : 44;
        for (int i = 0; i < trials; ++i) {
            Disk c = cand[rng()%n];
            if (i % 5 == 0) {
                add(c.x,c.x,true);
                add(c.y,c.y);
            }
            int scale = max(2, (int)(min<long double>(N,hit+4)));
            int k = 1 + (rng() % scale);
            if (rng() & 1) k = -k;
            int offset = (int)llround((long double)k*c.x/N);
            if (i % 3 == 0) offset += int(rng()%7)-3;
            add(row-offset,row+1+k-offset);
        }

        Probe best{};
        double bestCost = numeric_limits<double>::infinity();
        vector<long double> vals(n);
        vector<double> penalty(n+1);
        for (int i=2; i<=n; ++i) penalty[i] = 1 + 0.2*log2((double)i);
        for (Probe p : options) {
            for (int i=0; i<n; ++i) vals[i]=chord(cand[i],p);
            sort(vals.begin(),vals.end());
            if (vals.back()-vals.front() <= 2*ERR) continue;
            // Prefer probes that identify the candidate outright even with
            // worst-case rounding, and penalize large unresolved groups.
            double cost=0;
            int l=0, h=0;
            for (int i=0; i<n; ++i) {
                while (vals[i]-vals[l] > 2*ERR) ++l;
                while (h<n && vals[h]-vals[i] <= 2*ERR) ++h;
                cost += penalty[h-l];
                if (cost >= bestCost) break;
            }
            if (cost < bestCost) {
                bestCost=cost;
                best=p;
                if (cost==0) break;
            }
        }

        long double v=ask(best);
        cand.erase(remove_if(cand.begin(),cand.end(),[&](const Disk &c){
            return fabsl(chord(c,best)-v) > ERR;
        }),cand.end());
    }

    if (!cand.empty()) {
        Disk c=cand[0];
        cout << "answer " << c.x << ' ' << c.y << ' ' << c.r << endl;
    }
    return 0;
}
