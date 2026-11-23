#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "C 1\n" << flush;
        return 0;
    }

    vector<int> primes;
    vector<bool> composite(n + 1, false);
    for (int i = 2; i <= n; ++i) {
        if (!composite[i]) {
            primes.push_back(i);
            if (1LL * i * i <= n) {
                for (long long j = 1LL * i * i; j <= n; j += i) {
                    composite[j] = true;
                }
            }
        }
    }

    vector<unsigned char> inD(n + 1, 1);
    inD[0] = 0;
    int dSize = n;

    auto queryA = [&](long long a) -> int {
        cout << "A " << a << '\n' << flush;
        int response;
        if (!(cin >> response) || response == -1) exit(0);
        return response;
    };

    struct Record {
        int prime;
        int base;
        int response;
    };

    auto queryB = [&](int a) -> Record {
        int base = 0;
        for (int m = a; m <= n; m += a) {
            if (inD[m]) ++base;
        }

        cout << "B " << a << '\n' << flush;
        int response;
        if (!(cin >> response) || response == -1) exit(0);

        for (int m = a; m <= n; m += a) {
            if (inD[m]) {
                inD[m] = 0;
                --dSize;
            }
        }
        return {a, base, response};
    };

    auto finish = [&](long long x) {
        cout << "C " << x << '\n' << flush;
        exit(0);
    };

    long long smallPart = 1;
    bool attacked = false;
    size_t index = 0;

    auto addFactor = [&](int p) {
        smallPart *= p;
        long long primePower = p;

        while (smallPart * p <= n) {
            primePower *= p;
            if (queryA(primePower) == 1) {
                smallPart *= p;
            } else {
                break;
            }
        }
    };

    int batchSize = 1;

    while (index < primes.size()) {
        long long p = primes[index];
        if (p * p > n / smallPart) break;

        if (attacked) {
            Record rec = queryB(primes[index]);
            ++index;
            if (rec.response > rec.base) {
                addFactor(rec.prime);
            }
            continue;
        }

        vector<Record> records;
        while (index < primes.size() &&
               static_cast<int>(records.size()) < batchSize) {
            long long q = primes[index];
            if (q * q > n) break;
            records.push_back(queryB(primes[index]));
            ++index;
        }

        int currentSize = queryA(1);
        if (currentSize == dSize) {
            batchSize = min(16, batchSize * 2);
            continue;
        }
        if (currentSize != dSize + 1) return 0;

        attacked = true;
        vector<int> divisors;
        bool foundFirst = false;
        int m = static_cast<int>(records.size());

        for (int i = 0; i < m; ++i) {
            bool divides;
            if (foundFirst) {
                divides = records[i].response > records[i].base;
            } else if (i == m - 1) {
                divides = true;
            } else {
                divides = queryA(records[i].prime) == 1;
            }

            if (divides) {
                foundFirst = true;
                divisors.push_back(records[i].prime);
            }
        }

        for (int q : divisors) addFactor(q);
    }

    if (attacked) {
        long long limit = n / smallPart;
        while (index < primes.size() && primes[index] <= limit) {
            Record rec = queryB(primes[index]);
            ++index;
            if (rec.response > rec.base) {
                finish(smallPart * rec.prime);
            }
        }
        finish(smallPart);
    }

    int remaining = static_cast<int>(primes.size() - index);
    int blockSize = max(1, static_cast<int>(ceil(sqrt(static_cast<double>(remaining)))));

    while (index < primes.size()) {
        vector<Record> records;
        while (index < primes.size() &&
               static_cast<int>(records.size()) < blockSize) {
            records.push_back(queryB(primes[index]));
            ++index;
        }

        int currentSize = queryA(1);
        if (currentSize == dSize) continue;
        if (currentSize != dSize + 1) return 0;

        int m = static_cast<int>(records.size());
        for (int i = 0; i < m; ++i) {
            if (i == m - 1) {
                finish(records[i].prime);
            }
            if (queryA(records[i].prime) == 1) {
                finish(records[i].prime);
            }
        }
    }

    finish(1);
}
