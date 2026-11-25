#include <bits/stdc++.h>
using namespace std;

long long ask(const vector<pair<int, int>>& weighted) {
    int k = 0;
    for (auto [u, w] : weighted) k += w;
    cout << "? 1 " << k;
    for (auto [u, w] : weighted) {
        while (w--) cout << ' ' << u;
    }
    cout << '\n' << flush;
    long long response;
    if (!(cin >> response)) exit(0);
    return response;
}

void toggle(int u) {
    cout << "? 2 " << u << '\n' << flush;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    for (int i = 0, u, v; i < n - 1; ++i) cin >> u >> v;

    long long s = ask({{1, 1}});
    vector<int> b(n + 1);

    int t = 0;
    while ((1LL << (t + 1)) - 1 <= n) ++t;

    for (int first = 2; first <= n; first += t) {
        int count = min(t, n - first + 1);
        vector<pair<int, int>> query;
        int totalWeight = 0;
        for (int i = 0; i < count; ++i) {
            query.push_back({first + i, 1 << i});
            totalWeight += 1 << i;
        }
        long long response = ask(query);
        long long encoded = (response - totalWeight * s + totalWeight) / 2;
        for (int i = 0; i < count; ++i) {
            b[first + i] = ((encoded >> i) & 1) ? 1 : -1;
        }
    }

    vector<int> answer(n + 1);
    if (s == 1 || s == -1) {
        answer[1] = s;
        for (int v = 2; v <= n; ++v) answer[v] = b[v];
    } else if (s == 0) {
        bool uniform = true;
        for (int v = 3; v <= n; ++v) uniform &= b[v] == b[2];
        if (uniform) {
            answer[1] = -b[2];
        } else {
            toggle(1);
            long long after = ask({{1, 1}});
            answer[1] = after / 2;
        }
        for (int v = 2; v <= n; ++v) answer[v] = b[v];
    } else {
        int rootValue = s / 2;
        vector<int> candidates;
        for (int v = 2; v <= n; ++v) {
            if (b[v] == -rootValue) candidates.push_back(v);
        }

        if (candidates.size() == 1) {
            answer[1] = rootValue;
            for (int v = 2; v <= n; ++v) answer[v] = b[v];
            answer[candidates[0]] = rootValue;
        } else {
            toggle(1);
            int maxWeighted = 0;
            while ((maxWeighted + 1LL) * (maxWeighted + 2) / 2 <= n) {
                ++maxWeighted;
            }

            while ((int)candidates.size() > maxWeighted) {
                int half = candidates.size() / 2;
                vector<pair<int, int>> query;
                long long baseline = 0;
                for (int i = 0; i < half; ++i) {
                    query.push_back({candidates[i], 1});
                    baseline += b[candidates[i]];
                }
                long long response = ask(query);
                if (response == baseline + s) {
                    candidates.resize(half);
                } else {
                    candidates.erase(candidates.begin(), candidates.begin() + half);
                }
            }

            int root = candidates[0];
            if (candidates.size() > 1) {
                vector<pair<int, int>> query;
                long long baseline = 0;
                for (int i = 0; i < (int)candidates.size(); ++i) {
                    query.push_back({candidates[i], i + 1});
                    baseline += (i + 1LL) * b[candidates[i]];
                }
                long long response = ask(query);
                root = candidates[(response - baseline) / s - 1];
            }

            answer[1] = -rootValue;
            for (int v = 2; v <= n; ++v) answer[v] = b[v];
            answer[root] = rootValue;
        }
    }

    cout << '!';
    for (int v = 1; v <= n; ++v) cout << ' ' << answer[v];
    cout << '\n' << flush;
    return 0;
}
