#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll NEG = -(1LL << 60);

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, s;
    cin >> n >> s;
    --s;

    vector<ll> w(n), p(n);
    for (int i = 0; i < n; ++i) cin >> w[i];
    for (int i = 0; i < n; ++i) cin >> p[i];

    vector<int> a(n);
    vector<vector<int>> rev(n);
    for (int i = 0; i < n; ++i) {
        cin >> a[i];
        --a[i];
        rev[a[i]].push_back(i);
    }

    // Find the cycle reached from s.
    vector<int> pos(n, -1), path;
    int cur = s;
    while (pos[cur] == -1) {
        pos[cur] = (int)path.size();
        path.push_back(cur);
        cur = a[cur];
    }

    int cycStart = pos[cur];
    vector<int> cycleNodes(path.begin() + cycStart, path.end());
    vector<char> isCycle(n, false);
    for (int v : cycleNodes) isCycle[v] = true;

    bool sOnCycle = isCycle[s];

    if (!sOnCycle) {
        // The reachable part is a rooted tree (copy direction is parent -> child
        // in the reversed graph).  We compute a DP for this pure tree.
        vector<char> vis(n, false);
        vector<int> order;
        order.reserve(n);

        vector<int> st;
        st.push_back(s);
        vis[s] = true;

        while (!st.empty()) {
            int v = st.back();
            st.pop_back();
            order.push_back(v);
            for (int u : rev[v]) {
                if (!vis[u] && !isCycle[u]) {
                    vis[u] = true;
                    st.push_back(u);
                }
            }
        }

        vector<ll> dp1(n, NEG), dp0(n, NEG);

        for (int it = (int)order.size() - 1; it >= 0; --it) {
            int v = order[it];
            if (v == s) continue;

            ll sum1 = 0;
            ll sumAny = 0;
            ll bestOne = NEG;
            bool hasChild = false;

            for (int u : rev[v]) {
                if (!vis[u] || isCycle[u]) continue;

                if (dp1[u] > 0) sum1 += dp1[u];

                ll x = max(dp1[u], dp0[u]);
                if (x != NEG) {
                    hasChild = true;
                    bestOne = max(bestOne, x);
                    if (x > 0) sumAny += x;
                }
            }

            dp1[v] = w[v] - p[v] + sum1;

            if (!hasChild) {
                dp0[v] = NEG;
            } else {
                ll add = sumAny;
                if (sumAny == 0) add = bestOne;
                dp0[v] = -2LL * p[v] + add;
            }
        }

        ll ans = NEG;

        // s remains 1 in the end.
        {
            ll val = w[s];
            for (int u : rev[s]) {
                if (vis[u] && !isCycle[u]) {
                    val += max(0LL, dp1[u]);
                }
            }
            ans = max(ans, val);
        }

        // s is erased in the end.
        {
            ll val = -p[s];
            for (int u : rev[s]) {
                if (vis[u] && !isCycle[u]) {
                    val += max(0LL, max(dp1[u], dp0[u]));
                }
            }
            ans = max(ans, val);
        }

        cout << ans << '\n';
        return 0;
    }

    // s is on a cycle.
    // Build cycle in copy-edge order, starting from s.
    // If a[x] = y, then the copy edge is y -> x.
    vector<int> pred(n, -1);
    for (int v : cycleNodes) pred[a[v]] = v;

    vector<int> cyc;
    cur = s;
    do {
        cyc.push_back(cur);
        cur = pred[cur];
    } while (cur != s);

    int m = (int)cyc.size();

    // Compute connected-1 DP for all non-cycle subtrees attached to the cycle.
    vector<char> inComp(n, false);
    vector<int> order;
    order.reserve(n);

    vector<int> st;
    for (int v : cyc) {
        for (int u : rev[v]) {
            if (!isCycle[u] && !inComp[u]) {
                inComp[u] = true;
                st.push_back(u);
            }
        }
    }

    while (!st.empty()) {
        int v = st.back();
        st.pop_back();
        order.push_back(v);

        for (int u : rev[v]) {
            if (!isCycle[u] && !inComp[u]) {
                inComp[u] = true;
                st.push_back(u);
            }
        }
    }

    vector<ll> dp1(n, NEG);
    for (int it = (int)order.size() - 1; it >= 0; --it) {
        int v = order[it];
        ll sum = 0;
        for (int u : rev[v]) {
            if (!isCycle[u] && inComp[u]) {
                if (dp1[u] > 0) sum += dp1[u];
            }
        }
        dp1[v] = w[v] - p[v] + sum;
    }

    vector<ll> bonus(m, 0);
    for (int i = 0; i < m; ++i) {
        int v = cyc[i];
        ll b = 0;
        for (int u : rev[v]) {
            if (!isCycle[u] && inComp[u]) {
                b += max(0LL, dp1[u]);
            }
        }
        bonus[i] = b;
    }

    vector<ll> wc(m), pc(m);
    ll totalBonus = 0;
    for (int i = 0; i < m; ++i) {
        wc[i] = w[cyc[i]];
        pc[i] = p[cyc[i]];
        totalBonus += bonus[i];
    }

    int len2 = 2 * m;

    vector<ll> prefW(len2 + 1, 0), prefP(len2 + 1, 0);
    for (int i = 0; i < len2; ++i) {
        prefW[i + 1] = prefW[i] + wc[i % m];
        prefP[i + 1] = prefP[i] + pc[i % m];
    }

    vector<ll> prefBonus(m + 1, 0);
    for (int i = 0; i < m; ++i) {
        prefBonus[i + 1] = prefBonus[i] + bonus[i];
    }

    auto getBonusVisited = [&](int R) -> ll {
        if (R < m) return prefBonus[R + 1];
        return totalBonus;
    };

    // A[R] = prefW[R+1] + bonusVisited(R) - prefP[R+1]
    vector<ll> A(len2);
    for (int R = 0; R < len2; ++R) {
        A[R] = prefW[R + 1] + getBonusVisited(R) - prefP[R + 1];
    }

    ll ans = -pc[0]; // erase s and finish with no 1 on the cycle

    deque<int> dq;
    int addR = 0;

    // L is the first position of the final 1-block after the cut at s.
    // We allow L = 0..m, so one full extra lap can be used to visit all vertices.
    for (int L = 0; L <= m; ++L) {
        int rightLimit = min(len2 - 1, L + m - 1);

        while (addR <= rightLimit) {
            while (!dq.empty() && A[dq.back()] <= A[addR]) dq.pop_back();
            dq.push_back(addR);
            ++addR;
        }

        while (!dq.empty() && dq.front() < L) dq.pop_front();

        if (!dq.empty()) {
            int R = dq.front();
            ll cand = A[R] - prefW[L] - prefP[L] + prefP[1];
            ans = max(ans, cand);
        }
    }

    cout << ans << '\n';
    return 0;
}
