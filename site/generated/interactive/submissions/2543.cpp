#include <bits/stdc++.h>
using namespace std;

int n, refPos;
int val[805];

int query(const vector<int>& q) {
    cout << "? " << q.size();
    for (int x : q) cout << " " << x;
    cout << endl;
    int r;
    cin >> r;
    if (r == -1) exit(0);
    return r;
}

void brute_force(const vector<int>& pos, long long L, long long d) {
    int m = pos.size();
    if (m == 1) {
        val[pos[0]] = L + d;
        return;
    }
    long long Total = m * L + d * m * (m + 1) / 2;
    vector<int> resp(m);
    for (int i = 0; i < m; i++) {
        vector<int> q;
        q.reserve(m - 1);
        for (int j = 0; j < m; j++) if (j != i) q.push_back(pos[j]);
        resp[i] = query(q);
    }
    vector<vector<int>> allowed(m);
    int refIdx = -1;
    for (int i = 0; i < m; i++) if (pos[i] == refPos) refIdx = i;
    for (int i = 0; i < m; i++) {
        for (int r = 1; r <= m; r++) {
            long long v = L + d * r;
            bool cond = ((Total - v) % (m - 1) == 0);
            if (cond == (resp[i] == 1)) {
                allowed[i].push_back(r);
            }
        }
        if (i == refIdx) {
            allowed[i].clear();
            allowed[i].push_back(1);
        }
    }
    vector<int> order(m);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return allowed[a].size() < allowed[b].size();
    });
    vector<int> r(m, 0);
    vector<bool> used(m + 1, false);
    vector<vector<int>> solutions;
    function<void(int)> dfs = [&](int idx) {
        if (idx == m) {
            solutions.push_back(r);
            return;
        }
        int i = order[idx];
        for (int v : allowed[i]) {
            if (!used[v]) {
                used[v] = true;
                r[i] = v;
                bool ok = true;
                for (int j = idx + 1; j < m && ok; j++) {
                    int p = order[j];
                    bool has = false;
                    for (int v2 : allowed[p]) if (!used[v2]) { has = true; break; }
                    if (!has) ok = false;
                }
                if (ok) dfs(idx + 1);
                used[v] = false;
            }
        }
    };
    dfs(0);
    int chosen_sol = 0;
    if (solutions.size() > 1) {
        int chosen_i = -1;
        for (int i = 0; i < m; i++) {
            long long v0 = L + d * solutions[0][i];
            long long v1 = L + d * solutions[1][i];
            bool r0 = (v0 % 2 == 1);
            bool r1 = (v1 % 2 == 1);
            if (r0 != r1) {
                chosen_i = i;
                vector<int> q = {pos[i], refPos};
                int resp2 = query(q);
                if (resp2 == (r0 ? 1 : 0)) chosen_sol = 0;
                else chosen_sol = 1;
                break;
            }
        }
        if (chosen_i == -1) chosen_sol = 0;
    }
    for (int i = 0; i < m; i++) {
        val[pos[i]] = L + d * solutions[chosen_sol][i];
    }
}

void solve(vector<int> pos, long long L, long long d) {
    int m = pos.size();
    if (m == 1) {
        val[pos[0]] = L + d;
        return;
    }
    if (m <= 32) {
        brute_force(pos, L, d);
        return;
    }
    bool hasRef = false;
    for (int p : pos) if (p == refPos) { hasRef = true; break; }
    int K = 2 * d;
    vector<int> base(pos.begin(), pos.begin() + K);
    int b = base[0];
    int resp = query(base);
    int C = 1 - resp;
    vector<int> known(m, 0);
    for (int i = K; i < m; i++) {
        int p = pos[i];
        vector<int> q = base;
        q[0] = p;
        int r = query(q);
        int a = 1 - r;
        known[i] = C ^ a;
    }
    int j = pos[K];
    for (int t = 1; t < K; t++) {
        int p = base[t];
        vector<int> q = base;
        for (int u = 0; u < K; u++) if (q[u] == p) { q[u] = j; break; }
        int r = query(q);
        int a = 1 - r;
        known[t] = C ^ known[K] ^ a;
    }
    int xb;
    if (m % 2 == 1) {
        int count1 = 0;
        for (int i = 0; i < m; i++) if (known[i] == 1) count1++;
        int target = (m + 1) / 2;
        if (count1 == target) xb = 0;
        else xb = 1;
    } else {
        if (hasRef) {
            int refIdx = -1;
            for (int i = 0; i < m; i++) if (pos[i] == refPos) { refIdx = i; break; }
            xb = 1 ^ known[refIdx];
        } else {
            vector<int> candidates;
            long long Total = m * L + d * m * (m + 1) / 2;
            for (int i = 0; i < m; i++) {
                vector<int> q;
                q.reserve(m - 1);
                for (int j2 = 0; j2 < m; j2++) if (j2 != i) q.push_back(pos[j2]);
                int r = query(q);
                if (r == 1) candidates.push_back(i);
            }
            int A = candidates[0];
            int B = candidates[1];
            vector<int> q;
            q.reserve(m);
            for (int j2 = 0; j2 < m; j2++) if (j2 != B) q.push_back(pos[j2]);
            q.push_back(refPos);
            int r = query(q);
            long long vB_min = L + d;
            long long vB_max = L + d * m;
            bool resp_B_min = (((1 - vB_min) % m + m) % m == 0);
            bool resp_B_max = (((1 - vB_max) % m + m) % m == 0);
            bool B_is_min;
            if (r == 1) {
                if (resp_B_min) B_is_min = true;
                else B_is_min = false;
            } else {
                if (resp_B_min) B_is_min = false;
                else B_is_min = true;
            }
            int minIdx = B_is_min ? B : A;
            xb = 1 ^ known[minIdx];
        }
    }
    vector<int> x(m);
    for (int i = 0; i < m; i++) x[i] = xb ^ known[i];
    vector<int> oddPos, evenPos;
    for (int i = 0; i < m; i++) {
        if (x[i] == 1) oddPos.push_back(pos[i]);
        else evenPos.push_back(pos[i]);
    }
    if (!oddPos.empty()) solve(oddPos, L - d, 2 * d);
    if (!evenPos.empty()) solve(evenPos, L, 2 * d);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cin >> n;
    refPos = -1;
    for (int i = 1; i <= n; i++) {
        vector<int> q;
        q.reserve(n - 1);
        for (int j = 1; j <= n; j++) if (j != i) q.push_back(j);
        int r = query(q);
        if (r == 1) { refPos = i; break; }
    }
    vector<int> allPos(n);
    iota(allPos.begin(), allPos.end(), 1);
    solve(allPos, 0, 1);
    if (val[1] > n / 2) {
        for (int i = 1; i <= n; i++) val[i] = n + 1 - val[i];
    }
    cout << "!";
    for (int i = 1; i <= n; i++) cout << " " << val[i];
    cout << endl;
    return 0;
}
