#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int64 n, m;
map<int64, int64> responses;
set<int64> usedQueries;

int64 receiver(int64 x) {
    auto it = responses.find(x);
    if (it != responses.end()) return it->second;

    cout << "? " << x + 1 << endl;

    int64 dist;
    if (!(cin >> dist) || dist < 0) exit(0);

    responses[x] = dist;
    return dist;
}

bool sameParity(int64 a, int64 b) {
    return (a - b) % 2 == 0;
}

// Converts the value h(t) = dist(t,0) - dist(t,m)
// back to the corresponding vertex t.
int64 positionFromH(int64 h) {
    if (sameParity(h, m)) return (m + h) / 2;
    return (3 * m + 1 - h) / 2;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    m = (n - 1) / 2;

    int64 d0 = receiver(0);
    int64 dm = receiver(m);

    int64 H = d0 - dm;
    int64 E = d0 + dm - 2 * m;

    if (E == 0 || E == 2) {
        int64 C = H / 2;

        bool cHasParityOfM = sameParity(C, m);
        bool sameType =
            (E == 0 ? cHasParityOfM : !cHasParityOfM);

        int64 q = sameType ? C : -C;
        int64 value = receiver(positionFromH(q));

        int64 u;
        if (sameType) {
            u = C - value;
        } else {
            u = C - (n - value);
        }

        int64 v = H - u;
        cout << "! " << positionFromH(u) + 1 << ' '
             << positionFromH(v) + 1 << endl;
        return 0;
    }

    int64 low = max(-m, H - m);
    int64 high = min(m, H + m);

    if (!sameParity(low, m)) ++low;
    if (!sameParity(high, m)) --high;

    auto chooseQuery = [&](int64 target) -> int64 {
        target = max(-m, min(m, target));

        int64 numerator = target + m;
        int64 k0 = numerator / 2;

        for (int64 distance = 0; distance <= m; ++distance) {
            int64 candidates[2] = {
                k0 - distance,
                k0 + 1 + distance
            };

            int64 best = -1;
            int64 bestDistance = LLONG_MAX;

            for (int64 k : candidates) {
                if (k < 0 || k > m) continue;

                int64 q = -m + 2 * k;
                if (usedQueries.count(q)) continue;

                int64 currentDistance = llabs(2 * k - numerator);
                if (currentDistance < bestDistance) {
                    bestDistance = currentDistance;
                    best = q;
                }
            }

            if (best != -1) return best;
        }

        return -m;
    };

    int64 u = -1;
    bool forceExtreme = false;

    while (low <= high) {
        if (low == high) {
            u = low;
            break;
        }

        int64 center = (low + high) / 2;
        int64 target;

        if (forceExtreme) {
            target = (H > 0 ? low : high);
        } else if (H > 0) {
            target = center - (H + 1) / 2;
        } else {
            target = center - (H - 1) / 2;
        }

        int64 q = chooseQuery(target);
        usedQueries.insert(q);

        int64 value = receiver(positionFromH(q));
        int64 g = 2 * value - n;

        int64 oldLow = low;
        int64 oldHigh = high;

        if (g == -H) {
            if (H > 0) {
                high = min(high, q);
            } else {
                high = min(high, H + q - 1);
            }
        } else if (g == H) {
            if (H > 0) {
                low = max(low, H + q + 1);
            } else {
                low = max(low, q);
            }
        } else {
            if (H > 0) {
                u = (g + H + 2 * q) / 2;
            } else {
                u = (H + 2 * q - g) / 2;
            }
            break;
        }

        if (low > high) exit(0);

        forceExtreme = (low == oldLow && high == oldHigh);
    }

    if (u == -1) exit(0);

    int64 v = H - u;
    cout << "! " << positionFromH(u) + 1 << ' '
         << positionFromH(v) + 1 << endl;

    return 0;
}
