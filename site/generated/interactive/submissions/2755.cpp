#include <bits/stdc++.h>
using namespace std;

bool play(char move) {
    cout << move << endl;

    string response;
    if (!(cin >> response)) {
        exit(0);
    }

    if (response == "player") return true;
    if (response == "ai") return false;

    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int streak = 0;

    auto makeMove = [&](char move) {
        streak = play(move) ? streak + 1 : 0;
    };

    auto repeatUntilTen = [&](char move) {
        while (streak < 10) {
            makeMove(move);
        }
    };

    auto playSequenceUntilTen = [&](const string& sequence) {
        for (char move : sequence) {
            if (streak == 10) break;
            makeMove(move);
        }
    };

    // First probe.
    makeMove('P');

    if (streak == 0) {
        // The opponent is always Paper or always Rock.
        makeMove('R');

        if (streak > 0) {
            // Always Paper: Rock wins repeatedly.
            repeatUntilTen('R');
        } else {
            // Always Rock: Paper wins repeatedly.
            repeatUntilTen('P');
        }
        return 0;
    }

    // The opponent is always Scissors or uses a strategy depending
    // on the previous player move.
    makeMove('R');

    if (streak > 0) {
        // Always Scissors or the counter-strategy.
        makeMove('R');

        if (streak > 0) {
            // Always Scissors.
            repeatUntilTen('R');
        } else {
            // Counter-strategy: answer the previous move with the
            // move that beats it. Our moves must beat that counter.
            playSequenceUntilTen("SPR");
        }
    } else {
        // Copycat or the strategy that deliberately loses.
        makeMove('R');

        if (streak > 0) {
            // Deliberate-loser strategy.
            repeatUntilTen('R');
        } else {
            // Copycat: answer the previous move with the move that
            // beats it.
            playSequenceUntilTen("PSR");
        }
    }

    return 0;
}
