#include <bits/stdc++.h>
using namespace std;

class FastInput {
    static constexpr size_t SIZE = 1 << 16;
    char buffer[SIZE];
    size_t pos = 0, len = 0;
    inline char get() {
        if (pos == len) {
            len = fread(buffer, 1, SIZE, stdin);
            pos = 0;
            if (!len) return 0;
        }
        return buffer[pos++];
    }
public:
    unsigned long long read() {
        char c;
        do { c = get(); } while (c <= ' ' && c);
        unsigned long long value = 0;
        while (c >= '0' && c <= '9') {
            value = value * 10 + (c - '0');
            c = get();
        }
        return value;
    }
};

int main() {
    FastInput in;
    int n = (int)in.read();
    int q = (int)in.read();
    vector<int> suffix(1000002, 0);
    int maximum = 0;
    unsigned long long total = 0;
    for (int i = 0; i < n; ++i) {
        int a = (int)in.read();
        ++suffix[a];
        maximum = max(maximum, a);
        total += a;
    }
    for (int a = maximum - 1; a >= 1; --a)
        suffix[a] += suffix[a + 1];

    // cached[t] = min(maximum, sum_i floor(a_i / t)).
    // Zero is an unused sentinel: the largest topping always contributes.
    vector<int> cached(maximum + 1, 0);
    string output;
    output.reserve((size_t)q * 4);
    while (q--) {
        unsigned long long k = in.read();
        bool possible;
        if (k >= (unsigned long long)maximum) {
            // Tastiness one permits every count from maximum through total.
            possible = k <= total;
        } else {
            int t = (int)((maximum - 1) / k + 1);
            if (cached[t] == 0) {
                int blocks = 0;
                // floor(a/t) counts multiples of t that do not exceed a.
                for (int x = t; x <= maximum; x += t) {
                    blocks += suffix[x];
                    if (blocks >= maximum) {
                        blocks = maximum;
                        break;
                    }
                }
                cached[t] = blocks;
            }
            possible = (unsigned long long)cached[t] >= k;
        }
        output += possible ? "YES\n" : "NO\n";
    }
    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}
