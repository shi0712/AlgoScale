#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Candidate {
    int i, j;
    ll cx, cy;
};

ll N;
map<pair<ll, ll>, bool> cache;

bool query(ll x, ll y) {
    auto it = cache.find({x, y});
    if (it != cache.end()) return it->second;

    cout << "? " << x << ' ' << y << endl;
    string response;
    if (!(cin >> response)) exit(0);
    bool result = response == "true";
    cache[{x, y}] = result;
    return result;
}

ll findBoundary(ll x, ll y, ll dx, ll dy) {
    ll lo = 0;
    for (ll step = 1;; step *= 2) {
        ll nx = x + dx * step;
        ll ny = y + dy * step;
        bool outside = nx < 1 || nx > N || ny < 1 || ny > N;
        if (outside) {
            if (dx == 1) step = N - x;
            else if (dx == -1) step = x - 1;
            else if (dy == 1) step = N - y;
            else step = y - 1;
            nx = x + dx * step;
            ny = y + dy * step;
        }

        if (step == lo || query(nx, ny)) {
            if (outside) return dx ? nx : ny;
            lo = step;
            continue;
        }

        ll hi = step;
        while (hi - lo > 1) {
            ll mid = lo + (hi - lo) / 2;
            if (query(x + dx * mid, y + dy * mid)) lo = mid;
            else hi = mid;
        }
        return dx ? x + dx * lo : y + dy * lo;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ll X, Y;
    if (!(cin >> N >> X >> Y)) return 0;
    cache[{X, Y}] = true;

    ll left = findBoundary(X, Y, -1, 0);
    ll right = findBoundary(X, Y, 1, 0);
    ll M = right - left + 1;
    ll bottom = findBoundary(X, Y, 0, -1);

    vector<Candidate> candidates;
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if ((i + j) % 2) continue;
            ll a = left - i * M;
            ll b = bottom - j * M;
            if (a < 1 || b < 1 || a + 5 * M - 1 > N || b + 5 * M - 1 > N) continue;
            candidates.push_back({i, j, a + 2 * M + M / 2, b + 2 * M + M / 2});
        }
    }

    while (candidates.size() > 1) {
        ll bestX = -1, bestY = -1;
        int bestWorst = INT_MAX;
        for (int dx = -4; dx <= 4; ++dx) {
            for (int dy = -4; dy <= 4; ++dy) {
                ll qx = left + M / 2 + dx * M;
                ll qy = bottom + M / 2 + dy * M;
                if (qx < 1 || qx > N || qy < 1 || qy > N) continue;

                int red = 0;
                for (const auto &c : candidates) {
                    int i = c.i + dx, j = c.j + dy;
                    if (0 <= i && i < 5 && 0 <= j && j < 5 && (i + j) % 2 == 0) ++red;
                }
                int white = (int)candidates.size() - red;
                int worst = max(red, white);
                if (red && white && worst < bestWorst) {
                    bestWorst = worst;
                    bestX = qx;
                    bestY = qy;
                }
            }
        }

        bool result = query(bestX, bestY);
        vector<Candidate> remaining;
        for (const auto &c : candidates) {
            int i = c.i + (int)((bestX - (left + M / 2)) / M);
            int j = c.j + (int)((bestY - (bottom + M / 2)) / M);
            bool predicted = 0 <= i && i < 5 && 0 <= j && j < 5 && (i + j) % 2 == 0;
            if (predicted == result) remaining.push_back(c);
        }
        candidates = move(remaining);
    }

    cout << "! " << candidates[0].cx << ' ' << candidates[0].cy << endl;
    return 0;
}
