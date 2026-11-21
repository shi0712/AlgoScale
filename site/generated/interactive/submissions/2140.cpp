#include <bits/stdc++.h>
using namespace std;

int query(int l, int r, int k) {
    cout << "? " << l << " " << r << " " << k << endl;
    cout.flush();
    int ans;
    if (!(cin >> ans)) exit(0);
    if (ans == -1) exit(0);
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    if (!(cin >> n)) return 0;
    
    // Find position of n using binary search with k = n
    int low = 1, high = n;
    while (low < high) {
        int mid = (low + high) / 2;
        int d = query(low, mid, n);
        if (d == 2) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    int pos_n = low;
    
    // Distinct count of whole array for k = 2
    int D_total = query(1, n, 2);
    bool is_n_unique = (n % 2 == 0); // n's value is unique for even n
    
    // Binary search for position of 1 using prefix queries
    low = 1;
    high = n;
    while (low < high) {
        int mid = (low + high) / 2;
        int d_left = query(1, mid, 2);
        int d_right = query(mid + 1, n, 2);
        int size_left = mid;
        int uniq_left = 2 * d_left - size_left;
        int intersection = d_left + d_right - D_total;
        int left_has_one = uniq_left - intersection - (is_n_unique && pos_n <= mid ? 1 : 0);
        if (left_has_one > 0) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    int pos_one = low;
    
    cout << "! " << pos_one << endl;
    cout.flush();
    return 0;
}
