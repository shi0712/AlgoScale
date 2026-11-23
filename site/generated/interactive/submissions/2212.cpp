#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> A = {0}, B;
    A.reserve(n);
    B.reserve(n);

    auto ask = [&](const vector<int>& q) -> int {
        cout << "? " << q.size();
        for (int x : q) cout << ' ' << x;
        cout << endl;

        int result;
        if (!(cin >> result)) exit(0);
        if (result < 0) exit(0);
        return result;
    };

    auto ready = [&]() -> bool {
        return !B.empty() && (A.size() >= 2 || B.size() >= 2);
    };

    int cur = 1;

    while (cur < n && !ready()) {
        if (B.empty()) {
            int k = min((int)A.size(), n - cur);
            vector<int> q;
            q.reserve(2 * k);

            for (int j = 0; j < k; ++j) {
                q.push_back(A[j]);
                q.push_back(cur + j);
            }

            int r = ask(q);

            if (r == 0) {
                for (int j = 0; j < k; ++j) A.push_back(cur + j);
                cur += k;
            } else if (r == 1) {
                for (int j = 0; j + 1 < k; ++j) A.push_back(cur + j);
                B.push_back(cur + k - 1);
                cur += k;
            } else if (r == 2 * k - 2) {
                for (int j = 0; j + 1 < k; ++j) B.push_back(cur + j);
                A.push_back(cur + k - 1);
                cur += k;
            } else if (r == 2 * k - 1) {
                for (int j = 0; j < k; ++j) B.push_back(cur + j);
                cur += k;
            } else {
                int lo = cur;
                int hi = cur + k - 2;

                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    int m = mid - lo + 1;

                    vector<int> test;
                    test.reserve(2 * m);
                    for (int j = 0; j < m; ++j) {
                        test.push_back(A[j]);
                        test.push_back(lo + j);
                    }

                    if (ask(test) > 0) hi = mid;
                    else lo = mid + 1;
                }

                for (int x = cur; x < lo; ++x) A.push_back(x);
                B.push_back(lo);
                cur = lo + 1;
            }
        } else {
            int r = ask({0, cur});
            if (r == 0) A.push_back(cur);
            else B.push_back(cur);
            ++cur;
        }
    }

    while (cur < n) {
        if (cur + 1 == n) {
            int r = ask({0, cur});
            if (r == 0) A.push_back(cur);
            else B.push_back(cur);
            ++cur;
            break;
        }

        int x = cur, y = cur + 1;
        bool useA = A.size() >= 2;
        int r;

        if (useA) r = ask({A[0], x, A[1], B[0], y});
        else r = ask({B[0], x, B[1], A[0], y});

        bool xMajority = (r <= 2);
        bool yMajority = (r % 2 == 0);

        if (useA) {
            if (xMajority) A.push_back(x);
            else B.push_back(x);
            if (yMajority) A.push_back(y);
            else B.push_back(y);
        } else {
            if (xMajority) B.push_back(x);
            else A.push_back(x);
            if (yMajority) B.push_back(y);
            else A.push_back(y);
        }

        cur += 2;
    }

    cout << "! " << A.size() << endl;
    return 0;
}
