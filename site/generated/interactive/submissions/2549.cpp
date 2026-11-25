#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;
    if (N == 1) {
        cout << "! 1\n1\n";
        cout.flush();
        return 0;
    }

    deque<int> q;
    int total_distinct = 0;
    for (int i = 1; i <= N; ++i) {
        cout << "+ " << i << '\n';
        cout.flush();
        int d;
        if (!(cin >> d)) return 0;
        total_distinct = d;
        q.push_back(i);
    }

    if (total_distinct == N) {
        cout << "! " << N << '\n';
        for (int i = 1; i <= N; ++i) {
            cout << "1 " << i << '\n';
        }
        cout.flush();
        return 0;
    }
    if (total_distinct == 1) {
        cout << "! 1\n";
        cout << N;
        for (int i = 1; i <= N; ++i) cout << ' ' << i;
        cout << '\n';
        cout.flush();
        return 0;
    }

    for (int i = 0; i < N; ++i) {
        cout << "-\n";
        cout.flush();
        int d;
        if (!(cin >> d)) return 0;
    }
    q.clear();

    vector<int> cls(N + 1, 0);
    vector<vector<int>> classes;

    for (int x = 1; x <= N; ++x) {
        cout << "+ " << x << '\n';
        cout.flush();
        int D;
        if (!(cin >> D)) return 0;
        q.push_back(x);

        int k = (int)q.size() - 1;
        bool matched = false;
        for (int i = 0; i < k; ++i) {
            int r = q.front();
            q.pop_front();
            cout << "-\n";
            cout.flush();
            int d2;
            if (!(cin >> d2)) return 0;
            if (d2 == D) {
                cls[x] = cls[r];
                classes[cls[r]].push_back(x);
                matched = true;
                break;
            } else {
                cout << "+ " << r << '\n';
                cout.flush();
                int d3;
                if (!(cin >> d3)) return 0;
                q.push_back(r);
            }
        }
        if (!matched) {
            int new_id = (int)classes.size();
            cls[x] = new_id;
            classes.push_back({x});
        }
    }

    cout << "! " << classes.size() << '\n';
    for (int i = 0; i < (int)classes.size(); ++i) {
        cout << classes[i].size();
        for (int v : classes[i]) cout << ' ' << v;
        cout << '\n';
    }
    cout.flush();
    return 0;
}
