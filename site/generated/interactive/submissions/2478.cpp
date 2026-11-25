#include <bits/stdc++.h>
using namespace std;

int n;
vector<char> alive;

long long ask(char type, int a) {
    cout << type << ' ' << a << '\n' << flush;
    long long response;
    if (!(cin >> response)) exit(0);
    return response;
}

long long countMultiples(int a) {
    long long result = 0;
    for (int i = a; i <= n; i += a) result += alive[i];
    return result;
}

void eraseMultiples(int a) {
    for (int i = a; i <= n; i += a) alive[i] = 0;
}

int primePower(int p, int e) {
    long long result = 1;
    while (e--) result *= p;
    return static_cast<int>(result);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (n == 1) {
        cout << "C 1\n" << flush;
        return 0;
    }

    vector<char> isPrime(n + 1, true);
    isPrime[0] = isPrime[1] = false;
    vector<int> primes;
    for (int i = 2; i <= n; ++i) {
        if (!isPrime[i]) continue;
        primes.push_back(i);
        if (1LL * i * i <= n)
            for (int j = i * i; j <= n; j += i) isPrime[j] = false;
    }

    alive.assign(n + 1, true);
    long long baselineSize = n;
    long long answer = 1;
    int start = 0;
    int blockSize = 1;

    while (start < static_cast<int>(primes.size())) {
        int end = min(static_cast<int>(primes.size()), start + blockSize);
        int processedEnd = end;
        bool protectedNumber = false;

        for (int i = start; i < end; ++i) {
            int p = primes[i];
            long long expected = countMultiples(p);
            long long response = ask('B', p);
            eraseMultiples(p);
            baselineSize -= expected;
            if (response == expected + 1) {
                protectedNumber = true;
                processedEnd = i + 1;
                break;
            }
            if (response != expected) return 0;
        }

        if (!protectedNumber && end < static_cast<int>(primes.size())) {
            long long actualSize = ask('A', 1);
            if (actualSize == baselineSize + 1) protectedNumber = true;
            else if (actualSize != baselineSize) return 0;
        }

        if (protectedNumber || end == static_cast<int>(primes.size())) {
            for (int i = start; i < processedEnd; ++i) {
                int p = primes[i];
                if (p > n / answer) continue;
                long long expected = countMultiples(p);
                long long response = ask('A', p);
                if (response != expected + 1) {
                    if (response != expected) return 0;
                    continue;
                }

                int low = 1, high = 0;
                long long power = p;
                while (power <= n) {
                    ++high;
                    power *= p;
                }
                while (low < high) {
                    int middle = (low + high + 1) / 2;
                    int value = primePower(p, middle);
                    expected = countMultiples(value);
                    response = ask('A', value);
                    if (response == expected + 1) low = middle;
                    else if (response == expected) high = middle - 1;
                    else return 0;
                }
                answer *= primePower(p, low);
            }

            if (answer == 1) {
                cout << "C 1\n" << flush;
                return 0;
            }

            for (int i = processedEnd; i < static_cast<int>(primes.size()) && primes[i] <= n / answer; ++i) {
                int p = primes[i];
                long long expected = countMultiples(p);
                long long response = ask('A', p);
                if (response != expected + 1) {
                    if (response != expected) return 0;
                    continue;
                }

                int low = 1, high = 0;
                long long power = p;
                while (power <= n) {
                    ++high;
                    power *= p;
                }
                while (low < high) {
                    int middle = (low + high + 1) / 2;
                    int value = primePower(p, middle);
                    expected = countMultiples(value);
                    response = ask('A', value);
                    if (response == expected + 1) low = middle;
                    else if (response == expected) high = middle - 1;
                    else return 0;
                }
                answer *= primePower(p, low);
            }

            cout << "C " << answer << '\n' << flush;
            return 0;
        }

        start = end;
        blockSize = min(96, blockSize * 2);
    }

    cout << "C 1\n" << flush;
    return 0;
}
