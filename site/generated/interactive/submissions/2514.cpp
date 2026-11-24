#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Summary {
    int64 total = 0;
    int64 mss = 0;
    int64 prefix = 0;
    int64 suffix = 0;
};

struct Run {
    int left, right;
};

int n;
vector<int64> answer;
vector<int> positive;
vector<Run> runs;
unordered_map<int, int64> cache;

int64 ask(int left, int right) {
    if (left > right) return 0;
    int key = left * n + right;
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;

    cout << "? " << left + 1 << ' ' << right + 1 << endl;
    int64 value;
    if (!(cin >> value) || value < 0) exit(0);
    cache.emplace(key, value);
    return value;
}

Summary combine(const Summary& left, const Summary& right) {
    Summary result;
    result.total = left.total + right.total;
    result.prefix = max(left.prefix, left.total + right.prefix);
    result.suffix = max(right.suffix, right.total + left.suffix);
    result.mss = max({left.mss, right.mss, left.suffix + right.prefix});
    return result;
}

void discover_signs(int left, int right, int64 mss) {
    if (mss == 0) {
        fill(positive.begin() + left, positive.begin() + right + 1, 0);
        return;
    }
    if (left == right) {
        positive[left] = 1;
        answer[left] = mss;
        return;
    }

    constexpr int BLOCK = 32;
    if (right - left + 1 <= BLOCK) {
        bool all_previous_positive = true;
        int64 sum = 0;
        for (int i = left; i < right; ++i) {
            int64 value = ask(i, i);
            if (value > 0) {
                positive[i] = 1;
                answer[i] = value;
                sum += value;
            } else {
                positive[i] = 0;
                all_previous_positive = false;
            }
        }
        if (all_previous_positive) {
            if (sum == mss) {
                positive[right] = 0;
            } else {
                positive[right] = 1;
                answer[right] = mss - sum;
            }
        } else {
            int64 value = ask(right, right);
            positive[right] = value > 0;
            answer[right] = value;
        }
        return;
    }

    int middle = (left + right) / 2;
    int64 left_mss = ask(left, middle);
    int64 right_mss = ask(middle + 1, right);
    discover_signs(left, middle, left_mss);
    discover_signs(middle + 1, right, right_mss);
}

Summary solve(int left, int right, int run_left, int run_right) {
    if (left > right) return {};
    if (run_left == run_right) {
        int64 sum = 0;
        for (int i = left; i <= right; ++i) sum += answer[i];
        return {sum, sum, sum, sum};
    }

    int64 mss = ask(left, right);
    if (mss == 0) {
        fill(answer.begin() + left, answer.begin() + right + 1, 0);
        return {};
    }

    int count = run_right - run_left;
    vector<int64> left_values(count, -1), right_values(count, -1);
    auto get_left = [&](int index) -> int64 {
        int offset = index - run_left;
        if (left_values[offset] == -1) {
            left_values[offset] = ask(left, runs[index].left - 1);
        }
        return left_values[offset];
    };
    auto get_right = [&](int index) -> int64 {
        int offset = index - run_left;
        if (right_values[offset] == -1) {
            right_values[offset] = ask(runs[index].right + 1, right);
        }
        return right_values[offset];
    };

    int chosen = -1;
    if (get_left(run_left) == mss && get_right(run_left) == mss) {
        chosen = run_left;
    } else if (count > 1 && get_left(run_right - 1) == mss &&
               get_right(run_right - 1) == mss) {
        chosen = run_right - 1;
    }

    auto better = [&](int first, int second) {
        if (second == -1) return true;
        int64 first_left = get_left(first), first_right = get_right(first);
        int64 second_left = get_left(second), second_right = get_right(second);
        pair<int64, int64> first_score = {
            min(first_left, first_right), first_left + first_right
        };
        pair<int64, int64> second_score = {
            min(second_left, second_right), second_left + second_right
        };
        return first_score > second_score;
    };

    if (chosen == -1) {
        int low = run_left, high = run_right;
        while (low < high) {
            int middle = low + (high - low) / 2;
            if (get_left(middle) < get_right(middle)) {
                low = middle + 1;
            } else {
                high = middle;
            }
        }
        int crossing = low;

        if (crossing > run_left) {
        int last = crossing - 1;
        int64 target = get_left(last);
        low = run_left;
        high = last;
        while (low < high) {
            int middle = low + (high - low) / 2;
            if (get_left(middle) >= target) {
                high = middle;
            } else {
                low = middle + 1;
            }
        }
        chosen = low;
    }

    if (crossing < run_right) {
        int first = crossing;
        int64 target = get_right(first);
        low = first;
        high = run_right;
        while (low < high) {
            int middle = low + (high - low) / 2;
            if (middle == first) {
                low = middle + 1;
            } else if (get_right(middle) < target) {
                high = middle;
            } else {
                low = middle + 1;
            }
        }
        int last = low - 1;
        if (better(last, chosen)) chosen = last;
        }
    }

    const Run removed = runs[chosen];
    Summary left_summary = solve(left, removed.left - 1, run_left, chosen);
    Summary right_summary = solve(removed.right + 1, right, chosen + 1, run_right);

    int64 value = mss - left_summary.suffix - right_summary.prefix;
    answer[removed.left] = value;
    fill(answer.begin() + removed.left + 1, answer.begin() + removed.right + 1, 0);

    Summary middle_summary{value, 0, 0, 0};
    return combine(combine(left_summary, middle_summary), right_summary);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    answer.assign(n, 0);
    positive.assign(n, 0);
    cache.reserve(n * 64);

    int64 whole_mss = ask(0, n - 1);
    if (whole_mss == 0) {
        cout << '!';
        for (int i = 0; i < n; ++i) cout << ' ' << 0;
        cout << endl;
        return 0;
    }

    discover_signs(0, n - 1, whole_mss);
    for (int i = 0; i < n;) {
        if (positive[i]) {
            ++i;
            continue;
        }
        int right = i;
        while (right + 1 < n && !positive[right + 1]) ++right;
        runs.push_back({i, right});
        i = right + 1;
    }

    if (!runs.empty()) solve(0, n - 1, 0, runs.size());

    cout << '!';
    for (int i = 0; i < n; ++i) cout << ' ' << answer[i];
    cout << endl;
    return 0;
}
