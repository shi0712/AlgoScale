#include <bits/stdc++.h>
using namespace std;

int n;

// Returns the closed-neighbourhood size c(S) = f(S) + |S|, or exits on a failure response.
int closedQuery(const vector<char>& in) {
    string s(n, '0');
    int sz = 0;
    for (int i = 0; i < n; i++) {
        if (in[i]) {
            s[i] = '1';
            sz++;
        }
    }
    cout << "? " << s << endl;
    int f;
    if (!(cin >> f)) exit(0);
    if (f < 0) exit(0);
    return f + sz;
}

void answer(int x) {
    cout << "! " << x << endl;
    exit(0);
}

int main() {
    if (!(cin >> n)) return 0;
    if (n == 1) answer(1);

    vector<char> inS(n, 0);
    vector<int> cv(n, -1);
    inS[0] = 1;
    int cS = closedQuery(inS);
    cv[0] = cS;
    if (cS == n) answer(1);
    if (cS == 1) answer(0);

    vector<int> D;
    for (int i = 1; i < n; i++) D.push_back(i);

    while (true) {
        vector<int> nd;
        for (size_t idx = 0; idx < D.size(); idx++) {
            if (cS == n) answer(1);
            int v = D[idx];
            inS[v] = 1;
            int q = closedQuery(inS);
            int gain = q - cS;
            bool ok = false;
            if (gain == 0) {
                ok = true;
            } else {
                if (cv[v] < 0) {
                    vector<char> one(n, 0);
                    one[v] = 1;
                    cv[v] = closedQuery(one);
                }
                if (cv[v] == 1) answer(0);
                int overlap = cS + cv[v] - q;
                if (overlap > 0) ok = true;
            }
            if (ok) {
                cS = q;
            } else {
                inS[v] = 0;
                nd.push_back(v);
            }
        }
        if (cS == n) answer(1);
        if (nd.empty()) answer(1);
        // group test
        vector<char> dset(n, 0), uni(inS);
        for (int v : nd) {
            dset[v] = 1;
            uni[v] = 1;
        }
        int cD = closedQuery(dset);
        int cU = closedQuery(uni);
        if (cS + cD - cU == 0) answer(0);
        D = nd;
    }
    return 0;
}
