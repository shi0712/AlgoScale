// Single-point spectral rebalancing. C++17, online Bluestein FFT + upper hull.
// No answer table; fixed bounds: n<=100000, abs(f_i)<=2000.
#include <bits/stdc++.h>
using namespace std;

using ld = long double;
using cd = complex<ld>;
const ld PI = acosl(-1.0L);

// 长度为 2 的幂的 FFT：正变换用负号，逆变换用正号并除以长度。
void fft(vector<cd>& a, bool inverse) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        ld angle = (inverse ? 2.0L : -2.0L) * PI / len;
        cd step(cosl(angle), sinl(angle));
        for (int i = 0; i < n; i += len) {
            cd w(1, 0);
            for (int j = 0; j < len / 2; ++j) {
                cd u = a[i + j];
                cd v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                // Bound recurrence drift: recompute one unit root every 256 steps.
                if ((j & 255) == 255 && j + 1 < len / 2) {
                    ld theta = angle * (j + 1);
                    w = cd(cosl(theta), sinl(theta));
                } else {
                    w *= step;
                }
            }
        }
    }
    if (inverse) {
        for (cd& z : a) z /= (ld)n;
    }
}

// Bluestein 算法：计算任意长度 n 的 DFT。
vector<cd> dft(const vector<ld>& g) {
    int n = (int)g.size();
    if ((n & (n - 1)) == 0) {
        vector<cd> a(n);
        for (int i = 0; i < n; ++i) a[i] = cd(g[i], 0);
        fft(a, false);
        return a;
    }

    int m = 1;
    while (m < 2 * n - 1) m <<= 1;
    vector<cd> a(m), b(m), chirp(n);
    for (int j = 0; j < n; ++j) {
        // 先对 2*pi 取模，避免用过大的角度计算三角函数。
        long long r = 1LL * j * j % (2LL * n);
        ld angle = PI * (ld)r / n;
        chirp[j] = cd(cosl(angle), -sinl(angle));
        a[j] = g[j] * chirp[j];
        b[j] = conj(chirp[j]);
        if (j > 0) b[m - j] = b[j];
    }
    fft(a, false);
    fft(b, false);
    for (int i = 0; i < m; ++i) a[i] *= b[i];
    fft(a, true);

    vector<cd> res(n);
    for (int t = 0; t < n; ++t) res[t] = a[t] * chirp[t];
    return res;
}

struct Line {
    ld m, c; // y = m*x + c
};

ld solve(const vector<int>& f, int k) {
    int n = (int)f.size();
    vector<ld> g(n);
    for (int j = 1; j < n; ++j) g[j] = f[(k + j) % n];
    // g[0] = 0，修改后的值 x 单独考虑。
    vector<cd> A = dft(g);

    // 这两个频率是实数，可直接用整数加减精确计算。
    ld sum = 0, alternating = 0;
    for (int j = 0; j < n; ++j) {
        sum += g[j];
        alternating += (j & 1) ? -g[j] : g[j];
    }
    A[0] = cd(sum, 0);
    if (n % 2 == 0) A[n / 2] = cd(alternating, 0);

    // 实序列满足 A[n-t] = conj(A[t])，只需考虑前半部分频率。
    vector<Line> lines;
    lines.reserve(n / 2 + 1);
    for (int t = 0; t <= n / 2; ++t) {
        ld a = A[t].real(), b = A[t].imag();
        lines.push_back({2 * a, a * a + b * b});
    }
    sort(lines.begin(), lines.end(), [](const Line& a, const Line& b) {
        if (a.m != b.m) return a.m < b.m;
        return a.c > b.c; // 相同斜率时，截距大的排在前面。
    });

    vector<Line> hull;
    vector<ld> left;
    hull.reserve(lines.size());
    left.reserve(lines.size());
    const ld INF = numeric_limits<ld>::infinity();
    for (const Line& ln : lines) {
        if (!hull.empty() && ln.m == hull.back().m) continue;
        ld start = -INF;
        while (!hull.empty()) {
            start = (hull.back().c - ln.c) / (ln.m - hull.back().m);
            if (start > left.back()) break;
            hull.pop_back();
            left.pop_back();
        }
        if (hull.empty()) start = -INF;
        hull.push_back(ln);
        left.push_back(start);
    }

    // 第 i 段上 H(x) = x*x + m_i*x + c_i。
    ld x0 = 0;
    for (int i = 0; i < (int)hull.size(); ++i) {
        ld right = (i + 1 < (int)hull.size()) ? left[i + 1] : INF;
        ld vertex = -hull[i].m / 2;
        if (vertex <= right) {
            x0 = max(vertex, left[i]);
            break;
        }
    }

    auto evaluate = [&](long long x) -> ld {
        ld ans = 0;
        for (int t = 0; t <= n / 2; ++t) {
            ld real = (ld)x + A[t].real();
            ld imag = A[t].imag();
            ans = max(ans, real * real + imag * imag);
        }
        return ans;
    };

    // 理论上只需检查 floor(x0)、ceil(x0)。
    // 多检查左右各一个整数，减小整数边界处舍入误差的影响。
    long long base = (long long)floorl(x0);
    ld ans = INF;
    for (long long x = base - 1; x <= base + 2; ++x) {
        ans = min(ans, evaluate(x));
    }
    return sqrtl(ans);
}

#ifndef SPECTRAL_NO_MAIN
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;
    vector<int> f(n);
    for (int& x : f) cin >> x;
    cout << fixed << setprecision(15) << solve(f, k) << '\n';
    return 0;
}

#endif
