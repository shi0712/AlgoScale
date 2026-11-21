#include <bits/stdc++.h>
using namespace std;

int n;
vector<vector<signed char>> orderCache;
vector<vector<int>> leaves;

int reply() {
    int x;
    if (!(cin >> x) || x == -1) exit(0);
    return x;
}

// Either ordering of equal values is suitable.
bool before(int a, int b) {
    if (orderCache[a][b]) return orderCache[a][b] < 0;
    cout << "? " << a << ' ' << b << endl;
    bool r = reply() == 1;
    orderCache[a][b] = r ? -1 : 1;
    orderCache[b][a] = r ? 1 : -1;
    return r;
}

int add(int a, int b) {
    cout << "+ " << a << ' ' << b << endl;
    int c = reply();
    leaves[c] = move(leaves[a]);
    leaves[c].insert(leaves[c].end(), leaves[b].begin(), leaves[b].end());
    return c;
}

void insertBounded(vector<int>& chain, int x, int hi) {
    int lo = 0;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (before(x, chain[mid])) hi = mid;
        else lo = mid + 1;
    }
    chain.insert(chain.begin() + lo, x);
}

// Merge-insertion sort. Only search to the left of a pending
// element's partner, inserting in Jacobsthal batches.
vector<int> order(vector<int> v) {
    int m = v.size();
    if (m < 2) return v;
    vector<int> highs;
    vector<int> partner(n + 1);
    int forwardPairs = 0;
    for (int i = 0; i + 1 < m; i += 2) {
        int a = v[i], b = v[i + 1];
        if (before(a, b)) ++forwardPairs;
        else swap(a, b);
        highs.push_back(b);
        partner[b] = a;
    }

    if (m >= 8 && (forwardPairs == 0 || forwardPairs == m / 2)) {
        vector<int> run = v;
        if (!forwardPairs) reverse(run.begin(), run.end());
        bool ok = true;
        int start = (!forwardPairs && m % 2) ? 1 : 2;
        for (int i = start; i < m; i += 2) {
            if (before(run[i], run[i - 1])) {
                ok = false;
                break;
            }
        }
        if (ok) return run;
    }

    highs = order(move(highs));
    vector<int> chain = highs;
    chain.insert(chain.begin(), partner[highs[0]]);
    int count = (m + 1) / 2;
    int done = 1, batch = 2, nextBatch = 2;
    while (done < count) {
        int end = min(count, done + batch);
        for (int i = end - 1; i >= done; --i) {
            if (i == (int)highs.size()) {
                insertBounded(chain, v.back(), chain.size());
            } else {
                int hi = find(chain.begin(), chain.end(), highs[i]) - chain.begin();
                insertBounded(chain, partner[highs[i]], hi);
            }
        }
        done = end;
        int tmp = batch;
        batch = nextBatch;
        nextBatch += 2 * tmp;
    }
    return chain;
}

// Can the initial sorted order and total-sum bound alone certify this
// subset? Normalize each original value to t=(A/R+1)/2. The sorted t
// vector is a convex combination of suffix indicators, with expected
// suffix length <= (n+1)/2. Find a nonnegative slope bounding the
// subset sum from above for every suffix. Reverse ranks for -A.
bool certified(const vector<int>& ids) {
    if (ids.size() <= 1) return true;
    vector<char> used(n);
    int m = 0;
    for (int x : ids) {
        for (int j : leaves[x]) used[j] = 1;
        m += leaves[x].size();
    }
    for (int rev = 0; rev <= 1; ++rev) {
        int ln = 0, ld = 1, hn = m + 1, hd = n + 1;
        int cnt = 0;
        for (int k = 1; k <= n; ++k) {
            cnt += used[rev ? k - 1 : n - k];
            int y = 2 * cnt - m - 1;
            int x = 2 * k - n - 1;
            // Require y <= slope*x.
            if (x < 0) {
                if (y > 0) return false;
                if ((-y) * hd < hn * (-x)) hn = -y, hd = -x;
            } else if (x > 0) {
                if (y * ld > ln * x) ln = y, ld = x;
            } else if (y > 0) return false;
            if (ln * hd > hn * ld) return false;
        }
    }
    return true;
}

// v is sorted and its sum is known to be representable.
int reduceSorted(vector<int> v) {
    int m = v.size();
    if (m == 1) return v[0];
    if (m == 2) return add(v[0], v[1]);

    if (m % 2) {
        // For odd length, the alternating sum lies between the first
        // and last values (group the adjacent differences). Therefore
        // both (sum +/- alternating_sum)/2 are representable.
        vector<int> a, b;
        for (int i = 0; i < m; ++i)
            (i % 2 ? b : a).push_back(v[i]);
        int x = reduceSorted(move(a));
        int y = reduceSorted(move(b));
        return add(x, y);
    }

    // Look for a comparison-free split using the global rank certificate.
    for (int mode = 0; mode < 12; ++mode) {
        vector<int> a, b;
        for (int i = 0; i < m; ++i) {
            bool take;
            if (mode == 0) take = (i % 2 == 0);
            else if (mode == 1) take = (i % 2 == (i >= m / 2));
            else if (mode == 2) take = (i == 0 || i == m - 1);
            else if (mode < 8) {
                int q = max(1, m / 2 - 1);
                int offset = (mode - 3) * m / 5;
                take = ((i + 1) * q + offset) / m != (i * q + offset) / m;
            } else {
                int pos[] = {m / 2, m / 2 - 1, 0, m - 1};
                take = i == pos[mode - 8];
            }
            (take ? a : b).push_back(v[i]);
        }
        if (!a.empty() && !b.empty() && certified(a) && certified(b)) {
            int x = reduceSorted(move(a));
            int y = reduceSorted(move(b));
            return add(x, y);
        }
    }

    // Opposite signs are safe; if the extrema have the same sign,
    // the magnitude of their sum cannot exceed that of the whole list.
    int x = add(v.front(), v.back());
    v.pop_back();
    v.erase(v.begin());
    insertBounded(v, x, v.size());
    return reduceSorted(move(v));
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n < 2) return 0;
    leaves.resize(2 * n + 1);
    orderCache.assign(2 * n + 1, vector<signed char>(2 * n + 1));
    if (n == 2) {
        add(1, 2);
    } else {
        vector<int> v(n);
        iota(v.begin(), v.end(), 1);
        v = order(move(v));
        for (int i = 0; i < n; ++i) leaves[v[i]].push_back(i);
        reduceSorted(move(v));
    }
    cout << "!" << endl;
    return 0;
}
