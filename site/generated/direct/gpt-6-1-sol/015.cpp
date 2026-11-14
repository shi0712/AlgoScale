#include <bits/stdc++.h>
using namespace std;

using u32 = uint32_t;
using u64 = uint64_t;

class FastInput {
    static constexpr size_t SIZE = 1 << 20;
    char buffer[SIZE];
    size_t pos = 0, len = 0;

    inline char getChar() {
        if (pos == len) {
            len = fread(buffer, 1, SIZE, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buffer[pos++];
    }

public:
    bool readUInt(u32 &value) {
        char c;
        do {
            c = getChar();
            if (!c) return false;
        } while (c <= ' ');

        value = 0;
        do {
            value = value * 10 + static_cast<u32>(c - '0');
            c = getChar();
        } while (c >= '0' && c <= '9');

        return true;
    }
};

struct Fingerprint {
    u64 lo, hi;

    bool operator==(const Fingerprint &other) const {
        return lo == other.lo && hi == other.hi;
    }
};

static inline u64 mix64(u64 x) {
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

class VectorHasher {
    u64 seed1, seed2;

public:
    VectorHasher() {
        u64 seed = chrono::steady_clock::now().time_since_epoch().count();
        seed ^= reinterpret_cast<uintptr_t>(this);
        seed1 = mix64(seed + 0x9e3779b97f4a7c15ULL);
        seed2 = mix64(seed + 0x3c6ef372fe94f82aULL);
    }

    inline Fingerprint operator()(u32 value) const {
        return {
            mix64(static_cast<u64>(value) + seed1),
            mix64(static_cast<u64>(value) + seed2)
        };
    }
};

class FingerprintSorter {
    static constexpr int BITS = 16;
    static constexpr int RADIX = 1 << BITS;
    static constexpr u64 MASK = RADIX - 1;

    vector<Fingerprint> temporary;
    vector<u32> histogram;

    static bool lessFull(const Fingerprint &a, const Fingerprint &b) {
        if (a.lo != b.lo) return a.lo < b.lo;
        return a.hi < b.hi;
    }

public:
    FingerprintSorter() : histogram(4 * RADIX) {}

    u64 countDistinct(vector<Fingerprint> &values) {
        const size_t n = values.size();
        if (n == 0) return 0;
        if (n == 1) return 1;

        if (n < 4096) {
            sort(values.begin(), values.end(), lessFull);

            u64 result = 1;
            for (size_t i = 1; i < n; ++i) {
                if (!(values[i] == values[i - 1])) ++result;
            }
            return result;
        }

        if (temporary.size() < n) temporary.resize(n);
        fill(histogram.begin(), histogram.end(), 0);

        for (const auto &value : values) {
            u64 key = value.lo;
            ++histogram[0 * RADIX + ((key >> 0) & MASK)];
            ++histogram[1 * RADIX + ((key >> 16) & MASK)];
            ++histogram[2 * RADIX + ((key >> 32) & MASK)];
            ++histogram[3 * RADIX + ((key >> 48) & MASK)];
        }

        Fingerprint *source = values.data();
        Fingerprint *destination = temporary.data();

        for (int pass = 0; pass < 4; ++pass) {
            u32 *count = histogram.data() + pass * RADIX;

            bool constantDigit = false;
            for (int digit = 0; digit < RADIX; ++digit) {
                if (count[digit] == n) {
                    constantDigit = true;
                    break;
                }
            }
            if (constantDigit) continue;

            u32 sum = 0;
            for (int digit = 0; digit < RADIX; ++digit) {
                u32 frequency = count[digit];
                count[digit] = sum;
                sum += frequency;
            }

            const int shift = pass * BITS;
            for (size_t i = 0; i < n; ++i) {
                u32 digit = static_cast<u32>((source[i].lo >> shift) & MASK);
                destination[count[digit]++] = source[i];
            }

            swap(source, destination);
        }

        if (source != values.data()) {
            copy(source, source + n, values.data());
        }

        // Equal first halves are contiguous. Deduplicate their second halves.
        u64 result = 0;
        size_t left = 0;

        while (left < n) {
            size_t right = left + 1;
            while (right < n && values[right].lo == values[left].lo) {
                ++right;
            }

            bool sameHigh = true;
            for (size_t i = left + 1; i < right; ++i) {
                if (values[i].hi != values[left].hi) {
                    sameHigh = false;
                    break;
                }
            }

            if (sameHigh) {
                ++result;
            } else {
                sort(values.begin() + left, values.begin() + right,
                     [](const Fingerprint &a, const Fingerprint &b) {
                         return a.hi < b.hi;
                     });

                ++result;
                for (size_t i = left + 1; i < right; ++i) {
                    if (values[i].hi != values[i - 1].hi) ++result;
                }
            }

            left = right;
        }

        return result;
    }
};

int main() {
    FastInput input;

    u32 testCount;
    input.readUInt(testCount);

    VectorHasher hashVector;
    FingerprintSorter sorter;

    string output;
    output.reserve(static_cast<size_t>(testCount) * 12);

    while (testCount--) {
        u32 n;
        input.readUInt(n);

        // Only dimensions 1 through 31 require stored fingerprints.
        array<vector<Fingerprint>, 31> fingerprints;
        for (auto &list : fingerprints) list.reserve(n);

        // Greedy independent elements, ordered from newest to oldest.
        u32 retained[32];
        int retainedCount = 0;

        bool hasZeroSpace = false;
        bool hasFullSpace = false;

        // Temporary canonical basis. Entries outside pivotMask are ignored.
        u32 basis[32];
        Fingerprint basisHash[32];

        for (u32 position = 0; position < n; ++position) {
            u32 value;
            input.readUInt(value);

            if (value == 0) {
                hasZeroSpace = true;
                continue;
            }

            // Inserting the same newest retained value changes no span.
            if (retainedCount != 0 && retained[0] == value) {
                continue;
            }

            u32 pivotMask = 0;
            int dimension = 0;
            Fingerprint currentHash{0, 0};

            int removedIndex = -1;

            // candidateIndex == -1 denotes the newly appended value.
            for (int candidateIndex = -1;
                 candidateIndex < retainedCount;
                 ++candidateIndex) {

                u32 reduced = (candidateIndex == -1)
                    ? value
                    : retained[candidateIndex];

                // In RREF, each row contains no other pivot bits.
                // Therefore these eliminations can be done in any order.
                u32 eliminate = reduced & pivotMask;
                while (eliminate) {
                    int bit = __builtin_ctz(eliminate);
                    eliminate &= eliminate - 1;
                    reduced ^= basis[bit];
                }

                if (reduced == 0) {
                    removedIndex = candidateIndex;
                    break;
                }

                ++dimension;

                if (dimension == 32) {
                    hasFullSpace = true;

                    // If there were already 32 retained vectors, the last
                    // old vector is now the unique discarded one.
                    if (retainedCount == 32) removedIndex = 31;
                    break;
                }

                int pivot = 31 - __builtin_clz(reduced);
                u32 pivotBit = u32(1) << pivot;

                // Only rows with higher pivots can contain pivotBit.
                u32 higherPivots = 0;
                if (pivot != 31) {
                    higherPivots =
                        pivotMask & (~u32(0) << (pivot + 1));
                }

                while (higherPivots) {
                    int bit = __builtin_ctz(higherPivots);
                    higherPivots &= higherPivots - 1;

                    if (basis[bit] & pivotBit) {
                        currentHash.lo ^= basisHash[bit].lo;
                        currentHash.hi ^= basisHash[bit].hi;

                        basis[bit] ^= reduced;
                        basisHash[bit] = hashVector(basis[bit]);

                        currentHash.lo ^= basisHash[bit].lo;
                        currentHash.hi ^= basisHash[bit].hi;
                    }
                }

                basis[pivot] = reduced;
                basisHash[pivot] = hashVector(reduced);
                pivotMask |= pivotBit;

                currentHash.lo ^= basisHash[pivot].lo;
                currentHash.hi ^= basisHash[pivot].hi;

                auto &list = fingerprints[dimension - 1];

                // An inexpensive optimization for consecutive duplicates.
                if (list.empty() || !(list.back() == currentHash)) {
                    list.push_back(currentHash);
                }
            }

            u32 nextRetained[32];
            int nextCount = 0;
            nextRetained[nextCount++] = value;

            for (int i = 0; i < retainedCount; ++i) {
                if (i != removedIndex) {
                    nextRetained[nextCount++] = retained[i];
                }
            }

            retainedCount = nextCount;
            copy(nextRetained, nextRetained + retainedCount, retained);
        }

        u64 answer = static_cast<u64>(hasZeroSpace)
                   + static_cast<u64>(hasFullSpace);

        for (auto &list : fingerprints) {
            answer += sorter.countDistinct(list);
        }

        output += to_string(answer);
        output += '\n';
    }

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}
