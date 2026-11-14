#include <bits/stdc++.h>
using namespace std;
using cd = complex<double>;
const double PI = acos(-1.0);

void fft(vector<cd>& a, bool inverse) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        double angle = (inverse ? 2.0 : -2.0) * PI / len;
        cd root(cos(angle), sin(angle));
        for (int i = 0; i < n; i += len) {
            cd w(1, 0);
            for (int j = 0; j < len / 2; ++j) {
                cd u = a[i+j], v = a[i+j+len/2] * w;
                a[i+j] = u + v;
                a[i+j+len/2] = u - v;
                w *= root;
            }
        }
    }
    if (inverse) for (cd& x : a) x /= n;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, k;
    cin >> n >> k;
    vector<int> f(n);
    for (int& x : f) cin >> x;
    long long sum = 0, bound = 0;
    for (int i = 0; i < n; ++i) if (i != k) {
        sum += f[i];
        bound += abs(f[i]);
    }
    if (bound == 0) {
        cout << "0.0\n";
        return 0;
    }
    int m = 1;
    while (m < 2*n-1) m <<= 1;
    vector<cd> a(m), b(m), chirp(n);
    for (int j = 0; j < n; ++j) {
        long long square = (1LL*j*j) % (2LL*n);
        double angle = PI * (double)square / n;
        chirp[j] = cd(cos(angle), sin(angle));
        int value = j == 0 ? 0 : f[(j+k)%n];
        a[j] = double(value) * conj(chirp[j]);
        b[j] = chirp[j];
        if (j) b[m-j] = chirp[j];
    }
    fft(a, false);
    fft(b, false);
    for (int i = 0; i < m; ++i) a[i] *= b[i];
    fft(a, true);
    vector<double> re(n), normv(n);
    for (int t = 0; t < n; ++t) {
        cd z = a[t] * conj(chirp[t]);
        re[t] = z.real();
        normv[t] = norm(z);
    }
    // The zero-frequency coefficient is known exactly.
    re[0] = (double)sum;
    normv[0] = double(sum)*sum;
    auto calc = [&](long long x) {
        double best = 0;
        for (int i = 0; i < n; ++i)
            best = max(best, normv[i] + 2.0 * x * re[i]);
        return best + double(x)*x;
    };
    long long lo = -sum - bound, hi = -sum + bound;
    while (lo < hi) {
        long long mid = lo + (hi-lo)/2;
        if (calc(mid) <= calc(mid+1)) hi = mid;
        else lo = mid+1;
    }
    cout << fixed << setprecision(12) << sqrt(max(0.0, calc(lo))) << '\n';
}
