#include <bits/stdc++.h>
using namespace std;

struct Finished {};

class Solver {
    using U64 = uint64_t;
    const int dr[4] = {-1, 0, 1, 0};
    const int dc[4] = {0, 1, 0, -1};
    // Opposite directions are consecutive in this cyclic order.
    const int order[4] = {0, 2, 1, 3};
    const int rankDir[4] = {0, 2, 1, 3};

    vector<string> a;
    int R, C, A, W, S;
    vector<int> component, goal, classSize;
    array<vector<int>, 4> cls;
    array<vector<U64>, 4> possible;
    vector<vector<U64>> observationMask;
    vector<U64> goodMask;

    // Local coordinates: initial facing direction is north.
    vector<unsigned char> known, visited; // known: 0 unknown, 1 wall, 2 open
    int pos, heading = 0, distanceAhead = 0;
    bool navigating = false;
    function<int(const string&)> ask;
    function<void(bool, int, int)> report;

    pair<int, int> rotatePoint(int x, int y, int g) const {
        // Rotate counterclockwise by g * 90 degrees.
        while (g--) {
            int z = x;
            x = -y;
            y = z;
        }
        return {x, y};
    }

    int neighbour(int v, int d) const {
        int x = v / C + dr[d], y = v % C + dc[d];
        if (x < 0 || x >= R || y < 0 || y >= C || a[x][y] == '#')
            return -1;
        return x * C + y;
    }

    void preprocess() {
        vector<vector<int>> cells;
        component.assign(A, -1);
        for (int v = 0; v < A; ++v) {
            if (a[v / C][v % C] == '#' || component[v] != -1) continue;
            int id = (int)cells.size();
            cells.push_back({v});
            component[v] = id;
            for (size_t i = 0; i < cells[id].size(); ++i) {
                int u = cells[id][i];
                for (int d = 0; d < 4; ++d) {
                    int w = neighbour(u, d);
                    if (w != -1 && component[w] == -1) {
                        component[w] = id;
                        cells[id].push_back(w);
                    }
                }
            }
        }

        // Exact normalized shapes; no randomized hashes.
        map<vector<pair<int, int>>, int> shapeIDs;
        map<tuple<int, int, int>, int> stateIDs;
        int K = (int)cells.size();
        vector<array<int, 4>> shape(K);
        vector<int> canonical(K);
        for (auto &v : cls) v.assign(A, -1);

        for (int id = 0; id < K; ++id) {
            for (int g = 0; g < 4; ++g) {
                vector<pair<int, int>> p;
                int mnx = INT_MAX, mny = INT_MAX;
                for (int v : cells[id]) {
                    auto [x, y] = rotatePoint(v / C, v % C, g);
                    p.push_back({x, y});
                    mnx = min(mnx, x);
                    mny = min(mny, y);
                }
                for (auto &[x, y] : p) x -= mnx, y -= mny;
                sort(p.begin(), p.end());
                auto it = shapeIDs.find(p);
                int sid;
                if (it == shapeIDs.end()) {
                    sid = (int)shapeIDs.size();
                    shapeIDs.emplace(move(p), sid);
                } else sid = it->second;
                shape[id][g] = sid;

                for (int v : cells[id]) {
                    auto [x, y] = rotatePoint(v / C, v % C, g);
                    auto key = make_tuple(sid, x - mnx, y - mny);
                    auto jt = stateIDs.find(key);
                    int label;
                    if (jt == stateIDs.end()) {
                        label = (int)classSize.size();
                        stateIDs.emplace(key, label);
                        classSize.push_back(0);
                    } else label = jt->second;
                    cls[g][v] = label;
                    ++classSize[label];
                }
            }
            canonical[id] = *min_element(shape[id].begin(), shape[id].end());
        }

        vector<int> copies(shapeIDs.size(), 0);
        for (int x : canonical) ++copies[x];
        goal.assign(K, -1);
        goodMask.assign(W, 0);
        for (int id = 0; id < K; ++id) {
            bool good = copies[canonical[id]] == 1;
            bool symmetric = shape[id][0] == shape[id][2];
            if (symmetric) {
                int lx = R, rx = -1, ly = C, ry = -1;
                for (int v : cells[id]) {
                    lx = min(lx, v / C); rx = max(rx, v / C);
                    ly = min(ly, v % C); ry = max(ry, v % C);
                }
                if ((lx + rx) % 2 || (ly + ry) % 2) good = false;
                else {
                    int center = (lx + rx) / 2 * C + (ly + ry) / 2;
                    if (component[center] != id) good = false;
                    else goal[id] = center;
                }
            }
            if (good)
                for (int v : cells[id]) goodMask[v >> 6] |= U64(1) << (v & 63);
        }

        array<vector<int>, 4> dist;
        for (auto &v : dist) v.assign(A, 0);
        for (int v = 0; v < A; ++v) if (component[v] != -1) {
            int u = neighbour(v, 0);
            if (u != -1) dist[0][v] = dist[0][u] + 1;
            u = neighbour(v, 3);
            if (u != -1) dist[3][v] = dist[3][u] + 1;
        }
        for (int v = A - 1; v >= 0; --v) if (component[v] != -1) {
            int u = neighbour(v, 1);
            if (u != -1) dist[1][v] = dist[1][u] + 1;
            u = neighbour(v, 2);
            if (u != -1) dist[2][v] = dist[2][u] + 1;
        }
        observationMask.assign(400, vector<U64>(W, 0));
        for (auto &v : possible) v.assign(W, 0);
        for (int v = 0; v < A; ++v) if (component[v] != -1) {
            U64 bit = U64(1) << (v & 63);
            for (int g = 0; g < 4; ++g) {
                possible[g][v >> 6] |= bit;
                observationMask[g * 100 + dist[g][v]][v >> 6] |= bit;
            }
        }
    }

    void shift(vector<U64> &v, int delta) {
        int t = abs(delta), whole = t >> 6, bits = t & 63;
        if (delta > 0) {
            for (int i = W - 1; i >= 0; --i) {
                U64 x = 0;
                if (i >= whole) x = v[i - whole] << bits;
                if (bits && i > whole) x |= v[i - whole - 1] >> (64 - bits);
                v[i] = x;
            }
        } else {
            for (int i = 0; i < W; ++i) {
                U64 x = 0;
                if (i + whole < W) x = v[i + whole] >> bits;
                if (bits && i + whole + 1 < W)
                    x |= v[i + whole + 1] << (64 - bits);
                v[i] = x;
            }
        }
    }

    [[noreturn]] void finish(bool yes, int v = -1) {
        report(yes, yes ? v / C : -1, yes ? v % C : -1);
        throw Finished{};
    }

    // Find a shortest command sequence to the center in a known hypothesis.
    void goToCenter(int g, int v) {
        int target = goal[component[v]];
        if (target < 0) throw logic_error("Missing localization center");
        navigating = true;
        int start = 4 * v + ((g + heading) & 3);
        vector<int> parent(4 * A, -1), action(4 * A, -1), q;
        q.reserve(4 * A);
        parent[start] = start;
        q.push_back(start);
        int last = -1;
        for (size_t i = 0; i < q.size(); ++i) {
            int s = q[i], p = s / 4, d = s & 3;
            if (p == target) { last = s; break; }
            int forward = neighbour(p, d);
            int next[3] = {4 * p + ((d + 3) & 3),
                           4 * p + ((d + 1) & 3),
                           forward == -1 ? -1 : 4 * forward + d};
            for (int t = 0; t < 3; ++t) {
                int ns = next[t];
                if (ns != -1 && parent[ns] == -1) {
                    parent[ns] = s;
                    action[ns] = t;
                    q.push_back(ns);
                }
            }
        }
        if (last < 0) throw logic_error("Unreachable localization center");
        vector<int> path;
        for (int s = last; s != start; s = parent[s]) path.push_back(action[s]);
        reverse(path.begin(), path.end());
        for (int t : path) command(t);
        throw logic_error("Center should have produced a unique position");
    }

    void observe(int d) {
        if (d < 0 || d >= 100) throw Finished{};
        distanceAhead = d;
        int delta = dr[heading] * S + dc[heading], v = pos;
        known[v] = 2;
        for (int k = 0; k < d; ++k) { v += delta; known[v] = 2; }
        known[v + delta] = 1;

        int total = 0, firstG = -1, firstV = -1;
        bool hasGood = false;
        for (int g = 0; g < 4; ++g) {
            const auto &mask = observationMask[((g + heading) & 3) * 100 + d];
            for (int i = 0; i < W; ++i) {
                U64 x = (possible[g][i] &= mask[i]);
                total += __builtin_popcountll(x);
                hasGood |= (x & goodMask[i]) != 0;
                if (x && firstG == -1) {
                    firstG = g;
                    firstV = 64 * i + __builtin_ctzll(x);
                }
            }
        }
        if (!total) throw logic_error("No consistent state");

        int unique = -1;
        bool multiple = false;
        for (int i = 0; i < W; ++i) {
            U64 x = possible[0][i] | possible[1][i] |
                    possible[2][i] | possible[3][i];
            if (!x) continue;
            if (unique != -1 || (x & (x - 1))) { multiple = true; break; }
            unique = 64 * i + __builtin_ctzll(x);
        }
        if (!multiple) finish(true, unique);
        if (!hasGood) finish(false);

        // A belief is always a union of complete observational equivalence classes.
        if (!navigating && total == classSize[cls[firstG][firstV]])
            goToCenter(firstG, firstV);
    }

    // 0 = left, 1 = right, 2 = step.
    void command(int t) {
        static const string name[3] = {"left", "right", "step"};
        ++commands;
        if (t < 2) {
            ++turns;
            heading = (heading + (t == 0 ? 3 : 1)) & 3;
        } else {
            if (distanceAhead == 0) throw logic_error("Attempted wall collision");
            ++steps;
            pos += dr[heading] * S + dc[heading];
            for (int g = 0; g < 4; ++g) {
                int d = (g + heading) & 3;
                shift(possible[g], dr[d] * C + dc[d]);
            }
        }
        observe(ask(name[t]));
    }

    bool certainlyWall(int d) const {
        for (int g = 0; g < 4; ++g) {
            const auto &mask = observationMask[((g + d) & 3) * 100];
            for (int i = 0; i < W; ++i)
                if (possible[g][i] & ~mask[i]) return false;
        }
        return true;
    }

    void face(int d) {
        int t = (d - heading + 4) & 3;
        if (t == 3) command(0);
        else while (t--) command(1);
    }

public:
    long long commands = 0, steps = 0, turns = 0, wallProbes = 0, visitedCount = 0;

    Solver(vector<string> grid, function<int(const string&)> query,
           function<void(bool, int, int)> answer)
        : a(move(grid)), ask(move(query)), report(move(answer)) {
        R = (int)a.size(); C = (int)a[0].size(); A = R * C;
        W = (A + 63) / 64;
        S = 2 * max(R, C) + 3;
        int origin = max(R, C) + 1;
        pos = origin * S + origin;
        known.assign(S * S, 0);
        visited.assign(S * S, 0);
        preprocess();
    }

    void solve(int firstDistance) {
        try {
            observe(firstDistance);
            struct Frame { int entry, next; };
            vector<Frame> st;
            st.push_back({0, 0});
            visited[pos] = 1;
            ++visitedCount;
            while (!st.empty()) {
                if (st.back().next == 4) {
                    if (st.size() == 1) break;
                    int back = (st.back().entry + 2) & 3;
                    face(back);
                    st.pop_back();
                    command(2);
                    continue;
                }
                int back = (st.back().entry + 2) & 3;
                int k = ++st.back().next;
                int d = order[(rankDir[back] + k) & 3];
                int next = pos + dr[d] * S + dc[d];
                if (visited[next] || known[next] == 1) continue;
                if (known[next] == 0 && certainlyWall(d)) {
                    known[next] = 1;
                    continue;
                }
                face(d);
                if (distanceAhead == 0) { ++wallProbes; continue; }
                command(2);
                visited[pos] = 1;
                ++visitedCount;
                st.push_back({d, 0});
            }
            throw logic_error("A complete exploration must determine an equivalence class");
        } catch (const Finished&) {
            return;
        }
    }
};

#ifndef LOCAL_TEST
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int r, c;
    if (!(cin >> r >> c)) return 0;
    vector<string> a(r);
    for (auto &s : a) cin >> s;
    int firstDistance;
    if (!(cin >> firstDistance) || firstDistance < 0) return 0;
    auto ask = [](const string &s) -> int {
        cout << s << endl;
        int d;
        if (!(cin >> d) || d < 0) throw Finished{};
        return d;
    };
    auto report = [](bool yes, int x, int y) {
        if (yes) cout << "yes " << x + 1 << ' ' << y + 1 << endl;
        else cout << "no" << endl;
    };
    Solver solver(move(a), ask, report);
    solver.solve(firstDistance);
    return 0;
}
#endif
