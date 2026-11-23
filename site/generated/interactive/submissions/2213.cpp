#include <bits/stdc++.h>
using namespace std;

using ll = long long;

ll N, X0, Y0;
map<pair<ll, ll>, bool> memo;

bool ask(ll x, ll y) {
    auto key = make_pair(x, y);
    auto it = memo.find(key);
    if (it != memo.end()) return it->second;

    cout << "? " << x << ' ' << y << '\n' << flush;
    string response;
    if (!(cin >> response)) exit(0);
    if (response == "true") {
        memo[key] = true;
        return true;
    }
    if (response == "false") {
        memo[key] = false;
        return false;
    }
    exit(0);
}

bool inspect(ll x, ll y) {
    if (x < 1 || x > N || y < 1 || y > N) return false;
    return ask(x, y);
}

ll findHorizontalOffset(int direction) {
    ll knownRed = 0;
    ll step = 1;
    ll knownFalse = -1;

    while (true) {
        if (!inspect(X0 + direction * step, Y0)) {
            knownFalse = step;
            break;
        }
        knownRed = step;
        step <<= 1;
    }

    while (knownFalse - knownRed > 1) {
        ll mid = (knownRed + knownFalse) / 2;
        if (inspect(X0 + direction * mid, Y0))
            knownRed = mid;
        else
            knownFalse = mid;
    }
    return knownRed;
}

struct Hypothesis {
    ll originX, originY;
    ll centerX, centerY;
};

bool flattened(const Hypothesis &h, ll M, ll x, ll y) {
    ll side = 5 * M;
    if (x < h.originX || x >= h.originX + side ||
        y < h.originY || y >= h.originY + side)
        return false;

    ll column = (x - h.originX) / M;
    ll row = (y - h.originY) / M;
    return ((column + row) & 1LL) == 0;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N >> X0 >> Y0)) return 0;
    memo[{X0, Y0}] = true;

    ll leftOffset = findHorizontalOffset(-1);
    ll rightOffset = findHorizontalOffset(1);

    ll L = X0 - leftOffset;
    ll M = leftOffset + rightOffset + 1;

    ll downCap = min(M, Y0);
    ll upCap = min(M, N - Y0 + 1);
    bool searchDown = (downCap <= upCap);
    ll cap = searchDown ? downCap : upCap;

    ll knownRed = 0;
    ll knownFalse = cap;
    while (knownFalse - knownRed > 1) {
        ll mid = (knownRed + knownFalse) / 2;
        ll y = searchDown ? Y0 - mid : Y0 + mid;
        if (inspect(X0, y))
            knownRed = mid;
        else
            knownFalse = mid;
    }

    ll B;
    if (searchDown)
        B = Y0 - knownRed;
    else
        B = Y0 + knownRed - M + 1;

    ll half = (M - 1) / 2;
    ll side = 5 * M;
    vector<Hypothesis> hypotheses;

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (((i + j) & 1) != 0) continue;

            ll a = L - 1LL * i * M;
            ll b = B - 1LL * j * M;
            if (a < 1 || b < 1 || a + side - 1 > N || b + side - 1 > N)
                continue;

            Hypothesis h{a, b, a + 2 * M + half, b + 2 * M + half};
            bool consistent = true;
            for (const auto &entry : memo) {
                ll x = entry.first.first;
                ll y = entry.first.second;
                if (flattened(h, M, x, y) != entry.second) {
                    consistent = false;
                    break;
                }
            }
            if (consistent) hypotheses.push_back(h);
        }
    }

    set<pair<ll, ll>> optionSet;
    auto addOption = [&](ll x, ll y) {
        if (1 <= x && x <= N && 1 <= y && y <= N)
            optionSet.insert({x, y});
    };

    const ll offsets[3] = {0, half, M - 1};
    for (const Hypothesis &h : hypotheses) {
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j) {
                ll squareX = h.originX + 1LL * i * M;
                ll squareY = h.originY + 1LL * j * M;
                for (ll dx : offsets) {
                    for (ll dy : offsets)
                        addOption(squareX + dx, squareY + dy);
                }
            }
        }

        ll middleX = h.originX + 2 * M + half;
        ll middleY = h.originY + 2 * M + half;
        addOption(h.originX - 1, middleY);
        addOption(h.originX + side, middleY);
        addOption(middleX, h.originY - 1);
        addOption(middleX, h.originY + side);
    }

    vector<pair<ll, ll>> options(optionSet.begin(), optionSet.end());

    while (hypotheses.size() > 1) {
        int bestWorst = INT_MAX;
        int bestDifference = INT_MAX;
        pair<ll, ll> bestQuery{-1, -1};

        for (auto [x, y] : options) {
            int trueCount = 0;
            for (const Hypothesis &h : hypotheses)
                trueCount += flattened(h, M, x, y);

            int falseCount = (int)hypotheses.size() - trueCount;
            if (trueCount == 0 || falseCount == 0) continue;

            int worst = max(trueCount, falseCount);
            int difference = abs(trueCount - falseCount);
            if (worst < bestWorst ||
                (worst == bestWorst && difference < bestDifference)) {
                bestWorst = worst;
                bestDifference = difference;
                bestQuery = {x, y};
            }
        }

        if (bestQuery.first == -1) break;

        bool result = ask(bestQuery.first, bestQuery.second);
        vector<Hypothesis> remaining;
        for (const Hypothesis &h : hypotheses) {
            if (flattened(h, M, bestQuery.first, bestQuery.second) == result)
                remaining.push_back(h);
        }
        hypotheses.swap(remaining);
    }

    if (hypotheses.empty()) return 0;
    cout << "! " << hypotheses[0].centerX << ' '
         << hypotheses[0].centerY << '\n' << flush;
    return 0;
}
