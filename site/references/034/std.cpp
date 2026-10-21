#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;

int modPow(int a, int e) {
    int result = 1;
    for (; e > 0; e >>= 1, a = (long long)a * a % MOD) {
        if (e & 1) result = (long long)result * a % MOD;
    }
    return result;
}

struct Matrix {
    int a[3][3]{};
};

Matrix multiply(const Matrix& x, const Matrix& y) {
    Matrix z;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            // 三项乘积之和在 long long 范围内。
            z.a[i][j] = (
                1LL * x.a[i][0] * y.a[0][j] +
                1LL * x.a[i][1] * y.a[1][j] +
                1LL * x.a[i][2] * y.a[2][j]
            ) % MOD;
        }
    }
    return z;
}

Matrix transition(int low, int high) {
    Matrix m;
    m.a[0][0] = low;
    m.a[0][1] = high;
    m.a[1][2] = low;
    m.a[2][0] = low;
    return m;
}

class SegmentTree {
    int base, samples;
    vector<vector<Matrix>> tree;

    void pull(int p) {
        for (int x = 0; x < samples; ++x) {
            tree[p][x] = multiply(tree[p << 1][x], tree[p << 1 | 1][x]);
        }
    }

public:
    explicit SegmentTree(const vector<int>& length) {
        int n = (int)length.size();
        samples = n + 1; // 固定插值点：0, 1, ..., n。
        base = 1;
        while (base < n) base <<= 1;
        tree.assign(base << 1, vector<Matrix>(samples));

        for (int i = 0; i < base; ++i) {
            Matrix m;
            if (i < n) {
                m = transition(0, length[i]);
            } else {
                for (int j = 0; j < 3; ++j) m.a[j][j] = 1;
            }
            fill(tree[base + i].begin(), tree[base + i].end(), m);
        }
        for (int p = base - 1; p > 0; --p) pull(p);
    }

    // type=1：low(x)=x-l；type=2：low(x)=length。
    // 这里计算当前分段多项式的延拓值，不能进行 clamp。
    void update(int index, int type, int l, int length) {
        int p = base + index;
        for (int x = 0; x < samples; ++x) {
            int low;
            if (type == 1) {
                low = x - l;
                if (low < 0) low += MOD;
            } else {
                low = length;
            }
            int high = length - low;
            if (high < 0) high += MOD;
            tree[p][x] = transition(low, high);
        }
        for (p >>= 1; p > 0; p >>= 1) pull(p);
    }

    // 设当前分段多项式为 P，构造 G(k)=sum_{x=0}^{k-1} P(x)
    // 在 k=0,1,...,n+1 处的值。
    vector<int> prefixSamples() const {
        vector<int> y(samples + 1, 0);
        for (int x = 0; x < samples; ++x) {
            const Matrix& m = tree[1][x];
            int value = (0LL + m.a[0][0] + m.a[0][1] + m.a[0][2]) % MOD;
            y[x + 1] = y[x] + value;
            if (y[x + 1] >= MOD) y[x + 1] -= MOD;
        }
        return y;
    }
};

class Lagrange {
    int degree;
    vector<int> weight;

public:
    explicit Lagrange(int d) : degree(d), weight(d + 1) {
        vector<int> fac(d + 1), invFac(d + 1);
        fac[0] = 1;
        for (int i = 1; i <= d; ++i) fac[i] = (long long)fac[i - 1] * i % MOD;
        invFac[d] = modPow(fac[d], MOD - 2);
        for (int i = d; i >= 1; --i) invFac[i - 1] = (long long)invFac[i] * i % MOD;
        for (int i = 0; i <= d; ++i) {
            weight[i] = (long long)invFac[i] * invFac[d - i] % MOD;
            if ((d - i) & 1) weight[i] = MOD - weight[i];
        }
    }

    int evaluate(const vector<int>& y, int x) const {
        if (x <= degree) return y[x];
        vector<int> pre(degree + 2), suf(degree + 2);
        pre[0] = 1;
        for (int i = 0; i <= degree; ++i) {
            pre[i + 1] = (long long)pre[i] * (x - i) % MOD;
        }
        suf[degree + 1] = 1;
        for (int i = degree; i >= 0; --i) {
            suf[i] = (long long)suf[i + 1] * (x - i) % MOD;
        }
        int result = 0;
        for (int i = 0; i <= degree; ++i) {
            int term = (long long)y[i] * weight[i] % MOD;
            term = (long long)term * pre[i] % MOD;
            term = (long long)term * suf[i + 1] % MOD;
            result += term;
            if (result >= MOD) result -= MOD;
        }
        return result;
    }
};

struct Event {
    int x, index, type;
    bool operator<(const Event& other) const {
        return x < other.x;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        int n;
        cin >> n;
        vector<int> l(n), r(n), length(n);
        for (int& x : l) cin >> x;
        for (int& x : r) cin >> x;

        vector<Event> events;
        events.reserve(2 * n);
        int total = 1, maximum = 0;
        for (int i = 0; i < n; ++i) {
            length[i] = r[i] - l[i] + 1;
            total = (long long)total * length[i] % MOD;
            maximum = max(maximum, r[i]);
            events.push_back({l[i], i, 1});
            events.push_back({r[i] + 1, i, 2});
        }
        sort(events.begin(), events.end());

        SegmentTree seg(length);
        Lagrange lagrange(n + 1);
        int answer = (long long)maximum * total % MOD;
        int left = 1;

        for (int p = 0; p < (int)events.size();) {
            int right = events[p].x;
            if (left < right) {
                vector<int> y = seg.prefixSamples();
                int contribution = lagrange.evaluate(y, right) - lagrange.evaluate(y, left);
                if (contribution < 0) contribution += MOD;
                answer -= contribution;
                if (answer < 0) answer += MOD;
            }

            // 只需统计阈值 1..maximum。
            if (right == maximum + 1) break;

            while (p < (int)events.size() && events[p].x == right) {
                const Event& e = events[p++];
                seg.update(e.index, e.type, l[e.index], length[e.index]);
            }
            left = right;
        }
        cout << answer << '\n';
    }
    return 0;
}
