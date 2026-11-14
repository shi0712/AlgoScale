#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int MAXN = 1000000;

class FastInput {
    static constexpr int SIZE = 1 << 16;
    char buffer[SIZE];
    int pos = 0, len = 0;
public:
    inline char get() {
        if (pos == len) {
            len = (int)fread(buffer, 1, SIZE, stdin);
            pos = 0;
            if (!len) return 0;
        }
        return buffer[pos++];
    }
    int read() {
        char c;
        do c = get(); while (c <= ' ' && c);
        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + c - '0';
            c = get();
        }
        return x;
    }
};

static int fact[MAXN + 1], invfact[MAXN + 1], chooseCount[MAXN + 1];
static int parentOf[MAXN + 1], firstChild[MAXN + 1], nextSibling[MAXN + 1];
static int heightOf[MAXN + 1], offsetOf[MAXN + 1], downCount[2 * MAXN + 1];
static int sphere[3000];
static int currentSum, answer;

static int power(int a, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r = (long long)r * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return r;
}

inline void changeCount(int index, int delta) {
    int old = sphere[index];
    int now = old + delta;
    sphere[index] = now;
    currentSum += chooseCount[now] - chooseCount[old];
    if (currentSum < 0) currentSum += MOD;
    else if (currentSum >= MOD) currentSum -= MOD;
}

// The tree height is at most 1413: h <= number of children,
// and a path of h edges requires at least 1 + h(h+1)/2 vertices.
static void reroot(int v, int base) {
    answer += currentSum;
    if (answer >= MOD) answer -= MOD;
    for (int u = firstChild[v]; u; u = nextSibling[u]) {
        int childBase = base - 1;
        int length = heightOf[u] + 1;
        const int* down = downCount + offsetOf[u];
        // Moving the base shifts all old distances up by one. Replace
        // the child's subtree at those distances with its true distances.
        for (int d = 0; d < length; ++d) {
            changeCount(childBase + d, down[d]);
            changeCount(childBase + d + 2, -down[d]);
        }
        reroot(u, childBase);
        for (int d = length - 1; d >= 0; --d) {
            changeCount(childBase + d + 2, down[d]);
            changeCount(childBase + d, -down[d]);
        }
    }
}

int main() {
    FastInput in;
    int tests = in.read();
    fact[0] = 1;
    for (int i = 1; i <= MAXN; ++i) fact[i] = (long long)fact[i - 1] * i % MOD;
    invfact[MAXN] = power(fact[MAXN], MOD - 2);
    for (int i = MAXN; i >= 1; --i) invfact[i - 1] = (long long)invfact[i] * i % MOD;
    string output;
    output.reserve(tests * 12);
    while (tests--) {
        int n = in.read(), k = in.read();
        if (k <= 2) {
            for (int i = 2; i <= n; ++i) in.read();
            int result = k == 1 ? n : (long long)n * (n - 1) % MOD;
            output += to_string(result) + '\n';
            continue;
        }
        fill(firstChild + 1, firstChild + n + 1, 0);
        fill(heightOf + 1, heightOf + n + 1, 0);
        for (int v = 2; v <= n; ++v) {
            int p = parentOf[v] = in.read();
            nextSibling[v] = firstChild[p];
            firstChild[p] = v;
        }
        for (int v = n; v >= 2; --v) {
            int p = parentOf[v];
            heightOf[p] = max(heightOf[p], heightOf[v] + 1);
        }
        // Child counts strictly decrease along every downward path, so
        // height[v] <= child_count[v] and sum(height[v]) <= n - 1.
        // Thus these depth arrays and all reroot updates take O(n) space/time.
        int size = 0;
        for (int v = 1; v <= n; ++v) {
            offsetOf[v] = size;
            size += heightOf[v] + 1;
        }
        fill(downCount, downCount + size, 0);
        for (int v = 1; v <= n; ++v) downCount[offsetOf[v]] = 1;
        for (int v = n; v >= 2; --v) {
            int* to = downCount + offsetOf[parentOf[v]] + 1;
            const int* from = downCount + offsetOf[v];
            for (int d = 0; d <= heightOf[v]; ++d) to[d] += from[d];
        }
        int q = k - 1;
        fill(chooseCount, chooseCount + q, 0);
        for (int count = q; count <= n; ++count) {
            chooseCount[count] = (long long)fact[count] * invfact[q] % MOD
                                * invfact[count - q] % MOD;
        }
        int h = heightOf[1], base = h + 1;
        fill(sphere, sphere + 2 * h + 5, 0);
        currentSum = 0;
        answer = 0;
        for (int d = 0; d <= h; ++d) {
            int count = downCount[offsetOf[1] + d];
            sphere[base + d] = count;
            currentSum += chooseCount[count];
            if (currentSum >= MOD) currentSum -= MOD;
        }
        reroot(1, base);
        output += to_string(answer) + '\n';
    }
    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}
