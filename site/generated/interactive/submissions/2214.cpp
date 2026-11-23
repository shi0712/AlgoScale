#include <bits/stdc++.h>
using namespace std;

struct Node {
    int color;
    vector<int> children;
};

vector<Node> nodes;
vector<int> a;

int parse_tree(int l, int r) {
    int id = (int)nodes.size();
    nodes.push_back({a[l], {}});

    if (r - l == 1) return id;

    int c = a[l];
    int q = l + 1;
    while (q < r && a[q] != c) ++q;

    int p = l + 1;
    while (p < q) {
        int d = a[p];
        int e = p + 1;
        while (e < q && a[e] >= d) ++e;

        int child = parse_tree(p, e);
        nodes[id].children.push_back(child);
        p = e;
    }

    int child = parse_tree(q, r);
    nodes[id].children.push_back(child);
    return id;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int M;
    cin >> M;

    for (int i = 0; i < M; ++i) {
        cout << "? 1 " << i << ' ' << (M - 1 - i) << '\n' << flush;
        int response;
        cin >> response;
        if (response == -1) return 0;
        if (response != 1) return 0;
    }

    cout << "? 2\n" << flush;

    int N;
    cin >> N;
    if (N == -1) return 0;

    vector<int> values(N);
    for (int &x : values) cin >> x;

    a.resize(N);
    for (int i = 0; i < N; ++i) {
        a[i] = M - 1 - values[i];
    }

    nodes.reserve(N);
    int root = parse_tree(0, N);

    vector<int> label(N, -1);
    int next_internal = M;

    for (int i = 0; i < N; ++i) {
        if (i == root) {
            label[i] = N - 1;
        } else if (nodes[i].children.empty()) {
            label[i] = nodes[i].color;
        } else {
            label[i] = next_internal++;
        }
    }

    vector<int> parent(N - 1, -1);
    for (int i = 0; i < N; ++i) {
        for (int child : nodes[i].children) {
            parent[label[child]] = label[i];
        }
    }

    cout << "! " << N;
    for (int i = 0; i < N - 1; ++i) {
        cout << ' ' << parent[i];
    }
    cout << '\n' << flush;

    return 0;
}
