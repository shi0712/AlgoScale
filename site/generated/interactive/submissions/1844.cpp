#include <bits/stdc++.h>
using namespace std;

using Query = array<int, 3>;

int n, replyTypes;
int areaCode[769], tri[64];
bool injectivePair[4][4];
vector<int> valueAt, owner;
vector<int> representatives[4];
int knownCount = 0, frequency[4]{};
set<int> fresh, active;
unordered_map<uint64_t, int> history;

struct Unit {
    vector<int> index, states;
};
vector<Unit> units;

uint64_t keyOf(Query q) {
    sort(q.begin(), q.end());
    return (uint64_t(q[0] + 1) << 26) |
           (uint64_t(q[1] + 1) << 13) | uint64_t(q[2] + 1);
}

int ask(Query q) {
    sort(q.begin(), q.end());
    uint64_t key = keyOf(q);
    auto it = history.find(key);
    if (it != history.end()) return it->second;
    cout << "? " << q[0] + 1 << ' ' << q[1] + 1 << ' ' << q[2] + 1 << endl;
    int r;
    if (!(cin >> r) || r < 0 || r > 768 || areaCode[r] < 0) exit(0);
    return history.emplace(key, areaCode[r]).first->second;
}

[[noreturn]] void finish(bool unique) {
    if (!unique) {
        cout << "! -1" << endl;
    } else {
        cout << '!';
        for (int x : valueAt) cout << ' ' << x + 1;
        cout << endl;
    }
    exit(0);
}

void initTable() {
    fill(begin(areaCode), end(areaCode), -1);
    areaCode[0] = 0;
    replyTypes = 1;
    for (int c = 0; c < 64; ++c) {
        int x = (c & 3) + 1, y = ((c >> 2) & 3) + 1, z = (c >> 4) + 1;
        int p = (x + y + z) * (x + y - z) * (x + z - y) * (y + z - x);
        p = max(0, p);
        if (areaCode[p] == -1) areaCode[p] = replyTypes++;
        tri[c] = areaCode[p];
    }
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) {
        int mask = 0;
        for (int z = 0; z < 4; ++z) mask |= 1 << tri[x | (y << 2) | (z << 4)];
        injectivePair[x][y] = popcount(unsigned(mask)) == 4;
    }
}

void markKnown(int i, int x) {
    if (valueAt[i] >= 0) return;
    if (owner[i] == -2) fresh.erase(i);
    owner[i] = -1;
    valueAt[i] = x;
    ++knownCount;
    ++frequency[x];
    if (representatives[x].size() < 2) representatives[x].push_back(i);
}

struct VectorHash {
    size_t operator()(const vector<int>& a) const {
        uint64_t h = a.size();
        for (int x : a) h = h * 1000000007ULL + x + 1;
        return h ^ (h >> 29);
    }
};

// Solve a small, explicit decision problem. Equal full signatures are
// indistinguishable, independently of which queries are actually asked.
struct Exact {
    vector<int> vars, configs;
    vector<Query> questions;
    vector<vector<unsigned char>> out;
    vector<char> uniqueConfig;
    unordered_map<vector<int>, pair<int, int>, VectorHash> memo;

    Exact(vector<int> v, vector<int> c) : vars(move(v)), configs(move(c)) {
        vector<int> indices = vars;
        for (auto &r : representatives)
            indices.insert(indices.end(), r.begin(), r.end());
        int m = int(vars.size()), b = int(indices.size());
        for (int i = 0; i < m; ++i) for (int j = i + 1; j < b; ++j)
            for (int k = j + 1; k < b; ++k) {
                Query q{indices[i], indices[j], indices[k]};
                questions.push_back(q);
                out.emplace_back();
                auto &row = out.back();
                row.reserve(configs.size());
                for (int s : configs) {
                    int x = (s >> (2 * i)) & 3;
                    int y = j < m ? ((s >> (2 * j)) & 3) : valueAt[indices[j]];
                    int z = k < m ? ((s >> (2 * k)) & 3) : valueAt[indices[k]];
                    row.push_back(tri[x | (y << 2) | (z << 4)]);
                }
            }

        unordered_map<string, int> classes;
        vector<string> sig(configs.size(), string(out.size(), '\0'));
        for (int s = 0; s < int(configs.size()); ++s) {
            for (int q = 0; q < int(out.size()); ++q) sig[s][q] = char(out[q][s]);
            ++classes[sig[s]];
        }
        for (auto &s : sig) uniqueConfig.push_back(classes[s] == 1);
    }

    bool terminal(const vector<int>& c) const {
        if (c.size() <= 1) return true;
        for (int s : c) if (uniqueConfig[s]) return false;
        return true;
    }

    pair<int, int> optimal(const vector<int>& c) {
        if (terminal(c)) return {0, -1};
        auto it = memo.find(c);
        if (it != memo.end()) return it->second;

        int best = INT_MAX, bestQ = -1;
        for (int q = 0; q < int(out.size()); ++q) {
            array<vector<int>, 16> child;
            for (int s : c) child[out[q][s]].push_back(s);
            if (child[out[q][c[0]]].size() == c.size()) continue;
            int cost = int(c.size());
            for (auto &b : child) if (!terminal(b)) cost += int(b.size());
            if (cost >= best) continue;
            for (auto &b : child) if (!terminal(b)) {
                cost += optimal(b).first - int(b.size());
                if (cost >= best) break;
            }
            if (cost < best) best = cost, bestQ = q;
            if (best == int(c.size())) break;
        }
        return memo.emplace(c, pair{best, bestQ}).first->second;
    }

    [[noreturn]] void run(bool useDP) {
        vector<int> c(configs.size());
        iota(c.begin(), c.end(), 0);
        for (int q = 0; q < int(questions.size()); ++q) {
            auto it = history.find(keyOf(questions[q]));
            if (it != history.end())
                erase_if(c, [&](int s) { return out[q][s] != it->second; });
        }
        while (!terminal(c)) {
            int chosen = -1;
            if (useDP || c.size() <= 12) {
                chosen = optimal(c).second;
            } else {
                double best = -1;
                for (int q = 0; q < int(out.size()); ++q) {
                    int count[16]{}, canBeUnique[16]{};
                    for (int s : c) {
                        int r = out[q][s];
                        ++count[r];
                        canBeUnique[r] += uniqueConfig[s];
                    }
                    if (count[out[q][c.front()]] == int(c.size())) continue;
                    double score = 0;
                    bool done = true;
                    for (int r = 0; r < replyTypes; ++r) if (count[r]) {
                        double p = double(count[r]) / c.size();
                        score -= p * log2(p);
                        if (count[r] == 1 || !canBeUnique[r]) score += 1.2 * p;
                        else done = false;
                    }
                    if (done) { chosen = q; break; }
                    if (score > best) best = score, chosen = q;
                }
            }
            if (chosen < 0) finish(false);
            int r = ask(questions[chosen]);
            erase_if(c, [&](int s) { return out[chosen][s] != r; });
        }
        if (c.empty() || !uniqueConfig[c.front()]) finish(false);
        int s = configs[c.front()];
        for (int j = 0; j < int(vars.size()); ++j) valueAt[vars[j]] = (s >> (2 * j)) & 3;
        finish(true);
    }
};

int stampA[1 << 18], stampB[1 << 18], stampId;

void addUnit(vector<int> idx, vector<int> states);

void splitUnit(vector<int> idx, vector<int> states) {
    int m = int(idx.size());
    if (m <= 1) {
        addUnit(move(idx), move(states));
        return;
    }

    int full = (1 << m) - 1;
    int spread[1 << 9]{};
    for (int s = 1; s <= full; ++s)
        spread[s] = spread[s & (s - 1)] | (3 << (2 * countr_zero(unsigned(s))));

    for (int s = 1; s < full; ++s) {
        if (!(s & 1) && popcount(unsigned(s)) != 1) continue;
        if (s > (full ^ s) && popcount(unsigned(full ^ s)) == 1) continue;
        int a = spread[s], b = spread[full ^ s], ac = 0, bc = 0, id = ++stampId;
        for (int x : states) {
            if (stampA[x & a] != id) stampA[x & a] = id, ++ac;
            if (stampB[x & b] != id) stampB[x & b] = id, ++bc;
            if (ac * bc > int(states.size())) break;
        }
        if (ac * bc != int(states.size())) continue;
        vector<int> ai, bi, as, bs;
        for (int j = 0; j < m; ++j)
            ((s >> j & 1) ? ai : bi).push_back(idx[j]);
        ++stampId;
        for (int x : states) {
            if (stampA[x & a] != stampId) {
                stampA[x & a] = stampId;
                int y = 0, off = 0;
                for (int j = 0; j < m; ++j) if (s >> j & 1)
                    y |= ((x >> (2 * j)) & 3) << (2 * off++);
                as.push_back(y);
            }
            if (stampB[x & b] != stampId) {
                stampB[x & b] = stampId;
                int y = 0, off = 0;
                for (int j = 0; j < m; ++j) if (!(s >> j & 1))
                    y |= ((x >> (2 * j)) & 3) << (2 * off++);
                bs.push_back(y);
            }
        }
        splitUnit(move(ai), move(as));
        splitUnit(move(bi), move(bs));
        return;
    }
    addUnit(move(idx), move(states));
}

void addUnit(vector<int> idx, vector<int> states) {
    if (idx.empty()) return;
    int id = int(units.size());
    for (int i : idx) {
        if (owner[i] == -2) fresh.erase(i);
        owner[i] = id;
    }
    active.insert(id);
    units.push_back({move(idx), move(states)});
}

void install(vector<int> idx, vector<int> states) {
    if (states.empty()) exit(0);
    int diff = 0;
    for (int s : states) diff |= s ^ states.front();
    vector<int> positions, variables;
    for (int j = 0; j < int(idx.size()); ++j) {
        if (diff & (3 << (2 * j))) {
            positions.push_back(j);
            variables.push_back(idx[j]);
        } else {
            markKnown(idx[j], (states.front() >> (2 * j)) & 3);
        }
    }
    if (variables.empty()) return;
    if (variables.size() != idx.size()) {
        for (int &s : states) {
            int t = 0;
            for (int j = 0; j < int(positions.size()); ++j)
                t |= ((s >> (2 * positions[j])) & 3) << (2 * j);
            s = t;
        }
    }
    splitUnit(move(variables), move(states));
}

void eraseUnit(int id) {
    active.erase(id);
    units[id] = Unit{};
}

void apply(Query q, int r) {
    vector<int> idx, used;
    vector<int> states{0};
    for (int i : q) if (valueAt[i] < 0) {
        int id = owner[i];
        if (id >= 0 && find(used.begin(), used.end(), id) != used.end()) continue;
        int shift = 2 * int(idx.size());
        vector<int> next;
        if (id < 0) {
            idx.push_back(i);
            for (int s : states) for (int x = 0; x < 4; ++x) next.push_back(s | (x << shift));
        } else {
            used.push_back(id);
            auto &u = units[id];
            idx.insert(idx.end(), u.index.begin(), u.index.end());
            next.reserve(states.size() * u.states.size());
            for (int s : states) for (int x : u.states) next.push_back(s | (x << shift));
        }
        states.swap(next);
    }
    int fixed = 0, offset[3]{-1, -1, -1};
    for (int j = 0; j < 3; ++j) {
        if (valueAt[q[j]] >= 0) fixed |= valueAt[q[j]] << (2 * j);
        else offset[j] = 2 * int(find(idx.begin(), idx.end(), q[j]) - idx.begin());
    }
    erase_if(states, [&](int s) {
        int c = fixed;
        for (int j = 0; j < 3; ++j)
            if (offset[j] >= 0) c |= ((s >> offset[j]) & 3) << (2 * j);
        return tri[c] != r;
    });
    for (int id : used) eraseUnit(id);
    install(move(idx), move(states));
}

void forceValue(int i, int v) {
    int id = owner[i];
    if (id < 0) {
        markKnown(i, v);
        return;
    }
    Unit u = move(units[id]);
    eraseUnit(id);
    int j = int(find(u.index.begin(), u.index.end(), i) - u.index.begin());
    erase_if(u.states, [&](int s) { return ((s >> (2 * j)) & 3) != v; });
    install(move(u.index), move(u.states));
}

Query findGoodPair() {
    for (int x = 0; x < 4; ++x) for (int y = x; y < 4; ++y) {
        if (!injectivePair[x][y] || representatives[x].empty()) continue;
        if (x == y && representatives[x].size() == 2)
            return {representatives[x][0], representatives[x][1], -1};
        if (x != y && !representatives[y].empty())
            return {representatives[x][0], representatives[y][0], -1};
    }
    return {-1, -1, -1};
}

void bootstrap() {
    int m = 3;
    vector<int> states(64);
    iota(states.begin(), states.end(), 0);
    for (;;) {
        int same[4]{};
        for (int j = 0; j < m; ++j) {
            int x = (states.front() >> (2 * j)) & 3;
            bool fixed = true;
            for (int s : states) if (((s >> (2 * j)) & 3) != x) {
                fixed = false;
                break;
            }
            if (fixed) ++same[x];
        }
        bool ready = same[0] >= 2;
        for (int x = 0; x < 4; ++x) for (int y = x; y < 4; ++y)
            if (injectivePair[x][y] && same[x] && same[y] && (x != y || same[x] >= 2))
                ready = true;
        if (ready || (m == n && states.size() == 1)) {
            vector<int> idx(m);
            iota(idx.begin(), idx.end(), 0);
            install(move(idx), move(states));
            return;
        }
        double best = 0;
        Query q{-1, -1, -1};
        for (int i = 0; i < m; ++i) for (int j = i + 1; j < m; ++j)
            for (int k = j + 1; k < m; ++k) {
                int count[16]{};
                for (int s : states) {
                    int c = ((s >> (2 * i)) & 3) | (((s >> (2 * j)) & 3) << 2) |
                            (((s >> (2 * k)) & 3) << 4);
                    ++count[tri[c]];
                }
                double score = 0;
                for (int r = 0; r < replyTypes; ++r) if (count[r]) {
                    double p = double(count[r]) / states.size();
                    score -= p * log2(p);
                }
                if (score > best + 1e-10) best = score, q = {i, j, k};
            }
        if (q[0] < 0) {
            if (m == n) finish(false);
            // Among nine indices there is an equilateral triple, whose
            // response fixes all three sides.
            if (m >= 9) exit(0);
            int old = int(states.size());
            for (int x = 1; x < 4; ++x) for (int t = 0; t < old; ++t)
                states.push_back(states[t] | (x << (2 * m)));
            ++m;
        } else {
            int r = ask(q);
            erase_if(states, [&](int s) {
                int c = ((s >> (2 * q[0])) & 3) | (((s >> (2 * q[1])) & 3) << 2) |
                        (((s >> (2 * q[2])) & 3) << 4);
                return tri[c] != r;
            });
        }
    }
}

vector<int> feasibleConfigs(const vector<int>& idx, bool nononesOnly) {
    vector<pair<int, int>> mapping;
    for (int i : idx) {
        if (owner[i] < 0) mapping.emplace_back(-1, 0);
        else {
            auto &u = units[owner[i]];
            int p = int(find(u.index.begin(), u.index.end(), i) - u.index.begin());
            mapping.emplace_back(owner[i], p);
        }
    }
    vector<int> result;
    for (int s = 0; s < (1 << (2 * idx.size())); ++s) {
        bool ok = true;
        for (int j = 0; j < int(idx.size()) && ok; ++j) {
            int x = (s >> (2 * j)) & 3;
            if (nononesOnly && !x) { ok = false; break; }
            auto [id, pos] = mapping[j];
            if (id < 0) continue;
            int mask = 0, code = 0;
            for (int k = 0; k < int(idx.size()); ++k) if (mapping[k].first == id) {
                int off = 2 * mapping[k].second;
                mask |= 3 << off;
                code |= ((s >> (2 * k)) & 3) << off;
            }
            ok = false;
            for (int t : units[id].states) if ((t & mask) == code) { ok = true; break; }
        }
        if (ok) result.push_back(s);
    }
    return result;
}

int domain(int i) {
    if (valueAt[i] >= 0) return 1 << valueAt[i];
    if (owner[i] < 0) return 15;
    auto &u = units[owner[i]];
    int j = int(find(u.index.begin(), u.index.end(), i) - u.index.begin());
    int mask = 0;
    for (int s : u.states) mask |= 1 << ((s >> (2 * j)) & 3);
    return mask;
}

void recoverWithOnes() {
    int a = representatives[0][0], b = representatives[0][1];
    vector<int> nonones, undecided;
    for (int i = 0; i < n; ++i) {
        int d = domain(i);
        if (!(d & 1)) nonones.push_back(i);
        else if (d != 1) undecided.push_back(i);
        if (nonones.size() == 4) break;
    }
    for (int i : undecided) {
        if (nonones.size() == 4) break;
        int d = domain(i);
        if (d == 1) continue;
        if (d & 1) {
            Query q{a, b, i};
            int r = ask(q);
            apply(q, r);
            if (r != 0) continue;
        }
        nonones.push_back(i);
    }
    if (nonones.size() == 4) {
        // Four non-ones contain an equal pair. With a one, such a pair
        // has a positive response that identifies its common value.
        vector<pair<int, Query>> candidates;
        for (int j = 0; j < 4; ++j) for (int k = j + 1; k < 4; ++k) {
            Query q{a, nonones[j], nonones[k]};
            int mask = domain(q[1]) & domain(q[2]);
            if (!mask) continue;
            auto it = history.find(keyOf(q));
            if (it != history.end() && it->second == 0) continue;
            int priority = popcount(unsigned(mask)) * 16 /
                (popcount(unsigned(domain(q[1]))) * popcount(unsigned(domain(q[2]))));
            if (it != history.end()) priority += 100;
            candidates.emplace_back(-priority, q);
        }
        sort(candidates.begin(), candidates.end());
        for (auto &[priority, q] : candidates) {
            int r = ask(q);
            if (!r) continue;
            for (int x = 1; x < 4; ++x) if (tri[(x << 2) | (x << 4)] == r) {
                forceValue(q[1], x);
                forceValue(q[2], x);
                return;
            }
        }
        exit(0);
    }

    vector<int> idx;
    for (int i : nonones) if (valueAt[i] < 0) idx.push_back(i);
    Exact(idx, feasibleConfigs(idx, true)).run(true);
}

struct Entry {
    int code;
    double p;
};
using Marginal = vector<Entry>;

constexpr int HSIZE = 4096;
double hTable[HSIZE + 1];

struct Planner {
    struct Var { int index, id, pos, size, count; };
    vector<Var> vars;
    vector<Marginal> single, pairMarg;
    vector<vector<double>> weights;
    vector<int> weightId;
    int m, old;
    double best = -1;
    Query bestQuery{-1, -1, -1};

    Planner(bool withFresh) {
        for (int id : active) {
            const auto &u = units[id];
            for (int j = 0; j < int(u.index.size()); ++j) {
                if (vars.size() == 15) break;
                vars.push_back({u.index[j], id, j, int(u.index.size()), int(u.states.size())});
            }
            if (vars.size() == 15) break;
        }
        old = int(vars.size());
        if (withFresh) for (int i : fresh) {
            if (int(vars.size()) == old + 3) break;
            vars.push_back({i, -1, 0, 1, 4});
        }
        m = int(vars.size());
        single.resize(m);
        pairMarg.resize(m * m);
        weightId.assign(m, -1);

        double prior[4];
        for (int x = 0; x < 4; ++x)
            prior[x] = 0.88 * (frequency[x] + 3.0) / (knownCount + 12.0) + 0.03;

        for (int i = 0; i < m; ++i) {
            auto v = vars[i];
            if (v.id < 0) {
                for (int x = 0; x < 4; ++x) single[i].push_back({x, prior[x]});
                continue;
            }
            if (i && vars[i - 1].id == v.id) weightId[i] = weightId[i - 1];
            else {
                weightId[i] = int(weights.size());
                weights.emplace_back();
                auto &w = weights.back();
                double sum = 0;
                for (int s : units[v.id].states) {
                    double p = 1;
                    for (int j = 0; j < v.size; ++j) p *= prior[(s >> (2 * j)) & 3];
                    w.push_back(p);
                    sum += p;
                }
                for (double &p : w) p /= sum;
            }
            auto &w = weights[weightId[i]];
            auto &states = units[v.id].states;
            double mass[4]{};
            for (int s = 0; s < v.count; ++s) mass[(states[s] >> (2 * v.pos)) & 3] += w[s];
            for (int x = 0; x < 4; ++x) if (mass[x]) single[i].push_back({x, mass[x]});

            for (int j = i - 1; j >= 0 && same(i, j); --j) {
                double p[16]{};
                for (int s = 0; s < v.count; ++s) {
                    int c = ((states[s] >> (2 * vars[j].pos)) & 3) |
                            (((states[s] >> (2 * v.pos)) & 3) << 2);
                    p[c] += w[s];
                }
                for (int c = 0; c < 16; ++c) if (p[c])
                    pairMarg[j * m + i].push_back({c, p[c]});
            }
        }
    }

    bool same(int i, int j) const {
        return vars[i].id >= 0 && vars[i].id == vars[j].id;
    }

    void evaluate(int a, int b, int c, int known1, int known2) {
        int num = c >= 0 ? 3 : (b >= 0 ? 2 : 1);
        int last = c >= 0 ? c : (b >= 0 ? b : a);
        int nf = (a >= old) + (b >= old) + (c >= old);
        if (nf && last - old + 1 != nf) return;

        int size = vars[a].size, product = vars[a].count, parts = 1;
        bool ab = b >= 0 && same(a, b), bc = c >= 0 && same(b, c);
        if (b >= 0 && !ab) {
            size += vars[b].size;
            product *= vars[b].count;
            ++parts;
        }
        if (c >= 0 && !bc) {
            size += vars[c].size;
            product *= vars[c].count;
            ++parts;
        }
        if (parts > 1 && (size > 7 || product > 256)) return;

        double p[16]{};
        unsigned char high[16]{}, low[16];
        memset(low, 63, sizeof(low));
        unsigned seen = 0;
        auto add = [&](int s, double w) {
            int r = tri[s];
            seen |= 1U << r;
            p[r] += w;
            high[r] |= s;
            low[r] &= s;
        };

        if (num == 1) {
            int code = (valueAt[known1] << 2) | (valueAt[known2] << 4);
            for (auto x : single[a]) add(code | x.code, x.p);
        } else if (num == 2) {
            int code = valueAt[known1] << 4;
            if (ab) {
                for (auto x : pairMarg[a * m + b]) add(code | x.code, x.p);
            } else {
                for (auto x : single[a]) for (auto y : single[b])
                    add(code | x.code | (y.code << 2), x.p * y.p);
            }
        } else if (ab && bc) {
            auto &s = units[vars[a].id].states;
            auto &w = weights[weightId[a]];
            int ia = 2 * vars[a].pos, ib = 2 * vars[b].pos, ic = 2 * vars[c].pos;
            for (int j = 0; j < int(s.size()); ++j) {
                int t = s[j];
                add(((t >> ia) & 3) | (((t >> ib) & 3) << 2) | (((t >> ic) & 3) << 4), w[j]);
            }
        } else if (ab) {
            for (auto x : pairMarg[a * m + b]) for (auto z : single[c])
                add(x.code | (z.code << 4), x.p * z.p);
        } else if (bc) {
            for (auto x : single[a]) for (auto y : pairMarg[b * m + c])
                add(x.code | (y.code << 2), x.p * y.p);
        } else {
            for (auto x : single[a]) for (auto y : single[b]) {
                int code = x.code | (y.code << 2);
                double w = x.p * y.p;
                for (auto z : single[c]) add(code | (z.code << 4), w * z.p);
            }
        }

        if (!(seen & (seen - 1))) return;
        double score = 0.04 * (num - nf) - 0.025 * max(0, size - max(3, num));
        while (seen) {
            int r = countr_zero(seen);
            seen &= seen - 1;
            int d = high[r] ^ low[r];
            int fixed = !(d & 3);
            if (num >= 2) fixed += !(d & 12);
            if (num == 3) fixed += !(d & 48);
            int grid = min(HSIZE, int(p[r] * HSIZE + 0.5));
            score += hTable[grid] + p[r] * (0.33 * fixed + 0.07 * (fixed == num));
        }
        if (score > best + 1e-9) {
            best = score;
            bestQuery = {vars[a].index, b >= 0 ? vars[b].index : known1,
                         c >= 0 ? vars[c].index : (b >= 0 ? known1 : known2)};
        }
    }

    Query choose(Query good) {
        vector<pair<int, int>> knownPairs;
        if (good[0] >= 0) knownPairs.emplace_back(good[0], good[1]);
        else {
            for (int x = 0; x < 4; ++x) if (!representatives[x].empty()) {
                if (representatives[x].size() == 2)
                    knownPairs.emplace_back(representatives[x][0], representatives[x][1]);
                for (int y = x + 1; y < 4; ++y) if (!representatives[y].empty())
                    knownPairs.emplace_back(representatives[x][0], representatives[y][0]);
            }
        }
        for (int i = 0; i < m; ++i) {
            for (auto [a, b] : knownPairs) evaluate(i, -1, -1, a, b);
            for (int j = i + 1; j < m; ++j) {
                for (auto &r : representatives) if (!r.empty())
                    evaluate(i, j, -1, r[0], -1);
                for (int k = j + 1; k < m; ++k)
                    evaluate(i, j, k, -1, -1);
            }
        }
        return bestQuery;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n < 3 || n > 5000) return 0;

    initTable();
    history.reserve(4 * n);
    valueAt.assign(n, -1);
    owner.assign(n, -2);

    if (n <= 7) {
        vector<int> idx(n), configs(1 << (2 * n));
        iota(idx.begin(), idx.end(), 0);
        iota(configs.begin(), configs.end(), 0);
        Exact(move(idx), move(configs)).run(n <= 5);
    }

    for (int i = 1; i <= HSIZE; ++i) {
        double p = double(i) / HSIZE;
        hTable[i] = -p * log2(p);
    }
    for (int i = 0; i < n; ++i) fresh.insert(i);
    bootstrap();

    while (knownCount < n) {
        if (n - knownCount <= 5) {
            int count = 1;
            for (int id : active) count *= int(units[id].states.size());
            for (int i : fresh) count *= 4;
            if (count <= 16) {
                vector<int> idx;
                for (int i = 0; i < n; ++i) if (valueAt[i] < 0) idx.push_back(i);
                Exact(idx, feasibleConfigs(idx, false)).run(true);
            }
        }
        Query good = findGoodPair();
        bool withFresh = active.size() < 9;
        Query q = Planner(withFresh).choose(good);
        if (q[0] < 0 && !withFresh && !fresh.empty())
            q = Planner(true).choose(good);
        if (q[0] >= 0) {
            int r = ask(q);
            apply(q, r);
        } else {
            recoverWithOnes();
        }
    }
    finish(true);
}
