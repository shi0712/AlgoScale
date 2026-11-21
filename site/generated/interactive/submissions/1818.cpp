#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Interval {
    ll l, r;
};

int n;
ll h;
ll s;                 // response of query at 0
ll L0, R0;            // initial range for smaller radius
int side1v[4] = { 1, 1, -1, -1 };
int side2v[4] = { 1, -1, 1, -1 };

ll normmod(ll x) {
    x %= n;
    if (x < 0) x += n;
    return x;
}

ll distPoint(ll x, ll y) {
    ll d = llabs(x - y);
    return min(d, (ll)n - d);
}

pair<ll, ll> getPos(int id, ll r) {
    ll t = s - r;
    ll p1, p2;

    if (side1v[id] == 1) p1 = normmod(r);
    else p1 = normmod(-r);

    if (side2v[id] == 1) p2 = normmod(t);
    else p2 = normmod(-t);

    return {p1, p2};
}

ll evalCand(int id, ll x, ll r) {
    auto [p1, p2] = getPos(id, r);
    return distPoint(x, p1) + distPoint(x, p2);
}

void normalizeSeg(int id, ll l, ll r, vector<Interval>& out) {
    if (l > r) return;

    if (side1v[id] == side2v[id] && (s % 2 == 0)) {
        ll q = s / 2;
        if (q < l || q > r) {
            out.push_back({l, r});
        } else {
            if (l <= q - 1) out.push_back({l, q - 1});
            if (q + 1 <= r) out.push_back({q + 1, r});
        }
    } else {
        out.push_back({l, r});
    }
}

vector<Interval> mergeVec(vector<Interval> v) {
    if (v.empty()) return {};
    sort(v.begin(), v.end(), [](const Interval& a, const Interval& b) {
        if (a.l != b.l) return a.l < b.l;
        return a.r < b.r;
    });

    vector<Interval> res;
    for (auto cur : v) {
        if (cur.l > cur.r) continue;
        if (res.empty()) {
            res.push_back(cur);
        } else {
            if (cur.l <= res.back().r + 1) {
                res.back().r = max(res.back().r, cur.r);
            } else {
                res.push_back(cur);
            }
        }
    }
    return res;
}

vector<Interval> filterInterval(int id, ll l, ll r, ll x, ll val) {
    vector<Interval> res;
    if (l > r) return res;

    vector<ll> cuts;
    cuts.push_back(l);
    cuts.push_back(r + 1);

    for (int term = 0; term < 2; term++) {
        int side = (term == 0 ? side1v[id] : side2v[id]);
        bool first = (term == 0);

        ll targets[3] = {
            normmod(x),
            normmod(x + h),
            normmod(x + h + 1)
        };

        for (int ti = 0; ti < 3; ti++) {
            ll target = targets[ti];
            ll q0;

            if (side == 1) {
                q0 = target;
            } else {
                q0 = normmod(-target);
            }

            if (q0 < 0 || q0 > h) continue;

            ll r0;
            if (first) r0 = q0;
            else r0 = s - q0;

            for (int delta = -1; delta <= 2; delta++) {
                ll cur = r0 + delta;
                if (cur >= l && cur <= r + 1) {
                    cuts.push_back(cur);
                }
            }
        }
    }

    sort(cuts.begin(), cuts.end());
    cuts.erase(unique(cuts.begin(), cuts.end()), cuts.end());

    for (int i = 0; i + 1 < (int)cuts.size(); i++) {
        ll a = cuts[i];
        ll b = cuts[i + 1] - 1;
        if (a > b) continue;

        ll Ta = evalCand(id, x, a);

        if (a == b) {
            if (Ta == val) res.push_back({a, a});
            continue;
        }

        ll Ta1 = evalCand(id, x, a + 1);
        ll slope = Ta1 - Ta;

        if (slope == 0) {
            if (Ta == val) res.push_back({a, b});
        } else {
            ll diff = val - Ta;
            if (diff % slope == 0) {
                ll rr = a + diff / slope;
                if (rr >= a && rr <= b) {
                    res.push_back({rr, rr});
                }
            }
        }
    }

    return res;
}

void filterAll(vector<Interval> intervals[4], ll x, ll val) {
    for (int id = 0; id < 4; id++) {
        vector<Interval> raw;

        for (auto iv : intervals[id]) {
            auto got = filterInterval(id, iv.l, iv.r, x, val);
            for (auto z : got) raw.push_back(z);
        }

        vector<Interval> normalized;
        for (auto iv : raw) {
            normalizeSeg(id, iv.l, iv.r, normalized);
        }

        intervals[id] = mergeVec(normalized);
    }
}

bool getUnique(vector<Interval> intervals[4], pair<ll, ll>& answer) {
    bool found = false;
    pair<ll, ll> ans;

    for (int id = 0; id < 4; id++) {
        for (auto iv : intervals[id]) {
            if (iv.l != iv.r) return false;

            ll r = iv.l;
            auto [p1, p2] = getPos(id, r);
            if (p1 == p2) continue;

            if (p1 > p2) swap(p1, p2);
            pair<ll, ll> cur = {p1, p2};

            if (!found) {
                found = true;
                ans = cur;
            } else if (cur != ans) {
                return false;
            }
        }
    }

    if (found) answer = ans;
    return found;
}

void addCandidatePositions(int id, ll l, ll r, ll rv, vector<ll>& cand) {
    if (rv < l || rv > r) return;

    ll t = s - rv;
    ll p1, p2;

    if (side1v[id] == 1) p1 = normmod(rv);
    else p1 = normmod(-rv);

    if (side2v[id] == 1) p2 = normmod(t);
    else p2 = normmod(-t);

    cand.push_back(p1);
    cand.push_back(p2);
    cand.push_back(normmod(p1 + 1));
    cand.push_back(normmod(p2 + 1));
    cand.push_back(normmod(p1 - 1));
    cand.push_back(normmod(p2 - 1));
}

ll chooseSplitQuery(int id, ll l, ll r, const set<ll>& asked) {
    vector<ll> cand;

    addCandidatePositions(id, l, r, l, cand);
    addCandidatePositions(id, l, r, r, cand);

    ll mid = (l + r) / 2;
    addCandidatePositions(id, l, r, mid, cand);

    if (l + 1 <= r) addCandidatePositions(id, l, r, l + 1, cand);
    if (r - 1 >= l) addCandidatePositions(id, l, r, r - 1, cand);

    ll q1 = l + (r - l) / 4;
    ll q2 = l + 3LL * (r - l) / 4;
    addCandidatePositions(id, l, r, q1, cand);
    addCandidatePositions(id, l, r, q2, cand);

    cand.push_back(R0);
    cand.push_back(normmod(R0 + 1));

    vector<ll> samples;
    samples.push_back(l);
    samples.push_back(r);
    samples.push_back(mid);
    samples.push_back(q1);
    samples.push_back(q2);

    vector<ll> uniqSamples;
    for (ll z : samples) {
        if (z >= l && z <= r) uniqSamples.push_back(z);
    }
    sort(uniqSamples.begin(), uniqSamples.end());
    uniqSamples.erase(unique(uniqSamples.begin(), uniqSamples.end()), uniqSamples.end());

    ll bestX = -1;
    int bestScore = INT_MAX;

    for (ll x : cand) {
        x = normmod(x);
        if (asked.count(x)) continue;

        map<ll, int> freq;
        int mx = 0;

        for (ll rr : uniqSamples) {
            ll val = evalCand(id, x, rr);
            mx = max(mx, ++freq[val]);
        }

        if (mx < bestScore) {
            bestScore = mx;
            bestX = x;
        }
    }

    if (bestX == -1) {
        ll start = mid;
        for (ll off = 0; off <= (ll)asked.size() + 5; off++) {
            ll x = normmod(start + off);
            if (!asked.count(x)) {
                bestX = x;
                break;
            }
        }
    }

    return bestX;
}

ll choosePointQuery(const vector<pair<ll, ll>>& pairs, const set<ll>& asked) {
    vector<ll> cand;

    for (auto [u, v] : pairs) {
        cand.push_back(u);
        cand.push_back(v);
        cand.push_back(normmod(u + 1));
        cand.push_back(normmod(v + 1));
        cand.push_back(normmod(u - 1));
        cand.push_back(normmod(v - 1));
    }

    cand.push_back(0);
    cand.push_back(1);
    cand.push_back(h);
    cand.push_back(h + 1);

    ll bestX = -1;
    int bestScore = INT_MAX;

    for (ll x : cand) {
        x = normmod(x);
        if (asked.count(x)) continue;

        map<ll, int> freq;
        int mx = 0;

        for (auto [u, v] : pairs) {
            ll val = distPoint(x, u) + distPoint(x, v);
            mx = max(mx, ++freq[val]);
        }

        if (mx < bestScore) {
            bestScore = mx;
            bestX = x;
        }
    }

    if (bestX == -1) {
        for (int x = 0; x < n; x++) {
            if (!asked.count(x)) {
                bestX = x;
                break;
            }
        }
    }

    return bestX;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    h = (n - 1) / 2;

    auto ask = [&](ll x) -> ll {
        cout << "? " << (x + 1) << '\n';
        cout.flush();

        ll res;
        cin >> res;
        if (res < 0) exit(0);
        return res;
    };

    set<ll> asked;

    ll firstAns = ask(0);
    asked.insert(0);
    s = firstAns;

    L0 = max(0LL, s - h);
    R0 = min(h, s / 2);

    vector<Interval> intervals[4];

    for (int id = 0; id < 4; id++) {
        vector<Interval> tmp;
        normalizeSeg(id, L0, R0, tmp);
        intervals[id] = mergeVec(tmp);
    }

    bool pivotDone = false;

    while (true) {
        pair<ll, ll> answer;
        if (getUnique(intervals, answer)) {
            cout << "! " << (answer.first + 1) << ' ' << (answer.second + 1) << '\n';
            cout.flush();
            return 0;
        }

        int bestId = -1;
        ll bestL = 0, bestR = 0;
        ll bestLen = -1;

        for (int id = 0; id < 4; id++) {
            for (auto iv : intervals[id]) {
                ll len = iv.r - iv.l + 1;
                if (len > bestLen) {
                    bestLen = len;
                    bestId = id;
                    bestL = iv.l;
                    bestR = iv.r;
                }
            }
        }

        ll x;

        if (!pivotDone && bestLen > 1) {
            if (s <= h) {
                x = R0 + 1;
            } else {
                x = R0;
            }
            x = normmod(x);

            if (asked.count(x)) {
                x = chooseSplitQuery(bestId, bestL, bestR, asked);
            }

            pivotDone = true;
        } else if (bestLen > 1) {
            x = chooseSplitQuery(bestId, bestL, bestR, asked);
        } else {
            vector<pair<ll, ll>> points;

            for (int id = 0; id < 4; id++) {
                for (auto iv : intervals[id]) {
                    if (iv.l != iv.r) continue;
                    ll r = iv.l;
                    auto [p1, p2] = getPos(id, r);
                    if (p1 == p2) continue;
                    if (p1 > p2) swap(p1, p2);
                    points.push_back({p1, p2});
                }
            }

            sort(points.begin(), points.end());
            points.erase(unique(points.begin(), points.end()), points.end());

            x = choosePointQuery(points, asked);
        }

        ll res = ask(x);
        asked.insert(x);

        filterAll(intervals, x, res);
    }

    return 0;
}
