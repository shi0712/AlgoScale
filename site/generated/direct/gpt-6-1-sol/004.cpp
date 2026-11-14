#include <bits/stdc++.h>
using namespace std;

/*
    All DP values fit in signed 32-bit integers under the given constraints.
    The answer is printed as a 64-bit integer.
*/

class FastInput {
    static constexpr int SZ = 1 << 20;
    char buffer[SZ];
    int at = 0, len = 0;

    char getChar() {
        if (at == len) {
            len = (int)fread(buffer, 1, SZ, stdin);
            at = 0;
            if (len == 0) return 0;
        }
        return buffer[at++];
    }

public:
    int readInt() {
        char c;
        do {
            c = getChar();
        } while (c <= ' ' && c);

        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return x;
    }
};

int main() {
    FastInput in;
    int t = in.readInt();

    while (t--) {
        int n = in.readInt();
        int m = in.readInt();
        (void)m;

        vector<int> parent(n + 1);
        for (int u = 2; u <= n; ++u) {
            parent[u] = in.readInt();
        }

        vector<int> sz(n + 1, 1);
        vector<int> height(n + 1, 0);
        vector<int> heavy(n + 1, 0);

        // W[u] = maximum sum of subtree sizes along a downward path from u.
        vector<int> W(n + 1, 0);
        vector<int> bestW(n + 1, 0);

        for (int u = n; u >= 1; --u) {
            W[u] = sz[u] + bestW[u];

            int p = parent[u];
            if (p) {
                sz[p] += sz[u];
                bestW[p] = max(bestW[p], W[u]);

                if (height[u] + 1 > height[p]) {
                    height[p] = height[u] + 1;
                    heavy[p] = u;
                }
            }
        }

        int H = height[1];

        // Consecutive positions along each longest-child chain.
        vector<int> pos(n + 1);
        int used = 0;
        for (int u = 1; u <= n; ++u) {
            if (parent[u] == 0 || heavy[parent[u]] != u) {
                for (int v = u; v; v = heavy[v]) {
                    pos[v] = used++;
                }
            }
        }

        // Only non-leaf children need explicit processing.
        vector<int> first(n + 1, -1), nxt(n + 1, -1);
        vector<int> leafCount(n + 1, 0);
        vector<int> internal;
        internal.reserve(n);

        for (int u = n; u >= 1; --u) {
            if (height[u] > 0) {
                internal.push_back(u);
            }

            if (u != 1) {
                int p = parent[u];
                if (height[u] == 0) {
                    ++leafCount[p];
                } else {
                    nxt[u] = first[p];
                    first[p] = u;
                }
            }
        }

        vector<int> prevA(n + 1), prevG(n + 1);
        vector<int> curA(n + 1), curG(n + 1);

        // Reusable longest-chain storage for the fixed-parameter DP.
        vector<int> buffer(n);
        vector<int> lazy(n + 1, 0);

        /*
            Initially use parameter H + 1, which is larger than the height
            of every subtree:
                G_u(B) = B * size[u]
                A_u(B) = B * size[u] + W[u].
        */
        for (int u : internal) {
            prevG[u] = (H + 1) * sz[u];
            prevA[u] = prevG[u] + W[u];
        }

        for (int B = H; B >= 1; --B) {
            // internal is already in reverse vertex order.
            for (int u : internal) {
                int h = heavy[u];
                int L;

                if (height[h] == 0) {
                    // A leaf's fixed-parameter profile is always [1].
                    buffer[pos[h]] = 1;
                    lazy[h] = 0;
                    L = 0;
                } else {
                    L = lazy[h];
                }

                int a = (B + 1) * (leafCount[u] + 1);
                int g = B * (leafCount[u] + 1);
                int best = (leafCount[u] > 0 ? 1 : 0);

                for (int v = first[u]; v != -1; v = nxt[v]) {
                    a += prevG[v];
                    g += curG[v];
                    best = max(best, prevA[v] - prevG[v]);

                    if (v == h) continue;

                    int len = min(B, height[v] + 1);
                    int from = pos[v];
                    int to = pos[h];
                    int delta = lazy[v] - L;

                    for (int j = 0; j < len; ++j) {
                        int value = buffer[from + j] + delta;
                        if (value > buffer[to + j]) {
                            buffer[to + j] = value;
                        }
                    }
                }

                // All leaf children contribute the same one-element profile.
                if (leafCount[u] > 0) {
                    buffer[pos[h]] = max(buffer[pos[h]], 1 - L);
                }

                a += best;

                int q = 0;
                if (height[u] >= B) {
                    q = buffer[pos[u] + B] + L;
                }

                g += q;
                curA[u] = a;
                curG[u] = g;

                /*
                    Shift the merged profile one position to the right,
                    subtract q lazily, and prepend A_u(B) - G_u(B).
                */
                lazy[u] = L - q;
                buffer[pos[u]] = a - g - lazy[u];
            }

            prevA.swap(curA);
            prevG.swap(curG);
        }

        // A_root(0), obtained from the parameter-1 values of its children.
        long long answer = 1LL + leafCount[1];
        int best = (leafCount[1] > 0 ? 1 : 0);

        for (int v = first[1]; v != -1; v = nxt[v]) {
            answer += prevG[v];
            best = max(best, prevA[v] - prevG[v]);
        }
        answer += best;

        printf("%lld\n", answer);
    }

    return 0;
}
