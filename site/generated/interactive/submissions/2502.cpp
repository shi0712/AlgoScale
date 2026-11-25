#include <bits/stdc++.h>
using namespace std;

struct Record {
    int v, u;
    bool core;
};

struct Component {
    int l = 0, r = 0;
    list<int>::iterator it;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    const int missingTotal = n - 2;
    int knownMissing = 0;
    vector<int> knownDegree(n + 1, 0);
    set<int> alive;
    for (int v = 1; v <= n; ++v) alive.insert(v);
    int fewerThanTwo = n;

    auto addKnownEdge = [&](int v) {
        if (knownDegree[v] < 2 && ++knownDegree[v] == 2) --fewerThanTwo;
    };

    vector<Record> records;
    records.reserve(n);
    bool allowRefined = true;

    while (alive.size() > 1 && knownMissing < missingTotal) {
        bool core = fewerThanTwo == 0;
        if (!core) {
            cout << "? " << int(alive.size()) - 2 << '\n' << flush;
            int v, u;
            if (!(cin >> v >> u)) return 0;
            if (v == 0) {
                core = true;
            } else {
                records.push_back({v, u, false});
                if (u != 0) {
                    ++knownMissing;
                    addKnownEdge(v);
                    addKnownEdge(u);
                }
                if (knownDegree[v] < 2) --fewerThanTwo;
                alive.erase(v);
                continue;
            }
        }

        int v = 0, u = 0;
        if (allowRefined && alive.size() >= 4) {
            cout << "? " << int(alive.size()) - 4 << '\n' << flush;
            if (!(cin >> v >> u)) return 0;
            if (v == 0) allowRefined = false;
        }
        if (v == 0) {
            cout << "? 0\n" << flush;
            if (!(cin >> v >> u)) return 0;
            if (v == 0) return 0;
        }
        records.push_back({v, u, true});
        if (u != 0) {
            ++knownMissing;
            addKnownEdge(v);
            addKnownEdge(u);
        }
        if (knownDegree[v] < 2) --fewerThanTwo;
        alive.erase(v);
    }

    for (int v : alive) records.push_back({v, 0, false});

    vector<array<int, 2>> path(n + 1, {0, 0});
    vector<Component> components(n + 1);
    list<int> active;

    auto addPathEdge = [&](int a, int b) {
        if (path[a][0] == 0) path[a][0] = b;
        else path[a][1] = b;
        if (path[b][0] == 0) path[b][0] = a;
        else path[b][1] = a;
    };

    auto eligible = [&](int id, int forbidden) {
        return components[id].l != components[id].r || components[id].l != forbidden;
    };

    for (int i = int(records.size()) - 1; i >= 0; --i) {
        auto [v, u, core] = records[i];
        if (core) {
            active.push_back(v);
            components[v].l = components[v].r = v;
            components[v].it = prev(active.end());
            continue;
        }

        int chosen[2];
        int count = 0;
        for (int id : active) {
            if (!eligible(id, u)) continue;
            chosen[count++] = id;
            if (count == 2) break;
        }

        if (count == 0) {
            active.push_back(v);
            components[v].l = components[v].r = v;
            components[v].it = prev(active.end());
        } else if (count == 1) {
            int id = chosen[0];
            int connection = components[id].l != u ? components[id].l : components[id].r;
            int outer = connection == components[id].l ? components[id].r : components[id].l;
            addPathEdge(v, connection);
            components[id].l = v;
            components[id].r = outer;
        } else {
            int a = chosen[0], b = chosen[1];
            int aConnection = components[a].l != u ? components[a].l : components[a].r;
            int aOuter = aConnection == components[a].l ? components[a].r : components[a].l;
            int bConnection = components[b].l != u ? components[b].l : components[b].r;
            int bOuter = bConnection == components[b].l ? components[b].r : components[b].l;
            addPathEdge(aConnection, v);
            addPathEdge(v, bConnection);
            active.erase(components[a].it);
            active.erase(components[b].it);
            components[v].l = aOuter;
            components[v].r = bOuter;
            active.push_back(v);
            components[v].it = prev(active.end());
        }
    }

    int current = components[active.front()].l;
    int previous = 0;
    cout << '!';
    for (int i = 0; i < n; ++i) {
        cout << ' ' << current;
        int next = path[current][0] != previous ? path[current][0] : path[current][1];
        previous = current;
        current = next;
    }
    cout << '\n' << flush;
    return 0;
}
