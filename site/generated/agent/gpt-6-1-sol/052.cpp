#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
static constexpr int64 NEG = -(1LL << 60);
static constexpr int64 PENALTY = 1000000000000LL;

// Max-plus convolution with a concave sequence, separately in each residue
// modulo the item size. SMAWK finds every row maximum in linear time.
class ConcaveConvolution {
    const vector<int64>& a;
    const vector<int64>& b;
    int step, residue = 0;
    vector<int> best;

    inline int64 value(int row, int col) const {
        int d = row - col;
        int64 bonus;
        if (d < 0) bonus = b.front() + PENALTY * d;
        else if (d >= (int)b.size())
            bonus = b.back() - PENALTY * (d - (int)b.size() + 1);
        else bonus = b[d];
        return a[residue + col * step] + bonus;
    }

    void smawk(const vector<int>& rows, const vector<int>& columns) {
        if (rows.empty()) return;
        vector<int> reduced;
        reduced.reserve(min(rows.size(), columns.size()));
        for (int col : columns) {
            while (!reduced.empty()) {
                int row = rows[reduced.size() - 1];
                if (value(row, col) >= value(row, reduced.back()))
                    reduced.pop_back();
                else break;
            }
            if (reduced.size() < rows.size()) reduced.push_back(col);
        }
        vector<int> odd;
        odd.reserve(rows.size() / 2);
        for (int i = 1; i < (int)rows.size(); i += 2)
            odd.push_back(rows[i]);
        smawk(odd, reduced);

        int left = 0;
        for (int i = 0; i < (int)rows.size(); i += 2) {
            int right = (int)reduced.size() - 1;
            if (i + 1 < (int)rows.size()) {
                right = left;
                while (reduced[right] != best[rows[i + 1]]) ++right;
            }
            int row = rows[i], choice = reduced[left];
            int64 maximum = value(row, choice);
            for (int j = left + 1; j <= right; ++j) {
                int64 current = value(row, reduced[j]);
                if (current >= maximum) {
                    maximum = current;
                    choice = reduced[j];
                }
            }
            best[row] = choice;
            left = right;
        }
    }

public:
    ConcaveConvolution(const vector<int64>& a_, const vector<int64>& b_, int step_)
        : a(a_), b(b_), step(step_) {}

    vector<int64> run(int limit) {
        if (a.empty() || b.empty() || limit < 0) return {};
        int last = min<int64>(limit, (int)a.size() - 1 +
                                      (int64)((int)b.size() - 1) * step);
        vector<int64> result(last + 1, NEG);
        if (b.size() == 1) {
            for (int i = 0; i <= last; ++i)
                if (a[i] != NEG) result[i] = a[i] + b[0];
            return result;
        }
        for (residue = 0; residue < step && residue < (int)a.size()
                                              && residue <= last; ++residue) {
            int nr = (last - residue) / step + 1;
            int nc = min(nr, ((int)a.size() - 1 - residue) / step + 1);
            if (nc == 1) {
                if (a[residue] == NEG) continue;
                for (int row = 0; row < nr && row < (int)b.size(); ++row)
                    result[residue + row * step] = a[residue] + b[row];
                continue;
            }
            vector<int> rows(nr), columns(nc);
            iota(rows.begin(), rows.end(), 0);
            iota(columns.begin(), columns.end(), 0);
            best.resize(nr);
            smawk(rows, columns);
            for (int row = 0; row < nr; ++row) {
                int64 maximum = value(row, best[row]);
                // The artificial extension of b is negative enough that
                // only an actual feasible choice can have a nonnegative value.
                if (maximum >= 0) result[residue + row * step] = maximum;
            }
        }
        return result;
    }
};

struct Item {
    array<int, 10> w{};
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;
    array<vector<Item>, 11> groups;
    int totalWeight = 0;
    int64 totalValue = 0;
    for (int i = 0; i < n; ++i) {
        int p;
        cin >> p;
        Item item;
        for (int j = 0; j < p; ++j) cin >> item.w[j];
        groups[p].push_back(item);
        totalWeight += p;
        totalValue += item.w[p - 1];
    }
    if (k == 0) {
        cout << 0 << '\n';
        return 0;
    }
    if (totalWeight <= k) {
        cout << totalValue << '\n';
        return 0;
    }

    // full[s]: all selected items are fully upgraded.
    // partial[s]: exactly one selected item is partially upgraded.
    vector<int64> full(1, 0), partial;
    for (int p = 10; p >= 1; --p) {
        auto& items = groups[p];
        int m = items.size();
        if (m == 0) continue;
        sort(items.begin(), items.end(), [p](const Item& x, const Item& y) {
            return x.w[p - 1] > y.w[p - 1];
        });
        vector<int64> prefix(m + 1, 0);
        for (int i = 0; i < m; ++i)
            prefix[i + 1] = prefix[i] + items[i].w[p - 1];
        vector<int64> normal(prefix.begin(), prefix.begin() + min(m, k / p) + 1);
        vector<int64> nextFull = ConcaveConvolution(full, normal, p).run(k);
        vector<int64> nextPartial = ConcaveConvolution(partial, normal, p).run(k);

        for (int r = 1; r < p && r <= k; ++r) {
            // special[c] chooses c full items and one distinct partial item.
            // A partial item outside the top c leaves prefix[c] intact;
            // one inside the top c+1 replaces its full value by its bonus.
            // This sequence is concave: it is the maximum-weight assignment
            // to c identical full slots and one fixed partial slot. Varying
            // the capacity of the full slots gives nonincreasing marginals.
            vector<int64> suffix(m + 1, NEG);
            for (int i = m - 1; i >= 0; --i)
                suffix[i] = max(suffix[i + 1], (int64)items[i].w[r - 1]);
            int maxCount = min(m - 1, (k - r) / p);
            vector<int64> special(maxCount + 1);
            int64 bestReplacement = NEG;
            for (int c = 0; c <= maxCount; ++c) {
                bestReplacement = max(bestReplacement,
                    (int64)items[c].w[r - 1] - items[c].w[p - 1]);
                special[c] = max(prefix[c] + suffix[c],
                                 prefix[c + 1] + bestReplacement);
            }
            auto candidate = ConcaveConvolution(full, special, p).run(k - r);
            if (nextPartial.size() < candidate.size() + r)
                nextPartial.resize(candidate.size() + r, NEG);
            for (int s = 0; s < (int)candidate.size(); ++s)
                nextPartial[s + r] = max(nextPartial[s + r], candidate[s]);
        }
        full.swap(nextFull);
        partial.swap(nextPartial);
    }

    int64 answer = NEG;
    if (k < (int)full.size()) answer = max(answer, full[k]);
    if (k < (int)partial.size()) answer = max(answer, partial[k]);
    cout << answer << '\n';
    return 0;
}
