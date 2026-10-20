#include <bits/stdc++.h>
using namespace std;

struct FastOutput {
    static constexpr int SZ = 1 << 20;
    char buf[SZ];
    int ptr = 0;

    ~FastOutput() { flush(); }

    inline void push(char c) {
        if (ptr == SZ) flush();
        buf[ptr++] = c;
    }

    void flush() {
        if (ptr) {
            fwrite(buf, 1, ptr, stdout);
            ptr = 0;
        }
    }

    void writeInt(int x) {
        if (x == 0) {
            push('0');
            return;
        }
        if (x < 0) {
            push('-');
            x = -x;
        }
        char s[12];
        int n = 0;
        while (x > 0) {
            s[n++] = char('0' + x % 10);
            x /= 10;
        }
        while (n--) push(s[n]);
    }
};

struct Solver {
    int N, C, W;
    const vector<string>* S;
    const vector<int>* ord;
    vector<int16_t>* col;

    vector<uint64_t> used;
    vector<uint64_t> allowed;
    vector<uint64_t> unassigned;

    vector<int> matchColor;
    vector<int> tmpColor;
    vector<int> seen;
    int token = 0;

    bool dfs(int v, int tok) {
        int base = v * W;

        for (int w = 0; w < W; ++w) {
            uint64_t bits = allowed[base + w];

            while (bits) {
                int b = __builtin_ctzll(bits);
                bits &= bits - 1;

                int c = (w << 6) + b;
                if (c >= C) continue;
                if (seen[c] == tok) continue;

                seen[c] = tok;
                int u = matchColor[c];

                if (u == -1 || dfs(u, tok)) {
                    if (u == -1) {
                        unassigned[c >> 6] &= ~(1ULL << (c & 63));
                    }
                    matchColor[c] = v;
                    tmpColor[v] = c;
                    return true;
                }
            }
        }

        return false;
    }

    bool run() {
        used.assign((size_t)N * W, 0);
        fill(col->begin(), col->end(), int16_t(-1));

        seen.assign(C, 0);
        token = 0;

        uint64_t lastMask = (C % 64 == 0)
                                ? ~0ULL
                                : ((1ULL << (C % 64)) - 1);

        allowed.resize((size_t)N * W);
        tmpColor.assign(N, -1);

        for (int i = 0; i < N; ++i) {
            vector<uint64_t> avail(W);

            for (int w = 0; w < W; ++w) {
                avail[w] = ~used[(size_t)i * W + w];
            }
            avail[W - 1] &= lastMask;

            vector<int> neigh;
            neigh.reserve(N - i);

            for (int j = i + 1; j < N; ++j) {
                int u = (*ord)[i];
                int v = (*ord)[j];
                if ((*S)[u][v] == '1') {
                    neigh.push_back(j);
                }
            }

            int k = (int)neigh.size();
            if (k == 0) continue;

            vector<pair<int, int>> proc;
            proc.reserve(k);

            for (int v : neigh) {
                int base = v * W;
                int deg = 0;

                for (int w = 0; w < W; ++w) {
                    uint64_t a = avail[w] & ~used[(size_t)v * W + w];
                    allowed[base + w] = a;
                    deg += __builtin_popcountll(a);
                }

                proc.push_back({deg, v});
                tmpColor[v] = -1;
            }

            sort(proc.begin(), proc.end());

            matchColor.assign(C, -1);
            unassigned = avail;

            for (auto [deg, v] : proc) {
                int chosen = -1;
                int base = v * W;

                for (int w = 0; w < W; ++w) {
                    uint64_t b = allowed[base + w] & unassigned[w];
                    if (b) {
                        int bit = __builtin_ctzll(b);
                        chosen = (w << 6) + bit;
                        break;
                    }
                }

                if (chosen != -1) {
                    matchColor[chosen] = v;
                    tmpColor[v] = chosen;
                    unassigned[chosen >> 6] &= ~(1ULL << (chosen & 63));
                } else {
                    ++token;
                    if (!dfs(v, token)) {
                        return false;
                    }
                }
            }

            for (int v : neigh) {
                int c = tmpColor[v];
                if (c < 0) return false;

                (*col)[(size_t)i * N + v] = (int16_t)c;
                (*col)[(size_t)v * N + i] = (int16_t)c;

                uint64_t mask = 1ULL << (c & 63);

                used[(size_t)v * W + (c >> 6)] |= mask;
                used[(size_t)i * W + (c >> 6)] |= mask;
            }
        }

        return true;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<string> S(N);
    for (int i = 0; i < N; ++i) cin >> S[i];

    int C;
    if (N % 2 == 0) C = N - 3;
    else C = N - 2;

    int W = (C + 63) >> 6;

    vector<int> order(N);
    iota(order.begin(), order.end(), 0);

    vector<int> pos(N);

    vector<int16_t> col((size_t)N * N, int16_t(-1));

    bool ok = false;
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    Solver solver;
    solver.N = N;
    solver.C = C;
    solver.W = W;
    solver.S = &S;
    solver.ord = &order;
    solver.col = &col;

    for (int attempt = 0; attempt < 8 && !ok; ++attempt) {
        if (attempt > 0) {
            shuffle(order.begin(), order.end(), rng);
        }

        ok = solver.run();
    }

    if (!ok) {
        // This should not happen for the given graph class.
        // The retry mechanism above is only a safety net.
        return 0;
    }

    for (int i = 0; i < N; ++i) {
        pos[order[i]] = i;
    }

    FastOutput out;
    out.writeInt(C);
    out.push('\n');

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (j) out.push(' ');

            if (S[i][j] == '1') {
                int p = pos[i];
                int q = pos[j];
                int c = col[(size_t)p * N + q];
                out.writeInt(c + 1);
            } else {
                out.writeInt(-1);
            }
        }
        out.push('\n');
    }

    out.flush();
    return 0;
}
