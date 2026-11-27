#include <bits/stdc++.h>
using namespace std;

struct Candidate {
    int id;
    int last = -1;
    int prev = -1;
};

class Solver {
    int n;
    vector<Candidate> candidates;

    struct Score {
        int worst = INT_MAX;
        int minFixed = INT_MIN;
        int maxFixed = INT_MIN;
        int imbalance = INT_MAX;
        int length = INT_MAX;
        int l = 0, r = 0;
    };

    int ask(int l, int r) {
        cout << "? " << l << ' ' << r << endl;

        long long hands;
        if (!(cin >> hands) || hands == -1) exit(0);

        int len = r - l + 1;
        int low;

        if (hands == len) {
            low = 0;
        } else if (hands == len - 1) {
            low = 1;
        } else {
            exit(0);
        }

        vector<Candidate> next;
        next.reserve(candidates.size());

        for (const Candidate &c : candidates) {
            bool inside = (l <= c.id && c.id <= r);
            int dishonest = low ^ static_cast<int>(inside);

            if (c.prev == c.last && c.last == dishonest) continue;

            Candidate updated = c;
            updated.prev = c.last;
            updated.last = dishonest;
            next.push_back(updated);
        }

        candidates.swap(next);
        return low;
    }

    bool better(const Score &a, const Score &b) const {
        if (a.worst != b.worst) return a.worst < b.worst;
        if (a.minFixed != b.minFixed) return a.minFixed > b.minFixed;
        if (a.maxFixed != b.maxFixed) return a.maxFixed > b.maxFixed;
        if (a.imbalance != b.imbalance) return a.imbalance < b.imbalance;
        return a.length < b.length;
    }

    pair<int, int> chooseInterval() const {
        int m = static_cast<int>(candidates.size());

        vector<vector<int>> inSurv(2, vector<int>(m + 1, 0));
        vector<vector<int>> outSurv(2, vector<int>(m + 1, 0));
        vector<vector<vector<int>>> inFixed(
            2, vector<vector<int>>(2, vector<int>(m + 1, 0))
        );
        vector<vector<vector<int>>> outFixed(
            2, vector<vector<int>>(2, vector<int>(m + 1, 0))
        );

        vector<int> prefix(m + 1, 0);
        int same0 = 0, same1 = 0, freeCount = 0;

        for (int i = 0; i < m; ++i) {
            const Candidate &c = candidates[i];
            bool same = (c.last != -1 && c.prev == c.last);
            bool fixed = (c.last != -1);

            if (same) {
                if (c.last == 0) ++same0;
                else ++same1;
            } else {
                ++freeCount;
            }

            prefix[i + 1] = prefix[i];
            if (same && c.last == 0) ++prefix[i + 1];
            if (same && c.last == 1) --prefix[i + 1];

            for (int response = 0; response < 2; ++response) {
                bool surviveInside = true;
                bool surviveOutside = true;

                if (same) {
                    if (c.last == 0) {
                        surviveInside = (response == 0);
                        surviveOutside = (response == 1);
                    } else {
                        surviveInside = (response == 1);
                        surviveOutside = (response == 0);
                    }
                }

                inSurv[response][i + 1] =
                    inSurv[response][i] + surviveInside;
                outSurv[response][i + 1] =
                    outSurv[response][i] + surviveOutside;

                for (int lastValue = 0; lastValue < 2; ++lastValue) {
                    bool fixedInside = false;
                    bool fixedOutside = false;

                    if (fixed) {
                        fixedInside =
                            surviveInside && c.last == (response ^ 1);
                        fixedOutside =
                            surviveOutside && c.last == response;
                    }

                    inFixed[response][lastValue][i + 1] =
                        inFixed[response][lastValue][i] + fixedInside;
                    outFixed[response][lastValue][i + 1] =
                        outFixed[response][lastValue][i] + fixedOutside;
                }
            }
        }

        auto evaluate = [&](int l, int r) {
            Score result;
            result.l = l;
            result.r = r;
            result.length = r - l + 1;

            int survivors[2];
            int fixedCount[2];

            for (int response = 0; response < 2; ++response) {
                int insideSurvivors =
                    inSurv[response][r + 1] - inSurv[response][l];

                int outsideSurvivors =
                    outSurv[response][l] +
                    (outSurv[response][m] - outSurv[response][r + 1]);

                survivors[response] = insideSurvivors + outsideSurvivors;

                int insideFixed =
                    inFixed[response][response ^ 1][r + 1] -
                    inFixed[response][response ^ 1][l];

                int outsideFixed =
                    outFixed[response][response][l] +
                    (outFixed[response][response][m] -
                     outFixed[response][response][r + 1]);

                fixedCount[response] = insideFixed + outsideFixed;
            }

            result.worst = max(survivors[0], survivors[1]);
            result.minFixed = min(fixedCount[0], fixedCount[1]);
            result.maxFixed = max(fixedCount[0], fixedCount[1]);
            result.imbalance = abs(survivors[0] - survivors[1]);
            return result;
        };

        Score best = evaluate(0, m - 1);

        auto consider = [&](int l, int r) {
            Score current = evaluate(l, r);
            if (better(current, best)) best = current;
        };

        int target2 = same1 - same0;
        set<pair<int, int>> prefixes;

        for (int l = m - 1; l >= 0; --l) {
            prefixes.insert({2 * prefix[l + 1], l + 1});

            int wanted = 2 * prefix[l] + target2;
            auto it = prefixes.lower_bound({wanted, -1});

            vector<pair<int, int>> nearby;
            auto right = it;
            for (int cnt = 0; cnt < 4 && right != prefixes.end(); ++cnt, ++right) {
                nearby.push_back(*right);
            }
            auto left = it;
            for (int cnt = 0; cnt < 4 && left != prefixes.begin(); ++cnt) {
                --left;
                nearby.push_back(*left);
            }

            for (auto [value, index] : nearby) {
                int r = index - 1;
                if (r >= l) consider(l, r);
            }
        }

        return {candidates[best.l].id, candidates[best.r].id};
    }

    void fallbackSweep(bool reverseOrder) {
        ask(1, n);

        vector<int> ids;
        ids.reserve(candidates.size());
        for (const Candidate &c : candidates) ids.push_back(c.id);
        if (reverseOrder) reverse(ids.begin(), ids.end());

        for (int id : ids) {
            if (candidates.size() <= 2) return;
            ask(id, id);
        }
    }

public:
    explicit Solver(int n) : n(n) {
        candidates.reserve(n);
        for (int id = 1; id <= n; ++id) {
            candidates.push_back({id, -1, -1});
        }
    }

    void solve() {
        int stagnant = 0;

        while (candidates.size() > 2) {
            int oldSize = static_cast<int>(candidates.size());
            auto [l, r] = chooseInterval();
            ask(l, r);

            int newSize = static_cast<int>(candidates.size());
            if (newSize < oldSize) {
                stagnant = 0;
            } else {
                ++stagnant;
                if (stagnant >= 24) {
                    fallbackSweep((stagnant / 24) & 1);
                    stagnant = 0;
                }
            }
        }

        if (candidates.size() == 1) {
            cout << "! " << candidates[0].id << endl;
            int answer;
            if (!(cin >> answer) || answer == -1) exit(0);
        } else if (candidates.size() == 2) {
            cout << "! " << candidates[0].id << endl;
            int answer;
            if (!(cin >> answer) || answer == -1) exit(0);

            if (answer == 0) {
                cout << "! " << candidates[1].id << endl;
                if (!(cin >> answer) || answer == -1) exit(0);
            }
        }

        cout << "#" << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) return 0;

    Solver solver(n);
    solver.solve();
    return 0;
}
