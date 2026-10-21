#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int INV2 = (MOD + 1) / 2;

struct Group {
    int score;
    int cnt;
    int win;
};

static void radix_sort_signed(vector<Group>& a) {
    if (a.size() <= 1) return;
    vector<Group> tmp(a.size());
    for (int shift = 0; shift < 32; shift += 8) {
        int pos[256] = {};
        for (const auto& x : a) {
            uint32_t key = uint32_t(x.score) ^ 0x80000000u;
            ++pos[(key >> shift) & 255u];
        }
        int s = 0;
        for (int i = 0; i < 256; ++i) {
            int c = pos[i];
            pos[i] = s;
            s += c;
        }
        for (const auto& x : a) {
            uint32_t key = uint32_t(x.score) ^ 0x80000000u;
            tmp[pos[(key >> shift) & 255u]++] = x;
        }
        a.swap(tmp);
    }
}

static int mod_pow(long long a, int e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

struct Comb {
    vector<int> fac, ifac;

    explicit Comb(int lim = 0) { init(lim); }

    void init(int lim) {
        fac.resize(lim + 1);
        ifac.resize(lim + 1);
        fac[0] = 1;
        for (int i = 1; i <= lim; ++i)
            fac[i] = (long long)fac[i - 1] * i % MOD;
        ifac[lim] = mod_pow(fac[lim], MOD - 2);
        for (int i = lim; i >= 1; --i)
            ifac[i - 1] = (long long)ifac[i] * i % MOD;
    }

    int C(int n, int r) const {
        if (r < 0 || r > n) return 0;
        return (long long)fac[n] * ifac[r] % MOD * ifac[n - r] % MOD;
    }

    int inv_int(int x) const {
        // x < MOD and x >= 1. fac is precomputed at least through x.
        return (long long)fac[x - 1] * ifac[x] % MOD;
    }
};

static int count_group(int b, int w, int open, const Comb& comb) {
    int l = b - w;
    int low = max(0, l - w);
    int high = min(open, l);
    if (low > high) return 0;

    int t = l - low;
    int q = w - t;

    long long term = comb.C(open, low);
    term = term * comb.ifac[t] % MOD;
    term = term * comb.ifac[q] % MOD;
    term = term * mod_pow(INV2, t) % MOD;

    long long sum = 0;
    for (int r = low; r <= high; ++r) {
        sum += term;
        if (sum >= MOD) sum -= MOD;
        if (r == high) break;

        // T_{r+1}/T_r = (open-r)/(r+1) * (l-r)*2/(w-l+r+1).
        term = term * (open - r) % MOD;
        term = term * comb.inv_int(r + 1) % MOD;
        term = term * (l - r) % MOD;
        term = term * 2 % MOD;
        term = term * comb.inv_int(w - l + r + 1) % MOD;
    }

    return (long long)comb.fac[b] * sum % MOD;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int k, n, m;
    if (!(cin >> k >> n >> m)) return 0;

    vector<Group> a(n), c(m);
    for (auto& x : a) {
        cin >> x.score >> x.cnt;
        x.win = 0;
    }
    for (auto& x : c) {
        cin >> x.score >> x.cnt;
        x.win = 0;
    }

    radix_sort_signed(a);
    radix_sort_signed(c);

    // Determine, uniquely if possible, the number of winners in every initial score group.
    // For every final score s: D_s = W_{s-1} + L_{s+1}.
    const long long INF = (1LL << 60);
    int i = 0, j = 0, h = 0;
    while (i < n || j < n || h < m) {
        long long s = INF;
        if (i < n) s = min(s, (long long)a[i].score - 1); // determines L at a[i].score
        if (j < n) s = min(s, (long long)a[j].score + 1); // contributes W at a[j].score
        if (h < m) s = min(s, (long long)c[h].score);

        int need = 0;
        if (h < m && (long long)c[h].score == s) {
            need = c[h].cnt;
            ++h;
        }
        if (j < n && (long long)a[j].score + 1 == s) {
            need -= a[j].win;
            ++j;
        }
        if (i < n && (long long)a[i].score - 1 == s) {
            if (need < 0 || need > a[i].cnt) {
                cout << 0 << '\n';
                return 0;
            }
            a[i].win = a[i].cnt - need;
            ++i;
        } else if (need != 0) {
            cout << 0 << '\n';
            return 0;
        }
    }

    // 2k <= 1e7 < MOD, so all factorial denominators used below are invertible mod MOD.
    Comb comb(2 * k);

    int open = 0;
    long long ans = 1;
    for (const auto& x : a) {
        int w = x.win;
        int l = x.cnt - w;
        if (open + w < l) {
            cout << 0 << '\n';
            return 0;
        }
        ans = ans * count_group(x.cnt, w, open, comb) % MOD;
        open += w - l;
    }

    if (open != 0) ans = 0;
    cout << ans << '\n';
    return 0;
}
