#include <bits/stdc++.h>
using namespace std;

using ll = long long;

ll N, X0, Y0;
unordered_map<unsigned long long, bool> cache;

static inline unsigned long long make_key(ll x, ll y) {
    return (static_cast<unsigned long long>(x) << 32) | static_cast<unsigned int>(y);
}

bool ask(ll x, ll y) {
    if (x < 1 || x > N || y < 1 || y > N) return false;

    unsigned long long k = make_key(x, y);
    auto it = cache.find(k);
    if (it != cache.end()) return it->second;

    cout << "? " << x << " " << y << '\n';
    cout.flush();

    string s;
    if (!(cin >> s)) exit(0);
    if (s == "-1") exit(0);

    bool res;
    if (s == "true") res = true;
    else if (s == "false") res = false;
    else exit(0);

    cache[k] = res;
    return res;
}

ll binary_first_false(ll p, ll other, bool vertical, int dir, ll lo, ll hi) {
    while (hi - lo > 1) {
        ll mid = (lo + hi) / 2;
        bool res;
        if (vertical) res = ask(other, p + dir * mid);
        else res = ask(p + dir * mid, other);

        if (res) lo = mid;
        else hi = mid;
    }
    return hi;
}

ll first_false_unknown(ll p, ll other, bool vertical, int dir, ll H) {
    if (H <= 0) return 1;

    ll d = 1, lo = 0;
    while (d <= H) {
        bool res;
        if (vertical) res = ask(other, p + dir * d);
        else res = ask(p + dir * d, other);

        if (!res) return binary_first_false(p, other, vertical, dir, lo, d);

        lo = d;
        d <<= 1;
    }

    if (H == lo) return H + 1;

    bool res;
    if (vertical) res = ask(other, p + dir * H);
    else res = ask(p + dir * H, other);

    if (res) return H + 1;
    return binary_first_false(p, other, vertical, dir, lo, H);
}

ll first_false_known(ll p, ll other, bool vertical, int dir, ll M) {
    ll H = (dir == 1 ? N - p : p - 1);
    if (H > M) H = M;
    if (H <= 0) return 1;

    bool res;
    if (vertical) res = ask(other, p + dir * H);
    else res = ask(p + dir * H, other);

    if (res) return H + 1;
    return binary_first_false(p, other, vertical, dir, 0, H);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N >> X0 >> Y0)) return 0;

    cache.reserve(1000);
    cache[make_key(X0, Y0)] = true;

    ll maxM = N / 5;
    if (maxM % 2 == 0) --maxM;

    ll HxR = min(N - X0, maxM);
    ll HxL = min(X0 - 1, maxM);
    ll HyU = min(N - Y0, maxM);
    ll HyD = min(Y0 - 1, maxM);

    ll M, l, t;

    if (HxR + HxL <= HyU + HyD) {
        ll hR = first_false_unknown(X0, Y0, false, 1, HxR);
        ll hL = first_false_unknown(X0, Y0, false, -1, HxL);

        M = hR + hL - 1;
        l = X0 + hR - M;

        ll Hup = min(M, N - Y0);
        ll Hdown = min(M, Y0 - 1);

        if (Hup <= Hdown) {
            ll h = first_false_known(Y0, X0, true, 1, M);
            t = Y0 + h - M;
        } else {
            ll h = first_false_known(Y0, X0, true, -1, M);
            t = Y0 - h + 1;
        }
    } else {
        ll hU = first_false_unknown(Y0, X0, true, 1, HyU);
        ll hD = first_false_unknown(Y0, X0, true, -1, HyD);

        M = hU + hD - 1;
        t = Y0 + hU - M;

        ll Hr = min(M, N - X0);
        ll Hl = min(M, X0 - 1);

        if (Hr <= Hl) {
            ll h = first_false_known(X0, Y0, false, 1, M);
            l = X0 + h - M;
        } else {
            ll h = first_false_known(X0, Y0, false, -1, M);
            l = X0 - h + 1;
        }
    }

    int moves = 0;
    while (true) {
        if (l - M >= 1 && t - M >= 1 && ask(l - M, t - M)) {
            l -= M;
            t -= M;
            if (++moves == 4) break;
            continue;
        }
        if (l + M <= N && t - M >= 1 && ask(l + M, t - M)) {
            l += M;
            t -= M;
            if (++moves == 4) break;
            continue;
        }
        if (l - M >= 1 && t + M <= N && ask(l - M, t + M)) {
            l -= M;
            t += M;
            if (++moves == 4) break;
            continue;
        }
        break;
    }

    ll off = 2 * M + (M - 1) / 2;
    ll cx = l + off;
    ll cy = t + off;

    cout << "! " << cx << " " << cy << '\n';
    cout.flush();
    return 0;
}
