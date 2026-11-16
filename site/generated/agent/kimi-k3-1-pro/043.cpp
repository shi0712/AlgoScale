#include <bits/stdc++.h>
using namespace std;

struct CrossEdge {
    int u, v, id;
};

struct BipartiteColorer {
    int m;
    vector<int> color;

    explicit BipartiteColorer(int m_) : m(m_) {}

    vector<int> perfectMatching(const vector<CrossEdge>& edges,
                                const vector<vector<int>>& adj) {
        vector<int> pairL(m, -1), pairR(m, -1), pairEdge(m, -1);
        for (int eid = 0; eid < (int)edges.size(); ++eid) {
            const auto& e = edges[eid];
            if (pairL[e.u] == -1 && pairR[e.v] == -1) {
                pairL[e.u] = e.v;
                pairR[e.v] = e.u;
                pairEdge[e.u] = eid;
            }
        }

        vector<int> dist(m), queueBuf(m);
        auto bfs = [&]() {
            int qb = 0, qe = 0;
            for (int u = 0; u < m; ++u) {
                if (pairL[u] == -1) {
                    dist[u] = 0;
                    queueBuf[qe++] = u;
                } else {
                    dist[u] = -1;
                }
            }
            bool found = false;
            while (qb < qe) {
                int u = queueBuf[qb++];
                for (int eid : adj[u]) {
                    int v = edges[eid].v;
                    int w = pairR[v];
                    if (w == -1) {
                        found = true;
                    } else if (dist[w] == -1) {
                        dist[w] = dist[u] + 1;
                        queueBuf[qe++] = w;
                    }
                }
            }
            return found;
        };

        function<bool(int)> dfs = [&](int u) {
            for (int eid : adj[u]) {
                int v = edges[eid].v;
                int w = pairR[v];
                if (w == -1 || (dist[w] == dist[u] + 1 && dfs(w))) {
                    pairL[u] = v;
                    pairR[v] = u;
                    pairEdge[u] = eid;
                    return true;
                }
            }
            dist[u] = -1;
            return false;
        };

        while (bfs()) {
            for (int u = 0; u < m; ++u) {
                if (pairL[u] == -1) dfs(u);
            }
        }

        vector<int> result;
        result.reserve(m);
        for (int u = 0; u < m; ++u) {
            if (pairL[u] == -1) {
                cerr << "Internal error: bipartite perfect matching not found\n";
                exit(1);
            }
            result.push_back(pairEdge[u]);
        }
        return result;
    }

    void solve(vector<CrossEdge> edges, int degree, int offset) {
        if (degree == 0) return;
        if (degree == 1) {
            for (const auto& e : edges) color[e.id] = offset;
            return;
        }

        int nEdges = edges.size();
        vector<vector<int>> adj(2 * m);
        for (int i = 0; i < 2 * m; ++i) adj[i].reserve(degree);
        for (int i = 0; i < nEdges; ++i) {
            adj[edges[i].u].push_back(i);
            adj[m + edges[i].v].push_back(i);
        }

        vector<char> used(nEdges, 0);
        if (degree & 1) {
            vector<int> matching = perfectMatching(edges, adj);
            for (int eid : matching) {
                used[eid] = 1;
                color[edges[eid].id] = offset + degree - 1;
            }
            --degree;
        }

        vector<CrossEdge> parts[2];
        parts[0].reserve(edges.size() / 2);
        parts[1].reserve(edges.size() / 2);
        vector<int> ptr(2 * m, 0);
        vector<pair<int, int>> st;
        vector<int> tour;

        for (int start = 0; start < 2 * m; ++start) {
            bool hasEdge = false;
            for (int eid : adj[start]) {
                if (!used[eid]) {
                    hasEdge = true;
                    break;
                }
            }
            if (!hasEdge) continue;

            st.clear();
            tour.clear();
            st.push_back({start, -1});
            while (!st.empty()) {
                int v = st.back().first;
                while (ptr[v] < (int)adj[v].size() && used[adj[v][ptr[v]]]) {
                    ++ptr[v];
                }
                if (ptr[v] < (int)adj[v].size()) {
                    int eid = adj[v][ptr[v]++];
                    used[eid] = 1;
                    int w = edges[eid].u + edges[eid].v + m - v;
                    st.push_back({w, eid});
                } else {
                    int incoming = st.back().second;
                    st.pop_back();
                    if (incoming != -1) tour.push_back(incoming);
                }
            }
            reverse(tour.begin(), tour.end());
            for (int i = 0; i < (int)tour.size(); ++i) {
                parts[i & 1].push_back(edges[tour[i]]);
            }
        }

        int half = degree / 2;
        vector<CrossEdge>().swap(edges);
        vector<vector<int>>().swap(adj);
        vector<char>().swap(used);
        solve(move(parts[0]), half, offset);
        solve(move(parts[1]), half, offset + half);
    }

    vector<int> run(const vector<pair<int, int>>& input, int degree) {
        color.assign(input.size(), -1);
        vector<CrossEdge> edges;
        edges.reserve(input.size());
        for (int i = 0; i < (int)input.size(); ++i) {
            edges.push_back({input[i].first, input[i].second, i});
        }
        solve(move(edges), degree, 0);
        return color;
    }
};

struct EvenConstructor {
    int n, m;
    const vector<string>& graph;
    vector<vector<int>> comp;
    vector<int> side, label;
    vector<int> A, B;
    vector<pair<int, int>> internalA, internalB;
    vector<vector<int>> colorA, colorB;
    vector<int> internalColorA, internalColorB;
    vector<vector<int>> answer;
    mt19937 rng{712367821};

    explicit EvenConstructor(const vector<string>& g)
        : n(g.size()), m(n / 2), graph(g), comp(n), side(n, -1), label(n),
          answer(n, vector<int>(n, -1)) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j && graph[i][j] == '0') comp[i].push_back(j);
            }
        }
    }

    void partitionCycles() {
        vector<char> visited(n, 0);
        int oddIndex = 0;
        for (int s = 0; s < n; ++s) {
            if (visited[s]) continue;
            vector<int> cycle;
            int prev = -1, v = s;
            while (!visited[v]) {
                visited[v] = 1;
                cycle.push_back(v);
                int next = comp[v][0] == prev ? comp[v][1] : comp[v][0];
                prev = v;
                v = next;
            }
            if (cycle.size() % 2 == 0) {
                for (int i = 0; i < (int)cycle.size(); ++i) {
                    side[cycle[i]] = i & 1;
                }
            } else {
                int majority = oddIndex++ & 1;
                for (int i = 0; i < (int)cycle.size(); ++i) {
                    side[cycle[i]] = (i % 2 == 0 ? majority : 1 - majority);
                }
            }
        }
        for (int v = 0; v < n; ++v) {
            (side[v] == 0 ? A : B).push_back(v);
        }
        for (int v = 0; v < n; ++v) {
            for (int w : comp[v]) {
                if (v < w && side[v] == side[w]) {
                    (side[v] == 0 ? internalA : internalB).push_back({v, w});
                }
            }
        }
    }

    static vector<vector<int>> completeColoring(int m) {
        vector<vector<int>> col(m, vector<int>(m, -1));
        if (m % 2 == 0) {
            int q = m - 1;
            for (int k = 0; k < q; ++k) {
                col[m - 1][k] = col[k][m - 1] = k;
                for (int i = 1; i <= (q - 1) / 2; ++i) {
                    int a = (k - i + q) % q;
                    int b = (k + i) % q;
                    col[a][b] = col[b][a] = k;
                }
            }
        } else {
            for (int k = 0; k < m; ++k) {
                for (int i = 1; i <= (m - 1) / 2; ++i) {
                    int a = (k - i + m) % m;
                    int b = (k + i) % m;
                    col[a][b] = col[b][a] = k;
                }
            }
        }
        return col;
    }

    vector<int> arrangeSide(const vector<int>& pool,
                            const vector<pair<int, int>>& internal) const {
        vector<int> result(m, -1);
        vector<char> usedVertex(n, 0);
        for (int i = 0; i < (int)internal.size(); ++i) {
            auto [u, v] = internal[i];
            result[2 * i] = u;
            result[2 * i + 1] = v;
            usedVertex[u] = usedVertex[v] = 1;
        }
        int at = 2 * internal.size();
        for (int u : pool) {
            if (!usedVertex[u]) result[at++] = u;
        }
        return result;
    }

    void relabel() {
        colorA = completeColoring(m);
        colorB = completeColoring(m);
        A = arrangeSide(A, internalA);
        B = arrangeSide(B, internalB);
        for (int i = 0; i < m; ++i) {
            label[A[i]] = i;
            label[B[i]] = i;
        }
        int colors = m % 2 == 0 ? m - 1 : m;
        internalColorA.assign(colors, -1);
        internalColorB.assign(colors, -1);
        for (int i = 0; i < (int)internalA.size(); ++i) {
            auto [u, v] = internalA[i];
            internalColorA[colorA[label[u]][label[v]]] = i;
        }
        for (int i = 0; i < (int)internalB.size(); ++i) {
            auto [u, v] = internalB[i];
            internalColorB[colorB[label[u]][label[v]]] = i;
        }
    }

    bool groupMatching(int ca, int cb, vector<pair<int, int>>* saved,
                       const vector<vector<unsigned char>>* reserved = nullptr) const {
        if (m & 1) {
            int u = A[ca], v = B[cb];
            if (graph[u][v] == '0') return false;
            if (saved) saved->push_back({u, v});
        }
        if (internalColorA[ca] == -1) return true;

        auto [a1, a2] = internalA[internalColorA[ca]];
        auto [b1, b2] = internalB[internalColorB[cb]];
        vector<int> left{a1, a2}, right{b1, b2};
        for (int type = 0; type < 2; ++type) {
            int p0 = type, p1 = 1 - type;
            bool ok = true;
            for (int i = 0; i < 2; ++i) {
                int p = i == 0 ? p0 : p1;
                if (graph[left[i]][right[p]] == '0' ||
                    (reserved && (*reserved)[label[left[i]]][label[right[p]]])) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                if (saved) {
                    saved->push_back({left[0], right[p0]});
                    saved->push_back({left[1], right[p1]});
                }
                return true;
            }
        }
        if (saved && (m & 1)) saved->pop_back();
        return false;
    }

    bool findColorPermutation(vector<int>& mapping) {
        int colors = m % 2 == 0 ? m - 1 : m;
        vector<vector<unsigned char>> allowed(colors, vector<unsigned char>(colors, 0));
        for (int ca = 0; ca < colors; ++ca) {
            for (int cb = 0; cb < colors; ++cb) {
                if ((internalColorA[ca] == -1) != (internalColorB[cb] == -1)) continue;
                if (internalColorA[ca] != -1) {
                    allowed[ca][cb] = groupMatching(ca, cb, nullptr);
                } else if (m % 2 == 0) {
                    allowed[ca][cb] = 1;
                } else {
                    allowed[ca][cb] = graph[A[ca]][B[cb]] == '1';
                }
            }
        }

        vector<int> matchR(colors, -1);
        function<bool(int, vector<char>&)> dfs = [&](int u, vector<char>& seen) {
            for (int v = 0; v < colors; ++v) {
                if (!allowed[u][v] || seen[v]) continue;
                seen[v] = 1;
                if (matchR[v] == -1 || dfs(matchR[v], seen)) {
                    matchR[v] = u;
                    return true;
                }
            }
            return false;
        };
        for (int u = 0; u < colors; ++u) {
            vector<char> seen(colors, 0);
            if (!dfs(u, seen)) return false;
        }
        mapping.assign(colors, -1);
        for (int v = 0; v < colors; ++v) mapping[matchR[v]] = v;
        return true;
    }

    vector<vector<int>> run() {
        partitionCycles();
        if ((int)A.size() != m || (int)B.size() != m ||
            internalA.size() != internalB.size()) {
            cerr << "Internal error: invalid cycle partition\n";
            exit(1);
        }

        vector<int> mapping;
        vector<vector<unsigned char>> reserved;
        bool ok = false;
        for (int attempt = 0; attempt < 100 && !ok; ++attempt) {
            if (attempt) {
                shuffle(A.begin(), A.end(), rng);
                shuffle(B.begin(), B.end(), rng);
                shuffle(internalA.begin(), internalA.end(), rng);
                shuffle(internalB.begin(), internalB.end(), rng);
            }
            relabel();
            if (!findColorPermutation(mapping)) continue;
            reserved.assign(m, vector<unsigned char>(m, 0));
            if (m & 1) {
                for (int c = 0; c < m; ++c) reserved[c][mapping[c]] = 1;
            }
            ok = true;
            for (int ca = 0; ca < (int)mapping.size(); ++ca) {
                if (!groupMatching(ca, mapping[ca], nullptr, &reserved)) {
                    ok = false;
                    break;
                }
            }
        }
        if (!ok) {
            cerr << "Internal error: could not align the two halves\n";
            exit(1);
        }

        int internalColors = m % 2 == 0 ? m - 1 : m;
        vector<int> inverse(internalColors);
        for (int c = 0; c < internalColors; ++c) inverse[mapping[c]] = c;
        for (int i = 0; i < m; ++i) {
            for (int j = i + 1; j < m; ++j) {
                if (graph[A[i]][A[j]] == '1') {
                    answer[A[i]][A[j]] = answer[A[j]][A[i]] = colorA[i][j];
                }
                if (graph[B[i]][B[j]] == '1') {
                    answer[B[i]][B[j]] = answer[B[j]][B[i]] = inverse[colorB[i][j]];
                }
            }
        }

        vector<vector<int>>().swap(colorA);
        vector<vector<int>>().swap(colorB);

        for (int ca = 0; ca < internalColors; ++ca) {
            vector<pair<int, int>> edges;
            groupMatching(ca, mapping[ca], &edges, &reserved);
            for (auto [u, v] : edges) {
                answer[u][v] = answer[v][u] = ca;
                reserved[label[u]][label[v]] = 1;
            }
        }

        int crossDegree = m - 2 - (m & 1);
        vector<pair<int, int>> crossEdges;
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j) {
                if (!reserved[i][j] && graph[A[i]][B[j]] == '1') {
                    crossEdges.push_back({i, j});
                }
            }
        }
        if (crossDegree > 0) {
            BipartiteColorer colorer(m);
            vector<int> colors = colorer.run(crossEdges, crossDegree);
            for (int i = 0; i < (int)crossEdges.size(); ++i) {
                auto [u, v] = crossEdges[i];
                int c = internalColors + colors[i];
                answer[A[u]][B[v]] = answer[B[v]][A[u]] = c;
            }
        }
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int originalN;
    cin >> originalN;
    vector<string> original(originalN);
    for (auto& row : original) cin >> row;

    int n = originalN;
    vector<string> graph = original;
    if (originalN & 1) {
        int a = -1, b = -1;
        for (int i = 0; i < originalN && a == -1; ++i) {
            for (int j = i + 1; j < originalN; ++j) {
                if (original[i][j] == '0') {
                    a = i;
                    b = j;
                    break;
                }
            }
        }
        n = originalN + 1;
        graph.assign(n, string(n, '1'));
        for (int i = 0; i < n; ++i) graph[i][i] = '0';
        for (int i = 0; i < originalN; ++i) {
            for (int j = 0; j < originalN; ++j) graph[i][j] = original[i][j];
        }
        graph[a][b] = graph[b][a] = '1';
        graph[a][originalN] = graph[originalN][a] = '0';
        graph[b][originalN] = graph[originalN][b] = '0';
    }

    EvenConstructor constructor(graph);
    vector<vector<int>> coloring = constructor.run();
    int C = originalN % 2 == 0 ? originalN - 3 : originalN - 2;

    cout << C << '\n';
    for (int i = 0; i < originalN; ++i) {
        for (int j = 0; j < originalN; ++j) {
            if (j) cout << ' ';
            if (original[i][j] == '1') cout << coloring[i][j] + 1;
            else cout << -1;
        }
        cout << '\n';
    }
    return 0;
}
