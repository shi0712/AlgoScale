#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Block {
    int l, r;
    ll sum;
};

ll capGap(ll L, ll R, ll F) {
    ll mx = max(L, R);
    if (F > mx) {
        // The maximum subarray uses the whole gap.
        return F - L - R;
    } else {
        // The gap is small enough that crossing it is never useful
        // for the interval formed by these two positive blocks.
        return -min(L, R);
    }
}

vector<Block> buildBlocks(const vector<ll>& b, int n) {
    vector<Block> blocks;
    for (int i = 1; i <= n; ++i) {
        if (b[i] > 0) {
            int l = i;
            ll s = 0;
            while (i <= n && b[i] > 0) {
                s += b[i];
                ++i;
            }
            blocks.push_back({l, i - 1, s});
            --i;
        }
    }
    return blocks;
}

void assignGap(vector<ll>& b, int l, int r, ll s) {
    if (l > r) return;
    for (int i = l; i <= r; ++i) b[i] = 0;
    b[l] = s;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    auto ask = [&](int l, int r) -> ll {
        cout << "? " << l << " " << r << endl;
        ll x;
        cin >> x;
        if (x == -1) exit(0);
        return x;
    };

    auto answer = [&](const vector<ll>& b, int n) {
        cout << "!";
        for (int i = 1; i <= n; ++i) cout << " " << b[i];
        cout << endl;
    };

    while (T--) {
        int n;
        cin >> n;

        if (n == 3) {
            vector<ll> b(4, 0);

            ll q1 = ask(1, 1);
            ll q3 = ask(3, 3);
            ll F = ask(1, 3);

            if (q1 > 0) b[1] = q1;
            if (q3 > 0) b[3] = q3;

            bool lp = (q1 > 0);
            bool rp = (q3 > 0);
            ll lv = b[1], rv = b[3];

            if (lp && rp) {
                ll cand = F - lv - rv;
                if (cand > 0) b[2] = cand;
            } else if (lp && !rp) {
                if (F > lv) b[2] = F - lv;
            } else if (!lp && rp) {
                if (F > rv) b[2] = F - rv;
            } else {
                if (F > 0) b[2] = F;
            }

            auto blocks = buildBlocks(b, n);

            if (blocks.size() == 2) {
                ll s = capGap(blocks[0].sum, blocks[1].sum, F);
                assignGap(b, blocks[0].r + 1, blocks[1].l - 1, s);
            }

            answer(b, n);
        } else if (n == 5) {
            vector<ll> b(6, 0);

            ll q1 = ask(1, 1);
            ll q3 = ask(3, 3);
            ll q5 = ask(5, 5);

            ll F13 = ask(1, 3);
            ll F35 = ask(3, 5);
            ll F15 = ask(1, 5);

            if (q1 > 0) b[1] = q1;
            if (q3 > 0) b[3] = q3;
            if (q5 > 0) b[5] = q5;

            auto detect = [&](int idx, bool lp, ll lv, bool rp, ll rv, ll F) {
                if (lp && rp) {
                    ll cand = F - lv - rv;
                    if (cand > 0) b[idx] = cand;
                } else if (lp && !rp) {
                    if (F > lv) b[idx] = F - lv;
                } else if (!lp && rp) {
                    if (F > rv) b[idx] = F - rv;
                } else {
                    if (F > 0) b[idx] = F;
                }
            };

            bool p1 = (q1 > 0);
            bool p3 = (q3 > 0);
            bool p5 = (q5 > 0);

            detect(2, p1, b[1], p3, b[3], F13);
            detect(4, p3, b[3], p5, b[5], F35);

            auto blocks = buildBlocks(b, n);

            if (blocks.size() == 2) {
                ll s = capGap(blocks[0].sum, blocks[1].sum, F15);
                assignGap(b, blocks[0].r + 1, blocks[1].l - 1, s);
            } else if (blocks.size() == 3) {
                ll s0 = capGap(blocks[0].sum, blocks[1].sum, F13);
                assignGap(b, blocks[0].r + 1, blocks[1].l - 1, s0);

                ll s1 = capGap(blocks[1].sum, blocks[2].sum, F35);
                assignGap(b, blocks[1].r + 1, blocks[2].l - 1, s1);
            }

            answer(b, n);
        } else {
            vector<ll> val(n + 1, 0);
            vector<ll> b(n + 1, 0);

            for (int i = 1; i <= n; ++i) {
                ll x = ask(i, i);
                if (x > 0) {
                    val[i] = x;
                    b[i] = x;
                }
            }

            auto blocks = buildBlocks(b, n);

            for (int t = 0; t + 1 < (int)blocks.size(); ++t) {
                int l = blocks[t].l;
                int r = blocks[t + 1].r;
                ll F = ask(l, r);
                ll s = capGap(blocks[t].sum, blocks[t + 1].sum, F);

                int gl = blocks[t].r + 1;
                int gr = blocks[t + 1].l - 1;
                assignGap(b, gl, gr, s);
            }

            answer(b, n);
        }
    }

    return 0;
}
