#include <bits/stdc++.h>
using namespace std;

static int responseFor(int x, int y, int z) {
    if (x > y) swap(x, y);
    if (y > z) swap(y, z);
    if (x > y) swap(x, y);
    if (x + y <= z) return 0;
    int p = x + y + z;
    return p * (p - 2 * x) * (p - 2 * y) * (p - 2 * z);
}

static bool goodPair(int x, int y) {
    for (int z = 1; z <= 4; ++z)
        for (int w = z + 1; w <= 4; ++w)
            if (responseFor(x, y, z) == responseFor(x, y, w)) return false;
    return true;
}

static int ask(int i, int j, int k) {
    if (i > j) swap(i, j);
    if (j > k) swap(j, k);
    if (i > j) swap(i, j);
    cout << "? " << i << ' ' << j << ' ' << k << '\n' << flush;
    int r;
    if (!(cin >> r)) exit(0);
    return r;
}

static void answer(const vector<int>& a) {
    cout << '!';
    for (int x : a) cout << ' ' << x;
    cout << '\n' << flush;
    exit(0);
}

[[noreturn]] static void impossible() {
    cout << "! -1\n" << flush;
    exit(0);
}

struct BlockSolver {
    int m, queryCount, candidateCount;
    vector<array<int, 3>> queries;
    vector<vector<uint16_t>> table;

    explicit BlockSolver(int size) : m(size) {
        for (int i = 0; i < m; ++i)
            for (int j = i + 1; j < m; ++j)
                for (int k = j + 1; k < m; ++k)
                    queries.push_back({i, j, k});
        queryCount = queries.size();
        candidateCount = 1;
        for (int i = 0; i < m; ++i) candidateCount *= 4;
        table.assign(queryCount, vector<uint16_t>(candidateCount));
        for (int code = 0; code < candidateCount; ++code) {
            int x = code, a[9];
            for (int i = 0; i < m; ++i) {
                a[i] = x % 4 + 1;
                x /= 4;
            }
            for (int q = 0; q < queryCount; ++q) {
                auto [i, j, k] = queries[q];
                table[q][code] = responseFor(a[i], a[j], a[k]);
            }
        }
    }

    vector<int> decode(int code) const {
        vector<int> a(m);
        for (int i = 0; i < m; ++i) {
            a[i] = code % 4 + 1;
            code /= 4;
        }
        return a;
    }

    pair<vector<int>, bool> solve(const vector<int>& indices) const {
        vector<int> candidates(candidateCount);
        iota(candidates.begin(), candidates.end(), 0);
        vector<char> used(queryCount, false);
        int asked = 0;

        while (candidates.size() > 1) {
            int best = -1, bestMax = 0;
            long long bestScore = LLONG_MAX;
            if (asked == 0) best = 0;
            for (int q = 0; q < queryCount && asked > 0; ++q) {
                if (used[q]) continue;
                int counts[769] = {};
                for (int code : candidates) ++counts[table[q][code]];
                long long score = 0;
                int mx = 0;
                for (int c : counts) if (c) {
                    score += 1LL * c * c;
                    mx = max(mx, c);
                }
                if (score < bestScore || (score == bestScore && mx < bestMax)) {
                    bestScore = score;
                    bestMax = mx;
                    best = q;
                }
            }
            if (best < 0 || bestScore == 1LL * candidates.size() * candidates.size()) break;
            used[best] = true;
            auto [i, j, k] = queries[best];
            int r = ask(indices[i], indices[j], indices[k]);
            vector<int> next;
            for (int code : candidates)
                if (table[best][code] == r) next.push_back(code);
            candidates.swap(next);
            ++asked;
        }

        if (candidates.size() == 1) return {decode(candidates[0]), true};
        vector<int> common = decode(candidates[0]);
        for (int code : candidates) {
            vector<int> a = decode(code);
            for (int i = 0; i < m; ++i)
                if (a[i] != common[i]) common[i] = -1;
        }
        return {common, false};
    }
};

static int classify(int p, int q, int x, int y, int index) {
    int r = ask(p, q, index);
    for (int z = 1; z <= 4; ++z)
        if (responseFor(x, y, z) == r) return z;
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    vector<unique_ptr<BlockSolver>> solvers(10);
    auto getSolver = [&](int m) -> BlockSolver& {
        if (!solvers[m]) solvers[m] = make_unique<BlockSolver>(m);
        return *solvers[m];
    };

    if (n <= 9) {
        vector<int> indices(n);
        iota(indices.begin(), indices.end(), 1);
        auto result = getSolver(n).solve(indices);
        if (!result.second) impossible();
        answer(result.first);
    }

    int blocks = (n + 8) / 9;
    int small = n / blocks, extra = n % blocks;
    vector<vector<int>> indices(blocks);
    vector<pair<vector<int>, bool>> results(blocks);
    int at = 1;
    for (int b = 0; b < blocks; ++b) {
        int size = small + (b < extra);
        for (int i = 0; i < size; ++i) indices[b].push_back(at++);
        results[b] = getSolver(size).solve(indices[b]);
    }

    int p = -1, q = -1, x = 0, y = 0;
    for (int b = 0; b < blocks && p == -1; ++b) {
        if (!results[b].second) continue;
        for (int i = 0; i < (int)indices[b].size() && p == -1; ++i)
            for (int j = i + 1; j < (int)indices[b].size(); ++j)
                if (goodPair(results[b].first[i], results[b].first[j])) {
                    p = indices[b][i];
                    q = indices[b][j];
                    x = results[b].first[i];
                    y = results[b].first[j];
                    break;
                }
    }

    if (p != -1) {
        vector<int> a(n + 1);
        for (int b = 0; b < blocks; ++b) {
            for (int i = 0; i < (int)indices[b].size(); ++i) {
                int index = indices[b][i];
                if (results[b].second) a[index] = results[b].first[i];
                else if (results[b].first[i] != -1) a[index] = results[b].first[i];
                else a[index] = classify(p, q, x, y, index);
            }
        }
        vector<int> output(a.begin() + 1, a.end());
        answer(output);
    }

    bool allUnique = true;
    for (auto& result : results) allUnique &= result.second;
    if (allUnique) {
        vector<int> output;
        for (auto& result : results)
            output.insert(output.end(), result.first.begin(), result.first.end());
        answer(output);
    }

    int oneA = -1, oneB = -1;
    vector<int> a(n + 1, 1);
    vector<char> known(n + 1, false);
    vector<int> nonones;
    for (int b = 0; b < blocks; ++b) {
        for (int i = 0; i < (int)indices[b].size(); ++i) {
            int index = indices[b][i], value = results[b].first[i];
            if (value == -1) continue;
            a[index] = value;
            known[index] = true;
            if (value == 1) {
                if (oneA == -1) oneA = index;
                else if (oneB == -1 && index != oneA) oneB = index;
            } else nonones.push_back(index);
        }
    }
    for (int i = 1; i <= n; ++i) {
        if (known[i]) continue;
        int r = ask(oneA, oneB, i);
        if (r != 3) nonones.push_back(i);
    }
    if (nonones.empty()) {
        vector<int> output(a.begin() + 1, a.end());
        answer(output);
    }
    if (nonones.size() <= 2) {
        if (nonones.size() == 1) impossible();
        int r = ask(oneA, nonones[0], nonones[1]);
        if (r == 0) impossible();
        int v = (r == 15 ? 2 : r == 35 ? 3 : 4);
        a[nonones[0]] = a[nonones[1]] = v;
        vector<int> output(a.begin() + 1, a.end());
        answer(output);
    }

    p = q = -1;
    for (int i = 0; i < (int)nonones.size() && p == -1; ++i)
        for (int j = i + 1; j < (int)nonones.size(); ++j) {
            int r = ask(oneA, nonones[i], nonones[j]);
            if (r != 0) {
                p = nonones[i];
                q = nonones[j];
                x = y = (r == 15 ? 2 : r == 35 ? 3 : 4);
                break;
            }
        }
    if (p == -1) impossible();
    a[p] = a[q] = x;
    for (int index : nonones)
        if (index != p && index != q) a[index] = classify(p, q, x, y, index);
    vector<int> output(a.begin() + 1, a.end());
    answer(output);
}
