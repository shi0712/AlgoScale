#include <bits/stdc++.h>
using namespace std;

int main() {
    int M;
    if (scanf("%d", &M) != 1) return 0;
    for (int i = 0; i < M; i++) {
        int V = M - 1 - i;
        while (true) {
            printf("? 1 %d %d\n", i, V);
            fflush(stdout);
            int r;
            if (scanf("%d", &r) != 1) return 0;
            if (r == -1) return 0;
            if (r == 0) break;
        }
    }
    printf("? 2\n");
    fflush(stdout);
    int s;
    if (scanf("%d", &s) != 1) return 0;
    if (s == -1) return 0;
    vector<int> val(s);
    for (int i = 0; i < s; i++) {
        if (scanf("%d", &val[i]) != 1) return 0;
        if (val[i] == -1) return 0;
    }
    int N = s;
    vector<int> par(N, -1), childCnt(N, 0);
    vector<int> st;
    st.push_back(0);
    for (int i = 1; i < N; i++) {
        while (!st.empty() && val[st.back()] < val[i]) st.pop_back();
        if (st.empty()) {
            // should not happen
            st.push_back(0);
        }
        par[i] = st.back();
        childCnt[par[i]]++;
        st.push_back(i);
    }
    vector<int> label(N, -1);
    int nxt = M;
    for (int i = 0; i < N; i++) {
        if (i == 0) {
            label[i] = N - 1;
        } else if (childCnt[i] == 0) {
            label[i] = M - 1 - val[i];
        } else {
            label[i] = nxt++;
        }
    }
    vector<int> R(N - 1, 0);
    for (int i = 1; i < N; i++) {
        int l = label[i];
        if (l >= 0 && l < N - 1) R[l] = label[par[i]];
    }
    printf("! %d", N);
    for (int i = 0; i < N - 1; i++) printf(" %d", R[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}
