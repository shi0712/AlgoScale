#include <iostream>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int M;
    if (!(cin >> M)) return 0;

    vector<int> chainLength(M, 0);
    for (int leaf = 0; leaf < M; ++leaf) {
        int depth = 0;
        while (true) {
            int value = depth == 0 ? leaf : (M == 1 ? 0 : M);
            cout << "? 1 " << leaf << ' ' << value << endl;
            int response;
            if (!(cin >> response) || response == -1) return 0;
            if (response == 0) break;
            ++depth;
        }
        chainLength[leaf] = depth;
    }

    cout << "? 2" << endl;
    int collected;
    if (!(cin >> collected) || collected == -1) return 0;
    vector<int> sequence(collected);
    for (int &value : sequence) {
        if (!(cin >> value) || value == -1) return 0;
    }

    struct Node {
        int leaf;
        int depth;
        int parent = -1;
        int label = -1;
    };
    vector<Node> nodes;
    nodes.reserve(collected);
    int position = 0;

    auto parse = [&](auto &&self, int leaf, int depth) -> int {
        int current = static_cast<int>(nodes.size());
        nodes.push_back({leaf, depth});
        ++position;

        if (depth + 1 < chainLength[leaf]) {
            while (M > 1 && position < collected && sequence[position] < M) {
                int child = self(self, sequence[position], 0);
                nodes[child].parent = current;
            }
            int child = self(self, leaf, depth + 1);
            nodes[child].parent = current;
        }
        return current;
    };

    parse(parse, 0, 0);

    int N = static_cast<int>(nodes.size());
    int nextInternal = M;
    for (int i = 0; i < N; ++i) {
        if (i == 0) {
            nodes[i].label = N - 1;
        } else if (nodes[i].depth + 1 == chainLength[nodes[i].leaf]) {
            nodes[i].label = nodes[i].leaf;
        } else {
            nodes[i].label = nextInternal++;
        }
    }

    vector<int> answer(N - 1);
    for (int i = 1; i < N; ++i) {
        answer[nodes[i].label] = nodes[nodes[i].parent].label;
    }

    cout << "! " << N;
    for (int parent : answer) cout << ' ' << parent;
    cout << endl;
    return 0;
}
