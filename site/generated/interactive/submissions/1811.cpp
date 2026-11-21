#include <bits/stdc++.h>
using namespace std;

int n;

int query(char op, int a) {
    cout << op << ' ' << a << '\n';
    cout.flush();

    int r;
    if (!(cin >> r)) exit(0);
    if (r < 0) exit(0);
    return r;
}

void answer(long long x) {
    cout << "C " << x << '\n';
    cout.flush();
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    if (n == 1) {
        answer(1);
    }

    vector<bool> isPrime(n + 1, true);
    isPrime[0] = false;
    if (n >= 1) isPrime[1] = false;

    for (int i = 2; 1LL * i * i <= n; ++i) {
        if (isPrime[i]) {
            for (int j = i * i; j <= n; j += i) {
                isPrime[j] = false;
            }
        }
    }

    int R = (int)sqrt(n);
    while (1LL * (R + 1) * (R + 1) <= n) ++R;
    while (1LL * R * R > n) --R;

    vector<int> small, large;

    for (int i = 2; i <= n; ++i) {
        if (!isPrime[i]) continue;
        if (i <= R) small.push_back(i);
        else large.push_back(i);
    }

    // Delete all multiples of small primes, keeping x.
    for (int p : small) {
        query('B', p);
    }

    int M = (int)large.size();
    int cur = query('A', 1);

    if (cur == M + 1) {
        // x has no small prime factor: x is either 1 or a large prime.
        if (M == 0) {
            answer(1);
        }

        int J = 0;
        while (1LL * J * (J + 1) / 2 < M) ++J;

        long long Csum = 1LL * J * (J + 1) / 2;
        int D = (int)(Csum - M);

        vector<int> sizes;
        sizes.push_back(J - D);
        for (int b = 2; b <= J; ++b) {
            sizes.push_back(J - b + 1);
        }

        int pos = 0;

        for (int idx = 0; idx < J; ++idx) {
            int k = sizes[idx];

            for (int i = 0; i < k; ++i) {
                int q = large[pos + i];
                query('B', q);
            }

            int before = 1 + (M - pos);
            int ans = query('A', 1);

            int expectedNegative = before - k;
            int expectedPositive = expectedNegative + 1;

            if (ans == expectedPositive) {
                int xval = -1;

                for (int i = 0; i < k - 1; ++i) {
                    int q = large[pos + i];
                    int c = query('A', q);
                    if (c > 0) {
                        xval = q;
                        break;
                    }
                }

                if (xval == -1) {
                    xval = large[pos + k - 1];
                }

                answer(xval);
            }

            pos += k;
        }

        answer(1);
    } else {
        // x has at least one small prime factor.
        long long s = 1;

        for (int p : small) {
            if (s > 1 && s * 1LL * p > n) {
                answer(s);
            }

            long long bound = n / s;
            long long pk = p;
            int e = 0;

            while (pk <= bound) {
                int c = query('A', (int)pk);

                if (c > 0) {
                    ++e;
                    if (pk > bound / p) break;
                    pk *= p;
                } else {
                    break;
                }
            }

            if (e > 0) {
                for (int i = 0; i < e; ++i) {
                    s *= p;
                }
            }
        }

        long long minLarge = R + 1LL;

        if (s * minLarge > n) {
            answer(s);
        }

        long long limit = n / s;

        for (int q : large) {
            if (q > limit) break;

            int c = query('A', q);

            if (c >= 2) {
                answer(s * 1LL * q);
            }
        }

        answer(s);
    }

    return 0;
}
