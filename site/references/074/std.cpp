#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1'000'000'007;

class Solver {
private:
    int n = 0;
    vector<vector<unsigned char>> ok;
    vector<int> R;
    vector<vector<int>> children;

    int countForests(const vector<unsigned char>& token, int begin) const {
        // dp[h]: after the processed prefix, there are h open ordinary vertices.
        vector<int> dp(1, 1);
        for (int i = begin; i < (int)token.size(); ++i) {
            const int ordinary = token[i];
            vector<int> ndp(dp.size() + ordinary, 0);
            int suffix = 0;
            for (int h = (int)dp.size() - 1; h >= 0; --h) {
                suffix += dp[h];
                if (suffix >= MOD) suffix -= MOD;
                ndp[h + ordinary] = suffix;
            }
            dp.swap(ndp);
        }

        int ans = 0;
        for (int x : dp) {
            ans += x;
            if (ans >= MOD) ans -= MOD;
        }
        return ans;
    }

    int evaluate(int v, bool fixedRoot) {
        // token 1 = ordinary vertex; token 0 = compressed block forced to be a leaf.
        vector<unsigned char> token;
        long long factor = 1;

        // Follow the unique possible final singleton block iteratively.
        while (v != 0) {
            token.push_back(1);

            int nextVertex = 0;
            int groupBegin = 0;
            const auto& ch = children[v];

            for (int j = 0; j < (int)ch.size(); ++j) {
                if (!ok[v][R[ch[j]]]) continue;

                if (groupBegin == j) {
                    // A singleton minimal block can only be the final block.
                    nextVertex = ch[j];
                } else {
                    // A minimal block of at least two maximal child subtrees is
                    // compressed to one forced-leaf token. The inside choices
                    // are independent forests attached to one common parent.
                    token.push_back(0);
                    for (int k = groupBegin; k <= j; ++k) {
                        factor = factor * evaluate(ch[k], false) % MOD;
                    }
                }
                groupBegin = j + 1;
            }

            v = nextVertex;
        }

        // The real root 1 is always open, so its token is omitted from the stack DP.
        const int ways = countForests(token, fixedRoot ? 1 : 0);
        return (int)(factor * ways % MOD);
    }

public:
    int run() {
        int m;
        cin >> n >> m;

        // Relabel by the first DFS order, which becomes 1,2,...,n internally.
        vector<int> position(n + 1);
        for (int i = 1; i <= n; ++i) {
            int x;
            cin >> x;
            position[x] = i;
        }

        // low[x] / high[x] = min/max predecessor position of x over all orders.
        vector<int> low(n + 1), high(n + 1);
        for (int i = 2; i <= n; ++i) {
            low[i] = high[i] = i - 1; // predecessor in the first order
        }

        for (int k = 1; k < m; ++k) {
            int previous = 0;
            for (int i = 1; i <= n; ++i) {
                int x;
                cin >> x;
                const int v = position[x];
                if (i > 1) {
                    low[v] = min(low[v], previous);
                    high[v] = max(high[v], previous);
                }
                previous = v;
            }
        }

        // ok[l][r] means [l,r] is a subtree interval in every given order.
        ok.assign(n + 1, vector<unsigned char>(n + 1, 0));
        R.assign(n + 1, 0);

        for (int l = 1; l <= n; ++l) {
            ok[l][l] = 1;
            R[l] = l;

            int mn = n + 1;
            int mx = 0;
            for (int r = l + 1; r <= n; ++r) {
                mn = min(mn, low[r]);
                mx = max(mx, high[r]);
                if (mn >= l && mx <= r) {
                    ok[l][r] = 1;
                    R[l] = r;
                }
            }
        }

        // Maximal common intervals are laminar; build their containment tree.
        children.assign(n + 1, {});
        vector<int> st;
        st.reserve(n);
        for (int v = 1; v <= n; ++v) {
            while (!st.empty() && R[st.back()] < v) st.pop_back();
            if (!st.empty()) children[st.back()].push_back(v);
            st.push_back(v);
        }

        return evaluate(1, true);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Solver solver;
    cout << solver.run() << '\n';
    return 0;
}
