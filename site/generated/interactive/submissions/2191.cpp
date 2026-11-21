#include <bits/stdc++.h>
using namespace std;

int n, k;

struct Node {
    int id;
    int sz;
};

struct Compare {
    bool operator()(const Node& a, const Node& b) const {
        return a.sz > b.sz;
    }
};

bool ask(int c) {
    cout << "? " << c << '\n';
    cout.flush();

    string s;
    if (!(cin >> s)) exit(0);
    if (s == "E" || s == "-1") exit(0);

    if (s[0] == 'Y' || s[0] == 'y') return true;
    if (s[0] == 'N' || s[0] == 'n') return false;
    exit(0);
}

void reset() {
    cout << "R\n";
    cout.flush();
}

long long estimate(int M, int a) {
    if (M == 0 || a == 0) return 0;

    int maxp = min(k, M);
    long long best = LLONG_MAX;

    for (int p = 1; p <= maxp; ++p) {
        int q = k + 1 - p;
        int chunks = (a + q - 1) / q;
        long long cost_block = 1LL * chunks * p + a;
        int blocks = (M + p - 1) / p;
        long long total = 1LL * blocks * cost_block;

        if (total < best) best = total;
    }

    return best;
}

int choose_p(int M, int a) {
    int maxp = min(k, M);
    long long best_total = LLONG_MAX;
    long long best_resets = LLONG_MAX;
    int best_p = 1;

    for (int p = 1; p <= maxp; ++p) {
        int q = k + 1 - p;
        int chunks = (a + q - 1) / q;
        long long cost_block = 1LL * chunks * p + a;
        int blocks = (M + p - 1) / p;

        long long total = 1LL * blocks * cost_block;
        long long resets = 1LL * blocks * chunks;

        if (total < best_total ||
            (total == best_total &&
             (resets < best_resets ||
              (resets == best_resets && p > best_p)))) {
            best_total = total;
            best_resets = resets;
            best_p = p;
        }
    }

    return best_p;
}

vector<int> merge_reps(const vector<int>& A, const vector<int>& B) {
    if (A.empty()) return B;
    if (B.empty()) return A;

    long long cost_ab = estimate((int)B.size(), (int)A.size());
    long long cost_ba = estimate((int)A.size(), (int)B.size());

    const vector<int>* O;
    const vector<int>* X;

    if (cost_ab <= cost_ba) {
        X = &A;
        O = &B;
    } else {
        X = &B;
        O = &A;
    }

    int xs = (int)X->size();
    vector<char> found(xs, 0);
    vector<int> pos(n + 1, -1);

    for (int i = 0; i < xs; ++i) {
        pos[(*X)[i]] = i;
    }

    vector<int> active = *X;
    int o_pos = 0;
    int o_size = (int)O->size();

    while (o_pos < o_size && !active.empty()) {
        int M = o_size - o_pos;
        int a = (int)active.size();

        int p = choose_p(M, a);
        vector<int> block;
        block.reserve(p);

        for (int i = 0; i < p; ++i) {
            block.push_back((*O)[o_pos + i]);
        }
        o_pos += p;

        int qmax = k + 1 - p;

        for (int i = 0; i < a; i += qmax) {
            int q = min(qmax, a - i);

            reset();

            for (int idx : block) {
                ask(idx);
            }

            for (int j = 0; j < q; ++j) {
                int idx = active[i + j];
                if (ask(idx)) {
                    found[pos[idx]] = 1;
                }
            }
        }

        vector<int> next_active;
        next_active.reserve(active.size());

        for (int idx : active) {
            if (!found[pos[idx]]) {
                next_active.push_back(idx);
            }
        }

        active.swap(next_active);
    }

    vector<int> res;
    res.reserve(O->size() + xs);

    for (int idx : *O) {
        res.push_back(idx);
    }

    for (int i = 0; i < xs; ++i) {
        if (!found[i]) {
            res.push_back((*X)[i]);
        }
    }

    return res;
}

vector<int> direct_query(int l, int len) {
    if (len == 1) {
        return vector<int>{l};
    }

    reset();
    vector<int> reps;
    reps.reserve(len);

    for (int i = 0; i < len; ++i) {
        int idx = l + i;
        if (!ask(idx)) {
            reps.push_back(idx);
        }
    }

    return reps;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> k)) return 0;

    if (n == 1) {
        cout << "! 1\n";
        cout.flush();
        return 0;
    }

    int B = (k == 1 ? 2 : k);
    if (B > n) B = n;

    vector<vector<int>> pool;
    pool.reserve(2 * n + 5);

    for (int start = 0; start < n; start += B) {
        int len = min(B, n - start);
        pool.push_back(direct_query(start + 1, len));
    }

    priority_queue<Node, vector<Node>, Compare> pq;

    for (int i = 0; i < (int)pool.size(); ++i) {
        pq.push({i, (int)pool[i].size()});
    }

    int next_id = (int)pool.size();

    while (pq.size() > 1) {
        Node a = pq.top();
        pq.pop();

        Node b = pq.top();
        pq.pop();

        vector<int> merged = merge_reps(pool[a.id], pool[b.id]);
        pool.push_back(move(merged));

        pq.push({next_id++, (int)pool.back().size()});
    }

    cout << "! " << pool.back().size() << '\n';
    cout.flush();

    return 0;
}
