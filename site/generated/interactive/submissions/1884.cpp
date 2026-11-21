#include <bits/stdc++.h>
using namespace std;

class Blackboard {
    vector<vector<unsigned char>> cache;

    static int response() {
        int x;
        if (!(cin >> x) || x == -1) exit(0);
        return x;
    }

public:
    explicit Blackboard(int n) : cache(2 * n, vector<unsigned char>(2 * n)) {}

    bool less_than(int a, int b) {
        if (a == b) return false;
        auto &c = cache[a][b];
        if (c != 0) return c == 2;
        cout << "? " << a << ' ' << b << endl;
        bool answer = response() == 1;
        c = answer ? 2 : 1;
        if (answer) cache[b][a] = 1;
        return answer;
    }

    int add(int a, int b) {
        cout << "+ " << a << ' ' << b << endl;
        return response();
    }

    vector<int> merge_sorted(const vector<int>& a, const vector<int>& b) {
        vector<int> result;
        result.reserve(a.size() + b.size());
        int i = 0, j = 0, streak = 0;
        const int na = int(a.size()), nb = int(b.size());

        // Find the end of a winning streak without comparing every element.
        auto gallop = [](int start, int end, auto wins) {
            int last = start - 1, step = 1;
            while (start + step - 1 < end && wins(start + step - 1)) {
                last = start + step - 1;
                step *= 2;
            }
            int lo = last + 1, hi = min(end, start + step - 1);
            while (lo < hi) {
                int mid = (lo + hi) / 2;
                if (wins(mid)) lo = mid + 1;
                else hi = mid;
            }
            return lo;
        };

        while (i < na && j < nb) {
            if (less_than(b[j], a[i])) {
                result.push_back(b[j++]);
                streak = min(streak, 0) - 1;
                if (streak <= -5 && j < nb) {
                    int end = gallop(j, nb, [&](int k) {
                        return less_than(b[k], a[i]);
                    });
                    result.insert(result.end(), b.begin() + j, b.begin() + end);
                    j = end;
                    streak = 0;
                }
            } else {
                result.push_back(a[i++]);
                streak = max(streak, 0) + 1;
                if (streak >= 5 && i < na) {
                    int end = gallop(i, na, [&](int k) {
                        return !less_than(b[j], a[k]);
                    });
                    result.insert(result.end(), a.begin() + i, a.begin() + end);
                    i = end;
                    streak = 0;
                }
            }
        }
        result.insert(result.end(), a.begin() + i, a.end());
        result.insert(result.end(), b.begin() + j, b.end());
        return result;
    }

    vector<int> sort_ids(vector<int> ids, bool* possibly_all_equal = nullptr) {
        const int n = int(ids.size());
        if (n < 2) return ids;

        vector<int> large;
        large.reserve(n / 2);
        vector<int> partner(cache.size());
        int descending_pairs = 0;
        for (int i = 0; i + 1 < n; i += 2) {
            int a = ids[i], b = ids[i + 1];
            if (less_than(b, a)) {
                swap(a, b);
                ++descending_pairs;
            }
            large.push_back(b);
            partner[b] = a;
        }

        // The pair comparisons already provide half of a monotone-run test.
        if (n >= 12 && (descending_pairs == 0 || descending_pairs == n / 2)) {
            bool descending = descending_pairs != 0;
            int p = 2;
            while (p < n) {
                bool inverted = descending ? less_than(ids[p - 1], ids[p])
                                           : less_than(ids[p], ids[p - 1]);
                if (inverted) break;
                p = min(n, p + 2);
            }
            if (p == n) {
                if (descending) reverse(ids.begin(), ids.end());
                if (possibly_all_equal) *possibly_all_equal = !descending;
                return ids;
            }
            if (p >= max(12, n / 8)) {
                vector<int> first(ids.begin(), ids.begin() + p);
                if (descending) reverse(first.begin(), first.end());
                vector<int> rest(ids.begin() + p, ids.end());
                return merge_sorted(first, sort_ids(move(rest)));
            }
        }

        // Merge-insertion sort: each pending item has a known upper bound.
        large = sort_ids(move(large));
        vector<int> chain;
        chain.reserve(n);
        chain.push_back(partner[large[0]]);
        chain.insert(chain.end(), large.begin(), large.end());

        const int pending = (n + 1) / 2;
        int done = 1, bound = 3, previous_bound = 1;
        while (done < pending) {
            for (int i = min(pending, bound); i > done; --i) {
                int x, hi;
                if (i <= int(large.size())) {
                    int upper = large[i - 1];
                    x = partner[upper];
                    hi = int(find(chain.begin(), chain.end(), upper) - chain.begin());
                } else {
                    x = ids.back();
                    hi = int(chain.size());
                }
                int lo = 0;
                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    if (less_than(chain[mid], x)) lo = mid + 1;
                    else hi = mid;
                }
                chain.insert(chain.begin() + lo, x);
            }
            done = bound;
            int next_bound = bound + 2 * previous_bound;
            previous_bound = bound;
            bound = next_bound;
        }
        return chain;
    }
};

class InsertionSearch {
    static constexpr array<double, 5> prior{0.64, 0.09, 0.09, 0.10, 0.08};
    array<double, 5> weight = prior;
    int last_slot = -1;

public:
    void reset() {
        weight = prior;
        last_slot = -1;
    }

    int insert(Blackboard& judge, vector<int>& a, int x) {
        const int m = int(a.size()), slots = m + 1;
        const double center = 0.5 * m;
        const double local_center = last_slot < 0
            ? center : clamp(last_slot - 0.5, 0.0, double(m));
        const double center_width = max(2.0, 0.33 * m);
        const double local_width = max(2.0, 0.065 * m);

        vector<array<double, 5>> likelihood(slots);
        array<double, 5> normalizer{};
        for (int i = 0; i < slots; ++i) {
            likelihood[i] = {
                1.0, double(i == 0), double(i == m),
                max(0.0, 1.0 - abs(i - center) / center_width),
                max(0.0, 1.0 - abs(i - local_center) / local_width)
            };
            for (int k = 0; k < 5; ++k) normalizer[k] += likelihood[i][k];
        }

        vector<double> pref(slots + 1);
        for (int i = 0; i < slots; ++i) {
            double p = 0;
            for (int k = 0; k < 5; ++k) {
                likelihood[i][k] /= normalizer[k];
                p += weight[k] * likelihood[i][k];
            }
            pref[i + 1] = pref[i] + p;
        }

        // Bias toward likely insertion slots, but allow at most one
        // comparison more than a balanced search in the worst case.
        int depth = 1 + int(bit_width(unsigned(slots - 1)));
        int lo = 0, hi = m;
        while (lo < hi) {
            int capacity = 1 << (--depth);
            int first = max(lo + 1, hi + 1 - capacity);
            int last = min(hi, lo + capacity);
            double target = (pref[lo] + pref[hi + 1]) * 0.5;
            int k = int(lower_bound(pref.begin() + first, pref.begin() + last + 1,
                                    target) - pref.begin());
            k = min(k, last);
            if (k > first && abs(pref[k - 1] - target) <= abs(pref[k] - target)) --k;

            int pivot = k - 1;
            bool go_left;
            if (pref[k] - pref[lo] >= pref[hi + 1] - pref[k]) {
                go_left = !judge.less_than(a[pivot], x);
            } else {
                go_left = judge.less_than(x, a[pivot]);
            }

            if (go_left) hi = pivot;
            else lo = k;
        }

        double evidence = pref[lo + 1] - pref[lo];
        for (int k = 0; k < 5; ++k) {
            weight[k] = 0.90 * weight[k] * likelihood[lo][k] / evidence + 0.10 * prior[k];
        }
        last_slot = lo;
        a.insert(a.begin() + lo, x);
        return lo;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) return 0;
    Blackboard judge(n);
    vector<int> board(n);
    iota(board.begin(), board.end(), 1);

    if (n == 2) {
        judge.add(1, 2);
        cout << '!' << endl;
        return 0;
    }

    bool possibly_all_equal = false;
    board = judge.sort_ids(move(board), &possibly_all_equal);
    if (possibly_all_equal && !judge.less_than(board.front(), board.back())) {
        // All values have the same sign (or are zero).
        while (board.size() > 1) {
            int a = board.back();
            board.pop_back();
            int b = board.back();
            board.back() = judge.add(a, b);
        }
        cout << '!' << endl;
        return 0;
    }

    InsertionSearch insertion;
    deque<int> recent;
    while (board.size() > 2) {
        const int m = int(board.size());
        const int pairs = (m + 1) / 3;
        const bool edge_trend = count(recent.begin(), recent.end(), -1) >= 2
                             || count(recent.begin(), recent.end(), 1) >= 2;
        if (pairs >= 2 && !edge_trend && (m < 24 || recent.size() == 3)) {
            // In the current sorted snapshot, pair ranks i and m+1-i.
            // Write p=i-1. There are p values beyond either end of this
            // pair, and at least p between its members.
            // If its sum exceeded R, both operands would be positive.
            // The upper p values and p middle values would sum to at
            // least p times the pair sum, offsetting the lower p values.
            // Thus the total would exceed R too. Negation handles -R.
            // The pairs are disjoint, so all can be added before sorting.
            vector<int> made;
            made.reserve(pairs);
            for (int i = 0; i < pairs; ++i) {
                made.push_back(judge.add(board[i], board[m - 1 - i]));
            }
            vector<int> kept(board.begin() + pairs, board.end() - pairs);
            made = judge.sort_ids(move(made));
            board = judge.merge_sorted(kept, made);
            insertion.reset();
            recent.clear();
        } else {
            int x = judge.add(board.front(), board.back());
            board.pop_back();
            board.erase(board.begin());
            if (board.size() == 1) {
                board.push_back(x);
                break;
            }

            int remaining = int(board.size());
            int slot = insertion.insert(judge, board, x);
            int fringe = remaining / 48;
            int edge = slot <= fringe ? -1 : (slot >= remaining - fringe ? 1 : 0);
            recent.push_back(edge);
            if (recent.size() > 3) recent.pop_front();
        }
    }

    if (board.size() == 2) judge.add(board[0], board[1]);
    cout << '!' << endl;
    return 0;
}
