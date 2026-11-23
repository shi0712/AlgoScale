#include <bits/stdc++.h>
using namespace std;

struct Record {
    int v, u;
    bool typeA;
};

struct Segment {
    int a = 0, b = 0;
    int sz = 0;
};

static vector<int> buildAnswer(const vector<Record>& records,
                               const vector<int>& base,
                               bool baseComplete,
                               int n) {
    const int r = (int)records.size();

    vector<array<int, 2>> link(n + 1);
    for (int i = 1; i <= n; ++i) link[i] = {0, 0};

    vector<Segment> seg(2 * n + 5);
    vector<int> singletonOf(n + 1, 0);
    set<int> active, singletons;
    int nextId = n + 1;

    auto addLink = [&](int x, int y) {
        if (link[x][0] == 0) link[x][0] = y;
        else link[x][1] = y;
    };

    auto insertSegment = [&](int id, int a, int b, int sz) {
        seg[id] = {a, b, sz};
        active.insert(id);
        if (sz == 1) {
            singletons.insert(id);
            singletonOf[a] = id;
        }
    };

    auto eraseSegment = [&](int id) {
        active.erase(id);
        if (seg[id].sz == 1) {
            singletons.erase(id);
            singletonOf[seg[id].a] = 0;
        }
    };

    auto makeSingleton = [&](int v) {
        insertSegment(v, v, v, 1);
    };

    auto farEndpoint = [&](int id, int used) {
        if (seg[id].sz == 1) return used;
        return seg[id].a == used ? seg[id].b : seg[id].a;
    };

    if (baseComplete && base.size() >= 2) {
        for (int i = 0; i + 1 < (int)base.size(); ++i)
            addLink(base[i], base[i + 1]);
        insertSegment(nextId, base.front(), base.back(), (int)base.size());
    } else {
        for (int v : base) makeSingleton(v);
    }

    auto attach = [&](int id, int v, int forbidden) {
        int x = seg[id].a != forbidden ? seg[id].a : seg[id].b;
        int far = farEndpoint(id, x);
        int oldSize = seg[id].sz;
        eraseSegment(id);
        addLink(v, x);
        insertSegment(v, v, far, oldSize + 1);
    };

    auto mergeThrough = [&](int p, int q, int v, int forbidden) {
        int x = seg[p].a != forbidden ? seg[p].a : seg[p].b;
        int y = seg[q].a != forbidden ? seg[q].a : seg[q].b;
        int fp = farEndpoint(p, x);
        int fq = farEndpoint(q, y);
        int newSize = seg[p].sz + seg[q].sz + 1;

        eraseSegment(p);
        eraseSegment(q);
        addLink(x, v);
        addLink(v, y);
        insertSegment(v, fp, fq, newSize);
    };

    for (int i = r - 1; i >= 0; --i) {
        const Record& rec = records[i];
        int v = rec.v;

        if (!rec.typeA) {
            makeSingleton(v);
            continue;
        }

        int forbidden = rec.u;
        int blocked = 0;
        if (forbidden >= 1 && forbidden <= n)
            blocked = singletonOf[forbidden];

        vector<int> candidates;
        for (int id : singletons) {
            if (id == blocked) continue;
            candidates.push_back(id);
            if (candidates.size() == 2) break;
        }

        if (candidates.size() < 2) {
            for (int id : active) {
                if (id == blocked) continue;
                bool duplicate = false;
                for (int old : candidates)
                    if (old == id) duplicate = true;
                if (duplicate) continue;
                candidates.push_back(id);
                if (candidates.size() == 2) break;
            }
        }

        if (candidates.size() >= 2) {
            mergeThrough(candidates[0], candidates[1], v, forbidden);
        } else if (candidates.size() == 1) {
            attach(candidates[0], v, forbidden);
        } else {
            makeSingleton(v);
        }
    }

    // A small fallback that can also join two existing paths directly.
    if (active.size() > 1 && active.size() <= 32) {
        vector<int> pos(n + 1, r);
        for (int i = 0; i < r; ++i) pos[records[i].v] = i;

        auto guaranteedEdge = [&](int x, int y) {
            if (pos[x] == r && pos[y] == r) return baseComplete;
            if (pos[x] > pos[y]) swap(x, y);
            const Record& rec = records[pos[x]];
            return rec.typeA && rec.u != y;
        };

        bool progress = true;
        while (active.size() > 1 && progress) {
            progress = false;
            vector<int> ids(active.begin(), active.end());

            for (int i = 0; i < (int)ids.size() && !progress; ++i) {
                for (int j = i + 1; j < (int)ids.size() && !progress; ++j) {
                    int p = ids[i], q = ids[j];
                    if (!active.count(p) || !active.count(q)) continue;

                    int xp[2] = {seg[p].a, seg[p].b};
                    int xq[2] = {seg[q].a, seg[q].b};
                    int cp = seg[p].sz == 1 ? 1 : 2;
                    int cq = seg[q].sz == 1 ? 1 : 2;

                    for (int a = 0; a < cp && !progress; ++a) {
                        for (int b = 0; b < cq && !progress; ++b) {
                            int x = xp[a], y = xq[b];
                            if (!guaranteedEdge(x, y)) continue;

                            int fp = farEndpoint(p, x);
                            int fq = farEndpoint(q, y);
                            int newSize = seg[p].sz + seg[q].sz;
                            eraseSegment(p);
                            eraseSegment(q);
                            addLink(x, y);
                            insertSegment(++nextId, fp, fq, newSize);
                            progress = true;
                        }
                    }
                }
            }
        }
    }

    if (active.size() != 1) return {};

    int id = *active.begin();
    vector<int> answer;
    answer.reserve(n);

    int previous = 0;
    int current = seg[id].a;
    for (int i = 0; i < n; ++i) {
        if (current == 0) return {};
        answer.push_back(current);
        int nxt = link[current][0];
        if (nxt == previous) nxt = link[current][1];
        previous = current;
        current = nxt;
    }

    if ((int)answer.size() != n) return {};
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 2) {
        cout << "! 1 2" << endl;
        return 0;
    }

    set<int> remainingSet;
    for (int i = 1; i <= n; ++i) remainingSet.insert(i);

    vector<Record> records;
    records.reserve(n);

    int knownMissing = 0;
    int nonzeroA = 0;
    bool hadB = false;
    bool triedFirstFailure = false;

    auto printAnswer = [&](const vector<int>& answer) {
        cout << '!';
        for (int v : answer) cout << ' ' << v;
        cout << endl;
    };

    while (true) {
        int m = (int)remainingSet.size();
        vector<int> base(remainingSet.begin(), remainingSet.end());

        if (knownMissing == n - 2) {
            vector<int> answer = buildAnswer(records, base, true, n);
            if (!answer.empty()) {
                printAnswer(answer);
                return 0;
            }
        }

        if (m == 1) {
            vector<int> answer = buildAnswer(records, base, false, n);
            if (!answer.empty()) printAnswer(answer);
            return 0;
        }

        if (!hadB && m == 2) {
            int leftMissing = n - 2 - knownMissing;
            vector<int> answer = buildAnswer(records, base, leftMissing == 0, n);
            if (!answer.empty()) {
                printAnswer(answer);
                return 0;
            }
        }

        if (hadB && m == 2) {
            vector<int> answer = buildAnswer(records, base, false, n);
            if (!answer.empty()) {
                printAnswer(answer);
                return 0;
            }
        }

        // If all removed vertices were universal toward the remaining graph,
        // they can be used as connectors without learning anything more.
        if (!hadB && nonzeroA == 0 &&
            (int)records.size() >= m - 1) {
            vector<int> answer = buildAnswer(records, base, false, n);
            if (!answer.empty()) {
                printAnswer(answer);
                return 0;
            }
        }

        cout << "? " << m - 2 << endl;
        int v, u;
        if (!(cin >> v >> u)) return 0;
        if (v < 0 || u < 0) return 0;

        if (v != 0) {
            records.push_back({v, u, true});
            remainingSet.erase(v);
            if (u != 0) {
                ++knownMissing;
                ++nonzeroA;
            }
            continue;
        }

        // No vertex has at most one nonneighbor. First check whether the
        // already obtained connectors are sufficient by themselves.
        if (!triedFirstFailure) {
            triedFirstFailure = true;
            if ((int)records.size() >= m - 1) {
                vector<int> answer = buildAnswer(records, base, false, n);
                if (!answer.empty()) {
                    printAnswer(answer);
                    return 0;
                }
            }
        }

        cout << "? 0" << endl;
        if (!(cin >> v >> u)) return 0;
        if (v < 0 || u < 0) return 0;
        if (v == 0) return 0;

        records.push_back({v, u, false});
        remainingSet.erase(v);
        hadB = true;
        ++knownMissing;
    }
}
