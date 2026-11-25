#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int n, k;

bool ask(int c) {
    cout << "? " << c << '\n' << flush;
    string s;
    if (!(cin >> s) || s == "E") exit(0);
    return s == "Y";
}

void resetMemory() {
    cout << "R\n" << flush;
}

vector<char> reverseCompare(const vector<int>& a, const vector<int>& b) {
    int as = (int)a.size(), bs = (int)b.size();
    vector<char> present(bs, false);

    resetMemory();
    for (int x : a) ask(x);
    for (int j = 0; j < bs; ++j) {
        if (ask(b[j])) present[j] = true;
    }

    int secondA = 0;
    for (int j = 0; j < bs; ++j) {
        if (!present[j]) secondA = max(secondA, as + j - k);
    }
    resetMemory();
    for (int i = secondA - 1; i >= 0; --i) ask(a[i]);
    for (int j = bs - 1; j >= 0; --j) {
        if (!present[j] && as + j - k > 0) {
            if (ask(b[j])) present[j] = true;
        }
    }
    return present;
}

vector<char> compareSets(const vector<int>& a, const vector<int>& b,
                         bool uniqueA, bool lastGroup) {
    int as = (int)a.size(), bs = (int)b.size();
    vector<char> present(bs, false);

    resetMemory();
    for (int x : a) ask(x);
    bool any = false;
    for (int j = 0; j < bs; ++j) {
        if (ask(b[j])) {
            present[j] = true;
            any = true;
        }
    }

    if (any) {
        int secondA = 0;
        for (int j = 0; j < bs; ++j) {
            if (!present[j]) secondA = max(secondA, as + j - k);
        }
        resetMemory();
        for (int i = secondA - 1; i >= 0; --i) ask(a[i]);
        for (int j = bs - 1; j >= 0; --j) {
            if (!present[j] && as + j - k > 0) {
                if (ask(b[j])) present[j] = true;
            }
        }
        return present;
    }

    int secondA = min(as, as + bs - k - 1);
    int common = 0;
    for (int i = 0; i < secondA; ++i) {
        if (ask(a[i])) ++common;
    }

    if (common == 0) return present;
    if (uniqueA && common == bs) {
        fill(present.begin(), present.end(), true);
        return present;
    }
    if (uniqueA && lastGroup) {
        fill(present.begin(), present.end(), false);
        for (int i = 0; i < common; ++i) present[i] = true;
        return present;
    }
    return reverseCompare(a, b);
}

vector<char> comparePrevious(const vector<int>& a, const vector<int>& b) {
    vector<char> present(b.size(), false);
    int lastPosition = (b.back() - 1) % k;
    if (lastPosition == 0) return present;
    resetMemory();
    for (int i = lastPosition - 1; i >= 0; --i) ask(a[i]);
    for (int j = (int)b.size() - 1; j >= 0; --j) {
        if ((b[j] - 1) % k > 0 && ask(b[j])) present[j] = true;
    }
    return present;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> k)) return 0;
    int blocks = n / k;
    vector<vector<int>> reps(blocks);
    int repeated = 0;

    resetMemory();
    for (int cafe = 1; cafe <= n; ++cafe) {
        if (ask(cafe)) {
            ++repeated;
        } else {
            reps[(cafe - 1) / k].push_back(cafe);
        }
    }

    struct Group {
        vector<int> values;
        bool unique;
    };
    for (int block = 1; block < blocks; ++block) {
        vector<int> unresolved = reps[block];
        vector<Group> groups;
        vector<int> previous(k);
        for (int i = 0; i < k; ++i) previous[i] = (block - 1) * k + i + 1;
        groups.push_back({previous, (int)reps[block - 1].size() == k});

        vector<int> packed;
        int packedBlocks = 0;
        for (int i = block - 2; i >= 0; --i) {
            if (!packed.empty() && (int)packed.size() + (int)reps[i].size() > k) {
                groups.push_back({packed, packedBlocks == 1});
                packed.clear();
                packedBlocks = 0;
            }
            packed.insert(packed.end(), reps[i].begin(), reps[i].end());
            ++packedBlocks;
        }
        if (!packed.empty()) groups.push_back({packed, packedBlocks == 1});

        for (int g = 0; g < (int)groups.size() && !unresolved.empty(); ++g) {
            if (groups[g].values.empty()) continue;
            bool last = g == (int)groups.size() - 1;
            vector<char> present = g == 0
                ? comparePrevious(groups[g].values, unresolved)
                : compareSets(groups[g].values, unresolved, groups[g].unique, last);
            vector<int> left;
            for (int i = 0; i < (int)unresolved.size(); ++i) {
                if (present[i]) {
                    ++repeated;
                } else {
                    left.push_back(unresolved[i]);
                }
            }
            unresolved.swap(left);
        }
    }

    cout << "! " << n - repeated << '\n' << flush;
    return 0;
}
