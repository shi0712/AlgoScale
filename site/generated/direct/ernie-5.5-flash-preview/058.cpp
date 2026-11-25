#include <bits/stdc++.h>
using namespace std;

const int MAXA = 1000000;
int pref[MAXA + 2];
long long F[MAXA + 2];

namespace FastIO {
    const int SZ = 1 << 20;
    char buf[SZ];
    int pos = 0, len = 0;

    inline int getChar() {
        if (pos == len) {
            len = fread(buf, 1, SZ, stdin);
            pos = 0;
            if (len == 0) return -1;
        }
        return buf[pos++];
    }

    inline int nextInt() {
        int c;
        do { c = getChar(); } while (c <= 32 && c != -1);
        int x = 0;
        while (c > 32) {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return x;
    }

    inline long long nextLong() {
        int c;
        do { c = getChar(); } while (c <= 32 && c != -1);
        long long x = 0;
        while (c > 32) {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return x;
    }
}

const int OUTSZ = 1 << 20;
char outbuf[OUTSZ];
int outpos = 0;

inline void flushOut() {
    if (outpos) {
        fwrite(outbuf, 1, outpos, stdout);
        outpos = 0;
    }
}

inline void writeYesNo(bool yes) {
    if (yes) {
        if (outpos + 4 > OUTSZ) flushOut();
        outbuf[outpos++] = 'Y';
        outbuf[outpos++] = 'E';
        outbuf[outpos++] = 'S';
        outbuf[outpos++] = '\n';
    } else {
        if (outpos + 3 > OUTSZ) flushOut();
        outbuf[outpos++] = 'N';
        outbuf[outpos++] = 'O';
        outbuf[outpos++] = '\n';
    }
}

int main() {
    int N = FastIO::nextInt();
    int Q = FastIO::nextInt();

    int M = 0;
    long long S = 0;

    for (int i = 0; i < N; ++i) {
        int v = FastIO::nextInt();
        ++pref[v];
        if (v > M) M = v;
        S += v;
    }

    for (int v = 1; v <= M; ++v) {
        pref[v] += pref[v - 1];
    }

    for (int t = 1; t <= M; ++t) {
        long long sum = 0;
        int q = 1;
        for (int l = t; l <= M; l += t, ++q) {
            int r = l + t - 1;
            if (r > M) r = M;
            sum += (long long)q * (pref[r] - pref[l - 1]);
            if (r == M) break;
        }
        F[t] = sum;
    }

    for (int j = 0; j < Q; ++j) {
        long long K = FastIO::nextLong();
        int T;
        if (K > M) {
            T = 1;
        } else {
            T = (int)((M + K - 1) / K);
        }
        writeYesNo(F[T] >= K);
    }

    flushOut();
    return 0;
}
