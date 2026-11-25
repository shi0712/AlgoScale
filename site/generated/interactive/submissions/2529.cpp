#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    if (N == 1) {
        // Only one switch and one door.
        cout << "? 0\n" << flush;
        int r;
        cin >> r;
        if (r == -1) {
            cout << "! 0 0\n" << flush;
        } else {
            cout << "! 1 0\n" << flush;
        }
        return 0;
    }

    vector<int> c(N, 0), d(N, 0);
    vector<bool> c_known(N, false), d_known(N, false), door_used(N, false);
    vector<int> rem(N);
    iota(rem.begin(), rem.end(), 0);
    bool all_c_known = false;

    // First query: all zeros
    string s(N, '0');
    cout << "? " << s << '\n' << flush;
    int resp0;
    cin >> resp0;

    if (resp0 == -1) {
        // All switches have correct position 0
        for (int i = 0; i < N; ++i) {
            c[i] = 0;
            c_known[i] = true;
        }
        all_c_known = true;
    } else {
        int first_r = resp0;

        // If door 0 is closed with all zeros, it has correct position 1.
        // Check whether all switches are 1.
        if (resp0 == 0) {
            string s1(N, '1');
            cout << "? " << s1 << '\n' << flush;
            int resp1;
            cin >> resp1;
            if (resp1 == -1) {
                for (int i = 0; i < N; ++i) {
                    c[i] = 1;
                    c_known[i] = true;
                }
                all_c_known = true;
            }
        }

        if (!all_c_known) {
            int r = first_r;
            while (true) {
                // Find the switch with door r and correct position 1
                vector<int> cand = rem;
                while (true) {
                    if (cand.size() == 1) break;
                    int mid = cand.size() / 2;
                    vector<int> A(cand.begin(), cand.begin() + mid);

                    string q(N, '0');
                    for (int i = 0; i < N; ++i)
                        if (c_known[i]) q[i] = c[i] ? '1' : '0';
                    for (int i : A) q[i] = '1';

                    cout << "? " << q << '\n' << flush;
                    int resp2;
                    cin >> resp2;

                    if (resp2 == -1) {
                        // All remaining switches match q
                        for (int i : rem) {
                            c[i] = (q[i] == '1');
                            c_known[i] = true;
                        }
                        all_c_known = true;
                        cand = A;  // the target must be in A
                        continue;
                    }

                    bool inA;
                    if (resp2 == r) inA = false;
                    else inA = true;

                    if (inA) cand = A;
                    else cand.assign(cand.begin() + mid, cand.end());
                }

                int a = cand[0];
                c_known[a] = true;
                c[a] = 1;
                d_known[a] = true;
                d[a] = r;
                door_used[r] = true;
                rem.erase(find(rem.begin(), rem.end(), a));

                if (all_c_known) break;

                // Query all zeros to find the next door with correct position 1
                string q(N, '0');
                for (int i = 0; i < N; ++i)
                    if (c_known[i]) q[i] = c[i] ? '1' : '0';
                cout << "? " << q << '\n' << flush;
                int resp;
                cin >> resp;
                if (resp == -1) break;
                r = resp;
            }
        }
    }

    if (all_c_known) {
        // Recover doors for all switches whose door is still unknown
        vector<int> unassigned;
        for (int i = 0; i < N; ++i)
            if (!d_known[i]) unassigned.push_back(i);

        for (int idx = 0; idx + 1 < (int)unassigned.size(); ++idx) {
            int i = unassigned[idx];
            string q(N, '0');
            for (int j = 0; j < N; ++j)
                q[j] = c[j] ? '1' : '0';
            q[i] = c[i] ? '0' : '1';

            cout << "? " << q << '\n' << flush;
            int resp;
            cin >> resp;
            d[i] = resp;
            d_known[i] = true;
            door_used[resp] = true;
        }
        if (!unassigned.empty()) {
            int i = unassigned.back();
            int missing = -1;
            for (int j = 0; j < N; ++j)
                if (!door_used[j]) { missing = j; break; }
            d[i] = missing;
            d_known[i] = true;
            door_used[missing] = true;
        }
    } else {
        // All remaining switches have correct position 0
        for (int i : rem) {
            c[i] = 0;
            c_known[i] = true;
        }

        vector<int> unassigned = rem;
        for (int idx = 0; idx + 1 < (int)unassigned.size(); ++idx) {
            int i = unassigned[idx];
            string q(N, '0');
            for (int j = 0; j < N; ++j)
                if (c_known[j]) q[j] = c[j] ? '1' : '0';
            q[i] = '1';

            cout << "? " << q << '\n' << flush;
            int resp;
            cin >> resp;
            d[i] = resp;
            d_known[i] = true;
            door_used[resp] = true;
        }
        if (!unassigned.empty()) {
            int i = unassigned.back();
            int missing = -1;
            for (int j = 0; j < N; ++j)
                if (!door_used[j]) { missing = j; break; }
            d[i] = missing;
            d_known[i] = true;
            door_used[missing] = true;
        }
    }

    cout << "! ";
    for (int i = 0; i < N; ++i)
        cout << (c[i] ? '1' : '0');
    cout << ' ';
    for (int i = 0; i < N; ++i)
        cout << d[i] << (i == N - 1 ? '\n' : ' ');
    cout.flush();

    return 0;
}
