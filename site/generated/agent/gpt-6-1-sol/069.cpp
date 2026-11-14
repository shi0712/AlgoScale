#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

static int64 ask(int l, int r) {
    cout << "? " << l + 1 << ' ' << r + 1 << endl;
    int64 reply;
    if (!(cin >> reply) || reply == -1) exit(0);
    return reply;
}

struct Block {
    int l, r, prev = -1, next = -1, le = -1, re = -1;
    int64 sum;
    int version = 0;
    bool alive = true;
};

struct Gap {
    // The total of the interval between its two current positive blocks.
    int64 upper, value = 0;
    int left_child = -1, right_child = -1;
    int position = -1;
    int64 pivot = 0;
    bool assigned = false;
};

static void solve(int n) {
    const int64 INF = 4'000'000'000'000'000LL;
    // -1 means unqueried; 0 means that the original value is nonpositive.
    vector<int64> positive(n, -1);
    positive[0] = ask(0, 0);
    int i = 0;
    while (i + 1 < n) {
        if (positive[i] == 0) {
            if (positive[i + 1] == -1)
                positive[i + 1] = ask(i, i + 1);
            ++i;
        } else if (i + 2 < n) {
            int j = i + 2;
            positive[j] = ask(j, j);
            if (positive[j] == 0 && positive[i + 1] == -1)
                positive[i + 1] = max(0LL, ask(i, j) - positive[i]);
            // With two positive endpoints, leave the middle for the gap phase.
            if (positive[j] == 0 && positive[i + 1] > 0)
                ++i;
            else
                i = j;
        } else {
            if (positive[i + 1] == -1)
                positive[i + 1] = max(0LL, ask(i, i + 1) - positive[i]);
            ++i;
        }
    }

    vector<int64> answer(n, 0);
    vector<Block> blocks;
    vector<Gap> gaps;
    blocks.reserve(n);
    gaps.reserve(2 * n);
    for (i = 0; i < n; ++i) {
        if (positive[i] <= 0) continue;
        answer[i] = positive[i];
        if (!blocks.empty() && blocks.back().r == i - 1) {
            blocks.back().r = i;
            blocks.back().sum += positive[i];
        } else {
            if (!blocks.empty()) {
                int l = blocks.back().r + 1;
                int r = i - 1;
                Gap gap;
                gap.position = l;
                gap.upper = (l == r && positive[l] == -1) ? INF : 0;
                gaps.push_back(gap);
            }
            Block block;
            block.l = block.r = i;
            block.sum = positive[i];
            blocks.push_back(block);
        }
    }

    int k = (int)blocks.size();
    using Entry = tuple<int64, int, int>;
    priority_queue<Entry, vector<Entry>, greater<Entry>> heap;
    for (int j = 0; j < k; ++j) {
        blocks[j].prev = j - 1;
        blocks[j].next = (j + 1 < k ? j + 1 : -1);
        blocks[j].le = j - 1;
        blocks[j].re = (j + 1 < k ? j : -1);
        heap.emplace(blocks[j].sum, j, 0);
    }

    while (!heap.empty()) {
        auto [weight, id, version] = heap.top();
        heap.pop();
        if (!blocks[id].alive || blocks[id].version != version) continue;
        int left = blocks[id].prev, right = blocks[id].next;
        if (left == -1 && right == -1) break;

        bool merged = false;
        for (int side = 0; side < 2; ++side) {
            if ((side == 0 && left == -1) || (side == 1 && right == -1)) continue;
            int x = (side == 0 ? left : id);
            int y = (side == 0 ? id : right);
            int e = (side == 0 ? blocks[id].le : blocks[id].re);
            if (gaps[e].upper <= -weight) continue;

            int64 maximum = ask(blocks[x].l, blocks[y].r);
            if (maximum <= max(blocks[x].sum, blocks[y].sum)) {
                gaps[e].upper = -weight;
                continue;
            }

            // A strict improvement uses both entire endpoint blocks.
            gaps[e].value = maximum - blocks[x].sum - blocks[y].sum;
            gaps[e].assigned = true;
            blocks[x].sum = maximum;
            blocks[x].r = blocks[y].r;
            blocks[x].next = blocks[y].next;
            blocks[x].re = blocks[y].re;
            ++blocks[x].version;
            if (blocks[y].next != -1) {
                int z = blocks[y].next;
                blocks[z].prev = x;
                blocks[z].le = blocks[x].re;
            }
            blocks[y].alive = false;
            heap.emplace(blocks[x].sum, x, blocks[x].version);
            merged = true;
            break;
        }
        if (merged) continue;

        // Both incident gaps are at most -weight. The smallest block can
        // be removed: fix its left gap to -weight and transfer the remaining
        // total to its right gap. All MSS values are preserved.
        if (left == -1) {
            int e = blocks[id].re;
            gaps[e].value = -weight;
            gaps[e].assigned = true;
            blocks[right].prev = blocks[right].le = -1;
        } else if (right == -1) {
            int e = blocks[id].le;
            gaps[e].value = -weight;
            gaps[e].assigned = true;
            blocks[left].next = blocks[left].re = -1;
        } else {
            int le = blocks[id].le, re = blocks[id].re;
            Gap gap;
            gap.left_child = le;
            gap.right_child = re;
            gap.pivot = weight;
            gap.upper = gaps[le].upper + weight + gaps[re].upper;
            int e = (int)gaps.size();
            gaps.push_back(gap);
            blocks[left].next = right;
            blocks[left].re = e;
            blocks[right].prev = left;
            blocks[right].le = e;
        }
        blocks[id].alive = false;
    }

    // Every composite gap is created after its children. Propagate its
    // chosen total backwards, ultimately assigning one value per original gap.
    for (int e = (int)gaps.size() - 1; e >= 0; --e) {
        if (gaps[e].left_child == -1) {
            answer[gaps[e].position] = gaps[e].value;
        } else {
            int le = gaps[e].left_child, re = gaps[e].right_child;
            gaps[le].value = -gaps[e].pivot;
            gaps[re].value = gaps[e].value;
        }
    }

    // If U middle positions were left unresolved, the first phase costs n-U.
    // Let V be the other gaps and K=U+V+1 the number of initial blocks.
    // Each V gap uses either two nonpositive positions, or an extra positive
    // position in its left block. Thus n >= 2U+3V+1.
    // The second phase uses <=2K-3 queries (K>=2), giving <=5(n-1)/3 overall.
    cout << '!';
    for (int64 x : answer) cout << ' ' << x;
    cout << endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n;
        if (!(cin >> n) || n == -1) return 0;
        solve(n);
    }
    return 0;
}
