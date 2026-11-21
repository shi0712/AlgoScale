#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int to;
    long long w;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<long long> val(n + 1, 0);
    vector<int> pos;
    pos.reserve(n);
    vector<long long> prefPos(n + 1, 0);

    // Query all singleton intervals: gives max(0, a_i).
    {
        string out;
        out.reserve(n * 16);
        for (int i = 1; i <= n; ++i) {
            out += "? ";
            out += to_string(i);
            out += " ";
            out += to_string(i);
            out += "\n";
        }
        cout << out;
        cout.flush();

        for (int i = 1; i <= n; ++i) {
            long long ans;
            cin >> ans;
            if (ans < 0) return 0;
            val[i] = ans;
            if (ans > 0) pos.push_back(i);
            prefPos[i] = prefPos[i - 1] + ans;
        }
    }

    int k = (int)pos.size();

    if (k == 0) {
        cout << "!";
        for (int i = 1; i <= n; ++i) cout << " " << 0;
        cout << "\n";
        cout.flush();
        return 0;
    }

    vector<int> idxPref(n + 1, 0);
    for (int i = 1; i <= n; ++i) {
        idxPref[i] = idxPref[i - 1] + (val[i] == 0 ? 1 : 0);
    }
    int m = idxPref[n]; // number of non-positive positions

    vector<long long> prefX(k + 1, 0);
    for (int i = 0; i < k; ++i) {
        prefX[i + 1] = prefX[i] + val[pos[i]];
    }

    vector<long long> Q((size_t)k * k, -1);

    // Query all intervals between positive positions that contain
    // at least one non-positive position.
    {
        const int BATCH = 2000;
        vector<pair<int, int>> batch;
        batch.reserve(BATCH);
        string out;
        out.reserve(BATCH * 24);

        auto flushBatch = [&]() -> bool {
            if (batch.empty()) return true;
            cout << out;
            cout.flush();
            out.clear();

            for (int t = 0; t < (int)batch.size(); ++t) {
                long long ans;
                cin >> ans;
                if (ans < 0) return false;
                int i = batch[t].first;
                int j = batch[t].second;
                Q[(size_t)i * k + j] = ans;
            }
            batch.clear();
            return true;
        };

        for (int i = 0; i < k; ++i) {
            for (int j = i + 1; j < k; ++j) {
                int l = pos[i], r = pos[j];
                if (idxPref[r] > idxPref[l - 1]) {
                    out += "? ";
                    out += to_string(l);
                    out += " ";
                    out += to_string(r);
                    out += "\n";
                    batch.emplace_back(i, j);

                    if ((int)batch.size() == BATCH) {
                        if (!flushBatch()) return 0;
                    }
                }
            }
        }

        if (!flushBatch()) return 0;
    }

    // Fill Q for intervals containing no non-positive positions.
    for (int i = 0; i < k; ++i) {
        Q[(size_t)i * k + i] = val[pos[i]];
        for (int j = i + 1; j < k; ++j) {
            long long &q = Q[(size_t)i * k + j];
            if (q == -1) {
                q = prefX[j + 1] - prefX[i];
            }
        }
    }

    int SZ = m + 1;
    vector<long long> maxW((size_t)SZ * SZ, -1);
    vector<long long> eqW((size_t)SZ * SZ, -1);

    for (int i = 0; i < k; ++i) {
        int l = pos[i];
        for (int j = i; j < k; ++j) {
            int r = pos[j];
            int a = idxPref[l - 1];
            int b = idxPref[r];

            if (a < b) {
                long long qij = Q[(size_t)i * k + j];
                long long totalPos = prefX[j + 1] - prefX[i];
                long long W = totalPos - qij;

                size_t id = (size_t)a * SZ + b;
                if (W > maxW[id]) maxW[id] = W;

                if (i < j) {
                    long long q1 = Q[(size_t)i * k + (j - 1)];
                    long long q2 = Q[(size_t)(i + 1) * k + j];
                    long long maxProper = max(q1, q2);

                    if (qij > maxProper) {
                        eqW[id] = W;
                    }
                }
            }
        }
    }

    vector<vector<Edge>> adj(m + 1);

    // Cumulative drops are non-decreasing.
    for (int i = 1; i <= m; ++i) {
        adj[i - 1].push_back({i, 0});
    }

    for (int a = 0; a <= m; ++a) {
        for (int b = a + 1; b <= m; ++b) {
            size_t id = (size_t)a * SZ + b;
            if (maxW[id] >= 0) {
                adj[a].push_back({b, maxW[id]});
            }
            if (eqW[id] >= 0) {
                adj[b].push_back({a, -eqW[id]});
            }
        }
    }

    // Longest paths / difference constraints.
    vector<long long> dist(m + 1, 0);
    vector<char> inq(m + 1, 1);
    deque<int> dq;
    for (int i = 0; i <= m; ++i) dq.push_back(i);

    while (!dq.empty()) {
        int u = dq.front();
        dq.pop_front();
        inq[u] = 0;

        for (const Edge &e : adj[u]) {
            if (dist[e.to] < dist[u] + e.w) {
                dist[e.to] = dist[u] + e.w;
                if (!inq[e.to]) {
                    inq[e.to] = 1;
                    if (!dq.empty() && dist[e.to] > dist[dq.front()]) {
                        dq.push_front(e.to);
                    } else {
                        dq.push_back(e.to);
                    }
                }
            }
        }
    }

    long long offset = dist[0];
    for (int i = 0; i <= m; ++i) dist[i] -= offset;

    vector<long long> pref(n + 1, 0);
    for (int i = 0; i <= n; ++i) {
        pref[i] = prefPos[i] - dist[idxPref[i]];
    }

    cout << "!";
    for (int i = 1; i <= n; ++i) {
        long long bi = pref[i] - pref[i - 1];
        cout << " " << bi;
    }
    cout << "\n";
    cout.flush();

    return 0;
}
