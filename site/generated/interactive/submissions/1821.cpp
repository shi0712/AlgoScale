#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    deque<int> q;
    int distinct = 0;
    bool defer1 = false;

    vector<int> label(N + 1, -1);
    vector<vector<int>> groups;

    auto addOp = [&](int id) -> int {
        cout << "+ " << id << '\n' << flush;
        int r;
        if (!(cin >> r)) exit(0);
        if (r == -1) exit(0);
        return r;
    };

    auto remOp = [&]() -> int {
        cout << "-\n" << flush;
        int r;
        if (!(cin >> r)) exit(0);
        if (r == -1) exit(0);
        return r;
    };

    for (int id = 1; id <= N; ++id) {
        int c = addOp(id);
        q.push_back(id);

        if (defer1) {
            if (c == 1) {
                label[id] = 0;
                groups[0].push_back(id);
            } else {
                // The second distinct class has appeared.
                // Remove all obsolete duplicates of the first class,
                // leaving one representative of class 0 and the new id.
                int removeCnt = (int)q.size() - 2;
                for (int t = 0; t < removeCnt; ++t) {
                    remOp();
                    q.pop_front();
                }

                int gid = (int)groups.size();
                label[id] = gid;
                groups.push_back({id});

                distinct = 2;
                defer1 = false;
            }
        } else {
            if (c > distinct) {
                distinct = c;

                int gid = (int)groups.size();
                label[id] = gid;
                groups.push_back({id});

                if (distinct == 1) defer1 = true;
            } else {
                // id belongs to an already known class.
                int cur = c;
                vector<int> stored;
                int matchId = -1;

                while (true) {
                    int r = q.front();
                    int d = remOp();
                    q.pop_front();

                    if (d == cur) {
                        matchId = r;
                        break;
                    } else {
                        stored.push_back(r);
                        cur = d;
                    }
                }

                int gid = label[matchId];
                label[id] = gid;
                groups[gid].push_back(id);

                for (int rep : stored) {
                    int d = addOp(rep);
                    q.push_back(rep);
                    cur = d;
                }
            }
        }
    }

    int K = (int)groups.size();

    cout << "! " << K << '\n';
    for (const auto &g : groups) {
        cout << g.size();
        for (int x : g) cout << ' ' << x;
        cout << '\n';
    }
    cout << flush;

    return 0;
}
