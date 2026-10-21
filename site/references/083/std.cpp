#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

struct Block {
    int l, r;
    int64 sum;
    // Lower bound on the cost of the gap immediately to this block's left.
    int64 gap;
    // An adjustable position in that gap; -1 when there is no left block.
    int slot;
};

template <class Ask>
vector<int64> reconstruct(int n, Ask ask) {
    constexpr int64 A = 1'000'000'000LL;
    vector<int64> positive(n + 1, 0);
    vector<char> sampled(n + 1, false), unknown(n + 1, false);

    // Sample singletons: skip one position after a positive answer.
    for (int i = 1; i <= n; ) {
        positive[i] = ask(i, i);
        sampled[i] = true;
        if (positive[i] > 0 && i + 2 <= n) i += 2;
        else ++i;
    }

    // A skipped position whose right neighbour is nonpositive is easy to resolve.
    for (int i = 1; i <= n; ++i) {
        if (sampled[i]) continue;
        if (positive[i + 1] == 0) {
            positive[i] = ask(i - 1, i + 1) - positive[i - 1];
        } else {
            unknown[i] = true;
        }
    }

    vector<int64> b = positive;
    for (int i = 1; i <= n; ++i) {
        if (unknown[i]) b[i] = A;
    }

    // Maximal runs of already-known positive entries.
    vector<Block> runs;
    for (int i = 1; i <= n; ) {
        if (positive[i] == 0) {
            ++i;
            continue;
        }
        int l = i;
        int64 sum = 0;
        while (i <= n && positive[i] > 0) sum += positive[i++];
        runs.push_back({l, i - 1, sum, 0, -1});
    }

    vector<Block> st;
    st.reserve(runs.size());
    int previous_end = -1;

    for (Block cur : runs) {
        if (previous_end != -1) {
            cur.slot = previous_end + 1;
            if (cur.l == previous_end + 2 && unknown[cur.slot]) {
                cur.gap = -A;  // The skipped entry might be positive.
            } else {
                cur.gap = 0;   // Every entry in this gap is nonpositive.
            }
        }

        while (!st.empty()) {
            Block left = st.back();
            int64 small = min(left.sum, cur.sum);
            int64 large = max(left.sum, cur.sum);
            int64 answer = (cur.gap >= small)
                         ? large : ask(left.l, cur.r);

            if (answer > large) {
                // The entire interval is a maximum subarray: its sum is known.
                int64 needed = left.sum + cur.sum - answer;
                b[cur.slot] -= needed - cur.gap;
                cur.l = left.l;
                cur.sum = answer;
                cur.gap = left.gap;
                cur.slot = left.slot;
                st.pop_back();
            } else {
                // Only a lower bound on the gap cost is necessary for now.
                int64 needed = max(cur.gap, small);
                b[cur.slot] -= needed - cur.gap;
                cur.gap = needed;

                if (left.sum > cur.sum) break;

                // Remove the smaller left block from the active stack.
                // Its entries remain in b and become part of the combined gap.
                st.pop_back();
                if (st.empty()) {
                    cur.gap = 0;
                    cur.slot = -1;
                } else {
                    cur.gap += left.gap - left.sum;
                    cur.slot = left.slot;
                }
            }
        }
        st.push_back(cur);
        previous_end = cur.r;
    }
    return b;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n;
        if (!(cin >> n)) return 0;
        auto ask = [&](int l, int r) -> int64 {
            cout << "? " << l << ' ' << r << endl;
            int64 answer;
            if (!(cin >> answer) || answer < 0) exit(0);
            return answer;
        };
        vector<int64> b = reconstruct(n, ask);
        cout << '!';
        for (int i = 1; i <= n; ++i) cout << ' ' << b[i];
        cout << endl;
        // The statement specifies no extra verdict token after an answer.
    }
    return 0;
}
