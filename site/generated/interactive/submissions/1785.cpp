#include <bits/stdc++.h>
using namespace std;

int ask_less(int i, int j) {
    cout << "? " << i << ' ' << j << '\n';
    cout.flush();
    int r;
    cin >> r;
    if (r == -1) exit(0);
    return r;
}

void merge_sort_rec(vector<int>& v, vector<int>& tmp, int l, int r) {
    if (r - l <= 1) return;
    int m = (l + r) / 2;
    merge_sort_rec(v, tmp, l, m);
    merge_sort_rec(v, tmp, m, r);

    int i = l, j = m, k = l;
    while (i < m && j < r) {
        if (ask_less(v[i], v[j])) {
            tmp[k++] = v[i++];
        } else {
            tmp[k++] = v[j++];
        }
    }
    while (i < m) tmp[k++] = v[i++];
    while (j < r) tmp[k++] = v[j++];

    for (int p = l; p < r; ++p) v[p] = tmp[p];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    if (N == 1) {
        cout << "!\n";
        cout.flush();
        return 0;
    }

    if (N == 2) {
        cout << "+ 1 2\n";
        cout.flush();
        int v;
        cin >> v;
        if (v == -1) return 0;

        cout << "!\n";
        cout.flush();
        return 0;
    }

    vector<int> ord(N);
    iota(ord.begin(), ord.end(), 1);

    vector<int> tmp(N);
    merge_sort_rec(ord, tmp, 0, N);

    while (true) {
        int sz = (int)ord.size();

        if (sz == 1) {
            cout << "!\n";
            cout.flush();
            return 0;
        }

        if (sz == 2) {
            cout << "+ " << ord[0] << ' ' << ord[1] << '\n';
            cout.flush();

            int v;
            cin >> v;
            if (v == -1) return 0;

            cout << "!\n";
            cout.flush();
            return 0;
        }

        if (sz == 3) {
            int L = ord[0];
            int M = ord[1];
            int R = ord[2];

            cout << "+ " << L << ' ' << R << '\n';
            cout.flush();

            int v;
            cin >> v;
            if (v == -1) return 0;

            cout << "+ " << M << ' ' << v << '\n';
            cout.flush();

            int w;
            cin >> w;
            if (w == -1) return 0;

            cout << "!\n";
            cout.flush();
            return 0;
        }

        int L = ord.front();
        int R = ord.back();

        cout << "+ " << L << ' ' << R << '\n';
        cout.flush();

        int v;
        cin >> v;
        if (v == -1) return 0;

        ord.erase(ord.begin());
        ord.pop_back();

        int lo = 0, hi = (int)ord.size();
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (ask_less(ord[mid], v)) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }

        ord.insert(ord.begin() + lo, v);
    }

    return 0;
}
