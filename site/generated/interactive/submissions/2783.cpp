#include <bits/stdc++.h>
using namespace std;

int n;

int ask(int p) {
    cout << "? " << p + 1 << '\n';
    cout.flush();

    int response;
    if (!(cin >> response) || response == -1) {
        exit(0);
    }
    return response;
}

vector<int> sweep(int p) {
    vector<int> response(n + 1);
    for (int k = 1; k <= n; ++k) {
        response[k] = ask(p);
    }
    return response;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) {
        return 0;
    }

    vector<int> reference = sweep(0);

    // Coordinates are relative to the initial value of dial 0.
    vector<int> occupied(n, 0);
    occupied[0] = 1;
    for (int k = 1; k < n; ++k) {
        occupied[k] = reference[k];
    }

    bool allOccupied = true;
    for (int k = 0; k < n; ++k) {
        if (!occupied[k]) {
            allOccupied = false;
        }
    }

    auto isRotation = [&](int shift) {
        if (!occupied[shift]) {
            return false;
        }
        for (int k = 1; k < n; ++k) {
            if (reference[k] != occupied[(shift + k) % n]) {
                return false;
            }
        }
        return true;
    };

    bool aperiodic = true;
    for (int shift = 1; shift < n; ++shift) {
        if (isRotation(shift)) {
            aperiodic = false;
            break;
        }
    }

    vector<vector<int>> responses;
    if (!allOccupied) {
        responses.resize(n);
        responses[0] = reference;
        for (int p = 1; p < n; ++p) {
            responses[p] = sweep(p);
        }
    }

    vector<int> position(n, -1);
    bool direct = false;

    if (!allOccupied && aperiodic) {
        direct = true;

        for (int p = 0; p < n && direct; ++p) {
            int found = -1;

            for (int d = 0; d < n; ++d) {
                if (!occupied[d]) {
                    continue;
                }

                bool matches = true;
                for (int k = 1; k < n; ++k) {
                    if (responses[p][k] != occupied[(d + k) % n]) {
                        matches = false;
                        break;
                    }
                }

                if (matches) {
                    if (found != -1) {
                        direct = false;
                        break;
                    }
                    found = d;
                }
            }

            if (found == -1) {
                direct = false;
            } else {
                position[p] = found;
            }
        }
    }

    if (direct) {
        vector<int> frequency(n, 0);
        for (int p = 0; p < n; ++p) {
            ++frequency[position[p]];
        }

        for (int p = 0; p < n; ++p) {
            if (responses[p][n] != (frequency[position[p]] >= 2)) {
                direct = false;
                break;
            }
        }
    }

    if (direct) {
        int target = 0;
        int best = INT_MAX;

        for (int t = 0; t < n; ++t) {
            int cost = 0;
            for (int p = 0; p < n; ++p) {
                cost += (t - position[p] + n) % n;
            }
            if (cost < best) {
                best = cost;
                target = t;
            }
        }

        for (int p = 0; p < n; ++p) {
            int steps = (target - position[p] + n) % n;
            for (int k = 0; k < steps; ++k) {
                ask(p);
            }
        }

        cout << "!" << '\n';
        cout.flush();
        return 0;
    }

    if (!allOccupied) {
        // Move every dial that originally belonged to a duplicated value
        // into an empty value. This leaves a permutation of all n values.
        for (int p = 0; p < n; ++p) {
            if (responses[p][n] == 1) {
                while (ask(p) != 0) {
                }
            }
        }
    }

    // The current state is a permutation. Move one dial to create a
    // unique empty coordinate, 0, and place this dial at coordinate 1.
    int x = 0;
    ask(x);

    vector<int> count(n, 1);
    count[0] = 0;
    count[1] = 2;

    int hole = 0;
    vector<int> finalPosition(n, -1);
    finalPosition[x] = 1;

    vector<int> others;
    for (int p = 0; p < n; ++p) {
        if (p != x) {
            others.push_back(p);
        }
    }

    // Determine coordinates while preserving exactly one empty coordinate.
    for (int i = 0; i + 1 < (int)others.size(); ++i) {
        int p = others[i];
        int steps = 0;

        while (true) {
            int response = ask(p);
            ++steps;
            if (response == 0) {
                break;
            }
        }

        int source = (hole - steps % n + n) % n;

        --count[source];
        ++count[hole];

        if (count[source] == 0) {
            finalPosition[p] = hole;
            hole = source;
        } else {
            int next = (hole + 1) % n;
            ask(p);
            --count[hole];
            ++count[next];
            finalPosition[p] = next;
        }
    }

    // The one unprocessed dial can now be located by subtracting all
    // already-known final positions from the coordinate counts.
    int last = others.back();
    vector<int> remaining = count;
    --remaining[finalPosition[x]];
    for (int i = 0; i + 1 < (int)others.size(); ++i) {
        --remaining[finalPosition[others[i]]];
    }

    for (int c = 0; c < n; ++c) {
        if (remaining[c] > 0) {
            finalPosition[last] = c;
            break;
        }
    }

    int target = 0;
    int best = INT_MAX;

    for (int t = 0; t < n; ++t) {
        int cost = 0;
        for (int p = 0; p < n; ++p) {
            cost += (t - finalPosition[p] + n) % n;
        }
        if (cost < best) {
            best = cost;
            target = t;
        }
    }

    for (int p = 0; p < n; ++p) {
        int steps = (target - finalPosition[p] + n) % n;
        for (int k = 0; k < steps; ++k) {
            ask(p);
        }
    }

    cout << "!" << '\n';
    cout.flush();
    return 0;
}
