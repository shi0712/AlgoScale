#include <bits/stdc++.h>
using namespace std;

int readInt() {
    int x;
    if (!(cin >> x) || x == -1) exit(0);
    return x;
}

int M, B = 0, known = 0, root = -1;
vector<int> par, representative, lengthOf, pending;

int put(int leaf, int value) {
    B = max(B, value);
    cout << "? 1 " << leaf << ' ' << value << endl;
    return readInt();
}

vector<int> collect() {
    cout << "? 2" << endl;
    vector<int> a(readInt());
    for (int &x : a) x = readInt();
    return a;
}

int addNode(int leaf, int parent, bool last) {
    int u = leaf;
    if (!last) {
        u = (int)par.size();
        par.push_back(-1);
        representative.push_back(leaf);
        pending.push_back(0);
    }
    par[u] = parent;
    if (parent == -1) root = u;
    return u;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    M = readInt();
    par.assign(M, -1);
    pending.assign(M, 0);
    representative.resize(M);
    iota(representative.begin(), representative.end(), 0);
    lengthOf.assign(M, 0);

    while (known < M) {
        // pending[u] counts unclassified vertices whose deepest known
        // ancestor is u. Sometimes their placement is already forced.
        if (known) {
            int only = -1;
            for (int i = M; i < (int)par.size(); ++i)
                if (pending[i] >= lengthOf[known]) {
                    if (only != -1) { only = -2; break; }
                    only = i;
                }
            if (only >= 0 && (lengthOf[known] == 1 ||
                              pending[only] == lengthOf[known])) {
                pending[only] -= lengthOf[known];
                int u = only;
                for (int j = 1; j <= lengthOf[known]; ++j)
                    u = addNode(known, u, j == lengthOf[known]);
                ++known;
                continue;
            }
        }

        int n = par.size();
        vector<vector<int>> child(n), full(n);
        vector<char> use(n, 0);
        for (int u = 0; u < n; ++u) {
            if (par[u] != -1) full[par[u]].push_back(u);
            if (pending[u])
                for (int v = u; v != -1 && !use[v]; v = par[v])
                    use[v] = 1;
        }
        for (int u = 0; u < n; ++u)
            if (use[u] && par[u] >= 0) child[par[u]].push_back(u);

        // Color the old skeleton so that its projected preorder identifies
        // every vertex. For wide sibling sets use a two-symbol identifier:
        // the second symbol is on the child's first skeleton child.
        int degree = 0;
        for (auto &v : child) degree = max(degree, (int)v.size());
        int W = known ? max(1, degree) : 0, p = 0, h = 0;
        for (int t = 1; t <= degree; ++t) {
            int s = (degree + t - 1) / t;
            if (s + t < W) W = s + t, p = t, h = s;
        }
        vector<int> col(n), secondary(n, -1);
        function<void(int)> color = [&](int u) {
            if (secondary[u] >= 0 && child[u].empty())
                child[u].push_back(full[u].front());
            int start = secondary[u] >= 0;
            for (int i = 0; i < (int)child[u].size(); ++i) {
                int v = child[u][i];
                if (i < start) col[v] = secondary[u];
                else if (!p) col[v] = i;
                else {
                    int k = i - start;
                    col[v] = h + k % p;
                    if ((int)child[u].size() - start > p)
                        secondary[v] = k / p;
                }
                color(v);
            }
        };
        if (known) color(root);

        int left = M - known;
        // When the batch consists solely of singleton paths, its attachment
        // counts can simply be subtracted, without probing the remaining tail.
        vector<int> longPrefix(left + 1);
        for (int i = 0; i < left; ++i)
            longPrefix[i+1] = longPrefix[i] + (lengthOf[known+i] != 1);
        auto needTail = [&](int b) {
            return b < left && (!known || longPrefix[b] != 0);
        };

        int batch = 1, best = INT_MAX;
        for (int b = 1; b <= left; ++b) {
            int costB = max(B, W + b + (int)needTail(b) - 1);
            int score = costB + 1;
            int r = left - b;
            if (r) {
                int extra = INT_MAX;
                for (int t = 1; t <= r; ++t) {
                    int palette = max(1, W) + t +
                        (t < r && (!known || longPrefix[left] != longPrefix[b]));
                    extra = min(extra, max(0, palette - 1 - costB) +
                                       (r + t - 1) / t);
                }
                score += extra;
            }
            if (score <= best) best = score, batch = b;
        }

        bool tail = needTail(batch);
        int low = tail ? 1 : 0, oldBase = low + batch;
        function<void(int)> fillOld = [&](int u) {
            put(representative[u], oldBase + col[u]);
            for (int v : child[u]) fillOld(v);
        };
        if (known) fillOld(root);

        int end = known + batch, successes = 0;
        for (int i = known; i < (tail ? M : end); ++i) {
            int x = i < end ? low + end - 1 - i : 0;
            if (known) {
                for (int j = 0; j < lengthOf[i]; ++j) put(i, x);
            } else {
                do {
                    if (!put(i, x)) break;
                    ++successes;
                    ++lengthOf[i];
                } while (successes + M - 1 - i < 1000);
            }
        }

        vector<int> seq = collect(), projected, order;
        for (int x : seq)
            if (x >= oldBase) projected.push_back(x - oldBase);
        int z = 0;
        function<void(int)> decodeOld = [&](int u) {
            order.push_back(u);
            ++z;
            for (int i = 0; i < (int)child[u].size(); ++i) {
                int c = projected[z];
                for (int v : child[u]) {
                    if (col[v] != c) continue;
                    if (secondary[v] >= 0 && secondary[v] != projected[z+1])
                        continue;
                    decodeOld(v);
                    break;
                }
            }
        };
        if (known) decodeOld(root);

        if (tail) fill(pending.begin(), pending.end(), 0);
        int k = 0, oi = 0;
        function<void(int)> path = [&](int parent) {
            int x = seq[k], leaf = end - 1 - (x - low);
            if (known && !tail && end < M) --pending[parent];
            int u = parent;
            for (int j = 1; j <= lengthOf[leaf]; ++j) {
                ++k;
                u = addNode(leaf, u, j == lengthOf[leaf]);
                if (j == lengthOf[leaf]) break;
                // Every later path branches before this path's continuation.
                while (seq[k] < x) {
                    if (tail && seq[k] == 0) ++pending[u], ++k;
                    else path(u);
                }
            }
        };

        int anchor = -1;
        while (k < (int)seq.size()) {
            if (seq[k] >= oldBase) {
                anchor = order[oi++];
                ++k;
            } else if (tail && seq[k] == 0) {
                ++pending[anchor];
                ++k;
            } else path(anchor);
        }
        known = end;
    }

    int N = par.size();
    auto label = [&](int u) {
        return u == root ? N-1 : u == N-1 ? root : u;
    };
    vector<int> ans(N);
    for (int u = 0; u < N; ++u)
        if (u != root) ans[label(u)] = label(par[u]);
    cout << "! " << N;
    for (int i = 0; i+1 < N; ++i) cout << ' ' << ans[i];
    cout << endl;
    return 0;
}
