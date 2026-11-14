#include <bits/stdc++.h>
using namespace std;

class Random {
    uint64_t state;

public:
    Random() {
        state = chrono::steady_clock::now().time_since_epoch().count();
        state ^= uint64_t(reinterpret_cast<uintptr_t>(this));
    }

    uint64_t next() {
        uint64_t z = (state += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    int operator()(int bound) {
        return int(next() % uint64_t(bound));
    }

    template<class T>
    void shuffleVector(vector<T>& a) {
        for (int i = int(a.size()) - 1; i > 0; --i) {
            swap(a[i], a[(*this)(i + 1)]);
        }
    }
};

class Coloring {
    int n, colors;
    const vector<string>& adj;
    Random rng;

    // mate[v][c] is the other endpoint of the color-c edge at v,
    // or -1 if color c is missing at v.
    vector<int> mate;

    vector<array<int, 2>> missing;
    vector<int> missingCount;

    vector<pair<int, int>> holes;
    vector<int> path;

    int& at(int v, int c) {
        return mate[size_t(v) * colors + c];
    }

    void removeMissing(int v, int c) {
        for (int i = 0; i < missingCount[v]; ++i) {
            if (missing[v][i] == c) {
                missing[v][i] = missing[v][--missingCount[v]];
                return;
            }
        }
    }

    void exchangeMissing(int v, int a, int b) {
        for (int i = 0; i < missingCount[v]; ++i) {
            if (missing[v][i] == a) {
                missing[v][i] = b;
            } else if (missing[v][i] == b) {
                missing[v][i] = a;
            }
        }
    }

    void putEdge(int u, int v, int c) {
        at(u, c) = v;
        at(v, c) = u;
        removeMissing(u, c);
        removeMissing(v, c);
    }

    // a is missing at start. Build the alternating a,b path,
    // whose first edge, if present, has color b.
    int buildPath(int start, int a, int b) {
        path.clear();
        path.push_back(start);

        int v = start;
        int c = b;

        while (true) {
            int w = at(v, c);
            if (w == -1) return v;

            v = w;
            path.push_back(v);
            c = (c == a ? b : a);
        }
    }

    void swapBuiltPath(int a, int b) {
        if (path.size() <= 1) return;

        for (int v : path) {
            swap(at(v, a), at(v, b));
        }

        exchangeMissing(path.front(), a, b);
        exchangeMissing(path.back(), a, b);
    }

    void initialize() {
        fill(mate.begin(), mate.end(), -1);
        holes.clear();

        vector<int> label(n);
        iota(label.begin(), label.end(), 0);
        rng.shuffleVector(label);

        const int mod = n - 1;
        const int infinity = n - 1;

        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                if (adj[u][v] != '1') continue;

                int x = label[u];
                int y = label[v];
                int c;

                if (x == infinity) {
                    c = 2 * y % mod;
                } else if (y == infinity) {
                    c = 2 * x % mod;
                } else {
                    c = (x + y) % mod;
                }

                if (c < colors) {
                    at(u, c) = v;
                    at(v, c) = u;
                } else {
                    holes.emplace_back(u, v);
                }
            }
        }

        for (int v = 0; v < n; ++v) {
            missingCount[v] = 0;
            for (int c = 0; c < colors; ++c) {
                if (at(v, c) == -1) {
                    missing[v][missingCount[v]++] = c;
                }
            }
        }

        rng.shuffleVector(holes);
    }

    bool repair(int u, int v) {
        constexpr int ATTEMPT_LIMIT = 1024;

        for (int attempt = 0; attempt < ATTEMPT_LIMIT; ++attempt) {
            // First look for a color missing at both endpoints.
            for (int i = 0; i < missingCount[u]; ++i) {
                int c = missing[u][i];
                if (at(v, c) == -1) {
                    putEdge(u, v, c);
                    return true;
                }
            }

            int a = missing[u][rng(missingCount[u])];
            int b = missing[v][rng(missingCount[v])];

            int endpoint = buildPath(u, a, b);

            if (endpoint != v) {
                swapBuiltPath(a, b);
                putEdge(u, v, b);
                return true;
            }

            // The a,b path joins u and v. Change the partial coloring
            // with another Kempe interchange before trying again.
            int start;
            int absent;
            int otherAbsent;

            if (rng(2) == 0) {
                start = u;
                absent = a;
                otherAbsent = b;
            } else {
                start = v;
                absent = b;
                otherAbsent = a;
            }

            int c = -1;

            // Prefer a third color that is actually present at start.
            for (int tries = 0; tries < 12; ++tries) {
                int candidate = rng(colors);
                if (candidate != absent &&
                    candidate != otherAbsent &&
                    at(start, candidate) != -1) {
                    c = candidate;
                    break;
                }
            }

            if (c == -1) {
                // There is always a present color here, since there
                // was no common missing color.
                do {
                    c = rng(colors);
                } while (at(start, c) == -1);
            }

            buildPath(start, absent, c);
            swapBuiltPath(absent, c);
        }

        return false;
    }

public:
    Coloring(const vector<string>& graph)
        : n(int(graph.size())),
          colors(n - 3),
          adj(graph),
          mate(size_t(n) * colors, -1),
          missing(n),
          missingCount(n) {
        path.reserve(n);
        holes.reserve(n);
    }

    void solve() {
        while (true) {
            initialize();

            bool success = true;

            for (auto [u, v] : holes) {
                if (!repair(u, v)) {
                    success = false;
                    break;
                }
            }

            if (success) return;
        }
    }

    int numberOfColors() const {
        return colors;
    }

    void getRow(int v, vector<int>& row) const {
        fill(row.begin(), row.end(), -1);

        size_t base = size_t(v) * colors;
        for (int c = 0; c < colors; ++c) {
            int w = mate[base + c];
            if (w != -1) row[w] = c + 1;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int originalN;
    cin >> originalN;

    vector<string> original(originalN);
    for (string& s : original) cin >> s;

    vector<string> graph = original;

    if (originalN % 2 == 1) {
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

        // Subdivide complement edge ab using the new vertex.
        graph[a][b] = graph[b][a] = '1';

        for (string& s : graph) s.push_back('1');
        graph.push_back(string(originalN + 1, '1'));

        int x = originalN;
        graph[x][x] = '0';
        graph[a][x] = graph[x][a] = '0';
        graph[b][x] = graph[x][b] = '0';
    }

    Coloring solver(graph);
    solver.solve();

    cout << solver.numberOfColors() << '\n';

    vector<int> row(graph.size());
    string buffer;
    buffer.reserve(1 << 20);

    auto appendInteger = [&](int x) {
        char temp[16];
        auto result = to_chars(temp, temp + sizeof(temp), x);
        buffer.append(temp, result.ptr);
    };

    for (int i = 0; i < originalN; ++i) {
        solver.getRow(i, row);

        for (int j = 0; j < originalN; ++j) {
            if (j) buffer.push_back(' ');

            if (original[i][j] == '1') {
                appendInteger(row[j]);
            } else {
                buffer += "-1";
            }
        }
        buffer.push_back('\n');

        if (buffer.size() >= (1 << 20)) {
            cout.write(buffer.data(), streamsize(buffer.size()));
            buffer.clear();
        }
    }

    if (!buffer.empty()) {
        cout.write(buffer.data(), streamsize(buffer.size()));
    }

    return 0;
}
