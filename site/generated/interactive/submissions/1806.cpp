#include <bits/stdc++.h>
using namespace std;

static int N;
static vector<int> parArr, depArr, firstArr, lgEArr;
static vector<vector<int>> stE, upArr, childArr;

inline int lcaFast(int u, int v) {
    if (u == v) return u;
    int l = firstArr[u], r = firstArr[v];
    if (l > r) swap(l, r);
    int len = r - l + 1;
    int k = lgEArr[len];
    int a = stE[k][l];
    int b = stE[k][r - (1 << k) + 1];
    if (depArr[a] < depArr[b]) return a;
    if (depArr[b] < depArr[a]) return b;
    return a;
}

inline int distFast(int u, int v) {
    int w = lcaFast(u, v);
    return depArr[u] + depArr[v] - 2 * depArr[w];
}

int kthAncestor(int v, int steps) {
    for (int b = 0; steps; ++b) {
        if (steps & 1) v = upArr[b][v];
        steps >>= 1;
    }
    return v;
}

static uint32_t rngState = 712367821u;
inline uint32_t nextRand() {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return rngState;
}

struct Res {
    int worst;
    int k;
    int left;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N;

    parArr.assign(N + 1, 0);
    depArr.assign(N + 1, 0);
    childArr.assign(N + 1, {});

    for (int i = 2; i <= N; ++i) {
        cin >> parArr[i];
        childArr[parArr[i]].push_back(i);
    }

    for (int i = 2; i <= N; ++i) {
        depArr[i] = depArr[parArr[i]] + 1;
    }

    int LOG = 1;
    while ((1 << LOG) <= N) ++LOG;
    upArr.assign(LOG, vector<int>(N + 1, 0));

    for (int i = 1; i <= N; ++i) upArr[0][i] = parArr[i];
    for (int b = 1; b < LOG; ++b) {
        for (int i = 1; i <= N; ++i) {
            upArr[b][i] = upArr[b - 1][upArr[b - 1][i]];
        }
    }

    vector<int> euler;
    euler.reserve(2 * N);
    firstArr.assign(N + 1, -1);

    vector<int> it(N + 1, 0);
    vector<int> stk;
    stk.reserve(N);

    stk.push_back(1);
    euler.push_back(1);
    firstArr[1] = 0;

    while (!stk.empty()) {
        int v = stk.back();
        if (it[v] < (int)childArr[v].size()) {
            int c = childArr[v][it[v]++];
            euler.push_back(c);
            firstArr[c] = (int)euler.size() - 1;
            stk.push_back(c);
        } else {
            stk.pop_back();
            if (!stk.empty()) {
                euler.push_back(stk.back());
            }
        }
    }

    int E = (int)euler.size();
    lgEArr.assign(E + 1, 0);
    for (int i = 2; i <= E; ++i) lgEArr[i] = lgEArr[i / 2] + 1;

    int LG2 = lgEArr[E] + 1;
    stE.assign(LG2, vector<int>(E, 0));
    stE[0] = euler;

    for (int k = 1; k < LG2; ++k) {
        int half = 1 << (k - 1);
        int span = 1 << k;
        for (int i = 0; i + span <= E; ++i) {
            int a = stE[k - 1][i];
            int b = stE[k - 1][i + half];
            stE[k][i] = (depArr[a] < depArr[b] ? a : b);
        }
    }

    vector<int> cand;
    cand.reserve(N);
    for (int i = 1; i <= N; ++i) cand.push_back(i);
    sort(cand.begin(), cand.end(), [](int a, int b) {
        return firstArr[a] < firstArr[b];
    });

    vector<int> freq(N + 1, 0);
    vector<int> ds;
    ds.reserve(N);

    vector<int> mark(N + 1, 0);
    int token = 0;

    vector<int> cnt(N + 1, 0);

    while (true) {
        if (cand.empty()) return 0;
        if ((int)cand.size() == 1) break;

        int m = (int)cand.size();

        ++token;
        vector<int> centers;
        centers.reserve(1200);

        auto addCenter = [&](int v) {
            if (v >= 1 && v <= N && mark[v] != token) {
                mark[v] = token;
                centers.push_back(v);
            }
        };

        addCenter(1);

        if (m <= 500) {
            for (int x : cand) addCenter(x);
        }

        int qc, ql, rndCnt, pathCnt, halfCnt;
        if (m > 5000) {
            qc = 80;
            ql = 100;
            rndCnt = 25;
            pathCnt = 20;
            halfCnt = 80;
        } else if (m > 500) {
            qc = 60;
            ql = 80;
            rndCnt = 20;
            pathCnt = 20;
            halfCnt = 60;
        } else {
            qc = min(m, 100);
            ql = m;
            rndCnt = 25;
            pathCnt = 30;
            halfCnt = min(m, 60);
        }

        for (int j = 0; j < qc; ++j) {
            int idx = (int)((long long)j * m / qc);
            addCenter(cand[idx]);
        }

        for (int j = 0; j < rndCnt; ++j) {
            int idx = (int)(nextRand() % (uint32_t)m);
            addCenter(cand[idx]);
        }

        fill(cnt.begin(), cnt.end(), 0);
        for (int x : cand) ++cnt[x];
        for (int v = N; v >= 2; --v) {
            cnt[parArr[v]] += cnt[v];
        }

        int cen = 1;
        while (true) {
            int nxt = 0;
            for (int ch : childArr[cen]) {
                if (cnt[ch] > m / 2) {
                    nxt = ch;
                    break;
                }
            }
            if (nxt == 0) break;
            cen = nxt;
        }

        addCenter(cen);
        if (parArr[cen]) addCenter(parArr[cen]);
        for (int ch : childArr[cen]) addCenter(ch);

        for (int j = 0; j < ql; ++j) {
            int idx = (int)((long long)j * m / ql);
            int a = cand[idx];
            int b = cand[(idx + 1) % m];
            int l = lcaFast(a, b);
            addCenter(l);

            if (j < pathCnt) {
                int da = depArr[a] - depArr[l];
                int db = depArr[b] - depArr[l];
                if (da > 0) addCenter(kthAncestor(a, da / 2));
                if (db > 0) addCenter(kthAncestor(b, db / 2));
            }
        }

        for (int j = 0; j < 40 && m > 1; ++j) {
            int i1 = (int)(nextRand() % (uint32_t)m);
            int i2 = (int)(nextRand() % (uint32_t)m);
            addCenter(lcaFast(cand[i1], cand[i2]));
        }

        for (int j = 0; j < halfCnt; ++j) {
            int idx = (int)((long long)j * m / halfCnt);
            int x = cand[idx];

            int steps = depArr[x] / 2;
            if (steps > 0) addCenter(kthAncestor(x, steps));

            if (j < 20) {
                steps = depArr[x] / 3;
                if (steps > 0) addCenter(kthAncestor(x, steps));

                steps = (2 * depArr[x]) / 3;
                if (steps > 0) addCenter(kthAncestor(x, steps));
            }
        }

        if (centers.empty()) addCenter(cand[0]);

        auto evalCenter = [&](int u) -> Res {
            int bestW = m + 1;
            int bestKcur = 0;
            int bestL = 0;

            if (m <= 2000) {
                ds.resize(m);
                int ptr = 0;
                for (int x : cand) {
                    ds[ptr++] = distFast(u, x);
                }
                sort(ds.begin(), ds.end());

                int left = 0;
                for (int i = 0; i < m;) {
                    int d = ds[i];
                    while (i < m && ds[i] == d) {
                        ++i;
                        ++left;
                    }
                    int right = m - left;
                    int w = max(left, right);
                    int diff = abs(left - right);
                    int oldDiff = abs(bestL - (m - bestL));

                    if (w < bestW || (w == bestW && diff < oldDiff)) {
                        bestW = w;
                        bestKcur = d;
                        bestL = left;
                    }
                }
            } else {
                int maxd = 0;
                for (int x : cand) {
                    int d = distFast(u, x);
                    ++freq[d];
                    if (d > maxd) maxd = d;
                }

                int left = 0;
                for (int d = 0; d <= maxd; ++d) {
                    int c = freq[d];
                    freq[d] = 0;
                    if (c == 0) continue;

                    left += c;
                    int right = m - left;
                    int w = max(left, right);
                    int diff = abs(left - right);
                    int oldDiff = abs(bestL - (m - bestL));

                    if (w < bestW || (w == bestW && diff < oldDiff)) {
                        bestW = w;
                        bestKcur = d;
                        bestL = left;
                    }
                }
            }

            if (bestW == m + 1) {
                bestW = m;
                bestKcur = 0;
                bestL = 0;
            }

            return {bestW, bestKcur, bestL};
        };

        Res best;
        best.worst = m + 1;
        best.k = 0;
        best.left = 0;
        int bestU = cand[0];

        for (int u : centers) {
            Res r = evalCenter(u);
            int diffNew = abs(r.left * 2 - m);
            int diffOld = abs(best.left * 2 - m);

            if (r.worst < best.worst ||
                (r.worst == best.worst && diffNew < diffOld)) {
                best = r;
                bestU = u;
            }
        }

        if (best.worst >= m) {
            bestU = cand[0];
            best.k = 0;
            best.worst = m - 1;
            best.left = 1;
        }

        for (int step = 0; step < 6; ++step) {
            Res base = best;
            int baseU = bestU;

            Res candidateBest = base;
            int candidateU = baseU;

            vector<int> neigh;
            if (parArr[baseU]) neigh.push_back(parArr[baseU]);
            for (int ch : childArr[baseU]) neigh.push_back(ch);

            for (int v : neigh) {
                Res r = evalCenter(v);
                int diffNew = abs(r.left * 2 - m);
                int diffOld = abs(candidateBest.left * 2 - m);

                if (r.worst < candidateBest.worst ||
                    (r.worst == candidateBest.worst && diffNew < diffOld)) {
                    candidateBest = r;
                    candidateU = v;
                }
            }

            int diffNew = abs(candidateBest.left * 2 - m);
            int diffOld = abs(base.left * 2 - m);

            if (candidateU != baseU &&
                (candidateBest.worst < base.worst ||
                 (candidateBest.worst == base.worst && diffNew < diffOld))) {
                best = candidateBest;
                bestU = candidateU;
            } else {
                break;
            }
        }

        cout << "? " << bestU << ' ' << best.k << '\n';
        cout.flush();

        int o;
        if (!(cin >> o)) return 0;
        if (o == -1) return 0;
        if (o != 0 && o != 1) return 0;

        vector<int> newCand;
        newCand.reserve(m);

        for (int x : cand) {
            int d = distFast(bestU, x);
            bool inside = (d <= best.k);
            if (inside == (o == 1)) {
                newCand.push_back(x);
            }
        }

        cand.swap(newCand);
    }

    cout << "! " << cand[0] << '\n';
    cout.flush();

    return 0;
}
