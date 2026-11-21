#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int M;
    if (!(cin >> M)) return 0;

    auto query = [&](int type, int u = 0, int x = 0) {
        if (type == 1) cout << "? 1 " << u << ' ' << x << '\n';
        else cout << "? 2\n";
        cout.flush();
    };

    for (int u = M - 1; u >= 0; --u) {
        while (true) {
            query(1, u, u);
            int r;
            if (!(cin >> r)) return 0;
            if (r == -1) return 0;
            if (r == 0) break;
        }
    }

    query(2);
    int s;
    if (!(cin >> s)) return 0;
    if (s == -1) return 0;

    vector<int> a(s);
    for (int i = 0; i < s; ++i) {
        if (!(cin >> a[i])) return 0;
        if (a[i] == -1) return 0;
    }

    int N = s;
    vector<int> parent(N, -1);
    vector<int> st;

    if (N > 0) {
        st.push_back(0);
        for (int i = 1; i < N; ++i) {
            while (!st.empty() && a[st.back()] < a[i]) st.pop_back();
            parent[i] = st.empty() ? 0 : st.back();
            st.push_back(i);
        }
    }

    vector<int> deg(N, 0);
    for (int i = 1; i < N; ++i) {
        if (parent[i] >= 0) ++deg[parent[i]];
    }

    vector<int> label(N, -1), pos(N, -1);
    int nextInternal = M;

    if (N > 0) {
        label[0] = N - 1;
        pos[N - 1] = 0;
    }

    vector<int> leafSeen(M, -1);
    for (int i = 0; i < N; ++i) {
        if (i == 0) continue;
        if (deg[i] == 0) {
            int v = a[i];
            if (0 <= v && v < M && leafSeen[v] == -1) {
                label[i] = v;
                pos[v] = i;
                leafSeen[v] = i;
            } else {
                if (nextInternal < N - 1) {
                    label[i] = nextInternal++;
                    pos[label[i]] = i;
                }
            }
        }
    }

    for (int i = 0; i < N; ++i) {
        if (label[i] == -1) {
            if (nextInternal < N - 1) {
                label[i] = nextInternal++;
                pos[label[i]] = i;
            } else {
                label[i] = N - 1;
            }
        }
    }

    vector<int> R(max(0, N - 1), 0);
    for (int i = 0; i < N; ++i) {
        int lab = label[i];
        if (lab == N - 1) continue;
        if (lab < 0 || lab >= N) continue;
        int p = parent[i];
        if (p >= 0) R[lab] = label[p];
    }

    cout << "! " << N;
    for (int i = 0; i < N - 1; ++i) cout << ' ' << R[i];
    cout << '\n';
    cout.flush();

    return 0;
}
