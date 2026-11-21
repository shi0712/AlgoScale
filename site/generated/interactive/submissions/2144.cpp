#include <bits/stdc++.h>
using namespace std;

struct Comparator {
    unordered_map<unsigned long long, char> cache;
    bool cmp(int i, int j) {
        if (i == j) return false;
        unsigned long long key = ((unsigned long long)i << 32) | (unsigned long long)j;
        auto it = cache.find(key);
        if (it != cache.end()) {
            return it->second == 1;
        }
        cout << "? " << i << " " << j << "\n";
        cout.flush();
        string resp;
        if (!(cin >> resp)) exit(0);
        if (!resp.empty() && resp[0] == '-') exit(0);
        bool less = (resp[0] == '<');
        cache[key] = less ? 1 : 0;
        unsigned long long revKey = ((unsigned long long)j << 32) | (unsigned long long)i;
        cache[revKey] = less ? 0 : 1;
        return less;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 1);
    Comparator comp;
    comp.cache.reserve((size_t)n * 20);
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        return comp.cmp(a, b);
    });
    vector<int> ans(n + 1);
    for (int rank = 0; rank < n; ++rank) {
        ans[idx[rank]] = rank + 1;
    }
    cout << "!";
    for (int i = 1; i <= n; ++i) {
        cout << " " << ans[i];
    }
    cout << "\n";
    cout.flush();
    return 0;
}
