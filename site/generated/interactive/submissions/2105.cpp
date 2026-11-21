#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int N;
    if (!(cin >> N)) return 0;

    vector<int> door_of_switch(N, -1);   // p[j]
    vector<int> bit_of_switch(N, -1);    // c[j]
    vector<int> bit_of_door(N, -1);      // t[i]

    vector<int> unknown;
    unknown.reserve(N);
    for (int i = 0; i < N; ++i) unknown.push_back(i);

    auto ask = [&](const string &s) -> int {
        cout << "? " << s << "\n";
        cout.flush();
        int r;
        if (!(cin >> r)) exit(0);
        return r;
    };

    // main loop over doors
    for (int k = 0; k < N; ++k) {
        // ----- determine t[k] -----
        string s(N, '0');
        for (int j = 0; j < N; ++j) {
            if (door_of_switch[j] != -1) {
                s[j] = char('0' + bit_of_switch[j]);
            } else {
                s[j] = '0';
            }
        }
        int r = ask(s);
        int tk = (r == k) ? 1 : 0;
        bit_of_door[k] = tk;

        // ----- binary search the switch for door k -----
        vector<int> cand = unknown;
        while (cand.size() > 1) {
            int mid = (int)cand.size() / 2;
            vector<int> left(cand.begin(), cand.begin() + mid);
            vector<int> right(cand.begin() + mid, cand.end());

            // mark left half
            vector<char> isLeft(N, 0);
            for (int id : left) isLeft[id] = 1;

            string qs(N, '0');
            for (int j = 0; j < N; ++j) {
                if (door_of_switch[j] != -1) {
                    qs[j] = char('0' + bit_of_switch[j]);
                } else {
                    // unknown switch
                    if (isLeft[j]) qs[j] = char('0' + (1 - tk)); // opposite
                    else          qs[j] = char('0' + tk);       // same as tk
                }
            }
            int rr = ask(qs);
            if (rr == k) {
                cand.swap(left);
            } else {
                cand.swap(right);
            }
        }
        int sw = cand[0];               // the switch controlling door k
        door_of_switch[sw] = k;
        bit_of_switch[sw] = tk;

        // remove sw from unknown
        auto it = find(unknown.begin(), unknown.end(), sw);
        if (it != unknown.end()) {
            *it = unknown.back();
            unknown.pop_back();
        }
    }

    // ----- output final answer -----
    string final_bits(N, '0');
    for (int j = 0; j < N; ++j) final_bits[j] = char('0' + bit_of_switch[j]);

    cout << "! " << final_bits;
    for (int j = 0; j < N; ++j) {
        cout << ' ' << door_of_switch[j];
    }
    cout << "\n";
    cout.flush();
    return 0;
}
