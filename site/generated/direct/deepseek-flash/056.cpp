#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

struct Monster {
    int64 S, H, P;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 L, T;
    cin >> N >> L >> T;

    vector<Monster> mon(N);
    int64 totalWeightedHP = 0; // sum H_i * P_i

    for (int i = 0; i < N; ++i) {
        cin >> mon[i].S >> mon[i].H >> mon[i].P;
        totalWeightedHP += mon[i].H * mon[i].P;
    }

    sort(mon.begin(), mon.end(),
         [](const Monster& a, const Monster& b) {
             if (a.S != b.S) return a.S < b.S;
             return a.P > b.P;
         });

    auto calcPenalty = [&](int64 x) -> int64 {
        // Maximum saved penalty for difficulty x, by simulating the
        // optimal greedy schedule: at every moment attack the available
        // monster with largest P_i.
        struct Item {
            int64 P;
            int64 rem;
        };
        struct Cmp {
            bool operator()(const Item& a, const Item& b) const {
                return a.P < b.P; // max-heap by P
            }
        };

        priority_queue<Item, vector<Item>, Cmp> pq;

        i128 saved = 0;

        auto processInterval = [&](int64 len) {
            while (len > 0 && !pq.empty()) {
                Item cur = pq.top();
                pq.pop();

                int64 take = min(len, cur.rem);
                saved += (i128)take * cur.P;

                cur.rem -= take;
                len -= take;

                if (cur.rem > 0) {
                    pq.push(cur);
                    // The interval is fully used by this monster.
                    break;
                }
            }
        };

        int64 curTime = 0;
        int ptr = 0;

        while (ptr < N) {
            int64 s = mon[ptr].S;
            if (curTime < s) {
                processInterval(s - curTime);
                curTime = s;
            }

            while (ptr < N && mon[ptr].S == s) {
                // At difficulty x this monster has x * H_i HP.
                // It is sufficient to cap it because no more than T
                // attacks can ever be made.
                i128 cap128 = (i128)x * mon[ptr].H;
                int64 cap = (int64)min<i128>(cap128, (i128)4e18);
                if (cap > 0) {
                    pq.push({mon[ptr].P, cap});
                }
                ++ptr;
            }
        }

        if (curTime < T) {
            processInterval(T - curTime);
        }

        i128 initial = (i128)x * totalWeightedHP;
        i128 penalty = initial - saved;
        if (penalty < 0) penalty = 0;
        return (int64)penalty;
    };

    int Q;
    cin >> Q;

    vector<int64> M(Q);
    for (int i = 0; i < Q; ++i) cin >> M[i];

    vector<int> ans(Q, 0);

    // g(x) is non-decreasing, so parallel binary search is possible.
    // For safety with Q up to 1e6, we use divide-and-conquer over the
    // monotone answers and memoize all evaluated difficulties.
    unordered_map<long long, long long> memo;
    memo.reserve(1 << 20);

    auto getPenalty = [&](int64 x) -> int64 {
        auto it = memo.find(x);
        if (it != memo.end()) return it->second;
        int64 val = calcPenalty(x);
        memo.emplace(x, val);
        return val;
    };

    function<void(int64, int64, int, int)> solve =
        [&](int64 lo, int64 hi, int ql, int qr) {
            if (ql > qr) return;

            if (lo == hi) {
                for (int i = ql; i <= qr; ++i) ans[i] = (int)lo;
                return;
            }

            int64 mid = lo + (hi - lo) / 2;
            int64 penMid = getPenalty(mid);

            // Find the first query whose limit is >= penMid.
            int split = ql;
            while (split <= qr && M[split] < penMid) ++split;

            // Queries [ql, split-1] have answer < mid.
            // Queries [split, qr] have answer >= mid.
            solve(lo, mid - 1, ql, split - 1);
            solve(mid, hi, split, qr);
        };

    // If even difficulty 1 is impossible, answers are 0.
    int firstPossible = 0;
    while (firstPossible < Q && M[firstPossible] < getPenalty(1)) {
        ans[firstPossible] = 0;
        ++firstPossible;
    }

    if (firstPossible < Q) {
        // For remaining queries, answer is in [1, L].
        solve(1, L, firstPossible, Q - 1);
    }

    for (int i = 0; i < Q; ++i) {
        cout << ans[i] << '\n';
    }

    return 0;
}
