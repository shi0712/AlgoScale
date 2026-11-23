#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int64 n;

int64 ask(int64 l, int64 r) {
    cout << "xor " << l << ' ' << r << endl;
    int64 response;
    if (!(cin >> response)) exit(0);
    if (response == -1) exit(0);
    return response;
}

[[noreturn]] void answer(int64 a, int64 b, int64 c) {
    cout << "ans " << a << ' ' << b << ' ' << c << endl;
    exit(0);
}

int ceilLog2(int64 x) {
    if (x <= 1) return 0;
    return 64 - __builtin_clzll((unsigned long long)(x - 1));
}

// The interval [A, B] contains exactly two stolen books, and their XOR is W.
pair<int64, int64> recoverPair(int64 A, int64 B, int64 W) {
    int64 length = B - A + 1;
    if (length == 2) return {A, B};

    int simpleCost = ceilLog2(length);

    int h = 63 - __builtin_clzll((unsigned long long)W);
    int64 half = int64(1) << h;
    int64 blockSize = half << 1;

    int64 firstBlock = A / blockSize;
    int64 lastBlock = B / blockSize;
    int64 blockCount = lastBlock - firstBlock + 1;

    int blockCost = ceilLog2(blockCount) + (h == 0 ? 0 : 1);

    if (blockCost < simpleCost) {
        int64 lo = firstBlock, hi = lastBlock;

        // Locate the aligned block containing both numbers. Before that block
        // the interval XOR is zero; from that block onward it is W.
        while (lo < hi) {
            int64 mid = (lo + hi) >> 1;
            int64 end = min(B, (mid + 1) * blockSize - 1);
            if (ask(A, end) != 0)
                hi = mid;
            else
                lo = mid + 1;
        }

        int64 base = lo * blockSize;

        // With h = 0, the lower half of the block contains only base.
        if (h == 0) return {base, base ^ W};

        // The two numbers lie in opposite halves of this block.
        int64 cut = base + half - 1;
        int64 lower = ask(A, cut);
        return {lower, lower ^ W};
    }

    // Binary-search the first stolen number in the pair. The interval XOR is
    // zero before it and nonzero from it onward.
    int64 lo = A, hi = B;
    while (lo < hi) {
        int64 mid = (lo + hi) >> 1;
        if (ask(A, mid) != 0)
            hi = mid;
        else
            lo = mid + 1;
    }
    return {lo, lo ^ W};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    if (n == 3) answer(1, 2, 3);

    int64 total = ask(1, n);

    if (n == 4) {
        int64 missing = 4 ^ total;
        int64 a = -1, b = -1, c = -1;
        for (int64 i = 1; i <= 4; ++i) {
            if (i == missing) continue;
            if (a == -1) a = i;
            else if (b == -1) b = i;
            else c = i;
        }
        answer(a, b, c);
    }

    if (total == 0) {
        // At x = 2^k - 1, the first nonzero prefix XOR occurs when k is the
        // bit length of the smallest stolen number. It then equals that number,
        // and the other two stolen numbers are both greater than x.
        for (int k = 1;; ++k) {
            int64 x = (int64(1) << k) - 1;
            int64 value = ask(1, x);
            if (value != 0) {
                auto [b, c] = recoverPair(x + 1, n, value);
                answer(value, b, c);
            }
        }
    }

    // All three stolen numbers lie in (L, R].
    int64 L = 0, R = n;

    while (R - L > 3) {
        int64 x = (L + R) >> 1;
        int64 leftXor = ask(L + 1, x);

        if (leftXor == 0) {
            L = x;
            continue;
        }
        if (leftXor == total) {
            R = x;
            continue;
        }

        // The left part contains either one or two stolen numbers.
        int64 other = total ^ leftXor;

        // H1: leftXor is the single stolen number on the left.
        // H2: other is the single stolen number on the right.
        bool h1 = (leftXor > L && leftXor <= x);
        bool h2 = (other > x && other <= R);

        if (h1 && h2) {
            // Under H2, leftXor cannot itself be stolen: otherwise the other
            // member of a pair having XOR leftXor would have to be zero.
            if (ask(leftXor, leftXor) == leftXor)
                h2 = false;
            else
                h1 = false;
        }

        int64 single;
        int64 A, B, W;

        if (h1) {
            single = leftXor;
            A = x + 1;
            B = R;
            W = other;
        } else {
            single = other;
            A = L + 1;
            B = x;
            W = leftXor;
        }

        auto [b, c] = recoverPair(A, B, W);
        answer(single, b, c);
    }

    // Three distinct stolen numbers fit in this interval only if they are
    // precisely its three values.
    answer(L + 1, L + 2, L + 3);
}
