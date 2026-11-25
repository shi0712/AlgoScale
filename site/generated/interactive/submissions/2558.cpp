#include <bits/stdc++.h>
using namespace std;

const int MAXL = 2005;
int cache[MAXL][MAXL];

bool less_val(int i, int j) {
    if (i == j) return false;
    int &c = cache[i][j];
    if (c != -1) return c;
    cout << "? " << i << " " << j << endl;
    int res;
    cin >> res;
    if (res == -1) exit(0);
    c = res;
    if (res == 1) cache[j][i] = 0;
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    memset(cache, -1, sizeof(cache));

    int N;
    cin >> N;

    vector<int> v(N);
    iota(v.begin(), v.end(), 1);
    sort(v.begin(), v.end(), less_val);

    while (v.size() > 1) {
        int i = v.front();
        int j = v.back();

        cout << "+ " << i << " " << j << endl;
        int x;
        cin >> x;
        if (x == -1) return 0;

        v.erase(v.begin());
        v.pop_back();

        if (v.empty()) {
            v.push_back(x);
        } else if (less_val(x, v.front())) {
            v.insert(v.begin(), x);
        } else if (less_val(v.back(), x)) {
            v.push_back(x);
        } else {
            int lo = 1, hi = (int)v.size() - 2;
            int pos = (int)v.size() - 1;
            while (lo <= hi) {
                int mid = (lo + hi) / 2;
                if (less_val(x, v[mid])) {
                    pos = mid;
                    hi = mid - 1;
                } else {
                    lo = mid + 1;
                }
            }
            v.insert(v.begin() + pos, x);
        }
    }

    cout << "!" << endl;
    return 0;
}
