#include <bits/stdc++.h>
using namespace std;

int qcnt = 0;

int ask(int p, int e) {
    printf("? %d %d\n", p, e);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r == -1) exit(0);
    qcnt++;
    return r;
}

int main() {
    int m;
    if (scanf("%d", &m) != 1) return 0;
    vector<int> q; // q[i] = query id for the set R_i (q[0] = 0, all edges)
    q.push_back(0);
    vector<int> reps;           // reps[i-1] is the i-th chain edge
    vector<int> cls(m + 1, -1); // index of rep class (0-based in reps)
    vector<int> cnt;
    vector<char> bridge(m + 1, 0);
    for (int e = 1; e <= m; e++) {
        int k = reps.size();
        int r = ask(q[k], e);
        if (r == 1) {
            q.push_back(qcnt);
            reps.push_back(e);
            cls[e] = reps.size() - 1;
            cnt.push_back(1);
            continue;
        }
        // answer 0: bridge or conflict with some rep
        int b = ask(0, e);
        if (b == 0) {
            bridge[e] = 1;
            continue;
        }
        int lo = 0, hi = k; // f(lo)=1, f(hi)=0
        while (hi - lo > 1) {
            int mid = (lo + hi) / 2;
            int a = ask(q[mid], e);
            if (a == 0) hi = mid;
            else lo = mid;
        }
        // conflicts with reps[hi-1]
        int c = hi - 1;
        cls[e] = c;
        cnt[c]++;
    }
    printf("!");
    for (int e = 1; e <= m; e++) {
        int v;
        if (bridge[e]) v = -1;
        else {
            int s = cnt[cls[e]];
            v = s <= 14 ? s : 0;
        }
        printf(" %d", v);
    }
    printf("\n");
    fflush(stdout);
    int res;
    if (scanf("%d", &res) != 1) return 0;
    return 0;
}
