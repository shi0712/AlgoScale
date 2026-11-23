#include <bits/stdc++.h>
using namespace std;

int n;
int r0;

int ask(const string& s) {
    cout << s << "\n" << flush;
    int r;
    if (!(cin >> r)) exit(0);
    if (r == -1 || r == n) exit(0);
    return r;
}

// returns number of T in subset S
int cntT(const vector<int>& S) {
    string s(n, 'F');
    for (int i : S) s[i] = 'T';
    int r = ask(s);
    int d = r - r0 + (int)S.size();
    return d / 2;
}

int main() {
    cin >> n;
    string allF(n, 'F');
    r0 = ask(allF);
    string ans(n, 'F');
    int foundF = 0;
    for (int i = 0; i < n; i += 3) {
        int k = min(3, n - i);
        bool last = (i + k == n);
        int remF = r0 - foundF;
        if (last) {
            if (remF == 0) {
                for (int j = 0; j < k; j++) ans[i + j] = 'T';
                break;
            }
            if (remF == k) {
                break;
            }
        }
        if (k == 1) {
            // last block necessarily (n-i==1)
            ans[i] = remF ? 'F' : 'T';
            break;
        }
        if (k == 2) {
            int x = cntT({i});
            ans[i] = x ? 'T' : 'F';
            if (!x) foundF++;
            if (last) {
                int rf = remF - (x ? 0 : 1);
                ans[i + 1] = rf ? 'F' : 'T';
            } else {
                int y = cntT({i + 1});
                ans[i + 1] = y ? 'T' : 'F';
                if (!y) foundF++;
            }
            continue;
        }
        // k == 3
        int a = cntT({i, i + 1});
        int b = cntT({i + 1, i + 2});
        int x1, x2, x3;
        if (a == 0 && b == 0) { x1 = 0; x2 = 0; x3 = 0; }
        else if (a == 0 && b == 1) { x1 = 0; x2 = 0; x3 = 1; }
        else if (a == 1 && b == 0) { x1 = 1; x2 = 0; x3 = 0; }
        else if (a == 1 && b == 2) { x1 = 0; x2 = 1; x3 = 1; }
        else if (a == 2 && b == 1) { x1 = 1; x2 = 1; x3 = 0; }
        else if (a == 2 && b == 2) { x1 = 1; x2 = 1; x3 = 1; }
        else {
            // a==1,b==1: 010 or 101
            if (last) {
                // 010 has 2 F, 101 has 1 F
                if (remF == 2) { x1 = 0; x2 = 1; x3 = 0; }
                else { x1 = 1; x2 = 0; x3 = 1; }
            } else {
                int x = cntT({i});
                if (x) { x1 = 1; x2 = 0; x3 = 1; }
                else { x1 = 0; x2 = 1; x3 = 0; }
            }
        }
        int xs[3] = {x1, x2, x3};
        for (int j = 0; j < 3; j++) {
            ans[i + j] = xs[j] ? 'T' : 'F';
            if (!xs[j]) foundF++;
        }
    }
    ask(ans);
    return 0;
}
