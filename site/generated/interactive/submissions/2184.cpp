#include <bits/stdc++.h>
using namespace std;

struct Fenwick {
    int n = 0;
    vector<int> bit;

    Fenwick() {}
    Fenwick(int n_) { init(n_); }

    void init(int n_) {
        n = n_;
        bit.assign(n + 1, 0);
    }

    void build_all_ones() {
        for (int i = 1; i <= n; ++i) bit[i] = i & -i;
    }

    void add(int idx, int val) {
        for (; idx <= n; idx += idx & -idx) bit[idx] += val;
    }

    int sum(int idx) const {
        int res = 0;
        for (; idx > 0; idx -= idx & -idx) res += bit[idx];
        return res;
    }

    int range_sum(int l, int r) const {
        if (l > r) return 0;
        return sum(r) - sum(l - 1);
    }

    int kth(int k) const {
        int idx = 0;
        int mask = 1;
        while ((mask << 1) <= n) mask <<= 1;
        for (int d = mask; d; d >>= 1) {
            int nxt = idx + d;
            if (nxt <= n && bit[nxt] < k) {
                idx = nxt;
                k -= bit[nxt];
            }
        }
        return idx + 1;
    }
};

int n, t;
int B, m;

Fenwick guessBIT;
Fenwick blockBIT;
vector<int> block_zero;
int known = 0;
int known_sum = 0;

unordered_map<long long, pair<int, int>> cache;
vector<int> original_ones;

long long key_range(int l, int r) {
    return (static_cast<long long>(l) << 20) | static_cast<long long>(r);
}

int get_zeros(int l, int r) {
    if (l > r) return 0;

    auto it = cache.find(key_range(l, r));
    if (it != cache.end()) {
        int g_now = guessBIT.range_sum(l, r);
        int cur_sum = it->second.first + (g_now - it->second.second);
        return (r - l + 1) - cur_sum;
    }

    cout << "? " << l << ' ' << r << endl;
    int s;
    if (!(cin >> s)) exit(0);
    if (s < 0) exit(0);

    int g_at = guessBIT.range_sum(l, r);
    cache[key_range(l, r)] = {s, g_at};
    return (r - l + 1) - s;
}

int kth_unguessed(int L, int R, int k) {
    int lo = L, hi = R;
    while (lo < hi) {
        int mid = (lo + hi) >> 1;
        int len = mid - lo + 1;
        int g = guessBIT.range_sum(lo, mid);
        int un = len - g;
        if (un >= k) {
            hi = mid;
        } else {
            k -= un;
            lo = mid + 1;
        }
    }
    return lo;
}

int find_in_block(int L, int R, int k, int b) {
    int lo = L, hi = R;
    int before = 0;

    bool total_known = (b < known);
    int total = total_known ? block_zero[b] : -1;

    while (lo < hi) {
        int len = hi - lo + 1;
        int g = guessBIT.range_sum(lo, hi);
        int un = len - g;

        if (k == un) {
            return kth_unguessed(lo, hi, k);
        }

        if (total_known) {
            if (total == un) {
                return kth_unguessed(lo, hi, k);
            }

            if (total == un - 1 && un >= 2) {
                if (k == 1) {
                    int p = kth_unguessed(lo, hi, 1);
                    int z = get_zeros(p, p);
                    if (z == 1) return p;
                    return kth_unguessed(lo, hi, 2);
                } else if (k == total) {
                    int p = kth_unguessed(lo, hi, un);
                    int z = get_zeros(p, p);
                    if (z == 1) return p;
                    return kth_unguessed(lo, hi, un - 1);
                }
            }
        }

        int mid = (lo + hi) >> 1;
        int z_prefix = get_zeros(L, mid);
        int z_left = z_prefix - before;

        if (z_left >= k) {
            hi = mid;
            total_known = true;
            total = z_left;
        } else {
            k -= z_left;
            lo = mid + 1;
            before = z_prefix;
            if (total_known) total -= z_left;
        }
    }

    return lo;
}

long long estimate_find_all_ones(int n_, int c) {
    if (c <= 0) return 0;

    int h = 0;
    while ((1 << h) < n_) ++h;

    long long res = 1; // root query
    for (int d = 0; d < h; ++d) {
        int nodes = 1 << d;
        res += min(nodes, c);
    }
    return res;
}

void find_ones(int l, int r, int cnt) {
    if (cnt <= 0) return;

    if (cnt == r - l + 1) {
        for (int i = l; i <= r; ++i) original_ones.push_back(i);
        return;
    }

    if (l == r) {
        original_ones.push_back(l);
        return;
    }

    int mid = (l + r) >> 1;
    int left_len = mid - l + 1;
    int left_zeros = get_zeros(l, mid);
    int left_ones = left_len - left_zeros;

    find_ones(l, mid, left_ones);
    find_ones(mid + 1, r, cnt - left_ones);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> t)) return 0;

    long long bestCost = LLONG_MAX;
    int bestB = 1;

    for (int b = 1; b <= n; ++b) {
        int mm = (n + b - 1) / b;
        int depth = 0;
        if (b > 1) depth = 32 - __builtin_clz(static_cast<unsigned>(b - 1));

        long long top = (mm > 1 ? mm - 1 : 0);
        long long cost = top + 1LL * t * depth;

        if (cost < bestCost || (cost == bestCost && b < bestB)) {
            bestCost = cost;
            bestB = b;
        }
    }

    int ones_bound = n - t;
    long long findCost = estimate_find_all_ones(n, ones_bound);
    bool use_find_all_ones = (findCost < bestCost);

    guessBIT.init(n);
    cache.reserve(300000);
    cache.max_load_factor(0.7);

    int first_k;
    if (!(cin >> first_k)) return 0;
    if (first_k < 0) return 0;

    if (use_find_all_ones) {
        original_ones.clear();
        original_ones.reserve(max(0, ones_bound));

        if (ones_bound > 0) {
            int total_zeros = get_zeros(1, n);
            int ones = n - total_zeros;
            if (ones > 0) find_ones(1, n, ones);
        }

        Fenwick zeroBIT;
        zeroBIT.init(n);
        zeroBIT.build_all_ones();
        for (int p : original_ones) zeroBIT.add(p, -1);

        int k = first_k;
        for (int round = 0; round < t; ++round) {
            if (round > 0) {
                if (!(cin >> k)) return 0;
                if (k < 0) return 0;
            }

            int x = zeroBIT.kth(k);
            cout << "! " << x << endl;
            zeroBIT.add(x, -1);
        }

        return 0;
    }

    B = bestB;
    m = (n + B - 1) / B;

    blockBIT.init(m);
    block_zero.assign(m, -1);
    known = 0;
    known_sum = 0;

    auto suffix_shortcut = [&](int k) -> int {
        if (known_sum >= k) return -1;

        int S = known * B + 1;
        int U = n - S + 1;
        if (U <= 0) return -1;

        int g = guessBIT.range_sum(S, n);
        int max_zeros = U - g;
        int need = k - known_sum;

        if (need == max_zeros) {
            return kth_unguessed(S, n, need);
        }

        return -1;
    };

    int k = first_k;
    for (int round = 0; round < t; ++round) {
        if (round > 0) {
            if (!(cin >> k)) return 0;
            if (k < 0) return 0;
        }

        int x = -1;

        if (known_sum >= k) {
            int pos = blockBIT.kth(k);
            int b = pos - 1;
            int before = blockBIT.sum(pos - 1);
            int k_in = k - before;

            int L = b * B + 1;
            int R = min(n, (b + 1) * B);

            x = find_in_block(L, R, k_in, b);
        } else {
            x = suffix_shortcut(k);

            if (x == -1) {
                while (known < m - 1 && known_sum < k) {
                    int L = known * B + 1;
                    int R = min(n, (known + 1) * B);

                    int z = get_zeros(L, R);
                    block_zero[known] = z;
                    blockBIT.add(known + 1, z);
                    known_sum += z;
                    ++known;

                    x = suffix_shortcut(k);
                    if (x != -1) break;
                }

                if (x == -1) {
                    if (known_sum >= k) {
                        int pos = blockBIT.kth(k);
                        int b = pos - 1;
                        int before = blockBIT.sum(pos - 1);
                        int k_in = k - before;

                        int L = b * B + 1;
                        int R = min(n, (b + 1) * B);

                        x = find_in_block(L, R, k_in, b);
                    } else {
                        int b = m - 1;
                        int L = b * B + 1;
                        int R = n;
                        int k_in = k - known_sum;

                        int g = guessBIT.range_sum(L, R);
                        int un = (R - L + 1) - g;

                        if (un > 0 && k_in == un) {
                            x = kth_unguessed(L, R, k_in);
                        } else {
                            x = find_in_block(L, R, k_in, b);
                        }
                    }
                }
            }
        }

        cout << "! " << x << endl;

        guessBIT.add(x, 1);

        int xb = (x - 1) / B;
        if (xb < known) {
            --block_zero[xb];
            blockBIT.add(xb + 1, -1);
            --known_sum;
        }
    }

    return 0;
}
