#include <bits/stdc++.h>
using namespace std;

class Solver {
    int n;
    unordered_map<string, int> cache;

    static vector<int> mergeSets(const vector<int>& a, const vector<int>& b) {
        vector<int> result;
        result.reserve(a.size() + b.size());
        set_union(a.begin(), a.end(), b.begin(), b.end(),
                  back_inserter(result));
        return result;
    }

    static pair<vector<int>, vector<int>> splitSet(const vector<int>& s) {
        size_t mid = max<size_t>(1, s.size() / 2);
        vector<int> a(s.begin(), s.begin() + mid);
        vector<int> b(s.begin() + mid, s.end());
        return {move(a), move(b)};
    }

    int ask(const vector<int>& vertices) {
        cout << "? " << vertices.size() << '\n';
        for (int i = 0; i < (int)vertices.size(); ++i) {
            if (i) cout << ' ';
            cout << vertices[i] + 1;
        }
        cout << '\n' << flush;

        int answer;
        if (!(cin >> answer) || answer < 0) exit(0);
        return answer;
    }

    int induced(const vector<int>& vertices) {
        if (vertices.size() <= 1) return 0;

        string key((n + 7) / 8, '\0');
        for (int v : vertices) {
            key[v >> 3] = char(static_cast<unsigned char>(key[v >> 3]) |
                               (1u << (v & 7)));
        }

        auto it = cache.find(key);
        if (it != cache.end()) return it->second;

        int answer = ask(vertices);
        cache.emplace(move(key), answer);
        return answer;
    }

    // There is guaranteed to be an edge between a and b.
    // Returns a vertex of a incident to such an edge.
    int findEndpoint(vector<int> a, const vector<int>& b, int mB) {
        while (a.size() > 1) {
            auto [a1, a2] = splitSet(a);

            int mA1 = induced(a1);
            int crossing = induced(mergeSets(a1, b)) - mA1 - mB;

            if (crossing > 0) {
                a = move(a1);
            } else {
                a = move(a2);
            }
        }
        return a[0];
    }

    // The vertex a is guaranteed to have a neighbor in b.
    int findNeighbor(int a, vector<int> b) {
        while (b.size() > 1) {
            auto [b1, b2] = splitSet(b);

            int mB1 = induced(b1);
            int crossing = induced(mergeSets(vector<int>{a}, b1)) - mB1;

            if (crossing > 0) {
                b = move(b1);
            } else {
                b = move(b2);
            }
        }
        return b[0];
    }

    // Finds an edge with one endpoint in a and the other in b.
    pair<int, int> directedCrossingEdge(const vector<int>& a,
                                         const vector<int>& b) {
        int mB = induced(b);
        int x = findEndpoint(a, b, mB);
        int y = findNeighbor(x, b);
        return {x, y};
    }

    pair<int, int> crossingEdge(const vector<int>& a,
                                 const vector<int>& b) {
        if (a.size() <= b.size()) {
            return directedCrossingEdge(a, b);
        }
        auto [x, y] = directedCrossingEdge(b, a);
        return {y, x};
    }

    // The induced subgraph on s contains at least one edge.
    pair<int, int> findAnyEdge(const vector<int>& s) {
        auto [a, b] = splitSet(s);

        int mA = induced(a);
        if (mA > 0) return findAnyEdge(a);

        int mB = induced(b);
        if (mB > 0) return findAnyEdge(b);

        return crossingEdge(a, b);
    }

public:
    explicit Solver(int n) : n(n) {
        cache.max_load_factor(0.7f);
        cache.reserve(100000);
    }

    void solve() {
        vector<int> tree{0};
        vector<int> outside;
        for (int v = 1; v < n; ++v) outside.push_back(v);

        vector<int> parent(n, -1);
        vector<int> depth(n, 0);

        while (!outside.empty()) {
            auto [u, v] = crossingEdge(tree, outside);

            parent[v] = u;
            depth[v] = depth[u] + 1;

            tree.insert(lower_bound(tree.begin(), tree.end(), v), v);
            outside.erase(lower_bound(outside.begin(), outside.end(), v));
        }

        vector<int> side[2];
        for (int v = 0; v < n; ++v) {
            side[depth[v] & 1].push_back(v);
        }

        if (n == 1) {
            cout << "Y 1\n1\n" << flush;
            return;
        }

        int inside[2];
        inside[0] = induced(side[0]);
        inside[1] = induced(side[1]);

        if (inside[0] == 0 && inside[1] == 0) {
            const vector<int>& answer = side[0].size() >= side[1].size()
                                            ? side[0]
                                            : side[1];
            cout << "Y " << answer.size() << '\n';
            for (int i = 0; i < (int)answer.size(); ++i) {
                if (i) cout << ' ';
                cout << answer[i] + 1;
            }
            cout << '\n' << flush;
            return;
        }

        int badSide = inside[0] > 0 ? 0 : 1;
        auto [u, v] = findAnyEdge(side[badSide]);

        vector<int> ancestorsU;
        for (int x = u; x != -1; x = parent[x]) {
            ancestorsU.push_back(x);
        }

        vector<int> ancestorsV;
        for (int x = v; x != -1; x = parent[x]) {
            ancestorsV.push_back(x);
        }

        vector<char> isAncestorU(n, false);
        for (int x : ancestorsU) isAncestorU[x] = true;

        int lca = -1;
        int lcaIndexV = -1;
        for (int i = 0; i < (int)ancestorsV.size(); ++i) {
            if (isAncestorU[ancestorsV[i]]) {
                lca = ancestorsV[i];
                lcaIndexV = i;
                break;
            }
        }

        vector<int> cycle;
        for (int x = u; x != lca; x = parent[x]) {
            cycle.push_back(x);
        }
        cycle.push_back(lca);
        for (int i = lcaIndexV - 1; i >= 0; --i) {
            cycle.push_back(ancestorsV[i]);
        }

        cout << "N " << cycle.size() << '\n';
        for (int i = 0; i < (int)cycle.size(); ++i) {
            if (i) cout << ' ';
            cout << cycle[i] + 1;
        }
        cout << '\n' << flush;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    Solver solver(n);
    solver.solve();
    return 0;
}
