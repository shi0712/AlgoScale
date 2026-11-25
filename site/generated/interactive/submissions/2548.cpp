#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    cin >> n;
    if (n == 1) {
        cout << "C 1\n" << flush;
        return 0;
    }
    vector<bool> is_prime(n + 1, true);
    is_prime[0] = is_prime[1] = false;
    for (int i = 2; i * i <= n; ++i) {
        if (is_prime[i]) {
            for (int j = i * i; j <= n; j += i)
                is_prime[j] = false;
        }
    }
    vector<int> primes;
    for (int i = 2; i <= n; ++i)
        if (is_prime[i]) primes.push_back(i);

    vector<char> alive(n + 1, 1);
    int p0 = 0;
    for (int p : primes) {
        cout << "B " << p << "\n" << flush;
        int cB;
        cin >> cB;
        if (cB == -1) return 0;
        for (int m = p; m <= n; m += p) alive[m] = 0;
        cout << "A " << p << "\n" << flush;
        int cA;
        cin >> cA;
        if (cA == -1) return 0;
        if (cA == 1) {
            p0 = p;
            break;
        }
    }
    if (p0 == 0) {
        cout << "C 1\n" << flush;
        return 0;
    }

    long long power = p0;
    while (power * p0 <= n) {
        cout << "A " << power * p0 << "\n" << flush;
        int c;
        cin >> c;
        if (c == -1) return 0;
        if (c == 1)
            power *= p0;
        else
            break;
    }
    long long known = power;

    auto it = lower_bound(primes.begin(), primes.end(), p0);
    int idx = it - primes.begin();
    for (int i = idx + 1; i < (int)primes.size(); ++i) {
        int q = primes[i];
        if (q > n / known) break;
        int M = 0;
        for (int m = q; m <= n; m += q)
            if (alive[m]) ++M;
        cout << "B " << q << "\n" << flush;
        int c;
        cin >> c;
        if (c == -1) return 0;
        for (int m = q; m <= n; m += q) alive[m] = 0;
        if (c == M + 1) {
            long long qpow = q;
            while (qpow * q <= n) {
                cout << "A " << qpow * q << "\n" << flush;
                int c2;
                cin >> c2;
                if (c2 == -1) return 0;
                if (c2 == 1)
                    qpow *= q;
                else
                    break;
            }
            known *= qpow;
        }
    }

    cout << "C " << known << "\n" << flush;
    return 0;
}
