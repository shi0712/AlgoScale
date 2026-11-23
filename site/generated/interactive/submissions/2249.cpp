#include <bits/stdc++.h>
using namespace std;

static const char MOVE_CHAR[3] = {'R', 'P', 'S'};

vector<int> playerHistory;
vector<int> winHistory;
int winStreak = 0;
bool finished = false;
bool alive = true;

bool playMove(int move, bool &won) {
    if (!alive || finished) return false;

    cout << MOVE_CHAR[move] << '\n' << flush;

    string response;
    if (!(cin >> response)) {
        alive = false;
        return false;
    }
    if (response == "-1") {
        alive = false;
        return false;
    }
    if (response == "player") {
        won = true;
    } else if (response == "ai") {
        won = false;
    } else {
        alive = false;
        return false;
    }

    playerHistory.push_back(move);
    winHistory.push_back(won ? 1 : 0);
    winStreak = won ? winStreak + 1 : 0;
    if (winStreak >= 10) finished = true;
    return true;
}

void exploitFixed(int move) {
    while (alive && !finished) {
        bool won;
        if (!playMove(move, won)) return;
        if (!won) return;
    }
}

// step 1 exploits the forward cycle and copycat; step 2 exploits the counter strategy.
void exploitSequence(int lastMove, int step) {
    int last = lastMove;
    while (alive && !finished) {
        int move = (last + step) % 3;
        bool won;
        if (!playMove(move, won)) return;
        last = move;
        if (!won) return;
    }
}

struct Model {
    int kind = 0; // 0: periodic, 1: reacts to player's previous move, 2: reacts to its own previous move
    vector<int> sequence;
    int initial = 0;
    array<int, 3> mapping{0, 0, 0};
};

int modelMove(const Model &model, int round, const vector<int> &history) {
    if (model.kind == 0) {
        return model.sequence[round % (int)model.sequence.size()];
    }
    if (model.kind == 1) {
        if (round == 0) return model.initial;
        return model.mapping[history[round - 1]];
    }

    int move = model.initial;
    for (int i = 0; i < round; ++i) move = model.mapping[move];
    return move;
}

vector<Model> buildModels() {
    vector<Model> models;

    for (int length = 1; length <= 6; ++length) {
        int total = 1;
        for (int i = 0; i < length; ++i) total *= 3;

        for (int code = 0; code < total; ++code) {
            Model model;
            model.kind = 0;
            int value = code;
            for (int i = 0; i < length; ++i) {
                model.sequence.push_back(value % 3);
                value /= 3;
            }
            models.push_back(model);
        }
    }

    for (int kind = 1; kind <= 2; ++kind) {
        for (int initial = 0; initial < 3; ++initial) {
            for (int code = 0; code < 27; ++code) {
                Model model;
                model.kind = kind;
                model.initial = initial;
                int value = code;
                for (int i = 0; i < 3; ++i) {
                    model.mapping[i] = value % 3;
                    value /= 3;
                }
                models.push_back(model);
            }
        }
    }

    return models;
}

void fallbackStrategy() {
    vector<Model> models = buildModels();
    vector<int> consistent;

    for (int i = 0; i < (int)models.size(); ++i) {
        bool valid = true;
        for (int round = 0; round < (int)playerHistory.size(); ++round) {
            int ai = modelMove(models[i], round, playerHistory);
            bool predictedWin = (playerHistory[round] == (ai + 1) % 3);
            if (predictedWin != (winHistory[round] != 0)) {
                valid = false;
                break;
            }
        }
        if (valid) consistent.push_back(i);
    }

    while (alive && !finished && !consistent.empty()) {
        int round = playerHistory.size();
        vector<int> aiMoves;
        aiMoves.reserve(consistent.size());

        for (int index : consistent) {
            aiMoves.push_back(modelMove(models[index], round, playerHistory));
        }

        bool allSame = true;
        for (int ai : aiMoves) {
            if (ai != aiMoves[0]) {
                allSame = false;
                break;
            }
        }

        int chosenMove = 0;
        if (allSame) {
            chosenMove = (aiMoves[0] + 1) % 3;
        } else {
            int bestMove = -1;
            int bestLargestPart = INT_MAX;
            int bestWinners = -1;

            for (int move = 0; move < 3; ++move) {
                int winners = 0;
                for (int ai : aiMoves) {
                    if (move == (ai + 1) % 3) ++winners;
                }
                int losers = (int)aiMoves.size() - winners;
                if (winners == 0 || losers == 0) continue;

                int largestPart = max(winners, losers);
                if (largestPart < bestLargestPart ||
                    (largestPart == bestLargestPart && winners > bestWinners)) {
                    bestMove = move;
                    bestLargestPart = largestPart;
                    bestWinners = winners;
                }
            }

            if (bestMove == -1) {
                bestMove = 0;
                bestWinners = -1;
                for (int move = 0; move < 3; ++move) {
                    int winners = 0;
                    for (int ai : aiMoves) {
                        if (move == (ai + 1) % 3) ++winners;
                    }
                    if (winners > bestWinners) {
                        bestWinners = winners;
                        bestMove = move;
                    }
                }
            }
            chosenMove = bestMove;
        }

        bool won;
        if (!playMove(chosenMove, won)) return;
        if (finished) return;

        vector<int> nextConsistent;
        for (int i = 0; i < (int)consistent.size(); ++i) {
            bool predictedWin = (chosenMove == (aiMoves[i] + 1) % 3);
            if (predictedWin == won) nextConsistent.push_back(consistent[i]);
        }
        consistent.swap(nextConsistent);
    }

    // This is unreachable for the six strategies, but keeps the interaction well-formed
    // if an equivalent model was duplicated inconsistently.
    int last = playerHistory.empty() ? 0 : playerHistory.back();
    while (alive && !finished) {
        int move = (last + 1) % 3;
        bool won;
        if (!playMove(move, won)) return;
        last = move;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    bool won;

    // Round 1: paper beats the initial rock of four of the six strategies.
    if (!playMove(1, won)) return 0;

    if (!won) {
        // Only constant paper and constant scissors remain.
        if (!playMove(0, won)) return 0;
        exploitFixed(won ? 0 : 2);
    } else {
        // This gives immediate ten-round wins against the cycle and copycat.
        if (!playMove(2, won)) return 0;

        if (won) {
            exploitSequence(2, 1);
        } else {
            // Constant rock and the counter strategy both lose to paper here.
            if (!playMove(1, won)) return 0;

            if (won) {
                // Paper distinguishes them on the following round.
                if (!playMove(1, won)) return 0;
                if (won) {
                    exploitFixed(1);
                } else {
                    exploitSequence(1, 2);
                }
            }
        }
    }

    if (alive && !finished) fallbackStrategy();
    return 0;
}
