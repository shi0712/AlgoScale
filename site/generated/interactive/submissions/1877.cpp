#include <bits/stdc++.h>
using namespace std;

using Byte = unsigned char;

static double binom[501][501], estimate[501][501];
static short splitAt[501][501];

struct Matrix {
    int capacity = 0, a = 0, b = 0;
    vector<vector<Byte>> row;
};

struct Plan {
    int rows = 0;
    vector<int> reference;
    vector<Byte> inverted;
    vector<vector<int>> subsets;
};

class Solver {
    int n, m;
    uint64_t rng;
    vector<unordered_map<string, int>> cache;
    string inward;
    vector<int> depth, hint, from, to;
    vector<Byte> used, marker;
    vector<vector<int>> rootPath;
    vector<Matrix> matrices;
    vector<Plan> plans;
    vector<double> chainCost;
    vector<int> chainSplit;

    uint32_t randomWord() {
        rng ^= rng << 13;
        rng ^= rng >> 7;
        rng ^= rng << 17;
        return uint32_t(rng >> 16);
    }

    void shuffleList(vector<int>& a) {
        for (int i = int(a.size()) - 1; i > 0; --i)
            swap(a[i], a[randomWord() % (i + 1)]);
    }

    int ask(int v, const string& b) {
        string key = b;
        bool inv = key[0] == '1';
        if (inv) for (char& c : key) c ^= 1;
        auto it = cache[v].find(key);
        if (it != cache[v].end()) return inv ? m - it->second : it->second;
        cout << "? " << v + 1 << ' ' << b << endl;
        int answer;
        if (!(cin >> answer) || answer < 0 || answer > m) exit(0);
        cache[v].emplace(move(key), inv ? m - answer : answer);
        return answer;
    }

    void finish() {
        cout << '!';
        for (int e = 0; e < m; ++e)
            cout << ' ' << from[e] + 1 << ' ' << to[e] + 1;
        cout << endl;
    }

    void smallInstance() {
        vector<pair<int, int>> edges;
        for (int a = 0; a < n; ++a)
            for (int b = a + 1; b < n; ++b) edges.emplace_back(a, b);
        struct State {
            array<Byte, 4> edge{};
            array<Byte, 5> match{};
            int reversed;
        };
        vector<State> states;
        for (int set = 0; set < (1 << edges.size()); ++set) {
            if (__builtin_popcount(unsigned(set)) != m) continue;
            vector<int> perm;
            vector<vector<int>> adj(n);
            for (int i = 0; i < int(edges.size()); ++i) if (set >> i & 1) {
                perm.push_back(i);
                auto [a, b] = edges[i];
                adj[a].push_back(b);
                adj[b].push_back(a);
            }
            int d[5][5];
            bool connected = true;
            for (int r = 0; r < n; ++r) {
                fill(d[r], d[r] + n, -1);
                int q[5], sz = 1;
                q[0] = r;
                d[r][r] = 0;
                for (int i = 0; i < sz; ++i)
                    for (int v : adj[q[i]]) if (d[r][v] < 0) {
                        d[r][v] = d[r][q[i]] + 1;
                        q[sz++] = v;
                    }
                if (sz != n) { connected = false; break; }
            }
            if (!connected) continue;
            do {
                State s{};
                for (int i = 0; i < m; ++i) {
                    s.edge[i] = perm[i];
                    auto [a, b] = edges[perm[i]];
                    for (int r = 0; r < n; ++r)
                        if (d[r][b] < d[r][a]) s.match[r] |= 1 << i;
                }
                auto base = s.match;
                for (int f = 0; f < (1 << m); ++f) {
                    s.reversed = f;
                    for (int r = 0; r < n; ++r) s.match[r] = base[r] ^ f;
                    states.push_back(s);
                }
            } while (next_permutation(perm.begin(), perm.end()));
        }
        while (states.size() > 1) {
            long long best = LLONG_MAX;
            int br = 0, bb = 0, bw = INT_MAX;
            for (int r = 0; r < n; ++r) for (int b = 0; b < (1 << m); ++b) {
                int h[5]{};
                for (const auto& s : states)
                    ++h[__builtin_popcount(unsigned(s.match[r] ^ b))];
                long long score = 0;
                int worst = 0;
                for (int x : h) {
                    score += 1LL * x * x;
                    worst = max(worst, x);
                }
                if (score < best || (score == best && worst < bw)) {
                    best = score;
                    bw = worst;
                    br = r;
                    bb = b;
                }
            }
            string b(m, '0');
            for (int i = 0; i < m; ++i) b[i] += (bb >> i) & 1;
            int ans = m - ask(br, b);
            erase_if(states, [&](const State& s) {
                return __builtin_popcount(unsigned(s.match[br] ^ bb)) != ans;
            });
            if (states.empty()) exit(0);
        }
        for (int e = 0; e < m; ++e) {
            auto [a, b] = edges[states[0].edge[e]];
            if (states[0].reversed >> e & 1) swap(a, b);
            from[e] = a;
            to[e] = b;
        }
        finish();
    }

    void prepare() {
        matrices.resize(2);
        matrices[1].capacity = 1;
        matrices[1].row = {{1}};
        for (int r = 2; matrices.back().capacity < m; ++r) {
            Matrix mat;
            for (int b = 1; b <= r / 2; ++b) {
                int a = r - b;
                int cap = matrices[a].capacity + matrices[b].capacity + b - 1;
                if (cap > mat.capacity) {
                    mat.capacity = cap;
                    mat.a = a;
                    mat.b = b;
                }
            }
            int a = mat.a, b = mat.b;
            const auto& A = matrices[a];
            const auto& B = matrices[b];
            int x = A.capacity, y = B.capacity;
            mat.row.assign(r, vector<Byte>(mat.capacity));
            for (int i = 0; i < a; ++i)
                copy(A.row[i].begin(), A.row[i].end(), mat.row[i].begin());
            for (int i = 0; i < b; ++i) {
                copy(A.row[i].begin(), A.row[i].end(), mat.row[a+i].begin());
                for (int j = 0; j < y; ++j) {
                    mat.row[i][x+j] = B.row[i][j];
                    mat.row[a+i][x+j] = 1 - B.row[i][j];
                }
            }
            for (int i = 1; i < b; ++i) {
                int j = x + y + i - 1;
                mat.row[0][j] = mat.row[a][j] = mat.row[i][j] = 1;
            }
            matrices.push_back(move(mat));
        }

        plans.resize(m + 1);
        int r = 1;
        for (int len = 1; len <= m; ++len) {
            while (matrices[r].capacity < len) ++r;
            auto& p = plans[len];
            p.rows = r;
            p.reference.resize(r);
            p.inverted.resize(r);
            unordered_map<string, int> index;
            index.emplace(string(len, '0'), -1);
            for (int i = 0; i < r; ++i) {
                auto& row = matrices[r].row[i];
                p.inverted[i] = row[0];
                string key(len, '0');
                for (int j = 0; j < len; ++j) key[j] += row[j] ^ row[0];
                auto [it, fresh] = index.try_emplace(key, int(p.subsets.size()));
                if (fresh) {
                    vector<int> sub;
                    for (int j = 0; j < len; ++j) if (key[j] == '1') sub.push_back(j);
                    p.subsets.push_back(move(sub));
                }
                p.reference[i] = it->second;
            }
        }

        for (int i = 0; i <= m; ++i) {
            binom[i][0] = binom[i][i] = 1;
            for (int j = 1; j < i; ++j) binom[i][j] = binom[i-1][j-1] + binom[i-1][j];
        }
        for (int len = 2; len <= m; ++len) {
            vector<int> splits;
            if (len <= 25) {
                for (int a = 1; a <= len / 2; ++a) splits.push_back(a);
            } else {
                splits = {1, len/8, len/5, len/4, len/3, 2*len/5, len/2, len/2-1};
                int p = 1;
                while (p * 2 <= len / 2) p *= 2;
                splits.push_back(p);
                sort(splits.begin(), splits.end());
                splits.erase(unique(splits.begin(), splits.end()), splits.end());
            }
            for (int k = 1; k <= len / 2; ++k) {
                double best = plans[len].subsets.size();
                int sa = 0;
                for (int a : splits) {
                    int b = len - a;
                    double sum = 0;
                    for (int i = max(0, k-b); i <= min(a, k); ++i)
                        sum += binom[a][i] * binom[b][k-i] *
                               (estimate[a][i] + estimate[b][k-i]);
                    double now = 1 + sum / binom[len][k];
                    if (now < best - 1e-9) {
                        best = now;
                        sa = a;
                    }
                }
                estimate[len][k] = estimate[len][len-k] = best;
                splitAt[len][k] = splitAt[len][len-k] = sa;
            }
        }
        chainCost.assign(m + 1, 0);
        chainSplit.assign(m + 1, 0);
        for (int len = 2; len <= m; ++len) {
            double best = 1e30;
            for (int k = len / 2; k >= 1; --k) {
                double cost = chainCost[k] + chainCost[len-k] + estimate[len][k];
                if (cost < best - 1e-9) {
                    best = cost;
                    chainSplit[len] = k;
                }
            }
            chainCost[len] = best;
        }
    }

    void decode(int r, const vector<int>& vals, vector<Byte>& result, int start) {
        if (r == 1) {
            result[start] = vals[0];
            return;
        }
        const auto& mat = matrices[r];
        int a = mat.a, b = mat.b;
        int xs = matrices[a].capacity, ys = matrices[b].capacity;
        vector<int> x(a), y(b);
        y[0] = vals[0] - vals[a];
        int sumZ = 0;
        for (int i = 1; i < b; ++i) {
            int z = (vals[i] + vals[a+i] - y[0]) & 1;
            result[start+xs+ys+i-1] = z;
            sumZ += z;
            x[i] = (vals[i] + vals[a+i] - y[0] - z) / 2;
            y[i] = y[0] + x[i] - vals[a+i];
        }
        x[0] = vals[a] - sumZ;
        for (int i = b; i < a; ++i) x[i] = vals[i];
        decode(a, x, result, start);
        decode(b, y, result, start + xs);
    }

    using Oracle = function<int(const vector<int>&)>;

    vector<int> recover(const vector<int>& u, int k, const Oracle& count) {
        int len = u.size();
        if (k == 0) return {};
        if (k == len) return u;
        int a = splitAt[len][k];
        if (a) {
            vector<int> left(u.begin(), u.begin()+a), right(u.begin()+a, u.end());
            int kl = count(left);
            left = recover(left, kl, count);
            right = recover(right, k-kl, count);
            left.insert(left.end(), right.begin(), right.end());
            return left;
        }
        const Plan& p = plans[len];
        vector<int> got(p.subsets.size()), vals(p.rows);
        vector<int> query;
        for (int i = 0; i < int(p.subsets.size()); ++i) {
            query.clear();
            for (int j : p.subsets[i]) query.push_back(u[j]);
            got[i] = count(query);
        }
        for (int i = 0; i < p.rows; ++i) {
            int value = p.reference[i] < 0 ? 0 : got[p.reference[i]];
            vals[i] = p.inverted[i] ? k - value : value;
        }
        vector<Byte> result(matrices[p.rows].capacity);
        decode(p.rows, vals, result, 0);
        vector<int> out;
        out.reserve(k);
        for (int j = 0; j < len; ++j) if (result[j]) out.push_back(u[j]);
        return out;
    }

    vector<int> freeEdges() const {
        vector<int> u;
        for (int e = 0; e < m; ++e) if (!used[e]) u.push_back(e);
        return u;
    }

    int countPath(int v, const vector<int>& sub) {
        if (sub.empty()) return 0;
        string b = inward;
        for (int e : sub) b[e] ^= 1;
        return (ask(v, b) - m + depth[v] + int(sub.size())) / 2;
    }

    int countMask(int v, const vector<Byte>& mask) {
        string b = inward;
        int sz = 0;
        for (int e = 0; e < m; ++e) if (mask[e]) {
            b[e] ^= 1;
            ++sz;
        }
        return (ask(v, b) - m + depth[v] + sz) / 2;
    }

    int marked(const vector<int>& edges) const {
        int k = 0;
        for (int e : edges) k += marker[e];
        return k;
    }

    vector<int> findPathPart(int v, const vector<int>& u, int k, int prefixMarkers) {
        if (k == 0) return {};
        if (k == int(u.size())) return u;
        Oracle oracle = [&](const vector<int>& sub) { return countPath(v, sub); };
        if (hint[v] >= 0) {
            vector<int> part[2];
            for (int e : u) part[marker[e]].push_back(e);
            int ones = hint[v] - prefixMarkers, zeros = k - ones;
            if (estimate[part[1].size()][ones] + estimate[part[0].size()][zeros]
                <= estimate[u.size()][k] + 1e-9) {
                auto a = recover(part[0], zeros, oracle);
                auto b = recover(part[1], ones, oracle);
                a.insert(a.end(), b.begin(), b.end());
                return a;
            }
        }
        return recover(u, k, oracle);
    }

    void addEdge(int v, int p, int e) {
        if (inward[e] == '1') {
            from[e] = v;
            to[e] = p;
        } else {
            from[e] = p;
            to[e] = v;
        }
        used[e] = 1;
        rootPath[v] = rootPath[p];
        rootPath[v].push_back(e);
    }

    int identifyLabel(int v, int parent, vector<int> cand) {
        while (cand.size() > 1) {
            int k = int(cand.size()) / 2;
            vector<int> sub(cand.begin(), cand.begin()+k);
            // The tested labels are unused and cannot be on the parent's path.
            if (countPath(v, sub)) cand.resize(k);
            else cand.erase(cand.begin(), cand.begin()+k);
        }
        int e = cand[0];
        addEdge(v, parent, e);
        return e;
    }

    vector<Byte> balance(const vector<int>& w) {
        vector<int> order;
        for (int i = 0; i < int(w.size()); ++i) if (w[i]) order.push_back(i);
        stable_sort(order.begin(), order.end(), [&](int a, int b) { return w[a] > w[b]; });
        vector<Byte> flags(w.size());
        int sums[2]{};
        for (int x : order) {
            int b = (sums[1] < sums[0]);
            flags[x] = b;
            sums[b] += w[x];
        }
        return flags;
    }

    void identify(int v, const vector<int>& parents) {
        auto free = freeEdges();
        vector<int> cand;
        cand.reserve(free.size() * parents.size());
        for (int p = 0; p < int(parents.size()); ++p) {
            int baseHint = hint[v] >= 0 ? marked(rootPath[parents[p]]) : 0;
            for (int e : free)
                if (hint[v] < 0 || baseHint + marker[e] == hint[v])
                    cand.push_back((p << 9) | e);
        }

        const int P = parents.size(), D = depth[v] - 1;
        vector<int> incoming(P, -1);
        for (int p = 0; p < P; ++p)
            if (D) incoming[p] = rootPath[parents[p]].back();

        while (cand.size() > 1) {
            if ((cand.front() >> 9) == (cand.back() >> 9)) {
                vector<int> labels;
                for (int t : cand) labels.push_back(t & 511);
                identifyLabel(v, parents[cand.front() >> 9], move(labels));
                return;
            }
            vector<int> wp(P), we(m), activeP, activeE;
            for (int t : cand) {
                ++wp[t >> 9];
                ++we[t & 511];
            }
            for (int p = 0; p < P; ++p) if (wp[p]) activeP.push_back(p);
            for (int e : free) if (we[e]) activeE.push_back(e);
            auto bp = balance(wp), be = balance(we);
            vector<Byte> mask(m), bestMask;
            vector<int> offset(P), bestOff, hist(D+2), bestHist;
            long long best = LLONG_MAX;
            int bestWorst = INT_MAX;
            const int target = cand.size();

            auto consider = [&]() {
                fill(hist.begin(), hist.end(), 0);
                for (int p : activeP) {
                    int x = 0;
                    for (int e : rootPath[parents[p]]) x += mask[e];
                    offset[p] = x;
                }
                for (int t : cand) ++hist[offset[t >> 9] + mask[t & 511]];
                long long score = 0;
                int worst = 0;
                for (int x : hist) {
                    score += 1LL * x * x;
                    worst = max(worst, x);
                }
                if (score < best || (score == best && worst < bestWorst)) {
                    best = score;
                    bestWorst = worst;
                    bestMask = mask;
                    bestOff = offset;
                    bestHist = hist;
                }
            };

            for (int p : activeP) mask[incoming[p]] = bp[p];
            consider();
            if (best > target && activeE.size() > 1) {
                fill(mask.begin(), mask.end(), 0);
                for (int e : activeE) mask[e] = be[e];
                consider();
                for (int p : activeP) mask[incoming[p]] = bp[p];
                consider();
                for (int e : activeE) mask[e] ^= 1;
                consider();
            }

            int common = 0;
            const auto& first = rootPath[parents[activeP[0]]];
            if (best > target) {
                while (common < D) {
                    bool same = true;
                    for (int p : activeP)
                        if (rootPath[parents[p]][common] != first[common]) {
                            same = false;
                            break;
                        }
                    if (!same) break;
                    ++common;
                }
            }

            if (best > target && D-common >= 2) {
                array<int, 4> pivots{common, common+(D-common)/3,
                                     common+2*(D-common)/3, D-2};
                for (int h : pivots) {
                    if (h < common || best == target) continue;
                    vector<int> groupWeight(m), groups, value(m, -1);
                    for (int p : activeP)
                        groupWeight[rootPath[parents[p]][h]] += wp[p];
                    for (int e = 0; e < m; ++e) if (groupWeight[e]) groups.push_back(e);
                    sort(groups.begin(), groups.end(), [&](int a, int b) {
                        return groupWeight[a] > groupWeight[b];
                    });
                    vector<int> load(D-h+1);
                    for (int e : groups) {
                        int j = int(min_element(load.begin(), load.end()) - load.begin());
                        value[e] = j;
                        load[j] += groupWeight[e];
                    }
                    fill(mask.begin(), mask.end(), 0);
                    for (int p : activeP) {
                        const auto& path = rootPath[parents[p]];
                        for (int j = h; j < h+value[path[h]]; ++j) mask[path[j]] = 1;
                    }
                    for (int e : activeE) mask[e] = be[e];
                    consider();
                }
            }

            int trials = target > 12000 ? 7 : 12;
            for (int it = 0; it < trials && best > target; ++it) {
                if (it < 3) {
                    fill(mask.begin(), mask.end(), 0);
                    for (int p : activeP) mask[incoming[p]] = randomWord() & 1;
                } else {
                    uint32_t bits = 0;
                    for (int e = 0; e < m; ++e) {
                        if ((e & 31) == 0) bits = randomWord();
                        mask[e] = bits & 1;
                        bits >>= 1;
                    }
                }
                for (int e : activeE) mask[e] = it % 3 == 0 ? be[e] : (randomWord() & 1);
                consider();
            }

            // The incoming edges of the candidate parents are independent controls:
            // all the parents have equal depth. Unused labels are independent too.
            if (best > target && target > 2) {
                vector<vector<int>> byEdge(m), byParent(P);
                for (int e : activeE) byEdge[e].reserve(we[e]);
                for (int p : activeP) byParent[p].reserve(wp[p]);
                for (int t : cand) {
                    byEdge[t & 511].push_back(t >> 9);
                    byParent[t >> 9].push_back(t & 511);
                }
                vector<int> delta(D+2), stamp(D+2), touched;
                int tick = 0;
                auto tryMove = [&](int a, int b, int amount) {
                    if (!amount) return;
                    if (stamp[a] != tick) {
                        stamp[a] = tick;
                        delta[a] = 0;
                        touched.push_back(a);
                    }
                    if (stamp[b] != tick) {
                        stamp[b] = tick;
                        delta[b] = 0;
                        touched.push_back(b);
                    }
                    delta[a] -= amount;
                    delta[b] += amount;
                };
                auto apply = [&]() {
                    long long change = 0;
                    for (int i : touched)
                        change += 1LL * delta[i] * (2 * bestHist[i] + delta[i]);
                    if (change >= 0) return false;
                    best += change;
                    for (int i : touched) bestHist[i] += delta[i];
                    return true;
                };
                for (int pass = 0; pass < 2 && best > target; ++pass) {
                    auto old = best;
                    for (int p : activeP) {
                        int e = incoming[p], step = bestMask[e] ? -1 : 1;
                        int a = bestOff[p], one = 0;
                        for (int f : byParent[p]) one += bestMask[f];
                        ++tick;
                        touched.clear();
                        tryMove(a, a+step, wp[p]-one);
                        tryMove(a+1, a+1+step, one);
                        if (apply()) {
                            bestOff[p] += step;
                            bestMask[e] ^= 1;
                        }
                    }
                    for (int e : activeE) {
                        int flag = bestMask[e];
                        ++tick;
                        touched.clear();
                        for (int p : byEdge[e]) {
                            int a = bestOff[p];
                            tryMove(a+flag, a+1-flag, 1);
                        }
                        if (apply()) bestMask[e] ^= 1;
                    }
                    if (best == old) break;
                }
            }

            int answer = countMask(v, bestMask);
            erase_if(cand, [&](int t) {
                return bestOff[t >> 9] + bestMask[t & 511] != answer;
            });
        }
        if (cand.empty()) exit(0);
        int t = cand[0];
        addEdge(v, parents[t >> 9], t & 511);
    }

    void orderPath(const vector<int>& verts, int lo, int hi, const vector<int>& labels,
                   int previousMarkers) {
        if (hi == lo + 1) {
            addEdge(verts[hi], verts[lo], labels[0]);
            return;
        }
        int k = chainSplit[hi-lo], mid = lo + k;
        auto left = findPathPart(verts[mid], labels, k, previousMarkers);
        vector<Byte> inLeft(m);
        for (int e : left) inLeft[e] = 1;
        vector<int> right;
        for (int e : labels) if (!inLeft[e]) right.push_back(e);
        orderPath(verts, lo, mid, left, previousMarkers);
        orderPath(verts, mid, hi, right, previousMarkers + marked(left));
    }

    double binaryCost(int len) const {
        int p = 1, lg = 0;
        while (p < len) { p *= 2; ++lg; }
        return lg - double(p-len) / len;
    }

    void solveGroup(int root, vector<int> verts) {
        if (verts.empty()) return;
        int L = 0;
        for (int v : verts) L = max(L, depth[v]-depth[root]);
        vector<vector<int>> levels(L+1);
        levels[0] = {root};
        for (int v : verts) levels[depth[v]-depth[root]].push_back(v);

        auto labels = freeEdges();
        int U = labels.size(), prefixMarkers = marked(rootPath[root]);
        double projections = 0, naive = 0;
        for (int d = 1; d <= L; ++d) {
            naive += binaryCost(U-d+1);
            int t = levels[d].size();
            if (t <= 1) continue;
            if (d == L) {
                if (levels[d-1].size() > 1) projections += t-1;
            } else if (levels[d-1].size() == 1) {
                projections += (t+1)*0.5 - 1.0/t;
            } else {
                projections += t-1.0/t;
            }
        }
        int end = levels[L][0];
        double setCost = estimate[U][L];
        if (hint[end] >= 0) {
            int u1 = marked(labels), k1 = hint[end]-prefixMarkers;
            setCost = min(setCost, estimate[u1][k1] + estimate[U-u1][L-k1]);
        }
        if (L >= 3 && setCost + chainCost[L] + projections + 0.15 < naive) {
            auto pathLabels = findPathPart(end, labels, L, prefixMarkers);
            vector<int> pathVertices(L+1);
            vector<vector<int>> groups(L);
            pathVertices[0] = root;
            bool previousHasOther = false;
            for (int d = 1; d <= L; ++d) {
                int p = (d == L ? end : -1);
                int remaining = levels[d].size() - (p != -1);
                bool hasOther = false;
                for (int v : levels[d]) {
                    if (v == p) continue;
                    int j;
                    if (p == -1 && remaining == 1) j = d;
                    else if (p != -1 && !previousHasOther) j = d-1;
                    else j = countPath(v, pathLabels);
                    --remaining;
                    if (j == d) p = v;
                    else {
                        groups[j].push_back(v);
                        hasOther = true;
                    }
                }
                pathVertices[d] = p;
                previousHasOther = hasOther;
            }
            orderPath(pathVertices, 0, L, pathLabels, prefixMarkers);
            vector<int> order(L);
            iota(order.begin(), order.end(), 0);
            stable_sort(order.begin(), order.end(), [&](int a, int b) {
                return groups[a].size() > groups[b].size();
            });
            for (int j : order) if (!groups[j].empty())
                solveGroup(pathVertices[j], move(groups[j]));
        } else {
            for (int d = 1; d <= L; ++d) {
                stable_partition(levels[d].begin(), levels[d].end(),
                                 [&](int v) { return hint[v] >= 0; });
                for (int v : levels[d]) identify(v, levels[d-1]);
            }
        }
    }

    void solveStar(int center, const vector<int>& potential) {
        int stem = -1;
        if (center != 0) {
            depth[center] = 1;
            int flag = (1 - potential[center]) / 2;
            vector<int> cand;
            for (int e = 0; e < m; ++e) if (marker[e] == flag) cand.push_back(e);
            stem = identifyLabel(center, 0, move(cand));
        }
        for (int v = 1; v < n; ++v) if (v != center) {
            depth[v] = depth[center]+1;
            int flag = (1 - potential[v] + potential[center]) / 2;
            vector<int> cand;
            for (int e = 0; e < m; ++e) if (e != stem && !used[e] && marker[e] == flag)
                cand.push_back(e);
            identifyLabel(v, center, move(cand));
        }
    }

public:
    explicit Solver(int size) : n(size), m(n-1),
        rng(uint64_t(chrono::steady_clock::now().time_since_epoch().count()) | 1),
        cache(n), inward(m, '0'), depth(n), hint(n, -1), from(m), to(m),
        used(m), marker(m), rootPath(n) {}

    void run() {
        if (n <= 5) {
            smallInstance();
            return;
        }

        int q0 = ask(0, string(m, '0'));
        prepare();
        vector<int> all(m);
        iota(all.begin(), all.end(), 0);
        auto ones = recover(all, m-q0, [&](const vector<int>& sub) {
            string b(m, '0');
            for (int e : sub) b[e] = '1';
            return (ask(0, b) - q0 + int(sub.size())) / 2;
        });
        for (int e : ones) inward[e] = '1';

        shuffleList(all);
        string mixed = inward;
        int markedCount = m/2;
        for (int i = 0; i < markedCount; ++i) {
            marker[all[i]] = 1;
            mixed[all[i]] ^= 1;
        }

        vector<int> vertices(n-1), potential(n, INT_MAX);
        iota(vertices.begin(), vertices.end(), 1);
        shuffleList(vertices);
        potential[0] = 0;
        int odd = 0, centerCandidate = -1, evenSign = 0;
        bool starPossible = true;
        for (int v : vertices) {
            int w = potential[v] = m - markedCount - ask(v, mixed);
            if (abs(w) > 2) { starPossible = false; break; }
            if (abs(w) == 1) {
                ++odd;
                centerCandidate = v;
            } else if (w != 0) {
                if (evenSign && evenSign != w/2) { starPossible = false; break; }
                evenSign = w/2;
            }

            // The root is already one member of the even color class.
            // If a second even vertex and two odd vertices have appeared,
            // neither color class can be the singleton of a star.
            static_cast<void>(centerCandidate);
            int processedEven = 0;
            for (int x : vertices)
                if (potential[x] != INT_MAX && !(abs(potential[x]) & 1))
                    ++processedEven;
            if ((processedEven && odd > 1) ||
                (processedEven && odd == 1 && evenSign &&
                 evenSign != potential[centerCandidate])) {
                starPossible = false;
                break;
            }
        }
        if (starPossible) {
            solveStar(odd == 1 ? centerCandidate : 0, potential);
            finish();
            return;
        }

        hint[0] = 0;
        for (int v : vertices) {
            depth[v] = m - ask(v, inward);
            if (potential[v] != INT_MAX) hint[v] = (depth[v] - potential[v]) / 2;
        }
        solveGroup(0, move(vertices));
        finish();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 2 || n > 500) return 0;
    Solver(n).run();
    return 0;
}
