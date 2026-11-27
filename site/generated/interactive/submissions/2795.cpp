#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using ld = long double;

struct Query {
    int a, b, c;
    int64 answer;
};

struct Analysis {
    vector<array<int64, 5>> candidates;
    bool feasibleRankDeficient = false;
};

bool gaussJordan(vector<vector<ld>> a, vector<ld>& solution, int& rank) {
    const int n = 5;
    const ld EPS = 1e-12L;

    rank = 0;
    for (int col = 0; col < n; ++col) {
        int pivot = rank;
        for (int row = rank; row < (int)a.size(); ++row) {
            if (fabsl(a[row][col]) > fabsl(a[pivot][col])) {
                pivot = row;
            }
        }

        if (fabsl(a[pivot][col]) <= EPS) continue;

        swap(a[pivot], a[rank]);

        ld div = a[rank][col];
        for (int j = col; j <= n; ++j) {
            a[rank][j] /= div;
        }

        for (int row = 0; row < (int)a.size(); ++row) {
            if (row == rank) continue;
            ld factor = a[row][col];
            if (fabsl(factor) <= EPS) continue;
            for (int j = col; j <= n; ++j) {
                a[row][j] -= factor * a[rank][j];
            }
        }

        ++rank;
    }

    if (rank != n) return false;

    solution.assign(n, 0);
    for (int i = 0; i < n; ++i) {
        solution[i] = a[i][n];
    }
    return true;
}

int64 operation(int64 x, int64 y, int64 z) {
    return min({x, y, z}) + max({x, y, z});
}

bool validCandidate(const array<int64, 5>& value,
                    const array<int, 5>& order,
                    const vector<Query>& queries) {
    for (int i = 0; i < 5; ++i) {
        if (value[i] < 0 || value[i] > 1000000000LL) return false;
    }

    for (int i = 0; i + 1 < 5; ++i) {
        if (value[order[i]] > value[order[i + 1]]) return false;
    }

    for (const Query& q : queries) {
        if (operation(value[q.a], value[q.b], value[q.c]) != q.answer) {
            return false;
        }
    }
    return true;
}

bool feasibleRankDeficient(const vector<vector<ld>>& equations,
                           const array<int, 5>& order) {
    int rank;
    vector<ld> unused;
    gaussJordan(equations, unused, rank);

    int dimension = 5 - rank;
    if (dimension <= 0 || dimension > 6) return false;

    vector<vector<ld>> inequalities;
    for (int i = 0; i + 1 < 5; ++i) {
        vector<ld> row(6, 0);
        row[order[i]] += 1;
        row[order[i + 1]] -= 1;
        inequalities.push_back(row);
    }

    {
        vector<ld> row(6, 0);
        row[order[0]] = -1;
        inequalities.push_back(row);
    }
    {
        vector<ld> row(6, 0);
        row[order[4]] = 1;
        row[5] = 1000000000LL;
        inequalities.push_back(row);
    }

    for (int mask = 0; mask < (1 << 6); ++mask) {
        if (__builtin_popcount((unsigned)mask) != dimension) continue;

        vector<vector<ld>> combined = equations;
        for (int i = 0; i < 6; ++i) {
            if (mask & (1 << i)) {
                combined.push_back(inequalities[i]);
            }
        }

        vector<ld> x;
        int combinedRank;
        if (!gaussJordan(combined, x, combinedRank)) continue;

        bool ok = true;
        const ld TOL = 1e-2L;

        for (const auto& row : equations) {
            ld value = row[5];
            for (int j = 0; j < 5; ++j) {
                value -= row[j] * x[j];
            }
            if (fabsl(value) > TOL) {
                ok = false;
                break;
            }
        }

        if (ok) {
            for (const auto& row : inequalities) {
                ld value = row[5];
                for (int j = 0; j < 5; ++j) {
                    value -= row[j] * x[j];
                }
                if (value > TOL) {
                    ok = false;
                    break;
                }
            }
        }

        if (ok) return true;
    }

    return false;
}

Analysis analyzeQueries(const vector<Query>& queries) {
    Analysis result;
    array<int, 5> order = {0, 1, 2, 3, 4};

    do {
        array<int, 5> position;
        for (int i = 0; i < 5; ++i) {
            position[order[i]] = i;
        }

        vector<vector<ld>> equations;
        equations.reserve(queries.size());

        for (const Query& q : queries) {
            int minimumLabel = q.a;
            int maximumLabel = q.a;

            if (position[q.b] < position[minimumLabel]) minimumLabel = q.b;
            if (position[q.c] < position[minimumLabel]) minimumLabel = q.c;
            if (position[q.b] > position[maximumLabel]) maximumLabel = q.b;
            if (position[q.c] > position[maximumLabel]) maximumLabel = q.c;

            vector<ld> row(6, 0);
            row[minimumLabel] += 1;
            row[maximumLabel] += 1;
            row[5] = q.answer;
            equations.push_back(row);
        }

        vector<ld> solution;
        int rank;
        if (gaussJordan(equations, solution, rank)) {
            array<int64, 5> value{};
            bool ok = true;

            for (int i = 0; i < 5; ++i) {
                ld rounded = roundl(solution[i]);
                if (fabsl(solution[i] - rounded) > 1e-2L) {
                    ok = false;
                    break;
                }
                value[i] = (int64)rounded;
            }

            if (ok && validCandidate(value, order, queries)) {
                bool alreadyPresent = false;
                for (const auto& candidate : result.candidates) {
                    if (candidate == value) {
                        alreadyPresent = true;
                        break;
                    }
                }
                if (!alreadyPresent) result.candidates.push_back(value);
            }
        } else if (feasibleRankDeficient(equations, order)) {
            result.feasibleRankDeficient = true;
        }
    } while (next_permutation(order.begin(), order.end()));

    return result;
}

array<int64, 5> solveComplete(const vector<Query>& queries) {
    Analysis analysis = analyzeQueries(queries);
    if (analysis.candidates.empty()) exit(0);
    return analysis.candidates.front();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<Query> queries;

    auto ask = [&](int a, int b, int c) -> int64 {
        cout << "? " << a + 1 << ' ' << b + 1 << ' ' << c + 1 << endl;
        int64 answer;
        if (!(cin >> answer) || answer == -1) exit(0);
        queries.push_back({a, b, c, answer});
        return answer;
    };

    array<int64, 4> firstAnswers{};
    for (int omitted = 0; omitted < 4; ++omitted) {
        vector<int> triple;
        for (int i = 0; i < 4; ++i) {
            if (i != omitted) triple.push_back(i);
        }
        firstAnswers[omitted] = ask(triple[0], triple[1], triple[2]);
    }

    int64 maximum = *max_element(firstAnswers.begin(), firstAnswers.end());
    int64 minimum = *min_element(firstAnswers.begin(), firstAnswers.end());

    vector<int> maximumLabels, minimumLabels;
    for (int i = 0; i < 4; ++i) {
        if (firstAnswers[i] == maximum) maximumLabels.push_back(i);
        if (firstAnswers[i] == minimum) minimumLabels.push_back(i);
    }

    array<int64, 5> firstValues{};

    if (maximumLabels.size() != 1 || minimumLabels.size() != 1 ||
        maximumLabels[0] == minimumLabels[0]) {
        for (int i = 0; i < 4; ++i) {
            for (int j = i + 1; j < 4; ++j) {
                ask(i, j, 4);
            }
        }
        firstValues = solveComplete(queries);
    } else {
        int A = maximumLabels[0];
        int D = minimumLabels[0];
        int B = -1, C = -1;
        for (int i = 0; i < 4; ++i) {
            if (i != A && i != D) {
                if (B == -1) B = i;
                else C = i;
            }
        }

        int64 E = firstAnswers[B];
        int64 middleSum = firstAnswers[A] + firstAnswers[D] - E;

        ask(A, D, 4);
        int64 middleAnswer = ask(B, C, 4);

        bool low;
        if (middleAnswer < middleSum) {
            low = true;
            ask(A, B, 4);
            ask(A, C, 4);
        } else if (middleAnswer > middleSum) {
            low = false;
            ask(B, D, 4);
            ask(C, D, 4);
        } else {
            ask(A, B, 4);
            ask(A, C, 4);
            ask(B, D, 4);
            ask(C, D, 4);
            firstValues = solveComplete(queries);
            low = false;
            middleAnswer = middleSum;
        }

        if (middleAnswer != middleSum) {
            Analysis analysis = analyzeQueries(queries);

            bool unique = !analysis.candidates.empty() &&
                          !analysis.feasibleRankDeficient;
            if (unique) {
                for (const auto& candidate : analysis.candidates) {
                    if (candidate != analysis.candidates.front()) {
                        unique = false;
                        break;
                    }
                }
            }

            if (unique) {
                firstValues = analysis.candidates.front();
            } else {
                if (low) {
                    ask(B, D, 4);
                    ask(C, D, 4);
                } else {
                    ask(A, B, 4);
                    ask(A, C, 4);
                }
                firstValues = solveComplete(queries);
            }
        }
    }

    vector<int64> answer(n, 0);
    for (int i = 0; i < 5; ++i) {
        answer[i] = firstValues[i];
    }

    vector<int> knownLabels;
    vector<int64> knownValues;
    for (int i = 0; i < 5; ++i) {
        knownLabels.push_back(i);
        knownValues.push_back(firstValues[i]);
    }

    for (int label = 5; label < n; ++label) {
        int bestI = 0, bestJ = 1;
        int64 bestGap = LLONG_MAX;

        for (int i = 0; i < (int)knownValues.size(); ++i) {
            for (int j = i + 1; j < (int)knownValues.size(); ++j) {
                int64 gap = llabs(knownValues[i] - knownValues[j]);
                if (gap < bestGap) {
                    bestGap = gap;
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        int64 p = min(knownValues[bestI], knownValues[bestJ]);
        int64 q = max(knownValues[bestI], knownValues[bestJ]);
        int pLabel = knownValues[bestI] <= knownValues[bestJ]
                         ? knownLabels[bestI]
                         : knownLabels[bestJ];
        int qLabel = knownValues[bestI] <= knownValues[bestJ]
                         ? knownLabels[bestJ]
                         : knownLabels[bestI];

        int64 firstAnswer = ask(pLabel, qLabel, label);
        int64 x;

        if (firstAnswer != p + q) {
            x = firstAnswer < p + q ? firstAnswer - q : firstAnswer - p;
        } else if (p == q) {
            x = p;
        } else {
            int duplicateValue = -1;
            int duplicateFirst = -1, duplicateSecond = -1;

            for (int i = 0; i < (int)knownValues.size() && duplicateValue == -1; ++i) {
                for (int j = i + 1; j < (int)knownValues.size(); ++j) {
                    if (knownValues[i] == knownValues[j]) {
                        duplicateValue = knownValues[i];
                        duplicateFirst = knownLabels[i];
                        duplicateSecond = knownLabels[j];
                        break;
                    }
                }
            }

            if (duplicateValue != -1) {
                int64 secondAnswer = ask(duplicateFirst, duplicateSecond, label);
                x = secondAnswer - duplicateValue;
            } else {
                int lowerIndex = -1, upperIndex = -1;
                for (int i = 0; i < (int)knownValues.size(); ++i) {
                    if (knownValues[i] < p) lowerIndex = i;
                    if (knownValues[i] > q) upperIndex = i;
                }

                if (lowerIndex != -1) {
                    int64 r = knownValues[lowerIndex];
                    int64 secondAnswer = ask(knownLabels[lowerIndex], pLabel, label);
                    x = secondAnswer == r + p ? p : secondAnswer - r;
                } else {
                    int64 s = knownValues[upperIndex];
                    int64 secondAnswer = ask(qLabel, knownLabels[upperIndex], label);
                    x = secondAnswer == q + s ? q : secondAnswer - s;
                }
            }
        }

        if (x < 0 || x > 1000000000LL ||
            operation(p, q, x) != firstAnswer) {
            exit(0);
        }

        answer[label] = x;
        knownLabels.push_back(label);
        knownValues.push_back(x);
    }

    cout << '!';
    for (int i = 0; i < n; ++i) {
        cout << ' ' << answer[i];
    }
    cout << endl;

    return 0;
}
