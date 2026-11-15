#include <bits/stdc++.h>
using namespace std;

static const int MOD = 1'000'000'007;

static inline void mulMat3(const int* A, const int* B, int* C) {
    // C = A * B, all 3x3 row-major
    long long a00=A[0], a01=A[1], a02=A[2];
    long long a10=A[3], a11=A[4], a12=A[5];
    long long a20=A[6], a21=A[7], a22=A[8];

    C[0] = int((a00*B[0] + a01*B[3] + a02*B[6]) % MOD);
    C[1] = int((a00*B[1] + a01*B[4] + a02*B[7]) % MOD);
    C[2] = int((a00*B[2] + a01*B[5] + a02*B[8]) % MOD);

    C[3] = int((a10*B[0] + a11*B[3] + a12*B[6]) % MOD);
    C[4] = int((a10*B[1] + a11*B[4] + a12*B[7]) % MOD);
    C[5] = int((a10*B[2] + a11*B[5] + a12*B[8]) % MOD);

    C[6] = int((a20*B[0] + a21*B[3] + a22*B[6]) % MOD);
    C[7] = int((a20*B[1] + a21*B[4] + a22*B[7]) % MOD);
    C[8] = int((a20*B[2] + a21*B[5] + a22*B[8]) % MOD);
}

static inline int modPow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return int(r);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n;
        cin >> n;

        vector<int> lv(n), rv(n), lenMod(n), lMod(n), rMod(n);
        for (int i = 0; i < n; ++i) cin >> lv[i];
        for (int i = 0; i < n; ++i) cin >> rv[i];

        long long maxR = 0;
        long long totalArrays = 1;

        vector<pair<long long,int>> events;
        events.reserve(2 * n);

        for (int i = 0; i < n; ++i) {
            int l = lv[i], r = rv[i];
            lenMod[i] = r - l + 1;
            lMod[i] = l % MOD;
            rMod[i] = r % MOD;

            totalArrays = totalArrays * lenMod[i] % MOD;
            maxR = max(maxR, (long long)r);

            events.push_back({(long long)l + 1, i});
            events.push_back({(long long)r + 1, i});
        }

        sort(events.begin(), events.end());

        int N = n + 1; // possible polynomial evaluation points 0..n
        int size = 1;
        while (size < n) size <<= 1;

        int numNodes = 2 * size;
        int N9 = N * 9;

        vector<int> tree((size_t)numNodes * N9, 0);
        vector<int> mark(numNodes, 0);
        vector<int> affected;
        affected.reserve(numNodes);

        vector<int> typ(n, 0);
        // type:
        // 0 = forced high: h = len, L = 0
        // 1 = active:      h = r-v+1, L = v-l
        // 2 = forced low:  h = 0, L = len

        auto setLeafValue = [&](int p, int type, int e, int* C) {
            int h, L;
            if (type == 0) {
                h = lenMod[p];
                L = 0;
            } else if (type == 1) {
                L = e - lMod[p];
                if (L < 0) L += MOD;

                h = rMod[p] + 1 - e;
                if (h < 0) h += MOD;
            } else {
                h = 0;
                L = lenMod[p];
            }

            C[0] = 0; C[1] = 0; C[2] = h;
            C[3] = L; C[4] = 0; C[5] = 0;
            C[6] = 0; C[7] = L; C[8] = L;
        };

        auto setIdentity = [&](int* C) {
            C[0] = 1; C[1] = 0; C[2] = 0;
            C[3] = 0; C[4] = 1; C[5] = 0;
            C[6] = 0; C[7] = 0; C[8] = 1;
        };

        auto buildColumn = [&](int e) {
            for (int p = 0; p < size; ++p) {
                int node = size + p;
                int off = node * N9 + e * 9;
                int* C = &tree[off];

                if (p < n) {
                    setLeafValue(p, typ[p], e, C);
                } else {
                    setIdentity(C);
                }
            }

            for (int node = size - 1; node >= 1; --node) {
                int off  = node * N9 + e * 9;
                int offL = (node * 2) * N9 + e * 9;
                int offR = (node * 2 + 1) * N9 + e * 9;

                mulMat3(&tree[offR], &tree[offL], &tree[off]);
            }
        };

        auto setLeafRange = [&](int p, int type, int limit) {
            int node = size + p;
            for (int e = 0; e < limit; ++e) {
                int off = node * N9 + e * 9;
                setLeafValue(p, type, e, &tree[off]);
            }
        };

        auto recomputeAffected = [&](const vector<int>& posList, int limit) {
            affected.clear();

            for (int p : posList) {
                int node = (size + p) >> 1;
                while (node >= 1) {
                    if (!mark[node]) {
                        mark[node] = 1;
                        affected.push_back(node);
                    }
                    node >>= 1;
                }
            }

            sort(affected.begin(), affected.end(), greater<int>());

            for (int node : affected) {
                int off  = node * N9;
                int offL = (node * 2) * N9;
                int offR = (node * 2 + 1) * N9;

                for (int e = 0; e < limit; ++e) {
                    mulMat3(&tree[offR + e * 9],
                            &tree[offL + e * 9],
                            &tree[off + e * 9]);
                }
            }

            for (int node : affected) mark[node] = 0;
        };

        int activeCount = 0;
        int curK = 1; // we keep evaluations e = 0 .. curK-1

        buildColumn(0);

        int maxF = n + 2;
        vector<int> fact(maxF + 1), invfact(maxF + 1);
        fact[0] = 1;
        for (int i = 1; i <= maxF; ++i) {
            fact[i] = int(1LL * fact[i - 1] * i % MOD);
        }

        invfact[maxF] = modPow(fact[maxF], MOD - 2);
        for (int i = maxF; i >= 1; --i) {
            invfact[i - 1] = int(1LL * invfact[i] * i % MOD);
        }

        vector<int> y(N), q(n + 2), pref(n + 3), suff(n + 3);

        auto evalQ = [&](int m, long long t) -> int {
            // Q is degree <= m, known at 0..m in q[]
            if (t <= m) {
                return q[(int)t];
            }

            pref[0] = 1;
            for (int i = 0; i <= m; ++i) {
                long long x = (t - i) % MOD;
                pref[i + 1] = int(1LL * pref[i] * x % MOD);
            }

            suff[m + 1] = 1;
            for (int i = m; i >= 0; --i) {
                long long x = (t - i) % MOD;
                suff[i] = int(1LL * suff[i + 1] * x % MOD);
            }

            long long res = 0;

            for (int i = 0; i <= m; ++i) {
                long long num = 1LL * pref[i] * suff[i + 1] % MOD;
                long long term = 1LL * q[i] * num % MOD;
                term = term * invfact[i] % MOD;
                term = term * invfact[m - i] % MOD;

                if ((m - i) & 1) res -= term;
                else res += term;
            }

            res %= MOD;
            if (res < 0) res += MOD;
            return int(res);
        };

        auto queryInterval = [&](long long L, long long R, int K) -> int {
            // Current P(v) has degree <= K-1.
            // Need P at 0..K-1, then Q(t)=sum_{v=0}^{t-1} P(v).
            int rootOff = 1 * N9;

            for (int e = 0; e < K; ++e) {
                int base = rootOff + e * 9;
                int val = tree[base + 2] + tree[base + 5];
                if (val >= MOD) val -= MOD;
                val += tree[base + 8];
                if (val >= MOD) val -= MOD;
                y[e] = val;
            }

            q[0] = 0;
            for (int i = 0; i < K; ++i) {
                int val = q[i] + y[i];
                if (val >= MOD) val -= MOD;
                q[i + 1] = val;
            }

            int m = K; // Q degree <= K
            int sr = evalQ(m, R + 1);
            int sl = evalQ(m, L);

            int ans = sr - sl;
            if (ans < 0) ans += MOD;
            return ans;
        };

        long long sumD = 0;
        long long current = 1;
        int idx = 0;

        while (idx < (int)events.size()) {
            long long B = events[idx].first;

            if (B > current) {
                long long R = min(B - 1, maxR);
                if (current <= R) {
                    int add = queryInterval(current, R, curK);
                    sumD += add;
                    if (sumD >= MOD) sumD -= MOD;
                }
                current = B;
            }

            if (B > maxR) break;

            vector<int> poss;

            while (idx < (int)events.size() && events[idx].first == B) {
                poss.push_back(events[idx].second);
                ++idx;
            }

            sort(poss.begin(), poss.end());
            poss.erase(unique(poss.begin(), poss.end()), poss.end());

            int oldK = curK;
            int deltaActive = 0;

            for (int p : poss) {
                int newType;
                if (B <= lv[p]) newType = 0;
                else if (B <= rv[p]) newType = 1;
                else newType = 2;

                if (typ[p] == 1) --deltaActive;
                if (newType == 1) ++deltaActive;

                typ[p] = newType;
            }

            int newActive = activeCount + deltaActive;
            int newNeed = newActive + 1;

            if (newNeed <= oldK) {
                int limit = newNeed;

                for (int p : poss) {
                    setLeafRange(p, typ[p], limit);
                }

                recomputeAffected(poss, limit);
                curK = limit;
            } else {
                int limit = oldK;

                for (int p : poss) {
                    setLeafRange(p, typ[p], limit);
                }

                recomputeAffected(poss, limit);

                for (int e = oldK; e < newNeed; ++e) {
                    buildColumn(e);
                }

                curK = newNeed;
            }

            activeCount = newActive;
            current = B;
        }

        long long ans = ((maxR % MOD) * totalArrays % MOD - sumD) % MOD;
        if (ans < 0) ans += MOD;

        cout << ans << '\n';
    }

    return 0;
}
