#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 2) {
        cout << "! 1 2\n" << flush;
        return 0;
    }

    auto ask = [&](const vector<int>& positions) -> int {
        cout << "? " << positions.size();
        for (int x : positions) cout << ' ' << x + 1;
        cout << '\n' << flush;

        int response;
        if (!(cin >> response) || response == -1) exit(0);
        if (response != 0 && response != 1) exit(0);
        return response;
    };

    vector<int> valueAt(n, -1), positionOf(n + 1, -1);
    int unknownCount = n;

    auto setKnown = [&](int position, int value) {
        if (valueAt[position] != -1) return;
        valueAt[position] = value;
        positionOf[value] = position;
        --unknownCount;
    };

    auto inferLastGlobally = [&]() {
        if (unknownCount != 1) return;
        int position = -1, value = -1;
        for (int i = 0; i < n; ++i)
            if (valueAt[i] == -1) position = i;
        for (int v = 1; v <= n; ++v)
            if (positionOf[v] == -1) value = v;
        setKnown(position, value);
    };

    vector<int> extremes;
    for (int x = 0; x < n && (int)extremes.size() < 2; ++x) {
        int left = n - x;
        int needed = 2 - (int)extremes.size();
        if (left == needed) {
            for (int y = x; y < n; ++y) extremes.push_back(y);
            break;
        }

        vector<int> query;
        query.reserve(n - 1);
        for (int i = 0; i < n; ++i)
            if (i != x) query.push_back(i);

        if (ask(query) == 1) extremes.push_back(x);
    }

    setKnown(extremes[0], 1);
    setKnown(extremes[1], n);

    int nextSmall = 2;

    auto advanceSmall = [&]() {
        while (nextSmall < n && positionOf[nextSmall] != -1) ++nextSmall;
    };

    auto peelNextSmall = [&]() -> bool {
        advanceSmall();
        if (nextSmall >= n) return false;

        int target = nextSmall;
        vector<char> excluded(n, false);
        for (int v = 1; v < target; ++v)
            excluded[positionOf[v]] = true;

        vector<int> universal, candidates;
        for (int i = 0; i < n; ++i) {
            if (!excluded[i]) {
                universal.push_back(i);
                if (valueAt[i] == -1) candidates.push_back(i);
            }
        }

        if (candidates.empty()) return false;

        for (int i = 0; i < (int)candidates.size(); ++i) {
            int x = candidates[i];
            if (i + 1 == (int)candidates.size()) {
                setKnown(x, target);
                break;
            }

            vector<int> query;
            query.reserve(universal.size() - 1);
            for (int y : universal)
                if (y != x) query.push_back(y);

            if (ask(query) == 1) {
                setKnown(x, target);
                break;
            }
        }

        nextSmall = target + 1;
        advanceSmall();
        return true;
    };

    auto findNMinusOne = [&]() {
        if (positionOf[n - 1] != -1) return;

        vector<int> candidates;
        for (int i = 0; i < n; ++i)
            if (valueAt[i] == -1) candidates.push_back(i);

        for (int i = 0; i < (int)candidates.size(); ++i) {
            int x = candidates[i];
            if (i + 1 == (int)candidates.size()) {
                setKnown(x, n - 1);
                return;
            }

            vector<int> query;
            query.reserve(n - 2);
            for (int y = 0; y < n; ++y)
                if (y != positionOf[n] && y != x) query.push_back(y);

            if (ask(query) == 1) {
                setKnown(x, n - 1);
                return;
            }
        }
    };

    auto freeInference = [&](int modulus, const vector<vector<int>>& classes) {
        for (int r = 0; r < modulus; ++r) {
            vector<int> unknownPositions;
            for (int x : classes[r])
                if (valueAt[x] == -1) unknownPositions.push_back(x);

            if (unknownPositions.size() != 1) continue;

            int remainingValue = -1;
            int first = (r == 0 ? modulus : r);
            for (int v = first; v <= n; v += modulus) {
                if (positionOf[v] == -1) {
                    remainingValue = v;
                    break;
                }
            }
            if (remainingValue != -1)
                setKnown(unknownPositions[0], remainingValue);
        }
    };

    vector<int> residue(n, 0);
    int modulus = 1;
    mt19937_64 rng(0x9e3779b97f4a7c15ULL);

    auto solveBase = [&](int M, const vector<vector<int>>& classes) {
        array<int, 3> pairA{}, pairB{};

        auto findFillerPairs = [&]() {
            pairA.fill(-1);
            pairB.fill(-1);
            for (int i = 0; i < n; ++i) {
                if (valueAt[i] == -1) continue;
                for (int j = i + 1; j < n; ++j) {
                    if (valueAt[j] == -1) continue;
                    int target = (3 - (valueAt[i] + valueAt[j]) % 3) % 3;
                    if (pairA[target] == -1) {
                        pairA[target] = i;
                        pairB[target] = j;
                    }
                }
            }
            return pairA[0] != -1 && pairA[1] != -1 && pairA[2] != -1;
        };

        while (unknownCount > 1) {
            if (findFillerPairs()) break;

            if (positionOf[2] == -1) {
                peelNextSmall();
            } else if (positionOf[n - 1] == -1) {
                findNMinusOne();
            } else {
                if (!peelNextSmall()) break;
            }

            freeInference(M, classes);
            inferLastGlobally();
        }

        if (unknownCount <= 1) {
            inferLastGlobally();
            return;
        }
        if (!findFillerPairs()) return;

        for (int r = 0; r < M; ++r) {
            vector<int> positions, values;

            for (int x : classes[r])
                if (valueAt[x] == -1) positions.push_back(x);

            int first = (r == 0 ? M : r);
            for (int v = first; v <= n; v += M)
                if (positionOf[v] == -1) values.push_back(v);

            while (positions.size() > 1) {
                int x = positions.back();
                positions.pop_back();

                int chosen = -1;
                while (values.size() > 1) {
                    int candidate = values.back();
                    int target = candidate % 3;

                    vector<int> query = {
                        x, pairA[target], pairB[target]
                    };

                    if (ask(query) == 1) {
                        chosen = candidate;
                        values.pop_back();
                        break;
                    }
                    values.pop_back();
                }

                if (chosen == -1) {
                    chosen = values.back();
                    values.pop_back();
                }
                setKnown(x, chosen);
            }

            if (!positions.empty())
                setKnown(positions[0], values[0]);
        }
    };

    constexpr int MAXN = 800;

    while (unknownCount > 1) {
        inferLastGlobally();
        if (unknownCount <= 1) break;

        vector<vector<int>> classes(modulus);
        for (int i = 0; i < n; ++i)
            classes[residue[i]].push_back(i);

        freeInference(modulus, classes);
        inferLastGlobally();
        if (unknownCount <= 1) break;

        int maximumClassSize = 0;
        for (auto& c : classes)
            maximumClassSize = max(maximumClassSize, (int)c.size());

        if (maximumClassSize <= 3) {
            solveBase(modulus, classes);
            break;
        }

        const int M = modulus;
        const int groupSize = 2 * M;

        bitset<MAXN> basis[MAXN];
        int basisRhs[MAXN]{};
        int rank = 0;

        auto addEquation = [&](bitset<MAXN> v, int rhs) -> bool {
            for (int i = 0; i < n; ++i) {
                if (!v[i]) continue;
                if (basis[i].any()) {
                    v ^= basis[i];
                    rhs ^= basisRhs[i];
                } else {
                    basis[i] = v;
                    basisRhs[i] = rhs;
                    ++rank;
                    return true;
                }
            }
            return false;
        };

        auto independent = [&](bitset<MAXN> v) -> bool {
            for (int i = 0; i < n; ++i) {
                if (!v[i]) continue;
                if (basis[i].any()) v ^= basis[i];
                else return true;
            }
            return false;
        };

        for (int r = 0; r < M; ++r) {
            bitset<MAXN> equation;
            for (int x : classes[r]) equation.set(x);

            int rhs = 0;
            int first = (r == 0 ? M : r);
            for (int v = first; v <= n; v += M)
                rhs ^= (((v - r) / M) & 1);

            addEquation(equation, rhs);
        }

        auto addKnownUnits = [&]() {
            for (int i = 0; i < n; ++i) {
                if (valueAt[i] == -1) continue;
                int bit = ((valueAt[i] - residue[i]) / M) & 1;
                bitset<MAXN> unit;
                unit.set(i);
                addEquation(unit, bit);
            }
        };
        addKnownUnits();

        vector<int> sample(n);
        iota(sample.begin(), sample.end(), 0);
        vector<int> mark(n, 0);
        int stamp = 0;
        int stale = 0;

        while (rank < n && unknownCount > 1) {
            if (stale >= 1200) {
                stale = 0;
                bool more = peelNextSmall();
                freeInference(M, classes);
                inferLastGlobally();
                addKnownUnits();
                if (!more && unknownCount > 1) break;
                continue;
            }

            ++stamp;
            vector<int> group;
            group.reserve(groupSize);
            int residueSum = 0;

            for (int j = 0; j < groupSize - 1; ++j) {
                uniform_int_distribution<int> pick(j, n - 1);
                int at = pick(rng);
                swap(sample[j], sample[at]);

                int x = sample[j];
                group.push_back(x);
                mark[x] = stamp;
                residueSum += residue[x];
            }

            int needed = (M - residueSum % M) % M;
            int last = -1;

            auto& candidates = classes[needed];
            for (int attempt = 0; attempt < 8 && !candidates.empty(); ++attempt) {
                int x = candidates[rng() % candidates.size()];
                if (mark[x] != stamp) {
                    last = x;
                    break;
                }
            }
            if (last == -1) {
                for (int x : candidates) {
                    if (mark[x] != stamp) {
                        last = x;
                        break;
                    }
                }
            }

            if (last == -1) {
                ++stale;
                continue;
            }

            group.push_back(last);
            residueSum += needed;

            bitset<MAXN> equation;
            for (int x : group) equation.set(x);

            if (!independent(equation)) {
                ++stale;
                continue;
            }

            int response = ask(group);
            int rhs = ((residueSum / M) & 1) ^ (response == 0);
            addEquation(equation, rhs);
            stale = 0;
        }

        if (rank < n) break;
        if (unknownCount <= 1) break;

        vector<int> bits(n);
        for (int i = n - 1; i >= 0; --i) {
            int value = basisRhs[i];
            for (int j = i + 1; j < n; ++j)
                if (basis[i][j]) value ^= bits[j];
            bits[i] = value;
        }

        for (int i = 0; i < n; ++i)
            residue[i] += M * bits[i];

        modulus *= 2;
    }

    while (unknownCount > 1) {
        if (!peelNextSmall()) break;
    }
    inferLastGlobally();

    if (valueAt[0] > n / 2) {
        for (int& v : valueAt) v = n + 1 - v;
    }

    cout << "!";
    for (int v : valueAt) cout << ' ' << v;
    cout << '\n' << flush;

    return 0;
}
