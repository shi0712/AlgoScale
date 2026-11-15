#include <bits/stdc++.h>
using namespace std;

using ull = unsigned long long;
using ll = long long;

static int T_WEIGHT[11];

int combSmall(int n, int k, int mod) {
    if (k < 0 || k > n) return 0;
    int res = 1;
    for (int i = 1; i <= k; ++i) {
        res = res * (n - i + 1) / i;
    }
    return res % mod;
}

ll extGCD(ll a, ll b, ll &x, ll &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    ll x1, y1;
    ll g = extGCD(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

ll modInverse(ll a, ll mod) {
    ll x, y;
    extGCD(a, mod, x, y);
    x %= mod;
    if (x < 0) x += mod;
    return x;
}

int lucasMod(ull n, ull k, int p) {
    if (k > n) return 0;
    int res = 1;
    while (n > 0 || k > 0) {
        int ni = int(n % p);
        int ki = int(k % p);
        if (ki > ni) return 0;
        res = res * combSmall(ni, ki, p) % p;
        n /= p;
        k /= p;
    }
    return res;
}

struct AutomaticPrefix {
    int q;
    int K, K1, K2;
    ll P, Q1, Q2;
    vector<int> cdigit;
    vector<int> lowW, highW;

    void init(int qq, int c) {
        q = qq;
        cdigit.clear();

        int tmp = c;
        do {
            cdigit.push_back(tmp % q);
            tmp /= q;
        } while (tmp > 0);

        K = (int)cdigit.size();

        P = 1;
        for (int i = 0; i < K; ++i) P *= q;

        K1 = K / 2;
        K2 = K - K1;

        Q1 = 1;
        for (int i = 0; i < K1; ++i) Q1 *= q;
        Q2 = P / Q1;

        lowW.assign((int)Q1, 1);
        highW.assign((int)Q2, 1);

        for (int x = 0; x < (int)Q1; ++x) {
            ll t = x;
            int val = 1;
            for (int i = 0; i < K1; ++i) {
                int d = int(t % q);
                t /= q;
                int ci = cdigit[i];
                val = val * combSmall(d, ci, q) % q;
            }
            lowW[x] = val;
        }

        for (int y = 0; y < (int)Q2; ++y) {
            ll t = y;
            int val = 1;
            for (int j = 0; j < K2; ++j) {
                int d = int(t % q);
                t /= q;
                int ci = cdigit[K1 + j];
                val = val * combSmall(d, ci, q) % q;
            }
            highW[y] = val;
        }
    }

    // Sum_{a=0}^{R-1} C(B - Q*a, c) mod q.
    // B is given modulo P, Q is given modulo P.
    int prefixAP(ll Bmod, ll Qmod, ll R) const {
        if (R <= 0) return 0;

        ll R1 = R / Q1;
        ll R0 = R % Q1;

        vector<int> H((int)Q2);
        ll step = Qmod % Q2;

        for (int t = 0; t < (int)Q2; ++t) {
            ll idx = (step * t) % Q2;
            H[t] = highW[(int)idx];
        }

        vector<int> pref((int)Q2 + 1, 0);
        for (int i = 0; i < (int)Q2; ++i) {
            pref[i + 1] = pref[i] + H[i];
            if (pref[i + 1] >= q) pref[i + 1] -= q;
        }

        ll invQ = modInverse(Qmod, P);

        int total = 0;

        for (int x0 = 0; x0 < (int)Q1; ++x0) {
            int w = lowW[x0];
            if (w == 0) continue;

            ll C = Bmod - x0;
            C %= P;
            if (C < 0) C += P;

            ll A = (C * invQ) % P;
            ll a0 = A % Q1;
            ll A0 = A / Q1;

            int cur = 0;

            if (R1 > 0) {
                ll l = A0 - R1 + 1;
                int s;
                if (l >= 0) {
                    s = pref[(int)A0 + 1] - pref[(int)l];
                } else {
                    ll l0 = l + Q2;
                    s = (pref[(int)Q2] - pref[(int)l0]) + pref[(int)A0 + 1];
                }
                s %= q;
                if (s < 0) s += q;
                cur = s;
            }

            if (R0 > 0 && a0 < R0) {
                ll idx = A0 - R1;
                idx %= Q2;
                if (idx < 0) idx += Q2;
                cur += H[(int)idx];
                cur %= q;
            }

            total += w * cur;
            total %= q;
        }

        return total;
    }
};

int computeG(ull p, ull N, ull n, int c, int q,
             const AutomaticPrefix &ap) {
    if (N == 0) return 0;

    const ull DIRECT = 200000;

    if (N <= DIRECT) {
        int G = 0;
        for (ull m = 1; m <= N; ++m) {
            ull top = n - p * m + (ull)c;
            int val = lucasMod(top, (ull)c, q);
            int tr = T_WEIGHT[(int)(m % 11)] % q;
            G += tr * val;
        }
        return G % q;
    }

    ull M = N / 11;
    int s = int(N % 11);

    ll P = ap.P;
    ll n_modP = ll(n % (ull)P);
    ll c_modP = ll(c % (ull)P);
    ll p_modP = ll(p % (ull)P);
    ll Qmod = (p_modP * 11) % P;

    ll base = (n_modP + c_modP) % P;

    int periodSum = ((ll)c == P - 1) ? 1 : 0;

    int G = 0;

    for (int r = 0; r <= 10; ++r) {
        ull cnt;
        ll term;

        if (r == 0) {
            cnt = M;
            if (cnt == 0) continue;
            term = Qmod; // top for a=0 is n+c-11p
        } else {
            if (r <= s) cnt = M + 1;
            else cnt = M;

            if (cnt == 0) continue;
            term = (p_modP * r) % P;
        }

        ll Bmod = base - term;
        Bmod %= P;
        if (Bmod < 0) Bmod += P;

        ull full = cnt / (ull)P;
        ull rem = cnt % (ull)P;

        int val = 0;
        if (periodSum) {
            val = int(full % (ull)q) * periodSum % q;
        }

        val += ap.prefixAP(Bmod, Qmod, (ll)rem);
        val %= q;

        int tr = T_WEIGHT[r] % q;
        G = (G + tr * val) % q;
    }

    return G;
}

int computeSMod(ull n, int c, int q) {
    AutomaticPrefix ap;
    ap.init(q, c);

    int S = 0;
    ull p = 1;

    while (p <= n) {
        ull N = n / p;
        int G = computeG(p, N, n, c, q, ap);
        S = (S + G) % q;

        if (p > n / 11) break;
        p *= 11;
    }

    return S;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // w[d] = log_2(a_d mod 11) for d = 0..10
    int w[11] = {0, 0, 8, 7, 3, 7, 6, 3, 7, 2, 5};

    for (int e = 0; e <= 10; ++e) {
        int prev = (e == 0 ? 10 : e - 1);
        T_WEIGHT[e] = (w[e] - w[prev] + 10) % 10;
    }

    int T;
    cin >> T;

    while (T--) {
        ull n;
        int c;
        cin >> n >> c;

        int s2 = computeSMod(n, c, 2);
        int s5 = computeSMod(n, c, 5);

        int S = 0;
        for (int x = 0; x <= 9; ++x) {
            if (x % 2 == s2 && x % 5 == s5) {
                S = x;
                break;
            }
        }

        static int pow2mod11[10] = {1, 2, 4, 8, 5, 10, 9, 7, 3, 6};
        cout << pow2mod11[S] << '\n';
    }

    return 0;
}
