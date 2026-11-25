#include <iostream>
#include <vector>

using namespace std;

struct Node {
    int left = -1;
    int right = -1;
    int winner = -1;
};

int n, k;
vector<Node> tree;
bool failed = false;

int compare(int a, int b) {
    cout << "? " << a << ' ' << b << '\n' << flush;
    int winner;
    if (!(cin >> winner)) {
        failed = true;
        return -1;
    }
    return winner;
}

int build(vector<int> level) {
    while (level.size() > 1) {
        vector<int> next;
        for (int i = 0; i < (int)level.size(); i += 2) {
            if (i + 1 == (int)level.size()) {
                next.push_back(level[i]);
                continue;
            }
            int a = level[i], b = level[i + 1];
            int winner = compare(tree[a].winner, tree[b].winner);
            if (failed) return -1;
            Node node;
            node.left = a;
            node.right = b;
            node.winner = winner;
            tree.push_back(node);
            next.push_back((int)tree.size() - 1);
        }
        level = move(next);
    }
    return level[0];
}

int removeWinner(int node, int winner) {
    if (tree[node].left == -1) {
        tree[node].winner = -1;
        return -1;
    }

    if (tree[tree[node].left].winner == winner) {
        removeWinner(tree[node].left, winner);
    } else {
        removeWinner(tree[node].right, winner);
    }
    if (failed) return -1;

    int a = tree[tree[node].left].winner;
    int b = tree[tree[node].right].winner;
    if (a == -1) {
        tree[node].winner = b;
    } else if (b == -1) {
        tree[node].winner = a;
    } else {
        tree[node].winner = compare(a, b);
    }
    return tree[node].winner;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> k)) return 0;
    tree.reserve(2 * n);
    vector<int> leaves(n);
    for (int i = 0; i < n; ++i) {
        leaves[i] = (int)tree.size();
        Node leaf;
        leaf.winner = i + 1;
        tree.push_back(leaf);
    }

    int root = build(move(leaves));
    if (failed) return 0;

    vector<int> answer;
    answer.reserve(k);
    for (int i = 0; i < k; ++i) {
        int winner = tree[root].winner;
        answer.push_back(winner);
        if (i + 1 < k) {
            removeWinner(root, winner);
            if (failed) return 0;
        }
    }

    cout << '!';
    for (int x : answer) cout << ' ' << x;
    cout << '\n' << flush;
    return 0;
}
