#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

using namespace std;

int n, m;
int rootToward;

int ask(int v, const string &b) {
    cout << "? " << v << ' ' << b << '\n' << flush;
    int answer;
    if (!(cin >> answer) || answer < 0) {
        exit(0);
    }
    return answer;
}

void learnDirections(int l, int r, int ones, const string &zero,
                     vector<int> &direction) {
    if (ones == 0) {
        fill(direction.begin() + l, direction.begin() + r, 0);
        return;
    }
    if (ones == r - l) {
        fill(direction.begin() + l, direction.begin() + r, 1);
        return;
    }
    int mid = (l + r) / 2;
    string b = zero;
    fill(b.begin() + l, b.begin() + mid, '1');
    int answer = ask(1, b);
    int leftOnes = (rootToward + (mid - l) - answer) / 2;
    learnDirections(l, mid, leftOnes, zero, direction);
    learnDirections(mid, r, ones - leftOnes, zero, direction);
}

int pathCount(int v, int depth, const vector<int> &edges,
              const string &base) {
    string b = base;
    for (int edge : edges) {
        b[edge] = char('0' + '1' - b[edge]);
    }
    int answer = ask(v, b);
    return (depth + int(edges.size()) - answer) / 2;
}

int findSingleEdge(int v, int depth, vector<int> candidates,
                   const string &base) {
    while (candidates.size() > 1) {
        int leftSize = candidates.size() / 2;
        vector<int> left(candidates.begin(), candidates.begin() + leftSize);
        int count = pathCount(v, depth, left, base);
        if (count == 1) {
            candidates = move(left);
        } else if (count == 0) {
            candidates.erase(candidates.begin(), candidates.begin() + leftSize);
        } else {
            exit(0);
        }
    }
    return candidates[0];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) {
        return 0;
    }
    m = n - 1;
    string zero(m, '0');
    vector<int> direction(m);
    rootToward = ask(1, zero);
    learnDirections(0, m, rootToward, zero, direction);

    string base = zero;
    for (int i = 0; i < m; ++i) {
        base[i] = char('0' + direction[i]);
    }

    vector<int> depth(n + 1, 0);
    vector<int> order;
    for (int v = 2; v <= n; ++v) {
        depth[v] = (n == 2 ? 1 : ask(v, base));
        order.push_back(v);
    }
    sort(order.begin(), order.end(), [&](int a, int b) {
        return depth[a] < depth[b];
    });

    vector<vector<pair<int, int>>> children(n + 1);
    vector<int> edgeParent(m), edgeChild(m);
    vector<bool> used(m, false);

    for (int v : order) {
        int parent = 1;
        while (depth[parent] < depth[v] - 1) {
            vector<int> candidates;
            for (auto [child, edge] : children[parent]) {
                candidates.push_back(edge);
            }
            int edge = findSingleEdge(v, depth[v], candidates, base);
            for (auto [child, candidateEdge] : children[parent]) {
                if (candidateEdge == edge) {
                    parent = child;
                    break;
                }
            }
        }

        vector<int> candidates;
        for (int edge = 0; edge < m; ++edge) {
            if (!used[edge]) {
                candidates.push_back(edge);
            }
        }
        int edge = findSingleEdge(v, depth[v], candidates, base);
        used[edge] = true;
        edgeParent[edge] = parent;
        edgeChild[edge] = v;
        children[parent].push_back({v, edge});
    }

    cout << '!';
    for (int edge = 0; edge < m; ++edge) {
        int s = direction[edge] ? edgeParent[edge] : edgeChild[edge];
        int f = direction[edge] ? edgeChild[edge] : edgeParent[edge];
        cout << ' ' << s << ' ' << f;
    }
    cout << '\n' << flush;
    return 0;
}
