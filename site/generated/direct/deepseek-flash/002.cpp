#include <bits/stdc++.h>
using namespace std;

struct Witness {
    int K;          // 1-indexed wrapped block
    int r;          // prefix index: P ends at A[r], Q starts at A[r+1]
    int p;          // actual prefix length of wrapped block
    int t;          // K - 1
    vector<pair<int,int>> intervals; // intervals for labels K+1 ... m
};

struct Solver {
    int n, m;
    vector<int> A, B;
    vector<int> pref;
    vector<int> vals;
    unordered_map<int,int> prefId;
    vector<int> lcp; // lcp[pos] for A[pos..] and B[1..], 1-indexed pos

    Solver(const vector<int>& a, const vector<int>& b) : A(a), B(b) {
        n = A.size();
        m = B.size();
        pref.assign(n + 1, 0);
        for (int i = 1; i <= n; ++i) pref[i] = pref[i-1] ^ A[i-1];

        vals.assign(pref.begin(), pref.end());
        sort(vals.begin(), vals.end());
        vals.erase(unique(vals.begin(), vals.end()), vals.end());
        for (int i = 0; i < (int)vals.size(); ++i) prefId[vals[i]] = i;

        lcp.assign(n + 2, 0);
        for (int pos = 1; pos <= n; ++pos) {
            int L = 0;
            while (pos + L <= n && L < m && A[pos + L - 1] == B[L]) ++L;
            lcp[pos] = L;
        }
    }

    bool reconstructP(int K, int r, int pmod, Witness& w) {
        int t = K - 1;
        if (K == m) {
            int p = r;
            if ((p % 3 + 3) % 3 != pmod) return false;
            w.K = K; w.r = r; w.p = p; w.t = t;
            w.intervals.clear();
            return true;
        }

        int q = m - K;
        int N = n + 1;
        int V = vals.size();
        vector<unsigned char> dp((q + 1) * N * 3, 0);

        for (int i = 0; i <= n; ++i) {
            dp[((0 * N + i) * 3) + (i % 3)] = 1;
        }

        for (int idx = 1; idx <= q; ++idx) {
            int bv = B[K + idx - 1]; // B[K] in 0-index for label K+1
            int curBase = idx * N * 3;
            int prvBase = (idx - 1) * N * 3;

            for (int s = 0; s < 3; ++s) {
                vector<unsigned char> seen(3 * V, 0);
                for (int i = 0; i <= n; ++i) {
                    int target = pref[i] ^ bv;
                    auto it = prefId.find(target);
                    if (it != prefId.end()) {
                        int id = it->second;
                        int reqL = (i - 2 + 3) % 3;
                        if (seen[reqL * V + id]) {
                            dp[curBase + i * 3 + s] = 1;
                        }
                    }

                    if (dp[prvBase + i * 3 + s]) {
                        int id = prefId[pref[i]];
                        seen[(i % 3) * V + id] = 1;
                    }
                }
            }
        }

        int idx = q;
        int end = r;
        int s = pmod;
        if (!dp[((q * N + r) * 3) + pmod]) return false;

        vector<pair<int,int>> intervals(q);
        while (idx > 0) {
            int bv = B[K + idx - 1];
            int foundL = -1;
            for (int l = end - 1; l >= 0; --l) {
                if (end - l >= 2 && (end - l) % 3 == 2 &&
                    (pref[end] ^ pref[l]) == bv &&
                    dp[(((idx - 1) * N + l) * 3) + s]) {
                    foundL = l;
                    break;
                }
            }
            if (foundL == -1) return false;
            intervals[idx - 1] = {foundL + 1, end};
            end = foundL;
            --idx;
        }

        int p = end;
        if ((p % 3 + 3) % 3 != pmod) return false;

        w.K = K;
        w.r = r;
        w.p = p;
        w.t = t;
        w.intervals = intervals;
        return true;
    }

    bool solveClockwise(Witness& out) {
        if (n < m) return false;

        int W = (n + 1 + 63) / 64;
        vector<uint64_t> fNext((n + 1) * W, 0);

        // empty suffix: f[p] has bit p
        for (int p = 0; p <= n; ++p) {
            fNext[p * W + (p >> 6)] |= (1ULL << (p & 63));
        }

        int V = vals.size();

        for (int K = m; K >= 1; --K) {
            int t = K - 1;

            // Aggregate rows by start position modulo 3
            vector<uint64_t> aggAny(3 * W, 0), aggPos(3 * W, 0);
            for (int p = 0; p <= n; ++p) {
                int md = p % 3;
                const uint64_t* row = &fNext[p * W];
                for (int wd = 0; wd < W; ++wd) {
                    aggAny[md * W + wd] |= row[wd];
                    if (p > 0) aggPos[md * W + wd] |= row[wd];
                }
            }

            // Try all possible r: P ends at A[r], Q starts at A[r+1]
            for (int r = 0; r + t <= n; ++r) {
                bool exactQ;
                if (t == 0) exactQ = true;
                else {
                    exactQ = (r + 1 <= n && lcp[r + 1] >= t);
                }
                if (!exactQ) continue;

                for (int pmod = 0; pmod < 3; ++pmod) {
                    // p + sK - 1 == 0 (mod 3), sK = n - L
                    // 2p == 1 - n + r + t (mod 3)
                    int rhs = (1 - n + r + t) % 3;
                    if (rhs < 0) rhs += 3;
                    int needP = (2 * rhs) % 3;

                    if (needP != pmod) continue;

                    const vector<uint64_t>& useAgg = (r + t == n ? aggPos : aggAny);
                    const uint64_t* row = &useAgg[pmod * W];
                    if ((row[r >> 6] >> (r & 63)) & 1ULL) {
                        Witness w;
                        if (reconstructP(K, r, pmod, w)) {
                            out = w;
                            return true;
                        }
                    }
                }
            }

            if (K == 1) break;

            // Add block B[K-1] (1-indexed B_K) to the front of suffix
            int bv = B[K - 1];
            vector<uint64_t> newF((n + 1) * W, 0);

            // group[mod][prefix-id] stores OR of fNext[s] over processed s
            vector<uint64_t> group((size_t)3 * V * W, 0);

            for (int i = n; i >= 0; --i) {
                int s = i + 1;
                if (s <= n) {
                    int md = s % 3;
                    int id = prefId[pref[s]];
                    uint64_t* dst = &group[((size_t)md * V + id) * W];
                    const uint64_t* src = &fNext[s * W];
                    for (int wd = 0; wd < W; ++wd) dst[wd] |= src[wd];
                }

                int target = pref[i] ^ bv;
                auto it = prefId.find(target);
                if (it == prefId.end()) continue;

                int id = it->second;
                int reqMod = (i + 2) % 3;
                const uint64_t* src = &group[((size_t)reqMod * V + id) * W];
                uint64_t* dst = &newF[i * W];
                for (int wd = 0; wd < W; ++wd) dst[wd] = src[wd];
            }

            fNext.swap(newF);
        }

        return false;
    }
};

vector<int> getMoves(int a, int b) {
    // Find a sequence of moves from (a,b) to (0,1).
    // move 1: (x,y) -> (x-2,y+1)
    // move 2: (x,y) -> (x+1,y-2)
    if (a == 0 && b == 1) return {};

    int S = a + b;
    int sz = S + 1;
    vector<int> dist(sz * sz, -1), prev(sz * sz, -1), pmove(sz * sz, 0);
    queue<pair<int,int>> q;
    int startId = a * sz + b;
    dist[startId] = 0;
    q.push({a, b});

    int target = 0 * sz + 1;
    while (!q.empty()) {
        auto [x, y] = q.front(); q.pop();
        int id = x * sz + y;
        if (id == target) break;

        if (x >= 2) {
            int nx = x - 2, ny = y + 1;
            int nid = nx * sz + ny;
            if (dist[nid] == -1) {
                dist[nid] = dist[id] + 1;
                prev[nid] = id;
                pmove[nid] = 1;
                q.push({nx, ny});
            }
        }
        if (y >= 2) {
            int nx = x + 1, ny = y - 2;
            int nid = nx * sz + ny;
            if (dist[nid] == -1) {
                dist[nid] = dist[id] + 1;
                prev[nid] = id;
                pmove[nid] = 2;
                q.push({nx, ny});
            }
        }
    }

    if (dist[target] == -1) return {};

    vector<int> moves;
    int cur = target;
    while (cur != startId) {
        moves.push_back(pmove[cur]);
        cur = prev[cur];
    }
    reverse(moves.begin(), moves.end());
    return moves;
}

bool buildAndSimulate(const vector<int>& A, const vector<int>& B,
                      const Witness& w, vector<int>& ops) {
    int n = A.size(), m = B.size();
    int K = w.K, r = w.r, p = w.p, t = w.t;

    vector<int> label(n + 1, 0);

    // B_K: prefix and suffix
    for (int i = 1; i <= p; ++i) label[i] = K;
    int suffixStart = r + t + 1;
    for (int i = suffixStart; i <= n; ++i) label[i] = K;

    // B_{K+1} ... B_m
    int idx = 0;
    for (int j = K + 1; j <= m; ++j) {
        auto [l, rr] = w.intervals[idx++];
        for (int i = l; i <= rr; ++i) label[i] = j;
    }

    // B_1 ... B_{K-1}: exact single elements
    for (int j = 1; j <= K - 1; ++j) {
        int pos = r + j;
        if (pos < 1 || pos > n) return false;
        label[pos] = j;
    }

    for (int i = 1; i <= n; ++i) if (label[i] == 0) return false;

    struct Node { int lab, val; };
    deque<Node> dq;
    for (int i = 1; i <= n; ++i) dq.push_back({label[i], A[i - 1]});

    auto reduceLabel = [&](int lab, int a, int b) -> bool {
        vector<int> moves = getMoves(a, b);
        if (moves.empty() && !(a == 0 && b == 1)) {
            // Could be empty only if already target.
            if (!(a == 0 && b == 1)) return false;
        }

        for (int op : moves) {
            if (op == 1) {
                if (dq.size() < 2) return false;
                if (dq[0].lab != lab || dq[1].lab != lab) return false;
                int v = dq[0].val ^ dq[1].val;
                dq.pop_front();
                dq.pop_front();
                dq.push_back({lab, v});
                ops.push_back(1);
            } else {
                if (dq.size() < 2) return false;
                int sz = dq.size();
                if (dq[sz - 1].lab != lab || dq[sz - 2].lab != lab) return false;
                int v = dq[sz - 1].val ^ dq[sz - 2].val;
                dq.pop_back();
                dq.pop_back();
                dq.push_front({lab, v});
                ops.push_back(2);
            }
        }
        return true;
    };

    // Process B_K
    int suffixLen = n - (r + t);
    if (!reduceLabel(K, p, suffixLen)) return false;

    // Process B_{K+1} ... B_m
    idx = 0;
    for (int j = K + 1; j <= m; ++j) {
        auto [l, rr] = w.intervals[idx++];
        int sz = rr - l + 1;
        if (!reduceLabel(j, sz, 0)) return false;
    }

    // Optional verification of the final values
    if ((int)dq.size() != m) return false;
    for (int i = 0; i < m; ++i) {
        if (dq[i].val != B[i]) return false;
    }

    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int c, T;
    if (!(cin >> c >> T)) return 0;

    while (T--) {
        int n, m;
        cin >> n >> m;
        vector<int> A(n), B(m);
        for (int i = 0; i < n; ++i) cin >> A[i];
        for (int i = 0; i < m; ++i) cin >> B[i];

        int xa = 0, xb = 0;
        for (int x : A) xa ^= x;
        for (int x : B) xb ^= x;

        if (xa != xb) {
            cout << "No\n";
            continue;
        }

        if (m == 1) {
            cout << "Yes\n";
            for (int i = 0; i < n - 1; ++i) {
                if (i) cout << ' ';
                cout << 1;
            }
            cout << '\n';
            continue;
        }

        if (n == m) {
            if (A == B) {
                cout << "Yes\n\n";
            } else {
                cout << "No\n";
            }
            continue;
        }

        bool ok = false;
        vector<int> answer;

        // Try normal orientation
        {
            Solver solver(A, B);
            Witness w;
            if (solver.solveClockwise(w)) {
                vector<int> ops;
                if (buildAndSimulate(A, B, w, ops)) {
                    ok = true;
                    answer = ops;
                }
            }
        }

        // Try reversed orientation
        if (!ok) {
            vector<int> Ar(A.rbegin(), A.rend());
            vector<int> Br(B.rbegin(), B.rend());

            Solver solver(Ar, Br);
            Witness w;
            if (solver.solveClockwise(w)) {
                vector<int> ops;
                if (buildAndSimulate(Ar, Br, w, ops)) {
                    for (int& x : ops) x = 3 - x;
                    ok = true;
                    answer = ops;
                }
            }
        }

        if (!ok) {
            cout << "No\n";
        } else {
            cout << "Yes\n";
            for (int i = 0; i < (int)answer.size(); ++i) {
                if (i) cout << ' ';
                cout << answer[i];
            }
            cout << '\n';
        }
    }

    return 0;
}
