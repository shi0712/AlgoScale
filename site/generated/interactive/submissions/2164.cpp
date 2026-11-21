#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int64 ask(int l, int r) {
    cout << "? " << l << ' ' << r << '\n' << flush;
    int64 x;
    if (!(cin >> x)) exit(0);
    if (x == -1) exit(0);
    return x;
}

int64 highest_power_of_two(int64 x) {
    if (x <= 0) return 0;
    return 1LL << (63 - __builtin_clzll((unsigned long long)x));
}

int64 max_from_pair_sum(int64 s) {
    if (s <= 0) return 0;
    if (s == 1) return 1; // safety, a real pair sum is at least 2
    if ((s & (s - 1)) == 0) return s / 2;
    return highest_power_of_two(s);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    int64 total = ask(1, n);

    if (n == 1) {
        cout << "! " << total << '\n' << flush;
        return 0;
    }

    int64 avg = (total + n - 1) / n;
    int64 best = 1;
    while (best < avg) best <<= 1;

    auto can_stop = [&](int64 rem, int64 rem_sum) -> bool {
        if (rem <= 0) return true;
        int64 ub = rem_sum - rem + 1; // maximum possible value among remaining elements
        if (ub < 2 * best) return true;
        return false;
    };

    if (can_stop(n, total)) {
        cout << "! " << best << '\n' << flush;
        return 0;
    }

    struct Unit {
        int l, r;
        bool single;
    };

    vector<Unit> units;
    if (n % 2 == 0) {
        for (int i = 1; i <= n; i += 2) {
            units.push_back({i, i + 1, false});
        }
    } else {
        for (int i = 1; i <= n - 1; i += 2) {
            units.push_back({i, i + 1, false});
        }
        units.push_back({n, n, true});
    }

    int L = 0, R = (int)units.size() - 1;
    int64 queried_sum = 0;
    int queried_cnt = 0;
    bool stopped = false;

    auto process = [&](const Unit& u, int64 s) {
        queried_sum += s;
        queried_cnt += (u.r - u.l + 1);
        if (u.single) {
            if (s > best) best = s;
        } else {
            int64 m = max_from_pair_sum(s);
            if (m > best) best = m;
        }
    };

    while (L < R) {
        int64 s = ask(units[L].l, units[L].r);
        process(units[L], s);

        int64 rem = n - queried_cnt;
        int64 rem_sum = total - queried_sum;
        if (can_stop(rem, rem_sum)) {
            stopped = true;
            break;
        }

        ++L;

        if (L < R) {
            s = ask(units[R].l, units[R].r);
            process(units[R], s);

            rem = n - queried_cnt;
            rem_sum = total - queried_sum;
            if (can_stop(rem, rem_sum)) {
                stopped = true;
                break;
            }

            --R;
        }
    }

    if (!stopped && L == R) {
        int64 s = total - queried_sum;
        process(units[L], s);
    }

    cout << "! " << best << '\n' << flush;
    return 0;
}
