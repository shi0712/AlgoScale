#include <bits/stdc++.h>
using namespace std;

int n, m;
mt19937 rng(712367);

long long askq(int v, const vector<char>& b) {
    string s(m, '0');
    for (int i = 0; i < m; i++) s[i] = b[i] ? '1' : '0';
    printf("? %d %s\n", v, s.c_str());
    fflush(stdout);
    long long a;
    if (scanf("%lld", &a) != 1) exit(0);
    if (a < 0) exit(0);
    return a;
}

vector<int> rr;
vector<vector<int>> Dl;
vector<int> depthv, par_, pe_;
vector<vector<int>> pathE;
vector<double> gt;

int findParent(int w, int d) {
    vector<int> cand = Dl[d - 1];
    static vector<vector<int>> mem;
    if ((int)mem.size() != m) mem.assign(m, {});
    while (cand.size() > 1) {
        int M = cand.size();
        vector<int> usedE;
        for (int i = 0; i < M; i++) {
            for (int e : pathE[cand[i]]) {
                if (mem[e].empty()) usedE.push_back(e);
                mem[e].push_back(i);
            }
        }
        vector<int> rel;
        for (int e : usedE) if ((int)mem[e].size() < M) rel.push_back(e);
        vector<char> bestx(m, 0);
        vector<int> bestcv(M, 0);
        double bestF = 1e18;
        int starts = 2;
        vector<int> cnt(d + 3);
        for (int st = 0; st < starts; st++) {
            vector<char> x(m, 0);
            vector<int> cv(M, 0);
            for (int e : rel) {
                x[e] = rng() & 1;
                if (x[e]) for (int i : mem[e]) cv[i]++;
            }
            fill(cnt.begin(), cnt.end(), 0);
            for (int i = 0; i < M; i++) cnt[cv[i]]++;
            double F = 0;
            for (size_t c = 0; c < cnt.size(); c++) F += gt[cnt[c]];
            auto apply = [&](int e, int dl) {
                for (int i : mem[e]) {
                    int c = cv[i];
                    F -= gt[cnt[c]]; cnt[c]--; F += gt[cnt[c]];
                    F -= gt[cnt[c + dl]]; cnt[c + dl]++; F += gt[cnt[c + dl]];
                    cv[i] += dl;
                }
            };
            vector<int> order = rel;
            for (int pass = 0; pass < 5; pass++) {
                shuffle(order.begin(), order.end(), rng);
                bool imp = false;
                for (int e : order) {
                    double old = F;
                    int dl = x[e] ? -1 : 1;
                    apply(e, dl);
                    if (F < old - 1e-9) {
                        x[e] ^= 1;
                        imp = true;
                    } else {
                        apply(e, -dl);
                    }
                }
                if (!imp) break;
            }
            if (F < bestF - 1e-9) {
                bestF = F; bestx = x; bestcv = cv;
            }
        }
        double single = gt[M];
        if (bestF >= single - 1e-9) {
            fill(bestx.begin(), bestx.end(), 0);
            int e = rel[0];
            bestx[e] = 1;
            fill(bestcv.begin(), bestcv.end(), 0);
            for (int i : mem[e]) bestcv[i] = 1;
        }
        int sumx = 0;
        vector<char> b(m);
        for (int i = 0; i < m; i++) {
            sumx += bestx[i];
            b[i] = rr[i] ^ bestx[i];
        }
        long long ans = askq(w, b);
        int val = (int)((ans - m + sumx + d) / 2);
        vector<int> nc;
        for (int i = 0; i < M; i++) if (bestcv[i] == val) nc.push_back(cand[i]);
        cand = nc;
        for (int e : usedE) mem[e].clear();
        if (cand.empty()) exit(0);
    }
    return cand[0];
}

int findEdge(int w, int d, vector<int>& U) {
    vector<int> cand = U;
    while (cand.size() > 1) {
        int sz = cand.size();
        int t = sz / 2;
        vector<char> x(m, 0);
        for (int i = 0; i < t; i++) x[cand[i]] = 1;
        vector<char> b(m);
        for (int i = 0; i < m; i++) b[i] = rr[i] ^ x[i];
        long long ans = askq(w, b);
        int val = (int)((ans - m + t + d) / 2);
        vector<int> nc;
        if (val == 1) nc.assign(cand.begin(), cand.begin() + t);
        else nc.assign(cand.begin() + t, cand.end());
        cand = nc;
    }
    int e = cand[0];
    U.erase(find(U.begin(), U.end(), e));
    return e;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    m = n - 1;
    gt.assign(1000, 0.0);
    for (int c = 1; c < 1000; c++) gt[c] = c * log2((double)c);
    vector<char> ones(m, 1);
    int T = (int)askq(1, ones);
    rr.assign(m, 0);
    {
        vector<pair<vector<int>, int>> st;
        vector<int> all(m);
        iota(all.begin(), all.end(), 0);
        st.push_back({all, T});
        while (!st.empty()) {
            auto cur = st.back();
            st.pop_back();
            vector<int>& idx = cur.first;
            int k = cur.second;
            int sz = idx.size();
            if (k == 0) continue;
            if (k == sz) {
                for (int i : idx) rr[i] = 1;
                continue;
            }
            int t = sz / 2;
            vector<char> b(m, 0);
            for (int i = 0; i < t; i++) b[idx[i]] = 1;
            long long ans = askq(1, b);
            int kA = (int)((ans - m + T + t) / 2);
            vector<int> A(idx.begin(), idx.begin() + t), B(idx.begin() + t, idx.end());
            st.push_back({A, kA});
            st.push_back({B, k - kA});
        }
    }
    if (n == 2) {
        if (rr[0]) printf("! 2 1\n");
        else printf("! 1 2\n");
        fflush(stdout);
        return 0;
    }
    depthv.assign(n + 1, 0);
    {
        vector<char> b(m);
        for (int i = 0; i < m; i++) b[i] = rr[i];
        for (int w = 2; w <= n; w++) {
            long long ans = askq(w, b);
            depthv[w] = m - (int)ans;
        }
    }
    int maxD = 0;
    for (int w = 2; w <= n; w++) maxD = max(maxD, depthv[w]);
    Dl.assign(maxD + 2, {});
    Dl[0].push_back(1);
    for (int w = 2; w <= n; w++) Dl[depthv[w]].push_back(w);
    par_.assign(n + 1, 0);
    pe_.assign(n + 1, -1);
    pathE.assign(n + 1, {});
    vector<int> U(m);
    iota(U.begin(), U.end(), 0);
    vector<int> childOf(m, 0);
    for (int d = 1; d <= maxD; d++) {
        for (int w : Dl[d]) {
            int p;
            if (d == 1) p = 1;
            else p = findParent(w, d);
            int e = findEdge(w, d, U);
            par_[w] = p;
            pe_[w] = e;
            pathE[w] = pathE[p];
            pathE[w].push_back(e);
            childOf[e] = w;
        }
    }
    printf("!");
    for (int i = 0; i < m; i++) {
        int w = childOf[i];
        int p = par_[w];
        if (rr[i]) printf(" %d %d", w, p);
        else printf(" %d %d", p, w);
    }
    printf("\n");
    fflush(stdout);
    return 0;
}
