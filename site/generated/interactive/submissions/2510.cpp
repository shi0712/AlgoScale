#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using u128 = __uint128_t;

const u64 LIMIT = 100000000000000ULL;
const int TABLE_BALANCE = 100000;
const int MAX_TIME = 96;

vector<u64> capacity_table;

u64& table_entry(int time, int balance) {
    return capacity_table[(size_t)time * (TABLE_BALANCE + 1) + balance];
}

u64 capacity(int time, u64 balance) {
    if (time == 0) return 0;
    if (balance <= TABLE_BALANCE) return table_entry(time, balance);

    u64 estimate = (u64)((u128)table_entry(time, TABLE_BALANCE) * balance /
                         TABLE_BALANCE * balance / TABLE_BALANCE);
    u64 binary_cap = time < 47 ? (1ULL << time) - 1 : LIMIT;
    return min({LIMIT, binary_cap, estimate});
}

void precompute() {
    capacity_table.resize((size_t)(MAX_TIME + 1) * (TABLE_BALANCE + 1));

    for (int time = 1; time <= MAX_TIME; ++time) {
        for (int balance = 1; balance <= TABLE_BALANCE; ++balance) {
            int low = 0, high = balance;
            while (low < high) {
                int middle = (low + high + 1) / 2;
                if ((u64)(middle - 1) <= capacity(time - 1, balance - middle))
                    low = middle;
                else
                    high = middle - 1;
            }
            table_entry(time, balance) =
                min(LIMIT, (u64)low + capacity(time - 1, balance + low));
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    precompute();

    u64 lower = 0, upper = LIMIT + 1, balance = 1;
    int failures = 0;

    while (upper - lower > 1) {
        u64 width = upper - lower - 1;
        u64 bet;

        if (balance >= width) {
            bet = (width + 1) / 2;
        } else {
            int time = 1;
            while (time <= MAX_TIME && capacity(time, balance) < width) ++time;

            if (time > MAX_TIME) {
                bet = max<u64>(1, 3 * balance / 4);
            } else {
                u64 low = 0, high = balance;
                while (low < high) {
                    u64 middle = (low + high + 1) / 2;
                    if (middle - 1 <= capacity(time - 1, balance - middle))
                        low = middle;
                    else
                        high = middle - 1;
                }

                if (time > 20) {
                    int percent = failures == 0 ? 97 : 85;
                    bet = max<u64>(1, low * percent / 100);
                } else {
                    bet = low;
                }
            }
        }

        bet = min({bet, width, balance});
        if (bet == balance && bet > 1) --bet;

        u64 query = lower + bet;
        cout << "? " << query << '\n' << flush;

        string response;
        if (!(cin >> response)) return 0;
        if (response == "Fired!") return 0;

        if (response == "Lucky!") {
            lower = query;
            balance += bet;
        } else if (response == "Fraudster!") {
            upper = query;
            balance -= bet;
            ++failures;
        } else {
            return 0;
        }
    }

    cout << "! " << lower << '\n' << flush;
    return 0;
}
