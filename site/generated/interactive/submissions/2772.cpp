#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

constexpr int64 MAX_M = 100000000000000LL;

enum class Reply {
    Lucky,
    Fraudster
};

int64 balance = 1;

Reply ask(int64 x) {
    cout << "? " << x << endl;

    string response;
    if (!(cin >> response)) {
        exit(0);
    }

    if (response == "Lucky!") {
        balance += x;
        return Reply::Lucky;
    }

    if (response == "Fraudster!") {
        balance -= x;
        return Reply::Fraudster;
    }

    // In particular, terminate immediately on "Fired!".
    exit(0);
}

void answer(int64 m) {
    cout << "! " << m << endl;
    exit(0);
}

void safeQuery(int64 x) {
    if (x < 1 || x > MAX_M) {
        exit(0);
    }
    if (ask(x) != Reply::Lucky) {
        exit(0);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (ask(1) == Reply::Fraudster) {
        answer(0);
    }

    // We know M >= 1 and currently have 2 euros.
    if (ask(2) == Reply::Fraudster) {
        answer(1);
    }

    int64 low = 2;
    int64 high = MAX_M;

    // Bootstrap the account while tightening the lower bound.
    while (true) {
        int64 reserve = (low + 3) / 4;
        int64 x = balance - reserve;

        if (x <= 0) {
            exit(0);
        }

        if (x > MAX_M) {
            if (balance > MAX_M) {
                if (ask(MAX_M) == Reply::Lucky) {
                    answer(MAX_M);
                }
                high = MAX_M - 1;
            }
            break;
        }

        if (x <= low) {
            safeQuery(x);
            continue;
        }

        if (ask(x) == Reply::Lucky) {
            if (x == MAX_M) {
                answer(MAX_M);
            }
            low = x;
        } else {
            high = x - 1;
            break;
        }
    }

    int64 n = high - low + 1;
    if (n <= 1) {
        answer(low);
    }

    // cap[w] is the maximum number of candidates distinguishable
    // with weighted budget w, where success costs 1 and failure costs 2.
    vector<int64> cap = {1, 1};
    while (cap.back() < n) {
        int64 sz = static_cast<int64>(cap.size());
        cap.push_back(cap[sz - 1] + cap[sz - 2]);
    }

    int budget = static_cast<int>(cap.size()) - 1;

    while (high - low + 1 > 1) {
        n = high - low + 1;

        int64 failureSize = (budget >= 2 ? cap[budget - 2] : 1);
        int64 d = min(failureSize, n - 1);
        d = max<int64>(d, 1);

        int64 x = low + d;

        // All these queries are at most low, hence are guaranteed to succeed.
        // Keep one euro after a possible fraud so that searching can continue.
        while (balance < x + 1) {
            int64 y = (balance < low ? balance : low);
            if (y < 1) {
                exit(0);
            }
            safeQuery(y);
        }

        if (ask(x) == Reply::Lucky) {
            low = x;
            --budget;
        } else {
            high = x - 1;
            budget -= 2;
        }
    }

    answer(low);
}
