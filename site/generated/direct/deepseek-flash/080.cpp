#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int INV2 = (MOD + 1) / 2;

long long mod_pow(long long a, long long e) {
    long long r = 1;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

int get_parity(long long x) {
    long long r = x % 2;
    if (r < 0) r += 2;
    return (int)r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int k, n, m;
    cin >> k >> n >> m;

    vector<pair<long long, int>> init(n), fin(m);
    for (int i = 0; i < n; ++i) {
        cin >> init[i].first >> init[i].second;
    }
    for (int i = 0; i < m; ++i) {
        cin >> fin[i].first >> fin[i].second;
    }

    sort(init.begin(), init.end());
    sort(fin.begin(), fin.end());

    unordered_map<long long, int> Bmap, Dmap, idxMap;
    Bmap.reserve(n * 2 + 10);
    Dmap.reserve(m * 2 + 10);
    idxMap.reserve(n * 2 + 10);

    for (int i = 0; i < n; ++i) {
        Bmap[init[i].first] = init[i].second;
        idxMap[init[i].first] = i;
    }
    for (int i = 0; i < m; ++i) {
        Dmap[fin[i].first] = fin[i].second;
    }

    vector<long long> allPos;
    allPos.reserve(n + m);

    for (auto &p : init) allPos.push_back(p.first);
    for (auto &p : fin) allPos.push_back(p.first + 1);

    sort(allPos.begin(), allPos.end());
    allPos.erase(unique(allPos.begin(), allPos.end()), allPos.end());

    vector<long long> posByParity[2];
    for (long long x : allPos) {
        posByParity[get_parity(x)].push_back(x);
    }

    vector<int> W(n, -1);
    bool ok = true;

    for (int par = 0; par < 2 && ok; ++par) {
        long long pref = 0;
        long long prev = 0;
        bool hasPrev = false;

        for (long long pos : posByParity[par]) {
            if (hasPrev && pref != 0 && pos - prev > 2) {
                ok = false;
                break;
            }

            int B = 0, D = 0;
            auto itB = Bmap.find(pos);
            if (itB != Bmap.end()) B = itB->second;

            auto itD = Dmap.find(pos - 1);
            if (itD != Dmap.end()) D = itD->second;

            pref += (long long)B - D;

            if (B > 0) {
                if (pref < 0 || pref > B) {
                    ok = false;
                    break;
                }
                int idx = idxMap[pos];
                W[idx] = (int)pref;
            } else {
                if (pref != 0) {
                    ok = false;
                    break;
                }
            }

            prev = pos;
            hasPrev = true;
        }

        if (ok && pref != 0) ok = false;
    }

    if (!ok) {
        cout << 0 << '\n';
        return 0;
    }

    for (int i = 0; i < n; ++i) {
        if (W[i] < 0 || W[i] > init[i].second) {
            cout << 0 << '\n';
            return 0;
        }
    }

    int N = 2 * k;

    vector<int> fac(N + 1), ifac(N + 1);
    fac[0] = 1;
    for (int i = 1; i <= N; ++i) {
        fac[i] = (long long)fac[i - 1] * i % MOD;
    }

    ifac[N] = mod_pow(fac[N], MOD - 2);
    for (int i = N; i >= 1; --i) {
        ifac[i - 1] = (long long)ifac[i] * i % MOD;
    }

    auto count_group = [&](int u, int b, int w) -> int {
        int L = b - w;
        if (w < 0 || w > b) return 0;

        int A = w - L;
        int low = max(0, L - w);
        int high = min(L, u);

        if (low > high) return 0;

        long long common = (long long)fac[b] * fac[u] % MOD;

        int e0 = L - low;
        long long invPow = (e0 == 0 ? 1LL : mod_pow(INV2, e0));

        long long res = 0;

        for (int r = low; r <= high; ++r) {
            long long term = common;
            term = term * ifac[u - r] % MOD;
            term = term * ifac[r] % MOD;
            term = term * ifac[L - r] % MOD;
            term = term * invPow % MOD;
            term = term * ifac[A + r] % MOD;

            res += term;
            if (res >= MOD) res -= MOD;

            invPow = invPow * 2 % MOD;
        }

        return (int)res;
    };

    long long ans = 1;
    long long u = 0;

    for (int i = 0; i < n; ++i) {
        int b = init[i].second;
        int w = W[i];
        int L = b - w;

        if (u < 0) {
            ans = 0;
            break;
        }

        ans = ans * count_group((int)u, b, w) % MOD;

        u += (long long)w - L;
        if (u < 0) {
            ans = 0;
            break;
        }
    }

    if (u != 0) ans = 0;

    cout << ans % MOD << '\n';
    return 0;
}
