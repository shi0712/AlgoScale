#include <bits/stdc++.h>
using namespace std;

int n, k;
int targetPos = 0, wheel1Pos = 0;
set<int> distinctPos;

int rotationDelta(int to, int from) {
    int d = (to - from) % n;
    if (d < 0) d += n;
    if (d > n / 2) d -= n;
    return d;
}

int ask(int wheel, int amount) {
    cout << wheel << ' ' << amount << '\n' << flush;
    int response;
    if (!(cin >> response)) exit(0);
    if (response <= 1) exit(0);
    return response;
}

[[noreturn]] void bruteForceFallback() {
    vector<int> digit(n + 1, 0);
    while (true) {
        int i = 2;
        while (true) {
            ask(i, 1);
            ++digit[i];
            if (digit[i] < n) break;
            digit[i] = 0;
            ++i;
            if (i > n) exit(0);
        }
    }
}

struct Hypothesis {
    int p;
    bool singleton;
};

vector<Hypothesis> makeHypotheses(const set<int>& positions, int markerPos) {
    vector<Hypothesis> result;
    for (int p : positions) {
        if (p == markerPos) continue;
        result.push_back({p, false});
        result.push_back({p, true});
    }
    return result;
}

int predictedResponse(const Hypothesis& h, const set<int>& positions, int shift) {
    int pos = (h.p + shift) % n;
    bool occupied = positions.count(pos) && !(h.singleton && pos == h.p);
    int base = (int)positions.size() - (h.singleton ? 1 : 0);
    return base + (occupied ? 0 : 1);
}

int numberOfPositions(const vector<Hypothesis>& hypotheses) {
    bool seen[50] = {};
    int count = 0;
    for (const auto& h : hypotheses) {
        if (!seen[h.p]) {
            seen[h.p] = true;
            ++count;
        }
    }
    return count;
}

bool isAperiodic(const set<int>& positions) {
    if (positions.empty()) return false;
    for (int d = 1; d < n; ++d) {
        bool periodic = true;
        for (int x : positions) {
            if (!positions.count((x + d) % n)) {
                periodic = false;
                break;
            }
        }
        if (periodic) return false;
    }
    return true;
}

bool informativeState(const set<int>& positions, int markerPos) {
    vector<Hypothesis> hypotheses = makeHypotheses(positions, markerPos);
    if (numberOfPositions(hypotheses) <= 1) return true;
    if (isAperiodic(positions)) return true;

    map<vector<int>, uint64_t> signaturePositions;
    for (const auto& h : hypotheses) {
        vector<int> signature(n);
        for (int s = 0; s < n; ++s) {
            signature[s] = predictedResponse(h, positions, s);
        }
        uint64_t& mask = signaturePositions[signature];
        mask |= (1ULL << h.p);
        if (mask & (mask - 1)) return false;
    }
    return true;
}

void ensureInformative() {
    int markerPos = (wheel1Pos == targetPos ? -1 : wheel1Pos);
    if (informativeState(distinctPos, markerPos)) return;

    set<int> base = distinctPos;
    if (markerPos >= 0) base.erase(markerPos);

    if (markerPos >= 0 && informativeState(base, -1)) {
        k = ask(1, rotationDelta(targetPos, wheel1Pos));
        wheel1Pos = targetPos;
        distinctPos = base;
        if (k != (int)distinctPos.size()) bruteForceFallback();
        return;
    }

    for (int x = 0; x < n; ++x) {
        if (base.count(x)) continue;
        set<int> candidate = base;
        candidate.insert(x);
        if (!informativeState(candidate, x)) continue;

        k = ask(1, rotationDelta(x, wheel1Pos));
        wheel1Pos = x;
        distinctPos = candidate;
        if (k != (int)distinctPos.size()) bruteForceFallback();
        return;
    }

    bruteForceFallback();
}

void processWheel(int wheel) {
    ensureInformative();

    int markerPos = (wheel1Pos == targetPos ? -1 : wheel1Pos);
    vector<Hypothesis> candidates = makeHypotheses(distinctPos, markerPos);
    if (candidates.empty()) bruteForceFallback();

    vector<char> tested(n, false);
    tested[0] = true;
    int currentShift = 0;

    using Score = tuple<int, int, long long, long long, int, int>;

    while (numberOfPositions(candidates) > 1) {
        int bestShift = -1;
        Score bestScore{};
        bool first = true;

        for (int s = 0; s < n; ++s) {
            if (tested[s]) continue;

            map<int, vector<int>> groups;
            for (int idx = 0; idx < (int)candidates.size(); ++idx) {
                int response = predictedResponse(candidates[idx], distinctPos, s);
                groups[response].push_back(idx);
            }

            int worstPositions = 0;
            int worstHypotheses = 0;
            long long weightedPositions = 0;
            long long sumSquares = 0;

            for (const auto& entry : groups) {
                const vector<int>& group = entry.second;
                bool seen[50] = {};
                int positionCount = 0;
                for (int idx : group) {
                    int p = candidates[idx].p;
                    if (!seen[p]) {
                        seen[p] = true;
                        ++positionCount;
                    }
                }
                worstPositions = max(worstPositions, positionCount);
                worstHypotheses = max(worstHypotheses, (int)group.size());
                weightedPositions += (long long)group.size() * positionCount;
                sumSquares += (long long)group.size() * group.size();
            }

            int distance = (s - currentShift + n) % n;
            Score score{
                worstPositions,
                worstHypotheses,
                weightedPositions,
                sumSquares,
                -(int)groups.size(),
                distance
            };

            if (first || score < bestScore) {
                first = false;
                bestScore = score;
                bestShift = s;
            }
        }

        if (bestShift < 0) bruteForceFallback();

        k = ask(wheel, rotationDelta(bestShift, currentShift));
        currentShift = bestShift;
        tested[currentShift] = true;

        vector<Hypothesis> filtered;
        for (const auto& h : candidates) {
            if (predictedResponse(h, distinctPos, currentShift) == k) {
                filtered.push_back(h);
            }
        }
        candidates.swap(filtered);
        if (candidates.empty()) bruteForceFallback();
    }

    int p = candidates.front().p;
    int oldSize = (int)distinctPos.size();
    int currentPosition = (p + currentShift) % n;
    int amount = rotationDelta(targetPos, currentPosition);

    if (amount != 0) {
        k = ask(wheel, amount);
    }

    if (p != targetPos) {
        if (k == oldSize - 1) {
            distinctPos.erase(p);
        } else if (k != oldSize) {
            bruteForceFallback();
        }
    }

    if (k != (int)distinctPos.size()) bruteForceFallback();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (!(cin >> k)) return 0;
    if (k <= 1) return 0;

    vector<int> responseAt(n, -1);
    responseAt[0] = k;
    int currentShift = 0;
    set<int> otherPositions;
    bool learned = false;

    while (!learned) {
        bool haveUnique = false;
        bool ambiguous = false;
        set<int> uniqueSet;

        for (int m = 1; m <= n - 1 && !ambiguous; ++m) {
            vector<int> occupied;
            bool valid = true;

            for (int s = 0; s < n; ++s) {
                if (responseAt[s] < 0) continue;
                if (responseAt[s] == m) {
                    occupied.push_back(s);
                } else if (responseAt[s] != m + 1) {
                    valid = false;
                    break;
                }
            }
            if (!valid) continue;

            vector<int> freePositions;
            for (int s = 0; s < n; ++s) {
                if (responseAt[s] < 0) freePositions.push_back(s);
            }

            int needed = m - (int)occupied.size();
            if (needed < 0 || needed > (int)freePositions.size()) continue;

            if (needed > 0 && needed < (int)freePositions.size()) {
                ambiguous = true;
                break;
            }

            set<int> candidate(occupied.begin(), occupied.end());
            if (needed == (int)freePositions.size()) {
                candidate.insert(freePositions.begin(), freePositions.end());
            }

            if (haveUnique && candidate != uniqueSet) {
                ambiguous = true;
                break;
            }
            haveUnique = true;
            uniqueSet = candidate;
        }

        if (haveUnique && !ambiguous) {
            otherPositions = uniqueSet;
            learned = true;
            break;
        }

        int nextShift = -1;
        for (int step = 1; step < n; ++step) {
            int s = (currentShift + step) % n;
            if (responseAt[s] < 0) {
                nextShift = s;
                break;
            }
        }
        if (nextShift < 0) bruteForceFallback();

        k = ask(1, rotationDelta(nextShift, currentShift));
        currentShift = nextShift;
        responseAt[currentShift] = k;
    }

    wheel1Pos = currentShift;
    int expected = (int)otherPositions.size() +
                   (otherPositions.count(wheel1Pos) ? 0 : 1);
    if (k != expected || otherPositions.empty()) bruteForceFallback();

    if (otherPositions.count(wheel1Pos)) {
        targetPos = wheel1Pos;
    } else {
        targetPos = *otherPositions.begin();
        k = ask(1, rotationDelta(targetPos, wheel1Pos));
        wheel1Pos = targetPos;
    }

    distinctPos = otherPositions;
    if (k != (int)distinctPos.size()) bruteForceFallback();

    for (int wheel = 2; wheel <= n - 1; ++wheel) {
        processWheel(wheel);
    }

    if (wheel1Pos != targetPos) {
        int oldMarker = wheel1Pos;
        k = ask(1, rotationDelta(targetPos, wheel1Pos));
        wheel1Pos = targetPos;
        distinctPos.erase(oldMarker);
    }

    if ((int)distinctPos.size() != 2 || !distinctPos.count(targetPos)) {
        bruteForceFallback();
    }

    int lastPosition = -1;
    for (int p : distinctPos) {
        if (p != targetPos) lastPosition = p;
    }
    if (lastPosition < 0) bruteForceFallback();

    k = ask(n, rotationDelta(targetPos, lastPosition));
    bruteForceFallback();
}
