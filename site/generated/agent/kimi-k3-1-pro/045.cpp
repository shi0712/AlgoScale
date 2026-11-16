#include <bits/stdc++.h>
using namespace std;

struct Node {
    int h = 0, d = 0;
    uint32_t heightHeap = 0, diameterHeap = 0;
    uint32_t current = 0;
};

struct Entry {
    uint32_t node;
    uint32_t hl = 0, hr = 0, dl = 0, dr = 0;
    int h, d;
    unsigned char hrank = 1, drank = 1;
};

struct Solver {
    int n;
    unique_ptr<Node[]> nodes;
    vector<Entry> entries;

    explicit Solver(int n) : n(n), nodes(new Node[n]) {
        entries.reserve(4 * (size_t)n + 8);
    }

    Entry &entry(uint32_t i) { return entries[i - 1]; }
    Node *node(uint32_t i) { return &nodes[i]; }

    int rankH(uint32_t e) { return e ? entry(e).hrank : 0; }
    int rankD(uint32_t e) { return e ? entry(e).drank : 0; }

    uint32_t meldH(uint32_t a, uint32_t b) {
        if (!a) return b;
        if (!b) return a;
        if (entry(a).h < entry(b).h) swap(a, b);
        entry(a).hr = meldH(entry(a).hr, b);
        if (rankH(entry(a).hl) < rankH(entry(a).hr))
            swap(entry(a).hl, entry(a).hr);
        entry(a).hrank = rankH(entry(a).hr) + 1;
        return a;
    }

    uint32_t meldD(uint32_t a, uint32_t b) {
        if (!a) return b;
        if (!b) return a;
        if (entry(a).d < entry(b).d) swap(a, b);
        entry(a).dr = meldD(entry(a).dr, b);
        if (rankD(entry(a).dl) < rankD(entry(a).dr))
            swap(entry(a).dl, entry(a).dr);
        entry(a).drank = rankD(entry(a).dr) + 1;
        return a;
    }

    uint32_t cleanH(uint32_t e) {
        while (e && node(entry(e).node)->current != e)
            e = meldH(entry(e).hl, entry(e).hr);
        return e;
    }

    uint32_t cleanD(uint32_t e) {
        while (e && node(entry(e).node)->current != e)
            e = meldD(entry(e).dl, entry(e).dr);
        return e;
    }

    void addChild(Node *parent, Node *child) {
        uint32_t id = (uint32_t)entries.size() + 1;
        Entry e;
        e.node = (uint32_t)(child - nodes.get());
        e.h = child->h + 1;
        e.d = child->d;
        entries.push_back(e);
        child->current = id;
        parent->heightHeap = meldH(parent->heightHeap, id);
        parent->diameterHeap = meldD(parent->diameterHeap, id);
    }

    pair<int, Node *> tallest(Node *v) {
        v->heightHeap = cleanH(v->heightHeap);
        if (!v->heightHeap) return {0, nullptr};
        Entry &e = entry(v->heightHeap);
        return {e.h, node(e.node)};
    }

    pair<pair<int, Node *>, pair<int, Node *>> twoTallest(Node *v) {
        auto first = tallest(v);
        if (!first.second) return {first, {0, nullptr}};

        uint32_t id = v->heightHeap;
        v->heightHeap = meldH(entry(id).hl, entry(id).hr);
        entry(id).hl = entry(id).hr = 0;
        entry(id).hrank = 1;
        auto second = tallest(v);
        v->heightHeap = meldH(v->heightHeap, id);
        return {first, second};
    }

    Node *largestDiameterChild(Node *v) {
        v->diameterHeap = cleanD(v->diameterHeap);
        return v->diameterHeap ? node(entry(v->diameterHeap).node) : nullptr;
    }

    void recalculate(Node *v) {
        auto [a, b] = twoTallest(v);
        Node *diameterChild = largestDiameterChild(v);
        v->h = a.first;
        v->d = max({a.first, a.first + b.first,
                    diameterChild ? diameterChild->d : 0});
    }

    Node *reroot(Node *root, Node *child) {
        child->current = 0;
        recalculate(root);
        addChild(child, root);
        recalculate(child);
        return child;
    }

    Node *foldAtRoot(Node *root, Node *a, Node *b,
                     vector<Node *> &pa, vector<Node *> &pb,
                     vector<Node *> &path) {
        pa.clear();
        pb.clear();
        path.clear();

        for (Node *v = a;;) {
            pa.push_back(v);
            Node *next = tallest(v).second;
            if (!next) break;
            v = next;
        }
        for (Node *v = b;;) {
            pb.push_back(v);
            Node *next = tallest(v).second;
            if (!next) break;
            v = next;
        }

        a->current = b->current = 0;
        for (int i = 0; i + 1 < (int)pa.size(); ++i) pa[i + 1]->current = 0;
        for (int i = 0; i + 1 < (int)pb.size(); ++i) pb[i + 1]->current = 0;

        for (auto it = pa.rbegin(); it != pa.rend(); ++it) path.push_back(*it);
        path.push_back(root);
        path.insert(path.end(), pb.begin(), pb.end());

        int D = (int)path.size() - 1;
        int m = (D + 2) / 2;
        for (int i = 0; i < m; ++i) {
            Node *u = path[i];
            Node *v = path[D - i];
            if (u == v) continue;
            u->heightHeap = meldH(u->heightHeap, v->heightHeap);
            u->diameterHeap = meldD(u->diameterHeap, v->diameterHeap);
            v->heightHeap = v->diameterHeap = 0;
        }

        int rootPosition = min((int)pa.size(), (int)pb.size());
        Node *newRoot = path[rootPosition];

        if (rootPosition > 0) {
            recalculate(path[0]);
            for (int i = 1; i < rootPosition; ++i) {
                addChild(path[i], path[i - 1]);
                recalculate(path[i]);
            }
        }
        if (rootPosition + 1 < m) {
            recalculate(path[m - 1]);
            for (int i = m - 2; i > rootPosition; --i) {
                addChild(path[i], path[i + 1]);
                recalculate(path[i]);
            }
        }
        if (rootPosition > 0) addChild(newRoot, path[rootPosition - 1]);
        if (rootPosition + 1 < m) addChild(newRoot, path[rootPosition + 1]);
        recalculate(newRoot);
        return newRoot;
    }

    int run(const vector<int> &eu, const vector<int> &ev) {
        if (n == 1) return 0;

        vector<int> offset(n + 1, 0);
        for (int i = 0; i < n - 1; ++i) {
            ++offset[eu[i] + 1];
            ++offset[ev[i] + 1];
        }
        for (int i = 0; i < n; ++i) offset[i + 1] += offset[i];
        vector<int> adjacency(2 * (n - 1));
        vector<int> cursor = offset;
        for (int i = 0; i < n - 1; ++i) {
            adjacency[cursor[eu[i]]++] = ev[i];
            adjacency[cursor[ev[i]]++] = eu[i];
        }

        vector<int> parent(n, -2), order;
        order.reserve(n);
        parent[0] = -1;
        order.push_back(0);
        for (int i = 0; i < n; ++i) {
            int v = order[i];
            for (int j = offset[v]; j < offset[v + 1]; ++j) {
                int w = adjacency[j];
                if (parent[w] != -2) continue;
                parent[w] = v;
                order.push_back(w);
            }
        }
        for (int i = n - 1; i >= 0; --i) {
            int v = order[i];
            recalculate(node(v));
            if (parent[v] != -1) addChild(node(parent[v]), node(v));
        }
        vector<int>().swap(adjacency);
        vector<int>().swap(cursor);
        vector<int>().swap(offset);
        vector<int>().swap(parent);
        vector<int>().swap(order);

        Node *root = node(0);
        vector<Node *> pa, pb, path;
        int remaining = n, answer = 0;
        while (remaining > 1) {
            if (remaining == 2) {
                ++answer;
                break;
            }
            auto [a, b] = twoTallest(root);
            Node *diameterChild = largestDiameterChild(root);
            if (!b.second || diameterChild->d > a.first + b.first) {
                root = reroot(root, b.second ? diameterChild : a.second);
                continue;
            }
            if (a.first > b.first + 1) {
                root = reroot(root, a.second);
                continue;
            }

            int D = a.first + b.first;
            root = foldAtRoot(root, a.second, b.second, pa, pb, path);
            remaining -= (D + 1) / 2;
            ++answer;
        }
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        int n;
        cin >> n;
        vector<int> eu(n - 1), ev(n - 1);
        for (int i = 0; i < n - 1; ++i) {
            cin >> eu[i] >> ev[i];
            --eu[i];
            --ev[i];
        }
        Solver solver(n);
        cout << solver.run(eu, ev) << '\n';
    }
    return 0;
}
