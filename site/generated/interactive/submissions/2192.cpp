#include <bits/stdc++.h>
using namespace std;

int N, K;
vector<vector<int>> orig, tpq;
bool topMode;

int cmp_judge(int a, int b) {
    cout << "? " << a << ' ' << b << '\n';
    cout.flush();

    int w;
    if (!(cin >> w)) exit(0);
    if (w == -1) exit(0);
    return w;
}

int cmp_best(int a, int b) {
    int w = cmp_judge(a, b);
    if (topMode) return w;
    return (w == a ? b : a);
}

int build_initial(int l, int r) {
    if (l == r) return l;
    int m = (l + r) / 2;
    int a = build_initial(l, m);
    int b = build_initial(m + 1, r);

    int w = cmp_best(a, b);
    int lo = (w == a ? b : a);
    orig[w].push_back(lo);
    return w;
}

int build_tpq_rec(const vector<int>& v, int l, int r) {
    if (r - l == 1) return v[l];
    int m = (l + r) / 2;
    int a = build_tpq_rec(v, l, m);
    int b = build_tpq_rec(v, m, r);

    int w = cmp_best(a, b);
    int lo = (w == a ? b : a);
    tpq[w].push_back(lo);
    return w;
}

int build_tpq(const vector<int>& v) {
    int m = (int)v.size();
    if (m == 0) return -1;
    if (m == 1) return v[0];
    return build_tpq_rec(v, 0, m);
}

vector<int> find_best(int M) {
    orig.assign(N + 1, {});
    tpq.assign(N + 1, {});

    if (M == 0) return {};

    int root = build_initial(1, N);
    vector<int> ans;
    ans.reserve(M);
    ans.push_back(root);

    if (M >= 2) {
        vector<int> v = orig[root];
        int cur = build_tpq(v);

        for (int t = 2; t <= M; ++t) {
            ans.push_back(cur);

            if (t < M) {
                vector<int> u;
                u.reserve(tpq[cur].size() + orig[cur].size());
                for (int x : tpq[cur]) u.push_back(x);
                for (int x : orig[cur]) u.push_back(x);
                cur = build_tpq(u);
            }
        }
    }

    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N >> K)) return 0;

    if (K == N) {
        cout << "!";
        for (int i = 1; i <= N; ++i) cout << ' ' << i;
        cout << '\n';
        cout.flush();
        return 0;
    }

    int M = N - K;

    if (M < K) {
        topMode = false;
        vector<int> bottom = find_best(M);

        vector<char> isBottom(N + 1, 0);
        for (int x : bottom) isBottom[x] = 1;

        cout << "!";
        for (int i = 1; i <= N; ++i) {
            if (!isBottom[i]) cout << ' ' << i;
        }
        cout << '\n';
        cout.flush();
    } else {
        topMode = true;
        vector<int> top = find_best(K);

        cout << "!";
        for (int i = 0; i < K; ++i) cout << ' ' << top[i];
        cout << '\n';
        cout.flush();
    }

    return 0;
}
