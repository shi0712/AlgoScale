#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> deg;
vector<vector<int>> pairF;
vector<char> inC;
vector<int> C;

int queryString(const string& s) {
    cout << "? " << s << '\n';
    cout.flush();
    int ans;
    if (!(cin >> ans)) exit(0);
    if (ans == -1) exit(0);
    return ans;
}

int getDeg(int v) {
    if (deg[v] != -1) return deg[v];
    string s(n, '0');
    s[v] = '1';
    int ans = queryString(s);
    deg[v] = ans;
    if (ans == 0) {
        cout << "! 0\n";
        cout.flush();
        exit(0);
    }
    return ans;
}

int queryPair(int u, int v) {
    if (pairF[u][v] != -1) return pairF[u][v];
    string s(n, '0');
    s[u] = s[v] = '1';
    int ans = queryString(s);
    pairF[u][v] = pairF[v][u] = ans;
    return ans;
}

int queryCurrentC() {
    if (C.size() == 1) return deg[C[0]];
    if (C.size() == 2) return queryPair(C[0], C[1]);

    string s(n, '0');
    for (int i = 0; i < n; ++i) {
        if (inC[i]) s[i] = '1';
    }
    return queryString(s);
}

int findEdge(bool& fcChecked) {
    const int THRESH = 10;

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        int sa = (deg[a] == -1 ? 0 : deg[a]);
        int sb = (deg[b] == -1 ? 0 : deg[b]);
        if (sa != sb) return sa > sb;
        return a < b;
    });

    int noCount = 0;

    for (int idx = (int)C.size() - 1; idx >= 0; --idx) {
        int c = C[idx];
        int dc = getDeg(c);

        for (int v : order) {
            if (inC[v] || pairF[c][v] != -1) continue;

            int dv = getDeg(v);
            int f = queryPair(c, v);

            if (f < dc + dv) return v;

            ++noCount;
            if (noCount >= THRESH && !fcChecked) {
                int fc = queryCurrentC();
                fcChecked = true;
                if (fc == 0) return -2;
            }
        }
    }

    return -1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "! 1\n";
        cout.flush();
        return 0;
    }

    deg.assign(n, -1);
    getDeg(0);

    pairF.assign(n, vector<int>(n, -1));
    inC.assign(n, 0);

    C.push_back(0);
    inC[0] = 1;

    bool fcChecked = false;

    while (true) {
        if ((int)C.size() == n) {
            cout << "! 1\n";
            cout.flush();
            return 0;
        }

        if (C.size() == 2) {
            int fc = queryCurrentC();
            fcChecked = true;
            if (fc == 0) {
                cout << "! 0\n";
                cout.flush();
                return 0;
            }
        } else if (C.size() > 1 && n - (int)C.size() <= 10) {
            int fc = queryCurrentC();
            fcChecked = true;
            if (fc == 0) {
                cout << "! 0\n";
                cout.flush();
                return 0;
            }
        }

        int v = findEdge(fcChecked);

        if (v < 0) {
            cout << "! 0\n";
            cout.flush();
            return 0;
        }

        inC[v] = 1;
        C.push_back(v);
        fcChecked = false;
    }
}
