#include <bits/stdc++.h>
using namespace std;

static int N, K;
static unordered_map<long long, int> winCache;   // key -> winner

inline long long pairKey(int a, int b) {
    if (a > b) swap(a, b);
    return ( (long long)a << 14 ) | b;   // N <= 15000 < 2^14
}

// Query the judge for the winner of a vs b, using cache.
int queryWinner(int a, int b) {
    if (a == b) return a;
    long long key = pairKey(a, b);
    auto it = winCache.find(key);
    if (it != winCache.end()) return it->second;

    cout << "? " << a << " " << b << "\n" << flush;
    int w;
    if (!(cin >> w)) exit(0);
    if (w == -1) exit(0);
    winCache[key] = w;
    return w;
}

// Comparator for max‑heap: returns true if a is weaker than b.
struct Compare {
    bool operator()(int a, int b) const {
        int w = queryWinner(a, b);
        return w == b;   // a < b  iff b beats a
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N >> K)) return 0;
    vector<vector<int>> losers(N + 1);   // direct losers of each player

    // Build tournament tree to find the champion.
    vector<int> cur(N);
    iota(cur.begin(), cur.end(), 1);
    while (cur.size() > 1) {
        vector<int> nxt;
        size_t i = 0;
        for (; i + 1 < cur.size(); i += 2) {
            int a = cur[i];
            int b = cur[i + 1];
            int w = queryWinner(a, b);
            int l = (w == a) ? b : a;
            losers[w].push_back(l);
            nxt.push_back(w);
        }
        if (i < cur.size()) { // odd player gets a bye
            nxt.push_back(cur[i]);
        }
        cur.swap(nxt);
    }
    int champion = cur[0];

    // Extract top K using the tournament‑tree selection method.
    vector<int> answer;
    answer.reserve(K);
    answer.push_back(champion);

    priority_queue<int, vector<int>, Compare> pq;
    for (int x : losers[champion]) pq.push(x);

    while ((int)answer.size() < K && !pq.empty()) {
        int best = pq.top(); pq.pop();
        answer.push_back(best);
        for (int x : losers[best]) pq.push(x);
    }

    // Output the result.
    cout << "! ";
    for (int i = 0; i < K; ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << "\n" << flush;
    return 0;
}
