#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

using int64 = long long;
constexpr int64 NEG = numeric_limits<int64>::min() / 4;
constexpr int B = 400;
constexpr int D = 2 * B + 1;
constexpr int SMALL_K = 26;

struct Card {
    int64 v;
    int t;
};

struct State {
    int64 value = NEG;
    int count = 0;
};

static inline bool better(int64 value, int count, int64 otherValue, int otherCount) {
    return value > otherValue || (value == otherValue && count > otherCount);
}

State penalized(const vector<Card>& cards, int64 penalty) {
    array<State, D> cur, nxt;
    cur[B] = {0, 0};
    for (const Card& card : cards) {
        nxt = cur;
        int t = card.t;
        for (int d = -B; d <= B; ++d) {
            State from = cur[d + B];
            if (from.value == NEG) continue;
            for (int sign : {-1, 1}) {
                int to = d + sign * t;
                if (to >= -B && to <= B) {
                    int64 value = from.value + card.v;
                    State& dest = nxt[to + B];
                    if (better(value, from.count, dest.value, dest.count))
                        dest = {value, from.count};
                }
                to = d + sign * 2 * t;
                if (to >= -B && to <= B) {
                    int64 value = from.value + card.v - penalty;
                    int count = from.count + 1;
                    State& dest = nxt[to + B];
                    if (better(value, count, dest.value, dest.count))
                        dest = {value, count};
                }
            }
        }
        cur = nxt;
    }
    return cur[B];
}

int64 exactSmall(const vector<Card>& cards, int k) {
    vector<array<int64, D>> cur(k + 1), nxt(k + 1);
    for (auto& row : cur) row.fill(NEG);
    cur[0][B] = 0;
    for (const Card& card : cards) {
        nxt = cur;
        int t = card.t;
        for (int used = 0; used <= k; ++used) {
            for (int d = -B; d <= B; ++d) {
                int64 from = cur[used][d + B];
                if (from == NEG) continue;
                for (int sign : {-1, 1}) {
                    int to = d + sign * t;
                    if (to >= -B && to <= B)
                        nxt[used][to + B] = max(nxt[used][to + B], from + card.v);
                    if (used < k) {
                        to = d + sign * 2 * t;
                        if (to >= -B && to <= B)
                            nxt[used + 1][to + B] = max(nxt[used + 1][to + B], from + card.v);
                    }
                }
            }
        }
        cur.swap(nxt);
    }
    int64 answer = 0;
    for (int used = 0; used <= k; ++used) answer = max(answer, cur[used][B]);
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;
    vector<Card> cards(n);
    int64 positiveSum = 0;
    for (Card& card : cards) {
        cin >> card.v >> card.t;
        if (card.v > 0) positiveSum += card.v;
    }
    sort(cards.begin(), cards.end(), [](const Card& a, const Card& b) {
        return a.t < b.t;
    });

    if (k < SMALL_K) {
        cout << exactSmall(cards, k) << '\n';
        return 0;
    }

    State initial = penalized(cards, 0);
    if (initial.count <= k) {
        cout << initial.value << '\n';
        return 0;
    }

    int64 low = 0, high = positiveSum + 1;
    while (low < high) {
        int64 mid = low + (high - low) / 2;
        if (penalized(cards, mid).count <= k)
            high = mid;
        else
            low = mid + 1;
    }

    State at = penalized(cards, low);
    int64 answer;
    if (at.count == k) {
        answer = at.value + low * k;
    } else {
        State before = penalized(cards, low - 1);
        answer = before.value + (low - 1) * k;
    }
    cout << answer << '\n';
}
