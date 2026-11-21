#include <bits/stdc++.h>
using namespace std;

int N, M;

int ask(int v, const string &b) {
    cout << "? " << v << ' ' << b << '\n';
    cout.flush();

    int ans;
    cin >> ans;
    if (ans == -1) exit(0);
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N;
    M = N - 1;

    string zeros(M, '0');

    int A0 = ask(1, zeros);

    vector<int> hbit(M);
    for (int i = 0; i < M; ++i) {
        string b = zeros;
        b[i] = '1';
        int A = ask(1, b);
        hbit[i] = (A - A0 + 1) / 2;
    }

    int Z = A0;

    int baseBits = 0;
    while ((1 << baseBits) < M) ++baseBits;

    string depthQuery(M, '0');
    for (int i = 0; i < M; ++i) {
        int bit = hbit[i] ^ 1;
        depthQuery[i] = char('0' + bit);
    }

    vector<string> bitQuery(baseBits);
    vector<int> bitCount(baseBits, 0);

    for (int j = 0; j < baseBits; ++j) {
        bitQuery[j].assign(M, '0');
        for (int i = 0; i < M; ++i) {
            int c = (i >> j) & 1;
            bitCount[j] += c;
            int bit = hbit[i] ^ c;
            bitQuery[j][i] = char('0' + bit);
        }
    }

    vector<vector<int>> K(N + 1, vector<int>(baseBits, 0));
    vector<int> depth(N + 1, 0), W(N + 1, 0);

    for (int v = 2; v <= N; ++v) {
        int A = ask(v, depthQuery);
        depth[v] = A;

        for (int j = 0; j < baseBits; ++j) {
            int Aj = ask(v, bitQuery[j]);
            int k = (Aj - M + depth[v] + bitCount[j]) / 2;
            K[v][j] = k;
        }

        int A0v = ask(v, zeros);
        W[v] = A0v - Z;
    }

    vector<int> x(M);
    for (int i = 0; i < M; ++i) {
        x[i] = hbit[i] ? 1 : -1;
    }

    vector<vector<int>> extraC;
    vector<int> parent(N + 1, -1), label(N + 1, -1);

    mt19937 rng((uint64_t)chrono::steady_clock::now().time_since_epoch().count());

    bool done = false;

    while (!done) {
        bool ambiguous = false;

        fill(parent.begin(), parent.end(), -1);
        fill(label.begin(), label.end(), -1);

        for (int v = 2; v <= N; ++v) {
            int cnt = 0;
            int bestP = -1, bestI = -1;

            for (int p = 1; p <= N; ++p) {
                if (depth[p] != depth[v] - 1) continue;

                bool ok = true;
                int id = 0;

                for (int j = 0; j < baseBits; ++j) {
                    int diff = K[v][j] - K[p][j];
                    if (diff < 0 || diff > 1) {
                        ok = false;
                        break;
                    }
                    if (diff == 1) id |= (1 << j);
                }

                if (!ok) continue;
                if (id >= M) continue;

                for (int e = 0; e < (int)extraC.size(); ++e) {
                    int diff = K[v][baseBits + e] - K[p][baseBits + e];
                    if (diff != extraC[e][id]) {
                        ok = false;
                        break;
                    }
                }

                if (!ok) continue;
                if (W[v] - W[p] != x[id]) continue;

                ++cnt;
                bestP = p;
                bestI = id;

                if (cnt > 1) break;
            }

            if (cnt != 1) {
                ambiguous = true;
                break;
            }

            parent[v] = bestP;
            label[v] = bestI;
        }

        if (!ambiguous) {
            vector<int> used(M, 0);
            bool bad = false;

            for (int v = 2; v <= N; ++v) {
                int id = label[v];
                if (id < 0 || id >= M) {
                    bad = true;
                    break;
                }
                if (used[id]++) {
                    bad = true;
                    break;
                }
            }

            if (!bad) done = true;
            else ambiguous = true;
        }

        if (done) break;

        vector<int> C(M);
        int s = 0;

        for (int i = 0; i < M; ++i) {
            C[i] = rng() & 1;
            s += C[i];
        }

        string extraQuery(M, '0');
        for (int i = 0; i < M; ++i) {
            int bit = hbit[i] ^ C[i];
            extraQuery[i] = char('0' + bit);
        }

        for (int v = 2; v <= N; ++v) {
            int A = ask(v, extraQuery);
            int k = (A - M + depth[v] + s) / 2;
            K[v].push_back(k);
        }

        K[1].push_back(0);
        extraC.push_back(C);
    }

    vector<int> ansS(M), ansF(M);

    for (int v = 2; v <= N; ++v) {
        int id = label[v];
        int p = parent[v];

        if (hbit[id] == 1) {
            ansS[id] = v;
            ansF[id] = p;
        } else {
            ansS[id] = p;
            ansF[id] = v;
        }
    }

    cout << "!";
    for (int i = 0; i < M; ++i) {
        cout << ' ' << ansS[i] << ' ' << ansF[i];
    }
    cout << '\n';
    cout.flush();

    return 0;
}
