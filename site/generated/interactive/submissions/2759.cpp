#include <bits/stdc++.h>
using namespace std;

class Fenwick {
    int n;
    vector<int> bit;

public:
    explicit Fenwick(int n = 0) : n(n), bit(n + 1, 0) {}

    void buildOnes() {
        for (int i = 1; i <= n; ++i) {
            bit[i] = i & -i;
        }
    }

    void add(int pos, int value) {
        for (; pos <= n; pos += pos & -pos) {
            bit[pos] += value;
        }
    }

    int sum(int pos) const {
        int result = 0;
        for (; pos > 0; pos -= pos & -pos) {
            result += bit[pos];
        }
        return result;
    }

    int rangeSum(int l, int r) const {
        if (l > r) return 0;
        return sum(r) - sum(l - 1);
    }

    int kth(int rank) const {
        int pos = 0;
        int step = 1;
        while ((step << 1) <= n) step <<= 1;

        for (; step > 0; step >>= 1) {
            int next = pos + step;
            if (next <= n && bit[next] < rank) {
                pos = next;
                rank -= bit[next];
            }
        }
        return pos + 1;
    }
};

class Game {
    struct AffectedInterval {
        int node;
        int l;
        int r;
    };

    int n;

    vector<int> total;
    vector<unsigned char> hasTotal;
    vector<unsigned char> isKnown;

    vector<int> nextUnknown;

    Fenwick knownZeros;
    Fenwick unknownPositions;

    int findNextUnknown(int pos) {
        while (nextUnknown[pos] != pos) {
            nextUnknown[pos] = nextUnknown[nextUnknown[pos]];
            pos = nextUnknown[pos];
        }
        return pos;
    }

    void markRange(int l, int r, bool makeZero) {
        int pos = findNextUnknown(l);

        while (pos <= r) {
            isKnown[pos] = 1;
            unknownPositions.add(pos, -1);

            if (makeZero) {
                knownZeros.add(pos, 1);
            }

            nextUnknown[pos] = findNextUnknown(pos + 1);
            pos = nextUnknown[pos];
        }
    }

    bool fullyKnown(int l, int r) const {
        return unknownPositions.rangeSum(l, r) == 0;
    }

    void completeInterval(int node, int l, int r, int zeroCount) {
        int knownZeroCount = knownZeros.rangeSum(l, r);
        int unknownCount = unknownPositions.rangeSum(l, r);
        int unknownZeroCount = zeroCount - knownZeroCount;

        if (unknownCount == 0) return;
        if (unknownZeroCount < 0 || unknownZeroCount > unknownCount) return;

        if (unknownZeroCount == 0) {
            markRange(l, r, false);
        } else if (unknownZeroCount == unknownCount) {
            markRange(l, r, true);
        }
    }

    void setTotal(int node, int value) {
        hasTotal[node] = 1;
        total[node] = value;
    }

    void pullAndComplete(int node, int l, int r) {
        if (l == r) return;

        int mid = (l + r) / 2;
        int left = node * 2;
        int right = left + 1;

        if (hasTotal[left] && hasTotal[right]) {
            setTotal(node, total[left] + total[right]);
        }

        if (hasTotal[node]) {
            completeInterval(node, l, r, total[node]);
        }
    }

    int ask(int l, int r) {
        cout << "? " << l << ' ' << r << endl;

        int ones;
        if (!(cin >> ones) || ones < 0) {
            exit(0);
        }
        if (ones > r - l + 1) {
            exit(0);
        }

        return (r - l + 1) - ones;
    }

    int locateNode(int node, int l, int r, int k) {
        if (fullyKnown(l, r)) {
            int zeroCount = knownZeros.rangeSum(l, r);
            setTotal(node, zeroCount);

            int globalRank = knownZeros.sum(l - 1) + k;
            return knownZeros.kth(globalRank);
        }

        if (l == r) {
            if (hasTotal[node]) {
                completeInterval(node, l, r, total[node]);
            }
            return l;
        }

        int mid = (l + r) / 2;
        int left = node * 2;
        int right = left + 1;

        if (!hasTotal[left]) {
            if (hasTotal[node] && hasTotal[right]) {
                setTotal(left, total[node] - total[right]);
            } else if (fullyKnown(l, mid)) {
                setTotal(left, knownZeros.rangeSum(l, mid));
            } else {
                int zeroCount = ask(l, mid);
                setTotal(left, zeroCount);
            }
        }

        if (hasTotal[node] && !hasTotal[right]) {
            setTotal(right, total[node] - total[left]);
        }

        if (hasTotal[left]) {
            completeInterval(left, l, mid, total[left]);
        }
        if (hasTotal[right]) {
            completeInterval(right, mid + 1, r, total[right]);
        }

        int answer;
        if (k <= total[left]) {
            answer = locateNode(left, l, mid, k);
        } else {
            answer = locateNode(right, mid + 1, r, k - total[left]);
        }

        pullAndComplete(node, l, r);
        return answer;
    }

    void decrementTotals(int node, int l, int r, int x,
                         vector<AffectedInterval>& affected) {
        if (hasTotal[node]) {
            --total[node];
            affected.push_back({node, l, r});
        }

        if (l == r) return;

        int mid = (l + r) / 2;
        if (x <= mid) {
            decrementTotals(node * 2, l, mid, x, affected);
        } else {
            decrementTotals(node * 2 + 1, mid + 1, r, x, affected);
        }
    }

    void pullPath(int node, int l, int r, int x) {
        if (l == r) return;

        int mid = (l + r) / 2;
        if (x <= mid) {
            pullPath(node * 2, l, mid, x);
        } else {
            pullPath(node * 2 + 1, mid + 1, r, x);
        }

        pullAndComplete(node, l, r);
    }

public:
    explicit Game(int size)
        : n(size),
          total(4 * size + 5, 0),
          hasTotal(4 * size + 5, 0),
          isKnown(size + 2, 0),
          nextUnknown(size + 2),
          knownZeros(size),
          unknownPositions(size) {
        for (int i = 1; i <= n + 1; ++i) {
            nextUnknown[i] = i;
        }
        unknownPositions.buildOnes();
    }

    int locate(int k) {
        return locateNode(1, 1, n, k);
    }

    void applyFlip(int x) {
        if (isKnown[x]) {
            knownZeros.add(x, -1);
        } else {
            isKnown[x] = 1;
            unknownPositions.add(x, -1);
        }

        vector<AffectedInterval> affected;
        affected.reserve(32);
        decrementTotals(1, 1, n, x, affected);

        for (const auto& interval : affected) {
            completeInterval(interval.node, interval.l, interval.r,
                             total[interval.node]);
        }

        pullPath(1, 1, n, x);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, t;
    if (!(cin >> n >> t)) return 0;

    Game game(n);

    for (int round = 0; round < t; ++round) {
        int k;
        if (!(cin >> k) || k < 0) return 0;

        int answer = game.locate(k);
        game.applyFlip(answer);

        cout << "! " << answer << endl;
    }

    return 0;
}
