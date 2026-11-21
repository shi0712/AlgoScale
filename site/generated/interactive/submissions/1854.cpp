#include <bits/stdc++.h>
using namespace std;

constexpr int MAXN = 1024;
constexpr int MAXW = MAXN / 64;
using Mask = array<uint64_t, MAXW>;

int n, k, W;
int tastings = 0, resets = 0;

inline bool has(const Mask& a, int x) {
    return (a[x >> 6] >> (x & 63)) & 1;
}
inline void mark(Mask& a, int x) {
    a[x >> 6] |= uint64_t(1) << (x & 63);
}
inline void unmark(Mask& a, int x) {
    a[x >> 6] &= ~(uint64_t(1) << (x & 63));
}
inline bool intersects(const Mask& a, const Mask& b) {
    for (int j = 0; j < W; ++j)
        if (a[j] & b[j]) return true;
    return false;
}
template<class F>
void forBits(const Mask& a, F f) {
    for (int j = 0; j < W; ++j) {
        uint64_t w = a[j];
        while (w) {
            int b = __builtin_ctzll(w);
            w &= w - 1;
            f(64 * j + b);
        }
    }
}

struct Cup {
    int rep;
    Mask options;
};

struct State {
    array<Mask, MAXN> unknown{};
    array<int, MAXN> deg{}, possibleCount{}, certainCount{};
    Mask alive{}, blocked{}, certain{};
    deque<Cup> queue;
    int edges = 0, remaining = 0;

    void init() {
        remaining = n;
        edges = n * (n - 1) / 2;
        for (int x = 0; x < n; ++x) mark(alive, x);
        for (int x = 0; x < n; ++x) {
            unknown[x] = alive;
            unmark(unknown[x], x);
            deg[x] = n - 1;
        }
    }

    void clearMemory() {
        queue.clear();
        possibleCount.fill(0);
        certainCount.fill(0);
        blocked.fill(0);
        certain.fill(0);
    }

    void push(int rep, const Mask& options = {}) {
        if (rep >= 0) {
            if (++possibleCount[rep] == 1) mark(blocked, rep);
            if (++certainCount[rep] == 1) mark(certain, rep);
        } else {
            forBits(options, [&](int x) {
                if (++possibleCount[x] == 1) mark(blocked, x);
            });
        }
        queue.push_back({rep, options});
        if ((int)queue.size() <= k) return;
        const Cup& old = queue.front();
        if (old.rep >= 0) {
            int x = old.rep;
            if (--possibleCount[x] == 0) unmark(blocked, x);
            if (--certainCount[x] == 0) unmark(certain, x);
        } else {
            forBits(old.options, [&](int x) {
                if (--possibleCount[x] == 0) unmark(blocked, x);
            });
        }
        queue.pop_front();
    }

    void negative(int x) {
        for (int j = 0; j < W; ++j) {
            uint64_t w = unknown[x][j] & certain[j];
            unknown[x][j] ^= w;
            while (w) {
                int y = 64 * j + __builtin_ctzll(w);
                w &= w - 1;
                unmark(unknown[y], x);
                --deg[x];
                --deg[y];
                --edges;
            }
        }
        push(x);
    }

    void positive(int x) {
        // No cup in memory can have x as its representative. A match
        // therefore certifies that x can be removed, even if its match
        // is not uniquely identifiable.
        Mask options{};
        int cnt = 0, rep = -1;
        for (int j = 0; j < W; ++j) {
            options[j] = unknown[x][j] & blocked[j];
            if (options[j]) {
                cnt += __builtin_popcountll(options[j]);
                rep = 64 * j + __builtin_ctzll(options[j]);
            }
        }
        if (!cnt) exit(0);
        forBits(unknown[x], [&](int y) {
            unmark(unknown[y], x);
            --deg[y];
        });
        edges -= deg[x];
        deg[x] = 0;
        unknown[x].fill(0);
        unmark(alive, x);
        --remaining;
        if (cnt == 1) push(rep);
        else push(-1, options);
    }

    vector<int> relevant() const {
        vector<int> v;
        for (int x = 0; x < n; ++x)
            if (deg[x]) v.push_back(x);
        return v;
    }
};

bool taste(State& st, int x) {
    cout << "? " << x + 1 << endl;
    ++tastings;
    string answer;
    if (!(cin >> answer)) exit(0);
    if (answer == "N") {
        st.negative(x);
        return false;
    }
    if (answer == "Y") {
        st.positive(x);
        return true;
    }
    exit(0);
}

void resetMemory(State& st) {
    if (st.queue.empty()) return;
    cout << "R" << endl;
    ++resets;
    st.clearMemory();
}

vector<vector<int>> trails(int vertices, const vector<pair<int,int>>& edges,
                           int preferred = -1) {
    int dummy = vertices;
    vector<vector<pair<int,int>>> adj(vertices + 1);
    vector<int> degree(vertices, 0);
    int ids = 0;
    auto add = [&](int u, int v) {
        adj[u].emplace_back(v, ids);
        adj[v].emplace_back(u, ids++);
    };
    for (auto [u,v] : edges) {
        add(u,v);
        ++degree[u];
        ++degree[v];
    }
    for (int u = 0; u < vertices; ++u)
        if (degree[u] & 1) add(dummy, u);

    vector<char> used(ids);
    vector<int> pos(vertices + 1);
    vector<vector<int>> ans;
    auto walk = [&](int root) {
        vector<int> stack{root}, path;
        while (!stack.empty()) {
            int u = stack.back();
            auto& p = pos[u];
            while (p < (int)adj[u].size() && used[adj[u][p].second]) ++p;
            if (p == (int)adj[u].size()) {
                path.push_back(u);
                stack.pop_back();
            } else {
                auto [v,id] = adj[u][p++];
                used[id] = true;
                stack.push_back(v);
            }
        }
        if (path.size() <= 1) return;
        reverse(path.begin(), path.end());
        if (root != dummy) {
            ans.push_back(move(path));
        } else {
            vector<int> part;
            for (int u : path) {
                if (u == dummy) {
                    if (part.size() >= 2) ans.push_back(move(part));
                    part.clear();
                } else {
                    part.push_back(u);
                }
            }
        }
    };
    if (!adj[dummy].empty()) walk(dummy);
    if (preferred >= 0) walk(preferred);
    for (int u = 0; u < vertices; ++u)
        if (pos[u] < (int)adj[u].size()) walk(u);

    if (preferred >= 0) {
        for (int i = 0; i < (int)ans.size(); ++i) {
            if (ans[i].front() == preferred || ans[i].back() == preferred) {
                if (ans[i].front() != preferred) reverse(ans[i].begin(), ans[i].end());
                swap(ans[0], ans[i]);
                break;
            }
        }
    }
    return ans;
}

void solveOne(State& st) {
    while (st.edges) {
        vector<pair<int,int>> e;
        for (int x = 0; x < n; ++x)
            forBits(st.unknown[x], [&](int y) {
                if (x < y) e.emplace_back(x,y);
            });
        int last = st.queue.empty() ? -1 : st.queue.back().rep;
        auto paths = trails(n, e, last);
        bool changed = false;
        for (const auto& p : paths) {
            if (!st.edges) break;
            last = st.queue.empty() ? -1 : st.queue.back().rep;
            if (p.front() != last) {
                bool extra = last >= 0 && has(st.unknown[p.front()], last);
                if (taste(st, p.front()) || extra) {
                    changed = true;
                    break;
                }
            }
            for (int i = 1; i < (int)p.size() && st.edges; ++i) {
                if (taste(st, p[i])) {
                    changed = true;
                    break;
                }
            }
            if (changed) break;
        }
    }
}

struct Plan {
    vector<int> steps;
    int cost = 0;
};

struct Builder {
    State st;
    Plan plan;
    explicit Builder(const State& original) : st(original) {}

    void reset() {
        if (st.queue.empty()) return;
        st.clearMemory();
        plan.steps.push_back(-1);
    }
    void add(int x) {
        st.negative(x);
        plan.steps.push_back(x);
        ++plan.cost;
    }

    // Cover all useful pairs at cyclic distance at most k.
    // A circle of size at most 2*k+1 covers the whole induced subgraph.
    void circle(vector<int> v) {
        if (!st.edges || v.size() < 2) return;
        Mask group{};
        for (int x : v) mark(group, x);
        v.erase(remove_if(v.begin(), v.end(), [&](int x) {
            return !intersects(st.unknown[x], group);
        }), v.end());
        int s = (int)v.size();
        if (s < 2) return;
        for (int x : v) {
            if (has(st.blocked, x)) {
                reset();
                break;
            }
        }
        for (int x : v) {
            if (!st.edges) return;
            add(x);
        }
        int tail = min(k, s - k - 1);
        if (tail <= 0) return;
        Mask suffix{};
        for (int j = s - k; j < s; ++j) mark(suffix, v[j]);
        for (int i = 0; i < tail && st.edges; ++i) {
            if (intersects(st.unknown[v[i]], suffix)) add(v[i]);
            unmark(suffix, v[s - k + i]);
        }
    }
};

int gfMultiply(int x, int y, int bits) {
    static const int poly[] = {
        0, 3, 7, 11, 19, 37, 67, 131, 283, 529, 1033
    };
    int z = 0;
    while (y) {
        if (y & 1) z ^= x;
        y >>= 1;
        x <<= 1;
        if (x & (1 << bits)) x ^= poly[bits];
    }
    return z;
}

void geometry(Builder& b, const vector<int>& mapping, int p, int t) {
    if (!b.st.edges) return;
    if (p <= t) {
        vector<int> v;
        for (int x : mapping) if (x >= 0) v.push_back(x);
        b.circle(move(v));
        return;
    }
    int s = p - t;
    int xbits = min(s, t), xcount = 1 << xbits, q = 1 << s;
    int zcount = 1 << (t - xbits);
    vector<int> product(xcount);
    // These cosets cover every difference with a nonzero x coordinate.
    for (int m = 0; m < q && b.st.edges; ++m) {
        for (int x = 0; x < xcount; ++x) product[x] = gfMultiply(m, x, s);
        for (int c = 0; c < q && b.st.edges; ++c) {
            vector<int> v;
            for (int z = 0; z < zcount; ++z) {
                for (int x = 0; x < xcount; ++x) {
                    int at = ((z * q + (product[x] ^ c)) * xcount) + x;
                    if (mapping[at] >= 0) v.push_back(mapping[at]);
                }
            }
            b.circle(move(v));
        }
    }
    // Remaining differences are vertical; recurse in each vertical fiber.
    for (int x = 0; x < xcount && b.st.edges; ++x) {
        vector<int> v(q * zcount);
        for (int i = 0; i < (int)v.size(); ++i) v[i] = mapping[i * xcount + x];
        geometry(b, v, p - xbits, t);
    }
}

void addGeometry(Builder& b, int variant = 0) {
    auto v = b.st.relevant();
    if (v.empty()) return;
    int p = 0;
    while ((1 << p) < (int)v.size()) ++p;
    int t = __builtin_ctz((unsigned)k) + (variant == 2 ? 0 : 1);
    v.resize(1 << p, -1);
    if (variant == 1) {
        for (int i = 0; i < (1 << p); ++i) {
            int r = 0;
            for (int j = 0; j < p; ++j) r = (r << 1) | ((i >> j) & 1);
            if (r > i) swap(v[i], v[r]);
        }
    }
    geometry(b, v, p, t);
}

Plan blockTrails(const State& original) {
    Builder b(original);
    auto v = original.relevant();
    int m = k / 2, groups = ((int)v.size() + m - 1) / m;
    vector<Mask> masks(groups);
    for (int i = 0; i < (int)v.size(); ++i) mark(masks[i / m], v[i]);
    vector<pair<int,int>> edges;
    for (int i = 0; i < groups; ++i) {
        for (int j = i + 1; j < groups; ++j) {
            for (int a = i * m; a < min((i + 1) * m, (int)v.size()); ++a) {
                if (intersects(original.unknown[v[a]], masks[j])) {
                    edges.emplace_back(i,j);
                    break;
                }
            }
        }
    }
    for (const auto& path : trails(groups, edges)) {
        if (!b.st.edges) break;
        b.reset();
        for (int i : path) {
            for (int a = i * m; a < min((i + 1) * m, (int)v.size()) && b.st.edges; ++a)
                b.add(v[a]);
        }
    }
    for (int i = 0; i < groups && b.st.edges; ++i) {
        vector<int> part(v.begin() + i * m, v.begin() + min((i + 1) * m, (int)v.size()));
        b.circle(move(part));
    }
    return move(b.plan);
}

Plan arithmetic(const State& original, int mod, bool backwards) {
    Builder b(original);
    auto vertices = original.relevant();
    int s = (int)vertices.size(), D = mod / 2;
    vector<int> index(n, -1), label(mod, -1);
    if (backwards) {
        int lo = 0, hi = s - 1;
        for (int i = 0; i < s; ++i) label[i] = vertices[i & 1 ? hi-- : lo++];
    } else {
        copy(vertices.begin(), vertices.end(), label.begin());
    }
    for (int i = 0; i < mod; ++i)
        if (label[i] >= 0) index[label[i]] = i;

    vector<int> counts(D + 1);
    for (int x : vertices) {
        forBits(original.unknown[x], [&](int y) {
            if (y <= x) return;
            int d = abs(index[x] - index[y]);
            ++counts[min(d, mod - d)];
        });
    }
    vector<vector<int>> types(D + 1);
    for (int d = 1; d <= D; ++d) {
        for (int j = 1; j <= k; ++j) {
            int a = j * d % mod;
            if (a) types[d].push_back(min(a, mod - a));
        }
        sort(types[d].begin(), types[d].end());
        types[d].erase(unique(types[d].begin(), types[d].end()), types[d].end());
    }

    vector<int> schedule;
    if (k == 2) {
        vector<char> seen(D + 1);
        for (int d = 1; d <= D; ++d) {
            if (seen[d]) continue;
            vector<int> cycle;
            int x = d;
            do {
                seen[x] = true;
                cycle.push_back(x);
                x = min(2*x, mod - 2*x);
            } while (x != d);
            int len = (int)cycle.size(), start = 0;
            while (start < len && counts[cycle[start]]) ++start;
            if (start == len) {
                for (int j = 0; j < len; j += 2) schedule.push_back(cycle[j]);
            } else {
                for (int j = 1; j < len; ++j) {
                    int a = cycle[(start+j) % len];
                    if (counts[a]) {
                        schedule.push_back(a);
                        ++j;
                    }
                }
            }
        }
        stable_sort(schedule.begin(), schedule.end(), [&](int x, int y) {
            auto weight = [&](int d) {
                int w = 0;
                for (int a : types[d]) w += counts[a];
                return w;
            };
            return weight(x) > weight(y);
        });
    } else {
        auto left = counts;
        while (true) {
            int best = 0, key = 0;
            for (int d = 1; d <= D; ++d) {
                int gain = 0, weight = 0;
                for (int a : types[d]) if (left[a]) {
                    ++gain;
                    weight += left[a];
                }
                int val = gain * (mod * k + 1) + weight;
                if (val > key) {
                    key = val;
                    best = d;
                }
            }
            if (!best) break;
            schedule.push_back(best);
            for (int a : types[best]) left[a] = 0;
        }
    }

    for (int d : schedule) {
        if (!b.st.edges) break;
        vector<char> needed(mod), seen(mod);
        for (int i = 0; i < mod; ++i) {
            if (label[i] < 0 || !b.st.deg[label[i]]) continue;
            int j = i;
            for (int z = 0; z < k; ++z) {
                j += d;
                if (j >= mod) j -= mod;
                if (label[j] >= 0 && has(b.st.unknown[label[i]], label[j]))
                    needed[i] = needed[j] = true;
            }
        }
        for (int i = 0; i < mod && b.st.edges; ++i) {
            if (seen[i]) continue;
            vector<int> v;
            int j = i;
            do {
                seen[j] = true;
                if (needed[j]) v.push_back(label[j]);
                j += d;
                if (j >= mod) j -= mod;
            } while (j != i);
            b.circle(move(v));
        }
    }
    if (b.st.edges) addGeometry(b);
    return move(b.plan);
}

long long greedyBudget = 330000000;

uint32_t mixBits(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    return x ^ (x >> 16);
}

Plan greedyPlan(const State& original, int limit, int variant) {
    Builder b(original);
    auto v = original.relevant();
    vector<uint32_t> rank(n);
    for (int x : v) rank[x] = mixBits(x + 1779033703u * (variant + 1));
    while (b.st.edges && b.plan.cost < limit && greedyBudget > 0) {
        greedyBudget -= (long long)v.size() * W;
        if ((b.plan.cost & 127) == 0) {
            v.erase(remove_if(v.begin(), v.end(), [&](int x) {
                return b.st.deg[x] == 0;
            }), v.end());
        }
        int best = -1, bestKey = -1, bestTie = INT_MIN;
        for (int x : v) {
            int d = b.st.deg[x];
            if (!d || has(b.st.blocked, x)) continue;
            int gain = 0;
            for (int j = 0; j < W; ++j)
                gain += __builtin_popcountll(b.st.unknown[x][j] & b.st.certain[j]);
            if (!gain) continue;
            int key = gain * 1024;
            if (variant == 2)
                key += gain * 512 * k / (d * 3 + k + 1);
            if (variant == 3)
                key += gain * 768 * k / (d * 2 + k + 1);
            int tie = variant == 0 ? d : -d;
            if (best < 0 || key > bestKey ||
                (key == bestKey && (tie > bestTie ||
                   (tie == bestTie && rank[x] > rank[best])))) {
                best = x;
                bestKey = key;
                bestTie = tie;
            }
        }
        if (best < 0) {
            Mask free{};
            for (int j = 0; j < W; ++j)
                free[j] = b.st.alive[j] & ~b.st.blocked[j];
            int degree = -1;
            for (int x : v) {
                if (!has(free, x) || !b.st.deg[x]) continue;
                if (!intersects(b.st.unknown[x], free)) continue;
                if (best < 0 || b.st.deg[x] > degree ||
                    (b.st.deg[x] == degree && rank[x] > rank[best])) {
                    best = x;
                    degree = b.st.deg[x];
                }
            }
            if (best < 0) {
                b.reset();
                continue;
            }
        }
        b.add(best);
    }
    if (b.st.edges) return {};
    return move(b.plan);
}

Plan makePlan(const State& st, const Plan& previous, int from, int round) {
    Builder geo(st);
    addGeometry(geo);
    Plan best = move(geo.plan);
    auto improve = [&](Plan candidate) {
        if (candidate.cost && candidate.cost < best.cost) best = move(candidate);
    };

    if (from < (int)previous.steps.size()) {
        Builder repair(st);
        for (int i = from; i < (int)previous.steps.size() && repair.st.edges &&
             repair.plan.cost < best.cost; ++i) {
            int x = previous.steps[i];
            if (x < 0) repair.reset();
            else if (repair.st.deg[x] && !has(repair.st.blocked, x)) repair.add(x);
        }
        if (repair.plan.cost < best.cost) {
            if (repair.st.edges) addGeometry(repair);
            improve(move(repair.plan));
        }
    }

    if (round <= 2 || (round < 20 && round % 5 == 0)) {
        improve(blockTrails(st));
        Builder alternate(st);
        addGeometry(alternate, 1);
        improve(move(alternate.plan));
        if (round == 0) {
            Builder smaller(st);
            addGeometry(smaller, 2);
            improve(move(smaller.plan));
        }
    }
    if (round <= 1 && k <= 16) {
        int s = (int)st.relevant().size();
        if (s > 2*k + 1) {
            int odd = s | 1;
            improve(arithmetic(st, odd, false));
            if (round == 0) {
                improve(arithmetic(st, odd + 2, false));
                improve(arithmetic(st, odd, true));
                if (k == 2) improve(arithmetic(st, odd + 4, false));
            }
        }
    }

    int tries = round == 0 ? 4 : (round < 5 ? 2 : 1);
    for (int v = 0; v < tries && greedyBudget > 0 && best.cost > 1; ++v)
        improve(greedyPlan(st, best.cost - 1, (v + round) % 4));
    return best;
}

void adaptiveBatches(State& st) {
    auto v = st.relevant();
    int m = k / 2, groups = ((int)v.size() + m - 1) / m;
    for (int i = 0; i < groups && st.edges; ++i) {
        for (int j = i; j < groups && st.edges; ++j) {
            Mask group{}, use{};
            vector<int> batch;
            auto addGroup = [&](int g) {
                for (int p = g * m; p < min((g + 1) * m, (int)v.size()); ++p) {
                    if (st.deg[v[p]]) {
                        batch.push_back(v[p]);
                        mark(group, v[p]);
                    }
                }
            };
            addGroup(i);
            if (i != j) addGroup(j);
            for (int x : batch)
                if (intersects(st.unknown[x], group)) mark(use, x);
            if (intersects(use, st.blocked)) resetMemory(st);
            for (int x : batch) {
                if (!st.edges) break;
                if (has(use, x) && intersects(st.unknown[x], use)) taste(st, x);
            }
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> k)) return 0;
    W = (n + 63) / 64;
    State st;
    st.init();

    // With no repeated index in this pass, every positive answer is useful.
    for (int x = 0; x < n && st.edges; ++x) taste(st, x);
    if (k == 1) {
        solveOne(st);
    } else {
        Plan plan;
        int at = 0, round = 0;
        long long replanningWork = 0;
        while (st.edges) {
            if (resets >= 16000 || round >= 64 || replanningWork > 22000000LL ||
                tastings > 12000) {
                // Preserve useful comparisons from the existing plan, then
                // finish with short, distinct-index batches.
                for (; at < (int)plan.steps.size() && st.edges && resets < 16000; ++at) {
                    int x = plan.steps[at];
                    if (x < 0) resetMemory(st);
                    else if (st.deg[x] && !has(st.blocked, x)) taste(st, x);
                }
                if (st.edges) adaptiveBatches(st);
                break;
            }
            replanningWork += st.edges;
            plan = makePlan(st, plan, at, round++);
            at = 0;
            while (at < (int)plan.steps.size() && st.edges && resets < 16000) {
                int x = plan.steps[at++];
                if (x < 0) resetMemory(st);
                else if (taste(st, x)) break;
            }
        }
    }
    cout << "! " << st.remaining << endl;
    return 0;
}
