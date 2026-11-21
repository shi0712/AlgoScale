#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<vector<int>> child(n + 1);
    vector<int> depth(n + 1, 0);
    int maxDepth = 0;

    for (int i = 2; i <= n; ++i) {
        int p;
        cin >> p;
        child[p].push_back(i);
        depth[i] = depth[p] + 1;
        maxDepth = max(maxDepth, depth[i]);
    }

    vector<int> tin(n + 1), tout(n + 1), order(n + 1);
    int timer = 0;
    vector<pair<int, int>> st;
    st.reserve(2 * n);
    st.push_back({1, 0});

    while (!st.empty()) {
        auto [v, state] = st.back();
        st.pop_back();

        if (state == 0) {
            tin[v] = ++timer;
            order[timer] = v;
            st.push_back({v, 1});
            for (int i = (int)child[v].size() - 1; i >= 0; --i) {
                st.push_back({child[v][i], 0});
            }
        } else {
            tout[v] = timer;
        }
    }

    vector<int> cnt(maxDepth + 1, 0), first(maxDepth + 1, 0);
    for (int i = 1; i <= n; ++i) {
        int d = depth[i];
        ++cnt[d];
        if (!first[d]) first[d] = i;
    }

    auto query = [&](int u, int k) -> int {
        cout << "? " << u << ' ' << k << '\n';
        cout.flush();

        int o;
        if (!(cin >> o)) exit(0);
        if (o == -1) exit(0);
        return o;
    };

    auto answer = [&](int u) {
        cout << "! " << u << '\n';
        cout.flush();
        exit(0);
    };

    // Phase 1: determine the depth of the hidden vertex.
    int L = 0, R = maxDepth;
    while (true) {
        if (L > R) return 0;

        int N = 0, nonZero = 0, onlyDepth = -1;
        for (int d = L; d <= R; ++d) {
            N += cnt[d];
            if (cnt[d] > 0) {
                ++nonZero;
                onlyDepth = d;
            }
        }

        if (N <= 0) return 0;

        if (N == 1) {
            answer(first[onlyDepth]);
        }

        if (nonZero == 1) {
            L = R = onlyDepth;
            break;
        }

        int pref = 0;
        int bestK = L;
        int bestMax = N;
        int cross = -1;

        for (int d = L; d <= R; ++d) {
            pref += cnt[d];

            if (d < R) {
                int mx = max(pref, N - pref);
                if (mx < bestMax) {
                    bestMax = mx;
                    bestK = d;
                }
            }

            if (cross == -1 && 2 * pref >= N) {
                cross = d;
            }
        }

        if (cross == -1) cross = R;

        if (3 * bestMax <= 2 * N) {
            int resp = query(1, bestK);
            if (resp == 1) R = bestK;
            else L = bestK + 1;
        } else {
            if (cross == L) {
                int resp = query(1, cross);
                if (resp == 1) {
                    L = R = cross;
                } else {
                    L = cross + 1;
                }
            } else if (cross == R) {
                int resp = query(1, cross - 1);
                if (resp == 1) {
                    R = cross - 1;
                } else {
                    L = R = cross;
                }
            } else {
                int resp = query(1, cross - 1);
                if (resp == 1) {
                    R = cross - 1;
                } else {
                    L = cross;
                }
            }
        }
    }

    int d = L;

    // Phase 2: all candidates have the same depth d.
    vector<int> active;
    active.reserve(cnt[d]);
    for (int i = 1; i <= n; ++i) {
        if (depth[i] == d) active.push_back(i);
    }

    int N = (int)active.size();
    if (N == 0) return 0;
    if (N == 1) answer(active[0]);

    vector<int> weight(n + 1, 0);

    while (N > 1) {
        fill(weight.begin(), weight.end(), 0);
        for (int x : active) weight[x] = 1;

        for (int idx = n; idx >= 1; --idx) {
            int v = order[idx];
            int w = weight[v];
            for (int c : child[v]) w += weight[c];
            weight[v] = w;
        }

        int best = 0;
        int bestMax = N;

        for (int v = 1; v <= n; ++v) {
            int w = weight[v];
            if (w > 0 && w < N) {
                int mx = max(w, N - w);
                if (mx < bestMax) {
                    bestMax = mx;
                    best = v;
                }
            }
        }

        if (best == 0) best = active[0];

        int k = d - depth[best];
        int resp = query(best, k);

        vector<int> nextActive;
        nextActive.reserve(N);

        if (resp == 1) {
            for (int x : active) {
                if (tin[best] <= tin[x] && tin[x] < tout[best]) {
                    nextActive.push_back(x);
                }
            }
        } else {
            for (int x : active) {
                if (!(tin[best] <= tin[x] && tin[x] < tout[best])) {
                    nextActive.push_back(x);
                }
            }
        }

        active.swap(nextActive);
        N = (int)active.size();

        if (N == 0) return 0;
    }

    answer(active[0]);
    return 0;
}
