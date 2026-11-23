#include <bits/stdc++.h>
using namespace std;

int N, K;
bool flipMode = false;
vector<int> win;

int cmpBest(int a, int b) {
    cout << "? " << a << " " << b << endl;
    int w;
    if (!(cin >> w)) exit(0);
    if (w < 0) exit(0);
    if (!flipMode) return w;
    return (w == a) ? b : a;
}

int build(int node, int l, int r) {
    if (l == r) {
        win[node] = l + 1;
        return win[node];
    }
    int m = (l + r) / 2;
    int a = build(node * 2, l, m);
    int b = build(node * 2 + 1, m + 1, r);
    win[node] = cmpBest(a, b);
    return win[node];
}

void upd(int node, int l, int r, int pos) {
    if (l == r) {
        win[node] = -1;
        return;
    }
    int m = (l + r) / 2;
    if (pos <= m) upd(node * 2, l, m, pos);
    else upd(node * 2 + 1, m + 1, r, pos);
    int a = win[node * 2], b = win[node * 2 + 1];
    if (a == -1) win[node] = b;
    else if (b == -1) win[node] = a;
    else win[node] = cmpBest(a, b);
}

int main() {
    cin >> N >> K;
    int T = K;
    if (N - K < K) {
        flipMode = true;
        T = N - K;
    }
    vector<int> rec;
    if (T > 0) {
        win.assign(4 * N + 4, -1);
        build(1, 0, N - 1);
        for (int i = 0; i < T; i++) {
            int w = win[1];
            rec.push_back(w);
            if (i + 1 < T) upd(1, 0, N - 1, w - 1);
        }
    }
    vector<int> ans;
    if (!flipMode) {
        ans = rec;
    } else {
        vector<bool> in(N + 1, false);
        for (int x : rec) in[x] = true;
        for (int i = 1; i <= N; i++) if (!in[i]) ans.push_back(i);
    }
    cout << "!";
    for (int x : ans) cout << " " << x;
    cout << endl;
    return 0;
}
