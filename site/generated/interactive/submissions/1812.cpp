#include <bits/stdc++.h>
using namespace std;

struct Group {
    vector<int> labels;
    vector<int> coeff;
    int ksum;
};

struct Cand {
    int root;
    vector<int> v;
};

int n;
vector<Group> groups;
vector<int> groupIdx;
vector<int> coeffInGroup;

long long query1(const vector<int>& nodes) {
    cout << "? 1 " << nodes.size();
    for (int u : nodes) cout << ' ' << u;
    cout << '\n' << flush;

    long long ans;
    cin >> ans;
    if (!cin) exit(0);
    return ans;
}

void toggleNode(int u) {
    cout << "? 2 " << u << '\n' << flush;
}

bool decodeGroup(const Group& g, long long S, vector<int>& v) {
    int m = (int)g.labels.size();
    long long M = (1LL << m) - 1;
    long long num = S + M;

    if (num % 2 != 0) return false;
    num /= 2;

    if (num < 0 || num >= (1LL << m)) return false;

    for (int i = 0; i < m; ++i) {
        int bit = (num >> i) & 1LL;
        v[g.labels[i]] = bit ? 1 : -1;
    }
    return true;
}

void computeF(const Cand& c, vector<int>& F) {
    int r = c.root;

    if (r == 1) {
        F[1] = c.v[1];
        for (int u = 2; u <= n; ++u) {
            F[u] = c.v[1] + c.v[u];
        }
    } else {
        F[r] = c.v[r];
        F[1] = c.v[r] + c.v[1];

        for (int u = 2; u <= n; ++u) {
            if (u == r) continue;
            F[u] = c.v[r] + c.v[1] + c.v[u];
        }
    }
}

void outputAnswer(const vector<int>& v) {
    cout << "!";
    for (int i = 1; i <= n; ++i) {
        cout << ' ' << v[i];
    }
    cout << '\n' << flush;
}

void resolveCandidates(vector<Cand>& C) {
    mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count());

    while (true) {
        if (C.size() == 1) {
            outputAnswer(C[0].v);
            return;
        }

        bool sameValues = true;
        for (int i = 1; i < (int)C.size(); ++i) {
            if (C[i].v != C[0].v) {
                sameValues = false;
                break;
            }
        }

        if (sameValues) {
            outputAnswer(C[0].v);
            return;
        }

        int m = (int)C.size();
        vector<vector<int>> F(m, vector<int>(n + 1));

        for (int i = 0; i < m; ++i) {
            computeF(C[i], F[i]);
        }

        bool sameF = true;
        for (int i = 1; i < m && sameF; ++i) {
            for (int u = 1; u <= n; ++u) {
                if (F[i][u] != F[0][u]) {
                    sameF = false;
                    break;
                }
            }
        }

        if (sameF) {
            toggleNode(1);
            for (int i = 0; i < m; ++i) {
                C[i].v[1] = -C[i].v[1];
            }
            continue;
        }

        vector<int> bestX(n + 1, 0);
        long long bestMax = m + 1;
        int bestDistinct = -1;
        vector<long long> bestPred(m, 0);

        auto evaluate = [&](const vector<long long>& pred, const vector<int>& x) {
            unordered_map<long long, int> freq;
            freq.reserve(m * 2);

            int maxCnt = 0;
            for (long long val : pred) {
                int c = ++freq[val];
                if (c > maxCnt) maxCnt = c;
            }

            int distinct = (int)freq.size();

            if (maxCnt < bestMax || (maxCnt == bestMax && distinct > bestDistinct)) {
                bestMax = maxCnt;
                bestDistinct = distinct;
                bestX = x;
                bestPred = pred;
            }
        };

        for (int u = 1; u <= n; ++u) {
            vector<long long> pred(m);
            for (int i = 0; i < m; ++i) {
                pred[i] = F[i][u];
            }

            vector<int> x(n + 1, 0);
            x[u] = 1;
            evaluate(pred, x);

            if (bestMax == 1) break;
        }

        if (bestMax > 1) {
            vector<int> x(n + 1, 1);
            x[0] = 0;

            vector<long long> pred(m, 0);
            for (int i = 0; i < m; ++i) {
                long long sum = 0;
                for (int u = 1; u <= n; ++u) {
                    sum += F[i][u];
                }
                pred[i] = sum;
            }

            evaluate(pred, x);
        }

        for (int attempt = 0; attempt < 40 && bestMax > 1; ++attempt) {
            int pct = (attempt < 25 ? 50 : 25);
            vector<int> x(n + 1, 0);

            int k = 0;
            for (int u = 1; u <= n; ++u) {
                if ((int)(rng() % 100) < pct) {
                    x[u] = 1;
                    ++k;
                }
            }

            if (k == 0) {
                int u = 1 + (int)(rng() % n);
                x[u] = 1;
            }

            vector<long long> pred(m, 0);
            for (int i = 0; i < m; ++i) {
                long long sum = 0;
                for (int u = 1; u <= n; ++u) {
                    if (x[u]) sum += F[i][u];
                }
                pred[i] = sum;
            }

            evaluate(pred, x);
        }

        if (bestMax == m) {
            bool found = false;
            for (int u = 1; u <= n && !found; ++u) {
                for (int i = 1; i < m; ++i) {
                    if (F[i][u] != F[0][u]) {
                        vector<int> x(n + 1, 0);
                        x[u] = 1;

                        vector<long long> pred(m);
                        for (int j = 0; j < m; ++j) {
                            pred[j] = F[j][u];
                        }

                        evaluate(pred, x);
                        found = true;
                        break;
                    }
                }
            }
        }

        vector<int> nodes;
        for (int u = 1; u <= n; ++u) {
            for (int c = 0; c < bestX[u]; ++c) {
                nodes.push_back(u);
            }
        }

        if (nodes.empty()) nodes.push_back(1);

        long long actual = query1(nodes);

        vector<Cand> newC;
        for (int i = 0; i < m; ++i) {
            if (bestPred[i] == actual) {
                newC.push_back(C[i]);
            }
        }

        C.swap(newC);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    for (int i = 0; i < n - 1; ++i) {
        int a, b;
        cin >> a >> b;
    }

    int maxB = 0;
    while ((1LL << (maxB + 1)) - 1 <= n) {
        ++maxB;
    }
    if (maxB < 1) maxB = 1;

    int B = maxB;

    groupIdx.assign(n + 1, -1);
    coeffInGroup.assign(n + 1, 0);

    int cur = 2;
    int gid = 0;

    while (cur <= n) {
        int len = min(B, n - cur + 1);

        Group g;
        g.ksum = 0;

        for (int pos = 0; pos < len; ++pos) {
            int label = cur + pos;
            int c = 1 << pos;

            g.labels.push_back(label);
            g.coeff.push_back(c);
            g.ksum += c;

            groupIdx[label] = gid;
            coeffInGroup[label] = c;
        }

        groups.push_back(g);
        cur += len;
        ++gid;
    }

    long long A = query1({1});

    int G = (int)groups.size();
    vector<long long> resp(G);

    for (int g = 0; g < G; ++g) {
        vector<int> nodes;

        for (int pos = 0; pos < (int)groups[g].labels.size(); ++pos) {
            int label = groups[g].labels[pos];
            int c = groups[g].coeff[pos];

            for (int t = 0; t < c; ++t) {
                nodes.push_back(label);
            }
        }

        resp[g] = query1(nodes);
    }

    if (A % 2 != 0) {
        vector<int> v(n + 1, 0);
        v[1] = (int)A;

        for (int g = 0; g < G; ++g) {
            long long S = resp[g] - 1LL * groups[g].ksum * A;
            decodeGroup(groups[g], S, v);
        }

        outputAnswer(v);
        return 0;
    }

    vector<Cand> C;

    for (int r = 2; r <= n; ++r) {
        vector<int> v(n + 1, 0);
        bool ok = true;

        for (int g = 0; g < G; ++g) {
            long long x = 0;
            if (groupIdx[r] == g) {
                x = coeffInGroup[r];
            }

            long long S = resp[g] - (1LL * groups[g].ksum - x) * A;

            if (!decodeGroup(groups[g], S, v)) {
                ok = false;
                break;
            }
        }

        if (!ok) continue;

        int vr = v[r];
        long long v1ll = A - vr;

        if (v1ll != 1 && v1ll != -1) continue;

        v[1] = (int)v1ll;

        C.push_back({r, v});
    }

    resolveCandidates(C);

    return 0;
}
