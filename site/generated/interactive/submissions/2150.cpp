#include <bits/stdc++.h>
using namespace std;

int N;
vector<int> rem, cand;
vector<char> s, saved, good;
vector<int> c, door, pos;
int free_until = -1;
bool have_free = false;

int query() {
    fwrite("? ", 1, 2, stdout);
    fwrite(s.data(), 1, N, stdout);
    fwrite("\n", 1, 1, stdout);
    fflush(stdout);

    int resp;
    if (scanf("%d", &resp) != 1) exit(0);
    return resp;
}

void finish(int L) {
    int m = (int)rem.size();

    // Current s is all-open, so it gives the correct bits for all remaining switches.
    for (int i : rem) c[i] = s[i] - '0';

    long long total = 1LL * (L + N - 1) * m / 2;
    long long sum = 0;

    // Flip each remaining switch except the last one; the response is its door.
    for (int idx = 0; idx < m - 1; ++idx) {
        int i = rem[idx];
        char old = s[i];
        s[i] = (old == '0' ? '1' : '0');

        int resp = query();
        door[i] = resp;
        sum += resp;

        s[i] = old;
    }

    if (m > 0) {
        int lastDoor = (int)(total - sum);
        door[rem.back()] = lastDoor;
    }

    printf("! ");
    for (int i = 0; i < N; ++i) putchar(c[i] ? '1' : '0');
    for (int i = 0; i < N; ++i) printf(" %d", door[i]);
    putchar('\n');
    fflush(stdout);
    exit(0);
}

void remove_switch(int i) {
    int p = pos[i];
    int last = rem.back();
    rem[p] = last;
    pos[last] = p;
    rem.pop_back();
    pos[i] = -1;

    // Keep known switches correct in the current query string.
    s[i] = c[i] ? '1' : '0';
}

int main() {
    if (scanf("%d", &N) != 1) return 0;

    s.assign(N, '0');
    saved.assign(N, '0');
    good.assign(N, '0');
    c.assign(N, -1);
    door.assign(N, -1);
    pos.resize(N);
    rem.resize(N);

    for (int i = 0; i < N; ++i) {
        rem[i] = i;
        pos[i] = i;
    }

    int L = 0;

    while (L < N) {
        cand = rem;

        // Establish good[i]: if switch i controls door L, then c[i] = good[i].
        if (have_free && L < free_until) {
            for (int i : rem) good[i] = saved[i];
        } else {
            // Probe with all remaining switches set to 0.
            for (int i : rem) s[i] = '0';
            int resp = query();

            if (resp == -1) {
                finish(L);
            }

            if (resp > L) {
                for (int i : rem) good[i] = '0';
                if (resp > free_until) {
                    memcpy(saved.data(), s.data(), N);
                    free_until = resp;
                    have_free = true;
                }
            } else { // resp == L
                for (int i : rem) good[i] = '1';
            }
        }

        // Binary search for the switch controlling door L.
        while (cand.size() > 1) {
            int sz = (int)cand.size();
            int mid = sz / 2;

            // Base bits for remaining switches.
            if (have_free && L < free_until) {
                for (int i : rem) s[i] = saved[i];
            } else {
                for (int i : rem) s[i] = '0';
            }

            // First half: set wrong for door L.
            for (int idx = 0; idx < mid; ++idx) {
                int i = cand[idx];
                s[i] = (good[i] == '0' ? '1' : '0');
            }

            // Second half: set correct for door L.
            for (int idx = mid; idx < sz; ++idx) {
                int i = cand[idx];
                s[i] = good[i];
            }

            int resp = query();

            if (resp == -1) {
                finish(L);
            }

            if (resp == L) {
                // Door L is closed, so its switch is in the first half.
                cand.resize(mid);
            } else {
                // Door L is open, so its switch is in the second half.
                int bsz = sz - mid;
                for (int idx = 0; idx < bsz; ++idx) {
                    cand[idx] = cand[mid + idx];
                }
                cand.resize(bsz);

                if (resp > free_until) {
                    memcpy(saved.data(), s.data(), N);
                    free_until = resp;
                    have_free = true;
                }
            }
        }

        int i = cand[0];
        c[i] = good[i] - '0';
        door[i] = L;
        remove_switch(i);
        ++L;
    }

    printf("! ");
    for (int i = 0; i < N; ++i) putchar(c[i] ? '1' : '0');
    for (int i = 0; i < N; ++i) printf(" %d", door[i]);
    putchar('\n');
    fflush(stdout);

    return 0;
}
