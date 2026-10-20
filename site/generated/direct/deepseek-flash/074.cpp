#include <bits/stdc++.h>
#include <immintrin.h>
using namespace std;

static const int MOD = 1000000007;
static const int MAXN = 2005;
static unsigned char commonMat[MAXN][MAXN];

static inline int mod_add(long long a, long long b) {
    a += b;
    if (a >= MOD) a -= MOD;
    return (int)a;
}

// Computes all intervals of p1 that are contiguous in every permutation.
__attribute__((target("avx2")))
void computeCommon(int n, int m, int M, const vector<int>& p1,
                   const vector<unsigned short>& posAtVertex) {
    vector<unsigned short> mn(M), mx(M);

    for (int l = 1; l <= n; ++l) {
        int x = p1[l];
        int base = x * M;

        for (int b = 0; b < M; b += 16) {
            __m256i v = _mm256_loadu_si256((const __m256i*)(&posAtVertex[base + b]));
            _mm256_storeu_si256((__m256i*)(&mn[b]), v);
            _mm256_storeu_si256((__m256i*)(&mx[b]), v);
        }

        commonMat[l][l] = 1;

        for (int r = l + 1; r <= n; ++r) {
            x = p1[r];
            base = x * M;

            for (int b = 0; b < M; b += 16) {
                __m256i v = _mm256_loadu_si256((const __m256i*)(&posAtVertex[base + b]));
                __m256i mnv = _mm256_loadu_si256((const __m256i*)(&mn[b]));
                __m256i mxv = _mm256_loadu_si256((const __m256i*)(&mx[b]));
                mnv = _mm256_min_epu16(mnv, v);
                mxv = _mm256_max_epu16(mxv, v);
                _mm256_storeu_si256((__m256i*)(&mn[b]), mnv);
                _mm256_storeu_si256((__m256i*)(&mx[b]), mxv);
            }

            int len = r - l;
            bool ok = true;

            int full = m / 16;
            int rem = m % 16;
            __m256i need = _mm256_set1_epi16((short)len);

            for (int b = 0; b < full && ok; ++b) {
                __m256i mnv = _mm256_loadu_si256((const __m256i*)(&mn[b * 16]));
                __m256i mxv = _mm256_loadu_si256((const __m256i*)(&mx[b * 16]));
                __m256i diff = _mm256_sub_epi16(mxv, mnv);
                __m256i cmp = _mm256_cmpeq_epi16(diff, need);
                int mask = _mm256_movemask_epi8(cmp);
                if (mask != -1) ok = false;
            }

            for (int k = full * 16; k < m && ok; ++k) {
                if ((int)mx[k] - (int)mn[k] != len) ok = false;
            }

            commonMat[l][r] = ok ? 1 : 0;
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    int M = ((m + 15) / 16) * 16;
    if (M == 0) M = 16;

    vector<unsigned short> posAtVertex((long long)(n + 1) * M, 0);
    vector<int> p1(n + 1);
    vector<int> pos1(n + 1);

    for (int k = 0; k < m; ++k) {
        for (int i = 1; i <= n; ++i) {
            int v;
            cin >> v;
            int p = i - 1;
            posAtVertex[(long long)v * M + k] = (unsigned short)p;
            if (k == 0) {
                p1[i] = v;
                pos1[v] = i;
            }
        }
    }

    computeCommon(n, m, M, p1, posAtVertex);

    // minL_end[r] = minimum l such that [l, r] is common
    // maxR_start[l] = maximum r such that [l, r] is common
    vector<int> minL_end(n + 1, n + 1), maxR_start(n + 1, 0);

    for (int r = 1; r <= n; ++r) {
        for (int l = 1; l <= r; ++l) {
            if (commonMat[l][r]) {
                minL_end[r] = l;
                break;
            }
        }
    }

    for (int l = 1; l <= n; ++l) {
        for (int r = n; r >= l; --r) {
            if (commonMat[l][r]) {
                maxR_start[l] = r;
                break;
            }
        }
    }

    const int LOG = 12;
    int lg[MAXN];
    lg[1] = 0;
    for (int i = 2; i <= n + 1; ++i) lg[i] = lg[i / 2] + 1;

    // Sparse tables for range minimum of minL_end and range maximum of maxR_start.
    vector<int> minST((long long)LOG * (n + 1), 0);
    vector<int> maxST((long long)LOG * (n + 1), 0);

    for (int i = 1; i <= n; ++i) {
        minST[i] = minL_end[i];
        maxST[i] = maxR_start[i];
    }

    for (int j = 1; j < LOG; ++j) {
        int step = 1 << (j - 1);
        for (int i = 1; i + (1 << j) - 1 <= n; ++i) {
            minST[j * (n + 1) + i] =
                min(minST[(j - 1) * (n + 1) + i],
                    minST[(j - 1) * (n + 1) + i + step]);
            maxST[j * (n + 1) + i] =
                max(maxST[(j - 1) * (n + 1) + i],
                    maxST[(j - 1) * (n + 1) + i + step]);
        }
    }

    auto queryMin = [&](int l, int r) {
        if (l > r) return n + 1;
        int j = lg[r - l + 1];
        return min(minST[j * (n + 1) + l],
                   minST[j * (n + 1) + r - (1 << j) + 1]);
    };

    auto queryMax = [&](int l, int r) {
        if (l > r) return 0;
        int j = lg[r - l + 1];
        return max(maxST[j * (n + 1) + l],
                   maxST[j * (n + 1) + r - (1 << j) + 1]);
    };

    vector<int> Ls, Rs;

    for (int l = 1; l <= n; ++l) {
        for (int r = l; r <= n; ++r) {
            if (!commonMat[l][r]) continue;

            bool strong = true;

            if (l < r) {
                if (queryMin(l, r - 1) < l) strong = false;
                if (strong && queryMax(l + 1, r) > r) strong = false;
            }

            if (strong) {
                Ls.push_back(l);
                Rs.push_back(r);
            }
        }
    }

    int K = (int)Ls.size();

    // Build strong interval tree.
    vector<int> parent(K, -1);

    for (int u = 0; u < K; ++u) {
        int best = -1;
        int bestLen = INT_MAX;

        int lu = Ls[u], ru = Rs[u];
        int lenu = ru - lu + 1;

        for (int v = 0; v < K; ++v) {
            if (u == v) continue;
            if (Ls[v] <= lu && ru <= Rs[v]) {
                int lenv = Rs[v] - Ls[v] + 1;
                if (lenv > lenu && lenv < bestLen) {
                    bestLen = lenv;
                    best = v;
                }
            }
        }

        parent[u] = best;
    }

    vector<vector<int>> children(K);

    for (int u = 0; u < K; ++u) {
        if (parent[u] != -1) {
            children[parent[u]].push_back(u);
        }
    }

    for (int u = 0; u < K; ++u) {
        sort(children[u].begin(), children[u].end(), [&](int a, int b) {
            return Ls[a] < Ls[b];
        });
    }

    // Sparse table for finding the first element of an interval in each permutation.
    vector<unsigned short> st((long long)m * LOG * (n + 1), 0);

    for (int k = 0; k < m; ++k) {
        long long baseK = (long long)k * LOG * (n + 1);

        for (int i = 1; i <= n; ++i) {
            st[baseK + i] = (unsigned short)i;
        }

        for (int j = 1; j < LOG; ++j) {
            int step = 1 << (j - 1);
            int limit = n - (1 << j) + 1;

            for (int i = 1; i <= limit; ++i) {
                int idx1 = st[baseK + (long long)(j - 1) * (n + 1) + i];
                int idx2 = st[baseK + (long long)(j - 1) * (n + 1) + i + step];

                int v1 = p1[idx1];
                int v2 = p1[idx2];

                int posv1 = posAtVertex[(long long)v1 * M + k];
                int posv2 = posAtVertex[(long long)v2 * M + k];

                st[baseK + (long long)j * (n + 1) + i] =
                    (posv1 <= posv2 ? idx1 : idx2);
            }
        }
    }

    vector<unsigned char> rootable(K, 0);
    vector<int> rootVert(K, -1);

    for (int u = 0; u < K; ++u) {
        int l = Ls[u], r = Rs[u];
        int len = r - l + 1;
        int j = lg[len];
        int offset = r - (1 << j) + 1;

        int first = -1;
        bool ok = true;

        for (int k = 0; k < m; ++k) {
            long long baseK = (long long)k * LOG * (n + 1);

            int idx1 = st[baseK + (long long)j * (n + 1) + l];
            int idx2 = st[baseK + (long long)j * (n + 1) + offset];

            int v1 = p1[idx1];
            int v2 = p1[idx2];

            int posv1 = posAtVertex[(long long)v1 * M + k];
            int posv2 = posAtVertex[(long long)v2 * M + k];

            int v = (posv1 <= posv2 ? v1 : v2);

            if (first == -1) first = v;
            else if (first != v) {
                ok = false;
                break;
            }
        }

        rootable[u] = ok;
        if (ok) rootVert[u] = first;
    }

    vector<int> order(K);
    iota(order.begin(), order.end(), 0);

    sort(order.begin(), order.end(), [&](int a, int b) {
        return (Rs[a] - Ls[a]) < (Rs[b] - Ls[b]);
    });

    vector<long long> dp(K, 0), f(K, 0);

    auto coverRange = [&](const vector<int>& ch, int L, int R) -> long long {
        if (L > R) return 1LL;

        int q = R - L + 1;
        vector<long long> depth(q + 2, 0), arr(q + 2, 0), nd(q + 2, 0);
        depth[0] = 1;

        for (int ii = L; ii <= R; ++ii) {
            fill(arr.begin(), arr.end(), 0);

            int ci = ch[ii];
            long long fi = f[ci];
            long long di = dp[ci];

            long long split = fi - di;
            if (split < 0) split += MOD;

            for (int d = 0; d <= q; ++d) {
                long long cur = depth[d];
                if (!cur) continue;

                if (split) {
                    arr[d] += cur * split % MOD;
                    if (arr[d] >= MOD) arr[d] -= MOD;
                }
                if (di) {
                    arr[d + 1] += cur * di % MOD;
                    if (arr[d + 1] >= MOD) arr[d + 1] -= MOD;
                }
            }

            if (ii == R) {
                long long ans = 0;
                for (int d = 0; d <= q; ++d) {
                    ans += arr[d];
                    if (ans >= MOD) ans -= MOD;
                }
                return ans;
            } else {
                fill(nd.begin(), nd.end(), 0);

                long long suf = 0;
                for (int d = q; d >= 0; --d) {
                    suf += arr[d];
                    if (suf >= MOD) suf -= MOD;
                    nd[d] = suf;
                }

                depth.swap(nd);
            }
        }

        return 1LL;
    };

    for (int u : order) {
        const vector<int>& ch = children[u];
        int kc = (int)ch.size();

        if (kc == 0) {
            dp[u] = 1;
            f[u] = 1;
            continue;
        }

        if (kc == 1) {
            dp[u] = rootable[u] ? dp[ch[0]] : 0;
            f[u] = f[ch[0]];
            continue;
        }

        bool isLinear = true;
        bool hasReverse = false;

        for (int k = 0; k < m; ++k) {
            int v0 = p1[Ls[ch[0]]];
            int v1 = p1[Ls[ch[1]]];

            bool same =
                posAtVertex[(long long)v0 * M + k] <
                posAtVertex[(long long)v1 * M + k];

            for (int i = 1; i < kc - 1; ++i) {
                int a = p1[Ls[ch[i]]];
                int b = p1[Ls[ch[i + 1]]];

                bool before =
                    posAtVertex[(long long)a * M + k] <
                    posAtVertex[(long long)b * M + k];

                if (before != same) {
                    isLinear = false;
                    break;
                }
            }

            if (!isLinear) break;

            if (!same) hasReverse = true;
        }

        if (isLinear && !hasReverse) {
            f[u] = coverRange(ch, 0, kc - 1);
            dp[u] = 0;

            if (rootable[u]) {
                int pos = pos1[rootVert[u]];
                int idx = -1;

                for (int i = 0; i < kc; ++i) {
                    if (Ls[ch[i]] <= pos && pos <= Rs[ch[i]]) {
                        idx = i;
                        break;
                    }
                }

                if (idx == 0) {
                    long long val = dp[ch[0]];
                    val = val * coverRange(ch, 1, kc - 1) % MOD;
                    dp[u] = val;
                }
            }
        } else {
            long long prod = 1;
            for (int c : ch) prod = prod * f[c] % MOD;

            long long dpu = 0;

            if (rootable[u]) {
                int pos = pos1[rootVert[u]];
                int idx = -1;

                for (int i = 0; i < kc; ++i) {
                    if (Ls[ch[i]] <= pos && pos <= Rs[ch[i]]) {
                        idx = i;
                        break;
                    }
                }

                if (idx != -1) {
                    dpu = dp[ch[idx]];
                    for (int i = 0; i < kc; ++i) {
                        if (i == idx) continue;
                        dpu = dpu * f[ch[i]] % MOD;
                    }
                }
            }

            dp[u] = dpu;
            f[u] = prod;

            // Prime node: if the whole interval S can also be used as one subtree,
            // add that possibility. For a mixed linear node this is impossible.
            if (!isLinear && rootable[u]) {
                f[u] += dp[u];
                if (f[u] >= MOD) f[u] -= MOD;
            }
        }
    }

    int root = -1;
    for (int u = 0; u < K; ++u) {
        if (Ls[u] == 1 && Rs[u] == n) {
            root = u;
            break;
        }
    }

    cout << dp[root] % MOD << '\n';
    return 0;
}
