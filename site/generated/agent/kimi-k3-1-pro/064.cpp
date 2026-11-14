#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Request {
    int64 x, r;
};

struct Node {
    int64 x, r, offset;
};

struct Entry {
    int64 time;
    int side, i, j;
    bool waiting;
};

struct CompareEntry {
    bool operator()(const Entry &a, const Entry &b) const {
        return a.time > b.time;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, start, m;
    cin >> n >> start >> m;
    vector<Request> requests;
    requests.reserve(m);
    for (int i = 0; i < m; ++i) {
        int64 a, f;
        cin >> a >> f;
        requests.push_back({f, a});
    }
    if (requests.empty()) {
        cout << 0 << '\n';
        return 0;
    }

    sort(requests.begin(), requests.end(), [](const Request &a, const Request &b) {
        return a.x < b.x;
    });
    vector<Request> merged;
    for (const auto &q : requests) {
        if (!merged.empty() && merged.back().x == q.x) {
            merged.back().r = max(merged.back().r, q.r);
        } else {
            merged.push_back(q);
        }
    }
    if (merged.size() == 1) {
        cout << max(llabs(start - merged[0].x), merged[0].r) << '\n';
        return 0;
    }

    vector<Node> chain[2];
    for (const auto &q : merged) {
        int64 offset = q.r - q.x;
        if (chain[0].empty() || offset > chain[0].back().offset) {
            chain[0].push_back({q.x, q.r, offset});
        }
    }
    for (auto it = merged.rbegin(); it != merged.rend(); ++it) {
        int64 offset = it->r + it->x;
        if (chain[1].empty() || offset > chain[1].back().offset) {
            chain[1].push_back({it->x, it->r, offset});
        }
    }

    vector<int> cutoff[2];
    vector<int> waitingBest[2];
    for (int side = 0; side < 2; ++side) {
        int other = 1 - side;
        cutoff[side].resize(chain[side].size());
        waitingBest[side].assign(chain[side].size(), -1);
        int o = 0;
        for (int i = 0; i < (int)chain[side].size(); ++i) {
            while (o < (int)chain[other].size() && chain[other][o].r <= chain[side][i].r) {
                ++o;
            }
            cutoff[side][i] = o;
        }
    }

    unordered_map<uint64_t, int64> best;
    best.reserve(6 * (chain[0].size() + chain[1].size()) + 10);
    best.max_load_factor(0.7f);
    priority_queue<Entry, vector<Entry>, CompareEntry> queue;

    auto keyOf = [](int side, int i, int j) {
        return (uint64_t(side) << 40) | (uint64_t(i) << 20) | uint64_t(j);
    };

    auto push = [&](int side, int i, int j, int64 time) {
        uint64_t key = keyOf(side, i, j);
        auto it = best.find(key);
        if (it != best.end() && it->second <= time) {
            return;
        }
        best[key] = time;
        queue.push({time, side, i, j, false});
    };

    auto expand = [&](int side, int i, int j, int64 time) {
        int other = 1 - side;
        waitingBest[side][i + 1] = max(waitingBest[side][i + 1], j);

        int64 arrival = time + llabs(chain[side][i].x - chain[other][j].x);
        int o = cutoff[side][i];
        if (j < o) {
            int landing = o - 1;
            int64 lowTime = arrival + llabs(chain[other][j].x - chain[other][landing].x);
            push(other, landing, i + 1, lowTime);
        } else if (arrival <= chain[other][j].r) {
            waitingBest[other][j] = max(waitingBest[other][j], i + 1);
        } else {
            push(other, j, i + 1, arrival);
        }
    };

    for (int side = 0; side < 2; ++side) {
        int64 time = max<int64>(llabs(start - chain[side][0].x), chain[side][0].r);
        push(side, 0, 0, time);
        for (int i = 0; i < (int)chain[side].size(); ++i) {
            queue.push({chain[side][i].r, side, i, -1, true});
        }
    }

    const int64 INF = numeric_limits<int64>::max() / 4;
    int64 answer = INF;

    while (!queue.empty()) {
        Entry entry = queue.top();
        queue.pop();
        if (answer <= entry.time) {
            break;
        }

        int side = entry.side;
        int other = 1 - side;
        if (entry.waiting) {
            int i = entry.i;
            int j = waitingBest[side][i];
            if (j < 0) {
                continue;
            }
            int64 time = chain[side][i].r;
            if (i == (int)chain[side].size() - 1) {
                int64 completion = time + llabs(chain[side][i].x - chain[other][j].x);
                answer = min(answer, completion);
            } else {
                expand(side, i, j, time);
            }
            continue;
        }

        int i = entry.i;
        int j = entry.j;
        auto label = best.find(keyOf(side, i, j));
        if (label == best.end() || label->second < entry.time) {
            continue;
        }

        int64 characteristic = entry.time + (side == 0 ? -chain[side][i].x : chain[side][i].x);
        auto firstGreater = upper_bound(
            chain[side].begin() + i,
            chain[side].end(),
            characteristic,
            [](int64 value, const Node &node) {
                return value < node.offset;
            }
        );
        int k = int(firstGreater - chain[side].begin());
        if (k == (int)chain[side].size()) {
            int64 completion = entry.time + llabs(chain[side][i].x - chain[other][j].x);
            answer = min(answer, completion);
            continue;
        }

        int skipped = k - 1;
        int64 skipTime = entry.time + llabs(chain[side][i].x - chain[side][skipped].x);
        expand(side, skipped, j, skipTime);
    }

    cout << answer << '\n';
    return 0;
}
