#include <bits/stdc++.h>
using namespace std;

constexpr int MOD = 998244353;
constexpr int MAXN = 1'000'000;

int fact[MAXN + 1], invFact[MAXN + 1];

class FastInput {
    static constexpr size_t SIZE = 1 << 16;
    char buffer[SIZE];
    size_t pos = 0, len = 0;

    inline char getChar() {
        if (pos == len) {
            len = fread(buffer, 1, SIZE, stdin);
            pos = 0;
            if (!len) return 0;
        }
        return buffer[pos++];
    }

public:
    inline int readInt() {
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

int modPow(int a, int e) {
    int result = 1;
    while (e) {
        if (e & 1) result = (long long)result * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return result;
}

void prepareFactorials() {
    fact[0] = 1;
    for (int i = 1; i <= MAXN; ++i) {
        fact[i] = (long long)fact[i - 1] * i % MOD;
    }

    invFact[MAXN] = modPow(fact[MAXN], MOD - 2);
    for (int i = MAXN; i >= 1; --i) {
        invFact[i - 1] = (long long)invFact[i] * i % MOD;
    }
}

int main() {
    FastInput in;
    int t = in.readInt();
    prepareFactorials();

    string output;
    output.reserve(t * 12);

    while (t--) {
        int n = in.readInt();
        int k = in.readInt();

        // Empty selected set, or a singleton selected set.
        if (k <= 2) {
            for (int v = 2; v <= n; ++v) in.readInt();

            int answer = (k == 1)
                ? n
                : (long long)n * (n - 1) % MOD;

            output += to_string(answer);
            output += '\n';
            continue;
        }

        vector<int> parent(n + 1);
        vector<int> head(n + 1, 0);
        vector<int> nextSibling(n + 1, 0);
        vector<int> depth(n + 1, 0);
        vector<int> height(n + 1, 0);
        vector<int> offset(n + 1);

        for (int v = 2; v <= n; ++v) {
            int p = in.readInt();
            parent[v] = p;
            depth[v] = depth[p] + 1;
            nextSibling[v] = head[p];
            head[p] = v;
        }

        // Parents have smaller indices, so reverse order is bottom-up.
        for (int v = n; v >= 2; --v) {
            int p = parent[v];
            height[p] = max(height[p], height[v] + 1);
        }

        int totalLength = 0;
        for (int v = 1; v <= n; ++v) {
            offset[v] = totalLength;
            totalLength += height[v] + 1;
        }

        // down[offset[v] + j] = number of descendants of v
        // at distance j from v, including v itself for j = 0.
        vector<int> down(totalLength, 0);
        for (int v = 1; v <= n; ++v) {
            down[offset[v]] = 1;
        }

        for (int v = n; v >= 2; --v) {
            int src = offset[v];
            int dst = offset[parent[v]] + 1;
            for (int j = 0; j <= height[v]; ++j) {
                down[dst + j] += down[src + j];
            }
        }

        int m = k - 1;
        vector<int> ways(n + 1, 0);
        for (int x = m; x <= n; ++x) {
            ways[x] = (long long)fact[x] * invFact[m] % MOD
                    * invFact[x - m] % MOD;
        }

        int H = height[1];

        // At current center v:
        // count[H + q] counts vertices x with
        // q = distance(v, x) - depth[v].
        vector<int> count(2 * H + 1, 0);

        int current = 0;
        for (int j = 0; j <= H; ++j) {
            count[H + j] = down[offset[1] + j];
            current += ways[count[H + j]];
            if (current >= MOD) current -= MOD;
        }

        auto change = [&](int index, int delta) {
            int &value = count[index];

            current -= ways[value];
            if (current < 0) current += MOD;

            value += delta;

            current += ways[value];
            if (current >= MOD) current -= MOD;
        };

        // forward: reroot from parent[v] to v.
        // Otherwise undo that operation.
        auto moveCenter = [&](int v, bool forward) {
            int base = H - depth[v];
            int src = offset[v];

            for (int j = 0; j <= height[v]; ++j) {
                int amount = down[src + j];
                int low = base + j;
                int high = low + 2;

                if (forward) {
                    change(high, -amount);
                    change(low, amount);
                } else {
                    change(low, -amount);
                    change(high, amount);
                }
            }
        };

        int answer = current;

        // Iterative DFS. Consume each adjacency list as children are visited.
        int v = 1;
        while (true) {
            int u = head[v];

            if (u != 0) {
                head[v] = nextSibling[u];

                moveCenter(u, true);
                answer += current;
                if (answer >= MOD) answer -= MOD;

                v = u;
            } else {
                if (v == 1) break;
                moveCenter(v, false);
                v = parent[v];
            }
        }

        output += to_string(answer);
        output += '\n';
    }

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}
