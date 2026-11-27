#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct MaxRange {
    int64 low;
    int64 high;
};

int64 bestAnswer = 0;

int64 highestPowerOfTwo(int64 x) {
    if (x <= 0) return 0;
    return 1LL << (63 - __builtin_clzll(static_cast<unsigned long long>(x)));
}

int64 minimumCoinCount(int64 sum, int64 cap) {
    return sum / cap + __builtin_popcountll(
        static_cast<unsigned long long>(sum % cap)
    );
}

MaxRange getMaxRange(int64 sum, int length) {
    if (length == 1) return {sum, sum};

    if (length == 2) {
        int64 value;
        if ((sum & (sum - 1)) == 0) {
            value = sum / 2;
        } else {
            value = highestPowerOfTwo(sum);
        }
        return {value, value};
    }

    int64 low = 1;
    for (int64 cap = 1; cap <= (1LL << 30); cap <<= 1) {
        if (minimumCoinCount(sum, cap) <= length) {
            low = cap;
            break;
        }
    }

    int64 budget = sum - length + 1;
    int64 high = (1LL << 30);
    if (budget < high) {
        high = highestPowerOfTwo(budget);
    }

    for (; high >= 1; high >>= 1) {
        int64 remainder = sum - high;
        int otherCount = length - 1;

        if (otherCount == 0) {
            if (remainder == 0) return {low, high};
        } else if (remainder >= otherCount &&
                   minimumCoinCount(remainder, high) <= otherCount) {
            return {low, high};
        }
    }

    return {low, low};
}

int64 ask(int left, int right) {
    cout << "? " << left << ' ' << right << endl;

    int64 response;
    if (!(cin >> response) || response == -1) {
        exit(0);
    }
    return response;
}

void solveKnown(int left, int right, int64 sum, MaxRange range) {
    if (range.high <= bestAnswer) return;

    if (range.low == range.high) {
        bestAnswer = max(bestAnswer, range.low);
        return;
    }

    int length = right - left + 1;
    int firstLeft, firstRight, secondLeft, secondRight;

    if (length % 2 == 0) {
        int firstLength = 2 * (length / 4);
        firstLeft = left;
        firstRight = left + firstLength - 1;
        secondLeft = firstRight + 1;
        secondRight = right;
    } else {
        int firstLength = length / 2;
        firstLeft = left;
        firstRight = left + firstLength - 1;
        secondLeft = firstRight + 1;
        secondRight = right;
    }

    int64 firstSum = ask(firstLeft, firstRight);
    int64 secondSum = sum - firstSum;

    MaxRange firstRange = getMaxRange(firstSum, firstRight - firstLeft + 1);
    MaxRange secondRange = getMaxRange(secondSum, secondRight - secondLeft + 1);

    if (firstRange.low == firstRange.high) {
        bestAnswer = max(bestAnswer, firstRange.low);
    }
    if (secondRange.low == secondRange.high) {
        bestAnswer = max(bestAnswer, secondRange.low);
    }

    if (firstRange.high >= secondRange.high) {
        solveKnown(firstLeft, firstRight, firstSum, firstRange);
        solveKnown(secondLeft, secondRight, secondSum, secondRange);
    } else {
        solveKnown(secondLeft, secondRight, secondSum, secondRange);
        solveKnown(firstLeft, firstRight, firstSum, firstRange);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) return 0;

    int64 total = ask(1, n);
    solveKnown(1, n, total, getMaxRange(total, n));

    cout << "! " << bestAnswer << endl;
    return 0;
}
