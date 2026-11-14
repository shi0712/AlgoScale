#include <bits/stdc++.h>
using namespace std;

class FastInput {
    static constexpr size_t SIZE = 1 << 20;
    char buffer[SIZE];
    size_t pos = 0, len = 0;

    inline char getChar() {
        if (pos == len) {
            len = fread(buffer, 1, SIZE, stdin);
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
        } while (c <= ' ');

        int x = 0;
        do {
            x = x * 10 + (c - '0');
            c = getChar();
        } while (c >= '0' && c <= '9');
        return x;
    }
};

struct Evaluation {
    long long cost;
    int halves;
};

int main() {
    FastInput in;

    const int n = in.readInt();
    const int P = in.readInt();
    const int Q = in.readInt();

    constexpr int LIMIT = 2'000'000;

    vector<int> count(LIMIT + 1, 0);
    int maximum = 0;
    int positive = 0;
    int totalHalfSteps = 0;
    long long initialSum = 0;

    for (int i = 0; i < n; ++i) {
        int h = in.readInt();
        ++count[h];
        maximum = max(maximum, h);
        initialSum += h;
        if (h != 0) {
            ++positive;
            totalHalfSteps += int(bit_width(static_cast<unsigned>(h)));
        }
    }

    if (maximum == 0 || Q >= totalHalfSteps) {
        puts("0");
        return 0;
    }

    count.resize(maximum + 1);

    vector<uint16_t> root(maximum + 1);
    int r = 0;
    for (int h = 0; h <= maximum; ++h) {
        while ((r + 1) * (r + 1) <= h) ++r;
        root[h] = static_cast<uint16_t>(r);
    }

    // Cases with only one available operation type.
    if (P == 0) {
        int remaining = Q;
        long long answer = initialSum;

        for (int h = maximum; h >= 1 && remaining > 0; --h) {
            int take = min(remaining, count[h]);
            if (take == 0) continue;

            int next = h / 2;
            count[h] -= take;
            count[next] += take;
            remaining -= take;
            answer -= 1LL * take * (h - next);
        }

        printf("%lld\n", answer);
        return 0;
    }

    if (Q == 0) {
        int remaining = P;
        long long answer = initialSum;

        for (int h = maximum; h >= 2 && remaining > 0; --h) {
            int take = min(remaining, count[h]);
            if (take == 0) continue;

            int next = root[h];
            count[h] -= take;
            count[next] += take;
            remaining -= take;
            answer -= 1LL * take * (h - next);
        }

        printf("%lld\n", answer);
        return 0;
    }

    /*
       Every useful square-root operation corresponds to one state h >= 2
       on a mountain's repeated-square-root chain.

       weight[h] counts how often state h occurs among all such chains.
    */
    vector<int> weight = count;
    int totalRootSteps = 0;

    for (int h = maximum; h >= 2; --h) {
        totalRootSteps += weight[h];
        weight[root[h]] += weight[h];
    }

    if (P >= totalRootSteps) {
        printf("%d\n", max(0, positive - Q));
        return 0;
    }

    vector<int> active;
    active.reserve(min(maximum, n + 2000));

    array<int, 32> zeroPriceSavings{};
    for (int h = 2; h <= maximum; ++h) {
        if (weight[h] == 0) continue;

        active.push_back(h);
        int saving =
            int(bit_width(static_cast<unsigned>(h))) -
            int(bit_width(static_cast<unsigned>(root[h])));

        zeroPriceSavings[saving] += weight[h];
    }

    // Minimum number of halvings needed to reach zero, using P roots.
    int minimumHalvesToZero = totalHalfSteps;
    int remainingRoots = P;

    for (int saving = 31; saving >= 0 && remainingRoots > 0; --saving) {
        int take = min(remainingRoots, zeroPriceSavings[saving]);
        minimumHalvesToZero -= take * saving;
        remainingRoots -= take;
    }

    if (Q >= minimumHalvesToZero) {
        puts("0");
        return 0;
    }

    vector<int> value(maximum + 1, 0);
    vector<uint8_t> steps(maximum + 1, 0);
    vector<int> gainCount(maximum + 1, 0);
    vector<int> gainHalfSavings(maximum + 1, 0);

    /*
       For an integer price price per halving, minimize

           final sum + price * number of halvings,

       while using at most P square roots.

       value[h] is the best penalized value of h using only halvings.
       steps[h] breaks ties in favor of fewer halvings.

       A square root at state h saves
           value[h] - value[root[h]].
       Along every square-root chain these savings are nonincreasing,
       so the P largest savings can be selected globally.

       Equal savings are resolved by saving as many halvings as possible.
    */
    auto evaluate = [&](int price) -> Evaluation {
        memset(gainCount.data(), 0, gainCount.size() * sizeof(int));
        memset(gainHalfSavings.data(), 0,
               gainHalfSavings.size() * sizeof(int));

        long long cost = 0;
        int halves = 0;
        int largestGain = 0;

        for (int h = 1; h <= maximum; ++h) {
            int candidate = price + value[h >> 1];

            if (candidate < h) {
                value[h] = candidate;
                steps[h] = static_cast<uint8_t>(steps[h >> 1] + 1);
            } else {
                value[h] = h;
                steps[h] = 0;
            }

            if (count[h] != 0) {
                cost += 1LL * count[h] * value[h];
                halves += count[h] * int(steps[h]);
            }

            if (h >= 2 && weight[h] != 0) {
                int next = root[h];
                int gain = value[h] - value[next];
                int saving = int(steps[h]) - int(steps[next]);

                gainCount[gain] += weight[h];
                gainHalfSavings[gain] += weight[h] * saving;
                largestGain = max(largestGain, gain);
            }
        }

        int need = P;

        for (int gain = largestGain; gain >= 0; --gain) {
            int available = gainCount[gain];
            if (available == 0) continue;

            if (available <= need) {
                cost -= 1LL * available * gain;
                halves -= gainHalfSavings[gain];
                need -= available;

                if (need == 0) {
                    return {cost, halves};
                }
            } else {
                cost -= 1LL * need * gain;

                array<int, 32> savingsCount{};
                for (int h : active) {
                    int next = root[h];
                    if (value[h] - value[next] == gain) {
                        int saving = int(steps[h]) - int(steps[next]);
                        savingsCount[saving] += weight[h];
                    }
                }

                for (int saving = 31; saving >= 0 && need > 0; --saving) {
                    int take = min(need, savingsCount[saving]);
                    halves -= take * saving;
                    need -= take;
                }

                return {cost, halves};
            }
        }

        return {cost, halves};
    };

    /*
       The optimal total as a function of the halving budget is discretely
       convex. Its supporting prices are integers. Find the first price
       whose penalized optimum can use at most Q halvings.
    */
    int low = 1;
    int high = (maximum + 1) / 2;

    while (low < high) {
        int mid = low + (high - low) / 2;
        Evaluation current = evaluate(mid);

        if (current.halves <= Q) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    Evaluation current = evaluate(low);
    long long answer = current.cost - 1LL * low * Q;

    if (low > 1) {
        Evaluation previous = evaluate(low - 1);
        answer = max(answer, previous.cost - 1LL * (low - 1) * Q);
    }

    printf("%lld\n", answer);
    return 0;
}
