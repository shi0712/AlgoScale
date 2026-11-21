#include <bits/stdc++.h>
using namespace std;

static int N;

static int askLess(int i, int j) {
    printf("? %d %d\n", i, j);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);   // judge reported a failure: terminate immediately
    return r;
}

static int doAdd(int i, int j) {
    printf("+ %d %d\n", i, j);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}

// The board is kept in "ord" sorted by value (labels, ascending).
//
// Why it is safe to always add the current minimum and maximum:
// let the board values be v_1..v_k with sum T (= total of all A_i) and
// |v_i| <= R, |T| <= R.  If min <= 0 <= max then min+max lies between them,
// so its magnitude is at most R.  If every value is positive then
// T >= min+max, hence min+max <= T <= R.  If every value is negative then
// T <= min+max, hence min+max >= T >= -R.  So min+max always fits.
//
// The comparisons are spent sorting the initial values and locating each
// merged value inside the sorted array.  Two cheap adaptive shortcuts cut
// this down when the data is ordered or has many equal values.

static vector<int> ord;
static bool streakMax = false;  // recent insertions went to the max end
static int streakLen = 0;
static int prevIdx = -1;        // index in ord of the last inserted element
static int closeStreak = 0;     // recent insertions landed near prevIdx

static void updateStreaks(int pos, int s, int oldPrev) {
    if (oldPrev >= 0 && abs(pos - oldPrev) <= 2) closeStreak++;
    else closeStreak = 0;
    if (pos == s) { if (streakMax) streakLen++; else { streakMax = true; streakLen = 1; } }
    else if (pos == 0) { if (!streakMax) streakLen++; else { streakMax = false; streakLen = 1; } }
    else streakLen = 0;
}

static void insertValue(int x) {
    int s = (int)ord.size();
    int oldPrev = prevIdx;
    int pos = -1;
    if (s == 0) {
        pos = 0;
        streakMax = false; streakLen = 0; closeStreak = 0;
    } else {
        // 1) insertion at an end (ordered data, or all values of one sign)
        if (streakLen >= 2) {
            if (streakMax) {
                if (askLess(ord[s - 1], x)) pos = s;
                else if (!askLess(x, ord[s - 1])) pos = s;   // equal to the maximum
            } else {
                if (askLess(x, ord[0])) pos = 0;
                else if (!askLess(ord[0], x)) pos = 0;       // equal to the minimum
            }
        }
        // 2) equal to the element inserted last time (repeated sums)
        if (pos < 0 && closeStreak >= 1 && s >= 24 && oldPrev >= 0 && oldPrev < s) {
            int f = ord[oldPrev];
            if (!askLess(f, x) && !askLess(x, f)) pos = oldPrev;
        }
        // 3) plain binary search
        if (pos < 0) {
            int lo = 0, hi = s;
            while (lo < hi) {
                int mid = (lo + hi) >> 1;
                if (askLess(ord[mid], x)) lo = mid + 1;
                else hi = mid;
            }
            pos = lo;
        }
        updateStreaks(pos, s, oldPrev);
    }
    ord.insert(ord.begin() + pos, x);
    prevIdx = pos;
}

int main() {
    if (scanf("%d", &N) != 1) return 0;
    if (N <= 1) { printf("!\n"); fflush(stdout); return 0; }
    if (N == 2) {
        // With two values the sum is the total, which is always in range.
        doAdd(1, 2);
        printf("!\n");
        fflush(stdout);
        return 0;
    }

    ord.reserve(N);
    for (int x = 1; x <= N; x++) insertValue(x);

    while (ord.size() > 1) {
        size_t k = ord.size();
        if (k == 2) { doAdd(ord[0], ord[1]); break; }
        int m = ord.front(), M = ord.back();
        ord.erase(ord.begin());
        if (prevIdx >= 0) { if (prevIdx == 0) prevIdx = -1; else prevIdx--; }
        int bk = (int)ord.size() - 1;
        ord.pop_back();
        if (prevIdx == bk) prevIdx = -1;
        int nv = doAdd(m, M);
        if (ord.size() == 1) { doAdd(nv, ord[0]); break; }
        insertValue(nv);
    }

    printf("!\n");
    fflush(stdout);
    return 0;
}
