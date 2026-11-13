#include <bits/stdc++.h>
using namespace std;

const int MAXA = 1000000 + 5;

int freq[MAXA];
long long F[MAXA];

struct FastInput {
    static constexpr int SZ = 1 << 20;
    int idx = 0, size = 0;
    char buf[SZ];

    inline char readChar() {
        if (idx >= size) {
            size = (int)fread(buf, 1, SZ, stdin);
            idx = 0;
            if (size == 0) return 0;
        }
        return buf[idx++];
    }

    template <class T>
    bool nextInt(T &out) {
        char c = readChar();
        if (!c) return false;

        while (c != '-' && (c < '0' || c > '9')) {
            c = readChar();
            if (!c) return false;
        }

        int sign = 1;
        if (c == '-') {
            sign = -1;
            c = readChar();
        }

        T val = 0;
        while (c >= '0' && c <= '9') {
            val = val * 10 + (c - '0');
            c = readChar();
        }

        out = val * sign;
        return true;
    }
};

int main() {
    FastInput in;

    int N, Q;
    in.nextInt(N);
    in.nextInt(Q);

    int M = 0;
    for (int i = 0; i < N; ++i) {
        int a;
        in.nextInt(a);
        ++freq[a];
        if (a > M) M = a;
    }

    int running = 0;
    for (int t = M; t >= 1; --t) {
        running += freq[t];
        freq[t] = running;
    }

    for (int x = 1; x <= M; ++x) {
        long long sum = 0;
        for (int t = x; t <= M; t += x) {
            sum += freq[t];
        }
        F[x] = sum;
    }

    string output;
    output.reserve((size_t)Q * 4);

    for (int i = 0; i < Q; ++i) {
        long long K;
        in.nextInt(K);

        long long L = (M + K - 1) / K;

        if (F[(int)L] >= K) {
            output += "YES\n";
        } else {
            output += "NO\n";
        }
    }

    fwrite(output.c_str(), 1, output.size(), stdout);
    return 0;
}
