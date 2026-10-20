#include <bits/stdc++.h>
using namespace std;

struct Event {
    double d;
    double qb, qa;
    int tree;
};

vector<Event> solve_tree(int n, int s, const vector<vector<int>>& adj, int treeId) {
    vector<int> parent(n + 1, 0);
    vector<vector<int>> children(n + 1);
    vector<int> deg(n + 1);
    for (int i = 1; i <= n; ++i) deg[i] = (int)adj[i].size();

    vector<int> preorder;
    preorder.reserve(n);
    vector<int> st;
    st.push_back(1);
    parent[1] = -1;

    while (!st.empty()) {
        int v = st.back();
        st.pop_back();
        preorder.push_back(v);
        for (int to : adj[v]) {
            if (to == parent[v]) continue;
            parent[to] = v;
            children[v].push_back(to);
            st.push_back(to);
        }
    }

    vector<int> postorder = preorder;
    reverse(postorder.begin(), postorder.end());

    vector<char> active(n + 1, 0);
    active[1] = 1;

    vector<double> u(n + 1), l(n + 1);
    vector<double> beta(n + 1), A(n + 1), B(n + 1);

    auto compute_potentials = [&]() {
        for (int x : postorder) {
            if (!active[x]) continue;

            double sumBeta = 0.0;
            double sumA = 0.0;
            double sumB = 0.0;

            for (int c : children[x]) {
                if (active[c]) {
                    sumBeta += beta[c];
                    sumA += A[c];
                    sumB += B[c];
                }
            }

            if (x != 1) {
                double denB = (double)deg[x] - sumBeta;
                if (denB < 1e-18) denB = 1e-18;
                beta[x] = 1.0 / denB;

                double denA = (double)deg[x] - sumA;
                if (denA < 1e-18) denA = 1e-18;
                A[x] = 1.0 / denA;
                B[x] = A[x] * ((double)deg[x] + sumB);
            }
        }

        u[1] = 1.0;
        l[1] = 0.0;

        for (int x : preorder) {
            if (x == 1 || !active[x]) continue;
            int p = parent[x];
            u[x] = beta[x] * u[p];
            l[x] = A[x] * l[p] + B[x];
        }
    };

    compute_potentials();

    const double TOL = 1e-12;
    int remaining = n - 1;

    vector<Event> events;
    events.reserve(n);

    while (remaining > 0) {
        double best = -1.0;

        for (int v = 2; v <= n; ++v) {
            if (active[v]) continue;
            int p = parent[v];
            if (p > 0 && active[p]) {
                double cand = u[p] / ((double)deg[v] + l[p]);
                if (cand > best) best = cand;
            }
        }

        if (best < 0.0) break;

        double level = best;
        double q_before = active[s] ? max(0.0, min(1.0, 1.0 - u[s])) : 1.0;

        while (true) {
            int cnt = 0;

            for (int v = 2; v <= n; ++v) {
                if (active[v]) continue;
                int p = parent[v];
                if (p > 0 && active[p]) {
                    double cand = u[p] / ((double)deg[v] + l[p]);
                    if (cand >= level - TOL) {
                        active[v] = 1;
                        --remaining;
                        ++cnt;
                    }
                }
            }

            if (cnt == 0) break;

            compute_potentials();

            double maxCandidate = -1.0;
            for (int v = 2; v <= n; ++v) {
                if (active[v]) continue;
                int p = parent[v];
                if (p > 0 && active[p]) {
                    double cand = u[p] / ((double)deg[v] + l[p]);
                    if (cand > maxCandidate) maxCandidate = cand;
                }
            }

            if (maxCandidate < level - TOL) break;
        }

        double q_after = active[s] ? max(0.0, min(1.0, 1.0 - u[s])) : 1.0;
        if (q_after > q_before) q_after = q_before;

        if (q_before - q_after > 1e-15) {
            events.push_back({level, q_before, q_after, treeId});
        }
    }

    return events;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int k;
    cin >> k;

    vector<Event> allEvents;
    allEvents.reserve(1000000);

    for (int tc = 0; tc < k; ++tc) {
        int n, s;
        cin >> n >> s;

        vector<vector<int>> adj(n + 1);
        for (int i = 1; i < n; ++i) {
            int u, v;
            cin >> u >> v;
            adj[u].push_back(v);
            adj[v].push_back(u);
        }

        vector<Event> ev = solve_tree(n, s, adj, tc);
        allEvents.insert(allEvents.end(), ev.begin(), ev.end());
    }

    sort(allEvents.begin(), allEvents.end(), [](const Event& a, const Event& b) {
        return a.d > b.d;
    });

    vector<double> q(k, 1.0);

    double prod = 1.0;
    int zeros = 0;
    long double answer = 0.0L;

    for (const auto& ev : allEvents) {
        if (zeros > 0) break;

        int id = ev.tree;
        double cur = q[id];

        if (cur <= 0.0) continue;

        double nxt = ev.qa;
        if (nxt >= cur - 1e-18) continue;
        if (nxt < 0.0) nxt = 0.0;

        double S_before = prod;

        q[id] = nxt;

        if (nxt <= 0.0) {
            ++zeros;
            prod = 0.0;
        } else {
            prod *= nxt / cur;
        }

        double S_after = (zeros > 0 ? 0.0 : prod);

        answer += (S_before - S_after) / ev.d;

        if (zeros > 0) break;
    }

    cout << fixed << setprecision(15) << (double)answer << "\n";
    return 0;
}
