#include <bits/stdc++.h>
using namespace std;

using ll = long long;

unordered_map<long long, int> memo;

int ask(int a, int b, int c) {
    array<int, 3> t{a, b, c};
    sort(t.begin(), t.end());
    ll key = ((ll(t[0]) << 10 | t[1]) << 10) | t[2];
    auto it = memo.find(key);
    if (it != memo.end()) return it->second;
    cout << "? " << a << ' ' << b << ' ' << c << endl;
    int r;
    if (!(cin >> r) || r < 0) exit(0);
    memo[key] = r;
    return r;
}

[[noreturn]] void answer(int a, int b) {
    cout << "! " << a << ' ' << b << endl;
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 4) return 0;
    vector<int> todo(n);
    iota(todo.begin(), todo.end(), 1);
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    shuffle(todo.begin(), todo.end(), rng);

    auto pop = [&]() {
        int i = todo.back();
        todo.pop_back();
        return i;
    };

    // Coordinates are determined only up to translation and reflection.
    vector<ll> low(n + 1), high(n + 1);
    vector<int> known;
    int l = 0, h = 0;
    ll L = 0, H = 0;

    while (!l) {
        if (todo.size() <= 2) answer(todo.front(), todo.back());
        int g[4], r[4]{}, dummy = 0;
        if (todo.size() == 3) {
            dummy = 1;
            while (find(todo.begin(), todo.end(), dummy) != todo.end()) ++dummy;
            g[0] = dummy;
            for (int i = 1; i < 4; ++i) g[i] = pop();
        } else {
            for (int &i : g) i = pop();
        }

        for (int k = 0; k < 4; ++k) {
            r[k] = ask(g[(k+1)%4], g[(k+2)%4], g[(k+3)%4]);
            // Three equal elements cannot contain the unique zero.
            if (r[k] == 0) {
                if (g[k] != dummy) todo.push_back(g[k]);
                break;
            }
            if (k < 2) continue;

            // At least two of the four triples realize the full span.
            int D = *max_element(r, r+k+1);
            vector<int> possible, inner;
            for (int i = 0; i < 4; ++i) {
                if (i > k || r[i] < D) possible.push_back(i);
                else inner.push_back(i);
            }
            vector<int> viable;
            for (int i : possible)
                if (g[i] != dummy) viable.push_back(g[i]);
            if (todo.size() + viable.size() <= 2) {
                viable.insert(viable.end(), todo.begin(), todo.end());
                answer(viable.front(), viable.back());
            }
            if (possible.size() <= 1 || (k == 3 && possible.size() < 2)) {
                for (int i : viable) todo.push_back(i);
                break;
            }
            if (possible.size() == 2 && possible.back() <= k) {
                int a = possible[0], b = possible[1];
                l = g[a]; h = g[b];
                H = D;
                low[l] = high[l] = L;
                low[h] = high[h] = H;
                for (int i = 0; i < 4; ++i) {
                    known.push_back(g[i]);
                    if (i != a && i != b) {
                        low[g[i]] = H - r[a];
                        high[g[i]] = L + r[b];
                    }
                }
                break;
            }
        }
    }

    auto put = [&](int i, ll a, ll b) {
        known.push_back(i);
        low[i] = a;
        high[i] = b;
    };

    auto pivot = [&]() {
        int best = 0;
        ll margin = -1;
        for (int i : known) {
            ll m = min(low[i]-L, H-high[i]);
            if (m > margin || (m == margin &&
                high[i]-low[i] < high[best]-low[best])) {
                margin = m;
                best = i;
            }
        }
        return best;
    };

    auto single = [&](int x, int p, ll a, ll b) {
        ll v = ask(l, h, x);
        if (v == H-L) {
            put(x, max(a,L), min(b,H));
            return;
        }
        ll left = H-v, right = L+v;
        bool goesRight;
        if (left < a) goesRight = true;
        else if (right > b) goesRight = false;
        else {
            ll s = ask(l, p, x);
            goesRight = (s == v);
            if (!goesRight) low[p] = high[p] = left+s;
        }
        if (goesRight) {
            h = x;
            H = right;
            put(x,H,H);
        } else {
            l = x;
            L = left;
            put(x,L,L);
        }
    };

    int credit = 3, cooldown = 0;
    constexpr ll INF = (1LL << 50);
    while (!todo.empty()) {
        int p = pivot();
        int x = pop();
        if (todo.empty() || cooldown > 0) {
            cooldown = max(0, cooldown-1);
            single(x,p,-INF,INF);
            continue;
        }
        int y = pop();
        ll s = ask(p,x,y);
        ll a = low[p]-s, b = high[p]+s;
        if (a >= L && b <= H) {
            put(x,a,b);
            put(y,a,b);
            credit = min(5,credit+1);
        } else if (a >= L || b <= H) {
            // Only one boundary can move. Measure the more distant
            // of the two new points from the opposite boundary.
            bool right = (a >= L);
            int anchor = right ? l : h;
            ll v = ask(anchor,x,y);
            if (v <= H-L) {
                if (right) b = min(b,L+v);
                else a = max(a,H-v);
                put(x,max(a,L),min(b,H));
                put(y,max(a,L),min(b,H));
            } else {
                ll vx = ask(l,h,x);
                int far = vx == v ? x : y;
                int other = far == x ? y : x;
                ll oldD = H-L;
                if (right) H = L+v, h = far;
                else L = H-v, l = far;
                put(far, right ? H : L, right ? H : L);
                ll u = max(a,L), w = min(b,H);
                if (other == x) {
                    if (vx > oldD) u = w = right ? L+vx : H-vx;
                    else if (right) w = min(w,L+oldD);
                    else u = max(u,H-oldD);
                }
                put(other,u,w);
            }
        } else {
            single(x,p,a,b);
            single(y,pivot(),a,b);
            if (--credit < 0) {
                cooldown = 12;
                credit = 1;
            }
        }
    }
    answer(l,h);
}
