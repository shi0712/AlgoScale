#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;

    // Sieve to get all primes up to n
    vector<bool> is_prime(n + 1, true);
    if (n >= 0) is_prime[0] = false;
    if (n >= 1) is_prime[1] = false;
    for (int i = 2; (long long)i * i <= n; ++i) {
        if (is_prime[i]) {
            for (int j = i * i; j <= n; j += i) is_prime[j] = false;
        }
    }
    vector<int> primes;
    for (int i = 2; i <= n; ++i) if (is_prime[i]) primes.push_back(i);

    int limit = (int) sqrt(n);
    while ((long long)(limit + 1) * (limit + 1) <= n) ++limit;
    while ((long long)limit * limit > n) --limit;

    vector<int> small_primes, large_primes;
    for (int p : primes) {
        if (p <= limit) small_primes.push_back(p);
        else large_primes.push_back(p);
    }

    auto queryB = [&](int a) -> int {
        cout << "B " << a << "\n" << flush;
        int resp;
        if (!(cin >> resp)) exit(0);
        if (resp == -1) exit(0);
        return resp;
    };
    auto queryA = [&](int a) -> int {
        cout << "A " << a << "\n" << flush;
        int resp;
        if (!(cin >> resp)) exit(0);
        if (resp == -1) exit(0);
        return resp;
    };
    auto answer = [&](int a) {
        cout << "C " << a << "\n" << flush;
        exit(0);
    };

    // Remove multiples of all small primes (except x)
    for (int p : small_primes) {
        queryB(p);
    }

    // Current size of the set
    int curSize = queryA(1);
    int L = (int)large_primes.size();
    int extra = curSize - (L + 1); // 0 if x is 1 or a large prime, 1 otherwise

    if (extra == 0) {
        // x is either 1 or a large prime
        for (int p : large_primes) {
            queryB(p);
            int newSize = queryA(1);
            if (newSize == curSize) {
                answer(p); // x is this large prime
            } else {
                curSize = newSize;
            }
        }
        answer(1); // x must be 1
    } else {
        // x is not 1 and not a large prime
        int large_factor = -1;
        for (int p : large_primes) {
            int cnt = queryB(p);
            if (cnt >= 2) large_factor = p; // p divides x (x = p * something)
        }
        long long x_val = 1;
        for (int p : small_primes) {
            long long power = p;
            while (power <= n) {
                int cnt = queryA((int)power);
                if (cnt == 1) {
                    x_val *= p;
                    if (power > n / p) break;
                    power *= p;
                } else {
                    break;
                }
            }
        }
        if (large_factor != -1) x_val *= large_factor;
        answer((int)x_val);
    }
    return 0;
}
