#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int G = 3;

int n;
int K;
vector<int> fact, invfact;
int Csmall[12][12];

long long mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

void ntt(vector<int>& a, bool invert) {
    int n = (int)a.size();

    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        int wlen = (int)mod_pow(G, (MOD - 1) / len);
        if (invert) wlen = (int)mod_pow(wlen, MOD - 2);

        for (int i = 0; i < n; i += len) {
            long long w = 1;
            for (int j = 0; j < len / 2; j++) {
                int u = a[i + j];
                int v = (int)(w * a[i + j + len / 2] % MOD);
                int x = u + v;
                if (x >= MOD) x -= MOD;
                int y = u - v;
                if (y < 0) y += MOD;
                a[i + j] = x;
                a[i + j + len / 2] = y;
                w = w * wlen % MOD;
            }
        }
    }

    if (invert) {
        int inv_n = (int)mod_pow(n, MOD - 2);
        for (int& x : a) x = (long long)x * inv_n % MOD;
    }
}

vector<int> convolution_trunc(const vector<int>& a, const vector<int>& b, int limit) {
    if (limit <= 0 || a.empty() || b.empty()) return {};

    if ((int)a.size() == 1) {
        vector<int> res(min(limit, (int)b.size()));
        for (int i = 0; i < (int)res.size(); i++) {
            res[i] = (long long)a[0] * b[i] % MOD;
        }
        return res;
    }
    if ((int)b.size() == 1) {
        vector<int> res(min(limit, (int)a.size()));
        for (int i = 0; i < (int)res.size(); i++) {
            res[i] = (long long)b[0] * a[i] % MOD;
        }
        return res;
    }

    long long work = 1LL * a.size() * b.size();
    if (work <= 4096 || min(a.size(), b.size()) <= 16) {
        int sz = min(limit, (int)a.size() + (int)b.size() - 1);
        vector<int> res(sz, 0);
        for (int i = 0; i < (int)a.size(); i++) {
            if (a[i] == 0) continue;
            int maxj = min((int)b.size(), sz - i);
            for (int j = 0; j < maxj; j++) {
                res[i + j] = (res[i + j] + (long long)a[i] * b[j]) % MOD;
            }
        }
        return res;
    }

    int need = (int)a.size() + (int)b.size() - 1;
    int sz = 1;
    while (sz < need) sz <<= 1;

    vector<int> fa(sz, 0), fb(sz, 0);
    copy(a.begin(), a.end(), fa.begin());
    copy(b.begin(), b.end(), fb.begin());

    ntt(fa, false);
    ntt(fb, false);

    for (int i = 0; i < sz; i++) {
        fa[i] = (long long)fa[i] * fb[i] % MOD;
    }

    ntt(fa, true);

    fa.resize(min(limit, need));
    return fa;
}

struct Node {
    int l, r;
    int maxS;
    vector<vector<int>> F; // F[s], shifted by s*l
    vector<int> B;         // coefficients of segment, shifted by l
};

Node solve_segment(int l, int r) {
    if (l == r) {
        Node res;
        res.l = l;
        res.r = r;
        res.maxS = min(n, K / l);

        res.F.resize(res.maxS + 1);
        int c = invfact[l];

        for (int s = 1; s <= res.maxS; s++) {
            res.F[s] = vector<int>(1, (int)mod_pow(c, s));
        }

        res.B = vector<int>(1, c);
        return res;
    }

    int mid = (l + r) >> 1;

    Node left = solve_segment(l, mid);
    Node right = solve_segment(mid + 1, r);

    int Rl = mid + 1;
    int Llen = mid - l + 1;

    int maxQ = min(n, K / Rl);
    vector<vector<int>> powB(maxQ + 1);

    powB[0] = vector<int>{1};

    for (int q = 1; q <= maxQ; q++) {
        int limit = K - q * Rl + 1;
        if (limit <= 0) break;
        powB[q] = convolution_trunc(powB[q - 1], right.B, limit);
        if (powB[q].empty()) {
            maxQ = q - 1;
            powB.resize(maxQ + 1);
            break;
        }
    }

    Node res;
    res.l = l;
    res.r = r;
    res.maxS = min(n, K / l);
    res.F.resize(res.maxS + 1);

    for (int s = 1; s <= res.maxS; s++) {
        int targetOffset = s * l;
        int maxDeg = min(s * r, K);

        if (maxDeg < targetOffset) {
            res.F[s].clear();
            continue;
        }

        int len = maxDeg - targetOffset + 1;
        vector<int> cur(len, 0);

        // Suffixes completely inside the right segment.
        if (s < (int)right.F.size() && !right.F[s].empty()) {
            int diff = s * Rl - targetOffset;
            const vector<int>& add = right.F[s];
            for (int i = 0; i < (int)add.size(); i++) {
                int pos = diff + i;
                if (pos >= 0 && pos < len) {
                    cur[pos] += add[i];
                    if (cur[pos] >= MOD) cur[pos] -= MOD;
                }
            }
        }

        // Suffixes starting inside the left segment.
        for (int t = 0; t <= s; t++) {
            int q = s - t;

            if (q < 0 || q > maxQ) continue;
            if (powB[q].empty()) continue;

            int leftOffset = (t == 0 ? 0 : t * l);
            int rightOffset = q * Rl;
            int sumOffset = leftOffset + rightOffset;
            int diff = sumOffset - targetOffset;

            if (diff >= len) continue;

            int limit = len - diff;
            vector<int> prod;

            if (t == 0) {
                prod = powB[q];
            } else {
                if (t >= (int)left.F.size() || left.F[t].empty()) continue;
                prod = convolution_trunc(left.F[t], powB[q], limit);
            }

            long long mul = Csmall[s][t];
            if (t == 0) mul = mul * Llen % MOD;

            int upto = min((int)prod.size(), len - diff);
            for (int i = 0; i < upto; i++) {
                cur[diff + i] = (cur[diff + i] + mul * prod[i]) % MOD;
            }
        }

        res.F[s] = move(cur);
    }

    int up = min(r, K);
    res.B.resize(up - l + 1);
    for (int j = l; j <= up; j++) {
        res.B[j - l] = invfact[j];
    }

    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> K;

    fact.assign(K + 1, 1);
    invfact.assign(K + 1, 1);

    for (int i = 1; i <= K; i++) {
        fact[i] = (long long)fact[i - 1] * i % MOD;
    }

    invfact[K] = (int)mod_pow(fact[K], MOD - 2);
    for (int i = K; i >= 1; i--) {
        invfact[i - 1] = (long long)invfact[i] * i % MOD;
    }

    for (int i = 0; i <= 10; i++) {
        Csmall[i][0] = Csmall[i][i] = 1;
        for (int j = 1; j < i; j++) {
            Csmall[i][j] = Csmall[i - 1][j - 1] + Csmall[i - 1][j];
            if (Csmall[i][j] >= MOD) Csmall[i][j] -= MOD;
        }
    }

    Node root = solve_segment(1, K);

    long long answer = 0;

    for (int s = 1; s <= n; s++) {
        if (s > K) continue;

        vector<int> g(K + 1, 0);

        if (s < (int)root.F.size() && !root.F[s].empty()) {
            int offset = s; // root.l = 1
            for (int i = 0; i < (int)root.F[s].size(); i++) {
                int deg = offset + i;
                if (deg <= K) g[deg] = root.F[s][i];
            }
        }

        int other = n - s;
        long long T = 0;

        if (other == 0) {
            T = g[K];
        } else {
            vector<int> pw(K + 1);
            pw[0] = 1;
            for (int i = 1; i <= K; i++) {
                pw[i] = (long long)pw[i - 1] * other % MOD;
            }

            for (int d = 0; d <= K; d++) {
                if (g[d] == 0) continue;
                int e = K - d;
                long long ways = (long long)pw[e] * invfact[e] % MOD;
                T += (long long)g[d] * ways % MOD;
                if (T >= MOD) T -= MOD;
            }
        }

        T = T * fact[K] % MOD;

        long long comb = Csmall[n][s];
        long long add = comb * T % MOD;

        if (s & 1) {
            answer += add;
            if (answer >= MOD) answer -= MOD;
        } else {
            answer -= add;
            if (answer < 0) answer += MOD;
        }
    }

    cout << answer % MOD << '\n';
    return 0;
}
