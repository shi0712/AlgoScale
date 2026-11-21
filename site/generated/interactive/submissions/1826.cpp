#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    auto ask = [&](int l, int r) -> int {
        cout << "? " << l << " " << r << endl;
        int y;
        cin >> y;
        if (y == -1) exit(0);
        int len = r - l + 1;
        return (y == len - 1 ? 1 : 0);
    };

    vector<char> active(n + 1, 1);
    vector<unsigned char> last(n + 1, 0);
    vector<unsigned char> runlen(n + 1, 1);
    vector<double> pref(n + 1, 0.0);

    int firstR = max(1, n / 2);
    int a1 = ask(1, firstR);

    for (int x = 1; x <= n; ++x) {
        active[x] = 1;
        runlen[x] = 1;
        int inside = (x <= firstR ? 1 : 0);
        last[x] = (unsigned char)(a1 ^ inside);
    }

    while (true) {
        int A = 0, V = 0, L1 = 0;
        int baseX = 0, baseY = 0;

        for (int x = 1; x <= n; ++x) {
            if (!active[x]) continue;
            ++A;
            if (runlen[x] == 2) {
                ++V;
                if (last[x] == 0) ++baseX;
            } else {
                ++L1;
                if (last[x] == 0) ++baseY;
            }
        }

        if (A <= 2) break;

        double alpha = (V < 2 ? 1.0 : 0.5);
        double targetTotal = (V + alpha * L1) / 2.0;
        double baseC = baseX + alpha * baseY;
        double target = targetTotal - baseC;

        pref[0] = 0.0;
        for (int x = 1; x <= n; ++x) {
            double c = 0.0;
            if (active[x]) {
                double wK = 0.0, wY = 0.0;
                if (runlen[x] == 2) {
                    wK = (last[x] == 1 ? 1.0 : -1.0);
                } else {
                    wY = (last[x] == 1 ? 1.0 : -1.0);
                }
                c = wK + alpha * wY;
            }
            pref[x] = pref[x - 1] + c;
        }

        multiset<pair<double, int>> ms;
        double bestDiff = 1e100;
        int bestL = 1, bestR = 1;

        for (int r = 1; r <= n; ++r) {
            ms.insert({pref[r - 1], r - 1});

            double desired = pref[r] - target;
            auto key = pair<double, int>{desired, -1000000000};

            auto it = ms.lower_bound(key);
            if (it != ms.end()) {
                double sum = pref[r] - it->first;
                double diff = fabs(sum - target);
                if (diff < bestDiff) {
                    bestDiff = diff;
                    bestL = it->second + 1;
                    bestR = r;
                }
            }

            if (it != ms.begin()) {
                --it;
                double sum = pref[r] - it->first;
                double diff = fabs(sum - target);
                if (diff < bestDiff) {
                    bestDiff = diff;
                    bestL = it->second + 1;
                    bestR = r;
                }
            }
        }

        int a = ask(bestL, bestR);

        for (int x = 1; x <= n; ++x) {
            if (!active[x]) continue;
            int inside = (bestL <= x && x <= bestR ? 1 : 0);
            int h = a ^ inside;

            if (runlen[x] == 2) {
                if (last[x] == h) {
                    active[x] = 0;
                } else {
                    last[x] = (unsigned char)h;
                    runlen[x] = 1;
                }
            } else {
                if (last[x] == h) {
                    runlen[x] = 2;
                } else {
                    last[x] = (unsigned char)h;
                    runlen[x] = 1;
                }
            }
        }
    }

    vector<int> cand;
    for (int x = 1; x <= n; ++x) {
        if (active[x]) cand.push_back(x);
    }

    if (cand.empty()) cand.push_back(1);

    if ((int)cand.size() == 1) {
        cout << "! " << cand[0] << endl;
        int resp;
        cin >> resp;
        if (resp == -1) return 0;

        if (resp == 0) {
            int other = (cand[0] == 1 ? 2 : 1);
            cout << "! " << other << endl;
            cin >> resp;
            if (resp == -1) return 0;
        }
    } else {
        cout << "! " << cand[0] << endl;
        int resp;
        cin >> resp;
        if (resp == -1) return 0;

        if (resp == 0) {
            cout << "! " << cand[1] << endl;
            cin >> resp;
            if (resp == -1) return 0;
        }
    }

    cout << "#" << endl;
    return 0;
}
