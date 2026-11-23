#include <bits/stdc++.h>
using namespace std;

int n, k;
bool running = true;

char ask(int c) {
    if (!running) return 0;
    cout << "? " << c << '\n' << flush;
    string s;
    if (!(cin >> s)) {
        running = false;
        return 0;
    }
    if (s == "Y" || s == "N") return s[0];
    running = false;
    return 0;
}

void resetMemory() {
    if (!running) return;
    cout << "R\n" << flush;
}

struct Plan {
    int x = 0, y = 0;
    long long cost = LLONG_MAX;
    int resets = INT_MAX;
};

Plan getPlan(int a, int b) {
    Plan best;
    for (int p = 1; p <= a; ++p) {
        int x = (a + p - 1) / p;
        int remaining = k + 1 - x;
        if (remaining <= 0) continue;

        int q = (b + remaining - 1) / remaining;
        q = max(q, 1);
        int y = (b + q - 1) / q;
        if (x + y > k + 1) continue;

        int actualP = (a + x - 1) / x;
        int actualQ = (b + y - 1) / y;
        long long cost = 1LL * actualQ * a + 1LL * actualP * b;
        int resets = actualP * actualQ;

        if (cost < best.cost || (cost == best.cost && resets < best.resets)) {
            best = {x, y, cost, resets};
        }
    }
    return best;
}

struct MergeResult {
    vector<int> safe;
    vector<int> risky;
};

// A and B are internally duplicate-free. Return representatives of A union B.
// "safe" comes from A, while "risky" comes from B.
MergeResult genericMerge(const vector<int>& A, const vector<int>& B, const Plan& plan) {
    int a = (int)A.size(), b = (int)B.size();
    vector<char> matched(b, false);

    for (int as = 0; as < a && running; as += plan.x) {
        int ae = min(a, as + plan.x);
        int aLen = ae - as;

        for (int bs = 0; bs < b && running; bs += plan.y) {
            int be = min(b, bs + plan.y);
            vector<int> todo;
            for (int j = bs; j < be; ++j) {
                if (!matched[j]) todo.push_back(j);
            }
            if (todo.empty()) continue;

            resetMemory();
            for (int i = as; i < ae; ++i) {
                ask(A[i]);
                if (!running) return {};
            }

            int localMatches = 0;
            for (int j : todo) {
                char response = ask(B[j]);
                if (!running) return {};
                if (response == 'Y') {
                    matched[j] = true;
                    ++localMatches;
                }

                // Every remaining B value is different from all values
                // already matched, so no other B value can match this A chunk.
                if (localMatches == aLen) break;
            }
        }
    }

    MergeResult result;
    result.safe = A;
    for (int j = 0; j < b; ++j) {
        if (!matched[j]) result.risky.push_back(B[j]);
    }
    return result;
}

// Faster merge when A contains exactly k distinct values.
MergeResult fullMerge(const vector<int>& A, const vector<int>& B) {
    int r = (int)B.size();
    vector<char> backward(r, false);

    resetMemory();
    for (int x : A) {
        ask(x);
        if (!running) return {};
    }

    int notBackward = 0;
    int lastNotBackward = 0;
    for (int j = 1; j <= r; ++j) {
        char response = ask(B[j - 1]);
        if (!running) return {};
        if (response == 'Y') {
            backward[j - 1] = true;
        } else {
            ++notBackward;
            lastNotBackward = j;
        }
    }

    // Every B value already has a representative in A.
    if (notBackward == 0) return {A, {}};

    vector<char> removeA(k, false);

    // A forward match is possible only when B_lastNotBackward is not
    // backward-matched and some earlier A value is compared with it.
    if (lastNotBackward > 1) {
        int dummyCount = k - r + 1;
        for (int t = 0; t < dummyCount; ++t) {
            ask(A.back());
            if (!running) return {};
        }

        int forwardMatches = 0;
        for (int i = 1; i < r && i < lastNotBackward; ++i) {
            char response = ask(A[i - 1]);
            if (!running) return {};
            if (response == 'Y') {
                removeA[i - 1] = true;
                ++forwardMatches;
            }
            if (forwardMatches == notBackward) break;
        }
    }

    MergeResult result;
    for (int i = 0; i < k; ++i) {
        if (!removeA[i]) result.safe.push_back(A[i]);
    }
    for (int j = 0; j < r; ++j) {
        if (!backward[j]) result.risky.push_back(B[j]);
    }
    return result;
}

MergeResult mergeBlock(const vector<int>& A, const vector<int>& B) {
    Plan plan = getPlan((int)A.size(), (int)B.size());
    if ((int)A.size() == k &&
        2LL * k + (int)B.size() <= plan.cost) {
        return fullMerge(A, B);
    }
    return genericMerge(A, B, plan);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> k)) return 0;

    vector<vector<int>> blocks;

    for (int start = 1; start <= n && running; start += k) {
        resetMemory();

        vector<int> candidates;
        for (int c = start; c < start + k; ++c) {
            char response = ask(c);
            if (!running) return 0;
            if (response == 'N') candidates.push_back(c);
        }

        vector<vector<int>> oldBlocks = move(blocks);
        blocks.clear();

        vector<int> safe;
        vector<int> risky = move(candidates);

        for (const vector<int>& A : oldBlocks) {
            if (risky.empty()) {
                safe.insert(safe.end(), A.begin(), A.end());
                continue;
            }

            MergeResult merged = mergeBlock(A, risky);
            if (!running) return 0;

            safe.insert(safe.end(), merged.safe.begin(), merged.safe.end());
            risky = move(merged.risky);
        }

        safe.insert(safe.end(), risky.begin(), risky.end());

        for (int x : safe) {
            if (blocks.empty() || (int)blocks.back().size() == k) {
                blocks.emplace_back();
            }
            blocks.back().push_back(x);
        }
    }

    if (!running) return 0;

    int diversity = 0;
    for (const auto& block : blocks) diversity += (int)block.size();

    cout << "! " << diversity << '\n' << flush;
    return 0;
}
