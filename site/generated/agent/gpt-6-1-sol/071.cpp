#include <bits/stdc++.h>
using namespace std;

using U64 = unsigned long long;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
int ask(int l, int r) {
    cout << "? " << l << ' ' << r << endl;
    int reply;
    if (!(cin >> reply) || reply == -1) exit(0);
    return reply;
}

vector<int> solve(int n) {
    // rank[i] is the zero-based rank of position i in the known prefix.
    vector<int> rank(n + 1);
    rank[1] = 0;
    for (int r = 2; r <= n; ++r) {
        int words = (r + 63) / 64;
        vector<U64> table(size_t(r + 1) * words, 0);
        vector<int> base(r + 1, 0), bit(r + 1, 0);
        int inversion = 0;
        // base[l] is the inversion parity of the already known [l,r-1].
        // Bit k of table[l] is the extra parity caused by position r,
        // assuming exactly k previous elements are smaller than it.
        for (int l = r - 1; l >= 1; --l) {
            int t = rank[l];
            int count = 0;
            for (int x = t; x; x -= x & -x) count += bit[x];
            inversion ^= count & 1;
            base[l] = inversion;
            for (int x = t + 1; x <= r; x += x & -x) ++bit[x];
            U64* cur = &table[size_t(l) * words];
            const U64* prev = cur + words;
            copy(prev, prev + words, cur);
            int w = t >> 6;
            for (int j = 0; j < w; ++j) cur[j] = ~cur[j];
            cur[w] ^= ~0ULL >> (63 - (t & 63));
        }
        vector<U64> candidates(words, ~0ULL);
        if (r & 63) candidates.back() = (1ULL << (r & 63)) - 1;
        vector<int> active(words);
        iota(active.begin(), active.end(), 0);
        int remaining = r;
        while (remaining > 1) {
            int best_l = -1, best_score = -1;
            auto consider = [&](int l) {
                if (l < 1 || l >= r) return;
                const U64* row = &table[size_t(l) * words];
                int ones = 0;
                for (int w : active) ones += __builtin_popcountll(row[w] & candidates[w]);
                int score = min(ones, remaining - ones);
                if (score > best_score) {
                    best_score = score;
                    best_l = l;
                }
            };
            consider(1);
            // Try a small random sample; scan every interval if the split is poor.
            for (int t = 0; t < 24 && best_score < remaining / 2; ++t)
                consider(1 + rng() % (r - 1));
            if (best_score * 5 < remaining * 2) {
                for (int l = 1; l < r && best_score < remaining / 2; ++l) consider(l);
            }
            const U64* row = &table[size_t(best_l) * words];
            int parity = ask(best_l, r) ^ base[best_l];
            remaining = 0;
            vector<int> new_active;
            for (int w : active) {
                candidates[w] &= parity ? row[w] : ~row[w];
                remaining += __builtin_popcountll(candidates[w]);
                if (candidates[w]) new_active.push_back(w);
            }
            active.swap(new_active);
        }
        int position = active[0] * 64 + __builtin_ctzll(candidates[active[0]]);
        for (int j = 1; j < r; ++j) rank[j] += rank[j] >= position;
        rank[r] = position;
    }
    vector<int> answer(n);
    for (int i = 1; i <= n; ++i) answer[i - 1] = rank[i] + 1;
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    auto answer = solve(n);
    cout << '!';
    for (int value : answer) cout << ' ' << value;
    cout << endl;
}
