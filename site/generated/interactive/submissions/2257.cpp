#include <bits/stdc++.h>
using namespace std;

bool lessThan(int i, int j) {
    cout << "? " << i << ' ' << j << '\n' << flush;
    int response;
    if (!(cin >> response) || response < 0) exit(0);
    return response == 1;
}

int addValues(int i, int j) {
    cout << "+ " << i << ' ' << j << '\n' << flush;
    int label;
    if (!(cin >> label) || label <= 0) exit(0);
    return label;
}

void mergeSort(vector<int>& a, vector<int>& tmp, int left, int right) {
    if (right - left <= 1) return;
    int mid = (left + right) / 2;
    mergeSort(a, tmp, left, mid);
    mergeSort(a, tmp, mid, right);

    int i = left, j = mid, k = left;
    while (i < mid && j < right) {
        if (lessThan(a[j], a[i])) tmp[k++] = a[j++];
        else tmp[k++] = a[i++];
    }
    while (i < mid) tmp[k++] = a[i++];
    while (j < right) tmp[k++] = a[j++];
    for (int p = left; p < right; ++p) a[p] = tmp[p];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    if (N == 2) {
        addValues(1, 2);
        cout << "!\n" << flush;
        return 0;
    }

    vector<int> values(N), tmp(N);
    iota(values.begin(), values.end(), 1);
    mergeSort(values, tmp, 0, N);

    while (values.size() > 3) {
        int newLabel = addValues(values.front(), values.back());
        values.erase(values.begin());
        values.pop_back();

        int lo = 0, hi = static_cast<int>(values.size());
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (lessThan(values[mid], newLabel)) lo = mid + 1;
            else hi = mid;
        }
        values.insert(values.begin() + lo, newLabel);
    }

    int combined = addValues(values[0], values[2]);
    addValues(values[1], combined);
    cout << "!\n" << flush;
    return 0;
}
