#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

using int64 = long long;
static constexpr int64 INF = numeric_limits<int64>::max() / 4;

struct MinCostFlow {
    struct Edge { int to, rev, cap; int64 cost; };
    vector<vector<Edge>> graph;
    explicit MinCostFlow(int n) : graph(n) {}

    int add_edge(int from, int to, int cap, int64 cost) {
        int id = static_cast<int>(graph[from].size());
        graph[from].push_back({to, static_cast<int>(graph[to].size()), cap, cost});
        graph[to].push_back({from, id, 0, -cost});
        return id;
    }

    pair<int, int64> flow(int source, int sink, int required) {
        int n = static_cast<int>(graph.size()), sent = 0;
        int64 total_cost = 0;
        vector<int64> potential(n), distance(n);
        vector<int> previous_vertex(n), previous_edge(n);
        while (sent < required) {
            fill(distance.begin(), distance.end(), INF);
            priority_queue<pair<int64, int>, vector<pair<int64, int>>,
                           greater<pair<int64, int>>> heap;
            distance[source] = 0;
            heap.push({0, source});
            while (!heap.empty()) {
                auto [dist, vertex] = heap.top();
                heap.pop();
                if (dist != distance[vertex]) continue;
                for (int i = 0; i < static_cast<int>(graph[vertex].size()); ++i) {
                    const Edge &edge = graph[vertex][i];
                    if (edge.cap == 0) continue;
                    int64 next_dist = dist + edge.cost + potential[vertex] - potential[edge.to];
                    if (next_dist < distance[edge.to]) {
                        distance[edge.to] = next_dist;
                        previous_vertex[edge.to] = vertex;
                        previous_edge[edge.to] = i;
                        heap.push({next_dist, edge.to});
                    }
                }
            }
            if (distance[sink] == INF) break;
            for (int v = 0; v < n; ++v)
                if (distance[v] != INF) potential[v] += distance[v];

            int add = required - sent;
            for (int v = sink; v != source; v = previous_vertex[v])
                add = min(add, graph[previous_vertex[v]][previous_edge[v]].cap);
            for (int v = sink; v != source; v = previous_vertex[v]) {
                Edge &edge = graph[previous_vertex[v]][previous_edge[v]];
                total_cost += static_cast<int64>(add) * edge.cost;
                edge.cap -= add;
                graph[v][edge.rev].cap += add;
            }
            sent += add;
        }
        return {sent, total_cost};
    }
};

struct Casino {
    int first = -1, second = -1;
    int64 cancel_cost = 0, win_cost = 1;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int test_count = 1;
    while (test_count--) {
        int casino_count, opponent_count;
        cin >> casino_count >> opponent_count;
        vector<Casino> casinos(casino_count);

        for (Casino &casino : casinos) {
            vector<pair<int, int>> row(opponent_count);
            for (int player = 0; player < opponent_count; ++player) {
                cin >> row[player].first;
                row[player].second = player;
            }
            sort(row.begin(), row.end());

            for (int left = 0, right; left < opponent_count; left = right) {
                right = left + 1;
                while (right < opponent_count && row[right].first == row[left].first) ++right;
                if (right == left + 1 && row[left].first > 0) {
                    casino.second = casino.first;
                    casino.first = row[left].second;
                    casino.cancel_cost = row[left].first;
                }
            }

            casino.win_cost = casino.cancel_cost + 1;
            for (const auto &[value, player] : row) {
                (void)player;
                if (value == casino.win_cost) ++casino.win_cost;
                else if (value > casino.win_cost) break;
            }
        }

        // Only players that are first or second surviving candidates matter.
        vector<int> compressed_id(opponent_count, -1);
        int relevant_players = 0;
        for (Casino &casino : casinos) {
            for (int *player : {&casino.first, &casino.second}) {
                if (*player != -1) {
                    if (compressed_id[*player] == -1)
                        compressed_id[*player] = relevant_players++;
                    *player = compressed_id[*player];
                }
            }
        }

        auto solve = [&](int own_wins, vector<int64> *answer) -> int64 {
            int source = 0;
            int casino_base = 1;
            int player_base = casino_base + casino_count;
            int nobody = player_base + relevant_players;
            int self = nobody + 1;
            int hub = self + 1;
            int sink = hub + 1;
            MinCostFlow mcf(sink + 1);
            vector<vector<pair<int, int64>>> choices(casino_count);

            auto player_node = [&](int player) {
                return player == -1 ? nobody : player_base + player;
            };
            for (int i = 0; i < casino_count; ++i) {
                int node = casino_base + i;
                mcf.add_edge(source, node, 1, 0);
                auto add_choice = [&](int to, int64 dice) {
                    int edge_id = mcf.add_edge(node, to, 1, dice);
                    choices[i].push_back({edge_id, dice});
                };
                add_choice(player_node(casinos[i].first), 0);
                if (casinos[i].first != -1)
                    add_choice(player_node(casinos[i].second), casinos[i].cancel_cost);
                add_choice(self, casinos[i].win_cost);
            }
            for (int player = 0; player < relevant_players; ++player)
                mcf.add_edge(player_base + player, hub, own_wins, 0);
            mcf.add_edge(nobody, hub, casino_count, 0);
            mcf.add_edge(self, sink, own_wins, 0);
            mcf.add_edge(hub, sink, casino_count - own_wins, 0);

            auto [sent, cost] = mcf.flow(source, sink, casino_count);
            if (sent != casino_count) return INF;
            if (answer != nullptr) {
                answer->assign(casino_count, 0);
                for (int i = 0; i < casino_count; ++i) {
                    int node = casino_base + i;
                    for (auto [edge_id, dice] : choices[i])
                        if (mcf.graph[node][edge_id].cap == 0) (*answer)[i] = dice;
                }
            }
            return cost;
        };

        vector<int64> memo(casino_count + 1, -1);
        auto value = [&](int wins) -> int64 {
            if (memo[wins] == -1) memo[wins] = solve(wins, nullptr);
            return memo[wins];
        };

        int low = 0, high = (casino_count + 1) / 2;
        while (low < high) {
            int middle = (low + high) / 2;
            int64 current = value(middle);
            if (current == INF || current > value(middle + 1)) low = middle + 1;
            else high = middle;
        }

        vector<int64> answer;
        solve(low, &answer);
        for (int i = 0; i < casino_count; ++i)
            cout << answer[i] << (i + 1 == casino_count ? '\n' : ' ');
    }
}
