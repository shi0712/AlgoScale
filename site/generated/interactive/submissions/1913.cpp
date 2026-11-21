#include <bits/stdc++.h>
using namespace std;

int n, k, need;
bool reversedOrder;

int better(int a, int b) {
    cout << "? " << a << ' ' << b << '\n' << flush;
    int w;
    if (!(cin >> w) || (w != a && w != b)) exit(0);
    return reversedOrder ? (w == a ? b : a) : w;
}

struct Node {
    int label, first, second;
};
vector<Node> tree;
vector<int> labels;

int build(int l, int r) {
    if (r - l == 1) {
        tree.push_back({labels[l], -1, -1});
    } else {
        int m = (l + r) / 2;
        int a = build(l, m), b = build(m, r);
        if (better(tree[a].label, tree[b].label) == tree[b].label)
            swap(a, b);
        tree.push_back({tree[a].label, a, b});
    }
    return (int)tree.size() - 1;
}

// For very small selections, repeatedly take the champion. Only its
// direct opponents need to enter the next tournament.
vector<int> smallSelection() {
    using P = pair<int,int>;
    priority_queue<P, vector<P>, greater<P>> q;
    vector<vector<P>> opponents(n + 1);
    for (int x : labels) q.emplace(1, x);
    vector<int> ans;
    while ((int)ans.size() < need) {
        while (q.size() > 1) {
            auto [sa, a] = q.top(); q.pop();
            auto [sb, b] = q.top(); q.pop();
            if (better(a, b) == b) {
                swap(a, b);
                swap(sa, sb);
            }
            opponents[a].emplace_back(sb, b);
            q.emplace(sa + sb, a);
        }
        int a = q.top().second;
        q.pop();
        ans.push_back(a);
        if ((int)ans.size() < need)
            for (auto p : opponents[a]) q.push(p);
    }
    return ans;
}

using Mask = uint64_t;
Mask bit(int i) { return Mask(1) << i; }

// downSet[i] contains every currently represented label already known
// to be worse than i (in the requested selection direction).
Mask downSet[64];
int groupNode[64], cnt;
int slotVal[32], baseSize, cached[128];

void recordWin(int a, int b) {
    Mask extra = bit(b) | downSet[b];
    for (int j = 0; j < cnt; ++j)
        if (j == a || (downSet[j] & bit(a))) downSet[j] |= extra;
}

int worse(int a, int b) {
    if (downSet[a] & bit(b)) return b;
    if (downSet[b] & bit(a)) return a;
    if (better(tree[groupNode[a]].label, tree[groupNode[b]].label)
        == tree[groupNode[a]].label) {
        recordWin(a, b);
        return b;
    }
    recordWin(b, a);
    return a;
}

// Lazy minimum tournament on the selected slots.
int minimumAt(int t) {
    if (t >= baseSize) {
        int s = t - baseSize;
        return s < need ? slotVal[s] : -1;
    }
    if (cached[t] != -2) return cached[t];
    int a = minimumAt(2*t), b = minimumAt(2*t+1);
    return cached[t] = a < 0 ? b : b < 0 ? a : worse(a, b);
}

vector<int> layeredSelection() {
    vector<int> roots(need);
    for (int i = 0; i < need; ++i)
        roots[i] = build(i*n/need, (i+1)*n/need);

    baseSize = 1;
    while (baseSize < need) baseSize *= 2;
    fill(begin(cached), end(cached), -2);

    for (;;) {
        // The roots are the best 'need' group champions at this level.
        // A discarded group can never contribute an answer. Split only
        // the retained groups, exposing at most one challenger per slot.
        cnt = need;
        vector<pair<int,int>> challenges;
        for (int i = 0; i < need; ++i) {
            slotVal[i] = i;
            Node u = tree[roots[i]];
            groupNode[i] = roots[i];
            if (u.first != -1) {
                groupNode[i] = u.first;
                groupNode[cnt] = u.second;
                downSet[cnt] = 0;
                challenges.emplace_back(i, cnt++);
            }
        }
        if (challenges.empty()) break;
        for (auto [p, c] : challenges) recordWin(p, c);

        // Try children of stronger parents early, raising the cutoff.
        stable_sort(challenges.begin(), challenges.end(),
            [](auto x, auto y) {
                return popcount(downSet[x.first]) > popcount(downSet[y.first]);
            });

        Mask active = bit(need) - 1;
        for (auto [p, c] : challenges) {
            if (!(active & bit(p))) continue;
            if ((downSet[p] & active) == 0) {
                bool allAbove = true;
                for (int i = 0; i < need; ++i)
                    if (!(downSet[slotVal[i]] & bit(c))) {
                        allAbove = false;
                        break;
                    }
                if (allAbove) continue;
            }
            int m = minimumAt(1);
            if (worse(c, m) == c) continue;
            int s = 0;
            while (slotVal[s] != m) ++s;
            slotVal[s] = c;
            active ^= bit(m) | bit(c);
            for (int t = (baseSize + s)/2; t; t /= 2)
                cached[t] = -2;
        }

        // Project all learned comparisons onto the surviving champions.
        Mask nextSet[32]{};
        int renumber[64]{};
        for (int i = 0; i < need; ++i) {
            int a = slotVal[i];
            roots[i] = groupNode[a];
            renumber[a] = i;
            for (int j = 0; j < need; ++j)
                if (downSet[a] & bit(slotVal[j])) nextSet[i] |= bit(j);
        }
        copy(nextSet, nextSet + need, downSet);
        for (int t = 1; t < baseSize; ++t)
            if (cached[t] >= 0) cached[t] = renumber[cached[t]];
    }

    vector<int> ans;
    for (int r : roots) ans.push_back(tree[r].label);
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n >> k)) return 0;
    labels.resize(n);
    iota(labels.begin(), labels.end(), 1);
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    shuffle(labels.begin(), labels.end(), rng);
    reversedOrder = n-k < k;
    need = min(k, n-k);
    vector<int> chosen;
    if (need > 0)
        chosen = need <= 6 ? smallSelection() : layeredSelection();
    vector<bool> emit(n+1, reversedOrder);
    for (int x : chosen) emit[x] = !reversedOrder;
    cout << '!';
    for (int i = 1; i <= n; ++i)
        if (emit[i]) cout << ' ' << i;
    cout << '\n' << flush;
    return 0;
}
