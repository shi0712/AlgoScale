#include <bits/stdc++.h>
using namespace std;

static int n;
static int rnk[2005], invp[2005], cnt_[2005];
static unsigned long long M[40], bestM[40];
static int seenArr[65536], memoVer[65536], choiceL[65536];
static signed char memoVal[65536];
static unsigned int choiceP[65536];
static int curStamp = 0;
static vector<pair<unsigned int,int>> pats;
static long long workDone = 0, workCap = 0;
static bool aborted = false;

static bool feas(unsigned int S){
    int s = __builtin_popcount(S);
    if(s <= 3) return true;
    if(aborted) return false;
    if(memoVer[S] == curStamp) return memoVal[S] != 0;
    memoVer[S] = curStamp; memoVal[S] = 0;
    int d = 0; while((1<<d) < s) ++d;
    int lim = 1 << (d-1);
    for(size_t i = 0; i < pats.size(); ++i){
        if(++workDone > workCap){ aborted = true; return false; }
        unsigned int p = pats[i].first;
        unsigned int A = S & p, B = S & ~p;
        int a = __builtin_popcount(A), b = s - a;
        if(a == 0 || b == 0 || a > lim || b > lim) continue;
        if(feas(A) && feas(B)){ memoVal[S]=1; choiceL[S]=pats[i].second; choiceP[S]=p; return true; }
        if(aborted) return false;
    }
    return false;
}

static int ask(int l, int r){
    printf("? %d %d\n", l, r);
    fflush(stdout);
    int x;
    if(scanf("%d", &x) != 1) exit(0);
    if(x != 0 && x != 1) exit(0);
    return x;
}

int main(){
    if(scanf("%d", &n) != 1) return 0;
    if(n == 1){ printf("! 1\n"); fflush(stdout); return 0; }
    rnk[1] = 1; invp[1] = 0; invp[2] = 0;
    vector<int> T, NT;
    bool exactEnabled = true;
    const long long BUDGET = 20000000LL;
    for(int k = 1; k < n; ++k){
        int K = k + 1;
        T.resize(K);
        for(int i = 0; i < K; ++i) T[i] = i + 1;
        while((int)T.size() > 1){
            int m = (int)T.size();
            int W = (m + 63) >> 6;
            int idx = 0; cnt_[0] = 0;
            for(int x = 1; x <= K; ++x){ if(idx < m && T[idx] == x) ++idx; cnt_[x] = idx; }
            for(int i = 0; i < W; ++i) M[i] = 0ULL;
            for(int i = 0; i < m; ++i) if((K - T[i]) & 1) M[i>>6] |= 1ULL << (i & 63);
            int pc = 0;
            for(int i = 0; i < W; ++i) pc += __builtin_popcountll(M[i]);
            bool exact = exactEnabled && (m >= 4 && m <= 16);
            if(exact){ ++curStamp; pats.clear(); }
            int bestL = 1, bestDiff = INT_MAX;
            for(int i = 0; i < W; ++i) bestM[i] = M[i];
            for(int l = 1; l <= k; ++l){
                int z = m - pc;
                int diff = abs(m - 2*z);
                if(diff < bestDiff){ bestDiff = diff; bestL = l; for(int i = 0; i < W; ++i) bestM[i] = M[i]; }
                if(exact){
                    unsigned int p = (unsigned int)M[0];
                    if(seenArr[p] != curStamp){ seenArr[p] = curStamp; pats.push_back(make_pair(p, l)); }
                } else if(diff <= 1) break;
                int c = cnt_[rnk[l]];
                int w = c >> 6, b = c & 63;
                for(int i = 0; i < w; ++i){ pc += 64 - 2*__builtin_popcountll(M[i]); M[i] = ~M[i]; }
                if(b){ unsigned long long mk = (1ULL << b) - 1ULL; pc += b - 2*__builtin_popcountll(M[w] & mk); M[w] ^= mk; }
            }
            int useL = bestL; bool useEx = false; unsigned int useP = 0;
            if(exact){
                if(workDone > BUDGET) exactEnabled = false;
                for(size_t i = 0; i < pats.size(); ++i) if(pats[i].second == bestL){ if(i) swap(pats[0], pats[i]); break; }
                aborted = false; workCap = workDone + 20000;
                unsigned int full = (unsigned int)((1u << m) - 1u);
                if(feas(full) && !aborted){ useL = choiceL[full]; useP = choiceP[full]; useEx = true; }
            }
            int resp = ask(useL, k + 1);
            int b = resp ^ invp[useL];
            NT.clear();
            if(useEx){
                for(int i = 0; i < m; ++i) if((int)((useP >> i) & 1u) == b) NT.push_back(T[i]);
            } else {
                for(int i = 0; i < m; ++i) if((int)((bestM[i>>6] >> (i & 63)) & 1ULL) == b) NT.push_back(T[i]);
            }
            if((int)NT.size() == m || NT.empty()){
                // should be impossible; safety fallback to avoid stalling
                NT.clear();
                for(int i = 0; i < m; ++i) if((int)((bestM[i>>6] >> (i & 63)) & 1ULL) == b) NT.push_back(T[i]);
                if(NT.empty() || (int)NT.size() == m){ NT.clear(); NT.push_back(T[0]); }
            }
            T.swap(NT);
        }
        int t = T[0];
        int s = 0;
        for(int l = k; l >= 1; --l){ s ^= (rnk[l] >= t) ? 1 : 0; invp[l] ^= s; }
        invp[k+1] = 0;
        for(int l = 1; l <= k; ++l) if(rnk[l] >= t) ++rnk[l];
        rnk[k+1] = t;
    }
    printf("!");
    for(int i = 1; i <= n; ++i) printf(" %d", rnk[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}
