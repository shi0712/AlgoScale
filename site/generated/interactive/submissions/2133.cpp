#include <bits/stdc++.h>
using namespace std;

char beat(char c) {
    if (c == 'R') return 'P';
    if (c == 'P') return 'S';
    return 'R';               // c == 'S'
}
char lose(char c) {
    if (c == 'R') return 'S';
    if (c == 'P') return 'R';
    return 'P';               // c == 'S'
}

// send a move, flush, read answer, update streak, return true iff win
bool sendMove(char move, int &streak) {
    cout << move << "\n" << flush;
    string resp;
    if (!getline(cin, resp)) exit(0);          // EOF -> terminate
    if (resp == "-1") exit(0);                 // protocol error
    if (resp == "player") {
        ++streak;
        return true;
    } else { // "ai"
        streak = 0;
        return false;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<char> my;          // our moves history
    int streak = 0;           // consecutive wins

    // ----- round 1 -----
    my.push_back('R');
    bool win1 = sendMove('R', streak);
    if (win1) {
        // opponent is always S, we already have 1 win
        while (streak < 10) {
            my.push_back('R');
            sendMove('R', streak);
        }
        return 0;
    }

    // ----- round 2 -----
    my.push_back('P');
    bool win2 = sendMove('P', streak);
    if (win2) {
        // opponent is 0 (always R) or 3 (copy)
        // ----- round 3 -----
        my.push_back('P');
        bool win3 = sendMove('P', streak);
        if (win3) {
            // opponent always R
            // we have already 2 wins (round 2 and 3)
            while (streak < 10) {
                my.push_back('P');
                sendMove('P', streak);
            }
            return 0;
        } else {
            // opponent is copy
            // start winning streak from round 4 using beat(prev)
            while (streak < 10) {
                char prev = my.back();
                char nxt = beat(prev);
                my.push_back(nxt);
                sendMove(nxt, streak);
            }
            return 0;
        }
    } else {
        // opponent is 1 (always P), 4 (beat), or 5 (lose)
        // ----- round 3 -----
        my.push_back('R');
        bool win3 = sendMove('R', streak);
        if (win3) {
            // opponent is beat (strategy 4)
            // we have 1 win (round 3)
            while (streak < 10) {
                char prev = my.back();          // move we just played
                char nxt = lose(prev);          // lose(prev) beats beat(prev)
                my.push_back(nxt);
                sendMove(nxt, streak);
            }
            return 0;
        } else {
            // opponent is always P (1) or lose (5)
            // ----- round 4 -----
            my.push_back('R');
            bool win4 = sendMove('R', streak);
            if (win4) {
                // opponent is lose (strategy 5)
                // we have 1 win (round 4)
                while (streak < 10) {
                    // repeat the same move
                    char nxt = my.back();
                    my.push_back(nxt);
                    sendMove(nxt, streak);
                }
                return 0;
            } else {
                // opponent is always P (strategy 1)
                // start winning streak from round 5 using S
                while (streak < 10) {
                    my.push_back('S');
                    sendMove('S', streak);
                }
                return 0;
            }
        }
    }
}
