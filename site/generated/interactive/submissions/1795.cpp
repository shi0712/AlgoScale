#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    auto ask = [&](int i, int j, int k) -> long long {
        cout << "? " << i << ' ' << j << ' ' << k << endl;
        long long res;
        cin >> res;
        if (res == -1) exit(0);
        return res;
    };

    const int S = 5;
    long long qq[S][S][S];
    for (int i = 0; i < S; ++i)
        for (int j = 0; j < S; ++j)
            for (int k = 0; k < S; ++k)
                qq[i][j][k] = -1;

    for (int i = 0; i < S; ++i) {
        for (int j = i + 1; j < S; ++j) {
            for (int k = j + 1; k < S; ++k) {
                long long res = ask(i + 1, j + 1, k + 1);
                qq[i][j][k] = res;
            }
        }
    }

    auto getQ = [&](int a, int b, int c) -> long long {
        int v[3] = {a, b, c};
        sort(v, v + 3);
        return qq[v[0]][v[1]][v[2]];
    };

    vector<long long> c(n + 1);
    vector<int> perm(S);
    iota(perm.begin(), perm.end(), 0);

    bool found = false;
    do {
        long long q012 = getQ(perm[0], perm[1], perm[2]);
        long long q014 = getQ(perm[0], perm[1], perm[4]);
        long long q234 = getQ(perm[2], perm[3], perm[4]);
        long long q013 = getQ(perm[0], perm[1], perm[3]);
        long long q123 = getQ(perm[1], perm[2], perm[3]);

        long long sum = q012 + q014 - q234;
        if (sum % 2 != 0) continue;

        long long a0 = sum / 2;
        long long a2 = q012 - a0;
        long long a4 = q014 - a0;
        long long a3 = q013 - a0;
        long long a1 = q123 - a3;

        long long aval[5] = {a0, a1, a2, a3, a4};

        bool ok = true;
        for (int i = 1; i < 5; ++i)
            if (aval[i - 1] > aval[i]) ok = false;

        for (int i = 0; i < 5; ++i)
            if (aval[i] < 0 || aval[i] > 1000000000LL) ok = false;

        if (!ok) continue;

        long long localVal[5];
        for (int r = 0; r < 5; ++r)
            localVal[perm[r]] = aval[r];

        for (int i = 0; i < 5 && ok; ++i) {
            for (int j = i + 1; j < 5 && ok; ++j) {
                for (int k = j + 1; k < 5 && ok; ++k) {
                    long long mn = min({localVal[i], localVal[j], localVal[k]});
                    long long mx = max({localVal[i], localVal[j], localVal[k]});
                    if (mn + mx != getQ(i, j, k)) ok = false;
                }
            }
        }

        if (ok) {
            for (int r = 0; r < 5; ++r)
                c[perm[r] + 1] = aval[r];
            found = true;
            break;
        }
    } while (next_permutation(perm.begin(), perm.end()));

    vector<pair<long long, int>> known;
    for (int i = 1; i <= 5; ++i)
        known.push_back({c[i], i});
    sort(known.begin(), known.end());

    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());

    for (int i = 6; i <= n; ++i) {
        int dup = -1;
        for (int p = 0; p + 1 < (int)known.size(); ++p) {
            if (known[p].first == known[p + 1].first) {
                dup = p;
                break;
            }
        }

        long long x;

        if (dup != -1) {
            long long v = known[dup].first;
            int id1 = known[dup].second;
            int id2 = known[dup + 1].second;
            long long q = ask(id1, id2, i);
            x = q - v;
        } else {
            int K = (int)known.size();
            int g = (int)(rng() % (K - 1));

            long long a = known[g].first;
            long long b = known[g + 1].first;
            int ida = known[g].second;
            int idb = known[g + 1].second;

            long long q = ask(ida, idb, i);

            if (q == a + b) {
                int leftCnt = g + 1;

                if (leftCnt >= 2) {
                    long long lowVal = known[0].first;
                    int id1 = known[0].second;
                    int id2 = known[1].second;
                    long long q2 = ask(id1, id2, i);
                    x = q2 - lowVal;
                } else {
                    int K2 = (int)known.size();
                    long long highVal = known[K2 - 1].first;
                    int id1 = known[K2 - 2].second;
                    int id2 = known[K2 - 1].second;
                    long long q2 = ask(id1, id2, i);
                    x = q2 - highVal;
                }
            } else if (q < a + b) {
                x = q - b;
            } else {
                x = q - a;
            }
        }

        c[i] = x;
        known.insert(lower_bound(known.begin(), known.end(), make_pair(x, i)),
                     make_pair(x, i));
    }

    cout << "!";
    for (int i = 1; i <= n; ++i) cout << ' ' << c[i];
    cout << endl;

    return 0;
}
