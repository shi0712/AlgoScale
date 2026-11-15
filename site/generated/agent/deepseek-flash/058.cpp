#include <bits/stdc++.h>
using namespace std;

class FastScanner {
    static constexpr int BUFSIZE = 1 << 20;
    char buf[BUFSIZE];
    int idx = 0, size = 0;
public:
    inline char readChar() {
        if (idx >= size) {
            size = (int)fread(buf, 1, BUFSIZE, stdin);
            idx = 0;
            if (size == 0) return 0;
        }
        return buf[idx++];
    }

    template <typename T>
    bool nextInt(T &out) {
        char c;
        do {
            c = readChar();
            if (!c) return false;
        } while (c <= ' ');
        T val = 0;
        while (c > ' ') {
            val = val * 10 + (T)(c - '0');
            c = readChar();
        }
        out = val;
        return true;
    }
};

int main() {
    FastScanner fs;

    int N, Q;
    if (!fs.nextInt(N)) return 0;
    fs.nextInt(Q);

    static constexpr int MAXA = 1000000;
    // Initially frequencies; afterwards it becomes suffix counts.
    static int cnt[MAXA + 2];

    int A = 0;
    for (int i = 0; i < N; ++i) {
        int x = 0;
        fs.nextInt(x);
        ++cnt[x];
        if (x > A) A = x;
    }

    // cnt[y] = number of topping types with a_i >= y.
    for (int y = A - 1; y >= 1; --y) cnt[y] += cnt[y + 1];

    vector<unsigned long long> queries(Q);
    vector<unsigned char> needed(A + 1, 0);
    vector<int> neededT;
    neededT.reserve(2048);

    for (int j = 0; j < Q; ++j) {
        unsigned long long k = 0;
        fs.nextInt(k);
        queries[j] = k;
        int t = (int)((unsigned long long)(A - 1) / k + 1); // ceil(A / k)
        if (!needed[t]) {
            needed[t] = 1;
            neededT.push_back(t);
        }
    }

    vector<unsigned long long> f(A + 1, 0);

    // f[t] = sum_i floor(a_i / t) = sum_{m >= 1} cnt[m * t].
    for (int t : neededT) {
        unsigned long long s = 0;
        for (int m = t; m <= A; m += t) s += (unsigned int)cnt[m];
        f[t] = s;
    }

    string out;
    out.reserve((size_t)Q * 4);

    for (int j = 0; j < Q; ++j) {
        unsigned long long k = queries[j];
        int t = (int)((unsigned long long)(A - 1) / k + 1);
        out += (f[t] >= k ? "YES\n" : "NO\n");
    }

    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}
