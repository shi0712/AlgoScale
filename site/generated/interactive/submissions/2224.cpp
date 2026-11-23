#include <bits/stdc++.h>
using namespace std;

struct Comp {
    vector<int> a, b;
};

int N, M, rem;
int genuineRef = -1;
vector<int> stateOf;
vector<int> unusedCoins;
vector<Comp> comps;

int ask(int x, int y) {
    cout << "? " << x << " " << y << endl;
    int response;
    if (!(cin >> response) || response == -1) {
        exit(0);
    }
    return response;
}

int applyComp(const Comp &c, int option) {
    for (int x : c.a) {
        stateOf[x] = option ? 1 : 0;
    }
    for (int x : c.b) {
        stateOf[x] = option ? 0 : 1;
    }

    if (genuineRef == -1) {
        if (option == 0 && !c.a.empty()) {
            genuineRef = c.a[0];
        } else if (option == 1 && !c.b.empty()) {
            genuineRef = c.b[0];
        }
    }

    return option ? (int)c.a.size() : (int)c.b.size();
}

void infer() {
    while (true) {
        if (rem == 0) {
            for (int i = 1; i <= N; ++i) {
                if (stateOf[i] == -1) {
                    stateOf[i] = 0;
                }
            }
            unusedCoins.clear();
            comps.clear();
            return;
        }

        struct Item {
            int ca, cb;
        };

        vector<Item> items;
        items.reserve(comps.size() + unusedCoins.size());

        for (const Comp &c : comps) {
            items.push_back({(int)c.a.size(), (int)c.b.size()});
        }
        for (size_t i = 0; i < unusedCoins.size(); ++i) {
            items.push_back({1, 0});
        }

        int C = (int)items.size();
        if (C == 0) {
            return;
        }

        vector<vector<unsigned char>> pre(
            C + 1, vector<unsigned char>(rem + 1, 0));
        vector<vector<unsigned char>> suf(
            C + 1, vector<unsigned char>(rem + 1, 0));

        pre[0][0] = 1;
        for (int i = 0; i < C; ++i) {
            int counts[2] = {items[i].cb, items[i].ca};
            for (int s = 0; s <= rem; ++s) {
                if (!pre[i][s]) {
                    continue;
                }
                for (int o = 0; o < 2; ++o) {
                    if (s + counts[o] <= rem) {
                        pre[i + 1][s + counts[o]] = 1;
                    }
                }
            }
        }

        suf[C][0] = 1;
        for (int i = C - 1; i >= 0; --i) {
            int counts[2] = {items[i].cb, items[i].ca};
            for (int s = 0; s <= rem; ++s) {
                if (!suf[i + 1][s]) {
                    continue;
                }
                for (int o = 0; o < 2; ++o) {
                    if (s + counts[o] <= rem) {
                        suf[i][s + counts[o]] = 1;
                    }
                }
            }
        }

        vector<int> forced(C, -1);
        bool anyForced = false;

        for (int i = 0; i < C; ++i) {
            int counts[2] = {items[i].cb, items[i].ca};
            bool feasible[2] = {false, false};

            for (int o = 0; o < 2; ++o) {
                int c = counts[o];
                if (c > rem) {
                    continue;
                }
                for (int x = 0; x + c <= rem; ++x) {
                    if (!pre[i][x]) {
                        continue;
                    }
                    int need = rem - c - x;
                    if (suf[i + 1][need]) {
                        feasible[o] = true;
                        break;
                    }
                }
            }

            if (!feasible[0] && !feasible[1]) {
                exit(0);
            }
            if (feasible[0] != feasible[1]) {
                forced[i] = feasible[1] ? 1 : 0;
                anyForced = true;
            }
        }

        if (!anyForced) {
            return;
        }

        int fakeFound = 0;
        int K = (int)comps.size();

        vector<Comp> nextComps;
        nextComps.reserve(comps.size());

        for (int i = 0; i < K; ++i) {
            if (forced[i] == -1) {
                nextComps.push_back(std::move(comps[i]));
            } else {
                fakeFound += applyComp(comps[i], forced[i]);
            }
        }

        vector<int> nextUnused;
        nextUnused.reserve(unusedCoins.size());

        for (size_t j = 0; j < unusedCoins.size(); ++j) {
            int f = forced[K + (int)j];
            int x = unusedCoins[j];

            if (f == -1) {
                nextUnused.push_back(x);
            } else if (f == 0) {
                stateOf[x] = 0;
                if (genuineRef == -1) {
                    genuineRef = x;
                }
            } else {
                stateOf[x] = 1;
                ++fakeFound;
            }
        }

        comps.swap(nextComps);
        unusedCoins.swap(nextUnused);
        rem -= fakeFound;

        if (rem < 0) {
            exit(0);
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N >> M)) {
        return 0;
    }

    rem = M;
    stateOf.assign(N + 1, -1);
    unusedCoins.resize(N);
    iota(unusedCoins.begin(), unusedCoins.end(), 1);

    infer();

    while (rem > 0) {
        if (unusedCoins.empty() && comps.empty()) {
            break;
        }

        if (genuineRef != -1 && !comps.empty()) {
            Comp c = std::move(comps.back());
            comps.pop_back();

            int response = ask(genuineRef, c.a[0]);
            rem -= applyComp(c, response);
            infer();
            continue;
        }

        if (!unusedCoins.empty()) {
            if (genuineRef != -1 && comps.empty() &&
                (int)unusedCoins.size() <= rem + 1) {
                int x = unusedCoins.back();
                unusedCoins.pop_back();

                int response = ask(genuineRef, x);
                stateOf[x] = response;
                if (response == 1) {
                    --rem;
                }
                infer();
                continue;
            }

            int take = min((int)unusedCoins.size(), rem + 1);
            Comp c;

            c.a.push_back(unusedCoins.back());
            unusedCoins.pop_back();

            for (int i = 1; i < take; ++i) {
                int x = unusedCoins.back();
                unusedCoins.pop_back();

                int response = ask(c.a[0], x);
                if (response == 0) {
                    c.a.push_back(x);
                } else {
                    c.b.push_back(x);
                }
            }

            comps.push_back(std::move(c));
            infer();
            continue;
        }

        if (comps.size() >= 2) {
            Comp c1 = std::move(comps.back());
            comps.pop_back();
            Comp c2 = std::move(comps.back());
            comps.pop_back();

            int response = ask(c1.a[0], c2.a[0]);
            Comp merged;
            merged.a = c1.a;
            merged.b = c1.b;

            if (response == 0) {
                merged.a.insert(merged.a.end(), c2.a.begin(), c2.a.end());
                merged.b.insert(merged.b.end(), c2.b.begin(), c2.b.end());
            } else {
                merged.a.insert(merged.a.end(), c2.b.begin(), c2.b.end());
                merged.b.insert(merged.b.end(), c2.a.begin(), c2.a.end());
            }

            comps.push_back(std::move(merged));
            infer();
            continue;
        }

        break;
    }

    cout << "!";
    for (int i = 1; i <= N; ++i) {
        if (stateOf[i] == 1) {
            cout << " " << i;
        }
    }
    cout << endl;

    return 0;
}
