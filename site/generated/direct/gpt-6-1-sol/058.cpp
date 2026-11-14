#include <cstdio>
#include <vector>
#include <algorithm>

class FastInput {
    static constexpr size_t SIZE = 1 << 20;
    char buffer[SIZE];
    size_t pos = 0, len = 0;

    inline char getChar() {
        if (pos == len) {
            len = std::fread(buffer, 1, SIZE, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buffer[pos++];
    }

public:
    template <class T>
    bool read(T& value) {
        char c;
        do {
            c = getChar();
            if (!c) return false;
        } while (c <= ' ');

        value = 0;
        while (c >= '0' && c <= '9') {
            value = value * 10 + (c - '0');
            c = getChar();
        }
        return true;
    }
};

class FastOutput {
    static constexpr size_t SIZE = 1 << 20;
    char buffer[SIZE];
    size_t pos = 0;

public:
    void answer(bool yes) {
        if (pos + 4 > SIZE) flush();
        if (yes) {
            buffer[pos++] = 'Y';
            buffer[pos++] = 'E';
            buffer[pos++] = 'S';
        } else {
            buffer[pos++] = 'N';
            buffer[pos++] = 'O';
        }
        buffer[pos++] = '\n';
    }

    void flush() {
        std::fwrite(buffer, 1, pos, stdout);
        pos = 0;
    }
};

static FastInput input;
static FastOutput output;

int main() {
    int N, Q;
    input.read(N);
    input.read(Q);

    constexpr int LIMIT = 1'000'000;
    std::vector<int> suffix(LIMIT + 1, 0);

    int maximum = 0;
    long long total = 0;

    for (int i = 0; i < N; ++i) {
        int a;
        input.read(a);
        ++suffix[a];
        maximum = std::max(maximum, a);
        total += a;
    }

    // suffix[x] = number of topping types with at least x pieces.
    for (int x = maximum - 1; x >= 1; --x) {
        suffix[x] += suffix[x + 1];
    }

    // full[t] = sum_i floor(a_i / t).
    std::vector<long long> full(maximum + 1, 0);
    full[1] = total;

    for (int t = 2; t <= maximum; ++t) {
        long long count = 0;
        for (int x = t; x <= maximum; x += t) {
            count += suffix[x];
        }
        full[t] = count;
    }

    while (Q--) {
        long long K;
        input.read(K);

        if (K > total) {
            output.answer(false);
        } else if (K >= maximum) {
            // Tastiness 1 works.
            output.answer(true);
        } else {
            int t = static_cast<int>((maximum - 1LL) / K + 1);
            output.answer(full[t] >= K);
        }
    }

    output.flush();
    return 0;
}
