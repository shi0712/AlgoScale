#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

class FastInput {
    static constexpr int SIZE = 1 << 16;
    char buffer[SIZE];
    int index = 0, length = 0;

    char getChar() {
        if (index == length) {
            length = static_cast<int>(fread(buffer, 1, SIZE, stdin));
            index = 0;
            if (length == 0) return 0;
        }
        return buffer[index++];
    }

public:
    int nextInt() {
        char c;
        do {
            c = getChar();
        } while (c <= ' ' && c != 0);

        int result = 0;
        while (c >= '0' && c <= '9') {
            result = result * 10 + c - '0';
            c = getChar();
        }
        return result;
    }
};

int addMod(int a, int b) {
    int result = a + b;
    if (result >= MOD) result -= MOD;
    return result;
}

int modPow(int a, int exponent) {
    int result = 1;
    while (exponent > 0) {
        if (exponent & 1) {
            result = static_cast<long long>(result) * a % MOD;
        }
        a = static_cast<long long>(a) * a % MOD;
        exponent >>= 1;
    }
    return result;
}

int main() {
    FastInput input;
    int T = input.nextInt();

    string output;
    while (T--) {
        int n = input.nextInt();
        vector<int> position(n + 1);
        bool sorted = true;

        for (int i = 1; i <= n; ++i) {
            int value = input.nextInt();
            position[value] = i;
            if (value != i) sorted = false;
        }

        if (sorted) {
            output += "0\n";
            continue;
        }

        const int m = n - 1;
        const int inverse = modPow(m, MOD - 2);

        // rowPrefix[x][y] = sum_{t=0}^y f(x,t).
        // Only entries with x+y <= n-2 are needed.
        vector<int> rowPrefix(static_cast<size_t>(m) * m);
        vector<int> columnSum(m, 0);

        for (int x = 0; x < m; ++x) {
            int rowSum = 0;
            const size_t base = static_cast<size_t>(x) * m;

            for (int y = 0; y < m - x; ++y) {
                int value;
                if (x == 0 && y == 0) {
                    value = 1;
                } else {
                    int sum = addMod(columnSum[y], rowSum);
                    value = static_cast<long long>(sum) * inverse % MOD;
                }

                columnSum[y] = addMod(columnSum[y], value);
                rowSum = addMod(rowSum, value);
                rowPrefix[base + y] = rowSum;
            }
        }

        int total = 0;

        for (int split = 1; split < n; ++split) {
            int leftGreater = split;
            int rightLess = 0;

            for (int value = 1; value <= n; ++value) {
                if (position[value] <= split) {
                    --leftGreater;

                    if (rightLess > 0) {
                        size_t index =
                            static_cast<size_t>(leftGreater) * m
                            + rightLess - 1;
                        total = addMod(total, rowPrefix[index]);
                    }
                } else {
                    ++rightLess;
                }
            }
        }

        int answer = addMod(
            1, static_cast<long long>(total) * inverse % MOD
        );

        output += to_string(answer);
        output += '\n';
    }

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}
