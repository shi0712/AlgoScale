#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "! 1" << endl;
        return 0;
    }

    vector<int> choice(n + 1);
    vector<int> B;
    B.reserve(3);

    for (int i = n; i >= 1; --i) {
        int m = (int)B.size();
        if (m == 0) {
            B.push_back(i);
            choice[i] = 1;
        } else if (m == 1) {
            cout << "? " << i << " " << B[0] << endl;
            char c;
            if (!(cin >> c)) return 0;
            if (c == '<') {
                choice[i] = 1;
                B.insert(B.begin(), i);
            } else {
                choice[i] = 2;
                B.push_back(i);
            }
        } else if (m == 2) {
            cout << "? " << i << " " << B[0] << endl;
            char c;
            if (!(cin >> c)) return 0;
            if (c == '<') {
                choice[i] = 1;
                B = {i, B[0], B[1]};
            } else {
                cout << "? " << i << " " << B[1] << endl;
                if (!(cin >> c)) return 0;
                if (c == '<') {
                    choice[i] = 2;
                    B = {B[0], i, B[1]};
                } else {
                    choice[i] = 3;
                    B = {B[0], B[1], i};
                }
            }
        } else {
            cout << "? " << i << " " << B[0] << endl;
            char c;
            if (!(cin >> c)) return 0;
            if (c == '<') {
                choice[i] = 1;
                B = {i, B[0], B[1]};
            } else {
                cout << "? " << i << " " << B[1] << endl;
                if (!(cin >> c)) return 0;
                if (c == '<') {
                    choice[i] = 2;
                    B = {B[0], i, B[1]};
                } else {
                    choice[i] = 3;
                    B = {B[0], B[1], i};
                }
            }
        }
    }

    vector<int> buf;
    int init = min(3, n);
    for (int v = 1; v <= init; ++v) buf.push_back(v);

    vector<int> p(n + 1);
    for (int i = 1; i <= n; ++i) {
        int r = choice[i];
        int val = buf[r - 1];
        p[i] = val;
        buf.erase(buf.begin() + (r - 1));
        int newv = i + 3;
        if (newv <= n) {
            buf.push_back(newv);
        }
    }

    cout << "!";
    for (int i = 1; i <= n; ++i) cout << " " << p[i];
    cout << endl;
    return 0;
}
