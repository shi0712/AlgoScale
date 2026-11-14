#include <bits/stdc++.h>
using namespace std;

class FastScanner {
    static constexpr size_t BUFFER_SIZE = 1 << 16;
    char buffer[BUFFER_SIZE];
    size_t position = 0, length = 0;

    char getChar() {
        if (position >= length) {
            length = fread(buffer, 1, BUFFER_SIZE, stdin);
            position = 0;
            if (length == 0) return 0;
        }
        return buffer[position++];
    }

public:
    template <class T>
    T next() {
        char c;
        do {
            c = getChar();
        } while (c <= ' ');

        T value = 0;
        while (c > ' ') {
            value = value * 10 + static_cast<T>(c - '0');
            c = getChar();
        }
        return value;
    }
};

int main() {
    FastScanner input;

    const int N = input.next<int>();
    const int Q = input.next<int>();

    constexpr int LIMIT = 1000000;
    vector<int> atLeast(LIMIT + 2, 0);

    int maxA = 0;
    for (int i = 0; i < N; ++i) {
        const int a = input.next<int>();
        ++atLeast[a];
        maxA = max(maxA, a);
    }

    // Convert counts into suffix counts:
    // atLeast[x] = number of topping types with a_i >= x.
    for (int x = maxA - 1; x >= 1; --x) {
        atLeast[x] += atLeast[x + 1];
    }

    // fullGroups[t] = sum_i floor(a_i / t).
    vector<long long> fullGroups(maxA + 1, 0);
    for (int t = 1; t <= maxA; ++t) {
        long long total = 0;
        for (int multiple = t; multiple <= maxA; multiple += t) {
            total += atLeast[multiple];
        }
        fullGroups[t] = total;
    }

    string output;
    output.reserve(static_cast<size_t>(Q) * 4);

    const long long M = maxA;
    for (int i = 0; i < Q; ++i) {
        const long long K = input.next<long long>();

        // t = ceil(M / K), without addition that could overflow.
        long long t = M / K;
        if (M % K != 0) ++t;

        const bool possible = fullGroups[static_cast<size_t>(t)] >= K;
        output.append(possible ? "YES\n" : "NO\n");
    }

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}
