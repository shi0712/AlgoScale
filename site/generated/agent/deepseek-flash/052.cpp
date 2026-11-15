#include <bits/stdc++.h>
using namespace std;

static const long long INF = (long long)4e18;
static const long long NEG = -(long long)4e18;

struct Item {
    int p;
    int v;           // w[p]
    int w[11];       // w[1..p]
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if(!(cin>>n>>k)) return 0;
    vector<Item> items(n);
    long long totalP = 0, totalV = 0;
    for(int i=0;i<n;i++){
        int p; cin>>p;
        items[i].p = p;
        for(int j=1;j<=p;j++) cin>>items[i].w[j];
        items[i].v = items[i].w[p];
        totalP += p;
        totalV += items[i].v;
    }
    if(totalP <= (long long)k){
        cout << totalV << "\n";
        return 0;
    }

    // sort by full-value density v/p descending
    sort(items.begin(), items.end(), [](const Item&a, const Item&b){
        long long lhs = 1LL*a.v*b.p;
        long long rhs = 1LL*b.v*a.p;
        if(lhs != rhs) return lhs > rhs;
        return a.p < b.p;
    });

    // greedy prefix P: stop at the first item that does not fit
    long long WP = 0, VP = 0;
    int b = n;
    for(int i=0;i<n;i++){
        if(WP + items[i].p <= (long long)k){
            WP += items[i].p;
            VP += items[i].v;
        } else {
            b = i;
            break;
        }
    }
    // totalP > k, so b < n
    int u = (int)((long long)k - WP); // 0 <= u < weight of break item <= 10
    long long B = min(1000LL, (long long)k + 10);
    int Bint = (int)B;
    int AMAX = Bint + 9;

    vector<char> compP(n,0), compO(n,0);
    vector<int> idxP[11], idxO[11];
    for(int i=0;i<b;i++) idxP[items[i].p].push_back(i);
    for(int i=b;i<n;i++) idxO[items[i].p].push_back(i);

    vector<int> CP, CA;

    // smallest B/p values per weight in P
    for(int p=1;p<=10;p++){
        auto &vec = idxP[p];
        if(vec.empty()) continue;
        sort(vec.begin(), vec.end(), [&](int x,int y){
            return items[x].v < items[y].v;
        });
        int K = Bint / p;
        int take = min((int)vec.size(), K);
        for(int t=0;t<take;t++){ compP[vec[t]]=1; CP.push_back(vec[t]); }
    }

    // largest (B+9)/p + 1 values per weight outside P
    for(int p=1;p<=10;p++){
        auto &vec = idxO[p];
        if(vec.empty()) continue;
        sort(vec.begin(), vec.end(), [&](int x,int y){
            return items[x].v > items[y].v;
        });
        int K = AMAX / p + 1;
        int take = min((int)vec.size(), K);
        for(int t=0;t<take;t++){ compO[vec[t]]=1; CA.push_back(vec[t]); }
    }

    // aggregates for non-compressed items
    // bestDeltaP[p][r] = max(w_{j,r} - v_j)
    long long bestDeltaP[11][11];
    for(int p=0;p<=10;p++) for(int r=0;r<=10;r++) bestDeltaP[p][r] = NEG;
    for(int i=0;i<b;i++){
        if(compP[i]) continue;
        int p = items[i].p;
        for(int r=1;r<p;r++){
            long long val = (long long)items[i].w[r] - items[i].v;
            if(val > bestDeltaP[p][r]) bestDeltaP[p][r] = val;
        }
    }
    // bestWOut[r] = max w_{j,r} for non-compressed outside items
    long long bestWOut[11];
    for(int r=0;r<=10;r++) bestWOut[r] = NEG;
    for(int i=b;i<n;i++){
        if(compO[i]) continue;
        int p = items[i].p;
        for(int r=1;r<p;r++){
            if(items[i].w[r] > bestWOut[r]) bestWOut[r] = items[i].w[r];
        }
    }

    // base removal DP: minimum value of a subset of CP with exact weight
    vector<long long> remBase(Bint+1, INF);
    remBase[0] = 0;
    for(int id : CP){
        int p = items[id].p;
        long long v = items[id].v;
        for(int w=Bint; w>=p; --w){
            if(remBase[w-p] != INF){
                long long cand = remBase[w-p] + v;
                if(cand < remBase[w]) remBase[w] = cand;
            }
        }
    }

    // base addition DP: maximum value of a subset of CA with exact weight
    vector<long long> addBase(AMAX+1, NEG);
    addBase[0] = 0;
    for(int id : CA){
        int p = items[id].p;
        long long v = items[id].v;
        for(int a=AMAX; a>=p; --a){
            if(addBase[a-p] != NEG){
                long long cand = addBase[a-p] + v;
                if(cand > addBase[a]) addBase[a] = cand;
            }
        }
    }

    // bestDiff for delta in [u-9, u+9]
    const int OFF = 9;
    long long bestDiff[19];
    for(int d=0; d<19; d++) bestDiff[d] = NEG;
    for(int d=-9; d<=9; d++){
        int delta = u + d;
        long long best = NEG;
        for(int rr=0; rr<=Bint; rr++){
            if(remBase[rr] == INF) continue;
            int aa = rr + delta;
            if(aa < 0 || aa > AMAX) continue;
            if(addBase[aa] == NEG) continue;
            long long cand = addBase[aa] - remBase[rr];
            if(cand > best) best = cand;
        }
        bestDiff[d + OFF] = best;
    }
    auto getDiff = [&](int delta)->long long {
        int d = delta - u;
        if(d < -9 || d > 9) return NEG;
        return bestDiff[d + OFF];
    };

    long long ans = NEG;

    // no partial item, exact weight k
    {
        long long g = getDiff(u);
        if(g != NEG) ans = max(ans, VP + g);
    }

    // non-compressed outside items as partial
    for(int r=1;r<=9;r++){
        if(bestWOut[r] == NEG) continue;
        long long g = getDiff(u - r);
        if(g == NEG) continue;
        ans = max(ans, VP + bestWOut[r] + g);
    }

    // non-compressed P items as partial
    for(int p=2;p<=10;p++){
        for(int r=1;r<p;r++){
            if(bestDeltaP[p][r] == NEG) continue;
            long long g = getDiff(u + p - r);
            if(g == NEG) continue;
            ans = max(ans, VP + bestDeltaP[p][r] + g);
        }
    }

    // D&C exclusion for compressed P items
    // At leaf i, dp is the removal DP over CP except item i.
    function<void(int,int,vector<long long>&)> solveCP = [&](int l, int r, vector<long long>& dp){
        if(l == r){
            int id = CP[l];
            int p = items[id].p;
            long long v = items[id].v;
            int maxrr = Bint - p;
            if(maxrr < 0) maxrr = -1;
            for(int r=1;r<p;r++){
                int offset = p + u - r;
                long long best = NEG;
                for(int rr=0; rr<=maxrr; rr++){
                    if(dp[rr] == INF) continue;
                    int aa = rr + offset;
                    if(aa < 0 || aa > AMAX) continue;
                    if(addBase[aa] == NEG) continue;
                    long long cand = addBase[aa] - dp[rr];
                    if(cand > best) best = cand;
                }
                if(best != NEG){
                    long long cand = VP - v + items[id].w[r] + best;
                    if(cand > ans) ans = cand;
                }
            }
            return;
        }
        int mid = (l + r) >> 1;
        vector<long long> left = dp;
        for(int i=mid+1; i<=r; i++){
            int id = CP[i];
            int p = items[id].p;
            long long v = items[id].v;
            for(int w=Bint; w>=p; --w){
                if(left[w-p] != INF){
                    long long cand = left[w-p] + v;
                    if(cand < left[w]) left[w] = cand;
                }
            }
        }
        solveCP(l, mid, left);

        vector<long long> right = dp;
        for(int i=l; i<=mid; i++){
            int id = CP[i];
            int p = items[id].p;
            long long v = items[id].v;
            for(int w=Bint; w>=p; --w){
                if(right[w-p] != INF){
                    long long cand = right[w-p] + v;
                    if(cand < right[w]) right[w] = cand;
                }
            }
        }
        solveCP(mid+1, r, right);
    };

    if(!CP.empty()){
        vector<long long> dp0(Bint+1, INF);
        dp0[0] = 0;
        solveCP(0, (int)CP.size()-1, dp0);
    }

    // D&C exclusion for compressed outside items
    function<void(int,int,vector<long long>&)> solveCA = [&](int l, int r, vector<long long>& dp){
        if(l == r){
            int id = CA[l];
            int p = items[id].p;
            for(int r=1;r<p;r++){
                int offset = u - r;
                long long best = NEG;
                for(int rr=0; rr<=Bint; rr++){
                    if(remBase[rr] == INF) continue;
                    int aa = rr + offset;
                    if(aa < 0 || aa > AMAX) continue;
                    if(dp[aa] == NEG) continue;
                    long long cand = dp[aa] - remBase[rr];
                    if(cand > best) best = cand;
                }
                if(best != NEG){
                    long long cand = VP + items[id].w[r] + best;
                    if(cand > ans) ans = cand;
                }
            }
            return;
        }
        int mid = (l + r) >> 1;
        vector<long long> left = dp;
        for(int i=mid+1; i<=r; i++){
            int id = CA[i];
            int p = items[id].p;
            long long v = items[id].v;
            for(int a=AMAX; a>=p; --a){
                if(left[a-p] != NEG){
                    long long cand = left[a-p] + v;
                    if(cand > left[a]) left[a] = cand;
                }
            }
        }
        solveCA(l, mid, left);

        vector<long long> right = dp;
        for(int i=l; i<=mid; i++){
            int id = CA[i];
            int p = items[id].p;
            long long v = items[id].v;
            for(int a=AMAX; a>=p; --a){
                if(right[a-p] != NEG){
                    long long cand = right[a-p] + v;
                    if(cand > right[a]) right[a] = cand;
                }
            }
        }
        solveCA(mid+1, r, right);
    };

    if(!CA.empty()){
        vector<long long> dp1(AMAX+1, NEG);
        dp1[0] = 0;
        solveCA(0, (int)CA.size()-1, dp1);
    }

    cout << ans << "\n";
    return 0;
}
