#include <bits/stdc++.h>
using namespace std;

using ll = long long;

ll ask(int l, int r) {
    cout << "? " << l << " " << r << '\n';
    cout.flush();
    ll x;
    cin >> x;
    if (x == -1) exit(0);
    return x;
}

ll largestPowerLE(ll x) {
    if (x <= 0) return 0;
    ll p = 1;
    while ((p << 1) <= x) p <<= 1;
    return p;
}

ll getUpper(ll sum, int len) {
    if (len == 1) return sum;
    ll b1 = sum / 2;
    ll b2 = sum - (ll)len + 1;
    ll b = min(b1, b2);
    return largestPowerLE(b);
}

struct Node {
    int l, r;
    ll sum;
    int len;
    ll ub;

    bool operator<(const Node& other) const {
        return ub < other.ub; // max-heap
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    ll total = ask(1, n);

    ll hp = 1;
    while ((hp << 1) <= n) hp <<= 1;

    // Guaranteed lower bound for the maximum element.
    ll best = total / hp;

    ll rootUb = getUpper(total, n);
    if (n == 1 || rootUb <= best) {
        cout << "! " << best << '\n';
        cout.flush();
        return 0;
    }

    priority_queue<Node> pq;
    pq.push({1, n, total, n, rootUb});

    while (!pq.empty()) {
        Node cur = pq.top();
        pq.pop();

        if (cur.ub <= best) break;

        if (cur.len == 1) {
            best = max(best, cur.sum);
            continue;
        }

        ll half = cur.sum / 2;

        // Find the split point p such that sum(l..p) = half.
        int lo = cur.l, hi = cur.r - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            ll s = ask(cur.l, mid);
            if (s >= half) hi = mid;
            else lo = mid + 1;
        }
        int p = lo;

        int leftLen = p - cur.l + 1;
        int rightLen = cur.r - p;

        // If one side is a single element, its value is half,
        // and it is the maximum of this whole node.
        if (leftLen == 1 || rightLen == 1) {
            best = max(best, half);
            continue;
        }

        ll leftUb = getUpper(half, leftLen);
        if (leftUb > best) {
            pq.push({cur.l, p, half, leftLen, leftUb});
        }

        ll rightUb = getUpper(half, rightLen);
        if (rightUb > best) {
            pq.push({p + 1, cur.r, half, rightLen, rightUb});
        }
    }

    cout << "! " << best << '\n';
    cout.flush();
    return 0;
}
