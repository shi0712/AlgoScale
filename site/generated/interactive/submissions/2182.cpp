#include <bits/stdc++.h>
using namespace std;

int N;
vector<int> board;

bool askLess(int a, int b) {
    if (a == b) return false;
    cout << "? " << a << ' ' << b << endl;
    int ans;
    if (!(cin >> ans)) exit(0);
    if (ans == -1) exit(0);
    return ans == 1;
}

int doAdd(int a, int b) {
    cout << "+ " << a << ' ' << b << endl;
    int c;
    if (!(cin >> c)) exit(0);
    if (c == -1) exit(0);
    return c;
}

void insertValue(int x) {
    int l = 0, r = (int)board.size();
    while (l < r) {
        int m = (l + r) >> 1;
        if (askLess(x, board[m])) r = m;
        else l = m + 1;
    }
    board.insert(board.begin() + l, x);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N)) return 0;

    if (N == 1) {
        cout << "!" << endl;
        return 0;
    }

    if (N == 2) {
        doAdd(1, 2);
        cout << "!" << endl;
        return 0;
    }

    board.reserve(N + 5);

    // Build the sorted list of initial values.
    for (int i = 1; i <= N; ++i) {
        insertValue(i);
    }

    while (board.size() > 1) {
        int a = board.front();
        int b = board.back();

        board.erase(board.begin());
        board.pop_back();

        int c = doAdd(a, b);

        // If only one old value remains, the last addition is always legal
        // because the sum of the two remaining values is the original total.
        if (board.size() == 1) {
            int r = board.front();
            board.clear();
            doAdd(r, c);
            cout << "!" << endl;
            return 0;
        }

        insertValue(c);
    }

    cout << "!" << endl;
    return 0;
}
