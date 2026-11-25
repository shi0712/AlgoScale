#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> known;

int ask(int l, int r, int k) {
    cout << "? " << l << " " << r << " " << k << endl;
    int res;
    cin >> res;
    if (res == -1) exit(0);
    return res;
}

pair<int,int> find_max(int l, int r, int upper) {
    int M = -1;
    for (int k = upper; k >= 2; --k) {
        int res = ask(l, r, k);
        if (res > 1) {
            M = k;
            break;
        }
    }
    int L = l, R = r;
    while (R - L + 1 > 2) {
        int mid = (L + R) / 2;
        if (mid == L) mid = L + 1;
        int res = ask(L, mid, M);
        if (res > 1) R = mid;
        else L = mid + 1;
    }
    if (L == R) return {M, L};
    int a = L, b = L + 1;
    if (a > l) {
        int res = ask(l, a, M);
        if (res > 1) return {M, a};
        else return {M, b};
    } else if (b < r) {
        int res = ask(b, r, M);
        if (res > 1) return {M, b};
        else return {M, a};
    } else {
        if (a > 1 && known[a-1] != 0) {
            int A = known[a-1];
            for (int k = 2; k <= n; ++k) {
                int cA = A / k;
                int cM = M / k;
                int Q1 = (cA == cM) ? 1 : 2;
                int Q2 = (cA == 0) ? 1 : 2;
                if (Q1 != Q2) {
                    int res = ask(a-1, a, k);
                    if (res == Q1) return {M, a};
                    else return {M, b};
                }
            }
        }
        if (b < n && known[b+1] != 0) {
            int A = known[b+1];
            for (int k = 2; k <= n; ++k) {
                int cA = A / k;
                int cM = M / k;
                int Q1 = (cA == cM) ? 1 : 2;
                int Q2 = (cA == 0) ? 1 : 2;
                if (Q1 != Q2) {
                    int res = ask(b, b+1, k);
                    if (res == Q1) return {M, b};
                    else return {M, a};
                }
            }
        }
        return {M, a};
    }
}

int determine_value(int q, int M, int pos) {
    vector<int> possible;
    for (int x = 1; x < M; ++x) possible.push_back(x);
    while (possible.size() > 1) {
        int best_k = -1;
        int best_score = 1e9;
        vector<int> best_left, best_right;
        for (int k = 2; k <= n; ++k) {
            vector<int> left, right;
            int cM = M / k;
            for (int x : possible) {
                int cx = x / k;
                if (cx == cM) right.push_back(x);
                else left.push_back(x);
            }
            int score = max(left.size(), right.size());
            if (score < best_score) {
                best_score = score;
                best_k = k;
                best_left = left;
                best_right = right;
            }
        }
        int l = min(q, pos);
        int r = max(q, pos);
        int res = ask(l, r, best_k);
        if (res == 1) possible = best_right;
        else possible = best_left;
    }
    return possible[0];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    cin >> n;
    known.assign(n + 1, 0);
    priority_queue<tuple<int,int,int,int>> pq;
    auto [M, pos] = find_max(1, n, n);
    pq.push({M, 1, n, pos});
    while (!pq.empty()) {
        auto [max_val, l, r, p] = pq.top();
        pq.pop();
        known[p] = max_val;
        if (l <= p - 1) {
            int l2 = l, r2 = p - 1;
            if (l2 == r2) {
                int q = l2;
                int v = determine_value(q, max_val, p);
                known[q] = v;
                if (v == 1) {
                    cout << "! " << q << endl;
                    return 0;
                }
            } else {
                auto [subM, subPos] = find_max(l2, r2, max_val - 1);
                pq.push({subM, l2, r2, subPos});
            }
        }
        if (p + 1 <= r) {
            int l2 = p + 1, r2 = r;
            if (l2 == r2) {
                int q = l2;
                int v = determine_value(q, max_val, p);
                known[q] = v;
                if (v == 1) {
                    cout << "! " << q << endl;
                    return 0;
                }
            } else {
                auto [subM, subPos] = find_max(l2, r2, max_val - 1);
                pq.push({subM, l2, r2, subPos});
            }
        }
    }
    return 0;
}
