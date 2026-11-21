#include <bits/stdc++.h>
using namespace std;

int n, k;

struct DSU {
    vector<int> p, sz;
    void init(int n) {
        p.resize(n + 1);
        sz.assign(n + 1, 1);
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) {
        if (p[x] == x) return x;
        return p[x] = find(p[x]);
    }
    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
    }
} dsu;

int reset_cnt = 0;
vector<int> singleton(1, 0);

void do_reset() {
    cout << "R\n";
    cout.flush();
    ++reset_cnt;
}

char do_query(int c) {
    cout << "? " << c << "\n";
    cout.flush();
    char ans;
    if (!(cin >> ans)) exit(0);
    if (ans == 'y') ans = 'Y';
    if (ans == 'n') ans = 'N';
    if (ans != 'Y' && ans != 'N') exit(0);
    return ans;
}

// A and B contain cafes whose colors are known to be distinct inside A and inside B.
// Returns true iff some color appears in both A and B.
bool test(const vector<int>& A, const vector<int>& B) {
    int a = (int)A.size();
    int b = (int)B.size();
    if (a == 0 || b == 0) return false;

    do_reset();

    // If the whole first set is still in memory for every element of the second set,
    // one pass is enough.
    if (a + b <= k + 1) {
        for (int x : A) do_query(x);
        for (int y : B) {
            if (do_query(y) == 'Y') return true;
        }
        return false;
    }

    // Otherwise use P, Q, P where P is the smaller set.
    // Check Q and the final P.
    if (a <= b) {
        for (int x : A) do_query(x);
        for (int y : B) {
            if (do_query(y) == 'Y') return true;
        }
        for (int x : A) {
            if (do_query(x) == 'Y') return true;
        }
    } else {
        for (int x : B) do_query(x);
        for (int y : A) {
            if (do_query(y) == 'Y') return true;
        }
        for (int x : B) {
            if (do_query(x) == 'Y') return true;
        }
    }
    return false;
}

// Test singleton {x} against A.
bool test_one(const vector<int>& A, int x) {
    singleton[0] = x;
    return test(singleton, A);
}

// Test A against singleton {x}.
bool test_oneB(const vector<int>& A, int x) {
    singleton[0] = x;
    return test(A, singleton);
}

// Assumes there is at least one y in Y with the same color as x.
void find_one_positive(int x, const vector<int>& Y) {
    if (Y.empty()) return;
    if (Y.size() == 1) {
        if (test_one(Y, x)) dsu.unite(x, Y[0]);
        return;
    }

    int mid = (int)Y.size() / 2;
    vector<int> Y1(Y.begin(), Y.begin() + mid);
    vector<int> Y2(Y.begin() + mid, Y.end());

    if (test_one(Y1, x)) {
        find_one_positive(x, Y1);
    } else {
        find_one_positive(x, Y2);
    }
}

// Finds all common colors between X and Y (each side has distinct colors)
// and unions the corresponding representatives.
void find_matching(const vector<int>& X, const vector<int>& Y) {
    if (X.empty() || Y.empty()) return;

    if (X.size() == 1 && Y.size() == 1) {
        if (test(X, Y)) dsu.unite(X[0], Y[0]);
        return;
    }

    if (X.size() == 1) {
        if (test_one(Y, X[0])) find_one_positive(X[0], Y);
        return;
    }

    if (Y.size() == 1) {
        if (test_oneB(X, Y[0])) find_one_positive(Y[0], X);
        return;
    }

    if (!test(X, Y)) return;

    int mx = (int)X.size() / 2;
    int my = (int)Y.size() / 2;

    vector<int> X1(X.begin(), X.begin() + mx);
    vector<int> X2(X.begin() + mx, X.end());
    vector<int> Y1(Y.begin(), Y.begin() + my);
    vector<int> Y2(Y.begin() + my, Y.end());

    find_matching(X1, Y1);
    find_matching(X1, Y2);
    find_matching(X2, Y1);
    find_matching(X2, Y2);
}

vector<int> get_roots(const vector<int>& v) {
    vector<int> r;
    r.reserve(v.size());
    for (int x : v) r.push_back(dsu.find(x));
    sort(r.begin(), r.end());
    r.erase(unique(r.begin(), r.end()), r.end());
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> k)) return 0;
    dsu.init(n);

    int m = n / k;
    vector<vector<int>> reps(m);
    vector<int> all_reps;

    if (k == 1) {
        for (int b = 0; b < m; ++b) {
            int c = b + 1;
            reps[b].push_back(c);
            all_reps.push_back(c);
        }
    } else {
        for (int b = 0; b < m; ++b) {
            do_reset();
            for (int i = 0; i < k; ++i) {
                int c = b * k + i + 1;
                char ans = do_query(c);
                if (ans == 'N') {
                    reps[b].push_back(c);
                    all_reps.push_back(c);
                }
            }
        }
    }

    // Process a block with many representatives first; this often allows skips later.
    int center = 0;
    for (int i = 1; i < m; ++i) {
        if (reps[i].size() > reps[center].size()) center = i;
    }
    swap(reps[0], reps[center]);

    for (int i = 0; i < m; ++i) {
        for (int j = i + 1; j < m; ++j) {
            vector<int> roots_i = get_roots(reps[i]);
            vector<int> roots_j = get_roots(reps[j]);

            vector<int> U, V;
            U.reserve(reps[i].size());
            V.reserve(reps[j].size());

            for (int x : reps[i]) {
                if (!binary_search(roots_j.begin(), roots_j.end(), dsu.find(x)))
                    U.push_back(x);
            }
            for (int y : reps[j]) {
                if (!binary_search(roots_i.begin(), roots_i.end(), dsu.find(y)))
                    V.push_back(y);
            }

            if (!U.empty() && !V.empty()) {
                find_matching(U, V);
            }
        }
    }

    vector<int> roots = get_roots(all_reps);
    cout << "! " << roots.size() << "\n";
    cout.flush();
    return 0;
}
