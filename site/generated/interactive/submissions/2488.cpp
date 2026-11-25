#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n <= 2) {
        cout << "!\n" << n << '\n';
        for (int i = 0; i < n; ++i) {
            if (i) cout << ' ';
            cout << i;
        }
        cout << '\n' << flush;
        return 0;
    }

    vector<int> r(n, -1);
    for (int j = 1; j < n; ++j) {
        cout << "? 0 " << j << '\n' << flush;
        if (!(cin >> r[j])) return 0;
    }

    vector<int> candidates;
    vector<int> b(n);
    for (int x = 0; x < n; ++x) {
        vector<bool> seen(n, false);
        bool valid = true;
        for (int j = 1; j < n; ++j) {
            b[j] = x ^ r[j];
            if (b[j] >= n || seen[b[j]]) {
                valid = false;
                break;
            }
            seen[b[j]] = true;
        }
        if (!valid) continue;
        for (int v = 0; v < n; ++v) {
            if (!seen[v]) {
                b[0] = v;
                break;
            }
        }
        if (b[x] == 0) candidates.push_back(x);
    }

    int x = candidates[0];
    if (candidates.size() > 1) {
        vector<vector<int>> d(candidates.size(), vector<int>(n));
        vector<int> p(n);
        for (int c = 0; c < (int)candidates.size(); ++c) {
            x = candidates[c];
            vector<bool> seen(n, false);
            for (int j = 1; j < n; ++j) {
                b[j] = x ^ r[j];
                seen[b[j]] = true;
            }
            for (int v = 0; v < n; ++v) {
                if (!seen[v]) {
                    b[0] = v;
                    break;
                }
            }
            for (int j = 0; j < n; ++j) p[b[j]] = j;
            for (int i = 0; i < n; ++i) d[c][i] = p[i] ^ x;
        }

        vector<int> active(candidates.size());
        for (int c = 0; c < (int)candidates.size(); ++c) active[c] = c;
        while (true) {
            int bestI = -1, bestWorst = active.size() + 1, bestParts = 0;
            for (int i = 1; i < n; ++i) {
                unordered_map<int, int> parts;
                parts.reserve(active.size() * 2);
                int worst = 0;
                for (int c : active) {
                    int size = ++parts[d[c][i]];
                    worst = max(worst, size);
                }
                if (parts.size() > 1 && (worst < bestWorst ||
                    (worst == bestWorst && (int)parts.size() > bestParts))) {
                    bestI = i;
                    bestWorst = worst;
                    bestParts = parts.size();
                }
            }
            if (bestI == -1) break;

            cout << "? " << bestI << " 1\n" << flush;
            int answer;
            if (!(cin >> answer)) return 0;
            answer ^= r[1];
            vector<int> next;
            for (int c : active) {
                if (d[c][bestI] == answer) next.push_back(c);
            }
            active.swap(next);
        }
        x = candidates[active[0]];
    }

    for (int j = 1; j < n; ++j) b[j] = x ^ r[j];
    if (r[0] == -1) {
        vector<bool> seen(n, false);
        for (int j = 1; j < n; ++j) seen[b[j]] = true;
        for (int v = 0; v < n; ++v) {
            if (!seen[v]) {
                b[0] = v;
                break;
            }
        }
    }
    vector<int> p(n);
    for (int j = 0; j < n; ++j) p[b[j]] = j;

    int count = 0;
    for (int t = 0; t < n; ++t) {
        bool valid = true;
        for (int i = 0; i < n; ++i) {
            if ((i ^ t) >= n || p[i ^ t] != (p[i] ^ t)) {
                valid = false;
                break;
            }
        }
        count += valid;
    }

    cout << "!\n" << count << '\n';
    for (int i = 0; i < n; ++i) {
        if (i) cout << ' ';
        cout << p[i];
    }
    cout << '\n' << flush;
    return 0;
}
