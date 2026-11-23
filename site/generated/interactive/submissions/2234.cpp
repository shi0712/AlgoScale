#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, t;
    if (!(cin >> n >> t)) return 0;

    vector<int> leftSum(4 * n + 5, -1);
    vector<int> path;
    path.reserve(32);

    for (int round = 0; round < t; ++round) {
        int k;
        if (!(cin >> k)) return 0;

        int node = 1, l = 1, r = n;
        path.clear();

        while (l < r) {
            int mid = (l + r) >> 1;

            if (leftSum[node] == -1) {
                cout << "? " << l << ' ' << mid << endl;
                int response;
                if (!(cin >> response) || response < 0) return 0;
                leftSum[node] = response;
            }

            int zerosLeft = (mid - l + 1) - leftSum[node];
            if (k <= zerosLeft) {
                path.push_back(node);
                node <<= 1;
                r = mid;
            } else {
                k -= zerosLeft;
                node = (node << 1) | 1;
                l = mid + 1;
            }
        }

        for (int u : path) ++leftSum[u];
        cout << "! " << l << endl;
    }

    return 0;
}
