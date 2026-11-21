#include <bits/stdc++.h>
using namespace std;

int n;

// send a query for the set described by the boolean vector 'in'
// returns the integer answer, exits on -1
int ask(const vector<bool> &in) {
    string s(n, '0');
    for (int i = 0; i < n; ++i) if (in[i]) s[i] = '1';
    cout << "? " << s << "\n" << flush;
    int resp;
    if (!(cin >> resp)) exit(0);
    if (resp == -1) exit(0);
    return resp;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;

    // degrees of all vertices
    vector<int> deg(n);
    vector<bool> tmp(n, false);
    for (int i = 0; i < n; ++i) {
        fill(tmp.begin(), tmp.end(), false);
        tmp[i] = true;
        deg[i] = ask(tmp);
    }

    // component of vertex 0
    vector<bool> inComp(n, false);
    inComp[0] = true;
    int a = ask(inComp);               // query(C)

    while (true) {
        vector<int> toAdd;
        for (int u = 0; u < n; ++u) if (!inComp[u]) {
            vector<bool> set = inComp;
            set[u] = true;
            int b = ask(set);
            int diff = b - a;
            if (diff < deg[u]) {
                toAdd.push_back(u);
            }
        }
        if (toAdd.empty()) break;
        for (int v : toAdd) inComp[v] = true;
        a = ask(inComp);               // recompute for the enlarged set
    }

    int compSize = 0;
    for (bool x : inComp) if (x) ++compSize;
    cout << "! " << (compSize == n ? 1 : 0) << "\n" << flush;
    return 0;
}
