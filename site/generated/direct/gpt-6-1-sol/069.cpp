#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

constexpr int64 UNKNOWN = LLONG_MIN;
constexpr int64 VALUE_LIMIT = 1'000'000'000LL;

struct Constraint {
    int l, r;
    int64 value;
    bool equality;
};

struct Block {
    int l, r;
    int prev, next;
    int64 sum;
    bool alive;
};

struct Edge {
    int to;
    int64 weight;
};

class Solver {
private:
    int n;
    vector<int64> known;
    vector<Constraint> constraints;

    int64 ask(int l, int r) {
        cout << "? " << l << ' ' << r << endl;

        int64 answer;
        if (!(cin >> answer) || answer == -1) {
            exit(0);
        }
        return answer;
    }

    void readSingleton(int index) {
        if (known[index] != UNKNOWN) return;
        known[index] = ask(index, index);
    }

    /*
       Discover positive elements, while sometimes leaving one element
       unqueried between two known positive elements.
    */
    void discover() {
        int i = 1;

        while (i <= n) {
            readSingleton(i);

            if (known[i] == 0) {
                ++i;
                continue;
            }

            if (i + 2 <= n) {
                readSingleton(i + 2);

                if (known[i + 2] > 0) {
                    // Position i+1 remains unknown.
                    i += 2;
                } else {
                    readSingleton(i + 1);
                    i += 3;
                }
            } else {
                ++i;
            }
        }
    }

    void reconstructBlocks() {
        vector<Block> blocks;
        blocks.reserve(2 * n + 5);

        // Initially, a block is a maximal consecutive run of known positives.
        for (int i = 1; i <= n; ) {
            if (known[i] == UNKNOWN || known[i] == 0) {
                ++i;
                continue;
            }

            int l = i;
            int64 sum = 0;

            while (i <= n && known[i] != UNKNOWN && known[i] > 0) {
                sum += known[i];
                ++i;
            }

            int id = static_cast<int>(blocks.size());
            blocks.push_back({l, i - 1, id - 1, -1, sum, true});
            if (id > 0) blocks[id - 1].next = id;
        }

        int remaining = static_cast<int>(blocks.size());
        if (remaining <= 1) return;

        using HeapEntry = pair<int64, int>;
        priority_queue<HeapEntry, vector<HeapEntry>, greater<HeapEntry>> heap;

        for (int id = 0; id < remaining; ++id) {
            heap.emplace(blocks[id].sum, id);
        }

        auto mergeBlocks = [&](int left, int right, int64 sum) {
            int previous = blocks[left].prev;
            int following = blocks[right].next;
            int id = static_cast<int>(blocks.size());

            Block merged{
                blocks[left].l,
                blocks[right].r,
                previous,
                following,
                sum,
                true
            };

            blocks[left].alive = false;
            blocks[right].alive = false;
            blocks.push_back(merged);

            if (previous != -1) blocks[previous].next = id;
            if (following != -1) blocks[following].prev = id;

            heap.emplace(sum, id);
            --remaining;
        };

        auto compareBlocks = [&](int left, int right) -> bool {
            int l = blocks[left].l;
            int r = blocks[right].r;
            int64 answer = ask(l, r);

            bool joins = answer > max(blocks[left].sum, blocks[right].sum);
            constraints.push_back({l, r, answer, joins});

            if (joins) {
                mergeBlocks(left, right, answer);
            }

            return joins;
        };

        while (remaining > 1) {
            while (!blocks[heap.top().second].alive) {
                heap.pop();
            }

            int current = heap.top().second;
            heap.pop();

            int left = blocks[current].prev;
            int right = blocks[current].next;

            if (left != -1 && compareBlocks(left, current)) {
                continue;
            }

            if (right != -1 && compareBlocks(current, right)) {
                continue;
            }

            // The minimum block cannot help either neighboring block.
            // Remove it from the active list, retaining its constraints.
            blocks[current].alive = false;
            if (left != -1) blocks[left].next = right;
            if (right != -1) blocks[right].prev = left;

            --remaining;
        }
    }

    vector<int64> constructAnswer() {
        /*
           Let s[i] = b[1] + ... + b[i].

           Every condition is a difference constraint:
               s[v] <= s[u] + weight.
        */
        vector<vector<Edge>> graph(n + 1);

        auto addEdge = [&](int from, int to, int64 weight) {
            graph[from].push_back({to, weight});
        };

        for (int i = 1; i <= n; ++i) {
            // Keep all reported values within the original value bounds.
            addEdge(i - 1, i, VALUE_LIMIT);
            addEdge(i, i - 1, VALUE_LIMIT);

            if (known[i] == UNKNOWN) {
                continue;
            }

            if (known[i] == 0) {
                // The original element was nonpositive.
                addEdge(i - 1, i, 0);
            } else {
                // A positive singleton is known exactly.
                addEdge(i - 1, i, known[i]);
                addEdge(i, i - 1, -known[i]);
            }
        }

        for (const Constraint& c : constraints) {
            addEdge(c.l - 1, c.r, c.value);

            if (c.equality) {
                addEdge(c.r, c.l - 1, -c.value);
            }
        }

        // A super-source with zero-weight edges to every vertex is implicit.
        vector<int64> distance(n + 1, 0);
        vector<char> inQueue(n + 1, true);
        deque<int> queue;

        for (int i = 0; i <= n; ++i) {
            queue.push_back(i);
        }

        // The hidden sequence satisfies all constraints, so no negative cycle
        // exists.
        while (!queue.empty()) {
            int u = queue.front();
            queue.pop_front();
            inQueue[u] = false;

            for (const Edge& e : graph[u]) {
                int64 candidate = distance[u] + e.weight;
                if (candidate >= distance[e.to]) continue;

                distance[e.to] = candidate;

                if (!inQueue[e.to]) {
                    inQueue[e.to] = true;

                    if (!queue.empty() &&
                        distance[e.to] < distance[queue.front()]) {
                        queue.push_front(e.to);
                    } else {
                        queue.push_back(e.to);
                    }
                }
            }
        }

        vector<int64> answer(n + 1);
        for (int i = 1; i <= n; ++i) {
            answer[i] = distance[i] - distance[i - 1];
        }

        return answer;
    }

public:
    explicit Solver(int size) : n(size), known(n + 1, UNKNOWN) {}

    void solve() {
        discover();
        reconstructBlocks();

        vector<int64> answer = constructAnswer();

        cout << '!';
        for (int i = 1; i <= n; ++i) {
            cout << ' ' << answer[i];
        }
        cout << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T) || T == -1) return 0;

    while (T--) {
        int n;
        if (!(cin >> n) || n == -1) return 0;

        Solver solver(n);
        solver.solve();
    }

    return 0;
}
