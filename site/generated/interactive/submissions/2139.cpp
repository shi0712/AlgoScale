#include <bits/stdc++.h>
using namespace std;

int query_compare(int i, int j) {
    cout << "? " << i << " " << j << "\n" << flush;
    int ans;
    if (!(cin >> ans)) exit(0);
    if (ans == -1) exit(0);
    return ans;
}

bool less_than(int i, int j) {
    return query_compare(i, j) == 1;
}

int query_add(int i, int j) {
    cout << "+ " << i << " " << j << "\n" << flush;
    int new_label;
    if (!(cin >> new_label)) exit(0);
    if (new_label == -1) exit(0);
    return new_label;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int N;
    if (!(cin >> N)) return 0;
    if (N <= 0) return 0;
    vector<int> labels(N);
    for (int i = 0; i < N; ++i) labels[i] = i + 1;

    // Sort labels by their hidden values using interactive comparisons
    sort(labels.begin(), labels.end(),
         [&](int a, int b) { return less_than(a, b); });

    while (labels.size() > 1) {
        int i = labels.front();
        int j = labels.back();
        int new_label = query_add(i, j);
        // Remove the two used labels
        labels.erase(labels.begin());
        labels.pop_back();
        // Insert the new label into the sorted list
        int lo = 0, hi = (int)labels.size();
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (less_than(new_label, labels[mid]))
                hi = mid;
            else
                lo = mid + 1;
        }
        labels.insert(labels.begin() + lo, new_label);
    }

    cout << "!\n" << flush;
    return 0;
}
