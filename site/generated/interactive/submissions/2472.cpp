#include <bits/stdc++.h>
using namespace std;

int n, k;


int norm(int x) {
    x %= n;
    if (x < 0) x += n;
    return x;
}

void rotateWheel(int i, int delta) {
    delta = norm(delta);
    if (delta > n / 2) delta -= n;
    cout << i + 1 << ' ' << delta << '\n' << flush;
    if (!(cin >> k)) exit(0);
    if (k == 1) exit(0);
}

bool periodic(const set<int>& colors) {
    for (int g = 1; g < n; ++g) {
        bool ok = true;
        for (int c : colors) {
            if (!colors.count(norm(c + g))) {
                ok = false;
                break;
            }
        }
        if (ok) return true;
    }
    return false;
}

struct Candidate {
    int color;
    bool singleton;
};

int prediction(const Candidate& c, int off, const set<int>& colors) {
    int otherCount = colors.size() - (c.singleton ? 1 : 0);
    int displayed = norm(c.color + off);
    bool present;
    if (c.singleton) {
        present = displayed != c.color && colors.count(displayed);
    } else {
        present = colors.count(displayed);
    }
    return otherCount + (present ? 0 : 1);
}

void solve() {
    if (k == 1) return;
    const int initialK = k;
    int reference = 0;
    set<int> colors;
    bool referenceSingleton = false;

    int referenceOffset = 0;
    set<int> otherColors;
    if (initialK == n) {
        for (int c = 0; c < n; ++c) colors.insert(c);
        referenceSingleton = true;
        otherColors = colors;
        otherColors.erase(0);
    } else {
        map<int, int> history;
        history[0] = k;
        int status = -1;
        int otherCount = -1;
        set<int> matched;

        for (int q = 1; q < n; ++q) {
            referenceOffset = q;
            rotateWheel(0, 1);
            history[q] = k;
            if (status == -1) {
                if (k == initialK + 1) {
                    status = 0;
                    otherCount = initialK;
                    matched.insert(0);
                    for (auto [o, response] : history) {
                        if (response == otherCount) matched.insert(o);
                    }
                } else if (k == initialK - 1) {
                    status = 1;
                    otherCount = initialK - 1;
                    for (auto [o, response] : history) {
                        if (response == otherCount) matched.insert(o);
                    }
                }
            } else if (k == otherCount) {
                matched.insert(q);
            }
            if (status != -1 && (int)matched.size() == otherCount) break;
        }

        if (status == -1) {
            otherCount = initialK;
            for (auto [o, response] : history) otherCount = min(otherCount, response);
            status = (initialK == otherCount + 1) ? 1 : 0;
            for (auto [o, response] : history) {
                if (response == otherCount) matched.insert(o);
            }
        }
        referenceSingleton = status;
        otherColors = matched;
    }

    if (referenceSingleton) {
        colors = otherColors;
        if (otherColors.count(referenceOffset)) {
            reference = referenceOffset;
        } else {
            reference = *otherColors.begin();
            rotateWheel(0, reference - referenceOffset);
        }
    } else {
        colors = otherColors;
        if (referenceOffset != 0) rotateWheel(0, -referenceOffset);
    }

    bool referenceMayOccur = true;
    bool referenceRequired = true;
    if (periodic(colors)) {
        int fresh = 0;
        while (colors.count(fresh)) ++fresh;
        rotateWheel(0, fresh - reference);
        colors.insert(fresh);
        reference = fresh;
        referenceMayOccur = false;
        referenceRequired = false;
    }

    set<int> unidentified;
    for (int i = 1; i < n; ++i) unidentified.insert(i);
    vector<int> aligned;
    vector<pair<int, int>> deferred;

    while (true) {
        if (unidentified.empty()) {
            for (auto [i, color] : deferred) {
                if (color != reference) rotateWheel(i, reference - color);
            }
            return;
        }

        if (periodic(colors)) {
            int p = aligned.back();
            aligned.pop_back();
            int fresh = 0;
            while (colors.count(fresh)) ++fresh;
            rotateWheel(p, fresh - reference);
            colors.insert(fresh);
            deferred.push_back({p, fresh});
        }

        set<int> deferredColors;
        for (auto [i, color] : deferred) deferredColors.insert(color);
        set<int> remainingColors = colors;
        remainingColors.erase(reference);
        for (int color : deferredColors) remainingColors.erase(color);

        int wheel = *unidentified.begin();
        int m = unidentified.size();
        int required = remainingColors.size();
        int surplus = m - required;
        vector<Candidate> candidates;
        for (int color : remainingColors) {
            candidates.push_back({color, true});
            if (surplus >= (referenceRequired ? 2 : 1)) {
                candidates.push_back({color, false});
            }
        }
        if (referenceMayOccur && surplus >= 1) candidates.push_back({reference, false});

        int off = 0;
        vector<bool> queried(n, false);
        queried[0] = true;
        auto colorCount = [&]() {
            set<int> unique;
            for (auto c : candidates) unique.insert(c.color);
            return unique.size();
        };

        while (colorCount() > 1) {
            int bestOffset = -1;
            tuple<long long, int, int, long long, int> bestScore;
            for (int o = 0; o < n; ++o) {
                if (queried[o]) continue;
                int bucketSize[52] = {};
                unsigned long long bucketColors[52] = {};
                for (auto c : candidates) {
                    int response = prediction(c, o, colors);
                    ++bucketSize[response];
                    bucketColors[response] |= 1ULL << c.color;
                }
                int maxColors = 0;
                int maxSize = 0;
                long long weighted = 0;
                long long finishingMoves = 0;
                for (int response = 0; response <= n + 1; ++response) {
                    if (!bucketSize[response]) continue;
                    int count = __builtin_popcountll(bucketColors[response]);
                    maxColors = max(maxColors, count);
                    maxSize = max(maxSize, bucketSize[response]);
                    weighted += 1LL * bucketSize[response] * count;
                    if (count == 1) {
                        int color = __builtin_ctzll(bucketColors[response]);
                        if (norm(color + o) != reference) finishingMoves += bucketSize[response];
                    }
                }
                auto score = make_tuple(weighted, maxColors, maxSize, finishingMoves, o);
                if (bestOffset == -1 || score < bestScore) {
                    bestScore = score;
                    bestOffset = o;
                }
            }

            rotateWheel(wheel, bestOffset - off);
            off = bestOffset;
            queried[off] = true;
            vector<Candidate> filtered;
            for (auto c : candidates) {
                if (prediction(c, off, colors) == k) filtered.push_back(c);
            }
            candidates.swap(filtered);
        }

        int originalColor = candidates[0].color;
        int displayed = norm(originalColor + off);
        int oldSize = colors.size();
        if (displayed != reference) rotateWheel(wheel, reference - displayed);
        if (k == oldSize - 1) colors.erase(originalColor);
        if (originalColor == reference) referenceRequired = false;
        unidentified.erase(wheel);
        aligned.push_back(wheel);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    if (!(cin >> k)) return 0;
    solve();
    return 0;
}
