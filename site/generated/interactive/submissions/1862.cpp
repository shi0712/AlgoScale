#include <bits/stdc++.h>
using namespace std;

constexpr int CAP = 15;
constexpr int BRIDGE = 16;
constexpr int STATES = 17;

int queryCount = 0;

bool ask(int parent, int edge) {
    ++queryCount;
    cout << "? " << parent << ' ' << edge << '\n' << flush;

    int reply;
    if (!(cin >> reply) || (reply != 0 && reply != 1))
        exit(0);
    return reply == 1;
}

// Minimum-weight ordered decision tree. A cut at k separates
// candidates [left, k] from candidates [k+1, right].
struct DecisionTree {
    int cut[STATES][STATES];

    void build(int n, const double* weight) {
        if (n == 1) return;
        if (n == 2) {
            cut[0][1] = 0;
            return;
        }

        double dp[STATES][STATES];
        double prefix[STATES + 1] = {};
        for (int i = 0; i < n; ++i) {
            prefix[i + 1] = prefix[i] + weight[i];
            dp[i][i] = 0;
        }

        for (int len = 2; len <= n; ++len) {
            for (int l = 0; l + len <= n; ++l) {
                int r = l + len - 1;
                int bestCut = l;
                double best = dp[l][l] + dp[l + 1][r];
                for (int k = l + 1; k < r; ++k) {
                    double cost = dp[l][k] + dp[k + 1][r];
                    if (cost < best) {
                        best = cost;
                        bestCut = k;
                    }
                }
                cut[l][r] = bestCut;
                dp[l][r] = best + prefix[r + 1] - prefix[l];
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int m;
    if (!(cin >> m) || m < 1) return 0;

    vector<int> order(m), arrival(m + 1), answer(m + 1);
    iota(order.begin(), order.end(), 1);
    mt19937 rng(static_cast<unsigned>(
        chrono::steady_clock::now().time_since_epoch().count()));
    shuffle(order.begin(), order.end(), rng);

    // Layer t contains one representative of each cycle seen at
    // least t times. Its head is the query that removes the layer.
    array<vector<int>, CAP + 1> layer;
    int head[CAP + 1] = {};
    int count[CAP + 2] = {};
    int bridges = 0, overflow = 0;

    double recent[STATES] = {};
    recent[0] = 2.0;
    recent[BRIDGE] = 0.5;

    DecisionTree tree;
    int candidate[STATES];
    double weight[STATES];

    for (int pos = 0; pos < m; ++pos) {
        int e = order[pos];
        arrival[e] = pos;

        int n = 0;
        candidate[n++] = 0;
        for (int c = 1; c <= CAP; ++c) {
            if (count[c] > count[c + 1])
                candidate[n++] = c;
        }
        candidate[n++] = BRIDGE;

        // Use the frequency-of-frequencies and recent outcomes only
        // to choose the order of tests, never to rule out an outcome.
        double estimate[STATES] = {};
        for (int c = 0; c < CAP - 1; ++c)
            estimate[c] = (c + 1) * (count[c + 1] - count[c + 2]);
        double capMass = CAP * count[CAP] + overflow;
        double fraction = 0.0;
        if (count[CAP - 1] > count[CAP]) {
            fraction = (recent[CAP - 1] + 1.0) /
                       (recent[CAP - 1] + recent[CAP] + 2.0);
        }
        estimate[CAP - 1] = capMass * fraction;
        estimate[CAP] = capMass * (1.0 - fraction);
        estimate[BRIDGE] = bridges;

        double estimateSum = 0.0, recentSum = 0.0;
        for (int i = 0; i < n; ++i) {
            estimateSum += estimate[candidate[i]];
            recentSum += recent[candidate[i]];
        }
        for (int i = 0; i < n; ++i) {
            int c = candidate[i];
            weight[i] = 0.002;
            if (estimateSum > 0)
                weight[i] += 0.72 * estimate[c] / estimateSum;
            if (recentSum > 0)
                weight[i] += 0.28 * recent[c] / recentSum;
        }

        tree.build(n, weight);
        int l = 0, r = n - 1, lastSuccess = 0;
        while (l < r) {
            int k = tree.cut[l][r];
            int c = candidate[k];
            int parent = (c == CAP ? 0 : head[c + 1]);
            if (ask(parent, e)) {
                lastSuccess = queryCount;
                r = k;
            } else {
                l = k + 1;
            }
        }

        int c = candidate[l];
        if (c == BRIDGE) {
            answer[e] = -1;
            ++bridges;
        } else if (c == CAP) {
            answer[e] = CAP;
            ++overflow;
        } else {
            // The cut immediately after c was successful and used
            // layer c+1. Reuse it as the new head, without a query.
            int t = c + 1;
            head[t] = lastSuccess;
            layer[t].push_back(e);
            ++count[t];
            if (t == CAP) answer[e] = CAP;
        }

        for (double& x : recent) x *= 0.976;
        recent[c] += 1.0;
    }

    // Each edge of layer t+1 has a unique, earlier mate in layer t.
    // In reverse order, only later mates not already accounted for
    // can provide a length greater than t.
    for (int t = CAP - 1; t >= 1; --t) {
        int available[CAP + 1] = {};
        available[t] = count[t] - count[t + 1];
        int j = count[t + 1] - 1;

        for (int i = count[t] - 1; i >= 0; --i) {
            int e = layer[t][i];
            while (j >= 0 && arrival[layer[t + 1][j]] > arrival[e]) {
                ++available[answer[layer[t + 1][j]]];
                --j;
            }

            int n = 0;
            for (int len = t; len <= CAP; ++len) {
                if (available[len] > 0) {
                    candidate[n] = len;
                    weight[n++] = available[len];
                }
            }

            tree.build(n, weight);
            int l = 0, r = n - 1;
            while (l < r) {
                int k = tree.cut[l][r];
                int threshold = candidate[k] + 1;
                if (ask(head[threshold], e))
                    r = k;
                else
                    l = k + 1;
            }
            answer[e] = candidate[l];
            --available[answer[e]];
        }
    }

    cout << '!';
    for (int e = 1; e <= m; ++e)
        cout << ' ' << (answer[e] == CAP ? 0 : answer[e]);
    cout << '\n' << flush;

    int verdict;
    if (!(cin >> verdict) || verdict != 1) return 0;
    return 0;
}
