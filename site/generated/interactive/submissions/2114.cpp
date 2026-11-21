#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;          // read grid size, not used further

    char dir = '^';                     // centre footprint is north
    while (true) {
        cout << dir << "\n" << flush;   // send move and flush

        string resp;
        if (!(cin >> resp)) return 0;   // judge closed the stream

        if (resp == "G") break;         // treasure found
        if (resp == "-1") return 0;     // error, terminate

        // resp should be one of '^', 'v', '<', '>'
        if (resp.size() == 1 && (resp[0] == '^' || resp[0] == 'v' ||
                                 resp[0] == '<' || resp[0] == '>')) {
            dir = resp[0];               // follow the next footprint
        } else {
            // Unexpected reply (e.g., '.'), but this cannot happen on the correct path.
            // Terminate to avoid undefined behaviour.
            return 0;
        }
    }
    return 0;
}
