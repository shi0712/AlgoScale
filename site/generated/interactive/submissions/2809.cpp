#include <bits/stdc++.h>
using namespace std;

int n;
int areaTable[5][5][5];
vector<array<int, 3>> responseMatches[769];

int area16(int a, int b, int c) {
    array<int, 3> s = {a, b, c};
    sort(s.begin(), s.end());
    if (s[0] + s[1] <= s[2]) return 0;
    return (s[0] + s[1] + s[2])
         * (-s[0] + s[1] + s[2])
         * (s[0] - s[1] + s[2])
         * (s[0] + s[1] - s[2]);
}

int ask(int a, int b, int c) {
    if (a > b) swap(a, b);
    if (b > c) swap(b, c);
    if (a > b) swap(a, b);

    cout << "? " << a << ' ' << b << ' ' << c << endl;
    int response;
    if (!(cin >> response) || response == -1) exit(0);
    return response;
}

void printAnswer(const vector<int>& answer) {
    cout << "!";
    for (int x : answer) cout << ' ' << x;
    cout << endl;
}

void printImpossible() {
    cout << "! -1" << endl;
}

vector<array<int, 3>> getPermutations(int response) {
    vector<array<int, 3>> result;
    if (response <= 0 || response >= 769) return result;

    for (const auto& sortedValues : responseMatches[response]) {
        array<int, 3> p = sortedValues;
        do {
            result.push_back(p);
        } while (next_permutation(p.begin(), p.end()));
    }
    return result;
}

bool hasUsefulWitness(int response) {
    if (response <= 0 || response >= 769) return false;

    for (const auto& values : responseMatches[response]) {
        for (int i = 0; i < 3; ++i) {
            for (int j = i + 1; j < 3; ++j) {
                if (values[i] == values[j] && values[i] >= 2) {
                    return true;
                }
            }
        }
    }
    return false;
}

struct Candidate {
    array<int, 3> permutation;
    int externalValue;
};

bool tryOrientWitness(const array<int, 3>& indices, int response,
                      mt19937_64& rng, array<int, 3>& result) {
    vector<array<int, 3>> permutations = getPermutations(response);
    if (permutations.empty()) return false;

    if (permutations.size() == 1) {
        result = permutations[0];
        return true;
    }

    vector<int> triedExternal;
    const int pairIndices[3][2] = {{0, 1}, {0, 2}, {1, 2}};

    for (int trial = 0; trial < 3; ++trial) {
        int external;
        do {
            external = int(rng() % n);
        } while (external == indices[0] ||
                 external == indices[1] ||
                 external == indices[2] ||
                 find(triedExternal.begin(), triedExternal.end(), external)
                     != triedExternal.end());

        triedExternal.push_back(external);

        vector<Candidate> candidates;
        for (const auto& p : permutations) {
            for (int value = 1; value <= 4; ++value) {
                candidates.push_back({p, value});
            }
        }

        for (const auto& pair : pairIndices) {
            int received = ask(indices[pair[0]], indices[pair[1]], external);

            vector<Candidate> filtered;
            filtered.reserve(candidates.size());
            for (const auto& candidate : candidates) {
                int expected = areaTable[candidate.permutation[pair[0]]]
                                      [candidate.permutation[pair[1]]]
                                      [candidate.externalValue];
                if (expected == received) filtered.push_back(candidate);
            }

            candidates.swap(filtered);
            if (candidates.empty()) return false;

            bool samePermutation = true;
            for (const auto& candidate : candidates) {
                if (candidate.permutation != candidates[0].permutation) {
                    samePermutation = false;
                    break;
                }
            }
            if (samePermutation) {
                result = candidates[0].permutation;
                return true;
            }
        }
    }

    return false;
}

int decodeUsingEqualPair(int pairValue, int response) {
    if (pairValue == 2) {
        if (response == 15) return 1;
        if (response == 48) return 2;
        if (response == 63) return 3;
        if (response == 0) return 4;
    } else if (pairValue == 3) {
        if (response == 35) return 1;
        if (response == 128) return 2;
        if (response == 243) return 3;
        if (response == 320) return 4;
    } else if (pairValue == 4) {
        if (response == 63) return 1;
        if (response == 240) return 2;
        if (response == 495) return 3;
        if (response == 768) return 4;
    }
    return -1;
}

void solveUsingAllTriples() {
    vector<array<int, 3>> triples;
    vector<int> responses;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            for (int k = j + 1; k < n; ++k) {
                triples.push_back({i, j, k});
                responses.push_back(ask(i, j, k));
            }
        }
    }

    vector<vector<int>> endingAt(n);
    for (int t = 0; t < (int)triples.size(); ++t) {
        endingAt[triples[t][2]].push_back(t);
    }

    vector<int> current(n), best;
    int numberOfSolutions = 0;
    bool stop = false;

    function<void(int)> dfs = [&](int position) {
        if (stop) return;

        if (position == n) {
            ++numberOfSolutions;
            if (numberOfSolutions == 1) best = current;
            if (numberOfSolutions == 2) stop = true;
            return;
        }

        for (int value = 1; value <= 4; ++value) {
            current[position] = value;
            bool valid = true;

            for (int id : endingAt[position]) {
                const auto& t = triples[id];
                if (areaTable[current[t[0]]][current[t[1]]][current[t[2]]]
                    != responses[id]) {
                    valid = false;
                    break;
                }
            }

            if (valid) dfs(position + 1);
            if (stop) return;
        }
    };

    dfs(0);

    if (numberOfSolutions == 1) printAnswer(best);
    else printImpossible();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    for (int a = 1; a <= 4; ++a) {
        for (int b = 1; b <= 4; ++b) {
            for (int c = 1; c <= 4; ++c) {
                areaTable[a][b][c] = area16(a, b, c);
            }
        }
    }

    for (int a = 1; a <= 4; ++a) {
        for (int b = a; b <= 4; ++b) {
            for (int c = b; c <= 4; ++c) {
                int response = areaTable[a][b][c];
                if (response > 0) {
                    responseMatches[response].push_back({a, b, c});
                }
            }
        }
    }

    if (n <= 5) {
        solveUsingAllTriples();
        return 0;
    }

    uint64_t seed = chrono::steady_clock::now().time_since_epoch().count();
    seed ^= uint64_t(uintptr_t(&n));
    mt19937_64 rng(seed);

    const int maximumSearchQueries = max(1000, 8 * n);

    for (int searchQuery = 0; searchQuery < maximumSearchQueries; ++searchQuery) {
        array<int, 3> indices;
        do {
            indices = {int(rng() % n), int(rng() % n), int(rng() % n)};
            sort(indices.begin(), indices.end());
        } while (indices[0] == indices[1] ||
                 indices[1] == indices[2]);

        int response = ask(indices[0], indices[1], indices[2]);
        if (!hasUsefulWitness(response)) continue;

        array<int, 3> permutation;
        if (!tryOrientWitness(indices, response, rng, permutation)) continue;

        int repeatedValue = -1;
        int first = -1, second = -1;

        for (int i = 0; i < 3; ++i) {
            for (int j = i + 1; j < 3; ++j) {
                if (permutation[i] == permutation[j] &&
                    permutation[i] >= 2) {
                    repeatedValue = permutation[i];
                    first = indices[i];
                    second = indices[j];
                    break;
                }
            }
            if (first != -1) break;
        }

        if (first == -1) continue;

        vector<int> answer(n, -1);
        for (int i = 0; i < 3; ++i) {
            answer[indices[i]] = permutation[i];
        }

        bool valid = true;
        for (int i = 0; i < n; ++i) {
            if (answer[i] != -1) continue;

            int value = decodeUsingEqualPair(
                repeatedValue, ask(first, second, i)
            );
            if (value == -1) {
                valid = false;
                break;
            }
            answer[i] = value;
        }

        if (valid) printAnswer(answer);
        else printImpossible();
        return 0;
    }

    if (n <= 12) solveUsingAllTriples();
    else printImpossible();

    return 0;
}
