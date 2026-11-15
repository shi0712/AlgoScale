#include <bits/stdc++.h>
using namespace std;

using ll = long long;

const int MAXA = 1000000 + 5;

int freqA[MAXA];
int prefA[MAXA];
ll F[MAXA];

struct FastScanner {
    static const int SZ = 1 << 20;
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
    inline bool nextInt(T &out) {
        char c = readChar();
        if (!c) return false;

        while (c != '-' && (c < '0' || c > '9')) {
            c = readChar();
            if (!c) return false;
        }

        T sign = 1;
        if (c == '-') {
            sign = -1;
            c = readChar();
        }

        T num = 0;
        while (c >= '0' && c <= '9') {
            num = num * 10 + (c - '0');
            c = readChar();
        }

        out = num * sign;
        return true;
    }
};

int main() {
    FastScanner fs;

    int N, Q;
    fs.nextInt(N);
    fs.nextInt(Q);

    int M = 0;
    ll S = 0;

    for (int i = 0; i < N; ++i) {
        int a;
        fs.nextInt(a);
        ++freqA[a];
        M = max(M, a);
        S += a;
    }

    for (int i = 1; i <= M; ++i) {
        prefA[i] = prefA[i - 1] + freqA[i];
    }

    /*
        F[t] = sum_i floor(a_i / t)

        Using:
        floor(x / t) = number of positive multiples of t not exceeding x.

        So:
        F[t] = sum_{m=t,2t,3t,...} count(a_i >= m)
    */
    for (int t = 1; t <= M; ++t) {
        ll sum = 0;
        for (int m = t; m <= M; m += t) {
            sum += (ll)(N - prefA[m - 1]);
        }
        F[t] = sum;
    }

    string output;
    output.reserve((size_t)Q * 4);

    for (int qi = 0; qi < Q; ++qi) {
        ll K;
        fs.nextInt(K);

        bool ok;

        if (K > M) {
            // Tastiness 1 is possible iff all pieces can make K nonempty cakes.
            ok = (K <= S);
        } else {
            int k = (int)K;
            int T = (M + k - 1) / k; // ceil(M / K)
            ok = (F[T] >= K);
        }

        if (ok) output += "YES\n";
        else output += "NO\n";
    }

    fwrite(output.c_str(), 1, output.size(), stdout);
    return 0;
}
