#include <bits/stdc++.h>
using namespace std;

void answer(int x, int y) {
    cout << "! " << x << " " << y << endl;
    int res;
    if (!(cin >> res) || res == -1) exit(0);
    exit(0);
}

void solve(vector<int> S) {
    int sz = S.size();
    if (sz == 1) {
        answer(S[0], S[0]);
    }
    if (sz == 2) {
        answer(S[0], S[1]);
    }

    int i = S[0];
    vector<int> T;
    T.reserve(sz - 1);
    for (int j : S) if (j != i) T.push_back(j);

    int max_g = -1;
    vector<int> A;
    for (int j : T) {
        cout << "? " << i << " " << j << endl;
        int g;
        if (!(cin >> g) || g == -1) exit(0);
        if (g > max_g) {
            max_g = g;
            A.clear();
            A.push_back(j);
        } else if (g == max_g) {
            A.push_back(j);
        }
    }

    if (A.size() == 1) {
        answer(i, A[0]);
    } else {
        solve(A);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    vector<int> S(n);
    iota(S.begin(), S.end(), 1);
    solve(S);
    return 0;
}
