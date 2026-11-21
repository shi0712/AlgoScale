#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 2) {
        cout << "! 1 2\n";
        cout.flush();
        return 0;
    }

    auto ask = [&](int d) -> pair<int, int> {
        cout << "? " << d << '\n';
        cout.flush();
        int v, u;
        if (!(cin >> v >> u)) exit(0);
        if (v == -1) exit(0);
        return {v, u};
    };

    vector<int> A, uA;       // peeled vertices and their reported non-neighbor
    vector<int> C, fC;       // core vertices and their reported non-neighbor
    vector<char> removed(n + 1, 0);

    int sz = n;

    auto find_remaining = [&]() -> int {
        for (int i = 1; i <= n; ++i) {
            if (!removed[i]) return i;
        }
        return -1;
    };

    // Phase 1: remove vertices whose current missing-degree is 0 or 1.
    while (sz > 0) {
        if (sz == 1) {
            int v = find_remaining();
            removed[v] = 1;
            A.push_back(v);
            uA.push_back(0);
            sz = 0;
            break;
        }

        int d = (sz >= 3 ? sz - 2 : 0);
        auto [v, u] = ask(d);

        if (v == 0) break; // only the 2-core remains

        removed[v] = 1;
        A.push_back(v);
        uA.push_back(u);
        --sz;
    }

    // Phase 2: remove the remaining 2-core by minimum degree in G (maximum missing-degree).
    while (sz > 0) {
        if (sz == 1) {
            int v = find_remaining();
            removed[v] = 1;
            C.push_back(v);
            fC.push_back(0);
            sz = 0;
            break;
        }

        auto [v, u] = ask(0);
        if (v == 0) break; // should not happen

        removed[v] = 1;
        C.push_back(v);
        fC.push_back(u);
        --sz;
    }

    int t = (int)A.size();
    int r = (int)C.size();

    vector<char> isCore(n + 1, 0);
    for (int v : C) isCore[v] = 1;

    vector<char> usedA(t, 0);

    // Dangerous core-core gaps in the reverse core order.
    vector<int> pot(max(0, r - 1), 0);
    for (int i = 0; i + 1 < r; ++i) {
        if (fC[i] != 0 && C[i + 1] >= fC[i]) pot[i] = 1;
    }

    vector<int> potGaps;
    for (int i = 0; i + 1 < r; ++i) {
        if (pot[i]) potGaps.push_back(i);
    }

    // Buffers that are safe to use inside the core path:
    // - u == 0: no missing edge to any later vertex;
    // - u is a core vertex: its only missing edge to the remaining graph is in the core.
    vector<int> poolU, poolN;
    for (int i = 0; i < t; ++i) {
        int u = uA[i];
        if (u == 0) {
            poolU.push_back(i);
        } else if (u >= 1 && u <= n && isCore[u]) {
            poolN.push_back(i);
        }
    }

    vector<int> assign(max(0, r - 1), -1);

    if (!potGaps.empty()) {
        // If there are more dangerous gaps than buffers, keep the latest ones
        // (they appear earliest in the reverse order and are the most critical).
        if ((int)potGaps.size() > (int)poolU.size() + (int)poolN.size()) {
            sort(potGaps.begin(), potGaps.end(), greater<int>());
            potGaps.resize(poolU.size() + poolN.size());
        }

        vector<int> cntN(n + 1, 0);
        for (int b : poolN) {
            ++cntN[uA[b]];
        }

        vector<int> order = potGaps;
        sort(order.begin(), order.end(), [&](int a, int b) {
            int xa = C[a + 1], ya = C[a];
            int xb = C[b + 1], yb = C[b];
            int sa = (int)poolN.size() - cntN[xa] - cntN[ya];
            int sb = (int)poolN.size() - cntN[xb] - cntN[yb];
            if (sa != sb) return sa < sb;
            return a > b; // prefer later gaps if tied
        });

        set<int> unusedU(poolU.begin(), poolU.end());
        set<int> unusedN(poolN.begin(), poolN.end());

        for (int gi : order) {
            int i = gi;
            int x = C[i + 1], y = C[i];
            int nonSafe = (int)poolN.size() - cntN[x] - cntN[y];

            int chosen = -1;

            if (nonSafe == 0) {
                if (!unusedU.empty()) {
                    chosen = *unusedU.begin();
                } else {
                    auto it = unusedN.begin();
                    while (it != unusedN.end()) {
                        int b = *it;
                        if (uA[b] != x && uA[b] != y) {
                            chosen = b;
                            break;
                        }
                        ++it;
                    }
                }
            } else {
                auto it = unusedN.begin();
                while (it != unusedN.end()) {
                    int b = *it;
                    if (uA[b] != x && uA[b] != y) {
                        chosen = b;
                        break;
                    }
                    ++it;
                }
                if (chosen == -1 && !unusedU.empty()) {
                    chosen = *unusedU.begin();
                }
            }

            if (chosen != -1) {
                assign[i] = chosen;
                if (uA[chosen] == 0) unusedU.erase(chosen);
                else unusedN.erase(chosen);
            }
        }
    }

    deque<int> path;

    if (r > 0) {
        // Reverse core order, inserting buffers into dangerous gaps.
        for (int i = r - 1; i >= 0; --i) {
            path.push_back(C[i]);
            if (i > 0 && pot[i - 1] && assign[i - 1] != -1) {
                int b = assign[i - 1];
                path.push_back(A[b]);
                usedA[b] = 1;
            }
        }
    } else {
        // No core: start with two safe peeled vertices (the last two universal ones).
        vector<int> uni;
        for (int i = 0; i < t; ++i) {
            if (uA[i] == 0) uni.push_back(i);
        }

        if (uni.size() >= 2) {
            int i1 = uni.back();
            int i2 = uni[uni.size() - 2];
            path.push_back(A[i1]);
            path.push_back(A[i2]);
            usedA[i1] = usedA[i2] = 1;
        } else if (t >= 2) {
            int i1 = t - 1, i2 = t - 2;
            path.push_back(A[i1]);
            path.push_back(A[i2]);
            usedA[i1] = usedA[i2] = 1;
        } else if (t == 1) {
            path.push_back(A[0]);
            usedA[0] = 1;
        }
    }

    // Insert all remaining peeled vertices in reverse order.
    // Each such vertex has at most one missing neighbor among already inserted vertices.
    for (int idx = t - 1; idx >= 0; --idx) {
        if (usedA[idx]) continue;

        int v = A[idx];
        int u = uA[idx];

        if (path.empty()) {
            path.push_back(v);
        } else if (path.size() == 1) {
            // Should not be a problem with the chosen base, but keep a safe fallback.
            if (u == 0 || path.front() != u) path.push_back(v);
            else path.push_back(v);
        } else {
            if (u == 0) {
                path.push_back(v);
            } else if (path.front() != u) {
                path.push_front(v);
            } else {
                path.push_back(v);
            }
        }
    }

    // Safety net: if something went wrong, append any missing labels.
    vector<char> inPath(n + 1, 0);
    for (int x : path) inPath[x] = 1;
    for (int i = 1; i <= n; ++i) {
        if (!inPath[i]) path.push_back(i);
    }

    cout << "!";
    for (int x : path) cout << ' ' << x;
    cout << '\n';
    cout.flush();

    return 0;
}
