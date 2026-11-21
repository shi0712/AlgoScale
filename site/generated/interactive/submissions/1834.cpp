#include <bits/stdc++.h>
using namespace std;

struct Event {
    int v;
    int u;
    int type; // 0 = degree <= 1 (complete info), 1 = degree 2, 2 = high (forced), 3 = root
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    set<int> alive;
    for (int i = 1; i <= n; ++i) alive.insert(i);

    vector<Event> events;
    int s = n;

    auto query = [&](int d, int &v, int &u) -> bool {
        cout << "? " << d << '\n';
        cout.flush();
        if (!(cin >> v >> u)) return false;
        if (v == -1 || u == -1) return false;
        return true;
    };

    auto add_last = [&]() {
        int v = *alive.begin();
        alive.erase(alive.begin());
        --s;
        events.push_back({v, 0, 3});
    };

    // First peel all vertices of H-degree <= 1.
    while (s > 0) {
        if (s == 1) {
            add_last();
            break;
        }

        int d = s - 2; // degree in G at least d  <=> H-degree <= 1
        int v, u;
        if (!query(d, v, u)) return 0;

        if (v == 0) {
            break; // remaining graph has minimum H-degree at least 2
        }

        events.push_back({v, u, (u == 0 ? 3 : 0)});
        alive.erase(v);
        --s;
    }

    // Process the 2-core.
    while (s > 0) {
        if (s == 1) {
            add_last();
            break;
        }

        // Try to remove a vertex of H-degree exactly 2.
        bool removed = false;
        if (s >= 3) {
            int d = s - 3; // H-degree <= 2
            int v, u;
            if (!query(d, v, u)) return 0;
            if (v != 0) {
                events.push_back({v, u, (u == 0 ? 3 : 1)});
                alive.erase(v);
                --s;
                removed = true;
            }
        }

        if (removed) {
            // Peel the newly-created low degree vertices.
            while (s >= 2) {
                int d = s - 2;
                int v, u;
                if (!query(d, v, u)) return 0;
                if (v == 0) break;
                events.push_back({v, u, (u == 0 ? 3 : 0)});
                alive.erase(v);
                --s;
            }
            continue;
        }

        // The remaining graph has minimum H-degree at least 3.
        // Remove a low degree vertex of G (high degree in H).
        {
            int d = 0;
            int v, u;
            if (!query(d, v, u)) return 0;
            if (v == 0) {
                // Should never happen for d = 0 unless s = 0.
                break;
            }
            events.push_back({v, u, (u == 0 ? 3 : 2)});
            alive.erase(v);
            --s;
        }

        // After breaking the high-degree core, peel as much as possible.
        while (s >= 2) {
            int d = s - 2;
            int v, u;
            if (!query(d, v, u)) return 0;
            if (v == 0) break;
            events.push_back({v, u, (u == 0 ? 3 : 0)});
            alive.erase(v);
            --s;
        }
    }

    // Construct the Hamiltonian path by inserting vertices in reverse removal order.
    deque<int> path;

    for (int i = (int)events.size() - 1; i >= 0; --i) {
        Event e = events[i];
        int v = e.v;
        int u = e.u;

        if (path.empty()) {
            path.push_back(v);
            continue;
        }

        int fr = path.front();
        int bk = path.back();

        // Prefer an endpoint with label strictly smaller than u:
        // because u is the minimum non-neighbour among the remaining vertices,
        // every vertex with label < u is certainly adjacent to v in the original graph.
        if (u != 0) {
            bool fsafe = (fr < u);
            bool bsafe = (bk < u);

            if (bsafe) {
                path.push_back(v);
                continue;
            }
            if (fsafe) {
                path.push_front(v);
                continue;
            }
        }

        // Otherwise avoid the known non-neighbour.
        if (u == 0) {
            path.push_back(v);
        } else if (bk != u) {
            path.push_back(v);
        } else if (fr != u) {
            path.push_front(v);
        } else {
            // Only possible for a path of one vertex equal to u.
            // Insert at an arbitrary end; later insertions may separate them.
            path.push_back(v);
        }
    }

    cout << '!';
    for (int v : path) cout << ' ' << v;
    cout << '\n';
    cout.flush();

    return 0;
}
