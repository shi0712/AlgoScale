#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0; // no input

    int countA = 1; // mushroom 0 is known to be species A

    for (int i = 1; i < n; ++i) {
        cout << "? 2 0 " << i << "\n" << flush;
        int resp;
        if (!(cin >> resp)) return 0; // unexpected EOF
        if (resp == -1) return 0;     // judge indicates failure
        // resp is 0 if same species (i is A), 1 if different (i is B)
        if (resp == 0) ++countA;
    }

    cout << "! " << countA << "\n" << flush;
    return 0;
}
