#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> st;          // -1 = invalid, 0..3 = last two dishonest bits
int cnt[4];
int totalCand;

int queryInterval(int l, int r) {
    cout << "? " << l << " " << r << "\n";
    cout.flush();
    int c;
    if (!(cin >> c)) exit(0);
    if (c == -1) exit(0);
    return c;
}

void updatePrefix(int r, int a) {
    int nc[4] = {0, 0, 0, 0};
    int nt = 0;
    for (int i = 1; i <= n; ++i) {
        int s = st[i];
        if (s < 0) continue;
        int z = (i <= r) ? 0 : 1;       // 0 = inside prefix, 1 = outside
        int nd = a ^ z;
        int p = s >> 1;
        int q = s & 1;
        if (p == q && p == nd) {
            st[i] = -1;
        } else {
            int ns = (q << 1) | nd;
            st[i] = ns;
            ++nc[ns];
            ++nt;
        }
    }
    memcpy(cnt, nc, sizeof(cnt));
    totalCand = nt;
}

int chooseTwoStepPrefix() {
    long long A0 = 2LL * cnt[0] + 2LL * cnt[2] + cnt[1];
    long long A1 = 2LL * cnt[3] + 2LL * cnt[1] + cnt[2];

    long long best = LLONG_MAX;
    int bestR = 1;
    long long Y = 0;

    for (int i = 1; i <= n; ++i) {
        int s = st[i];
        long long w = 0;
        if (s == 0) w = 2;
        else if (s == 3) w = -2;
        else if (s == 2) w = 1;
        else if (s == 1) w = -1;
        Y += w;

        long long val = max(A0 - Y, A1 + Y);
        if (val < best) {
            best = val;
            bestR = i;
        }
    }
    return bestR;
}

int chooseOneStepPrefix() {
    long long C = cnt[1] + cnt[2];
    long long n00 = cnt[0];
    long long n11 = cnt[3];

    long long best = LLONG_MAX;
    int bestR = 1;
    long long X = 0;

    for (int i = 1; i <= n; ++i) {
        int s = st[i];
        long long w = 0;
        if (s == 0) w = 1;
        else if (s == 3) w = -1;
        X += w;

        long long val = C + max(n00 - X, n11 + X);
        if (val < best) {
            best = val;
            bestR = i;
        }
    }
    return bestR;
}

vector<int> finishSmall(int m, const vector<int>& pos, const vector<int>& init) {
    if (m <= 2) return pos;

    vector<pair<int, int>> actions;
    for (int l = 1; l <= m; ++l)
        for (int r = l; r <= m; ++r)
            actions.push_back({l, r});
    int actCnt = (int)actions.size();

    int pow5[9];
    pow5[0] = 1;
    for (int i = 1; i <= m; ++i) pow5[i] = pow5[i - 1] * 5;
    int S = pow5[m];

    uint8_t trans[5][2][2];
    for (int s = 0; s < 5; ++s) {
        for (int z = 0; z < 2; ++z) {
            for (int a = 0; a < 2; ++a) {
                if (s == 4) {
                    trans[s][z][a] = 4;
                } else {
                    int p = s >> 1;
                    int q = s & 1;
                    int nd = a ^ z;
                    if (p == q && p == nd) trans[s][z][a] = 4;
                    else trans[s][z][a] = (q << 1) | nd;
                }
            }
        }
    }

    int contrib0[36][8][5];
    int contrib1[36][8][5];
    for (int idx = 0; idx < actCnt; ++idx) {
        int l = actions[idx].first;
        int r = actions[idx].second;
        for (int i = 0; i < m; ++i) {
            int z = (l <= i + 1 && i + 1 <= r) ? 0 : 1;
            for (int d = 0; d < 5; ++d) {
                contrib0[idx][i][d] = trans[d][z][0] * pow5[i];
                contrib1[idx][i][d] = trans[d][z][1] * pow5[i];
            }
        }
    }

    vector<uint32_t> nxt0((size_t)S * actCnt), nxt1((size_t)S * actCnt);
    vector<uint8_t> valid(S, 0), marked(S, 0), action(S, 255);
    vector<int> unmarked;

    for (int code = 0; code < S; ++code) {
        int tmp = code;
        uint8_t d[8];
        int vc = 0;
        for (int i = 0; i < m; ++i) {
            d[i] = (uint8_t)(tmp % 5);
            if (d[i] < 4) ++vc;
            tmp /= 5;
        }
        valid[code] = (uint8_t)vc;

        if (vc <= 2) {
            marked[code] = 1;
            action[code] = 254;
        } else {
            unmarked.push_back(code);
        }

        size_t base = (size_t)code * actCnt;
        for (int idx = 0; idx < actCnt; ++idx) {
            uint32_t c0 = 0, c1 = 0;
            for (int i = 0; i < m; ++i) {
                c0 += (uint32_t)contrib0[idx][i][d[i]];
                c1 += (uint32_t)contrib1[idx][i][d[i]];
            }
            nxt0[base + idx] = c0;
            nxt1[base + idx] = c1;
        }
    }

    // Retrograde attractor: a state is winning if some action has both
    // answers leading to already winning states.
    int scans = 0;
    while (!unmarked.empty() && scans < 100) {
        ++scans;
        vector<int> nextUnmarked;
        bool changed = false;
        for (int code : unmarked) {
            bool win = false;
            size_t base = (size_t)code * actCnt;
            for (int idx = 0; idx < actCnt; ++idx) {
                if (marked[nxt0[base + idx]] && marked[nxt1[base + idx]]) {
                    win = true;
                    action[code] = (uint8_t)idx;
                    break;
                }
            }
            if (win) {
                marked[code] = 1;
                changed = true;
            } else {
                nextUnmarked.push_back(code);
            }
        }
        unmarked.swap(nextUnmarked);
        if (!changed) break;
    }

    vector<int> cur = init;
    int code = 0;
    for (int i = 0; i < m; ++i) code += cur[i] * pow5[i];
    int vc = m;

    int steps = 0;
    while (vc > 2) {
        int idx = action[code];
        if (idx == 255) {
            // Should not be needed for valid instances; safe fallback.
            int bestIdx = 0, bestMax = INT_MAX;
            size_t base = (size_t)code * actCnt;
            for (int i = 0; i < actCnt; ++i) {
                int mx = max((int)valid[nxt0[base + i]], (int)valid[nxt1[base + i]]);
                if (mx < bestMax) {
                    bestMax = mx;
                    bestIdx = i;
                }
            }
            idx = bestIdx;
        }

        int l = actions[idx].first;
        int r = actions[idx].second;
        int c = queryInterval(pos[l - 1], pos[r - 1]);
        int len = pos[r - 1] - pos[l - 1] + 1;
        int a = c - (len - 1);

        int newCode = 0;
        int newVc = 0;
        for (int i = 0; i < m; ++i) {
            int z = (l - 1 <= i && i <= r - 1) ? 0 : 1;
            int nd = trans[cur[i]][z][a];
            cur[i] = nd;
            if (nd < 4) ++newVc;
            newCode += nd * pow5[i];
        }
        code = newCode;
        vc = newVc;

        if (++steps > 1000) break; // safety; should never trigger
    }

    vector<int> res;
    for (int i = 0; i < m; ++i)
        if (cur[i] < 4) res.push_back(pos[i]);
    if (res.size() > 2) res.resize(2);
    return res;
}

void outputCandidates(const vector<int>& cand) {
    vector<int> out;
    for (int x : cand) {
        if (out.size() < 2) out.push_back(x);
    }
    if (out.empty()) out.push_back(1);

    for (int x : out) {
        cout << "! " << x << "\n";
        cout.flush();
        int resp;
        if (!(cin >> resp)) exit(0);
        if (resp == 1) {
            cout << "#\n";
            cout.flush();
            exit(0);
        }
    }
    cout << "#\n";
    cout.flush();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    if (n <= 2) {
        vector<int> cand;
        for (int i = 1; i <= n; ++i) cand.push_back(i);
        outputCandidates(cand);
        return 0;
    }

    // Two full queries to establish the first two dishonest bits for everyone.
    int c1 = queryInterval(1, n);
    int b1 = c1 - (n - 1);
    int c2 = queryInterval(1, n);
    int b2 = c2 - (n - 1);

    st.assign(n + 1, -1);
    memset(cnt, 0, sizeof(cnt));
    totalCand = n;
    int initState = (b1 << 1) | b2;
    for (int i = 1; i <= n; ++i) {
        st[i] = initState;
        ++cnt[initState];
    }

    // Reduce to at most 8 candidates using a provable two-query block.
    int blocks = 0;
    while (totalCand > 8) {
        int r1 = chooseTwoStepPrefix();
        int c = queryInterval(1, r1);
        int a = c - (r1 - 1);
        updatePrefix(r1, a);

        int r2 = chooseOneStepPrefix();
        c = queryInterval(1, r2);
        a = c - (r2 - 1);
        updatePrefix(r2, a);

        if (++blocks > 200) break; // safety; should not trigger
    }

    vector<int> pos;
    vector<int> init;
    for (int i = 1; i <= n; ++i) {
        if (st[i] >= 0) {
            pos.push_back(i);
            init.push_back(st[i]);
        }
    }

    vector<int> finalCand = finishSmall((int)pos.size(), pos, init);
    outputCandidates(finalCand);
    return 0;
}
