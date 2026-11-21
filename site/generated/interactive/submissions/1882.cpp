#include <bits/stdc++.h>
using namespace std;

constexpr int M = 11;

int n;
vector<vector<int>> tree;
vector<int> f;

long long ask(const vector<pair<int,int>>& terms) {
    int k = 0;
    for (auto [v, w] : terms) k += w;
    cout << "? 1 " << k;
    for (auto [v, w] : terms)
        for (int j = 0; j < w; ++j) cout << ' ' << v + 1;
    cout << endl;
    long long result;
    // Negative sums, including -1, are legal replies.
    if (!(cin >> result) || result < -1LL * n * k || result > 1LL * n * k)
        exit(0);
    return result;
}

void flip(int v) {
    cout << "? 2 " << v + 1 << endl;
}

void answer(const vector<int>& a) {
    cout << '!';
    for (int x : a) cout << ' ' << x;
    cout << endl;
}

struct Code {
    array<int,M> p{};
    int sum = 0;
    vector<int> mask;
};
vector<Code> codes[M+1];

void initCodes() {
    const int u[] = {0,1,2,4,7,13,24,44,84,161,309,594};
    for (int m = 1; m <= M; ++m) {
        for (int h = 0; h <= m; ++h) {
            if (h > 0 && h < 4) continue;
            int low = m - h;
            Code c;
            for (int i = 0; i < h; ++i) c.p[i] = (u[h] - u[i]) << low;
            for (int i = h; i < m; ++i) c.p[i] = 1 << (m-1-i);
            if (c.p[0] > n) continue;
            for (int i = 0; i < m; ++i) c.sum += c.p[i];
            c.mask.assign(c.sum + 1, -1);
            int s[1 << M] = {};
            bool good = true;
            for (int b = 0; b < (1 << m); ++b) {
                if (b) s[b] = s[b & (b-1)] + c.p[__builtin_ctz((unsigned)b)];
                if (c.mask[s[b]] != -1) {
                    good = false;
                    break;
                }
                c.mask[s[b]] = b;
            }
            if (good) codes[m].push_back(move(c));
        }
    }
}

struct Rooted {
    int b;
    vector<int> par, size, height, order;
    vector<vector<int>> ch;
    Rooted(int root) : b(root), par(n,-1), size(n,1), height(n,0), ch(n) {
        order.push_back(b);
        for (int i = 0; i < (int)order.size(); ++i) {
            int v = order[i];
            for (int w : tree[v]) if (w != par[v]) {
                par[w] = v;
                ch[v].push_back(w);
                order.push_back(w);
            }
        }
        for (int i = n-1; i > 0; --i) {
            int v = order[i], p = par[v];
            size[p] += size[v];
            height[p] = max(height[p], height[v] + 1);
        }
    }
};

struct Pack {
    const Code* code = nullptr;
    array<int,M> index{};
    array<int,M> weight{};
};

Pack makePack(const vector<int>& v, const vector<int>& pos,
              const vector<int>& par, int start, int m) {
    if (codes[m].empty()) return {};
    int parent[M], sz[M], ht[M], childMask[M+1] = {};
    for (int i = 0; i < m; ++i) {
        int p = pos[par[v[start+i]]];
        parent[i] = p < start ? m : p - start;
        childMask[parent[i]] |= 1 << i;
        sz[i] = 1;
        ht[i] = 0;
    }
    for (int i = m-1; i >= 0; --i) if (parent[i] != m) {
        sz[parent[i]] += sz[i];
        ht[parent[i]] = max(ht[parent[i]], ht[i] + 1);
    }

    array<int,M> linear{};
    for (int mode = 0; mode < 3; ++mode) {
        if (mode == 2) {
            iota(linear.begin(), linear.begin()+m, 0);
        } else {
            int next = 0;
            auto dfs = [&](auto&& self, int mask) -> void {
                while (mask) {
                    int pick = -1, best = -1;
                    for (int bits = mask; bits; bits &= bits-1) {
                        int x = __builtin_ctz((unsigned)bits);
                        int score = mode == 0 ? 16*sz[x]+ht[x] : 16*ht[x]+sz[x];
                        if (score > best) best = score, pick = x;
                    }
                    mask ^= 1 << pick;
                    linear[next++] = pick;
                    self(self, childMask[pick]);
                }
            };
            dfs(dfs, childMask[m]);
        }
        if (mode == 0 && childMask[m] == (1 << m)-1 &&
            (1 << m)-1 > n)
            return {};
        for (const Code& code : codes[m]) {
            int weights[M+1] = {};
            int k = 0;
            for (int t = 0; t < m; ++t) {
                int x = linear[t], p = code.p[t];
                weights[x] += p;
                if (parent[x] == m) k += p;
                else weights[parent[x]] -= p;
            }
            if (k > n) continue;
            bool good = true;
            for (int i = 0; i < m; ++i) if (weights[i] < 0) good = false;
            if (!good) continue;
            Pack result;
            result.code = &code;
            result.index = linear;
            for (int i = 0; i < m; ++i) result.weight[i] = weights[i];
            return result;
        }
        if (codes[m].size() == 1 && codes[m][0].sum == (1 << m)-1) break;
    }
    return {};
}

struct Cost {
    int worst;
    double mean;
};
bool better(Cost a, Cost b) {
    return a.worst != b.worst ? a.worst < b.worst : a.mean < b.mean - 1e-9;
}

struct Plan {
    Cost cost{INT_MAX, 1e30};
    int root = -1, initial = 0;
    vector<int> order, par, step;
};
Plan bestPlan;
int leafBits;
vector<Cost> leafCost;

void initLeafCost() {
    leafBits = 0;
    while ((1 << (leafBits+1))-1 <= n) ++leafBits;
    leafCost.resize(n+1);
    leafCost[0] = {0,0};
    for (int d = 1; d <= n; ++d) {
        if (d <= leafBits) {
            leafCost[d] = {1,1};
        } else {
            int top = n - ((1 << leafBits)-1);
            if (!top) {
                leafCost[d] = leafCost[d-leafBits];
                ++leafCost[d].worst;
                ++leafCost[d].mean;
            } else {
                double p = 1.0 - double(top)/(1 << leafBits);
                leafCost[d] = {
                    1 + leafCost[d-leafBits].worst,
                    1 + p*leafCost[d-leafBits].mean + (1-p)*leafCost[d-leafBits-1].mean
                };
            }
        }
    }
}

void consider(const Rooted& root, const vector<int>& v) {
    int L = n-1;
    vector<int> pos(n, -1);
    for (int i = 0; i < L; ++i) pos[v[i]] = i;
    int leafStart = L;
    while (leafStart && root.ch[v[leafStart-1]].empty()) --leafStart;
    vector<Cost> dp(L+1);
    vector<int> step(L+1);
    dp[L] = {0,0};
    for (int i = L-1; i >= 0; --i) {
        if (i >= leafStart) {
            dp[i] = leafCost[L-i];
            step[i] = -1;
            continue;
        }
        dp[i] = {INT_MAX, 1e30};
        for (int m = min(M,L-i); m >= 1; --m) {
            Cost trial{dp[i+m].worst + 1, dp[i+m].mean + 1};
            if (better(trial, dp[i]) && makePack(v,pos,root.par,i,m).code) {
                dp[i] = trial;
                step[i] = m;
            }
        }
    }
    for (int m = min(M,L); m >= 0; --m) {
        if (m && (1 << m)+1 > n) continue;
        Cost trial{dp[m].worst+1,dp[m].mean+1};
        if (better(trial,bestPlan.cost)) {
            bestPlan = {trial,root.b,m,v,root.par,step};
        }
    }
}

pair<int,vector<int>> farthest(int b) {
    vector<int> p(n,-1), q{b};
    for (int i = 0; i < (int)q.size(); ++i) {
        int v = q[i];
        for (int w : tree[v]) if (w != p[v]) {
            p[w] = v;
            q.push_back(w);
        }
    }
    return {q.back(),move(p)};
}

void planQueries() {
    initCodes();
    initLeafCost();
    int a = farthest(0).first;
    auto [b, p] = farthest(a);
    vector<int> diam;
    for (int x = b; x != -1; x = p[x]) diam.push_back(x);

    Rooted base(0);
    int centroid = 0, highDegree = 0;
    auto balance = [&](int v) {
        int r = n - base.size[v];
        for (int w : base.ch[v]) r = max(r,base.size[w]);
        return r;
    };
    int bal = balance(0);
    for (int v = 1; v < n; ++v) {
        int z = balance(v);
        if (z < bal) centroid = v, bal = z;
        if (tree[v].size() > tree[highDegree].size()) highDegree = v;
    }

    mt19937 rng(0x4c415544);
    vector<int> roots{centroid,highDegree,a,b,diam[diam.size()/2],0};
    for (int t = 0; t < 5; ++t) roots.push_back(int(rng()%n));
    sort(roots.begin(),roots.end());
    roots.erase(unique(roots.begin(),roots.end()),roots.end());
    for (int r : roots) {
        Rooted g(r);
        set<vector<int>> tested;
        for (int mode = 0; mode < 7; ++mode) {
            vector<vector<int>> ch = g.ch;
            vector<int> v;
            for (auto& c : ch) {
                if (mode >= 4) shuffle(c.begin(),c.end(),rng);
                else sort(c.begin(),c.end(),[&](int x, int y) {
                    if (mode == 0) return tie(g.size[x],x) > tie(g.size[y],y);
                    if (mode == 1) return tie(g.size[x],x) < tie(g.size[y],y);
                    if (mode == 2) return tie(g.height[x],g.size[x],x) >
                                          tie(g.height[y],g.size[y],y);
                    return tie(g.height[x],g.size[x],x) <
                           tie(g.height[y],g.size[y],y);
                });
            }
            if (mode == 6) {
                v = g.order;
                v.erase(v.begin());
            } else {
                auto dfs = [&](auto&& self, int x) -> void {
                    for (int y : ch[x]) {
                        v.push_back(y);
                        self(self,y);
                    }
                };
                dfs(dfs,r);
            }
            if (tested.insert(v).second) consider(g,v);
            vector<int> w = v;
            stable_partition(w.begin(),w.end(),[&](int x) { return !g.ch[x].empty(); });
            if (tested.insert(w).second) consider(g,w);
            if (mode == 0 || mode == 2 || mode == 4) {
                vector<char> deferred(n);
                for (int x : v) if (g.ch[x].empty()) deferred[x] = (rng()%4 != 0);
                stable_partition(v.begin(),v.end(),[&](int x) { return !deferred[x]; });
                if (tested.insert(v).second) consider(g,v);
            }
        }
    }
}

struct LeafEntry { int v, z, o; };

void solveLeaves(const vector<int>& v, const vector<int>& par, int i) {
    vector<vector<LeafEntry>> pool;
    vector<pair<int,int>> weights;
    for (; i < (int)v.size(); ++i) {
        int x = v[i], y = f[par[x]];
        pool.push_back({{x,y-1,y+1}});
    }

    while (!pool.empty()) {
        int m = min((int)pool.size(),leafBits+1);
        if (m == leafBits+1 && n == (1 << leafBits)-1) --m;
        vector<vector<LeafEntry>> current(m);
        weights.clear();
        int mid = 0;
        long long base = 0;
        for (int j = 0; j < m; ++j) {
            current[j] = move(pool.back());
            pool.pop_back();
            const auto& e = current[j][0];
            int w = min(1 << j,n-mid);
            mid += w;
            weights.emplace_back(e.v,w);
            base += 1LL*w*e.z;
        }
        int top = weights.back().second;
        int low = (1 << (m-1))-1;
        long long result = ask(weights)-base;
        if (result < 0 || result%2) exit(0);
        int s = int(result/2);
        int x = s <= low ? s : -1;
        int y = (s >= top && s-top <= low) ? (s-top) | (1 << (m-1)) : -1;
        if (x < 0) swap(x,y);
        if (x < 0) exit(0);

        vector<LeafEntry> pending;
        for (int j = 0; j < m; ++j) {
            bool a = x >> j & 1;
            bool b = y < 0 ? a : (y >> j & 1);
            for (const auto& e : current[j]) {
                int z = a ? e.o : e.z, o = b ? e.o : e.z;
                if (z == o) f[e.v] = z;
                else pending.push_back({e.v,z,o});
            }
        }
        if (!pending.empty()) {
            if (pending[0].z > pending[0].o)
                for (auto& e : pending) swap(e.z,e.o);
            pool.push_back(move(pending));
        }
    }
}

void learnPathSums() {
    const auto& v = bestPlan.order;
    const auto& par = bestPlan.par;
    int b = bestPlan.root, m = bestPlan.initial;
    vector<int> pos(n,-1);
    for (int i = 0; i < n-1; ++i) pos[v[i]] = i;
    int k = m ? (1 << m)+1 : 1;
    int parts = 0;
    array<int,M> wt{};
    for (int i = 0; i < m; ++i) wt[i] = 1 << (m-1-i);
    for (int i = 0; i < m; ++i) {
        int p = par[v[i]], w = 1 << (m-1-i);
        if (p == b) parts += w;
        else wt[pos[p]] -= w;
    }
    vector<pair<int,int>> query{{b,k-parts}};
    for (int i = 0; i < m; ++i) if (wt[i]) query.emplace_back(v[i],wt[i]);
    long long res = ask(query);
    int S = (1 << m)-1, mask = -1;
    for (int x = -n; x <= n; ++x) {
        long long d = res - 1LL*k*x + S;
        if (0 <= d && d <= 2*S && d%2 == 0) {
            f[b] = x;
            mask = int(d/2);
            break;
        }
    }
    if (mask < 0) exit(0);
    for (int i = 0; i < m; ++i)
        f[v[i]] = f[par[v[i]]] + ((mask >> (m-1-i) & 1) ? 1 : -1);

    for (int i = m; i < n-1; ) {
        m = bestPlan.step[i];
        if (m == -1) {
            solveLeaves(v,par,i);
            break;
        }
        Pack pk = makePack(v,pos,par,i,m);
        if (!pk.code) exit(0);
        const Code& code = *pk.code;
        query.clear();
        long long base = 0;
        for (int j = 0; j < m; ++j) {
            int x = v[i+pk.index[j]];
            int p = par[x];
            if (pos[p] < i) base += 1LL*code.p[j]*f[p];
        }
        for (int j = 0; j < m; ++j)
            if (pk.weight[j]) query.emplace_back(v[i+j],pk.weight[j]);
        long long d = ask(query) - base + code.sum;
        if (d < 0 || d > 2*code.sum || d%2 || (mask = code.mask[d/2]) < 0) exit(0);
        array<int,M> delta{};
        for (int j = 0; j < m; ++j) delta[pk.index[j]] = (mask >> j & 1) ? 1 : -1;
        for (int j = 0; j < m; ++j) {
            int x = v[i+j];
            f[x] = f[par[x]] + delta[j];
        }
        i += m;
    }
}

vector<vector<short>> hop;
vector<vector<int8_t>> val;
vector<int> id, parity;

struct RootQuery {
    int c = -1, k = 0;
    vector<int> count;
    tuple<int,long long,int,int,int> score{INT_MAX,LLONG_MAX,INT_MAX,INT_MAX,INT_MAX};
};

RootQuery chooseRootQuery(const vector<int>& cand, int ongoing, int oldParity) {
    RootQuery best;
    vector<int> classSize(n);
    for (int r : cand) ++classSize[id[r]];

    for (int c = ongoing < 0 ? 0 : ongoing; c < (ongoing < 0 ? n : ongoing+1); ++c) {
        int mul = ongoing < 0 ? parity[c] : oldParity;
        struct Item { int h, sign, mass, size; };
        vector<Item> items;
        vector<int> lookup(n,-1), itemOf;
        array<vector<int>,2> side;
        int pivotItem = -1;
        for (int r : cand) {
            int h = hop[c][r];
            int j = lookup[h];
            if (j == -1) {
                j = lookup[h] = (int)items.size();
                int s = val[r][c]*mul > 0;
                items.push_back({h,s,0,0});
                side[s].push_back(j);
                if (h == c) pivotItem = j;
            }
            itemOf.push_back(j);
            items[j].mass += 100000/classSize[id[r]];
            ++items[j].size;
        }
        if (items.size() <= 1) continue;
        for (auto& v : side) sort(v.begin(),v.end(),[&](int a, int b) {
            return tie(items[a].mass,items[a].size) > tie(items[b].mass,items[b].size);
        });
        int largest = max(side[0].size(),side[1].size());
        vector<int> tries{1,2,3,4,6,8,12,16,24,32,45,64};
        tries.push_back(min(64,largest));
        sort(tries.begin(),tries.end());
        tries.erase(unique(tries.begin(),tries.end()),tries.end());

        vector<int> code(items.size()), count(n);
        for (int limit : tries) {
            if (limit > largest) break;
            for (int separate = 0; separate <= (pivotItem >= 0 && limit > 1); ++separate) {
                fill(code.begin(),code.end(),0);
                int total = 0, maxWeight = 0;
                for (int s = 0; s < 2; ++s) {
                    const auto& ls = side[s];
                    if (ls.empty()) continue;
                    int bins = min(limit,(int)ls.size());
                    vector<int> binOf(items.size(),-1);
                    vector<int> mass(bins), cnt(bins);
                    bool pivotHere = pivotItem >= 0 && items[pivotItem].sign == s;
                    if (pivotHere) {
                        binOf[pivotItem] = 0;
                        mass[0] = items[pivotItem].mass;
                        cnt[0] = 1;
                    }
                    int start = separate && pivotHere && bins > 1;
                    for (int j : ls) if (j != pivotItem) {
                        int pick = start;
                        for (int u = start+1; u < bins; ++u)
                            if (tie(mass[u],cnt[u]) < tie(mass[pick],cnt[pick])) pick = u;
                        binOf[j] = pick;
                        mass[pick] += items[j].mass;
                        ++cnt[pick];
                    }
                    vector<int> ord(bins), perm(bins);
                    iota(ord.begin(),ord.end(),0);
                    sort(ord.begin()+int(pivotHere),ord.end(),[&](int a, int b) {
                        return cnt[a] > cnt[b];
                    });
                    for (int j = 0, label = 0; j < bins; ++j) if (cnt[ord[j]]) {
                        perm[ord[j]] = label++;
                    }
                    for (int j : ls) {
                        int w = perm[binOf[j]];
                        code[j] = w;
                        if (j != pivotItem) total += w;
                        maxWeight = max(maxWeight,w);
                    }
                }
                int k = max(total,maxWeight+1);
                if (k > n) continue;

                int groups[128] = {}, distinct[128] = {}, groupItems[128] = {};
                vector<bitset<128>> seen(n);
                int cmax = 0, smax = 0;
                for (int j = 0; j < (int)items.size(); ++j)
                    ++groupItems[64*items[j].sign+code[j]];
                for (int i = 0; i < (int)cand.size(); ++i) {
                    int j = itemOf[i];
                    int z = 64*items[j].sign+code[j], cl = id[cand[i]];
                    smax = max(smax,++groups[z]);
                    if (!seen[cl][z]) {
                        seen[cl][z] = true;
                        cmax = max(cmax,++distinct[z]);
                    }
                }
                if (smax == (int)cand.size()) continue;
                long long residual = 0;
                for (int z = 0; z < 128; ++z)
                    if (distinct[z] > 1)
                        residual += 1LL*groups[z]*
                                    (4*(distinct[z]-1) + (groupItems[z] > 1));
                auto score = make_tuple(cmax,residual,smax,k,c);
                if (score < best.score) {
                    fill(count.begin(),count.end(),0);
                    for (int j = 0; j < (int)items.size(); ++j)
                        if (j != pivotItem) count[items[j].h] = code[j];
                    count[c] = k-total;
                    best = {c,k,count,score};
                }
            }
        }
    }
    return best;
}

void recoverValues() {
    vector<int> cand;
    for (int r = 0; r < n; ++r) if (abs(f[r]) == 1) cand.push_back(r);
    if (cand.empty()) exit(0);

    hop.assign(n, vector<short>(n));
    for (int c = 0; c < n; ++c) {
        hop[c][c] = c;
        auto dfs = [&](auto&& self, int v, int p, int head) -> void {
            hop[c][v] = head;
            for (int w : tree[v]) if (w != p) self(self,w,v,head);
        };
        for (int w : tree[c]) dfs(dfs,w,c,w);
    }
    val.resize(n);
    for (int r : cand) {
        val[r].resize(n);
        for (int v = 0; v < n; ++v)
            val[r][v] = v == r ? f[v] : f[v]-f[hop[v][r]];
    }

    vector<int> sorted = cand;
    sort(sorted.begin(),sorted.end(),[](int a, int b) { return val[a] < val[b]; });
    id.assign(n,-1);
    int cls = -1, last = -1;
    for (int r : sorted) {
        if (last < 0 || val[r] != val[last]) ++cls;
        id[r] = cls;
        last = r;
    }
    parity.assign(n,1);
    int ongoing = -1, oldParity = 1;
    while (true) {
        int r0 = cand.front();
        bool done = true;
        for (int r : cand) if (id[r] != id[r0]) done = false;
        if (done) {
            vector<int> a(n);
            for (int v = 0; v < n; ++v) a[v] = parity[v]*val[r0][v];
            answer(a);
            return;
        }

        RootQuery rq = chooseRootQuery(cand,ongoing,oldParity);
        if (rq.c < 0) exit(0);
        int c = rq.c;
        if (ongoing < 0) {
            ongoing = c;
            oldParity = parity[c];
            flip(c);
            parity[c] = -parity[c];
        }
        vector<pair<int,int>> query;
        long long base = 0;
        for (int v = 0; v < n; ++v) if (rq.count[v]) {
            query.emplace_back(v,rq.count[v]);
            base += 1LL*rq.count[v]*f[v];
        }
        long long diff = ask(query)-base;
        vector<int> next;
        for (int r : cand) {
            int h = hop[c][r];
            int except = h == c ? 0 : rq.count[h];
            if (diff == -2LL*oldParity*val[r][c]*(rq.k-except))
                next.push_back(r);
        }
        if (next.empty()) exit(0);
        cand = move(next);
        int h = hop[c][cand.front()];
        bool same = true;
        for (int r : cand) if (hop[c][r] != h) same = false;
        if (same) {
            int change = -2*oldParity*val[cand.front()][c];
            for (int v = 0; v < n; ++v)
                if (v == c || hop[c][v] != h) f[v] += change;
            ongoing = -1;
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n) || n < 2 || n > 1000) return 0;
    tree.resize(n);
    for (int i = 1; i < n; ++i) {
        int u, v;
        if (!(cin >> u >> v)) return 0;
        --u; --v;
        tree[u].push_back(v);
        tree[v].push_back(u);
    }
    if (n == 2) {
        long long s = ask({{0,1},{1,1}});
        int a = s > 0 ? 1 : -1;
        if (abs(s) == 3) {
            answer({a,a});
        } else if (abs(s) == 1) {
            long long x = ask({{0,1}});
            answer(x % 2 ? vector<int>{a,-a} : vector<int>{-a,a});
        }
        return 0;
    }
    f.resize(n);
    planQueries();
    learnPathSums();
    recoverValues();
}
