#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    const int r = 1;                     // fixed reference index

    long long bestAns = -1;
    vector<pair<int,int>> bestPairs;
    bestPairs.reserve((n-1)*(n-2)/2);

    for (int i = 2; i <= n; ++i) {
        for (int j = i + 1; j <= n; ++j) {
            cout << "? " << r << ' ' << i << ' ' << j << "\n";
            cout.flush();
            long long ans;
            if (!(cin >> ans)) return 0;
            if (ans == -1) return 0;     // error from judge
            if (ans > bestAns) {
                bestAns = ans;
                bestPairs.clear();
                bestPairs.emplace_back(i, j);
            } else if (ans == bestAns) {
                bestPairs.emplace_back(i, j);
            }
        }
    }

    vector<int> cnt(n + 1, 0);
    for (auto &p : bestPairs) {
        ++cnt[p.first];
        ++cnt[p.second];
    }

    int maxCnt = 0;
    for (int i = 2; i <= n; ++i) maxCnt = max(maxCnt, cnt[i]);

    vector<int> cand;
    for (int i = 2; i <= n; ++i)
        if (cnt[i] == maxCnt) cand.push_back(i);

    int iAns, jAns;
    if (cand.size() == 1) {
        iAns = r;
        jAns = cand[0];
    } else {
        // at least two candidates
        if (cnt[cand[0]] == 1) {          // zero and max (both appear once)
            iAns = cand[0];
            jAns = cand[1];
        } else {                          // r is the zero, candidates are maxima
            iAns = r;
            jAns = cand[0];
        }
    }

    cout << "! " << iAns << ' ' << jAns << "\n";
    cout.flush();
    return 0;
}
