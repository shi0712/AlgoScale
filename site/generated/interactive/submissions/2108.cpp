#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int64 n;
    if (!(cin >> n)) return 0;          // no input
    const int64 half = (n + 1) / 2;     // (n+1)/2, used only for debugging

    unordered_map<int64,int64> cache;   // position -> f(position)

    auto norm = [&](int64 x)->int64{
        x %= n;
        if (x <= 0) x += n;
        return x;
    };

    auto ask = [&](int64 x)->int64{
        x = norm(x);
        auto it = cache.find(x);
        if (it != cache.end()) return it->second;
        cout << "? " << x << "\n" << flush;
        int64 v;
        if (!(cin >> v)) exit(0);
        if (v == -1) exit(0);
        cache[x] = v;
        return v;
    };

    // ---------- 1. binary search for a vertex of the minimum segment ----------
    int64 L = 1, R = n;
    int64 best = ask(1);                // current known minimum value

    while (true) {
        int64 len = (R - L + n) % n + 1;
        if (len <= 3) break;

        int64 midOffset = len / 2;
        int64 mid = norm(L + midOffset - 1);
        int64 midNext = norm(mid + 1);

        int64 fmid = ask(mid);
        int64 fmidNext = ask(midNext);
        best = min(best, min(fmid, fmidNext));

        if (fmid < fmidNext) {
            // minimum lies on the left side (including mid)
            R = mid;
        } else if (fmid > fmidNext) {
            // minimum lies on the right side (including midNext)
            L = midNext;
        } else { // equal
            if (fmid == best) {
                // we are already inside the minimum segment
                L = mid;
                R = midNext;
                break;
            } else {
                // flat top, discard it
                L = midNext;
            }
        }
    }

    // query the remaining at most three vertices
    vector<int64> seg;
    int64 cur = L;
    while (true) {
        seg.push_back(cur);
        if (cur == R) break;
        cur = norm(cur + 1);
    }

    int64 D = (int64)4e18;
    int64 x0 = -1;
    for (int64 v : seg) {
        int64 val = ask(v);
        if (val < D) {
            D = val;
            x0 = v;
        }
    }

    // ---------- 2. find left end (A) ----------
    int64 step = 1;
    while (true) {
        int64 pos = norm(x0 - step);
        int64 val = ask(pos);
        if (val == D) {
            step <<= 1;
            if (step > n) { step = n; break; }
        } else break;
    }
    int64 low = step >> 1;
    int64 high = step;
    while (low < high) {
        int64 mid = (low + high) / 2;
        int64 pos = norm(x0 - mid);
        int64 val = ask(pos);
        if (val == D) low = mid + 1;
        else high = mid;
    }
    int64 A = norm(x0 - (low - 1));

    // ---------- 3. find right end (B) ----------
    step = 1;
    while (true) {
        int64 pos = norm(x0 + step);
        int64 val = ask(pos);
        if (val == D) {
            step <<= 1;
            if (step > n) { step = n; break; }
        } else break;
    }
    low = step >> 1;
    high = step;
    while (low < high) {
        int64 mid = (low + high) / 2;
        int64 pos = norm(x0 + mid);
        int64 val = ask(pos);
        if (val == D) low = mid + 1;
        else high = mid;
    }
    int64 B = norm(x0 + (low - 1));

    // ---------- 4. answer ----------
    cout << "! " << A << " " << B << "\n" << flush;
    return 0;
}
