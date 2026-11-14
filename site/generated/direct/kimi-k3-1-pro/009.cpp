#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = 1000000000LL;

struct Solver {
    struct Result {
        // 0 = feasible, 1 = lower failure, 2 = upper failure
        int type;
        int jump;
    };

    int n;
    int64 W;
    vector<int64> w, r;

    // Items with fixed weights, sorted as Alice sorts them.
    vector<int> fixedByWeight;
    vector<int> position;       // 1-based position, 0 for missing-weight items
    vector<int64> prefixWeight;

    // Items with fixed values, sorted as Bob sorts them.
    vector<int> fixedByValue;

    // Partition of fixed-value items, needed to merge in missing-value items.
    vector<int> topFixed, middleFixed, bottomFixed;
    vector<int> flexibleByIndex;
    vector<int> topAll, bottomAll;

    vector<int> variableIndices;
    int variableCount = 0;
    int minimumVariableIndex = INT_MAX;

    int fixedCount = 0;
    int maximumCut = 0;

    Solver(int n_, int64 W_, vector<int64> w_, vector<int64> r_)
        : n(n_), W(W_), w(std::move(w_)), r(std::move(r_)) {
        initialize();
    }

    static void mergeByIndex(const vector<int>& a, const vector<int>& b,
                             vector<int>& out) {
        out.clear();
        out.reserve(a.size() + b.size());
        int i = 0, j = 0;
        while (i < (int)a.size() || j < (int)b.size()) {
            if (j == (int)b.size() ||
                (i < (int)a.size() && a[i] < b[j])) {
                out.push_back(a[i++]);
            } else {
                out.push_back(b[j++]);
            }
        }
    }

    void initialize() {
        position.assign(n, 0);

        for (int i = 0; i < n; ++i) {
            if (w[i] != 0) {
                fixedByWeight.push_back(i);
            } else {
                ++variableCount;
                variableIndices.push_back(i);
                minimumVariableIndex = min(minimumVariableIndex, i);
            }
            if (r[i] == 0) {
                flexibleByIndex.push_back(i);
            } else {
                fixedByValue.push_back(i);
            }
        }

        sort(fixedByWeight.begin(), fixedByWeight.end(),
             [&](int a, int b) {
                 if (w[a] != w[b]) return w[a] < w[b];
                 return a < b;
             });

        fixedCount = (int)fixedByWeight.size();
        prefixWeight.assign(fixedCount + 1, 0);
        for (int k = 1; k <= fixedCount; ++k) {
            int i = fixedByWeight[k - 1];
            position[i] = k;
            prefixWeight[k] = prefixWeight[k - 1] + w[i];
        }

        maximumCut = int(upper_bound(prefixWeight.begin(),
                                     prefixWeight.end(), W)
                         - prefixWeight.begin()) - 1;

        sort(fixedByValue.begin(), fixedByValue.end(),
             [&](int a, int b) {
                 if (r[a] != r[b]) return r[a] > r[b];
                 return a < b;
             });

        for (int i : fixedByValue) {
            if (r[i] == INF) {
                topFixed.push_back(i);
            } else if (r[i] == 1) {
                bottomFixed.push_back(i);
            } else {
                middleFixed.push_back(i);
            }
        }

        sort(flexibleByIndex.begin(), flexibleByIndex.end());
        sort(variableIndices.begin(), variableIndices.end());

        mergeByIndex(topFixed, flexibleByIndex, topAll);
        mergeByIndex(bottomFixed, flexibleByIndex, bottomAll);
    }

    int64 firstUnselectedWeight(int cut) const {
        if (cut == fixedCount) return -1;
        return w[fixedByWeight[cut]];
    }

    int firstUnselectedIndex(int cut) const {
        if (cut == fixedCount) return -1;
        return fixedByWeight[cut];
    }

    int64 variableCapacity(int i, int cut) const {
        if (cut == fixedCount) return INF - 1;

        int64 c = firstUnselectedWeight(cut);
        if (c == INF) return INF - 1;

        int p = firstUnselectedIndex(cut);
        return c - 1 + (i < p ? 1 : 0);
    }

    Result check(int cut) const {
        const int64 fixedSum = prefixWeight[cut];
        const int64 budget = W - fixedSum;

        int64 totalCapacity = 0;
        if (cut == fixedCount) {
            totalCapacity = (int64)variableCount * (INF - 1);
        } else {
            int64 c = firstUnselectedWeight(cut);
            if (c == INF) {
                totalCapacity = (int64)variableCount * (INF - 1);
            } else {
                int p = firstUnselectedIndex(cut);
                int eligible = int(lower_bound(variableIndices.begin(),
                                               variableIndices.end(), p)
                                   - variableIndices.begin());
                totalCapacity = (int64)variableCount * (c - 1) + eligible;
            }
        }

        int64 selectedSum =
            fixedSum + min(budget, totalCapacity);

        // Alice must stop at the first fixed-weight item after the cutoff.
        if (cut < fixedCount &&
            selectedSum + firstUnselectedWeight(cut) <= W) {
            return {1, 0};
        }

        // The only possible tie problem at weight INF occurs when a fixed
        // item of weight INF is selected. It then consumes the whole capacity.
        if (cut > 0 && variableCount > 0) {
            int lastSelected = fixedByWeight[cut - 1];
            if (w[lastSelected] == INF &&
                minimumVariableIndex < lastSelected) {
                return {2, maximumCut + 1};
            }
        }

        int64 fixedBefore = 0;
        int64 capacityBefore = 0;

        auto process = [&](int i, Result& answer) -> bool {
            if (w[i] == 0) {
                int64 cap = variableCapacity(i, cut);
                int64 alreadyUsed = min(budget, capacityBefore);
                int64 remaining = budget - alreadyUsed;
                int64 assigned = min(cap, remaining);

                if (assigned == 0) {
                    int64 processed =
                        fixedBefore + min(budget, capacityBefore);

                    if (processed + INF <= W) {
                        if (fixedBefore + budget + INF <= W) {
                            answer = {2, maximumCut + 1};
                        } else {
                            answer = {1, 0};
                        }
                        return false;
                    }
                }

                capacityBefore += cap;
                return true;
            }

            if (position[i] <= cut) {
                fixedBefore += w[i];
                return true;
            }

            int64 processed =
                fixedBefore + min(budget, capacityBefore);

            if (processed + w[i] <= W) {
                if (fixedBefore + budget + w[i] <= W) {
                    answer = {2, position[i]};
                } else {
                    answer = {1, 0};
                }
                return false;
            }
            return true;
        };

        Result answer{0, 0};
        bool stopped = false;

        // Value INF: fixed items and selected missing-value items,
        // merged by original index.
        for (int i : topAll) {
            if (r[i] == 0 && position[i] > cut) continue;
            if (!process(i, answer)) {
                stopped = true;
                break;
            }
        }

        if (!stopped) {
            for (int i : middleFixed) {
                if (!process(i, answer)) {
                    stopped = true;
                    break;
                }
            }
        }

        // Value 1: fixed items and unselected missing-value items.
        if (!stopped) {
            for (int i : bottomAll) {
                if (r[i] == 0 && position[i] <= cut) continue;
                if (!process(i, answer)) {
                    stopped = true;
                    break;
                }
            }
        }

        if (stopped) return answer;
        return {0, 0};
    }

    void printCompletion(int cut, const vector<int64>& originalW,
                         const vector<int64>& originalR) const {
        vector<int64> answerW = originalW;
        vector<int64> answerR = originalR;

        int64 budget = W - prefixWeight[cut];

        // Variables occur in fixedByValue in exactly their canonical Bob order.
        for (int i : fixedByValue) {
            if (originalW[i] != 0) continue;

            int64 cap = variableCapacity(i, cut);
            int64 assigned = min(cap, budget);

            if (assigned > 0) {
                answerW[i] = assigned;
                budget -= assigned;
            } else {
                answerW[i] = INF;
            }
        }

        for (int i = 0; i < n; ++i) {
            if (originalR[i] == 0) {
                answerR[i] = (position[i] <= cut ? INF : 1);
            }
        }

        cout << "Yes\n";
        for (int i = 0; i < n; ++i) {
            if (i) cout << ' ';
            cout << answerW[i];
        }
        cout << '\n';
        for (int i = 0; i < n; ++i) {
            if (i) cout << ' ';
            cout << answerR[i];
        }
        cout << '\n';
    }

    bool solve(const vector<int64>& originalW,
               const vector<int64>& originalR) const {
        int current = 0;

        while (current <= maximumCut) {
            Result first = check(current);

            if (first.type == 0) {
                printCompletion(current, originalW, originalR);
                return true;
            }

            if (first.type == 2) {
                current = max(current + 1, first.jump);
                continue;
            }

            // A lower failure at k rules out every cut <= k.
            if (current == maximumCut) break;

            int low = current;
            int high = maximumCut + 1;
            int jump = maximumCut + 1;
            bool haveUpper = false;

            while (high - low > 1) {
                int mid = low + (high - low) / 2;
                Result result = check(mid);

                if (result.type == 0) {
                    printCompletion(mid, originalW, originalR);
                    return true;
                }

                if (result.type == 1) {
                    low = mid;
                } else {
                    high = mid;
                    jump = result.jump;
                    haveUpper = true;
                }
            }

            if (!haveUpper) break;
            current = jump;
        }

        return false;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n;
        int64 W;
        cin >> n >> W;

        vector<int64> w(n), r(n);
        for (auto& x : w) cin >> x;
        for (auto& x : r) cin >> x;

        // If assigning weight 1 to every missing weight makes everything fit,
        // both algorithms necessarily select every item.
        int64 minimumSum = 0;
        int missingWeights = 0;
        for (int i = 0; i < n; ++i) {
            if (w[i] == 0) {
                ++missingWeights;
                ++minimumSum;
            } else {
                minimumSum += w[i];
            }
        }

        if (minimumSum <= W) {
            cout << "Yes\n";
            for (int i = 0; i < n; ++i) {
                if (i) cout << ' ';
                cout << (w[i] == 0 ? 1 : w[i]);
            }
            cout << '\n';
            for (int i = 0; i < n; ++i) {
                if (i) cout << ' ';
                cout << (r[i] == 0 ? 1 : r[i]);
            }
            cout << '\n';
            continue;
        }

        // With the upper bound equal to 1, every entry is forced to be 1.
        // Since W = 1, both algorithms select exactly item 0.
        if (INF == 1) {
            cout << "Yes\n";
            for (int i = 0; i < n; ++i) {
                if (i) cout << ' ';
                cout << 1;
            }
            cout << '\n';
            for (int i = 0; i < n; ++i) {
                if (i) cout << ' ';
                cout << 1;
            }
            cout << '\n';
            continue;
        }

        Solver solver(n, W, w, r);
        if (!solver.solve(w, r)) {
            cout << "No\n";
        }
    }

    return 0;
}
