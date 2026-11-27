#include <bits/stdc++.h>
using namespace std;

using ull = unsigned long long;

static inline ull mix64(ull x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

class GroupMap {
    vector<ull> keys;
    vector<int> values;
    size_t mask;

public:
    explicit GroupMap(int n) {
        size_t capacity = 1;
        while (capacity < 4ULL * max(1, n)) capacity <<= 1;
        keys.resize(capacity);
        values.assign(capacity, -1);
        mask = capacity - 1;
    }

    int getOrCreate(ull key, int groupId) {
        size_t pos = mix64(key) & mask;
        while (values[pos] != -1) {
            if (keys[pos] == key) return values[pos];
            pos = (pos + 1) & mask;
        }
        keys[pos] = key;
        values[pos] = groupId;
        return groupId;
    }

    int find(ull key) const {
        size_t pos = mix64(key) & mask;
        while (values[pos] != -1) {
            if (keys[pos] == key) return values[pos];
            pos = (pos + 1) & mask;
        }
        return -1;
    }
};

struct Group {
    ull mask = 0;
    vector<int> vertices;
    int pairHits = 0;
    int firstA = -1;
    int firstB = -1;
    int singletonOwner = -1;
    bool ambiguous = false;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    auto readResponse = [&](string &response) -> bool {
        response.resize(n);
        for (int i = 0; i < n; ++i) {
            char c;
            if (!(cin >> c)) return false;
            if (c == '-' || (c != '0' && c != '1')) return false;
            response[i] = c;
        }
        return true;
    };

    auto ask = [&](const vector<int> &d, string &response) -> bool {
        cout << "?";
        for (int x : d) cout << ' ' << x;
        cout << '\n' << flush;
        return readResponse(response);
    };

    vector<vector<unsigned char>> seen(n, vector<unsigned char>(n, 0));
    vector<vector<int>> adjacency(n);
    int edgeCount = 0;

    auto addEdge = [&](int u, int v) {
        if (u == v) return;
        if (seen[u][v]) return;
        seen[u][v] = seen[v][u] = 1;
        adjacency[u].push_back(v);
        adjacency[v].push_back(u);
        ++edgeCount;
    };

    if (n < 55) {
        vector<int> order(n - 1);
        iota(order.begin(), order.end(), 0);

        uint64_t seed = chrono::steady_clock::now().time_since_epoch().count();
        mt19937_64 rng(seed);
        shuffle(order.begin(), order.end(), rng);

        vector<int> d(n, 0);
        string response;

        for (int center : order) {
            fill(d.begin(), d.end(), 0);
            d[center] = 1;

            if (!ask(d, response)) return 0;

            for (int v = 0; v < n; ++v) {
                if (response[v] == '1') addEdge(center, v);
            }

            if (edgeCount == n - 1) break;
        }
    } else {
        int lg = 0;
        while ((1 << lg) < n) ++lg;
        int m = max(28, 4 * lg);

        uint64_t seed = chrono::steady_clock::now().time_since_epoch().count();
        mt19937_64 rng(seed);
        ull codeMask = (1ULL << m) - 1;

        vector<ull> code(n);
        unordered_set<ull> used;
        used.reserve(4 * n);

        for (int i = 0; i < n; ++i) {
            while (true) {
                ull x = rng() & codeMask;
                if (used.insert(x).second) {
                    code[i] = x;
                    break;
                }
            }
        }

        unordered_map<ull, int> codeOwner;
        codeOwner.reserve(4 * n);
        for (int i = 0; i < n; ++i) codeOwner.emplace(code[i], i);

        vector<ull> responseMask(n, 0);
        vector<int> d(n, 0);
        string response;

        for (int bit = 0; bit < m; ++bit) {
            for (int i = 0; i < n; ++i) {
                d[i] = static_cast<int>((code[i] >> bit) & 1ULL);
            }

            if (!ask(d, response)) return 0;

            for (int i = 0; i < n; ++i) {
                if (response[i] == '1') {
                    responseMask[i] |= (1ULL << bit);
                }
            }
        }

        GroupMap groupMap(n);
        vector<Group> groups;
        groups.reserve(n);

        for (int i = 0; i < n; ++i) {
            int oldSize = static_cast<int>(groups.size());
            int id = groupMap.getOrCreate(responseMask[i], oldSize);
            if (id == oldSize) {
                groups.emplace_back();
                groups.back().mask = responseMask[i];
            }
            groups[id].vertices.push_back(i);
        }

        for (Group &g : groups) {
            auto it = codeOwner.find(g.mask);
            if (it != codeOwner.end()) g.singletonOwner = it->second;
        }

        for (int a = 0; a < n; ++a) {
            for (int b = a + 1; b < n; ++b) {
                ull united = code[a] | code[b];
                int id = groupMap.find(united);
                if (id == -1) continue;

                Group &g = groups[id];
                ++g.pairHits;
                if (g.pairHits == 1) {
                    g.firstA = a;
                    g.firstB = b;
                } else {
                    g.ambiguous = true;
                }
            }
        }

        bool haveCandidate = false;
        for (const Group &g : groups) {
            if (g.pairHits == 1 && g.singletonOwner == -1 && !g.ambiguous) {
                haveCandidate = true;
                break;
            }
        }

        if (haveCandidate) {
            for (int a = 0; a < n; ++a) {
                for (int b = a + 1; b < n; ++b) {
                    ull first = code[a] | code[b];
                    for (int c = b + 1; c < n; ++c) {
                        ull united = first | code[c];
                        int id = groupMap.find(united);
                        if (id == -1) continue;

                        Group &g = groups[id];
                        if (g.pairHits == 1 &&
                            g.singletonOwner == -1 &&
                            !g.ambiguous) {
                            g.ambiguous = true;
                        }
                    }
                }
            }
        }

        vector<int> exactQueries;
        for (int i = 0; i < n; ++i) {
            const Group &g = groups[groupMap.find(responseMask[i])];

            if (g.pairHits == 0 && g.singletonOwner != -1) {
                addEdge(i, g.singletonOwner);
            } else if (g.pairHits == 1 &&
                       g.singletonOwner == -1 &&
                       !g.ambiguous) {
                addEdge(i, g.firstA);
                addEdge(i, g.firstB);
            } else {
                exactQueries.push_back(i);
            }
        }

        for (int center : exactQueries) {
            fill(d.begin(), d.end(), 0);
            d[center] = 1;

            if (!ask(d, response)) return 0;

            for (int v = 0; v < n; ++v) {
                if (response[v] == '1') addEdge(center, v);
            }
        }

        if (edgeCount != n - 1) {
            for (int center = 0; center < n; ++center) {
                fill(d.begin(), d.end(), 0);
                d[center] = 1;

                if (!ask(d, response)) return 0;

                for (int v = 0; v < n; ++v) {
                    if (response[v] == '1') addEdge(center, v);
                }
            }
        }
    }

    cout << "!\n";
    for (int u = 0; u < n; ++u) {
        for (int v : adjacency[u]) {
            if (u < v) {
                cout << u + 1 << ' ' << v + 1 << '\n';
            }
        }
    }
    cout << flush;

    return 0;
}
