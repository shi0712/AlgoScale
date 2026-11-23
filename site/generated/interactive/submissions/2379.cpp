#include <bits/stdc++.h>
using namespace std;

int n;

int ask(const vector<int>& v) {
    printf("? %d", (int)v.size());
    for (int x : v) printf(" %d", x);
    printf("\n");
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    vector<int> A, B;
    A.push_back(0);
    int T = max(2, (int)llround(sqrt((double)n)));
    int next = 1;
    while (next < n && (int)max(A.size(), B.size()) < T) {
        bool useA;
        vector<int>* S = nullptr;
        if (A.size() >= B.size()) { S = &A; useA = true; }
        else { S = &B; useA = false; }
        if (S->size() >= 2 && n - next >= 2) {
            int x = next, y = next + 1;
            next += 2;
            vector<int> q = {(*S)[0], x, (*S)[1], y};
            int r = ask(q);
            bool xd = r >= 2;
            bool yd = r & 1;
            // diff means other species than S
            bool xA = useA ? !xd : xd;
            bool yA = useA ? !yd : yd;
            (xA ? A : B).push_back(x);
            (yA ? A : B).push_back(y);
        } else {
            int x = next++;
            vector<int> q = {0, x};
            int r = ask(q);
            if (r == 1) B.push_back(x);
            else A.push_back(x);
        }
    }
    long long cntA = A.size();
    while (next < n) {
        bool useA = A.size() >= B.size();
        vector<int>& S = useA ? A : B;
        int rem = n - next;
        int r = min(rem, (int)S.size());
        vector<int> q;
        q.reserve(2 * r);
        for (int i = 0; i < r; i++) {
            q.push_back(S[i]);
            q.push_back(next + i);
        }
        int res = ask(q);
        int diff = (res + (res & 1)) / 2;
        bool lastDiff = res & 1;
        int last = next + r - 1;
        if (useA) cntA += r - diff;
        else cntA += diff;
        bool lastA = useA ? !lastDiff : lastDiff;
        if (lastA) A.push_back(last);
        else B.push_back(last);
        next += r;
    }
    // cntA counted A.size() at phase-1 end; batch additions of last elements to A
    // were already counted via diff, so recompute carefully:
    // we added last to sets after counting it, so cntA must not double count.
    printf("! %lld\n", cntA);
    fflush(stdout);
    return 0;
}
