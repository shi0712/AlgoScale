#include <bits/stdc++.h>
using namespace std;

constexpr int L = 15;       // The 15th occurrence certifies a big cycle.
constexpr int EXTRA = 16;
constexpr int BRIDGE = 17;

int queryCount = 0;
unordered_map<long long, int> memo;

int ask(int parent, int edge) {
    long long key = (static_cast<long long>(parent) << 16) | edge;
    auto it = memo.find(key);
    if (it != memo.end()) return it->second;

    cout << "? " << parent << ' ' << edge << endl;
    int reply;
    if (!(cin >> reply) || (reply != 0 && reply != 1)) exit(0);
    int id = ++queryCount;
    return memo[key] = (reply ? id : 0);
}

// Minimum weighted cost binary decision tree with ordered outcomes.
// splitAt[a][b] separates [a,k] from [k+1,b].
int splitAt[20][20];
void makeTree(const vector<double>& weights) {
    int n = (int)weights.size();
    double dp[20][20], pref[21] = {};
    for (int i = 0; i < n; ++i) {
        dp[i][i] = 0;
        pref[i + 1] = pref[i] + weights[i];
    }
    for (int len = 2; len <= n; ++len) {
        for (int a = 0, b = len - 1; b < n; ++a, ++b) {
            double best = 1e100;
            for (int k = a; k < b; ++k) {
                double v = dp[a][k] + dp[k + 1][b];
                if (v < best) {
                    best = v;
                    splitAt[a][b] = k;
                }
            }
            dp[a][b] = best + pref[b + 1] - pref[a];
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int m;
    if (!(cin >> m)) return 0;
    memo.reserve(12 * m);

    vector<int> layer[L + 2];
    int head[L + 2] = {};
    int balanced[L + 1][L + 1] = {};
    vector<int> length(m + 1);
    double recent[18] = {}, transition[18][18] = {};
    int prev = 0;

    // Layer i deletes exactly one representative from each cycle seen
    // at least i times: its i-th edge in input order.
    for (int e = 1; e <= m; ++e) {
        vector<int> options{1};
        for (int i = 2; i <= L; ++i)
            if (layer[i - 1].size() > layer[i].size()) options.push_back(i);
        if (!layer[L].empty()) options.push_back(EXTRA);
        options.push_back(BRIDGE);

        vector<double> weights;
        for (int x : options) {
            weights.push_back((x == 1 ? 1.4 : 0.45) +
                              recent[x] + 2.0 * transition[prev][x]);
        }
        makeTree(weights);
        int lo = 0, hi = (int)options.size() - 1;
        int accepted[18] = {};
        while (lo < hi) {
            int mid = splitAt[lo][hi], x = options[mid];
            // Success iff not a bridge and occurrence index <= x.
            // For EXTRA this is just the singleton deletion test.
            int id = ask(x == EXTRA ? 0 : head[x], e);
            if (id) {
                accepted[x] = id;
                hi = mid;
            } else {
                lo = mid + 1;
            }
        }
        int x = options[lo];
        for (int j = 1; j <= BRIDGE; ++j) {
            recent[j] *= 0.97;
            transition[prev][j] *= 0.93;
        }
        recent[x] += 1;
        transition[prev][x] += 1;
        prev = x;

        if (x == BRIDGE) length[e] = -1;
        else if (x == EXTRA) length[e] = L;
        else {
            // The split immediately to the right of this outcome was
            // necessarily tested, so the new version already exists.
            head[x] = accepted[x];
            layer[x].push_back(e);
            if (x == L) length[e] = L;
            for (int i = 1; i < x; ++i)
                if (layer[i].size() == layer[x].size()) balanced[i][x] = e;
        }
    }

    // Propagate capped lengths down one layer at a time. A continuing
    // cycle must match a later, as yet unaccounted-for next-layer edge.
    for (int i = L - 1; i >= 1; --i) {
        int available[L + 1] = {};
        available[i] = (int)layer[i].size() - (int)layer[i + 1].size();
        int j = (int)layer[i + 1].size() - 1;
        int lastEqual = balanced[i][i + 1];
        int early = upper_bound(layer[i].begin(), layer[i].end(), lastEqual)
                    - layer[i].begin();

        for (int pos = (int)layer[i].size() - 1; pos >= 0; --pos) {
            int e = layer[i][pos];
            while (j >= 0 && layer[i + 1][j] > e)
                ++available[length[layer[i + 1][j--]]];

            int lower = i;
            for (int k = i + 1; k <= L; ++k)
                if (balanced[i][k] >= e) lower = k;

            // All cycles stopping at i have their i-th edge after the
            // last equal prefix. Sometimes every remaining such slot is due.
            int ans = i;
            if (!(pos >= early && available[i] == pos - early + 1)) {
                vector<int> options;
                vector<double> weights;
                for (int k = lower; k <= L; ++k) {
                    if (available[k] > 0) {
                        options.push_back(k);
                        weights.push_back(available[k]);
                    }
                }
                makeTree(weights);
                int lo = 0, hi = (int)options.size() - 1;
                while (lo < hi) {
                    int mid = splitAt[lo][hi];
                    int threshold = options[mid + 1];
                    if (ask(head[threshold], e)) hi = mid;
                    else lo = mid + 1;
                }
                ans = options[lo];
            }
            length[e] = ans;
            --available[ans];
        }
    }

    cout << '!';
    for (int e = 1; e <= m; ++e)
        cout << ' ' << (length[e] == L ? 0 : length[e]);
    cout << endl;
    int confirmation;
    if (!(cin >> confirmation) || confirmation != 1) return 0;
    return 0;
}
