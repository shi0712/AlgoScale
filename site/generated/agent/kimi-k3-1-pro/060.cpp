#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using i128 = __int128_t;
using u128 = __uint128_t;

static u64 ceil_div(u64 a, u64 b) {
    return a / b + (a % b != 0);
}

struct Feasibility {
    u64 h, w, s, k;

    bool search(u64 left, u64 right) const {
        u128 lower = u128(ceil_div(h, right)) * ceil_div(w, k - left);
        if (lower > s) return false;
        if (left == right) return true;

        u64 middle = left + (right - left) / 2;
        if (u128(ceil_div(h, middle)) * ceil_div(w, k - middle) <= s)
            return true;
        return search(left, middle) || search(middle + 1, right);
    }

    bool check() const {
        i128 required = (i128(h) * w + s - 1) / s;
        i128 discriminant = i128(k) * k - 4 * required;
        if (discriminant < 0) return false;

        u64 root = sqrt((long double)discriminant);
        while (i128(root + 1) * (root + 1) <= discriminant) ++root;
        while (i128(root) * root > discriminant) --root;

        u64 left = k > root ? (k - root) / 2 : 0;
        left = left > 2 ? left - 2 : 1;
        u64 right = (k + root) / 2 + 2;
        left = max(left, 1ULL);
        if (k > w) left = max(left, k - w);
        right = min(right, k - 1);
        right = min(right, h);
        if (left > right) return false;
        return search(left, right);
    }
};

static bool feasible(u64 h, u64 w, u64 s, u64 k) {
    return Feasibility{h, w, s, k}.check();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;
    while (t--) {
        u64 h, w, s;
        cin >> h >> w >> s;
        if (u128(h) * w <= s) {
            cout << 0 << '\n';
            continue;
        }

        u64 low = 2, high = h + w;
        while (low < high) {
            u64 middle = low + (high - low) / 2;
            if (feasible(h, w, s, middle)) high = middle;
            else low = middle + 1;
        }
        cout << low - 2 << '\n';
    }
    return 0;
}
