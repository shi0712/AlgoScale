#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int M;
    if (!(cin >> M)) return 0;

    vector<int> val(M);
    for (int u = 0; u < M; ++u) {
        val[u] = M - 1 - u;
    }

    for (int u = 0; u < M; ++u) {
        while (true) {
            cout << "? 1 " << u << ' ' << val[u] << '\n';
            cout.flush();

            int r;
            if (!(cin >> r)) return 0;
            if (r == -1) return 0;

            if (r == 0) break;
        }
    }

    cout << "? 2\n";
    cout.flush();

    int N;
    if (!(cin >> N)) return 0;
    if (N == -1) return 0;

    vector<int> seq(N);
    for (int i = 0; i < N; ++i) {
        cin >> seq[i];
        if (seq[i] == -1) return 0;
    }

    vector<int> parent(N, -1);
    vector<int> st;
    st.reserve(N);

    for (int i = 0; i < N; ++i) {
        while (!st.empty() && seq[st.back()] < seq[i]) {
            st.pop_back();
        }

        if (!st.empty()) parent[i] = st.back();
        st.push_back(i);
    }

    vector<int> last(M, -1);
    for (int i = 0; i < N; ++i) {
        int v = seq[i];
        if (0 <= v && v < M) {
            last[v] = i;
        }
    }

    vector<int> label(N, -1);

    label[0] = N - 1;

    for (int u = 0; u < M; ++u) {
        label[last[val[u]]] = u;
    }

    int nxt = M;
    for (int i = 0; i < N; ++i) {
        if (label[i] == -1) {
            label[i] = nxt++;
        }
    }

    vector<int> ans(N, -1);

    for (int i = 0; i < N; ++i) {
        if (parent[i] != -1) {
            ans[label[i]] = label[parent[i]];
        }
    }

    cout << "! " << N;
    for (int i = 0; i < N - 1; ++i) {
        cout << ' ' << ans[i];
    }
    cout << '\n';
    cout.flush();

    return 0;
}
