#include <bits/stdc++.h>
using namespace std;

class FastInput {
    static constexpr int S = 1 << 20;
    char buf[S];
    int at = 0, len = 0;
    int getChar() {
        if (at == len) {
            len = fread(buf, 1, S, stdin);
            at = 0;
            if (len == 0) return EOF;
        }
        return buf[at++];
    }
public:
    int readInt() {
        int c = getChar();
        while (c <= ' ' && c != EOF) c = getChar();
        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + c - '0';
            c = getChar();
        }
        return x;
    }
};

int main() {
    FastInput in;
    int tests = in.readInt();
    while (tests--) {
        int n = in.readInt();
        in.readInt(); // The algorithm uses the actual height, which is <= m.
        vector<int> parent(n + 1), size(n + 1, 1), height(n + 1), heavy(n + 1), path(n + 1);
        for (int v = 2; v <= n; ++v) parent[v] = in.readInt();
        for (int v = n; v >= 2; --v) {
            int p = parent[v];
            size[p] += size[v];
            if (height[v] + 1 > height[p]) {
                height[p] = height[v] + 1;
                heavy[p] = v;
            }
            path[p] = max(path[p], size[v] + path[v]);
        }

        // Each longest downward path occupies consecutive positions. Its DP
        // array can be reused by its parent after inserting one front element.
        vector<int> pos(n + 1);
        int used = 0;
        for (int v = 1; v <= n; ++v) {
            if (v == 1 || heavy[parent[v]] != v) {
                for (int u = v; u; u = heavy[u]) pos[u] = used++;
            }
        }
        vector<int> head(n + 1, -1), next(n + 1, -1), internal;
        internal.reserve(n);
        for (int v = n; v >= 1; --v) {
            if (height[v] == 0) continue;
            internal.push_back(v);
            if (v != 1) {
                next[v] = head[parent[v]];
                head[parent[v]] = v;
            }
        }

        /*
        An equivalent objective is obtained by choosing one continuing child
        at each vertex. Let c[v] be its position in the resulting downward
        path. Each vertex contributes max(c[u]) over its ancestors u.

        F_v(k,l): optimal subtree score with previous maximum k and current
        path position l <= k. Set A_v(k)=F_v(k,1), D_v(k)=F_v(k,k).
        extra[v] stores A_v(k)-k*size[v]; diagonal[v] stores
        D_v(k+1)-(k+1)*size[v]. The array at pos[v] stores
        F_v(k,k-g)-A_v(k), indexed by g, with a common lazy offset.

        Continuing into a child shifts g by one. Merge children by maximum,
        then subtract the entry for l=1. Longest-path array reuse makes all
        light-child merges O(n) per k, using O(n) memory overall.
        */
        vector<int> extra(n + 1), diagonal = path, newDiagonal = path;
        vector<int> lazy(n + 1), data(n);
        for (int k = height[1]; k >= 1; --k) {
            for (int v : internal) {
                int* dst = data.data() + pos[v];
                int hc = heavy[v];
                if (height[v] < k) {
                    // No newly started path can exceed the baseline k.
                    dst[0] = path[v];
                    for (int c = head[v]; c != -1; c = next[c]) {
                        if (c == hc) continue;
                        const int* src = data.data() + pos[c];
                        for (int j = 0; j < height[c]; ++j)
                            dst[j + 1] = max(dst[j + 1], src[j]);
                    }
                    continue;
                }

                int sum = 0, best = 1;
                for (int c = head[v]; c != -1; c = next[c]) {
                    sum += extra[c];
                    best = max(best, size[c] + diagonal[c] - extra[c]);
                }
                int offset = lazy[hc];
                dst[0] = best - offset;
                for (int c = head[v]; c != -1; c = next[c]) {
                    if (c == hc) continue;
                    int count = min(height[c], k - 1);
                    int delta = lazy[c] - offset;
                    const int* src = data.data() + pos[c];
                    for (int j = 0; j < count; ++j)
                        dst[j + 1] = max(dst[j + 1], src[j] + delta);
                }
                int gain = dst[k - 1] + offset;
                extra[v] = sum + gain;
                newDiagonal[v] = sum + best;
                lazy[v] = offset - gain;
            }
            diagonal.swap(newDiagonal);
        }
        printf("%d\n", n + extra[1]);
    }
}
