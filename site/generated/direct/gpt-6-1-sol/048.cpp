#include <bits/stdc++.h>
using namespace std;

class FastInput {
    static constexpr int SZ = 1 << 20;
    char buffer[SZ];
    int pos = 0, len = 0;

    char getChar() {
        if (pos == len) {
            len = (int)fread(buffer, 1, SZ, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buffer[pos++];
    }

public:
    int readInt() {
        char c;
        do {
            c = getChar();
        } while (c <= ' ' && c);

        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return x;
    }
};

struct FenwickMax {
    int n;
    vector<int> bit;

    explicit FenwickMax(int n) : n(n), bit(n + 1, 0) {}

    void update(int p, int value) {
        for (; p <= n; p += p & -p)
            bit[p] = max(bit[p], value);
    }

    int query(int p) const {
        int result = 0;
        for (; p > 0; p -= p & -p)
            result = max(result, bit[p]);
        return result;
    }
};

static constexpr int MAX_STACKS = 180;

bool attemptConstruction(
    const vector<int>& input,
    int n,
    bool reversed,
    int variation,
    mt19937_64& rng,
    vector<int>& answer
) {
    const int m = 2 * n;

    vector<int> first(n + 1, 0), second(n + 1, 0);
    vector<int> order;
    order.reserve(n);

    auto cardAt = [&](int position) -> int {
        return reversed ? input[m - position] : input[position - 1];
    };

    for (int i = 1; i <= m; ++i) {
        int x = cardAt(i);
        if (first[x] == 0) {
            first[x] = i;
            order.push_back(x);
        } else {
            second[x] = i;
        }
    }

    // Position p is stored at index m - p + 1.
    // Consequently, query(m - t) considers positions strictly after t.
    FenwickMax fenwick(m);

    auto suffix = [&](int t) -> int {
        return fenwick.query(m - t);
    };

    vector<int> block(n + 1, 0);
    vector<int> updates;
    updates.reserve(n);

    // Patience-sorting tails for the values -second[x].
    vector<int> tails;
    tails.reserve(MAX_STACKS + 16);

    int blockNumber = 1;
    int blockStart = 0;

    auto chooseBudget = [&]() -> int {
        if (variation == 0) return MAX_STACKS;

        // Candidates are checked afterwards, so slightly more permissive
        // budgets are safe to try as well.
        int radius = min(8, 1 + variation / 4);
        return MAX_STACKS
             + int(rng() % (2 * radius + 1))
             - radius;
    };

    int budget = chooseBudget();

    auto finishBlock = [&](int end) {
        updates.clear();

        // Compute all values using completed, earlier blocks only.
        for (int j = blockStart; j < end; ++j) {
            int x = order[j];
            updates.push_back(1 + suffix(second[x]));
        }

        // Only now make this entire block visible.
        for (int j = blockStart; j < end; ++j) {
            int x = order[j];
            fenwick.update(
                m - second[x] + 1,
                updates[j - blockStart]
            );
        }

        blockStart = end;
        ++blockNumber;
        tails.clear();
        budget = chooseBudget();
    };

    int i = 0;
    while (i < n) {
        int x = order[i];
        int value = -second[x];

        int position = int(
            lower_bound(tails.begin(), tails.end(), value)
            - tails.begin()
        );

        int decreasingEndingHere = position + 1;
        int upperBound =
            decreasingEndingHere + 1 + suffix(first[x]);

        if (upperBound <= budget) {
            block[x] = blockNumber;

            if (position == (int)tails.size())
                tails.push_back(value);
            else
                tails[position] = value;

            ++i;
            continue;
        }

        if (i != blockStart) {
            finishBlock(i);
            continue;
        }

        /*
         * A singleton block has an exact, better bound:
         *
         * first occurrence, then earlier-block seconds:
         *     1 + S(first[x])
         *
         * both occurrences, then earlier-block seconds:
         *     2 + S(second[x])
         *
         * Even when this exceeds the target, keep the singleton:
         * the final stack-assignment pass is the definitive check.
         */
        block[x] = blockNumber;
        ++i;
        finishBlock(i);
    }

    // Keys are distinct between ranks and equal for the two copies.
    vector<long long> key(n + 1);
    for (int x = 1; x <= n; ++x) {
        key[x] = 1LL * block[x] * (m + 1) + second[x];
    }

    // Sorted stack tops, with their actual stack labels.
    vector<long long> top;
    vector<int> label;
    top.reserve(MAX_STACKS);
    label.reserve(MAX_STACKS);

    answer.assign(m, 0);
    int stackCount = 0;

    for (int p = 1; p <= m; ++p) {
        long long value = key[cardAt(p)];

        int at = int(
            lower_bound(top.begin(), top.end(), value)
            - top.begin()
        );

        int chosen;
        if (at == 0) {
            if (stackCount == MAX_STACKS)
                return false;

            chosen = ++stackCount;
            top.insert(top.begin(), value);
            label.insert(label.begin(), chosen);
        } else {
            --at;
            chosen = label[at];
            top[at] = value;
        }

        int originalPosition = reversed ? m - p : p - 1;
        answer[originalPosition] = chosen;
    }

    /*
     * Reversing the input reverses every stack's rank constraints.
     * Acyclicity is preserved, so the same labels are valid for the
     * original deck after reversing the assignment positions.
     */
    return true;
}

int main() {
    FastInput in;

    int n = in.readInt();
    int m = 2 * n;

    vector<int> a(m);
    for (int& x : a) x = in.readInt();

    mt19937_64 rng(
        chrono::steady_clock::now().time_since_epoch().count()
    );

    vector<int> answer;

    for (int attempt = 0; ; ++attempt) {
        bool reversed = attempt & 1;
        int variation = attempt / 2;

        if (attemptConstruction(
                a, n, reversed, variation, rng, answer)) {
            break;
        }
    }

    string output;
    output.reserve(4ULL * m);

    for (int i = 0; i < m; ++i) {
        if (i) output.push_back(' ');
        output += to_string(answer[i]);
    }
    output.push_back('\n');

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}
