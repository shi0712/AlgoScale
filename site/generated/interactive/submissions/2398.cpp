#include <bits/stdc++.h>
using namespace std;

int n;

string query(const vector<int>& d) {
    string out = "?";
    for (int i = 0; i < n; i++) {
        out += " " + to_string(d[i]);
    }
    out += "\n";
    fputs(out.c_str(), stdout);
    fflush(stdout);
    string s;
    if (!(cin >> s)) exit(0);
    if (s == "-1") exit(0);
    return s;
}

int main() {
    if (!(cin >> n)) return 0;
    vector<int> depth(n, 0);
    // Phase 1: BFS depths from vertex 0 via ball queries
    int known = 1;
    for (int k = 1; known < n; k++) {
        vector<int> d(n, 0);
        d[0] = k;
        string s = query(d);
        for (int i = 1; i < n; i++) {
            if (s[i] == '1' && depth[i] == 0) {
                depth[i] = k;
                known++;
            }
        }
        if (k > n + 5) break;
    }
    int maxd = 0;
    for (int i = 0; i < n; i++) maxd = max(maxd, depth[i]);
    vector<vector<int>> layers(maxd + 1);
    vector<int> pos(n, 0);
    for (int i = 0; i < n; i++) {
        pos[i] = layers[depth[i]].size();
        layers[depth[i]].push_back(i);
    }
    vector<int> pp(n, 0);
    for (int c = 0; c < 3; c++) {
        int m = 0;
        for (int L = c; L + 1 <= maxd; L += 3) {
            m = max(m, (int)layers[L].size());
        }
        int bits = 0;
        while ((1 << bits) < m) bits++;
        for (int b = 0; b < bits; b++) {
            vector<int> d(n, 0);
            for (int v = 0; v < n; v++) {
                if (depth[v] % 3 == c && ((pos[v] >> b) & 1)) d[v] = 1;
            }
            string s = query(d);
            for (int v = 0; v < n; v++) {
                if (depth[v] >= 1 && (depth[v] - 1) % 3 == c && s[v] == '1') {
                    pp[v] |= (1 << b);
                }
            }
        }
    }
    string out = "!\n";
    for (int v = 1; v < n; v++) {
        int p = layers[depth[v] - 1][pp[v]];
        out += to_string(p + 1) + " " + to_string(v + 1) + "\n";
    }
    fputs(out.c_str(), stdout);
    fflush(stdout);
    return 0;
}
