#include <algorithm>
#include <array>
#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using Candidate = pair<int, int>;

struct State {
    vector<Candidate> candidates;
    int streak;

    bool operator<(const State& other) const {
        return tie(candidates, streak) < tie(other.candidates, other.streak);
    }
};

const array<int, 3> beats = {1, 2, 0};

int opponentMove(int id, int state) {
    if (id < 4) return state;
    if (state == 3) return 0;
    return id == 4 ? state : beats[state];
}

int nextState(int id, int state, int action) {
    if (id < 3) return state;
    if (id == 3) return (state + 1) % 3;
    return action;
}

vector<State> successors(const State& state, int action) {
    array<vector<Candidate>, 2> groups;
    for (auto [id, internal] : state.candidates) {
        bool wins = action == beats[opponentMove(id, internal)];
        groups[wins].push_back({id, nextState(id, internal, action)});
    }

    vector<State> result;
    for (int outcome = 0; outcome < 2; ++outcome) {
        if (!groups[outcome].empty()) {
            result.push_back({move(groups[outcome]), outcome ? state.streak + 1 : 0});
        }
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    State initial{{{0, 0}, {1, 1}, {2, 2}, {3, 0}, {4, 3}, {5, 3}}, 0};
    map<State, int> value;
    deque<State> pending{initial};
    value[initial] = 1000;

    while (!pending.empty()) {
        State state = move(pending.front());
        pending.pop_front();
        if (state.streak == 10) continue;
        for (int action = 0; action < 3; ++action) {
            for (State& next : successors(state, action)) {
                if (!value.count(next)) {
                    value[next] = next.streak == 10 ? 0 : 1000;
                    pending.push_back(next);
                }
            }
        }
    }

    map<State, int> choice;
    while (true) {
        bool changed = false;
        for (const auto& [state, oldValue] : value) {
            if (state.streak == 10) continue;
            int best = 1000;
            int bestSum = numeric_limits<int>::max();
            int bestAction = 0;
            for (int action = 0; action < 3; ++action) {
                vector<State> next = successors(state, action);
                int worst = 0;
                int sum = 0;
                for (const State& successor : next) {
                    worst = max(worst, value[successor]);
                    sum += value[successor];
                }
                if (1 + worst < best || (1 + worst == best && sum < bestSum)) {
                    best = 1 + worst;
                    bestSum = sum;
                    bestAction = action;
                }
            }
            if (best != oldValue) {
                value[state] = best;
                changed = true;
            }
            choice[state] = bestAction;
        }
        if (!changed) break;
    }

    State state = initial;
    string response;
    while (state.streak < 10) {
        int action = choice[state];
        cout << "RPS"[action] << '\n' << flush;
        if (!(cin >> response)) return 0;

        bool won = response == "player";
        vector<Candidate> remaining;
        for (auto [id, internal] : state.candidates) {
            bool beatsOpponent = action == beats[opponentMove(id, internal)];
            if (beatsOpponent == won) {
                remaining.push_back({id, nextState(id, internal, action)});
            }
        }
        state = {move(remaining), won ? state.streak + 1 : 0};
    }
}
