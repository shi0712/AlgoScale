#include <bits/stdc++.h>
using namespace std;

int N, K;
unordered_map<uint64_t, bool> cache;

bool better(int x, int y) {
    if (x == y) return false;
    if (x > y) return !better(y, x);
    uint64_t key = ((uint64_t)x << 15) | y;
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;
    cout << "? " << x << ' ' << y << '\n';
    cout.flush();
    int w;
    cin >> w;
    if (w == -1) exit(0);
    bool res = (w == x);
    cache[key] = res;
    return res;
}

void makeHeap(vector<int>& v) {
    int n = v.size();
    for (int i = n / 2 - 1; i >= 0; --i) {
        int idx = i;
        while (true) {
            int l = 2 * idx + 1;
            int r = 2 * idx + 2;
            int best = idx;
            if (l < n && better(v[l], v[best])) best = l;
            if (r < n && better(v[r], v[best])) best = r;
            if (best != idx) {
                swap(v[idx], v[best]);
                idx = best;
            } else break;
        }
    }
}

void popHeap(vector<int>& v) {
    int n = v.size();
    swap(v[0], v[n - 1]);
    v.pop_back();
    --n;
    int idx = 0;
    while (true) {
        int l = 2 * idx + 1;
        int r = 2 * idx + 2;
        int best = idx;
        if (l < n && better(v[l], v[best])) best = l;
        if (r < n && better(v[r], v[best])) best = r;
        if (best != idx) {
            swap(v[idx], v[best]);
            idx = best;
        } else break;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cache.reserve(20000);

    cin >> N >> K;

    if (K == N) {
        cout << "!";
        for (int i = 1; i <= N; ++i) cout << ' ' << i;
        cout << '\n';
        cout.flush();
        return 0;
    }

    vector<vector<int>> defeated(N + 1);
    vector<int> cur(N);
    iota(cur.begin(), cur.end(), 1);

    while (cur.size() > 1) {
        vector<int> nxt;
        nxt.reserve((cur.size() + 1) / 2);
        for (size_t i = 0; i < cur.size(); i += 2) {
            if (i + 1 == cur.size()) {
                nxt.push_back(cur[i]);
            } else {
                int a = cur[i], b = cur[i + 1];
                if (better(a, b)) {
                    defeated[a].push_back(b);
                    nxt.push_back(a);
                } else {
                    defeated[b].push_back(a);
                    nxt.push_back(b);
                }
            }
        }
        cur = move(nxt);
    }

    int root = cur[0];
    vector<int> selected;
    selected.reserve(K);
    selected.push_back(root);

    if (K == 1) {
        cout << "! " << root << '\n';
        cout.flush();
        return 0;
    }

    vector<vector<int>> batches;
    vector<int> globalHeap;

    int b0 = batches.size();
    batches.push_back(move(defeated[root]));
    makeHeap(batches[b0]);
    if (!batches[b0].empty()) {
        globalHeap.push_back(b0);
        int i = globalHeap.size() - 1;
        while (i > 0) {
            int p = (i - 1) / 2;
            if (better(batches[globalHeap[i]][0], batches[globalHeap[p]][0])) {
                swap(globalHeap[i], globalHeap[p]);
                i = p;
            } else break;
        }
    }

    for (int step = 2; step <= K; ++step) {
        int b = globalHeap[0];
        int last = globalHeap.back();
        globalHeap.pop_back();
        if (!globalHeap.empty()) {
            globalHeap[0] = last;
            int i = 0;
            int n = globalHeap.size();
            while (true) {
                int l = 2 * i + 1;
                int r = 2 * i + 2;
                int best = i;
                if (l < n && better(batches[globalHeap[l]][0], batches[globalHeap[best]][0])) best = l;
                if (r < n && better(batches[globalHeap[r]][0], batches[globalHeap[best]][0])) best = r;
                if (best != i) {
                    swap(globalHeap[i], globalHeap[best]);
                    i = best;
                } else break;
            }
        }

        int x = batches[b][0];
        selected.push_back(x);
        popHeap(batches[b]);

        if (!batches[b].empty()) {
            globalHeap.push_back(b);
            int i = globalHeap.size() - 1;
            while (i > 0) {
                int p = (i - 1) / 2;
                if (better(batches[globalHeap[i]][0], batches[globalHeap[p]][0])) {
                    swap(globalHeap[i], globalHeap[p]);
                    i = p;
                } else break;
            }
        }

        if (!defeated[x].empty()) {
            int newId = batches.size();
            batches.push_back(move(defeated[x]));
            makeHeap(batches[newId]);
            globalHeap.push_back(newId);
            int i = globalHeap.size() - 1;
            while (i > 0) {
                int p = (i - 1) / 2;
                if (better(batches[globalHeap[i]][0], batches[globalHeap[p]][0])) {
                    swap(globalHeap[i], globalHeap[p]);
                    i = p;
                } else break;
            }
        }
    }

    cout << "!";
    for (int x : selected) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    return 0;
}
