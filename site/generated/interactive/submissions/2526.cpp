#include <bits/stdc++.h>
using namespace std;

int M;
vector<int> A, B, posInB, par;

int solve(int a_l, int a_r, int b_r) {
    int root = A[a_l];
    int cur_a = a_l + 1;
    int cur_b_r = b_r;
    while (cur_a <= a_r) {
        int child = A[cur_a];
        int pos = posInB[child];
        int L = cur_b_r - pos + 1;
        solve(cur_a, cur_a + L - 1, cur_b_r);
        par[child] = root;
        cur_a += L;
        cur_b_r = pos - 1;
    }
    return root;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> M;
    if (M == -1) return 0;

    // Phase 1
    vector<int> U1;
    int consecutive_fail = 0;
    int success_count = 0;
    int leaf = 0;
    while (consecutive_fail < M) {
        int val = success_count;
        cout << "? 1 " << leaf << " " << val << endl;
        int res;
        cin >> res;
        if (res == -1) return 0;
        if (res == 1) {
            U1.push_back(leaf);
            success_count++;
            consecutive_fail = 0;
        } else {
            consecutive_fail++;
        }
        leaf = (leaf + 1) % M;
    }
    int N = success_count;

    cout << "? 2" << endl;
    int s;
    cin >> s;
    if (s == -1) return 0;
    A.resize(s);
    for (int i = 0; i < s; i++) cin >> A[i];

    // Phase 2
    consecutive_fail = 0;
    success_count = 0;
    leaf = 0;
    while (consecutive_fail < M) {
        int val = N - 1 - success_count;
        cout << "? 1 " << leaf << " " << val << endl;
        int res;
        cin >> res;
        if (res == -1) return 0;
        if (res == 1) {
            success_count++;
            consecutive_fail = 0;
        } else {
            consecutive_fail++;
        }
        leaf = (leaf + 1) % M;
    }

    cout << "? 2" << endl;
    cin >> s;
    if (s == -1) return 0;
    vector<int> S2(s);
    for (int i = 0; i < s; i++) cin >> S2[i];

    B.resize(s);
    for (int i = 0; i < s; i++) B[i] = N - 1 - S2[i];

    posInB.resize(N);
    for (int i = 0; i < N; i++) posInB[B[i]] = i;

    par.assign(N, -1);
    solve(0, N - 1, N - 1);

    vector<int> L(M, -1);
    for (int v = 0; v < N; v++) {
        int u = U1[v];
        L[u] = v;
    }

    vector<int> mapping(N, -1);
    mapping[0] = N - 1;
    for (int u = 0; u < M; u++) mapping[L[u]] = u;

    int next_label = M;
    for (int i = 0; i < N; i++) {
        if (mapping[i] == -1) mapping[i] = next_label++;
    }

    vector<int> R(N - 1);
    for (int x = 0; x < N; x++) {
        if (x == 0) continue;
        int new_x = mapping[x];
        int new_p = mapping[par[x]];
        R[new_x] = new_p;
    }

    cout << "! " << N;
    for (int i = 0; i < N - 1; i++) cout << " " << R[i];
    cout << endl;

    return 0;
}
