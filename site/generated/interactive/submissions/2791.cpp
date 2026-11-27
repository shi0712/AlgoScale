#include <bits/stdc++.h>
using namespace std;

int n;
vector<unsigned char> removed;
int removedCount = 0;

[[noreturn]] void stop() {
    cout.flush();
    exit(0);
}

int askA(int a) {
    cout << "A " << a << '\n' << flush;
    int response;
    if (!(cin >> response) || response == -1) stop();
    return response;
}

int askB(int a) {
    cout << "B " << a << '\n' << flush;
    int response;
    if (!(cin >> response) || response == -1) stop();
    return response;
}

void answer(int x) {
    cout << "C " << x << '\n' << flush;
    exit(0);
}

void markRemoved(int a) {
    for (int y = a; y <= n; y += a) {
        if (!removed[y]) {
            removed[y] = 1;
            ++removedCount;
        }
    }
}

int unremovedMultiples(int a) {
    int result = 0;
    for (int y = a; y <= n; y += a) {
        if (!removed[y]) ++result;
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    vector<bool> isPrime(n + 1, true);
    if (n >= 0) isPrime[0] = false;
    if (n >= 1) isPrime[1] = false;

    for (int i = 2; 1LL * i * i <= n; ++i) {
        if (isPrime[i]) {
            for (int j = i * i; j <= n; j += i) {
                isPrime[j] = false;
            }
        }
    }

    int root = static_cast<int>(sqrt(static_cast<long double>(n)));
    while (1LL * (root + 1) * (root + 1) <= n) ++root;
    while (1LL * root * root > n) --root;

    vector<int> smallPrimes, largePrimes;
    for (int p = 2; p <= n; ++p) {
        if (!isPrime[p]) continue;
        if (p <= root) smallPrimes.push_back(p);
        else largePrimes.push_back(p);
    }

    uint64_t seed = chrono::steady_clock::now().time_since_epoch().count();
    mt19937 rng(static_cast<uint32_t>(seed ^ (seed >> 32)));
    shuffle(smallPrimes.begin(), smallPrimes.end(), rng);
    shuffle(largePrimes.begin(), largePrimes.end(), rng);

    removed.assign(n + 1, 0);

    vector<int> factors;
    constexpr int SMALL_BLOCK = 8;
    const int smallCount = static_cast<int>(smallPrimes.size());

    for (int left = 0; left < smallCount;) {
        int right = min(smallCount, left + SMALL_BLOCK);

        for (int i = left; i < right; ++i) {
            int p = smallPrimes[i];
            askB(p);
            markRemoved(p);
        }

        int expected = n - removedCount;
        int current = askA(1);

        if (current == expected) {
            left = right;
            continue;
        }

        for (int i = left; i < right; ++i) {
            int p = smallPrimes[i];
            if (askA(p) == 1) factors.push_back(p);
        }

        for (int i = right; i < smallCount; ++i) {
            int p = smallPrimes[i];
            int expectedMultiples = unremovedMultiples(p);
            int count = askB(p);
            markRemoved(p);

            if (count > expectedMultiples) {
                factors.push_back(p);
            }
        }

        break;
    }

    long long smallPart = 1;

    for (int p : factors) {
        smallPart *= p;

        long long power = 1LL * p * p;
        while (power <= n) {
            int count = askB(static_cast<int>(power));
            if (count != 1) break;

            smallPart *= p;
            if (power > n / p) break;
            power *= p;
        }
    }

    if (smallPart == 1) {
        constexpr int LARGE_BLOCK = 96;
        const int largeCount = static_cast<int>(largePrimes.size());

        for (int left = 0; left < largeCount; left += LARGE_BLOCK) {
            int right = min(largeCount, left + LARGE_BLOCK);

            for (int i = left; i < right; ++i) {
                int p = largePrimes[i];
                askB(p);
                markRemoved(p);
            }

            int expected = n - removedCount;
            int current = askA(1);

            if (current > expected) {
                for (int i = left; i < right; ++i) {
                    int p = largePrimes[i];
                    if (askA(p) == 1) answer(p);
                }
            }
        }

        answer(1);
    }

    int limit = static_cast<int>(n / smallPart);
    vector<int> candidates;
    for (int p : largePrimes) {
        if (p <= limit) candidates.push_back(p);
    }

    for (int p : candidates) {
        int expectedMultiples = unremovedMultiples(p);
        int count = askB(p);
        markRemoved(p);

        if (count > expectedMultiples) {
            answer(static_cast<int>(smallPart * p));
        }
    }

    answer(static_cast<int>(smallPart));
}
