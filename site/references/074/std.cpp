#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int u, v, w;
};

struct Fraction {
    long long num = 0, den = 0;  // den == 0: no candidate

    void relax(long long length, long long sum, long long square_sum) {
        long long a = length * square_sum - sum * sum;
        long long b = length * length;
        if (den == 0 || a * den < num * b) {
            num = a;
            den = b;
        }
    }
};

Fraction solve(int n, const vector<Edge>& input) {
    vector<vector<int>> g(n), rg(n);
    for (const auto& e : input) {
        g[e.u].push_back(e.v);
        rg[e.v].push_back(e.u);
    }

    auto reach = [&](int start, const vector<vector<int>>& graph) {
        vector<char> vis(n, false);
        vector<int> st{start};
        vis[start] = true;
        while (!st.empty()) {
            int u = st.back();
            st.pop_back();
            for (int v : graph[u]) {
                if (!vis[v]) {
                    vis[v] = true;
                    st.push_back(v);
                }
            }
        }
        return vis;
    };

    auto from_s = reach(0, g);
    if (!from_s[n - 1]) return {};
    auto to_t = reach(n - 1, rg);

    vector<int> id(n, -1);
    int N = 0;
    for (int v = 0; v < n; ++v) {
        if (from_s[v] && to_t[v]) id[v] = N++;
    }
    const int source = id[0], target = id[n - 1];

    vector<Edge> edges;
    int W = 0;
    for (const auto& e : input) {
        if (id[e.u] != -1 && id[e.v] != -1) {
            edges.push_back({id[e.u], id[e.v], e.w});
            W = max(W, e.w);
        }
    }

    const int S = N * W + 1;
    const int layer_size = N * S;
    const int INF = 1000000000;

    // Minimum sum of squares for the current / next length.
    vector<int> dp(layer_size, INF), ndp(layer_size, INF);
    // Parent edge for every length, endpoint, and weight sum.
    vector<int> parent((N + 1) * layer_size, -1);
    dp[source * S] = 0;

    Fraction answer;
    for (int len = 1; len <= N; ++len) {
        fill(ndp.begin(), ndp.end(), INF);
        const int prev_max_sum = (len - 1) * W;
        for (int i = 0; i < (int)edges.size(); ++i) {
            const auto& e = edges[i];
            int from = e.u * S;
            int to = e.v * S + e.w;
            for (int sum = 0; sum <= prev_max_sum; ++sum) {
                if (dp[from + sum] == INF) continue;
                int candidate = dp[from + sum] + e.w * e.w;
                if (candidate < ndp[to + sum]) {
                    ndp[to + sum] = candidate;
                    parent[len * layer_size + to + sum] = i;
                }
            }
        }
        dp.swap(ndp);

        if (len < N) {
            for (int sum = 0; sum <= len * W; ++sum) {
                int q = dp[target * S + sum];
                if (q != INF) answer.relax(len, sum, q);
            }
            if (answer.den != 0 && answer.num == 0) return answer;
        }
    }

    // Every N-edge walk on N vertices contains a cycle.
    // Only the first cycle found in each representative walk is needed.
    vector<int> seen(N), saved_sum(N), saved_square_sum(N);
    for (int end = 0; end < N; ++end) {
        for (int total = 0; total < S; ++total) {
            if (dp[end * S + total] == INF) continue;
            fill(seen.begin(), seen.end(), -1);

            int v = end, sum = total;
            int steps = 0, accumulated_sum = 0, accumulated_square_sum = 0;
            seen[v] = 0;
            saved_sum[v] = 0;
            saved_square_sum[v] = 0;

            for (int len = N; len >= 1; --len) {
                int edge_id = parent[len * layer_size + v * S + sum];
                const auto& e = edges[edge_id];
                v = e.u;
                sum -= e.w;
                ++steps;
                accumulated_sum += e.w;
                accumulated_square_sum += e.w * e.w;

                if (seen[v] != -1) {
                    answer.relax(steps - seen[v],
                                 accumulated_sum - saved_sum[v],
                                 accumulated_square_sum - saved_square_sum[v]);
                    break;
                }
                seen[v] = steps;
                saved_sum[v] = accumulated_sum;
                saved_square_sum[v] = accumulated_square_sum;
            }
            if (answer.den != 0 && answer.num == 0) return answer;
        }
    }
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<Edge> edges(m);
    for (auto& e : edges) {
        cin >> e.u >> e.v >> e.w;
        --e.u;
        --e.v;
    }

    Fraction result = solve(n, edges);
    if (result.den == 0) {
        cout << "-1\n";
    } else {
        cout << fixed << setprecision(15)
             << (long double)result.num / result.den << '\n';
    }
    return 0;
}
