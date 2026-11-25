#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

struct Family {
    int64 l, r;
    int64 a, c, b, d;
};

int n;
int64 m;

int64 norm(i128 x) {
    x %= n;
    if (x < 0) x += n;
    return (int64)x;
}

int dist(int64 x, int64 y) {
    int64 z = llabs(x - y);
    return (int)min(z, n - z);
}

int valueAt(int64 x, const Family &f, int64 i) {
    int64 a = norm((i128)f.a * i + f.c);
    int64 b = norm((i128)f.b * i + f.d);
    return dist(x, a) + dist(x, b);
}

int64 floorDiv(int64 a, int64 b) {
    int64 q = a / b, r = a % b;
    if (r < 0) --q;
    return q;
}

vector<int64> splitPoints(int64 x, const Family &f) {
    vector<int64> p = {f.l, f.r + 1};
    auto addRoots = [&](int64 coef, int64 constant) {
        for (int64 target : {x - m, x, x + m}) {
            int64 k0 = floorDiv(coef * f.l + constant - target, n);
            int64 k1 = floorDiv(coef * f.r + constant - target, n);
            int64 lo = min(k0, k1) - 2, hi = max(k0, k1) + 2;
            for (int64 k = lo; k <= hi; ++k) {
                int64 i = coef * (target + k * n - constant);
                if (f.l <= i && i <= f.r + 1) p.push_back(i);
            }
        }
    };
    addRoots(f.a, f.c);
    addRoots(f.b, f.d);
    sort(p.begin(), p.end());
    p.erase(unique(p.begin(), p.end()), p.end());
    return p;
}

void addWeight(map<int, int64> &weights, int response, int64 amount) {
    if (amount > 0) weights[response] += amount;
}

map<int, int64> responseWeights(int64 x, const vector<Family> &families) {
    map<int, int64> weights;
    for (const Family &f : families) {
        vector<int64> p = splitPoints(x, f);
        for (int j = 0; j + 1 < (int)p.size(); ++j) {
            int64 l = p[j], r = p[j + 1] - 1;
            if (l > r) continue;
            int vl = valueAt(x, f, l), vr = valueAt(x, f, r);
            if (vl == vr) {
                addWeight(weights, vl, r - l + 1);
            } else {
                addWeight(weights, vl, 1);
                addWeight(weights, vr, 1);
            }
        }
    }
    return weights;
}

vector<Family> filterFamilies(int64 x, int response, const vector<Family> &families) {
    vector<Family> result;
    for (const Family &f : families) {
        vector<int64> p = splitPoints(x, f);
        for (int j = 0; j + 1 < (int)p.size(); ++j) {
            int64 l = p[j], r = p[j + 1] - 1;
            if (l > r) continue;
            int vl = valueAt(x, f, l), vr = valueAt(x, f, r);
            if (vl == vr) {
                if (vl == response) result.push_back({l, r, f.a, f.c, f.b, f.d});
                continue;
            }
            if (response < min(vl, vr) || response > max(vl, vr)) continue;
            int64 num = response - vl, den = vr - vl, len = r - l;
            i128 product = (i128)num * len;
            if (product % den != 0) continue;
            int64 at = l + (int64)(product / den);
            result.push_back({at, at, f.a, f.c, f.b, f.d});
        }
    }
    return result;
}

int ask(int64 x) {
    cout << "? " << x + 1 << '\n' << flush;
    int response;
    if (!(cin >> response) || response < 0) exit(0);
    return response;
}

pair<int64, int64> position(const Family &f, int64 i) {
    return {norm((i128)f.a * i + f.c), norm((i128)f.b * i + f.d)};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    m = n / 2;
    int first = ask(0);

    int64 lo = max<int64>(0, first - m);
    int64 hi = min<int64>(m, first);
    vector<Family> families;
    for (int64 a : {-1LL, 1LL}) {
        for (int64 b : {-1LL, 1LL}) {
            families.push_back({lo, hi, a, 0, -b, b * first});
        }
    }

    while (true) {
        int64 total = 0;
        for (const Family &f : families) total += f.r - f.l + 1;

        if (total <= 8) {
            set<pair<int64, int64>> candidates;
            for (const Family &f : families) {
                for (int64 i = f.l; i <= f.r; ++i) {
                    auto [a, b] = position(f, i);
                    if (a > b) swap(a, b);
                    candidates.insert({a, b});
                }
            }
            if (candidates.size() == 1) {
                auto [a, b] = *candidates.begin();
                cout << "! " << a + 1 << ' ' << b + 1 << '\n' << flush;
                return 0;
            }
        }

        set<int64> querySet = {0, m, m + 1};
        for (const Family &f : families) {
            for (int64 i : {f.l, (f.l + f.r) / 2, f.r}) {
                auto [a, b] = position(f, i);
                querySet.insert(a);
                querySet.insert(b);
            }
        }

        int64 bestX = -1, bestWorst = LLONG_MAX;
        for (int64 x : querySet) {
            auto weights = responseWeights(x, families);
            int64 worst = 0;
            for (auto [response, count] : weights) worst = max(worst, count);
            if (worst < bestWorst) {
                bestWorst = worst;
                bestX = x;
            }
        }

        if (bestWorst == total) {
            for (int64 x = 0; x < n && bestWorst == total; ++x) {
                auto weights = responseWeights(x, families);
                int64 worst = 0;
                for (auto [response, count] : weights) worst = max(worst, count);
                if (worst < bestWorst) {
                    bestWorst = worst;
                    bestX = x;
                }
            }
        }

        int response = ask(bestX);
        families = filterFamilies(bestX, response, families);
    }
}
