#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using u64 = uint64_t;

struct State {
    int root;
    int mask;
    int cls;
    u64 hash;
};

struct Solver {
    int n;
    vector<vector<int>> adj;
    vector<pair<int, int>> edges;
    vector<int> parentTable;
    vector<int> tinTable, toutTable;
    vector<int> known;
    vector<int> missing;
    vector<vector<int>> fs;
    vector<vector<int8_t>> as;
    vector<State> states;
    int classCount = 0;
    mt19937 rng{0x9e3779b9u};

    int &par(int r, int v) { return parentTable[r * n + v]; }
    int par(int r, int v) const { return parentTable[r * n + v]; }
    int &tin(int r, int v) { return tinTable[r * n + v]; }
    int &tout(int r, int v) { return toutTable[r * n + v]; }
    int tin(int r, int v) const { return tinTable[r * n + v]; }
    int tout(int r, int v) const { return toutTable[r * n + v]; }

    bool ancestor(int r, int u, int v) const {
        return tin(r, u) <= tin(r, v) && tout(r, v) <= tout(r, u);
    }

    void preprocess() {
        parentTable.assign(n * n, -2);
        tinTable.assign(n * n, 0);
        toutTable.assign(n * n, 0);
        vector<int> order;
        for (int r = 0; r < n; ++r) {
            order.clear();
            order.push_back(r);
            par(r, r) = -1;
            for (int i = 0; i < (int)order.size(); ++i) {
                int v = order[i];
                for (int w : adj[v]) {
                    if (par(r, w) != -2) continue;
                    par(r, w) = v;
                    order.push_back(w);
                }
            }
            int timer = 0;
            vector<int> it(n, 0), st;
            st.push_back(r);
            tin(r, r) = timer++;
            while (!st.empty()) {
                int v = st.back();
                if (it[v] < (int)adj[v].size()) {
                    int w = adj[v][it[v]++];
                    if (w == par(r, v)) continue;
                    tin(r, w) = timer++;
                    st.push_back(w);
                } else {
                    tout(r, v) = timer++;
                    st.pop_back();
                }
            }
        }
    }

    bool readResponse(int64 &x) {
        if (!(cin >> x)) return false;
        return true;
    }

    bool askSingleton(int v, int64 &ans) {
        cout << "? 1 1 " << v + 1 << '\n' << flush;
        return readResponse(ans);
    }

    u64 hashValues(const vector<int8_t> &a) const {
        u64 h = 1469598103934665603ULL;
        for (int8_t x : a) {
            h ^= static_cast<unsigned char>(x + 1);
            h *= 1099511628211ULL;
        }
        h ^= static_cast<u64>(n) << 32;
        return h;
    }

    void buildStates() {
        fs.clear();
        as.clear();
        states.clear();
        classCount = 0;

        vector<vector<int>> options(missing.size());
        for (int i = 0; i < (int)missing.size(); ++i) {
            int v = missing[i];
            bool first = true;
            for (int w : adj[v]) {
                if (known[w] == INT_MAX) continue;
                vector<int> cur = {known[w] - 1, known[w] + 1};
                if (first) {
                    options[i] = cur;
                    first = false;
                } else {
                    vector<int> nxt;
                    for (int x : options[i]) {
                        if (x == cur[0] || x == cur[1]) nxt.push_back(x);
                    }
                    options[i] = move(nxt);
                }
            }
            sort(options[i].begin(), options[i].end());
            options[i].erase(unique(options[i].begin(), options[i].end()), options[i].end());
            if (options[i].empty()) return;
        }

        vector<int> f = known;
        function<void(int)> gen = [&](int at) {
            if (at < (int)missing.size()) {
                for (int x : options[at]) {
                    f[missing[at]] = x;
                    gen(at + 1);
                }
                return;
            }
            for (auto [v, w] : edges) {
                if (abs(f[v] - f[w]) != 1) return;
            }
            fs.push_back(f);
        };
        gen(0);

        for (int mask = 0; mask < (int)fs.size(); ++mask) {
            const vector<int> &fv = fs[mask];
            for (int r = 0; r < n; ++r) {
                if (abs(fv[r]) != 1) continue;
                vector<int8_t> a(n);
                for (int v = 0; v < n; ++v) {
                    a[v] = (v == r ? fv[v] : fv[v] - fv[par(r, v)]);
                }
                State st{r, mask, 0, hashValues(a)};
                states.push_back(st);
                as.push_back(move(a));
            }
        }

        vector<int> ord(states.size());
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int x, int y) {
            return states[x].hash < states[y].hash;
        });
        int i = 0;
        while (i < (int)ord.size()) {
            int j = i;
            vector<int> representatives;
            while (j < (int)ord.size() && states[ord[j]].hash == states[ord[i]].hash) {
                int id = ord[j];
                int cls = -1;
                for (int rep : representatives) {
                    if (as[id] == as[rep]) {
                        cls = states[rep].cls;
                        break;
                    }
                }
                if (cls == -1) {
                    cls = classCount++;
                    representatives.push_back(id);
                }
                states[id].cls = cls;
                ++j;
            }
            i = j;
        }
    }

    vector<int> randomCoefficients(int kind) {
        vector<int> c(n, 0);
        if (kind == 0) {
            fill(c.begin(), c.end(), 1);
            return c;
        }
        if (kind == 1) {
            for (int i = 0; i < n; ++i) c[rng() % n]++;
            return c;
        }
        int used = 0;
        for (int i = 0; i < n; ++i) {
            if (rng() & 1) c[i] = 1, ++used;
        }
        c[rng() % n] += n - used;
        return c;
    }

    bool evaluateQuery(int u, const vector<int> &c, bool toggle,
                       vector<int64> *predictions = nullptr) {
        int masks = fs.size();
        vector<int64> base(masks, 0);
        for (int mask = 0; mask < masks; ++mask) {
            int64 sum = 0;
            for (int v = 0; v < n; ++v) sum += (int64)c[v] * fs[mask][v];
            base[mask] = sum;
        }

        vector<int> comp(n, -1), compSum;
        int total = accumulate(c.begin(), c.end(), 0);
        if (toggle) {
            for (int w : adj[u]) {
                int id = compSum.size();
                compSum.push_back(0);
                vector<int> st = {w};
                comp[w] = id;
                while (!st.empty()) {
                    int v = st.back();
                    st.pop_back();
                    compSum[id] += c[v];
                    for (int z : adj[v]) {
                        if (z == u || comp[z] != -1) continue;
                        comp[z] = id;
                        st.push_back(z);
                    }
                }
            }
        }

        vector<int64> pred(states.size());
        vector<int64> classResponse(classCount, LLONG_MIN);
        unordered_map<int64, int> responseClass;
        responseClass.reserve(states.size() * 2 + 1);
        for (int i = 0; i < (int)states.size(); ++i) {
            const State &st = states[i];
            int64 value = base[st.mask];
            if (toggle) {
                int w = total;
                if (st.root != u) w = total - compSum[comp[st.root]];
                value -= 2LL * as[i][u] * w;
            }
            pred[i] = value;
            if (classResponse[st.cls] == LLONG_MIN) {
                classResponse[st.cls] = value;
            } else if (classResponse[st.cls] != value) {
                return false;
            }
            auto [it, inserted] = responseClass.emplace(value, st.cls);
            if (!inserted && it->second != st.cls) return false;
        }
        if (predictions) *predictions = move(pred);
        return true;
    }

    bool findFullQuery(bool toggle, int &bestU, vector<int> &bestC,
                       vector<int64> &predictions) {
        vector<int> vertices(n);
        iota(vertices.begin(), vertices.end(), 0);
        sort(vertices.begin(), vertices.end(), [&](int x, int y) {
            return adj[x].size() > adj[y].size();
        });
        int trials = states.size() > 12000 ? 350 : 1200;
        for (int t = 0; t < trials; ++t) {
            int u = 0;
            if (toggle) {
                if (t < (int)vertices.size()) u = vertices[t];
                else u = rng() % n;
            }
            vector<int> c = randomCoefficients(t == 0 ? 0 : (t % 3 == 0 ? 2 : 1));
            if (evaluateQuery(u, c, toggle, &predictions)) {
                bestU = u;
                bestC = move(c);
                return true;
            }
        }
        return false;
    }

    bool askWeighted(const vector<int> &c, int64 &ans) {
        vector<int> nodes;
        for (int v = 0; v < n; ++v) {
            for (int j = 0; j < c[v]; ++j) nodes.push_back(v);
        }
        cout << "? 1 " << nodes.size();
        for (int v : nodes) cout << ' ' << v + 1;
        cout << '\n' << flush;
        return readResponse(ans);
    }

    void printAnswer(const vector<int8_t> &a) {
        cout << '!';
        for (int8_t x : a) cout << ' ' << int(x);
        cout << '\n' << flush;
    }

    void deterministicFallback() {
        struct DState {
            int root, cls;
            vector<int> f;
            vector<int8_t> a;
        };
        vector<DState> ds;
        for (int i = 0; i < (int)states.size(); ++i) {
            ds.push_back({states[i].root, states[i].cls, fs[states[i].mask], as[i]});
        }

        while (true) {
            unordered_set<int> remainingClasses;
            for (const auto &st : ds) remainingClasses.insert(st.cls);
            int currentClasses = remainingClasses.size();
            if (currentClasses <= 1) {
                printAnswer(ds[0].a);
                return;
            }

            int bestWorst = currentClasses;
            int bestGroups = 1;
            int bestU = -1;
            bool bestToggle = false;
            vector<int> bestC;
            vector<int64> bestPred;

            auto consider = [&](int u, const vector<int> &c, bool toggle,
                                const vector<int64> &givenBase) {
                vector<int> comp(n, -1), compSum;
                int total = accumulate(c.begin(), c.end(), 0);
                if (toggle) {
                    for (int w : adj[u]) {
                        int id = compSum.size();
                        compSum.push_back(0);
                        vector<int> stack = {w};
                        comp[w] = id;
                        while (!stack.empty()) {
                            int x = stack.back();
                            stack.pop_back();
                            compSum[id] += c[x];
                            for (int z : adj[x]) {
                                if (z == u || comp[z] != -1) continue;
                                comp[z] = id;
                                stack.push_back(z);
                            }
                        }
                    }
                }
                vector<int64> base = givenBase;
                if (base.empty()) {
                    base.resize(ds.size());
                    for (int i = 0; i < (int)ds.size(); ++i) {
                        for (int v = 0; v < n; ++v) base[i] += (int64)c[v] * ds[i].f[v];
                    }
                }
                vector<int64> pred(ds.size());
                map<int64, vector<int>> groups;
                for (int i = 0; i < (int)ds.size(); ++i) {
                    int64 value = base[i];
                    if (toggle) {
                        int w = total;
                        if (ds[i].root != u) w = total - compSum[comp[ds[i].root]];
                        value -= 2LL * ds[i].a[u] * w;
                    }
                    pred[i] = value;
                    groups[value].push_back(ds[i].cls);
                }
                int worst = 0;
                for (auto &[value, classes] : groups) {
                    sort(classes.begin(), classes.end());
                    classes.erase(unique(classes.begin(), classes.end()), classes.end());
                    worst = max(worst, (int)classes.size());
                }
                if (worst < bestWorst ||
                    (worst == bestWorst && (int)groups.size() > bestGroups)) {
                    bestWorst = worst;
                    bestGroups = groups.size();
                    bestU = u;
                    bestToggle = toggle;
                    bestC = c;
                    bestPred = move(pred);
                }
            };

            vector<int> ones(n, 1);
            vector<int64> onesBase(ds.size(), 0);
            for (int i = 0; i < (int)ds.size(); ++i) {
                for (int v = 0; v < n; ++v) onesBase[i] += ds[i].f[v];
            }
            consider(0, ones, false, onesBase);
            for (int u = 0; u < n; ++u) consider(u, ones, true, onesBase);

            vector<int> vertices(n);
            iota(vertices.begin(), vertices.end(), 0);
            sort(vertices.begin(), vertices.end(), [&](int x, int y) {
                return adj[x].size() > adj[y].size();
            });
            int trials = ds.size() > 400 ? 60 : 120;
            for (int t = 0; t < trials; ++t) {
                bool toggle = t % 4 != 0;
                int u = 0;
                if (toggle) {
                    if (t <= min(8, n)) u = vertices[t - 1];
                    else if (rng() & 1) u = ds[rng() % ds.size()].root;
                    else u = rng() % n;
                }
                vector<int> c = randomCoefficients(t % 3 == 0 ? 2 : 1);
                consider(u, c, toggle, {});
            }
            if (bestWorst < currentClasses) {
                if (bestToggle) cout << "? 2 " << bestU + 1 << '\n' << flush;
                int64 response;
                if (!askWeighted(bestC, response)) exit(0);
                vector<DState> next;
                for (int i = 0; i < (int)ds.size(); ++i) {
                    if (bestPred[i] != response) continue;
                    DState &st = ds[i];
                    if (bestToggle) {
                        for (int x = 0; x < n; ++x) {
                            if (ancestor(st.root, bestU, x)) st.f[x] -= 2 * st.a[bestU];
                        }
                        st.a[bestU] = -st.a[bestU];
                    }
                    next.push_back(move(st));
                }
                ds = move(next);
                continue;
            }

            int queryV = -1;
            int bestDifferent = 0;
            for (int v = 0; v < n; ++v) {
                int low = ds[0].f[v], high = low;
                for (const auto &st : ds) {
                    low = min(low, st.f[v]);
                    high = max(high, st.f[v]);
                }
                int different = high - low;
                if (different > bestDifferent) {
                    bestDifferent = different;
                    queryV = v;
                }
            }
            if (queryV != -1) {
                int64 response;
                if (!askSingleton(queryV, response)) exit(0);
                vector<DState> next;
                for (auto &st : ds) {
                    if (st.f[queryV] == response) next.push_back(move(st));
                }
                ds = move(next);
                continue;
            }

            int ia = 0, ib = 0;
            for (int i = 1; i < (int)ds.size(); ++i) {
                if (ds[i].cls != ds[0].cls) {
                    ib = i;
                    break;
                }
            }
            int u = ds[ia].root;
            int v = ds[ib].root;
            vector<int64> pred(ds.size());
            for (int i = 0; i < (int)ds.size(); ++i) {
                pred[i] = ds[i].f[v] - 2LL * ds[i].a[u] * ancestor(ds[i].root, u, v);
            }
            cout << "? 2 " << u + 1 << '\n' << flush;
            int64 response;
            if (!askSingleton(v, response)) exit(0);
            vector<DState> next;
            for (int i = 0; i < (int)ds.size(); ++i) {
                if (pred[i] != response) continue;
                DState &st = ds[i];
                for (int x = 0; x < n; ++x) {
                    if (ancestor(st.root, u, x)) st.f[x] -= 2 * st.a[u];
                }
                st.a[u] = -st.a[u];
                next.push_back(move(st));
            }
            ds = move(next);
        }
    }

    void run() {
        preprocess();
        known.assign(n, INT_MAX);

        vector<char> isMissing(n, false);
        vector<int> order(n);
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int x, int y) {
            return adj[x].size() > adj[y].size();
        });
        int target = min(8, n - 1);
        for (int v : order) {
            if ((int)missing.size() >= target) break;
            bool ok = true;
            for (int w : adj[v]) {
                if (isMissing[w]) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                isMissing[v] = true;
                missing.push_back(v);
            }
        }

        for (int v = 0; v < n; ++v) {
            if (isMissing[v]) continue;
            int64 response;
            if (!askSingleton(v, response)) return;
            known[v] = response;
        }

        while (true) {
            buildStates();
            if (states.empty()) return;
            if (classCount == 1) {
                printAnswer(as[0]);
                return;
            }

            bool inferred = false;
            for (int i = 0; i < (int)missing.size(); ++i) {
                int v = missing[i];
                int value = fs[0][v];
                bool same = true;
                for (const vector<int> &f : fs) {
                    if (f[v] != value) {
                        same = false;
                        break;
                    }
                }
                if (same) {
                    known[v] = value;
                    missing.erase(missing.begin() + i);
                    inferred = true;
                    break;
                }
            }
            if (inferred) continue;

            int u = -1;
            vector<int> c;
            vector<int64> predictions;
            if (findFullQuery(false, u, c, predictions)) {
                int64 response;
                if (!askWeighted(c, response)) return;
                for (int i = 0; i < (int)states.size(); ++i) {
                    if (predictions[i] == response) {
                        printAnswer(as[i]);
                        return;
                    }
                }
                return;
            }

            if (findFullQuery(true, u, c, predictions)) {
                cout << "? 2 " << u + 1 << '\n' << flush;
                int64 response;
                if (!askWeighted(c, response)) return;
                for (int i = 0; i < (int)states.size(); ++i) {
                    if (predictions[i] == response) {
                        vector<int8_t> answer = as[i];
                        answer[u] = -answer[u];
                        printAnswer(answer);
                        return;
                    }
                }
                return;
            }

            if (!missing.empty()) {
                int bestIndex = 0;
                int bestWorstClasses = INT_MAX;
                int bestWorstStates = INT_MAX;
                for (int i = 0; i < (int)missing.size(); ++i) {
                    int v = missing[i];
                    map<int, vector<int>> groups;
                    for (const State &st : states) groups[fs[st.mask][v]].push_back(st.cls);
                    int worstClasses = 0, worstStates = 0;
                    for (auto &[value, classes] : groups) {
                        worstStates = max(worstStates, (int)classes.size());
                        sort(classes.begin(), classes.end());
                        classes.erase(unique(classes.begin(), classes.end()), classes.end());
                        worstClasses = max(worstClasses, (int)classes.size());
                    }
                    if (worstClasses < bestWorstClasses ||
                        (worstClasses == bestWorstClasses && worstStates < bestWorstStates)) {
                        bestWorstClasses = worstClasses;
                        bestWorstStates = worstStates;
                        bestIndex = i;
                    }
                }
                int v = missing[bestIndex];
                missing.erase(missing.begin() + bestIndex);
                int64 response;
                if (!askSingleton(v, response)) return;
                known[v] = response;
                continue;
            }

            deterministicFallback();
            return;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Solver solver;
    if (!(cin >> solver.n)) return 0;
    solver.adj.assign(solver.n, {});
    for (int i = 0; i < solver.n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;
        solver.adj[u].push_back(v);
        solver.adj[v].push_back(u);
        solver.edges.push_back({u, v});
    }
    solver.run();
    return 0;
}
