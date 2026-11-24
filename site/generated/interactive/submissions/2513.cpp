#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;

using int64 = long long;

struct KnownScore {
    int64 value;
    int index;
};

int64 query(int i, int j, int k) {
    cout << "? " << i << ' ' << j << ' ' << k << '\n' << flush;
    int64 answer;
    if (!(cin >> answer) || answer < 0) {
        exit(0);
    }
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) {
        return 0;
    }

    vector<int64> scores(n + 1, -1);
    static int64 answers[5][5][5];
    fill(&answers[0][0][0], &answers[0][0][0] + 5 * 5 * 5, -1);

    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            for (int k = j + 1; k < 5; ++k) {
                answers[i][j][k] = query(i + 1, j + 1, k + 1);
            }
        }
    }

    auto get_answer = [&](int i, int j, int k) {
        array<int, 3> indices = {i, j, k};
        sort(indices.begin(), indices.end());
        return answers[indices[0]][indices[1]][indices[2]];
    };

    array<int64, 5> query_sums{};
    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            for (int k = j + 1; k < 5; ++k) {
                int64 value = answers[i][j][k];
                query_sums[i] += value;
                query_sums[j] += value;
                query_sums[k] += value;
            }
        }
    }

    array<int, 5> order = {0, 1, 2, 3, 4};
    sort(order.begin(), order.end(), [&](int i, int j) {
        if (query_sums[i] != query_sums[j]) {
            return query_sums[i] < query_sums[j];
        }
        return i < j;
    });

    int64 ac = get_answer(order[0], order[1], order[2]);
    int64 ae = get_answer(order[0], order[1], order[4]);
    int64 ce = get_answer(order[2], order[3], order[4]);
    array<int64, 5> sorted_scores;
    sorted_scores[0] = (ac + ae - ce) / 2;
    sorted_scores[2] = (ac + ce - ae) / 2;
    sorted_scores[4] = (ae + ce - ac) / 2;
    sorted_scores[3] = get_answer(order[0], order[1], order[3]) - sorted_scores[0];
    sorted_scores[1] = get_answer(order[1], order[2], order[4]) - sorted_scores[4];

    vector<KnownScore> known;
    known.reserve(n);
    for (int i = 0; i < 5; ++i) {
        scores[order[i] + 1] = sorted_scores[i];
        known.push_back({sorted_scores[i], order[i] + 1});
    }

    for (int next = 6; next <= n; ++next) {
        int best = 0;
        for (int i = 1; i + 1 < static_cast<int>(known.size()); ++i) {
            if (known[i + 1].value - known[i].value <
                known[best + 1].value - known[best].value) {
                best = i;
            }
        }

        int64 low = known[best].value;
        int64 high = known[best + 1].value;
        int64 result = query(known[best].index, known[best + 1].index, next);
        int64 value;

        if (low == high) {
            value = result - low;
        } else if (result < low + high) {
            value = result - high;
        } else if (result > low + high) {
            value = result - low;
        } else if (best + 2 < static_cast<int>(known.size())) {
            int64 upper = known[best + 2].value;
            result = query(known[best + 1].index, known[best + 2].index, next);
            value = result - upper;
        } else {
            int64 lower = known[best - 1].value;
            result = query(known[best - 1].index, known[best].index, next);
            value = result - lower;
        }

        scores[next] = value;
        KnownScore entry = {value, next};
        known.insert(lower_bound(known.begin(), known.end(), entry,
                                 [](const KnownScore &a, const KnownScore &b) {
                                     return a.value < b.value;
                                 }),
                     entry);
    }

    cout << '!';
    for (int i = 1; i <= n; ++i) {
        cout << ' ' << scores[i];
    }
    cout << '\n' << flush;
    return 0;
}
