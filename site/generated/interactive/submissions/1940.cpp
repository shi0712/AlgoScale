#include <bits/stdc++.h>
using namespace std;

using ll = long long;
constexpr ll LIM = 1000000000LL;

struct Observation {
    int l, r;
    ll value;
};

vector<Observation> obs;

ll ask(int l, int r, ll low = 0, ll high = LLONG_MAX) {
    high = min(high, (r - l + 1) * LIM);
    for (const auto &q : obs) {
        if (q.l <= l && r <= q.r) high = min(high, q.value);
        if (l <= q.l && q.r <= r) low = max(low, q.value);
    }
    if (low == high) return low;
    cout << "? " << l << ' ' << r << endl;
    ll x;
    if (!(cin >> x) || x < 0) exit(0);
    obs.push_back({l, r, x});
    return x;
}

// A valley has its highest prefix level at the beginning and its
// lowest at the end. Its depth can be chosen anywhere above its bound.
struct Valley {
    ll bound;
    int pos, left, right;
    ll middle;
};

struct Hill {
    int l, r;
    ll height;
    int gap; // valley immediately to its right, if any
};

vector<Valley> valleys;
vector<ll> b;

void setDepth(int id, ll depth) {
    const Valley &v = valleys[id];
    if (v.pos != -1) {
        b[v.pos] = -depth;
    } else {
        ll x = valleys[v.left].bound;
        setDepth(v.left, x);
        setDepth(v.right, depth + v.middle - x);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n <= 0) return 0;
    b.assign(n + 1, 0);
    ll whole = ask(1, n);

    // Decode pairs: MSS(x,y) and MSS(x) determine both positive parts.
    // Probe larger groups after empty groups to skip sparse stretches.
    int step = 2;
    for (int p = 1; whole && p <= n; ) {
        int l = p, r = min(n, p + step - 1);
        ll m = ask(l, r);
        if (!m) {
            p = r + 1;
            step = min(2 * step, 2 * n);
            continue;
        }
        while (r - l + 1 > 2 && m != (r - l + 1) * LIM) {
            int len = r - l + 1;
            int mid = l + 2 * ((len + 1) / 4) - 1;
            ll left = ask(l, mid, max(0LL, m - (r - mid) * LIM));
            if (left) {
                r = mid;
                m = left;
            } else {
                l = mid + 1;
                obs.push_back({l, r, m});
            }
        }
        if (m == (r - l + 1) * LIM) {
            fill(b.begin() + l, b.begin() + r + 1, LIM);
        } else if (l == r) {
            b[l] = m;
        } else {
            b[l] = ask(l, l, max(0LL, m - LIM), min(m, LIM));
            b[r] = m - b[l];
        }
        p = r + 1;
        step = 2;
    }

    // Nonpositive ends can be trimmed from any recorded observation.
    vector<Observation> trimmed;
    for (auto q : obs) {
        while (q.l <= q.r && !b[q.l]) ++q.l;
        while (q.l <= q.r && !b[q.r]) --q.r;
        if (q.l <= q.r) trimmed.push_back(q);
    }
    obs.swap(trimmed);

    list<Hill> hills;
    for (int i = 1; i <= n; ) {
        if (!b[i]) {
            ++i;
            continue;
        }
        int l = i;
        ll h = 0;
        while (i <= n && b[i]) h += b[i++];
        if (!hills.empty()) {
            Hill &prev = hills.back();
            prev.gap = (int)valleys.size();
            valleys.push_back({0, prev.r + 1, -1, -1, 0});
        }
        hills.push_back({l, i - 1, h, -1});
    }

    // For adjacent hills of heights x,y and valley depth d, their
    // union has MSS max(x,y,x+y-d): internal valley peaks never
    // exceed the smaller hill. A strict improvement fixes d exactly.
    auto tryMerge = [&](list<Hill>::iterator a, list<Hill>::iterator c) {
        int g = a->gap;
        ll small = min(a->height, c->height);
        ll big = max(a->height, c->height);
        if (valleys[g].bound >= small) return false;

        ll upper = max(big, a->height + c->height - valleys[g].bound);
        ll m = ask(a->l, c->r, hills.size() == 2 ? whole : big,
                   min(upper, whole));
        if (m == big) {
            valleys[g].bound = small;
            return false;
        }
        setDepth(g, a->height + c->height - m);
        a->height = m;
        a->r = c->r;
        a->gap = c->gap;
        hills.erase(c);
        return true;
    };

    while (hills.size() > 1) {
        auto it = min_element(hills.begin(), hills.end(),
            [](const Hill &x, const Hill &y) { return x.height < y.height; });
        auto right = next(it);
        bool hasLeft = it != hills.begin();
        auto left = hasLeft ? prev(it) : hills.end();

        if (hasLeft && tryMerge(left, it)) continue;
        if (right != hills.end() && tryMerge(it, right)) continue;

        // Both incident valleys are at least as deep as this hill is
        // high. Valley-hill-valley is therefore another valley; only
        // its aggregate depth can matter to a crossing subarray.
        if (hasLeft && right != hills.end()) {
            int x = left->gap, y = it->gap;
            left->gap = (int)valleys.size();
            valleys.push_back({valleys[x].bound + valleys[y].bound - it->height,
                               -1, x, y, it->height});
        } else {
            int g = hasLeft ? left->gap : it->gap;
            setDepth(g, valleys[g].bound);
            if (hasLeft) left->gap = -1;
        }
        hills.erase(it);
    }

    cout << '!';
    for (int i = 1; i <= n; ++i) cout << ' ' << b[i];
    cout << endl;
    return 0;
}
