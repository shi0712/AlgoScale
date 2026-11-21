#include <bits/stdc++.h>
using namespace std;

int n, areaCode[64], codeOf[769];
vector<int> areaValue;

int ask(int a, int b, int c) {
    array<int, 3> t{a, b, c};
    sort(t.begin(), t.end());
    cout << "? " << t[0] << ' ' << t[1] << ' ' << t[2] << endl;
    int r;
    if (!(cin >> r) || r < 0 || r > 768 || codeOf[r] < 0) exit(0);
    return codeOf[r];
}

void ambiguous() {
    cout << "! -1" << endl;
    exit(0);
}

// On small instances, optimize the whole decision tree. All complete
// transcripts with multiple preimages have the same required output.
void small() {
    vector<array<int, 3>> q;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            for (int k = j + 1; k < n; ++k)
                q.push_back({i, j, k});
    int m = q.size(), N = 1 << (2*n);
    vector<vector<int>> sig(N);
    map<vector<int>, int> freq;
    vector<int> label(N);
    for (int s = 0; s < N; ++s) {
        for (auto [i,j,k] : q)
            sig[s].push_back(areaCode[((s >> (2*i)) & 3) |
                                (((s >> (2*j)) & 3) << 2) |
                                (((s >> (2*k)) & 3) << 4)]);
        ++freq[sig[s]];
    }
    for (int s = 0; s < N; ++s)
        label[s] = freq[sig[s]] == 1 ? s : -1;

    struct D { int cost = -1, depth = 0, query = -1; };
    vector<D> dp((1<<m)*N);
    auto solve = [&](auto&& self, int mask, const vector<int>& g) -> D {
        D& ret = dp[mask*N + g.front()];
        if (ret.cost != -1) return ret;
        bool done = true;
        for (int x : g) if (label[x] != label[g.front()]) done = false;
        if (done) { ret.cost = 0; return ret; }
        ret.cost = INT_MAX;
        for (int i = 0; i < m; ++i) if (!(mask >> i & 1)) {
            array<vector<int>, 20> sub;
            for (int x : g) sub[sig[x][i]].push_back(x);
            if (sub[sig[g.front()][i]].size() == g.size()) continue;
            int cost = g.size(), depth = 0;
            for (auto &h : sub) if (!h.empty()) {
                D d = self(self, mask | (1<<i), h);
                cost += d.cost;
                depth = max(depth, d.depth + 1);
                if (cost > ret.cost) break;
            }
            if (tie(cost,depth) < tie(ret.cost,ret.depth))
                ret = {cost,depth,i};
        }
        return ret;
    };
    vector<int> g(N);
    iota(g.begin(), g.end(), 0);
    int mask = 0;
    while (true) {
        int i = solve(solve,mask,g).query;
        if (i < 0) break;
        auto [a,b,c] = q[i];
        int r = ask(a+1,b+1,c+1);
        erase_if(g, [&](int x){ return sig[x][i] != r; });
        if (g.empty()) exit(0);
        mask |= 1<<i;
    }
    int x = label[g.front()];
    if (x < 0) ambiguous();
    cout << '!';
    for (int i = 0; i < n; ++i) cout << ' ' << (1+((x>>(2*i))&3));
    cout << endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    fill(begin(codeOf),end(codeOf),-1);
    for (int s = 0; s < 64; ++s) {
        int a = 1+(s&3), b = 1+((s>>2)&3), c = 1+(s>>4);
        int v = max(0,(a+b+c)*(a+b-c)*(a+c-b)*(b+c-a));
        if (codeOf[v] < 0) {
            codeOf[v] = areaValue.size();
            areaValue.push_back(v);
        }
        areaCode[s] = codeOf[v];
    }
    if (n <= 5) { small(); return 0; }

    vector<int> ans(n+1), active, rows{0};
    vector<int> rep[4];
    int countKnown[4]{}, used = 0;

    while (true) {
        // Project away coordinates that are identical in every feasible row.
        int diff = 0;
        for (int x : rows) diff |= x ^ rows[0];
        for (int i = (int)active.size()-1; i >= 0; --i) if (!((diff>>(2*i))&3)) {
            int v = (rows[0]>>(2*i))&3, id = active[i];
            ans[id] = v+1;
            ++countKnown[v];
            if (rep[v].size()<2) rep[v].push_back(id);
            active.erase(active.begin()+i);
            int lo = (1<<(2*i))-1;
            for (int &x : rows) x = (x&lo) | ((x>>2)&~lo);
        }
        int A = active.size();
        if (used == n && !A) break;

        // Probabilities are only for ranking queries, never for elimination.
        double pr[4], freshProb[4][64]{};
        int tot = accumulate(countKnown,countKnown+4,0);
        for (int v = 0; v < 4; ++v)
            pr[v] = .01 + .96*(countKnown[v]+1.0)/(tot+4.0);
        freshProb[0][0] = 1;
        for (int k = 1; k <= 3; ++k)
            for (int s = 0; s < (1<<(2*k)); ++s)
                freshProb[k][s] = freshProb[k-1][s>>2]*pr[s&3];

        vector<double> wt(rows.size(),1);
        double total = 0;
        for (int j = 0; j < (int)rows.size(); ++j) {
            int x = rows[j];
            for (int i = 0; i < A; ++i, x >>= 2) wt[j] *= pr[x&3];
            total += wt[j];
        }
        for (double &w : wt) w /= total;

        double best = -1;
        struct Choice {
            int p,k,known;
            array<int,3> sel, ids;
        } ch{};
        array<int,3> sel{};
        auto search = [&](bool force) {
            auto dfs = [&](auto&& self, int start, int p) -> void {
                double marg[64]{};
                for (int j = 0; j < (int)rows.size(); ++j) {
                    int s = 0;
                    for (int h = 0; h < p; ++h)
                        s |= ((rows[j]>>(2*sel[h]))&3)<<(2*h);
                    marg[s] += wt[j];
                }
                int M = 1<<(2*p);
                for (int k = 0; k+p <= 3 && k <= n-used; ++k) {
                    if ((!k && force) || (!k && !p)) continue;
                    int F = 1<<(2*k);
                    if (k && (force ? k>1 :
                        (A+k>6 || rows.size()*F>384))) continue;
                    int b = 3-p-k;
                    double joint[64];
                    for (int f = 0; f < F; ++f)
                        for (int s = 0; s < M; ++s)
                            joint[f*M+s] = marg[s]*freshProb[k][f];
                    auto test = [&](int known, int i1, int i2) {
                        double hist[20]{};
                        int L = M*F;
                        for (int u = 0; u < L; ++u)
                            hist[areaCode[known*L+u]] += joint[u];
                        int cnt = 0;
                        double score = 0;
                        for (int h = 0; h < (int)areaValue.size(); ++h)
                            if (hist[h]>0) {
                                ++cnt;
                                score -= hist[h]*log(hist[h]);
                            }
                        if (cnt<2 || score <= best+1e-12) return;
                        best = score;
                        ch = {p,k,known,sel,{}};
                        for (int h = 0; h < p; ++h) ch.ids[h]=active[sel[h]];
                        for (int h = 0; h < k; ++h) ch.ids[p+h]=used+h+1;
                        if (b>=1) ch.ids[p+k]=i1;
                        if (b==2) ch.ids[2]=i2;
                    };
                    if (!b) test(0,0,0);
                    if (b==1) for (int v=0;v<4;++v) if (!rep[v].empty())
                        test(v,rep[v][0],0);
                    if (b==2) for (int v=0;v<4;++v) if (!rep[v].empty())
                        for (int w=v;w<4;++w) if ((int)rep[w].size()>(w==v))
                            test(v+4*w,rep[v][0],rep[w][w==v]);
                }
                if (p<3) for (int i=start;i<A;++i) {
                    sel[p]=i;
                    self(self,i+1,p+1);
                }
            };
            dfs(dfs,0,0);
        };
        search(false);
        if (best<0 && used<n) {
            // All queries within the processed part are constant. At this
            // point A <= 8: three equal active entries in any feasible row
            // would give a uniquely identifying equilateral response.
            search(true);
            if (best<0) {
                // One new entry alone may give no information (e.g. too
                // few old entries). Extend the exact state without asking.
                vector<int> next;
                for (int x : rows) for (int f=0;f<4;++f)
                    next.push_back(x|(f<<(2*A)));
                rows.swap(next);
                active.push_back(++used);
                continue;
            }
        }
        if (best<0) ambiguous();
        int r = ask(ch.ids[0],ch.ids[1],ch.ids[2]);
        vector<int> next;
        for (int x : rows) {
            int s = 0;
            for (int h=0;h<ch.p;++h)
                s |= ((x>>(2*ch.sel[h]))&3)<<(2*h);
            for (int f=0;f<(1<<(2*ch.k));++f) {
                int code = s | (f<<(2*ch.p)) | (ch.known<<(2*(ch.p+ch.k)));
                if (areaCode[code]==r) next.push_back(x|(f<<(2*A)));
            }
        }
        if (next.empty()) return 0;
        rows.swap(next);
        for (int h=0;h<ch.k;++h) active.push_back(++used);
    }
    cout << '!';
    for (int i=1;i<=n;++i) cout << ' ' << ans[i];
    cout << endl;
}
