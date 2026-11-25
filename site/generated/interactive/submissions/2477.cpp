#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    auto ask = [](int l, int r, int k) {
        cout << "? " << l << ' ' << r << ' ' << k << endl;
        int response;
        if (!(cin >> response)) exit(0);
        if (response < 0) exit(0);
        return response;
    };

    auto answer = [](int position) {
        cout << "! " << position << endl;
        exit(0);
    };

    if (n == 3) {
        int prefix = ask(1, 2, 2);
        int suffix = ask(2, 3, 2);
        if (prefix == 1) answer(3);
        if (suffix == 1) answer(1);
        answer(2);
    }

    vector<int> left(n + 1, -1), right(n + 1, -1);
    left[1] = 1;
    right[n] = 1;

    for (int i = 2; i < n; ++i) left[i] = ask(1, i, 2);

    vector<bool> candidate(n + 1, true);
    for (int i = 2; i < n; ++i) {
        if (left[i] == left[i - 1]) candidate[i] = false;
    }

    auto getRight = [&](int i) {
        if (right[i] == -1) right[i] = ask(i, n, 2);
        return right[i];
    };

    vector<int> certified;
    auto certify = [&](int i) {
        if (n % 2 == 1) answer(i);
        certified.push_back(i);
        if (certified.size() < 2) return;

        int a = certified[0], b = certified[1];
        if (a > b) swap(a, b);
        int l, r, tested;
        if (a > 1) {
            l = a - 1;
            r = a;
            tested = a;
        } else if (b > 2) {
            l = 1;
            r = 2;
            tested = a;
        } else {
            l = 2;
            r = 3;
            tested = b;
        }
        int response = ask(l, r, n);
        bool testedIsN = (response == 2);
        if (tested == a) answer(testedIsN ? b : a);
        answer(testedIsN ? a : b);
    };

    for (int i = n - 1; i >= 1; --i) {
        if (!candidate[i]) continue;
        if (getRight(i) > getRight(i + 1)) certify(i);
    }

    if (getRight(1) > left[n - 1]) certify(n);
    return 0;
}
