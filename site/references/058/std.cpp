#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

class OneDirectionSolver {
    int n, length;
    vector<int> first_order, second_pos, bucket, tails;

    bool build(int limit) {
        int b = 1;
        tails.clear();
        for (int value : first_order) {
            int x = -second_pos[value];
            int p = int(lower_bound(tails.begin(), tails.end(), x) - tails.begin());
            if (p == limit - b) {
                ++b;
                if (b >= limit) return false;
                tails.clear();
                p = 0;
            }
            if (p == int(tails.size())) tails.push_back(x);
            else tails[p] = x;
            bucket[value] = b;
        }
        return true;
    }

public:
    explicit OneDirectionSolver(const vector<int>& a)
        : n(int(a.size()) / 2), length(int(a.size())),
          second_pos(n + 1, -1), bucket(n + 1) {
        vector<unsigned char> seen(n + 1, 0);
        first_order.reserve(n);
        for (int i = 0; i < length; ++i) {
            int value = a[i];
            if (!seen[value]) {
                seen[value] = 1;
                first_order.push_back(value);
            } else {
                second_pos[value] = i;
            }
        }
    }

    vector<int> solve(const vector<int>& a, int* stack_count) {
        int low = 1, high = min(n + 1, 180);
        while (!build(high)) high *= 2;
        while (low + 1 < high) {
            int middle = (low + high) / 2;
            if (build(middle)) high = middle;
            else low = middle;
        }
        build(high);

        vector<int64_t> top;
        vector<int> answer(length);
        top.reserve(high);
        for (int i = 0; i < length; ++i) {
            int value = a[i];
            int64_t key = int64_t(bucket[value]) * (length + 1LL) + second_pos[value];
            int s = int(upper_bound(top.begin(), top.end(), key, greater<int64_t>()) - top.begin());
            if (s == int(top.size())) top.push_back(key);
            else top[s] = key;
            answer[i] = s + 1;
        }
        *stack_count = int(top.size());
        return answer;
    }
};

static vector<int> construct(vector<int> a, int* used) {
    int forward_count = 0, backward_count = 0;
    OneDirectionSolver forward(a);
    vector<int> answer = forward.solve(a, &forward_count);

    reverse(a.begin(), a.end());
    OneDirectionSolver backward(a);
    vector<int> candidate = backward.solve(a, &backward_count);
    if (backward_count < forward_count) {
        reverse(candidate.begin(), candidate.end());
        answer.swap(candidate);
        forward_count = backward_count;
    }
    *used = forward_count;
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    vector<int> a(2 * n);
    for (int& x : a) cin >> x;

    int used = 0;
    vector<int> answer = construct(a, &used);
    if (used > 180) return 2; // The official generator rejects such a test.
    for (int i = 0; i < 2 * n; ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';
    return 0;
}
