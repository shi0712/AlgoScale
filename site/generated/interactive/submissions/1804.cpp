#include <bits/stdc++.h>
using namespace std;

int n, k;

char ask(int c) {
    cout << "? " << c << '\n';
    cout.flush();

    char ch;
    if (!(cin >> ch)) exit(0);
    if (ch == 'E' || ch == '-') exit(0);
    return ch;
}

void resetMemory() {
    cout << "R\n";
    cout.flush();
}

void answer(int d) {
    cout << "! " << d << '\n';
    cout.flush();
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> k;
    if (!cin) return 0;

    if (n == 1) {
        answer(1);
    }

    // If the memory can contain every café, one pass is enough.
    if (n <= k) {
        resetMemory();
        int ans = 0;
        for (int i = 1; i <= n; ++i) {
            if (ask(i) == 'N') ++ans;
        }
        answer(ans);
    }

    // For very small k, process positions directly and test against known values in batches.
    if (k <= 2) {
        int ans = 0;
        vector<int> introduced;

        for (int pos = 1; pos <= n; ++pos) {
            bool duplicate = false;
            ++ans;

            int idx = 0;
            while (idx < (int)introduced.size() && !duplicate) {
                int v = 1; // current block has one position
                int cap = k - v + 1;
                int take = min(cap, (int)introduced.size() - idx);

                resetMemory();
                for (int t = 0; t < take; ++t) {
                    ask(introduced[idx + t]);
                }

                char res = ask(pos);
                if (res == 'Y') {
                    duplicate = true;
                    --ans;
                }

                idx += take;
            }

            if (!duplicate) {
                introduced.push_back(pos);
            }
        }

        answer(ans);
    }

    /*
        k >= 4, n > k.

        Use blocks of size s = k / 2.
        Inside one block, memory can hold all previous elements of the block,
        so each N is a new distinct value inside that block.

        For a current block, let cur be its representatives.
        We test cur against all globally new representatives from previous blocks.
        If a chunk of previous representatives has size u and |cur| = v,
        then u + v - 1 <= k, so during the whole test all elements of the chunk
        stay in memory while we query cur.
    */

    int s = k / 2;
    int ans = 0;
    vector<int> introduced;

    for (int start = 1; start <= n; start += s) {
        resetMemory();

        vector<int> cur;
        for (int pos = start; pos < start + s && pos <= n; ++pos) {
            if (ask(pos) == 'N') {
                cur.push_back(pos);
            }
        }

        ans += (int)cur.size();

        int idx = 0;
        while (idx < (int)introduced.size() && !cur.empty()) {
            int v = (int)cur.size();
            int cap = k - v + 1;
            int take = min(cap, (int)introduced.size() - idx);

            resetMemory();

            for (int t = 0; t < take; ++t) {
                ask(introduced[idx + t]);
            }

            vector<int> nxt;
            for (int pos : cur) {
                char res = ask(pos);
                if (res == 'Y') {
                    --ans;
                } else {
                    nxt.push_back(pos);
                }
            }

            cur.swap(nxt);
            idx += take;
        }

        for (int pos : cur) {
            introduced.push_back(pos);
        }
    }

    answer(ans);
}
