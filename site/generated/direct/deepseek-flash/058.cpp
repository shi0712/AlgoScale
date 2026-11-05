#include <bits/stdc++.h>
using namespace std;

struct FastScanner {
    static constexpr int BUFSIZE = 1 << 20;
    int idx = 0, size = 0;
    char buf[BUFSIZE];

    char readChar() {
        if (idx >= size) {
            size = (int)fread(buf, 1, BUFSIZE, stdin);
            idx = 0;
            if (size == 0) return 0;
        }
        return buf[idx++];
    }

    template <class T>
    bool nextInt(T &out) {
        char c = readChar();
        if (!c) return false;
        while (c != '-' && (c < '0' || c > '9')) {
            c = readChar();
            if (!c) return false;
        }
        T sign = 1;
        if (c == '-') {
            sign = -1;
            c = readChar();
        }
        T num = 0;
        while (c >= '0' && c <= '9') {
            num = num * 10 + (c - '0');
            c = readChar();
        }
        out = num * sign;
        return true;
    }
};

const long long NEG = -(1LL << 60);

// ndp[t] = max_{0 <= j <= cnt, j*d <= t} dp[t-j*d] + pref[j]
// pref is concave: its increments are sorted decreasing.
void mergeDP(
    const vector<long long> &dp,
    vector<long long> &ndp,
    int d,
    const vector<long long> &pref,
    int k
) {
    int m = (int)pref.size() - 1;

    if (m == 0) {
        for (int i = 0; i <= k; ++i) ndp[i] = dp[i];
        return;
    }

    vector<int> qL(k + 1), qS(k + 1);

    for (int R = 0; R < d && R <= k; ++R) {
        int L = (k - R) / d;
        int head = 0, tail = 0;

        for (int i = 0; i <= L; ++i) {
            int minL = i - m;
            if (minL < 0) minL = 0;

            while (head < tail && qL[head] < minL) ++head;

            int pos = R + i * d;
            long long curOld = dp[pos];

            if (curOld != NEG) {
                int l2 = i;
                int newStart = i;

                while (head < tail) {
                    int l1 = qL[tail - 1];
                    int st1 = qS[tail - 1];
                    int expire1 = l1 + m;

                    if (st1 > expire1) {
                        --tail;
                        continue;
                    }

                    int lower = max(i, st1);
                    int dist = l2 - l1;
                    int lowX = lower - l2;
                    int maxX = m - dist;

                    int cross;

                    if (lowX > maxX) {
                        cross = expire1 + 1;
                    } else {
                        long long diff = curOld - dp[R + l1 * d];

                        int lo = lowX, hi = maxX, ansX = -1;
                        while (lo <= hi) {
                            int mid = (lo + hi) >> 1;
                            long long need = pref[mid + dist] - pref[mid];

                            if (need <= diff) {
                                ansX = mid;
                                hi = mid - 1;
                            } else {
                                lo = mid + 1;
                            }
                        }

                        if (ansX == -1) cross = expire1 + 1;
                        else cross = l2 + ansX;
                    }

                    if (cross <= st1) {
                        --tail;
                        continue;
                    }

                    newStart = cross;
                    break;
                }

                if (head == tail) newStart = i;

                qL[tail] = l2;
                qS[tail] = newStart;
                ++tail;
            }

            while (head + 1 < tail && qS[head + 1] <= i) ++head;

            if (head < tail) {
                int l = qL[head];
                ndp[pos] = dp[R + l * d] + pref[i - l];
            } else {
                ndp[pos] = NEG;
            }
        }
    }
}

int main() {
    FastScanner fs;

    int n, k;
    if (!fs.nextInt(n)) return 0;
    fs.nextInt(k);

    vector<array<int, 11>> W(n);
    vector<vector<pair<long long, int>>> groups(11);

    long long sumP = 0;
    long long sumFull = 0;

    for (int i = 0; i < n; ++i) {
        int p;
        fs.nextInt(p);
        sumP += p;

        for (int j = 1; j <= p; ++j) {
            fs.nextInt(W[i][j]);
        }

        long long full = W[i][p];
        sumFull += full;
        groups[p].push_back({full, i});
    }

    if (sumP <= k) {
        printf("%lld\n", sumFull);
        return 0;
    }

    if (k == 0) {
        printf("0\n");
        return 0;
    }

    vector<long long> pref[11];
    int cnt[11] = {};

    for (int d = 1; d <= 10; ++d) {
        auto &vec = groups[d];

        sort(vec.begin(), vec.end(), [](const auto &a, const auto &b) {
            if (a.first != b.first) return a.first > b.first;
            return a.second < b.second;
        });

        cnt[d] = (int)vec.size();
        pref[d].assign(cnt[d] + 1, 0);

        for (int i = 0; i < cnt[d]; ++i) {
            pref[d][i + 1] = pref[d][i] + vec[i].first;
        }
    }

    vector<vector<long long>> P(11), S(12);

    P[0].assign(k + 1, NEG);
    P[0][0] = 0;

    for (int d = 1; d <= 10; ++d) {
        P[d].assign(k + 1, NEG);

        if (cnt[d] == 0) {
            P[d] = P[d - 1];
        } else {
            mergeDP(P[d - 1], P[d], d, pref[d], k);
        }
    }

    S[11].assign(k + 1, NEG);
    S[11][0] = 0;

    for (int d = 10; d >= 1; --d) {
        S[d].assign(k + 1, NEG);

        if (cnt[d] == 0) {
            S[d] = S[d + 1];
        } else {
            mergeDP(S[d + 1], S[d], d, pref[d], k);
        }
    }

    long long ans = P[10][k];

    vector<long long> cur(k + 1), nxt(k + 1);

    for (int c = 2; c <= 10; ++c) {
        if (cnt[c] == 0) continue;

        int leftGroups = c - 1;
        int rightGroups = 10 - c;

        bool useLeft = (rightGroups <= leftGroups);

        if (useLeft) {
            cur = P[c - 1];

            for (int d = c + 1; d <= 10; ++d) {
                if (cnt[d] == 0) continue;

                mergeDP(cur, nxt, d, pref[d], k);
                cur.swap(nxt);
            }
        } else {
            cur = S[c + 1];

            for (int d = 1; d < c; ++d) {
                if (cnt[d] == 0) continue;

                mergeDP(cur, nxt, d, pref[d], k);
                cur.swap(nxt);
            }
        }

        // cur[t] = best full value using all groups except c, with exact cost t.
        int maxR = min(c - 1, k);
        int cntc = cnt[c];

        auto &vec = groups[c];

        for (int r = 1; r <= maxR; ++r) {
            int T = k - r;
            int maxM = T / c;

            int limitF = min(maxM, cntc);
            int limitG = min(maxM, cntc - 1);

            vector<long long> prefF(limitF + 1, NEG);

            long long best = NEG;
            for (int m = 0; m <= limitF; ++m) {
                long long e = cur[T - m * c];
                long long val = NEG;

                if (e != NEG) {
                    val = e + pref[c][m];
                }

                if (val > best) best = val;
                prefF[m] = best;
            }

            vector<long long> suffG;

            if (limitG >= 0) {
                suffG.assign(limitG + 2, NEG);

                best = NEG;
                for (int m = limitG; m >= 0; --m) {
                    long long e = cur[T - m * c];
                    long long val = NEG;

                    if (e != NEG) {
                        val = e + pref[c][m + 1];
                    }

                    if (val > best) best = val;
                    suffG[m] = best;
                }

                for (int idx = 0; idx < cntc; ++idx) {
                    int pos = idx + 1;
                    long long v = vec[idx].first;
                    int id = vec[idx].second;

                    long long bestFull = NEG;

                    int idxF = pos - 1;
                    if (idxF > limitF) idxF = limitF;
                    if (idxF >= 0) bestFull = prefF[idxF];

                    if (pos <= limitG) {
                        long long second = suffG[pos];

                        if (second != NEG) {
                            second -= v;
                            if (second > bestFull) bestFull = second;
                        }
                    }

                    if (bestFull != NEG) {
                        long long total = bestFull + W[id][r];
                        if (total > ans) ans = total;
                    }
                }
            } else {
                for (int idx = 0; idx < cntc; ++idx) {
                    int pos = idx + 1;
                    int id = vec[idx].second;

                    long long bestFull = NEG;

                    int idxF = pos - 1;
                    if (idxF > limitF) idxF = limitF;
                    if (idxF >= 0) bestFull = prefF[idxF];

                    if (bestFull != NEG) {
                        long long total = bestFull + W[id][r];
                        if (total > ans) ans = total;
                    }
                }
            }
        }
    }

    printf("%lld\n", ans);
    return 0;
}
