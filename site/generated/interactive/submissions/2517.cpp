#include <bits/stdc++.h>
using namespace std;

const int MAXN = 600;
int n;
bitset<MAXN> full;

struct BitsetHash {
    size_t operator()(const bitset<MAXN>& b) const {
        size_t h = 0;
        for (int i = 0; i < MAXN; i += 64) {
            unsigned long long word = 0;
            for (int j = 0; j < 64 && i + j < MAXN; ++j) {
                if (b[i + j]) word |= (1ULL << j);
            }
            h ^= word + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        }
        return h;
    }
};

unordered_map<bitset<MAXN>, int, BitsetHash> cache;

int ask(bitset<MAXN> S) {
    auto it = cache.find(S);
    if (it != cache.end()) return it->second;
    vector<int> v;
    for (int i = 0; i < n; ++i) if (S[i]) v.push_back(i + 1);
    cout << "? " << v.size() << "\n";
    for (int x : v) cout << x << " ";
    cout << "\n";
    cout.flush();
    int res;
    cin >> res;
    if (res == -1) exit(0);
    cache[S] = res;
    return res;
}

pair<int, int> find_edge_between(bitset<MAXN> A, bitset<MAXN> B) {
    int eA = ask(A);
    bitset<MAXN> S = B;
    int v = -1;
    while (S.count() > 1) {
        vector<int> verts;
        for (int i = 0; i < n; ++i) if (S[i]) verts.push_back(i);
        int mid = verts.size() / 2;
        bitset<MAXN> S1, S2;
        for (int i = 0; i < mid; ++i) S1.set(verts[i]);
        for (int i = mid; i < (int)verts.size(); ++i) S2.set(verts[i]);
        int eS1 = ask(S1);
        bitset<MAXN> AS1 = A | S1;
        int eAS1 = ask(AS1);
        int cross = eAS1 - eA - eS1;
        if (cross > 0) S = S1;
        else S = S2;
    }
    for (int i = 0; i < n; ++i) if (S[i]) { v = i; break; }

    S = A;
    int u = -1;
    while (S.count() > 1) {
        vector<int> verts;
        for (int i = 0; i < n; ++i) if (S[i]) verts.push_back(i);
        int mid = verts.size() / 2;
        bitset<MAXN> S1, S2;
        for (int i = 0; i < mid; ++i) S1.set(verts[i]);
        for (int i = mid; i < (int)verts.size(); ++i) S2.set(verts[i]);
        int eS1 = ask(S1);
        bitset<MAXN> vS1 = S1;
        vS1.set(v);
        int evS1 = ask(vS1);
        int cross = evS1 - eS1;
        if (cross > 0) S = S1;
        else S = S2;
    }
    for (int i = 0; i < n; ++i) if (S[i]) { u = i; break; }
    return {u, v};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    cin >> n;
    if (n == 1) {
        cout << "Y 1\n1\n";
        cout.flush();
        return 0;
    }

    full.reset();
    for (int i = 0; i < n; ++i) full.set(i);

    vector<int> treeAdj[MAXN];
    bitset<MAXN> T;
    T.reset();
    T.set(0);

    while (T.count() < n) {
        bitset<MAXN> outside = full;
        outside ^= T;
        auto [u, v] = find_edge_between(T, outside);
        treeAdj[u].push_back(v);
        treeAdj[v].push_back(u);
        T.set(v);
    }

    vector<int> color(n, -1), parent(n, -1), depth(n, 0);
    queue<int> q;
    color[0] = 0;
    q.push(0);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : treeAdj[u]) {
            if (color[v] == -1) {
                color[v] = color[u] ^ 1;
                parent[v] = u;
                depth[v] = depth[u] + 1;
                q.push(v);
            }
        }
    }

    bitset<MAXN> Aset, Bset;
    for (int i = 0; i < n; ++i) {
        if (color[i] == 0) Aset.set(i);
        else Bset.set(i);
    }

    int eA = ask(Aset);
    int eB = ask(Bset);

    if (eA == 0 && eB == 0) {
        vector<int> side;
        for (int i = 0; i < n; ++i) if (color[i] == 0) side.push_back(i + 1);
        cout << "Y " << side.size() << "\n";
        for (int x : side) cout << x << " ";
        cout << "\n";
        cout.flush();
        return 0;
    }

    bitset<MAXN> S;
    int ecur;
    if (eA > 0) { S = Aset; ecur = eA; }
    else { S = Bset; ecur = eB; }

    function<pair<int, int>(bitset<MAXN>, int)> find_edge_inside =
        [&](bitset<MAXN> cur, int ec) -> pair<int, int> {
            if (cur.count() == 1) return {-1, -1};
            vector<int> verts;
            for (int i = 0; i < n; ++i) if (cur[i]) verts.push_back(i);
            int mid = verts.size() / 2;
            bitset<MAXN> S1, S2;
            for (int i = 0; i < mid; ++i) S1.set(verts[i]);
            for (int i = mid; i < (int)verts.size(); ++i) S2.set(verts[i]);
            int e1 = ask(S1);
            int e2 = ask(S2);
            int cross = ec - e1 - e2;
            if (e1 > 0) return find_edge_inside(S1, e1);
            if (e2 > 0) return find_edge_inside(S2, e2);
            return find_edge_between(S1, S2);
        };

    auto [u, v] = find_edge_inside(S, ecur);

    vector<int> up, down;
    int x = u, y = v;
    while (depth[x] > depth[y]) {
        up.push_back(x);
        x = parent[x];
    }
    while (depth[y] > depth[x]) {
        down.push_back(y);
        y = parent[y];
    }
    while (x != y) {
        up.push_back(x);
        down.push_back(y);
        x = parent[x];
        y = parent[y];
    }
    up.push_back(x);

    vector<int> path = up;
    reverse(down.begin(), down.end());
    path.insert(path.end(), down.begin(), down.end());

    cout << "N " << path.size() << "\n";
    for (int idx : path) cout << idx + 1 << " ";
    cout << "\n";
    cout.flush();
    return 0;
}
