#include <bits/stdc++.h>
using namespace std;

using ll = long long;

const ll BOX = 100000;
const ll MAXR = 50000;

double query(ll x1, ll y1, ll x2, ll y2) {
    cout << "query " << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << '\n' << flush;
    double v;
    if (!(cin >> v)) exit(0);
    if (v < -0.5) exit(0);
    return v;
}

ll isqrt(ll x) {
    if (x < 0) return -1;
    ll r = (ll)sqrt((double)x);
    while ((r + 1) * (r + 1) <= x) ++r;
    while (r * r > x) --r;
    return r;
}

struct Probe {
    ll y;
    double L;
    bool pos;
    ll D;
};

Probe make_probe(ll y, double L) {
    Probe p;
    p.y = y;
    p.L = L;
    p.pos = (L > 0.5);
    if (p.pos) {
        long double h = (long double)L / 2.0L;
        p.D = (ll)llroundl(h * h);
    } else {
        p.D = 0;
    }
    return p;
}

struct Cand {
    ll cy, r, err;
};

vector<Cand> brute(const vector<Probe>& probes, int tol) {
    if (probes.empty() || !probes[0].pos) return {};

    ll y1 = probes[0].y;
    ll D1 = probes[0].D;

    vector<Cand> res;
    ll start = max(100LL, isqrt(D1));

    for (ll r = start; r <= MAXR; ++r) {
        ll rr = r * r;
        ll t2 = rr - D1;
        if (t2 < 0) continue;

        ll t = isqrt(t2);
        if (t * t != t2) continue;

        auto check_cy = [&](ll cy) {
            if (cy < r || cy > BOX - r) return;

            bool ok = true;
            ll err = 0;

            for (const Probe& p : probes) {
                ll dist = cy - p.y;
                ll D = rr - dist * dist;

                if (p.pos) {
                    if (D <= 0) {
                        ok = false;
                        break;
                    }
                    if (llabs(D - p.D) > tol) {
                        ok = false;
                        break;
                    }
                    err += llabs(D - p.D);
                } else {
                    if (D > tol) {
                        ok = false;
                        break;
                    }
                    if (D > 0) err += D;
                }
            }

            if (ok) res.push_back({cy, r, err});
        };

        if (t == 0) {
            check_cy(y1);
        } else {
            check_cy(y1 + t);
            check_cy(y1 - t);
        }
    }

    sort(res.begin(), res.end(), [](const Cand& a, const Cand& b) {
        if (a.err != b.err) return a.err < b.err;
        if (a.r != b.r) return a.r < b.r;
        return a.cy < b.cy;
    });

    vector<Cand> uniq;
    for (const Cand& c : res) {
        if (uniq.empty() || uniq.back().cy != c.cy || uniq.back().r != c.r) {
            uniq.push_back(c);
        }
    }
    return uniq;
}

bool unique_best(const vector<Cand>& cands) {
    if (cands.empty()) return false;
    ll best = cands[0].err;
    for (size_t i = 1; i < cands.size(); ++i) {
        if (cands[i].err == best) return false;
    }
    return true;
}

pair<ll, ll> fallback_cyr(const vector<Probe>& probes) {
    if (probes.empty() || !probes[0].pos) return {50000, 100};

    ll y1 = probes[0].y;
    ll D1 = probes[0].D;

    Probe posOther, zeroOther;
    bool hasPos = false, hasZero = false;

    for (const Probe& p : probes) {
        if (p.y == y1) continue;
        if (p.pos && !hasPos) {
            posOther = p;
            hasPos = true;
        }
        if (!p.pos && llabs(p.y - y1) == 1 && !hasZero) {
            zeroOther = p;
            hasZero = true;
        }
    }

    if (hasPos) {
        ll y2 = posOther.y;
        ll D2 = posOther.D;
        ll dy = y2 - y1;

        ll q = (ll)llround((double)(D1 - D2) / (double)dy);
        ll cy0 = (y1 + y2 - q) / 2;

        for (ll cy = cy0 - 10; cy <= cy0 + 10; ++cy) {
            ll r2 = D1 + (y1 - cy) * (y1 - cy);
            ll r = isqrt(r2);
            for (ll rr = r - 5; rr <= r + 5; ++rr) {
                if (rr < 100 || rr > MAXR) continue;
                if (cy < rr || cy > BOX - rr) continue;

                ll d1 = rr * rr - (y1 - cy) * (y1 - cy);
                ll d2 = rr * rr - (y2 - cy) * (y2 - cy);

                if (llabs(D1 - d1) <= 2 && llabs(D2 - d2) <= 2) {
                    return {cy, rr};
                }
            }
        }
    } else if (hasZero) {
        ll off = zeroOther.y - y1;
        ll r = (ll)llroundl((D1 + 1.0L) / 2.0L);
        ll cy = y1 + off * (1 - r);
        if (r >= 100 && r <= MAXR && cy >= r && cy <= BOX - r) {
            return {cy, r};
        }
    }

    return {y1, 100};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    unordered_map<ll, double> hist;
    hist.reserve(4096);

    ll y1 = -1;
    double L1 = 0.0;
    ll kFound = 0;

    for (ll k = 1; k <= 502; ++k) {
        ll y = 199 * k;
        double L = query(0, y, BOX, y);
        hist[y] = L;
        if (L > 0.5) {
            y1 = y;
            L1 = L;
            kFound = k;
            break;
        }
    }

    if (y1 == -1) {
        for (ll y = 0; y <= BOX; y += 50) {
            if (hist.count(y)) continue;
            double L = query(0, y, BOX, y);
            hist[y] = L;
            if (L > 0.5) {
                y1 = y;
                L1 = L;
                break;
            }
        }
    }

    if (y1 == -1) {
        for (ll y = 0; y <= BOX; y += 10) {
            if (hist.count(y)) continue;
            double L = query(0, y, BOX, y);
            hist[y] = L;
            if (L > 0.5) {
                y1 = y;
                L1 = L;
                break;
            }
        }
    }

    vector<Probe> probes;
    probes.push_back(make_probe(y1, L1));

    if (kFound > 1) {
        ll y0 = 199 * (kFound - 1);
        probes.push_back(make_probe(y0, 0.0));
    }

    vector<Cand> cands = brute(probes, 1);
    if (cands.empty()) cands = brute(probes, 2);

    if (!unique_best(cands)) {
        int firstOff = (y1 < BOX ? 1 : -1);
        vector<int> offs = {firstOff, -firstOff, 2, -2, 5, -5, 10, -10, 50, -50, 100, -100};

        for (int off : offs) {
            ll y = y1 + off;
            if (y < 0 || y > BOX) continue;

            bool dup = false;
            for (const Probe& p : probes) {
                if (p.y == y) {
                    dup = true;
                    break;
                }
            }
            if (dup) continue;

            double L;
            auto it = hist.find(y);
            if (it != hist.end()) {
                L = it->second;
            } else {
                L = query(0, y, BOX, y);
                hist[y] = L;
            }

            probes.push_back(make_probe(y, L));

            cands = brute(probes, 1);
            if (cands.empty()) cands = brute(probes, 2);

            if (unique_best(cands)) break;
        }
    }

    if (cands.empty()) {
        cands = brute(probes, 3);
        if (cands.empty()) {
            auto p = fallback_cyr(probes);
            cands.push_back({p.first, p.second, 0});
        }
    }

    ll cy = cands[0].cy;
    ll r = cands[0].r;

    if (r < 100) r = 100;
    if (r > MAXR) r = MAXR;
    if (cy < r) cy = r;
    if (cy > BOX - r) cy = BOX - r;

    ll low = 0;
    ll high = BOX - 2 * r + 1;
    if (high < 1) high = 1;

    ll cx = -1;

    while (high - low > 1) {
        ll mid = (low + high) / 2;
        double L = query(0, cy, mid, cy);

        if (L > 2.0 * r - 0.5) {
            high = mid;
        } else if (L < 0.5) {
            low = mid;
        } else {
            ll left = (ll)llround((long double)mid - (long double)L);
            cx = left + r;
            break;
        }
    }

    if (cx == -1) cx = high + r - 1;

    if (cx < r) cx = r;
    if (cx > BOX - r) cx = BOX - r;

    cout << "answer " << cx << ' ' << cy << ' ' << r << '\n' << flush;
    return 0;
}
