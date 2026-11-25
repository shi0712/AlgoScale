#include <array>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

using namespace std;

int ask(const vector<int>& mushrooms) {
    cout << "? " << mushrooms.size();
    for (int x : mushrooms) cout << ' ' << x;
    cout << '\n' << flush;

    int answer;
    if (!(cin >> answer)) exit(0);
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> known[2];
    known[0].push_back(0);
    auto add = [&](int species, int mushroom) {
        known[species].push_back(mushroom);
    };

    int next = 1;
    while (next < n && next < 3) {
        add(ask({0, next}), next);
        ++next;
    }

    auto ready = [&]() {
        return known[0].size() >= 2 && known[1].size() >= 2 &&
               (known[0].size() >= 3 || known[1].size() >= 3);
    };

    while (next < n && !ready()) {
        int majority = known[1].size() > known[0].size() ? 1 : 0;
        if (next + 1 == n) {
            add(ask({0, next}), next);
            ++next;
            continue;
        }

        int x = next, y = next + 1;
        int result = ask({known[majority][0], x, known[majority][1], y});
        add(majority ^ ((result >> 1) & 1), x);
        add(majority ^ (result & 1), y);
        next += 2;
    }

    // Unknowns are 0..4, majority anchors are 5..7, minority anchors are 8..9.
    const array<vector<int>, 2> pattern = {{
        {1, 7, 0, 6, 4, 5, 9, 2, 8, 3},
        {3, 9, 8, 4, 1, 2, 5, 7, 0, 6}
    }};

    auto signature = [&](const vector<int>& seq, int mask) {
        auto species = [&](int item) {
            if (item < 5) return (mask >> item) & 1;
            return item >= 8 ? 1 : 0;
        };
        int result = 0;
        for (int i = 0; i + 1 < (int)seq.size(); ++i)
            result += species(seq[i]) != species(seq[i + 1]);
        return result;
    };

    map<pair<int, int>, int> decode;
    for (int mask = 0; mask < 32; ++mask)
        decode[{signature(pattern[0], mask), signature(pattern[1], mask)}] = mask;

    while (next + 5 <= n) {
        int majority = known[1].size() > known[0].size() ? 1 : 0;
        int minority = majority ^ 1;
        array<int, 2> result{};

        for (int q = 0; q < 2; ++q) {
            vector<int> sequence;
            for (int item : pattern[q]) {
                if (item < 5) sequence.push_back(next + item);
                else if (item < 8) sequence.push_back(known[majority][item - 5]);
                else sequence.push_back(known[minority][item - 8]);
            }
            result[q] = ask(sequence);
        }

        auto it = decode.find({result[0], result[1]});
        if (it == decode.end()) return 0;
        int mask = it->second;
        for (int j = 0; j < 5; ++j)
            add(majority ^ ((mask >> j) & 1), next + j);
        next += 5;
    }

    while (next + 2 <= n) {
        int majority = known[1].size() > known[0].size() ? 1 : 0;
        int result = ask({known[majority][0], next,
                          known[majority][1], next + 1});
        add(majority ^ ((result >> 1) & 1), next);
        add(majority ^ (result & 1), next + 1);
        next += 2;
    }

    if (next < n) add(ask({0, next}), next);

    cout << "! " << known[0].size() << '\n' << flush;
    return 0;
}
