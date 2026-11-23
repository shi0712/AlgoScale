#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;

    auto ask = [&](int c) -> char {
        cout << "? " << c << '\n' << flush;
        string s;
        if (!(cin >> s) || s == "E" || s == "-1") exit(0);
        return s[0];
    };

    auto resetMemory = [&]() {
        cout << "R\n" << flush;
    };

    vector<int> representatives;

    if (k == 1) {
        representatives.push_back(1);

        for (int c = 2; c <= n; ++c) {
            resetMemory();
            bool duplicate = false;

            for (size_t j = 0; j < representatives.size(); ++j) {
                char first = ask(representatives[j]);

                // Before every iteration after the first, the memory contains c.
                if (first == 'Y') {
                    duplicate = true;
                    break;
                }

                if (ask(c) == 'Y') {
                    duplicate = true;
                    break;
                }
                // The memory again contains c.
            }

            if (!duplicate) representatives.push_back(c);
        }
    } else {
        // A sequence of at most k+1 tastings has no eviction before any answer.
        int initial = (k == 2 ? min(n, k + 1) : min(n, k));

        resetMemory();
        for (int c = 1; c <= initial; ++c) {
            if (ask(c) == 'N') representatives.push_back(c);
        }

        int batchSize = max(1, k / 2);
        int pos = initial + 1;

        while (pos <= n) {
            int q = min(batchSize, n - pos + 1);
            vector<int> active;
            active.reserve(q);
            for (int c = pos; c < pos + q; ++c) active.push_back(c);

            size_t offset = 0;
            while (offset < representatives.size() && !active.empty()) {
                int capacity = k + 1 - static_cast<int>(active.size());
                int take = min<int>(capacity, representatives.size() - offset);

                resetMemory();
                for (int j = 0; j < take; ++j) {
                    ask(representatives[offset + j]);
                }

                vector<int> survivors;
                survivors.reserve(active.size());
                for (int c : active) {
                    if (ask(c) == 'N') survivors.push_back(c);
                }

                active.swap(survivors);
                offset += take;
            }

            representatives.insert(representatives.end(), active.begin(), active.end());
            pos += q;
        }
    }

    cout << "! " << representatives.size() << '\n' << flush;
    return 0;
}
