#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    if (!(cin >> n)) return 0;
    char d = '^';
    while (true) {
        cout << d << "\n";
        cout.flush();
        string s;
        if (!(cin >> s)) return 0;
        char c = s[0];
        if (c == 'G') return 0;
        if (c == '^' || c == '<' || c == 'v' || c == '>') {
            d = c;
        } else {
            // '.' or failure response: terminate
            return 0;
        }
    }
}
