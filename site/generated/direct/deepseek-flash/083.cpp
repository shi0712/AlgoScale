#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n;
        cin >> n;

        vector<int> parent(n + 1, 0), depth(n + 1, 0);
        vector<vector<int>> children(n + 1);

        depth[1] = 0;
        int maxDepth = 0;

        for (int i = 2; i <= n; ++i) {
            int p;
            cin >> p;
            parent[i] = p;
            depth[i] = depth[p] + 1;
            maxDepth = max(maxDepth, depth[i]);
            children[p].push_back(i);
        }

        // DFS for subtree intervals.
        vector<int> tin(n + 1), tout(n + 1);
        int timer = 0;
        stack<pair<int, int>> st;
        st.push({1, 0});

        while (!st.empty()) {
            auto [v, state] = st.top();
            st.pop();

            if (state == 0) {
                tin[v] = timer++;
                st.push({v, 1});

                for (int i = (int)children[v].size() - 1; i >= 0; --i) {
                    st.push({children[v][i], 0});
                }
            } else {
                tout[v] = timer;
            }
        }

        vector<int> cand(n);
        iota(cand.begin(), cand.end(), 1);

        int queries = 0;

        auto ask = [&](int u, int k) -> int {
            cout << "? " << u << " " << k << endl;
            int res;
            cin >> res;
            ++queries;
            return res;
        };

        while ((int)cand.size() > 1) {
            int m = (int)cand.size();

            int minD = INT_MAX, maxD = -1;
            for (int x : cand) {
                minD = min(minD, depth[x]);
                maxD = max(maxD, depth[x]);
            }

            if (minD == maxD) {
                // All remaining candidates are on the same depth.
                int d = minD;

                while ((int)cand.size() > 1) {
                    int m2 = (int)cand.size();

                    vector<int> cnt(n + 1, 0);
                    for (int x : cand) {
                        ++cnt[x];
                    }

                    // Accumulate subtree counts. parent[i] < i is guaranteed.
                    for (int v = n; v >= 2; --v) {
                        if (cnt[v]) {
                            cnt[parent[v]] += cnt[v];
                        }
                    }

                    int bestV = -1;
                    int bestMax = m2 + 1;

                    for (int v = 1; v <= n; ++v) {
                        int c = cnt[v];
                        if (c > 0 && c < m2) {
                            int mx = max(c, m2 - c);
                            if (mx < bestMax) {
                                bestMax = mx;
                                bestV = v;
                            }
                        }
                    }

                    // Since m2 > 1, there is always a proper subtree
                    // containing at least one but not all candidates.
                    int k = d - depth[bestV];
                    int res = ask(bestV, k);

                    vector<int> nxt;
                    nxt.reserve(cand.size());

                    for (int x : cand) {
                        bool inside = (tin[bestV] <= tin[x] && tin[x] < tout[bestV]);
                        if (inside == (res == 1)) {
                            nxt.push_back(x);
                        }
                    }

                    cand.swap(nxt);
                }

                break;
            }

            // Split by depth as evenly as possible in terms of the number
            // of remaining candidate vertices.
            vector<int> freq(maxDepth + 1, 0);
            for (int x : cand) {
                ++freq[depth[x]];
            }

            int bestD = -1;
            int bestMax = m + 1;
            int pref = 0;

            for (int d = 0; d <= maxDepth; ++d) {
                if (freq[d] == 0) continue;

                pref += freq[d];

                if (pref < m) {
                    int mx = max(pref, m - pref);
                    if (mx < bestMax) {
                        bestMax = mx;
                        bestD = d;
                    }
                }
            }

            int res = ask(1, bestD);

            vector<int> nxt;
            nxt.reserve(cand.size());

            for (int x : cand) {
                bool left = (depth[x] <= bestD);
                if (left == (res == 1)) {
                    nxt.push_back(x);
                }
            }

            cand.swap(nxt);
        }

        cout << "! " << cand[0] << endl;
    }

    return 0;
}
