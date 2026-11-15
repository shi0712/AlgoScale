#include <bits/stdc++.h>
using namespace std;

namespace io {
    static const int SZ = 1 << 16;
    char buf[SZ];
    int pos = 0, len = 0;
    inline int readChar() {
        if (pos == len) {
            len = (int)fread(buf, 1, SZ, stdin);
            pos = 0;
            if (len == 0) return -1;
        }
        return buf[pos++];
    }
    inline bool readInt(int &out) {
        int c = readChar();
        while (c != -1 && (c < '0' || c > '9') && c != '-') c = readChar();
        if (c == -1) return false;
        bool neg = false;
        if (c == '-') { neg = true; c = readChar(); }
        int x = 0;
        while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = readChar(); }
        out = neg ? -x : x;
        return true;
    }
}

struct Solver {
    int n;
    vector<int> parent, h, hpos, chead, cto, cnext, childFree;
    vector<char> active;
    vector<vector<int>> hch;

    inline bool better(int a, int b) const { return h[a] > h[b]; }

    void siftUp(int u, int i) {
        auto &H = hch[u];
        int x = H[i];
        while (i > 0) {
            int p = (i - 1) >> 1;
            if (!better(x, H[p])) break;
            H[i] = H[p];
            hpos[H[i]] = i;
            i = p;
        }
        H[i] = x;
        hpos[x] = i;
    }
    void siftDown(int u, int i) {
        auto &H = hch[u];
        int sz = (int)H.size();
        int x = H[i];
        while (true) {
            int l = 2 * i + 1;
            if (l >= sz) break;
            int r = l + 1;
            int b = l;
            if (r < sz && better(H[r], H[l])) b = r;
            if (!better(H[b], x)) break;
            H[i] = H[b];
            hpos[H[i]] = i;
            i = b;
        }
        H[i] = x;
        hpos[x] = i;
    }
    void addChild(int u, int c) {
        auto &H = hch[u];
        H.push_back(c);
        int i = (int)H.size() - 1;
        hpos[c] = i;
        siftUp(u, i);
    }
    void removeChild(int u, int c) {
        auto &H = hch[u];
        int i = hpos[c];
        int last = H.back();
        H.pop_back();
        if (i < (int)H.size()) {
            H[i] = last;
            hpos[last] = i;
            if (i > 0 && better(last, H[(i - 1) >> 1])) siftUp(u, i);
            else siftDown(u, i);
        }
    }
    int topChild(int u) const {
        return hch[u].empty() ? -1 : hch[u][0];
    }
    int topChildExcl(int u, int excl) const {
        const auto &H = hch[u];
        if (H.empty()) return -1;
        if (H[0] != excl) return H[0];
        int sz = (int)H.size();
        if (sz == 1) return -1;
        if (sz == 2) return H[1];
        return better(H[1], H[2]) ? H[1] : H[2];
    }
    void recalc(int v) {
        int nh = hch[v].empty() ? 0 : h[hch[v][0]] + 1;
        if (nh != h[v]) {
            int old = h[v];
            h[v] = nh;
            int p = parent[v];
            if (p != -1) {
                int i = hpos[v];
                if (nh > old) siftUp(p, i);
                else siftDown(p, i);
            }
        }
    }

    long long solveTree(int n_, vector<int> &to, vector<int> &nxt, vector<int> &head) {
        n = n_;
        parent.assign(n, -1);
        h.assign(n, 0);
        hpos.assign(n, -1);
        chead.assign(n, -1);
        active.assign(n, 1);
        hch.assign(n, {});
        childFree.clear();
        cto.clear(); cnext.clear();
        cto.reserve((size_t)n + (size_t)n / 2 + 16);
        cnext.reserve((size_t)n + (size_t)n / 2 + 16);

        vector<int> order;
        order.reserve(n);
        order.push_back(0);
        for (size_t qi = 0; qi < order.size(); qi++) {
            int u = order[qi];
            for (int e = head[u]; e != -1; e = nxt[e]) {
                int v = to[e];
                if (v == parent[u]) continue;
                parent[v] = u;
                order.push_back(v);
            }
        }
        vector<int>().swap(to);
        vector<int>().swap(nxt);
        vector<int>().swap(head);

        for (int i = 1; i < n; i++) {
            int u = order[i];
            cto.push_back(u);
            cnext.push_back(chead[parent[u]]);
            chead[parent[u]] = (int)cto.size() - 1;
        }
        for (int i = n - 1; i >= 1; i--) {
            int u = order[i];
            int p = parent[u];
            if (h[u] + 1 > h[p]) h[p] = h[u] + 1;
        }
        for (int i = 1; i < n; i++) {
            int u = order[i];
            hch[parent[u]].push_back(u);
        }
        for (int u = 0; u < n; u++) {
            auto &H = hch[u];
            for (int i = (int)H.size() / 2 - 1; i >= 0; --i) siftDown(u, i);
            for (int i = 0; i < (int)H.size(); i++) hpos[H[i]] = i;
        }
        vector<int>().swap(order);

        int count = n;
        long long folds = 0;
        vector<int> path;
        path.reserve(64);

        while (count > 1) {
            int s = 0;
            while (true) {
                int w = topChild(s);
                if (w == -1) break;
                s = w;
            }

            int best = -1, bestu = -1, bestw = -1;
            {
                int u = s, pathchild = -1, dist = 0;
                while (u != -1) {
                    if (dist > best) { best = dist; bestu = u; bestw = -1; }
                    int w = topChildExcl(u, pathchild);
                    if (w != -1) {
                        int val = dist + h[w] + 1;
                        if (val > best) { best = val; bestu = u; bestw = w; }
                    }
                    pathchild = u;
                    u = parent[u];
                    ++dist;
                }
            }
            int a = bestu;

            path.clear();
            {
                int x = s;
                while (x != a) {
                    path.push_back(x);
                    x = parent[x];
                }
                path.push_back(a);
            }
            int p0 = (int)path.size() - 1;
            if (bestw != -1) {
                int x = bestw;
                while (true) {
                    path.push_back(x);
                    int w = topChild(x);
                    if (w == -1) break;
                    x = w;
                }
            }
            int k = (int)path.size();
            if (k - 1 <= 2) {
                folds += (count - 1);
                break;
            }
            int half = (k + 1) / 2;
            if (p0 >= half) {
                reverse(path.begin(), path.end());
                p0 = k - 1 - p0;
            }
            for (int i = k - 1; i >= half; --i) {
                int v = path[i];
                int u = path[k - 1 - i];
                int e = chead[v];
                while (e != -1) {
                    int nxtE = cnext[e];
                    int w = cto[e];
                    if (active[w] && parent[w] == v) {
                        parent[w] = u;
                        addChild(u, w);
                        int ne;
                        if (!childFree.empty()) {
                            ne = childFree.back(); childFree.pop_back();
                            cto[ne] = w; cnext[ne] = chead[u];
                        } else {
                            ne = (int)cto.size();
                            cto.push_back(w); cnext.push_back(chead[u]);
                        }
                        chead[u] = ne;
                    }
                    childFree.push_back(e);
                    e = nxtE;
                }
                chead[v] = -1;
                vector<int>().swap(hch[v]);
                int pv = parent[v];
                if (pv != -1) removeChild(pv, v);
                active[v] = 0;
                --count;
            }
            for (int i = 0; i < p0; i++) recalc(path[i]);
            for (int i = half - 1; i > p0; i--) recalc(path[i]);
            recalc(path[p0]);
            ++folds;
        }
        return folds;
    }
};

int main() {
    int T;
    if (!io::readInt(T)) return 0;
    Solver solver;
    while (T--) {
        int n;
        io::readInt(n);
        if (n <= 1) {
            printf("0\n");
            continue;
        }
        int m = n - 1;
        vector<int> head(n, -1), to(2 * m), nxt(2 * m);
        int ec = 0;
        for (int i = 0; i < m; i++) {
            int x, y;
            io::readInt(x); io::readInt(y);
            --x; --y;
            to[ec] = y; nxt[ec] = head[x]; head[x] = ec++;
            to[ec] = x; nxt[ec] = head[y]; head[y] = ec++;
        }
        printf("%lld\n", solver.solveTree(n, to, nxt, head));
    }
    return 0;
}
