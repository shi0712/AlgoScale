#include <bits/stdc++.h>
using namespace std;

namespace {

constexpr int max_cafes = 1024;
constexpr int reset_memory = -1;
using Mask = bitset<max_cafes>;

bool taste(int cafe) {
    cout << "? " << cafe + 1 << endl;
    char response;
    if (!(cin >> response) || (response != 'Y' && response != 'N')) exit(0);
    return response == 'Y';
}

void reset_friend() {
    cout << 'R' << endl;
}

struct Knowledge {
    int n, k, alive, remaining;
    vector<Mask> unknown;
    vector<int> degree, gain;
    Mask unresolved, present, exact;
    vector<Mask> slots;
    vector<int> present_count, exact_count;
    int front = 0, used = 0;

    Knowledge(int n, int k)
        : n(n), k(k), alive(n), remaining(n * (n - 1) / 2),
          unknown(n), degree(n, n - 1), gain(n), slots(k),
          present_count(n), exact_count(n) {
        for (int x = 0; x < n; ++x) unresolved.set(x);
        for (int x = 0; x < n; ++x) {
            unknown[x] = unresolved;
            unknown[x].reset(x);
        }
    }

    void rule_out(int x, int y) {
        unknown[x].reset(y);
        unknown[y].reset(x);
        if (--degree[x] == 0) unresolved.reset(x);
        if (--degree[y] == 0) unresolved.reset(y);
        gain[x] -= exact[y];
        gain[y] -= exact[x];
        --remaining;
    }

    void change_slot(const Mask& options, int delta) {
        size_t first = options._Find_first();
        for (size_t x = first; x < size_t(n); x = options._Find_next(x)) {
            present_count[x] += delta;
            present[x] = present_count[x] != 0;
        }
        if (options._Find_next(first) < size_t(n)) return;

        exact_count[first] += delta;
        bool is_exact = exact_count[first] != 0;
        if (is_exact == exact[first]) return;
        exact[first] = is_exact;
        for (size_t y = unknown[first]._Find_first(); y < size_t(n);
             y = unknown[first]._Find_next(y)) {
            gain[y] += delta;
        }
    }

    void push(const Mask& options) {
        if (used == k) {
            change_slot(slots[front], -1);
            slots[front] = options;
            front = (front + 1) % k;
        } else {
            slots[(front + used) % k] = options;
            ++used;
        }
        change_slot(options, 1);
    }

    void clear_memory() {
        present.reset();
        exact.reset();
        fill(present_count.begin(), present_count.end(), 0);
        fill(exact_count.begin(), exact_count.end(), 0);
        fill(gain.begin(), gain.end(), 0);
        front = used = 0;
    }

    void no_match(int x) {
        Mask comparisons = unknown[x] & exact;
        for (size_t y = comparisons._Find_first(); y < size_t(n);
             y = comparisons._Find_next(y)) {
            rule_out(x, y);
        }
        Mask slot;
        slot.set(x);
        push(slot);
    }

    void match(int x) {
        // A slot describes which surviving cafes could represent its variety.
        // A queried cafe is absent from every slot, so removing it is safe.
        Mask options = unknown[x] & present;
        size_t y = options._Find_first();
        if (y >= size_t(n)) exit(0);
        if (options._Find_next(y) >= size_t(n)) {
            Mask inherited = unknown[y] & ~unknown[x];
            inherited.reset(x);
            for (size_t z = inherited._Find_first(); z < size_t(n);
                 z = inherited._Find_next(z)) {
                rule_out(y, z);
            }
        }

        Mask edges = unknown[x];
        for (size_t z = edges._Find_first(); z < size_t(n);
             z = edges._Find_next(z)) {
            rule_out(x, z);
        }
        --alive;
        push(options);
    }

    bool query(int x) {
        if (taste(x)) {
            match(x);
            return true;
        }
        no_match(x);
        return false;
    }
};

// Each block in a cover is a set of consecutive tastings. If two blocks
// appear within the specified radius, every pair across them fits in memory.
struct Cover {
    int blocks, radius;
    initializer_list<int> path;
    bool both_orders_at_radius = false;

    bool fits(int width, int k) const {
        if (both_orders_at_radius) return radius * width <= k;
        return (radius + 1) * width <= k + 1;
    }
};

// BEGIN COVER DATA
// These covers were generated offline and checked by enumerating all block pairs.
constexpr Cover covers[] = {
    {8, 2, {
        0, 1, 2, 3, 4, 0, 5, 6, 1, 4, 7, 2, 5, 6, 3, 7, 0,
    }},
    {16, 2, {
        3, 8, 11, 4, 12, 9, 10, 13, 7, 3, 1, 4, 6, 10, 5, 2, 8, 6, 0, 3, 5, 1, 9, 11,
        2, 13, 7, 15, 11, 5, 14, 7, 0, 4, 15, 9, 3, 14, 10, 8, 1, 13, 0, 6, 9, 7, 8, 12,
        15, 1, 2, 14, 4, 5, 13, 12, 14, 6, 15, 11, 10, 0, 12, 2, 3,
    }},
    {16, 3, {
        15, 12, 4, 11, 5, 2, 3, 8, 6, 15, 7, 5, 0, 11, 13, 6, 8, 12, 9, 3, 0, 2, 10, 4,
        6, 8, 1, 14, 0, 11, 5, 10, 9, 1, 15, 14, 2, 13, 12, 7, 1, 10, 14, 3, 13, 4, 7, 9,
    }},
    {16, 4, {
        14, 2, 0, 6, 13, 3, 4, 10, 8, 15, 9, 14, 2, 0, 7, 11, 12, 10, 4, 5, 1, 2, 14, 8,
        3, 6, 12, 11, 9, 15, 13, 1, 5, 7, 0, 6, 3, 8,
    }},
    {32, 2, {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31, 2, 8, 21, 15, 24, 2, 7, 12, 21, 18, 13, 2, 9, 25, 21, 14,
        29, 2, 19, 10, 14, 24, 18, 8, 25, 29, 7, 18, 10, 15, 25, 12, 19, 24, 9, 29, 12, 5, 8, 19,
        13, 7, 3, 14, 8, 26, 23, 2, 5, 28, 24, 13, 30, 25, 5, 11, 21, 3, 10, 29, 13, 26, 5, 18,
        9, 3, 15, 19, 26, 30, 21, 27, 6, 2, 11, 20, 8, 30, 22, 18, 11, 27, 19, 23, 6, 18, 28, 4,
        8, 27, 17, 13, 6, 22, 25, 3, 28, 12, 22, 26, 7, 11, 30, 14, 9, 27, 22, 5, 14, 20, 6, 9,
        26, 4, 10, 22, 17, 2, 21, 16, 28, 9, 23, 17, 14, 28, 31, 19, 22, 16, 29, 11, 17, 24, 3, 27,
        20, 10, 28, 1, 11, 15, 23, 29, 20, 4, 25, 17, 31, 5, 10, 16, 30, 3, 23, 31, 13, 20, 16, 24,
        6, 31, 15, 27, 7, 16, 23, 12, 4, 30, 15, 0, 28, 7, 17, 20, 12, 1, 27, 5, 0, 18, 31, 1,
        9, 22, 0, 11, 4, 31, 7, 12, 6, 0, 10, 23, 1, 7, 29, 6, 30, 1, 17, 26, 0, 3, 29, 6,
        0, 24, 8, 1, 16, 25, 0, 14, 13, 1, 4, 19, 14, 0, 20, 15, 22, 5, 21, 1, 0, 26, 20, 24,
        21, 4, 31, 16, 26,
    }},
    {32, 3, {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31, 17, 6, 21, 28, 16, 9, 20, 25, 31, 15, 3, 11, 28, 18, 7, 13,
        25, 17, 4, 11, 23, 16, 30, 3, 7, 20, 27, 14, 6, 23, 18, 29, 9, 4, 14, 30, 21, 25, 10, 29,
        5, 16, 24, 31, 8, 18, 27, 22, 5, 17, 9, 26, 3, 13, 21, 8, 2, 15, 30, 22, 26, 12, 16, 2,
        27, 9, 19, 15, 24, 7, 1, 12, 21, 5, 0, 11, 20, 24, 2, 10, 17, 18, 1, 30, 13, 19, 5, 23,
        28, 15, 10, 4, 0, 27, 13, 31, 22, 7, 23, 0, 2, 25, 6, 12, 19, 3, 29, 11, 22, 1, 6, 10,
        26, 19, 31, 4, 12, 20, 8, 23, 1, 25, 9, 16, 0, 30, 18, 26, 11, 7, 20, 29, 13, 15, 6, 24,
        30, 4, 22, 28, 14, 24, 3, 10, 14, 5, 26, 8, 0, 19, 14, 31, 1, 2, 28, 29, 0, 8, 17, 12,
        28, 18, 27, 11, 21, 1,
    }},
    {32, 4, {
        12, 11, 23, 9, 2, 30, 13, 15, 0, 20, 19, 3, 2, 27, 6, 24, 1, 7, 0, 29, 20, 18, 28, 23,
        5, 21, 10, 27, 0, 12, 26, 22, 14, 15, 2, 10, 18, 16, 1, 25, 11, 22, 8, 29, 21, 7, 19, 10,
        9, 30, 31, 4, 12, 26, 5, 2, 29, 28, 31, 17, 11, 27, 15, 7, 3, 13, 31, 4, 18, 16, 27, 17,
        14, 5, 8, 6, 7, 30, 2, 25, 28, 21, 15, 4, 6, 24, 29, 22, 31, 0, 20, 25, 17, 26, 12, 19,
        24, 22, 28, 18, 30, 9, 1, 3, 17, 26, 12, 10, 7, 13, 16, 23, 11, 6, 0, 8, 10, 9, 4, 24,
        20, 16, 14, 11, 26, 19, 5, 4, 23, 1, 15, 31, 14, 21, 8, 1, 13, 16, 28, 30, 29, 3, 14, 27,
        9, 25, 24, 5, 13, 23, 3, 22, 17, 21, 8, 18, 26, 20, 12, 6,
    }},
    {32, 8, {
        18, 29, 17, 15, 7, 21, 1, 19, 25, 20, 10, 14, 5, 26, 24, 6, 8, 22, 11, 4, 0, 17, 27, 19,
        20, 13, 30, 31, 23, 16, 5, 14, 10, 15, 25, 0, 3, 4, 2, 12, 11, 27, 19, 17, 20, 9, 28, 5,
        10, 14, 7, 18, 29, 0, 4, 11, 27, 6, 31, 26, 1, 21, 23, 16, 13, 2, 12, 18, 8, 7, 29, 24,
        22, 30, 6, 3, 15, 26, 21, 1, 28, 9, 13, 31, 8, 23, 24, 25, 16, 22,
    }},
    {64, 3, {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 25, 20, 45, 11, 3, 17, 41, 25,
        36, 14, 60, 3, 31, 21, 49, 25, 34, 7, 13, 3, 24, 52, 10, 45, 34, 14, 53, 24, 49, 11, 60, 4,
        34, 17, 50, 46, 24, 36, 31, 20, 53, 7, 60, 46, 41, 10, 20, 49, 32, 13, 45, 60, 50, 54, 22, 3,
        46, 32, 28, 11, 7, 36, 21, 45, 4, 41, 31, 13, 22, 17, 53, 10, 21, 28, 50, 14, 20, 57, 4, 52,
        46, 25, 59, 8, 53, 3, 57, 12, 16, 25, 50, 32, 15, 41, 53, 23, 16, 4, 10, 54, 15, 31, 11, 57,
        23, 35, 45, 28, 22, 16, 34, 8, 20, 54, 35, 12, 41, 24, 28, 57, 33, 17, 49, 54, 23, 36, 32, 52,
        8, 17, 12, 55, 7, 22, 14, 52, 33, 39, 60, 12, 23, 27, 34, 55, 41, 59, 33, 47, 20, 15, 55, 39,
        3, 35, 27, 47, 53, 19, 39, 29, 34, 57, 47, 40, 32, 4, 24, 39, 8, 62, 45, 15, 27, 59, 36, 22,
        29, 40, 49, 8, 28, 19, 60, 15, 35, 40, 52, 31, 27, 16, 51, 7, 35, 59, 50, 26, 31, 8, 47, 43,
        23, 7, 33, 40, 19, 3, 51, 23, 29, 14, 46, 35, 62, 13, 21, 57, 27, 43, 10, 18, 35, 25, 42, 30,
        54, 7, 39, 18, 26, 46, 33, 21, 16, 29, 55, 60, 43, 26, 51, 20, 42, 38, 27, 8, 33, 58, 13, 51,
        39, 28, 44, 59, 54, 13, 40, 26, 30, 53, 62, 43, 38, 3, 58, 26, 44, 34, 15, 38, 30, 46, 12, 51,
        63, 4, 22, 26, 56, 11, 16, 39, 43, 49, 63, 15, 58, 7, 29, 62, 44, 4, 33, 37, 42, 53, 58, 48,
        18, 4, 28, 55, 38, 63, 13, 23, 18, 37, 50, 11, 27, 44, 63, 32, 21, 59, 12, 42, 49, 62, 55, 37,
        32, 51, 56, 18, 34, 62, 5, 48, 23, 59, 30, 6, 16, 24, 38, 47, 54, 21, 58, 5, 52, 28, 43, 37,
        6, 54, 14, 27, 48, 56, 17, 35, 63, 6, 2, 34, 42, 9, 61, 28, 47, 56, 2, 60, 38, 18, 9, 59,
        14, 43, 19, 32, 48, 26, 6, 12, 45, 18, 40, 61, 44, 14, 8, 30, 63, 41, 52, 29, 18, 1, 24, 8,
        51, 2, 21, 15, 48, 37, 24, 63, 19, 42, 31, 56, 5, 12, 29, 37, 2, 45, 30, 55, 19, 50, 6, 62,
        40, 11, 2, 58, 19, 46, 9, 5, 27, 49, 61, 2, 26, 52, 14, 0, 42, 47, 13, 36, 6, 56, 44, 49,
        1, 35, 38, 5, 0, 45, 51, 9, 33, 25, 56, 0, 43, 24, 61, 13, 19, 1, 12, 36, 43, 58, 16, 50,
        1, 63, 33, 10, 5, 26, 36, 57, 9, 63, 3, 53, 61, 30, 36, 4, 0, 27, 59, 11, 1, 47, 30, 17,
        5, 60, 44, 53, 0, 12, 6, 46, 61, 33, 11, 48, 52, 38, 7, 19, 57, 0, 18, 22, 49, 30, 58, 20,
        56, 45, 40, 46, 1, 55, 31, 48, 44, 9, 16, 24, 55, 5, 11, 25, 40, 29, 48, 10, 0, 39, 55, 21,
        61, 35, 9, 4, 30, 21, 43, 34, 1, 22, 15, 42, 5, 32, 50, 38, 61, 29, 17, 54, 9, 62, 1, 41,
        28, 26, 0, 17, 37, 8, 59, 10, 56, 2, 50, 7, 39, 9, 22, 37, 47, 62, 52, 51, 36, 6, 41, 48,
        60, 37, 3, 42, 46, 29, 13, 48, 20, 5, 8, 14, 38, 22, 48, 44, 57, 2, 20, 37, 16, 61, 7, 22,
        15, 6, 29, 21, 20, 40, 9, 0, 15, 32, 58, 35, 24, 59, 2, 33, 22, 43, 17, 40, 16, 42, 62, 0,
        23, 31, 61, 58, 41, 10, 19, 44, 25, 4, 37, 1, 7, 20, 60, 2, 53, 32, 1, 14, 58, 6, 25, 39,
        31, 38, 37, 18, 3, 36, 55, 56, 21, 15, 41, 5, 39, 1, 17, 58, 23, 2, 31, 36, 41, 10, 38, 62,
        6, 57, 1, 61, 51, 11, 17, 10, 42, 30, 57, 47, 12, 10, 18, 63, 40, 47, 25, 2, 13, 9, 23, 56,
        52, 44, 0, 34, 50, 63, 54, 19, 2, 0,
    }},
    {64, 16, {
        32, 53, 26, 0, 36, 5, 43, 48, 23, 15, 42, 30, 7, 20, 2, 29, 47, 28, 19, 33, 22, 18, 25, 39,
        63, 62, 52, 11, 57, 45, 27, 24, 6, 26, 32, 14, 60, 5, 43, 23, 38, 28, 4, 51, 49, 35, 31, 29,
        12, 21, 39, 33, 56, 13, 42, 2, 62, 9, 7, 20, 37, 52, 41, 1, 34, 55, 3, 46, 17, 16, 44, 59,
        50, 38, 23, 5, 29, 28, 26, 43, 32, 58, 56, 8, 9, 40, 61, 37, 10, 54, 41, 1, 7, 13, 62, 6,
        14, 27, 60, 57, 48, 35, 11, 0, 24, 15, 53, 45, 36, 30, 31, 3, 56, 38, 4, 21, 12, 9, 37, 25,
        41, 47, 1, 18, 19, 10, 22, 55, 63, 17, 16, 40, 3, 46, 51, 49, 48, 15, 0, 53, 34, 36, 30, 44,
        58, 61, 8, 52, 54, 50, 59, 11, 33, 21, 39, 45, 6, 20, 10, 12, 55, 42, 16, 40, 46, 2, 17, 27,
        57, 24, 4, 60, 14, 58, 31, 61, 35, 34, 59, 8, 54, 18, 44, 50, 47, 63, 19, 22, 13, 25,
    }},
    {16, 2, {
        14, 3, 6, 7, 4, 3, 1, 12, 5, 15, 1, 2, 4, 13, 15, 14, 8, 1, 9, 10, 2, 14, 0, 4,
        5, 6, 3, 10, 13, 2, 9, 8, 11, 12, 9, 7, 14, 12, 13, 3, 8, 6, 2, 12, 0, 6, 15, 9,
        0, 3, 5, 9, 13, 0, 5, 8, 7, 15, 10, 12, 4, 8, 10, 0, 1, 7, 13, 0, 11, 2, 3, 15,
        11, 4, 9, 14, 6, 10, 8, 11, 13, 6, 1, 11, 14, 5, 10, 11, 7, 2, 5,
    }, true},
    {32, 2, {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 17, 11, 26, 2, 17, 10, 7, 2, 23, 11, 7, 14, 17, 23, 6, 10, 26,
        23, 8, 14, 2, 6, 29, 17, 8, 24, 11, 6, 3, 14, 26, 29, 7, 24, 3, 17, 12, 5, 2, 24, 12,
        29, 23, 3, 9, 29, 5, 14, 10, 24, 21, 14, 9, 12, 26, 21, 3, 10, 27, 8, 3, 20, 7, 12, 27,
        6, 21, 16, 29, 10, 20, 5, 23, 27, 20, 14, 18, 11, 29, 25, 2, 21, 8, 12, 18, 10, 25, 16, 3,
        31, 18, 2, 27, 9, 17, 21, 30, 7, 18, 21, 5, 11, 16, 8, 5, 26, 18, 24, 16, 27, 30, 5, 25,
        17, 20, 13, 2, 16, 20, 26, 30, 6, 20, 24, 9, 30, 2, 22, 31, 14, 25, 22, 11, 20, 31, 8, 13,
        29, 22, 27, 13, 24, 31, 5, 19, 3, 30, 11, 19, 27, 31, 7, 16, 22, 6, 18, 9, 22, 19, 10, 13,
        30, 23, 16, 12, 19, 25, 8, 30, 4, 24, 19, 15, 2, 28, 19, 14, 4, 27, 15, 11, 4, 21, 31, 15,
        29, 1, 21, 13, 25, 6, 31, 12, 4, 20, 28, 25, 7, 4, 26, 13, 9, 4, 16, 1, 14, 30, 0, 10,
        31, 28, 9, 15, 20, 0, 29, 19, 7, 1, 5, 22, 15, 30, 12, 28, 3, 22, 0, 12, 25, 15, 18, 13,
        3, 15, 10, 4, 22, 17, 28, 1, 10, 20, 1, 12, 24, 22, 26, 1, 31, 23, 0, 21, 28, 24, 0, 11,
        9, 1, 25, 27, 0, 18, 4, 23, 25, 3, 9, 31, 26, 19, 16, 0, 13, 5, 28, 16, 9, 5, 0, 26,
        15, 7, 13, 28, 11, 26, 1, 18, 28, 8, 22, 7, 0, 17, 14, 27, 21, 19, 23, 9, 7, 6, 19, 13,
        17, 1, 8, 15, 6, 28, 23, 15, 21, 5, 1, 6, 0, 8, 19, 1, 27, 11, 4, 29, 8, 18, 30, 1,
        3, 0, 2, 4, 0, 1, 24, 15, 17, 4, 6, 23, 18, 13, 1, 23, 4, 28, 14,
    }, true},
};
// END COVER DATA

vector<int> expand(const Cover& cover, const vector<int>& labels, int k) {
    int m = labels.size();
    int width = (m + cover.blocks - 1) / cover.blocks;
    if (!cover.fits(width, k)) return {};

    vector<int> path;
    path.reserve(cover.path.size() * width);
    for (int block : cover.path) {
        int begin = block * m / cover.blocks;
        int end = (block + 1) * m / cover.blocks;
        path.insert(path.end(), labels.begin() + begin, labels.begin() + end);
    }
    return path;
}

vector<int> circle(const vector<int>& order, int k) {
    vector<int> path = order;
    int repeat = max(0, int(order.size()) - k - 1);
    path.insert(path.end(), order.begin(), order.begin() + repeat);
    return path;
}

vector<vector<int>> sweeps(const vector<int>& order, int k, int width) {
    int m = order.size();
    int blocks = (m + width - 1) / width;
    blocks += blocks % 2;
    vector<vector<int>> result;
    // The shifted zigzags partition the block pairs into Hamiltonian paths.
    // Two directions cover the entire boundary between two wide blocks.
    int directions = 2 * width <= k + 1 ? 1 : 2;
    for (int shift = 0; shift < blocks / 2; ++shift) {
        vector<int> path;
        for (int i = 0; i < blocks; ++i) {
            int offset = i % 2 ? -(i + 1) / 2 : i / 2;
            int block = (shift + offset + blocks) % blocks;
            path.push_back(block);
        }
        for (int direction = 0; direction < directions; ++direction) {
            result.emplace_back();
            for (int block : path) {
                int start = block * m / blocks, end = (block + 1) * m / blocks;
                auto& sweep = result.back();
                sweep.insert(sweep.end(), order.begin() + start, order.begin() + end);
            }
            reverse(path.begin(), path.end());
        }
    }
    return result;
}

// In a cyclic arithmetic progression, offsets 1 through k reveal the
// associated modular differences. The planner fills gaps after these walks.
struct ModularCover {
    int cafes, radius, modulus;
    initializer_list<pair<int, int>> walks;
};

constexpr ModularCover modular_covers[] = {
    {64, 4, 68, {
        {65, 24}, {47, 15}, {53, 30}, {27, 42}, {57, 4}, {43, 29}, {39, 20},
        {33, 50}, {17, 2}, {67, 44},
    }},
    {128, 4, 131, {
        {73, 2}, {25, 113}, {23, 3}, {86, 45}, {18, 75}, {55, 73}, {111, 111},
        {47, 22}, {128, 18}, {26, 47}, {120, 126}, {83, 118}, {124, 1}, {34, 24},
        {8, 60}, {66, 15}, {112, 75}, {74, 118},
    }},
    {128, 8, 131, {
        {94, 46}, {36, 86}, {14, 30}, {100, 21}, {22, 16}, {16, 84}, {79, 83},
        {3, 110}, {34, 53}, {45, 114}, {30, 70},
    }},
    {256, 8, 255, {
        {134, 53}, {89, 157}, {164, 47}, {254, 156}, {61, 68}, {165, 180}, {139, 86},
        {211, 118}, {76, 117}, {136, 79}, {151, 27}, {239, 33}, {106, 71}, {196, 110},
        {74, 106}, {124, 180}, {184, 166}, {199, 198},
    }},
};

vector<int> modular_path(const vector<int>& labels, int k) {
    int n = labels.size();
    for (const auto& cover : modular_covers) {
        if (n != cover.cafes || k != cover.radius) continue;
        vector<int> path;
        int m = cover.modulus;
        for (auto [step, start] : cover.walks) {
            int cycles = gcd(step, m);
            int length = m / cycles;
            for (int cycle = 0; cycle < cycles; ++cycle) {
                int x = (start + cycle) % m;
                for (int i = 0; i < length + min(k, length - 1); ++i) {
                    if (x < n) path.push_back(labels[x]);
                    x = (x + step) % m;
                }
            }
        }
        return path;
    }
    return {};
}

struct Plan {
    vector<int> actions;
    int cost = 0;

    void add(int action) {
        actions.push_back(action);
        cost += action != reset_memory;
    }
};

struct Planner {
    const Knowledge& original;
    mt19937& rng;
    Plan best;

    Planner(const Knowledge& original, mt19937& rng)
        : original(original), rng(rng) {
        best.cost = INT_MAX;
    }

    void evaluate(const vector<int>& prefix, const vector<int>& order, int mode) {
        // Forecast the cost if every new comparison is negative. The real
        // interaction drops redundant actions and replans after matches.
        Knowledge state = original;
        Plan result;
        auto apply = [&](int x) {
            result.add(x);
            if (x == reset_memory) state.clear_memory();
            else state.no_match(x);
        };

        for (int x : prefix) {
            if (state.remaining == 0 || result.cost >= best.cost) break;
            if (x == reset_memory || (state.degree[x] && !state.present[x])) apply(x);
        }
        while (state.remaining && result.cost < best.cost) {
            Mask eligible = state.unresolved & ~state.present;
            int oldest = -1;
            if (state.used == state.k) {
                const Mask& slot = state.slots[state.front];
                size_t x = slot._Find_first();
                if (slot._Find_next(x) >= size_t(state.n) && state.exact_count[x] == 1) {
                    oldest = x;
                }
            }

            int next = -1, best_value = INT_MIN;
            for (int x : order) {
                if (!eligible[x] || state.gain[x] == 0) continue;
                int value = 4096 * state.gain[x];
                if (mode != 1 && oldest != -1 && state.unknown[x][oldest]) value += 1024;
                if (mode == 2) value += min(state.k, state.degree[x] - state.gain[x]);
                else value -= state.degree[x];
                if (value > best_value) {
                    best_value = value;
                    next = x;
                }
            }
            if (next == -1) {
                int best_degree = 0;
                for (int x : order) {
                    if (!eligible[x]) continue;
                    int available = (state.unknown[x] & eligible).count();
                    if (available > best_degree) {
                        best_degree = available;
                        next = x;
                    }
                }
            }
            apply(next);
        }
        if (state.remaining == 0 && result.cost < best.cost) best = move(result);
    }

    Plan finish(const vector<int>& discovery_order, const vector<int>& continuation) {
        vector<int> chronological;
        for (int x : discovery_order) {
            if (original.degree[x]) chronological.push_back(x);
        }
        vector<int> labels = chronological;
        shuffle(labels.begin(), labels.end(), rng);
        evaluate({}, labels, 0);
        evaluate({}, labels, 1);
        if (!continuation.empty()) evaluate(continuation, labels, 0);

        if (labels.size() <= size_t(2 * original.k)) {
            evaluate(circle(chronological, original.k), labels, 0);
            auto cyclic = circle(labels, original.k);
            evaluate(cyclic, labels, 0);
            cyclic.insert(cyclic.begin(), reset_memory);
            evaluate(cyclic, labels, 0);
        } else {
            const Cover* candidate = nullptr;
            int estimate = INT_MAX;
            for (const auto& cover : covers) {
                int width = (int(labels.size()) + cover.blocks - 1) / cover.blocks;
                if (!cover.fits(width, original.k)) continue;
                int length = (cover.path.size() * labels.size() + cover.blocks - 1) / cover.blocks;
                if (length < estimate) {
                    candidate = &cover;
                    estimate = length;
                }
            }
            if (candidate && estimate < 2 * best.cost) {
                evaluate(expand(*candidate, labels, original.k), labels, 0);
            }
        }
        if (labels.size() > 8 && best.cost > int(labels.size())) {
            shuffle(labels.begin(), labels.end(), rng);
            evaluate({}, labels, 2);
        }
        return move(best);
    }

    Plan start(const vector<int>& labels) {
        auto modular = modular_path(labels, original.k);
        if (!modular.empty()) evaluate(modular, labels, 0);
        evaluate(labels, labels, 0);
        evaluate(labels, labels, 1);
        for (const auto& cover : covers) {
            auto path = expand(cover, labels, original.k);
            if (path.empty() || path.size() > size_t(best.cost) * 5 / 4) continue;
            evaluate(path, labels, 0);
        }
        return move(best);
    }
};


void cover_with_sweeps(Knowledge& state, const vector<int>& order) {
    vector<int> unresolved;
    for (int x : order) {
        if (state.degree[x]) unresolved.push_back(x);
    }
    auto phases = sweeps(unresolved, state.k, state.k);
    auto short_phases = sweeps(unresolved, state.k, state.k / 2);
    phases.insert(phases.end(), make_move_iterator(short_phases.begin()),
                  make_move_iterator(short_phases.end()));

    while (state.remaining) {
        int chosen = -1;
        int best_gain = 0;
        vector<int> best_path;
        for (int i = 0; i < int(phases.size()); ++i) {
            vector<int> path;
            Mask trailing, needed;
            int gain = 0;
            for (int x : phases[i]) {
                if (state.degree[x] == 0) continue;
                Mask pairs = state.unknown[x] & trailing;
                if (pairs.any()) {
                    needed.set(x);
                    needed |= pairs;
                    gain += pairs.count();
                }
                if (int(path.size()) >= state.k) {
                    trailing.reset(path[path.size() - state.k]);
                }
                trailing.set(x);
                path.push_back(x);
            }
            if (gain == 0) continue;
            int cost = needed.count();
            if (gain * int(best_path.size()) > best_gain * cost ||
                (gain * int(best_path.size()) == best_gain * cost && gain > best_gain)) {
                path.erase(remove_if(path.begin(), path.end(),
                                     [&](int x) { return !needed[x]; }), path.end());
                chosen = i;
                best_gain = gain;
                best_path = move(path);
            }
        }
        if (chosen == -1) break;
        phases[chosen].clear();
        // The reset makes each phase a sequence of first appearances. A
        // positive reply therefore leaves all earlier witnesses available.
        if (state.used) {
            reset_friend();
            state.clear_memory();
        }
        for (int x : best_path) {
            if (state.degree[x]) state.query(x);
        }
    }
}

vector<int> one_cup_trails(const Knowledge& state, int last) {
    int n = state.n;
    vector<Mask> edges = state.unknown;
    vector<int> result;
    Mask pending = state.unresolved;

    // Use the current cup first if it can start a minimum trail decomposition.
    while (pending.any()) {
        int first = last >= 0 && pending[last] ? last : int(pending._Find_first());
        vector<int> component{first};
        pending.reset(first);
        for (size_t i = 0; i < component.size(); ++i) {
            Mask neighbors = edges[component[i]] & pending;
            for (size_t y = neighbors._Find_first(); y < size_t(n);
                 y = neighbors._Find_next(y)) {
                component.push_back(y);
            }
            pending &= ~neighbors;
        }

        Mask odd;
        for (int x : component) {
            if (state.degree[x] % 2) odd.set(x);
        }

        // A dummy vertex pairs the odd endpoints of trails.
        int start = odd.any() ? n : first;
        vector<int> stack{start}, tour;
        while (!stack.empty()) {
            int x = stack.back();
            int y;
            if (x == n && stack.size() == 1 && tour.empty() && last >= 0 && odd[last]) {
                y = last;
            } else {
                y = x == n ? int(odd._Find_first()) : int(edges[x]._Find_first());
            }
            if (y < n) {
                if (x == n) odd.reset(y);
                else {
                    edges[x].reset(y);
                    edges[y].reset(x);
                }
                stack.push_back(y);
            } else if (x < n && odd[x]) {
                odd.reset(x);
                stack.push_back(n);
            } else {
                tour.push_back(x);
                stack.pop_back();
            }
        }
        for (auto it = tour.rbegin(); it != tour.rend(); ++it) {
            if (*it != n) result.push_back(*it);
        }
    }
    return result;
}

int solve(int n, int k) {
    if (n == 1) return 1;
    vector<int> labels(n);
    iota(labels.begin(), labels.end(), 0);
    mt19937 rng(131296703u + 193u * n + k);
    shuffle(labels.begin(), labels.end(), rng);

    if (n <= k + 1) {
        int distinct = 0;
        for (int x : labels) distinct += !taste(x);
        return distinct;
    }

    Knowledge state(n, k);
    if (k == 1) {
        int last = -1;
        for (int x : labels) {
            if (!state.query(x)) last = x;
        }
        while (state.remaining) {
            if (last >= 0 && n - state.alive >= max(2, n / 8)) {
                Mask separated = state.unresolved & ~state.unknown[last];
                separated.reset(last);
                int candidate = -1, overlap = 0;
                for (size_t x = state.unknown[last]._Find_first(); x < size_t(n);
                     x = state.unknown[last]._Find_next(x)) {
                    int shared = (separated & ~state.unknown[x]).count();
                    if (shared > overlap) {
                        candidate = x;
                        overlap = shared;
                    }
                }
                if (candidate >= 0) {
                    if (!state.query(candidate)) last = candidate;
                    continue;
                }
            }
            auto path = one_cup_trails(state, last);
            for (int x : path) {
                if (state.remaining == 0) break;
                if (x == last) continue;
                if (state.query(x)) break;
                last = x;
            }
        }
        return state.alive;
    }

    Plan initial;
    if (n == 2 * k) initial.actions = circle(labels, k);
    else initial = Planner(state, rng).start(labels);

    Mask visited;
    vector<int> discovery_order;
    size_t cursor = 0;
    while (cursor < initial.actions.size() && state.remaining) {
        int x = initial.actions[cursor++];
        if (x == reset_memory) {
            reset_friend();
            state.clear_memory();
            continue;
        }
        if (state.degree[x] == 0 || state.present[x]) continue;
        if (!visited[x]) {
            visited.set(x);
            discovery_order.push_back(x);
        }
        if (state.query(x)) break;
    }
    if (state.remaining) {
        for (int x : labels) {
            if (!visited[x]) {
                discovery_order.push_back(x);
                state.query(x);
            }
        }
    }
    vector<int> continuation(initial.actions.begin() + cursor, initial.actions.end());
    if (n == max_cafes && int(state.unresolved.count()) > 2 * k) {
        cover_with_sweeps(state, discovery_order);
    }
    while (state.remaining) {
        auto plan = Planner(state, rng).finish(discovery_order, continuation);
        continuation.clear();
        int replanning_threshold = state.remaining / 2;
        for (int x : plan.actions) {
            if (state.remaining == 0) break;
            if (x == reset_memory) {
                if (state.used) {
                    reset_friend();
                    state.clear_memory();
                }
            } else if (state.degree[x] && !state.present[x]) {
                if (state.query(x) && state.remaining <= replanning_threshold) break;
            }
        }
    }
    return state.alive;
}

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;
    int answer = solve(n, k);
    cout << "! " << answer << endl;
}
