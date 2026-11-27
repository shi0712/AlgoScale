#include <bits/stdc++.h>
using namespace std;

class Solver {
private:
    int n;

    int queryUnionSize(int l, int r) {
        string subset(n, '0');
        for (int i = l; i < r; ++i) {
            subset[i] = '1';
        }

        cout << "? " << subset << endl;

        int response;
        if (!(cin >> response) || response == -1) {
            exit(0);
        }

        return response + (r - l);
    }

    struct Result {
        int unionSize;
        bool connected;
    };

    Result solve(int l, int r) {
        if (r - l == 1) {
            return {queryUnionSize(l, r), true};
        }

        int mid = (l + r) / 2;
        Result left = solve(l, mid);
        Result right = solve(mid, r);

        int totalUnionSize = queryUnionSize(l, r);
        bool hasCrossEdge =
            totalUnionSize < left.unionSize + right.unionSize;

        return {
            totalUnionSize,
            hasCrossEdge && left.connected && right.connected
        };
    }

public:
    explicit Solver(int n) : n(n) {}

    bool isConnected() {
        return solve(0, n).connected;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) {
        return 0;
    }

    Solver solver(n);
    cout << "! " << (solver.isConnected() ? 1 : 0) << endl;

    return 0;
}
