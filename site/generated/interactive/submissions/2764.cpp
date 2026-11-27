#include <bits/stdc++.h>
using namespace std;

int ask(int v, const string& b) {
    cout << "? " << v << ' ' << b << '\n' << flush;

    int response;
    if (!(cin >> response) || response < 0) {
        exit(0);
    }
    return response;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    const int m = n - 1;
    const int root = 1;

    // For the root r, side[e] is 1 exactly when r belongs to the
    // component containing s_e after deleting edge e.
    vector<int> side(m);

    string allZero(m, '0');
    const int total = ask(root, allZero);

    function<void(int, int, int)> recover = [&](int l, int r, int sum) {
        if (l > r) return;

        if (l == r) {
            side[l] = sum;
            return;
        }

        int mid = (l + r) / 2;
        int leftSize = mid - l + 1;

        string b(m, '1');
        for (int e = l; e <= mid; ++e) {
            b[e] = '0';
        }

        int response = ask(root, b);
        int numerator = response - (m - leftSize) + total;

        if (numerator < 0 || (numerator & 1)) exit(0);

        int leftSum = numerator / 2;
        int rightSum = sum - leftSum;

        if (leftSum < 0 || leftSum > leftSize || rightSum < 0) exit(0);

        recover(l, mid, leftSum);
        recover(mid + 1, r, rightSum);
    };

    recover(0, m - 1, total);

    string base(m, '0');
    for (int e = 0; e < m; ++e) {
        base[e] = char('0' + side[e]);
    }

    // With b = side[root], the response at v equals dist(root, v).
    vector<int> depth(n + 1, 0);
    for (int v = 2; v <= n; ++v) {
        depth[v] = ask(v, base);
    }

    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (depth[a] != depth[b]) return depth[a] < depth[b];
        return a < b;
    });

    // Count the edges of the root-v path belonging to a given subset.
    auto pathCount = [&](int v, const vector<int>& subset, int d) -> int {
        string b = base;
        for (int e : subset) {
            b[e] = (b[e] == '0' ? '1' : '0');
        }

        int response = ask(v, b);
        int numerator = d + static_cast<int>(subset.size()) - response;

        if (numerator < 0 || (numerator & 1)) exit(0);
        return numerator / 2;
    };

    // The supplied candidate set is known to contain exactly one edge
    // of the root-v path.
    auto findUnique = [&](int v, const vector<int>& candidates, int d) -> int {
        vector<int> current = candidates;

        while (current.size() > 1) {
            int mid = static_cast<int>(current.size()) / 2;

            vector<int> left(current.begin(), current.begin() + mid);
            int count = pathCount(v, left, d);

            if (count == 1) {
                current.swap(left);
            } else if (count == 0) {
                vector<int> right(current.begin() + mid, current.end());
                current.swap(right);
            } else {
                exit(0);
            }
        }

        return current[0];
    };

    vector<vector<int>> layer(n + 1);
    vector<int> unassigned(m);
    iota(unassigned.begin(), unassigned.end(), 0);

    vector<int> edgeChild(m, -1);
    vector<int> edgeParent(m, -1);
    vector<int> vertexParent(n + 1, -1);

    for (int v : order) {
        if (v == root) continue;

        int d = depth[v];
        int parent;

        if (d == 1) {
            parent = root;
        } else {
            if (layer[d - 1].empty()) exit(0);

            int parentEdge = findUnique(v, layer[d - 1], d);
            parent = edgeChild[parentEdge];

            if (parent == -1) exit(0);
        }

        if (unassigned.empty()) exit(0);

        int connectingEdge = findUnique(v, unassigned, d);

        auto it = find(unassigned.begin(), unassigned.end(), connectingEdge);
        if (it == unassigned.end()) exit(0);
        unassigned.erase(it);

        edgeChild[connectingEdge] = v;
        edgeParent[connectingEdge] = parent;
        vertexParent[v] = parent;
        layer[d].push_back(connectingEdge);
    }

    cout << "!";
    for (int e = 0; e < m; ++e) {
        int parent = edgeParent[e];
        int child = edgeChild[e];

        if (parent == -1 || child == -1) exit(0);

        int s = (side[e] == 1 ? parent : child);
        int f = (side[e] == 1 ? child : parent);

        cout << ' ' << s << ' ' << f;
    }
    cout << '\n' << flush;

    return 0;
}
