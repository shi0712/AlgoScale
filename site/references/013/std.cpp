#include <bits/stdc++.h>
using namespace std;
using i64 = long long;
using i128 = __int128_t;

// All scaled lengths, scan coordinates and scan accumulators are 128-bit.
string decimal(i128 x) {
    if (x == 0) return "0";
    string s;
    while (x) { s += char('0' + x % 10); x /= 10; }
    reverse(s.begin(), s.end());
    return s;
}

// Deterministic linear-time selection, in-place, [left,right), 0-based kth.
i128 selectK(vector<i128>& v, int left, int right, int kth) {
    while (true) {
        if (right - left <= 32) {
            sort(v.begin() + left, v.begin() + right);
            return v[kth];
        }
        int groups = 0;
        for (int i = left; i < right; i += 5) {
            int end = min(i + 5, right);
            sort(v.begin() + i, v.begin() + end);
            swap(v[left + groups], v[i + (end - i) / 2]);
            ++groups;
        }
        i128 pivot = selectK(v, left, left + groups, left + groups / 2);
        int lt = left, cur = left, gt = right;
        while (cur < gt) {
            if (v[cur] < pivot) swap(v[lt++], v[cur++]);
            else if (v[cur] > pivot) swap(v[cur], v[--gt]);
            else ++cur;
        }
        if (kth < lt) right = lt;
        else if (kth >= gt) left = gt;
        else return pivot;
    }
}

#ifdef LOCAL_DIAGNOSTICS
struct Diagnostics {
    i64 d = 0, initial_count = 0, final_count = 0, peak_elements = 0;
    int streaming_rounds = 0;
    bool upper_fallback = false, pivot_answer = false;
    i128 w = 0, initial_l = 0, initial_r = 0;
} diag;
#endif

i128 solve(i64 n, i64 k, i64 a, i64 b) {
#ifdef LOCAL_DIAGNOSTICS
    diag = Diagnostics{};
#endif
    i64 dl = 1, dr = min(n - 1, k);
    while (dl < dr) {
        i64 mid = dl + (dr - dl) / 2;
        if ((i128)mid * (mid + 1) / 2 >= k) dr = mid;
        else dl = mid + 1;
    }
    const i64 D = dl;
    const i128 W = (i128)D * ((i128)a * (D + 2) + 2 * (i128)b);
    auto start = [&](i64 d) -> i128 {
        return (i128)d * ((i128)a * d + 2 * (i128)b);
    };
    auto finish = [&](i64 d) -> i128 {
        return (i128)d * ((i128)a * (2 * (i128)n - d) + 2 * (i128)b);
    };
    auto weight = [&](i64 d) -> i128 { return W / (2 * (i128)a * d); };

    const i128 targetL = (i128)(k - D) * W;
    const i128 targetR = (i128)(k + D) * W;
    i128 L = 0, R = W, x = 0, value = 0, slope = 0;
    bool foundL = (targetL == 0), foundR = false;
    i64 enter = 1, leave = 1;
    while (x < W) {
        i128 ns = enter <= D ? start(enter) : W + 1;
        i128 ne = leave <= D ? finish(leave) : W + 1;
        i128 nx = min(W, min(ns, ne)), dx = nx - x;
        if (slope > 0) {
            // For positive gap, ceil(gap/slope) <= dx iff (gap-1)/slope < dx.
            // Test BEFORE multiplying. If the upper root is not reached,
            // value + slope*dx < targetR < 2^127, so the update cannot overflow.
            if (!foundL && (targetL - value - 1) / slope < dx) {
                L = x + (targetL - value) / slope;
                foundL = true;
            }
            if ((targetR - value - 1) / slope < dx) {
                R = x + (targetR - value) / slope;
                foundR = true;
                break;
            }
        }
        value += slope * dx;
        x = nx;
        if (x == W) break;
        if (ns == x) { slope += weight(enter); ++enter; }
        if (ne == x) { slope -= weight(leave); ++leave; }
    }
    assert(foundL);
    (void)foundR;

    // Count one row, avoiding negative division and saturating before narrowing.
    auto rowCount = [&](i128 bound, i128 base, i64 step, i64 len) -> i64 {
        if (bound <= base) return 0;
        return (i64)min((i128)len, (bound - base) / step);
    };
    auto rankAndEqual = [&](i128 bound) -> pair<i64, i64> {
        i64 rank = 0, equal = 0;
        for (i64 d = 1; d <= D; ++d) {
            i128 base = start(d);
            if (base >= bound) break;
            i64 step = 2 * a * d;
            i64 c = rowCount(bound, base, step, n - d);
            rank += c;
            if (c > 0 && base + (i128)step * c == bound) ++equal;
        }
        return {rank, equal};
    };
    // Enumerate the CURRENT interval only; no row arrays are stored.
    auto enumerate = [&](auto&& emit) {
        for (i64 d = 1; d <= D; ++d) {
            i128 base = start(d);
            if (base >= R) break;
            i64 step = 2 * a * d;
            i64 first = rowCount(L, base, step, n - d) + 1;
            i64 last = rowCount(R, base, step, n - d);
            i128 v = base + (i128)step * first;
            for (i64 i = first; i <= last; ++i, v += step) emit(v);
        }
    };
    i64 rankL = rankAndEqual(L).first;
    i64 rankR = rankAndEqual(R).first;
    assert(rankL < k && k <= rankR && rankR - rankL < 4 * D);
#ifdef LOCAL_DIAGNOSTICS
    diag.d = D; diag.w = W; diag.initial_l = L; diag.initial_r = R;
    diag.initial_count = rankR - rankL; diag.upper_fallback = !foundR;
#endif
    const i64 materializeLimit = max<i64>(D, 256);

    // Retain the earlier O(sqrt(k)) scan + BFPRT algorithm, but stream a few
    // BFPRT pivot rounds before materialization. At most one vector is alive.
    // A group-of-five pivot leaves <= 7m/10+6 elements: <= 5 rounds suffice
    // from m<4D until m<=max(D,256). This keeps worst-case memory below 256 MiB.
    while (rankR - rankL > materializeLimit) {
        const i64 count = rankR - rankL;
        i128 pivot;
        {
            vector<i128> medians;
            medians.reserve((size_t)((count + 4) / 5));
            array<i128, 5> group{};
            int used = 0;
            enumerate([&](i128 v) {
                group[used++] = v;
                if (used == 5) {
                    sort(group.begin(), group.end());
                    medians.push_back(group[2]);
                    used = 0;
                }
            });
            if (used) {
                sort(group.begin(), group.begin() + used);
                medians.push_back(group[used / 2]);
            }
            assert((i64)medians.size() == (count + 4) / 5);
#ifdef LOCAL_DIAGNOSTICS
            ++diag.streaming_rounds;
            diag.peak_elements = max(diag.peak_elements, (i64)medians.capacity());
#endif
            pivot = selectK(medians, 0, (int)medians.size(), (int)medians.size() / 2);
        } // Release ALL medians before allocating any final candidate vector.
        auto [le, eq] = rankAndEqual(pivot);
        if (k <= le - eq) { R = pivot - 1; rankR = le - eq; }
        else if (k <= le) {
#ifdef LOCAL_DIAGNOSTICS
            diag.pivot_answer = true;
#endif
            return pivot;
        } else { L = pivot; rankL = le; }
        assert(rankL < k && k <= rankR);
        assert(rankR - rankL <= count * 7 / 10 + 6);
    }
    vector<i128> candidates;
    candidates.reserve((size_t)(rankR - rankL));
    enumerate([&](i128 v) { candidates.push_back(v); });
    assert((i64)candidates.size() == rankR - rankL);
#ifdef LOCAL_DIAGNOSTICS
    diag.final_count = (i64)candidates.size();
    diag.peak_elements = max(diag.peak_elements, (i64)candidates.capacity());
    assert(diag.streaming_rounds <= 5);
#endif
    return selectK(candidates, 0, (int)candidates.size(), (int)(k - rankL - 1));
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    i64 n, k, a, b;
    if (!(cin >> n >> k >> a >> b)) return 0;
    i128 answer = solve(n, k, a, b);
    i64 g = gcd((i64)(answer % a), a);
    cout << decimal(answer / g) << ' ' << a / g << '\n';
#ifdef LOCAL_DIAGNOSTICS
    cerr << "{\"D\":" << diag.d << ",\"W\":\"" << decimal(diag.w)
         << "\",\"L\":\"" << decimal(diag.initial_l)
         << "\",\"R\":\"" << decimal(diag.initial_r)
         << "\",\"initial_candidates\":" << diag.initial_count
         << ",\"final_candidates\":" << diag.final_count
         << ",\"streaming_rounds\":" << diag.streaming_rounds
         << ",\"peak_vector_bytes\":" << diag.peak_elements * 16
         << ",\"upper_fallback\":" << diag.upper_fallback
         << ",\"pivot_answer\":" << diag.pivot_answer << "}\n";
#endif
    return 0;
}
