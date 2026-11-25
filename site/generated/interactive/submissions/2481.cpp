#include <iostream>
#include <vector>

using namespace std;

int n;
vector<int> answer;

bool lessAt(int i, int j) {
    cout << "? " << i + 1 << ' ' << j + 1 << '\n' << flush;
    char response;
    if (!(cin >> response)) exit(0);
    return response == '<';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    answer.assign(n, 0);
    if (n == 1) {
        cout << "! 1\n" << flush;
        return 0;
    }

    vector<int> previous(n), next(n);
    for (int i = 0; i < n; ++i) {
        previous[i] = i - 1;
        next[i] = i + 1;
    }
    int tail = n - 1;

    auto erase = [&](int position) {
        int left = previous[position];
        int right = next[position];
        if (left >= 0) next[left] = right;
        if (right < n) previous[right] = left;
        if (tail == position) tail = left;
    };

    int best = lessAt(previous[tail], tail) ? tail : previous[tail];
    bool ordered = true;

    for (int value = n; value >= 3; --value) {
        int candidate = previous[previous[tail]];
        if (lessAt(best, candidate)) {
            answer[candidate] = value;
            erase(candidate);
            ordered = true;
        } else {
            answer[best] = value;
            erase(best);
            ordered = false;
            if (value > 3) {
                best = lessAt(previous[tail], tail) ? tail : previous[tail];
                ordered = true;
            }
        }
    }

    if (!ordered) best = lessAt(previous[tail], tail) ? tail : previous[tail];
    answer[best] = 2;
    answer[best == tail ? previous[tail] : tail] = 1;

    cout << '!';
    for (int value : answer) cout << ' ' << value;
    cout << '\n' << flush;
    return 0;
}
