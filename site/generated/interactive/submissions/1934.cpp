#include <bits/stdc++.h>
using namespace std;

int ask(const vector<int>& v) {
    cout << "? " << v.size();
    for (int i : v) cout << ' ' << i;
    cout << endl;
    int r;
    if (!(cin >> r) || r < 0) exit(0);
    return r;
}

struct Plan {
    int worst, action;
    double mean;
};

int effectiveN;
map<pair<int,int>, Plan> memo;

Plan plan(int a, int b) {
    if (a < b) swap(a, b);
    int left = effectiveN - a - b;
    if (!left) return {0, 0, 0};
    auto key = make_pair(a,b);
    if (auto it = memo.find(key); it != memo.end()) return it->second;

    // Batch-only bound: one endpoint is identified in each query.
    int i = 0, covered = 0;
    while (covered < left) {
        covered += max(a, (a+b+i+1)/2);
        ++i;
    }
    double p = (a+1.0)/(a+b+2.0);
    double est = 0;
    int j = 0;
    while (est + 1e-8 < left) {
        est += max(a+p*j, b+(1-p)*j);
        ++j;
    }
    Plan best{i, 0, double(j)};
    if (a >= 2 && left > a && a+b < 350) {
        // First query: one endpoint and g interior unknowns,
        // all separated by known mushrooms of the majority type.
        for (int g = 1; g <= 3; ++g) {
            if (a < g+1 || (g == 3 && b < 2)) continue;
            int extra = g == 1 ? 0 : (g == 2 && b >= 2 ? 2 : 1);
            if (left < g+1+extra) continue;
            int w = 0;
            double e = 0;
            for (int x=0; x<=1; ++x) for (int h=0; h<=g; ++h) {
                // x and h count majority-type labels.
                int d = x+h;
                double prob = (x ? p : 1-p) * pow(p,h)*pow(1-p,g-h);
                if (h && h<g) prob *= g; // binomial(g,h), g<=3
                int k = (h && h<g) ? extra : 0;
                for (int t=0; t<=k; ++t) {
                    Plan sub = plan(a+d+t,b+g+1-d+k-t);
                    int cost = 1+(k!=0);
                    w = max(w, cost+sub.worst);
                    double q = pow(p,t)*pow(1-p,k-t);
                    if (k==2 && t==1) q *= 2;
                    e += prob*q*(cost+sub.mean);
                }
            }
            if (w < best.worst || (w == best.worst && e < best.mean-1e-8))
                best = {w,g,e};
        }
    }
    return memo[key] = best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    vector<int> pool[2] = {{0}, {}};
    int pos=1, anonymousA=0;
    effectiveN=n;
    bool bulk=false, tryPure=true;
    while (pos<n) {
        int c = pool[1].size() > pool[0].size();
        auto &s=pool[c], &o=pool[c^1];
        int a=s.size(), b=o.size();
        if (a==1 && n-pos>1) {
            int id=pos++;
            pool[c^ask({s[0],id})].push_back(id);
            continue;
        }
        bool probe=!bulk && tryPure && b==0 && a>=4;
        int g=(bulk || probe) ? 0 : plan(a,b).action;
        if (!g) {
            if (!probe) bulk=true;
            int m=min(a,n-pos), base=pos;
            vector<int> v;
            for (int j=0; j<m; ++j) {
                v.push_back(pos++);
                v.push_back(s[j]);
            }
            int r=ask(v);
            pool[c^(r&1)].push_back(base);
            int diff=r/2;
            if (diff==0 || diff==m-1) {
                int type=c^(diff!=0);
                for (int j=1; j<m; ++j) pool[type].push_back(base+j);
            } else {
                anonymousA += c ? diff : m-1-diff;
                if (probe) {
                    effectiveN-=m-1;
                    memo.clear();
                }
            }
            if (r) tryPure=false;
            continue;
        }

        int base=pos;
        vector<int> v;
        for (int j=0; j<=g; ++j) {
            v.push_back(pos++);
            v.push_back(s[j]);
        }
        int r=ask(v), h=r/2;
        pool[c^(r&1)].push_back(base);
        if (h==0 || h==g) {
            for (int j=1; j<=g; ++j)
                pool[c^(h!=0)].push_back(base+j);
            continue;
        }

        // Interior group is mixed. Encode its orientation together
        // with fresh bits in a second machine result.
        int y=base+1, z=base+2;
        if (g==2 && b<2) {
            int u=pos++;
            int t=ask({y,s[0],u,s[1]});
            pool[c^(t&1)].push_back(y);
            pool[c^1^(t&1)].push_back(z);
            pool[c^(t/2)].push_back(u);
        } else if (g==2) {
            int u=pos++, w=pos++;
            int t=ask({o[0],y,o[1],s[0],z,s[1],u,s[2],w})-1;
            // y and z are opposite; the first two sandwiches
            // contribute either zero or four.
            pool[c^(1^(t/4))].push_back(y);
            pool[c^(t/4)].push_back(z);
            pool[c^((t/2)&1)].push_back(u);
            pool[c^(t&1)].push_back(w);
        } else {
            int u=pos++;
            int t=ask({s[0],y,s[1],o[0],z,o[1],u})-1;
            pool[c^1^(t&1)].push_back(u);
            // t/2 = bit(y) + 1 - bit(z), bits relative to c.
            for (int mask=0; mask<8; ++mask) {
                if (__builtin_popcount((unsigned)mask)!=h ||
                    (mask&1)+1-((mask>>1)&1)!=t/2) continue;
                for (int j=0; j<3; ++j)
                    pool[c^((mask>>j)&1)].push_back(y+j);
                break;
            }
        }
    }
    cout << "! " << int(pool[0].size())+anonymousA << endl;
    return 0;
}
