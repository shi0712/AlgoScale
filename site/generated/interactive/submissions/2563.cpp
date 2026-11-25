#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> deg;

bool query(const string& s, int& res) {
    cout << "? " << s << endl;
    if (!(cin >> res)) return false;
    if (res == -1) return false;
    return true;
}

int getDeg(int i) {
    if (deg[i] != -1) return deg[i];
    string s(n, '0');
    s[i] = '1';
    int res;
    if (!query(s, res)) exit(0);
    if (n > 1 && res == 0) {
        cout << "! 0" << endl;
        exit(0);
    }
    deg[i] = res;
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "! 1" << endl;
        return 0;
    }

    if (n == 2) {
        string s = "10";
        int res;
        if (!query(s, res)) return 0;
        cout << "! " << (res > 0 ? 1 : 0) << endl;
        return 0;
    }

    deg.assign(n, -1);
    vector<int> visited(n, 0);
    queue<int> q;

    visited[0] = 1;
    q.push(0);
    int cnt = 1;

    while (!q.empty() && cnt < n) {
        int u = q.front();
        q.pop();
        int du = getDeg(u);

        for (int v = 0; v < n; ++v) {
            if (visited[v]) continue;

            int dv = getDeg(v);
            string s(n, '0');
            s[u] = '1';
            s[v] = '1';

            int f;
            if (!query(s, f)) return 0;

            if (f < du + dv) {
                visited[v] = 1;
                ++cnt;
                q.push(v);
                if (cnt == n) break;
            }
        }
    }

    cout << "! " << (cnt == n ? 1 : 0) << endl;
    return 0;
}
