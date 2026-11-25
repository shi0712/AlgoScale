#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    
    if (n <= 4) {
        string s(n, 'T');
        cout << s << endl;
        int r;
        cin >> r;
        if (r == -1) return 0;
        if (r == n) return 0;
        int k = r;
        vector<int> pref(n + 1, 0);
        for (int i = 1; i <= n - 1; ++i) {
            string q(n, 'T');
            for (int j = 0; j < i; ++j) q[j] = 'F';
            cout << q << endl;
            cin >> r;
            if (r == -1) return 0;
            if (r == n) return 0;
            pref[i] = (i + k - r) / 2;
        }
        vector<int> sec(n);
        for (int i = 1; i <= n - 1; ++i) {
            sec[i - 1] = pref[i] - pref[i - 1];
        }
        sec[n - 1] = k - pref[n - 1];
        string ans(n, 'F');
        for (int i = 0; i < n; ++i) if (sec[i] == 1) ans[i] = 'T';
        cout << ans << endl;
        cin >> r;
        return 0;
    }
    
    string s(n, 'F');
    cout << s << endl;
    int r;
    cin >> r;
    if (r == -1) return 0;
    if (r == n) return 0;
    int k = n - r;
    int m = n - 2;
    vector<int> y(m);
    for (int j = 0; j < m; ++j) {
        fill(s.begin(), s.end(), 'F');
        s[j + 1] = 'T';
        s[n - 1] = 'T';
        cout << s << endl;
        cin >> r;
        if (r == -1) return 0;
        if (r == n) return 0;
        y[j] = (r - n + 2 + k) / 2;
    }
    int sumY = accumulate(y.begin(), y.end(), 0);
    vector<int> sec(n);
    for (int cand = 0; cand <= 1; ++cand) {
        int s0 = k - sumY + (m - 1) * cand;
        if (s0 == 0 || s0 == 1) {
            bool ok = true;
            for (int j = 0; j < m; ++j) {
                int val = y[j] - cand;
                if (val < 0 || val > 1) { ok = false; break; }
            }
            if (ok) {
                sec[0] = s0;
                for (int j = 0; j < m; ++j) sec[j + 1] = y[j] - cand;
                sec[n - 1] = cand;
                break;
            }
        }
    }
    string ans(n, 'F');
    for (int i = 0; i < n; ++i) if (sec[i] == 1) ans[i] = 'T';
    cout << ans << endl;
    cin >> r;
    return 0;
}
