#include <bits/stdc++.h>
using namespace std;

int n;

int ask(int i, int j) {
    cout << "? " << i << " " << j << endl;
    int r;
    cin >> r;
    if (r == -1) exit(0);
    return r;
}

void answer(int x, int y) {
    cout << "! " << x << " " << y << endl;
    int r;
    cin >> r;
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    if (n == 2) {
        answer(1, 2);
    }

    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 1);

    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count() ^
                (uint64_t)n * 1000003ULL);
    shuffle(ord.begin(), ord.end(), rng);

    int a = ord[0];
    int b = ord[1];

    int g = ask(a, b);
    int threshold = (n - 1) / 2;

    if (g > threshold) {
        answer(a, b);
    }

    for (int idx = 2; idx < n; ++idx) {
        int i = ord[idx];
        bool last = (idx == n - 1);

        int x = ask(a, i);

        if (x < g) {
            if (last) answer(a, b);
        } else if (x > g) {
            if (last) {
                answer(a, i);
            } else {
                b = i;
                g = x;
                if (g > threshold) answer(a, b);
            }
        } else {
            if (last) {
                answer(b, i);
            } else {
                int y = ask(b, i);

                if (y >= g) {
                    a = b;
                    b = i;
                    g = y;
                } else {
                    b = i;
                }

                if (g > threshold) answer(a, b);
            }
        }
    }

    answer(a, b);
    return 0;
}
