#include <bits/stdc++.h>
using namespace std;

static const int MOD = 1000000007;
static const int NMAX = 1005;
static const int TH = 30; // small slab simulation threshold

int n;
int A[NMAX], B[NMAX];

int Csmall[NMAX][NMAX];
int invNum[NMAX];

struct Root {
    int c;
    int left;
    int right;
};

struct Piece {
    int start;
    int len;
    int type; // 0: constant, 1: small values, 2: polynomial in binomial basis
    int constVal;
    vector<int> vals;
    vector<int> coeff;
};

struct Interval {
    int l, r, length;
    int finalG;
    vector<Root> roots;
    vector<int> coords;
    vector<Piece> pieces;

    Interval() : l(0), r(0), length(0), finalG(0) {}
    Interval(int _l, int _r) : l(_l), r(_r), length(_r - _l + 1), finalG(0) {}
};

vector<Interval> intervals;
int idMap[NMAX][NMAX];

int buildInterval(int l, int r) {
    if (l > r) return 0;
    if (idMap[l][r] != -1) return idMap[l][r];

    int id = (int)intervals.size();
    intervals.emplace_back(l, r);
    idMap[l][r] = id;

    vector<pair<int,int>> rootsTmp;

    // Balanced split: |(c-l) - (r-c)| <= 2
    for (int c = l; c <= r; ++c) {
        int leftSize = c - l;
        int rightSize = r - c;
        if (abs(leftSize - rightSize) <= 2) {
            rootsTmp.push_back({c, 0});
        }
    }

    vector<Root> roots;
    for (auto &p : rootsTmp) {
        int c = p.first;
        int lid = buildInterval(l, c - 1);
        int rid = buildInterval(c + 1, r);
        roots.push_back({c, lid, rid});
    }
    intervals[id].roots = move(roots);

    return id;
}

void addPoly(vector<int> &a, const vector<int> &b) {
    if (a.size() < b.size()) a.resize(b.size(), 0);
    for (int i = 0; i < (int)b.size(); ++i) {
        a[i] += b[i];
        if (a[i] >= MOD) a[i] -= MOD;
    }
}

int evalPolySmall(const vector<int> &coeff, int t) {
    if (t < 0) return 0;
    long long res = 0;
    int deg = (int)coeff.size() - 1;
    int upto = min(deg, t);
    for (int k = 0; k <= upto; ++k) {
        res += 1LL * coeff[k] * Csmall[t][k] % MOD;
        if (res >= (long long)MOD * MOD) res %= MOD;
    }
    return res % MOD;
}

int evalPolyLarge(const vector<int> &coeff, int t) {
    if (t < 0) return 0;
    if (t <= NMAX - 5) return evalPolySmall(coeff, t);

    long long res = 0;
    long long comb = 1;
    int deg = (int)coeff.size() - 1;

    for (int k = 0; k <= deg; ++k) {
        if (k > t) break;
        res += 1LL * coeff[k] * comb % MOD;
        if (res >= (long long)MOD * MOD) res %= MOD;

        if (k < deg) {
            comb = comb * ((t - k) % MOD) % MOD;
            comb = comb * invNum[k + 1] % MOD;
        }
    }
    return res % MOD;
}

vector<int> valuesToBinom(const vector<int> &vals) {
    int sz = (int)vals.size();
    vector<int> diff = vals;
    vector<int> c(sz, 0);

    for (int k = 0; k < sz; ++k) {
        c[k] = diff[0];
        for (int i = 0; i < sz - k - 1; ++i) {
            diff[i] = diff[i + 1] - diff[i];
            if (diff[i] < 0) diff[i] += MOD;
        }
    }
    return c;
}

vector<int> shiftPoly(const vector<int> &a, int shift) {
    if (a.empty()) return vector<int>{0};
    int d = (int)a.size() - 1;
    vector<int> b(d + 1, 0);

    vector<int> combShift(d + 1, 0);
    combShift[0] = 1;
    for (int m = 1; m <= d; ++m) {
        long long factor = shift - m + 1;
        factor %= MOD;
        if (factor < 0) factor += MOD;
        combShift[m] = 1LL * combShift[m - 1] * factor % MOD * invNum[m] % MOD;
    }

    for (int j = 0; j <= d; ++j) {
        long long sum = 0;
        for (int k = j; k <= d; ++k) {
            sum += 1LL * a[k] * combShift[k - j] % MOD;
            if (sum >= (long long)MOD * MOD) sum %= MOD;
        }
        b[j] = sum % MOD;
    }
    return b;
}

vector<int> integralProduct(const vector<int> &a, const vector<int> &b) {
    int dA = (int)a.size() - 1;
    int dB = (int)b.size() - 1;

    bool zeroA = true, zeroB = true;
    for (int x : a) if (x) { zeroA = false; break; }
    for (int x : b) if (x) { zeroB = false; break; }
    if (zeroA || zeroB) return vector<int>{0};

    // If both are constants: P(t)=a0*b0*t
    if (dA == 0 && dB == 0) {
        int mul = 1LL * a[0] * b[0] % MOD;
        return vector<int>{0, mul};
    }

    // If A is constant: P(t)=a0 * sum_{z=0}^{t-1} B(z)
    if (dA == 0) {
        int c = a[0];
        vector<int> res(dB + 2, 0);
        // sum_{z=0}^{t-1} C(z,k) = C(t,k+1)
        for (int k = 0; k <= dB; ++k) {
            res[k + 1] = (res[k + 1] + 1LL * c * b[k]) % MOD;
        }
        return res;
    }

    // If B is constant: P(t)=b0 * sum_{y=1}^{t} A(y)
    if (dB == 0) {
        int c = b[0];
        vector<int> res(dA + 2, 0);
        // k=0: sum 1 = t = C(t,1)
        res[1] = (res[1] + 1LL * c * a[0]) % MOD;
        // k>=1: sum_{y=1}^{t} C(y,k)=C(t,k+1)+C(t,k)
        for (int k = 1; k <= dA; ++k) {
            res[k] = (res[k] + 1LL * c * a[k]) % MOD;
            res[k + 1] = (res[k + 1] + 1LL * c * a[k]) % MOD;
        }
        return res;
    }

    // General case
    int D = dA + dB + 1;
    vector<int> Pval(D + 1, 0);
    int prefix = 0;

    for (int y = 1; y <= D; ++y) {
        int av = evalPolySmall(a, y);
        int bv = evalPolySmall(b, y - 1);
        prefix += 1LL * av * bv % MOD;
        if (prefix >= MOD) prefix -= MOD;
        Pval[y] = prefix;
    }

    // Convert values P(0),...,P(D) to binomial coefficients.
    vector<int> diff = Pval;
    vector<int> c(D + 1, 0);

    for (int k = 0; k <= D; ++k) {
        c[k] = diff[0];
        for (int i = 0; i < D - k; ++i) {
            diff[i] = diff[i + 1] - diff[i];
            if (diff[i] < 0) diff[i] += MOD;
        }
    }

    while ((int)c.size() > 1 && c.back() == 0) c.pop_back();
    return c;
}

int evalInterval(int id, int x);

vector<int> getShiftPoly(int id, int S) {
    if (id == 0) return vector<int>{1};

    Interval &in = intervals[id];

    if (S < in.coords[0]) {
        return vector<int>{0};
    }

    if (S >= in.coords.back()) {
        return vector<int>{in.finalG};
    }

    int pos = lower_bound(in.coords.begin(), in.coords.end(), S) - in.coords.begin();
    int idx, shift;

    if (pos < (int)in.coords.size() && in.coords[pos] == S) {
        if (pos == (int)in.coords.size() - 1) {
            return vector<int>{in.finalG};
        }
        idx = pos;
        shift = 0;
    } else {
        idx = pos - 1;
        shift = S - in.coords[idx];
    }

    Piece &p = in.pieces[idx];
    vector<int> base;

    if (p.type == 0) {
        base = vector<int>{p.constVal};
    } else if (p.type == 1) {
        base = valuesToBinom(p.vals);
    } else {
        base = p.coeff;
    }

    return shiftPoly(base, shift);
}

int evalInterval(int id, int x) {
    if (id == 0) return 1;

    Interval &in = intervals[id];

    long long h = 1LL + x; // first height not allowed: heights <= x are allowed

    if (h < in.coords[0]) return 0;

    int maxB = in.coords.back() - 1;
    if (x >= maxB) return in.finalG;

    int pos = upper_bound(in.coords.begin(), in.coords.end(), (int)h) - in.coords.begin() - 1;
    int s = pos;
    int S = in.coords[s];
    int t = (int)(h - S);

    Piece &p = in.pieces[s];

    if (p.type == 0) return p.constVal;
    if (p.type == 1) {
        if (t < 0) return 0;
        if (t >= (int)p.vals.size()) t = (int)p.vals.size() - 1;
        return p.vals[t];
    }
    return evalPolyLarge(p.coeff, t);
}

void computeInterval(int id) {
    Interval &in = intervals[id];

    vector<int> coords;
    coords.reserve((in.r - in.l + 1) * 2);

    for (int i = in.l; i <= in.r; ++i) {
        coords.push_back(A[i]);
        coords.push_back(B[i] + 1);
    }

    sort(coords.begin(), coords.end());
    coords.erase(unique(coords.begin(), coords.end()), coords.end());

    in.coords = coords;
    in.pieces.clear();
    in.pieces.reserve(max(0, (int)coords.size() - 1));

    int current = 0; // before the first possible height, non-empty interval has 0 ways

    for (int s = 0; s + 1 < (int)coords.size(); ++s) {
        int S = coords[s];
        int E = coords[s + 1];
        int L = E - S;

        vector<Root> active;
        for (const Root &rt : in.roots) {
            int c = rt.c;
            if (A[c] <= S && B[c] + 1 >= E) {
                active.push_back(rt);
            }
        }

        Piece piece;
        piece.start = S;
        piece.len = L;

        if (active.empty()) {
            piece.type = 0;
            piece.constVal = current;
        } else if (L <= TH) {
            piece.type = 1;
            piece.vals.assign(L + 1, 0);
            piece.vals[0] = current;

            int val = current;

            for (int t = 1; t <= L; ++t) {
                int y = S + t - 1;
                long long delta = 0;

                for (const Root &rt : active) {
                    int lv = evalInterval(rt.left, y);
                    int rv = evalInterval(rt.right, y - 1);
                    delta += 1LL * lv * rv % MOD;
                    if (delta >= (long long)MOD * MOD) delta %= MOD;
                }

                val += delta % MOD;
                if (val >= MOD) val -= MOD;
                piece.vals[t] = val;
            }

            current = val;
        } else {
            piece.type = 2;
            vector<int> coeff(1, current);

            for (const Root &rt : active) {
                vector<int> leftPoly = getShiftPoly(rt.left, S);
                vector<int> rightPoly = getShiftPoly(rt.right, S);

                vector<int> part = integralProduct(leftPoly, rightPoly);
                addPoly(coeff, part);
            }

            piece.coeff = coeff;
            current = evalPolyLarge(coeff, L);
        }

        in.pieces.push_back(move(piece));
    }

    in.finalG = current;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    for (int i = 1; i <= n; ++i) {
        cin >> A[i] >> B[i];
    }

    // Precompute binomial coefficients for small arguments and modular inverses.
    Csmall[0][0] = 1;
    for (int i = 1; i < NMAX; ++i) {
        Csmall[i][0] = Csmall[i][i] = 1;
        for (int j = 1; j < i; ++j) {
            Csmall[i][j] = Csmall[i - 1][j - 1] + Csmall[i - 1][j];
            if (Csmall[i][j] >= MOD) Csmall[i][j] -= MOD;
        }
    }

    invNum[1] = 1;
    for (int i = 2; i < NMAX; ++i) {
        invNum[i] = MOD - 1LL * (MOD / i) * invNum[MOD % i] % MOD;
    }

    memset(idMap, -1, sizeof(idMap));

    // Empty interval id = 0
    intervals.emplace_back();

    int rootId = buildInterval(1, n);

    vector<int> order;
    order.reserve(intervals.size());

    for (int i = 1; i < (int)intervals.size(); ++i) {
        order.push_back(i);
    }

    sort(order.begin(), order.end(), [&](int x, int y) {
        return intervals[x].length < intervals[y].length;
    });

    for (int id : order) {
        computeInterval(id);
    }

    cout << intervals[rootId].finalG % MOD << '\n';

    return 0;
}
