#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<long long> pref(n + 1, 0);
    vector<char> known(n + 1, 0);
    known[0] = 1;

    auto getPrefix = [&](int k) -> long long {
        if (known[k]) return pref[k];
        cout << "? 1 " << k << '\n' << flush;
        long long value;
        if (!(cin >> value)) exit(0);
        if (value < 0) exit(0);
        pref[k] = value;
        known[k] = 1;
        return value;
    };

    long long total = getPrefix(n);

    int l = 1, r = n;
    long long sum = total;
    long long answer = 0;

    while (true) {
        int len = r - l + 1;

        if (len == 1) {
            answer = sum;
            break;
        }
        if (sum == len) {
            answer = 1;
            break;
        }
        if (len <= 3) {
            answer = sum / 2;
            break;
        }

        long long base = pref[l - 1];
        long long target = base + sum / 2;

        int lo = l, hi = r - 1;

        for (int k = l; k <= r - 1 && lo < hi; ++k) {
            if (!known[k]) continue;
            if (pref[k] == target) {
                lo = hi = k;
                break;
            }
            if (pref[k] < target) {
                lo = max(lo, k + 1);
            } else {
                hi = min(hi, k - 1);
            }
        }

        while (lo < hi) {
            int mid = (lo + hi) / 2;
            long long value = getPrefix(mid);

            if (value == target) {
                lo = hi = mid;
            } else if (value < target) {
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }

        int p = lo;
        pref[p] = target;
        known[p] = 1;

        int leftLen = p - l + 1;
        int rightLen = r - p;

        if (leftLen <= rightLen) {
            r = p;
        } else {
            l = p + 1;
        }
        sum /= 2;
    }

    cout << "! " << answer << '\n' << flush;
    return 0;
}
