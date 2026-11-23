#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int cnt;
    if (!(cin >> cnt) || cnt < 0) return 0;

    int remaining = cnt;
    long long answer = 0;
    int i = 0;

    while (remaining--) {
        cout << "- " << (1LL << i) << '\n' << flush;

        int newCnt;
        if (!(cin >> newCnt) || newCnt < 0) return 0;

        if (newCnt < cnt) {
            answer |= 1LL << i;
            ++i;
        } else {
            int j = i + (newCnt - cnt) + 1;
            answer |= 1LL << j;
            i = j;
        }
        cnt = newCnt;
    }

    cout << "! " << answer << '\n' << flush;
    return 0;
}
