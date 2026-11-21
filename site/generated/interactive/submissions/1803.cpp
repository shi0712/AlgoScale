#include <bits/stdc++.h>
using namespace std;

int n;
bool removedPos[805];
int val[805];

int askVector(const vector<int>& a) {
    cout << "? " << (int)a.size();
    for (int x : a) cout << ' ' << x;
    cout << '\n' << flush;

    int ans;
    cin >> ans;
    if (ans == -1) exit(0);
    return ans;
}

int askExceptRem(const vector<int>& rem, int exclude) {
    cout << "? " << (int)rem.size() - 1;
    for (int x : rem) {
        if (x != exclude) cout << ' ' << x;
    }
    cout << '\n' << flush;

    int ans;
    cin >> ans;
    if (ans == -1) exit(0);
    return ans;
}

pair<int, int> findNextPair() {
    vector<int> rem;
    for (int i = 1; i <= n; ++i) {
        if (!removedPos[i]) rem.push_back(i);
    }

    if ((int)rem.size() == 2) {
        return {rem[0], rem[1]};
    }

    vector<int> found;
    for (int x : rem) {
        int ans = askExceptRem(rem, x);
        if (ans == 1) {
            found.push_back(x);
            if ((int)found.size() == 2) break;
        }
    }

    return {found[0], found[1]};
}

vector<vector<int>> buildWitness(int k, const vector<int>& vals) {
    int K = (int)vals.size();
    int c = k - 1;
    vector<vector<int>> res(k);

    if (c < 0 || c > K) return res;

    vector<vector<char>> reach(c + 1, vector<char>(k, 0));
    vector<vector<int>> lastIdx(c + 1, vector<int>(k, -1));
    vector<vector<int>> prevRes(c + 1, vector<int>(k, -1));

    reach[0][0] = 1;

    for (int idx = 0; idx < K; ++idx) {
        int v = vals[idx] % k;
        int up = min(c - 1, idx);

        for (int cnt = up; cnt >= 0; --cnt) {
            for (int r = 0; r < k; ++r) {
                if (!reach[cnt][r]) continue;

                int nr = (r + v) % k;
                if (!reach[cnt + 1][nr]) {
                    reach[cnt + 1][nr] = 1;
                    lastIdx[cnt + 1][nr] = idx;
                    prevRes[cnt + 1][nr] = r;
                }
            }
        }
    }

    for (int target = 0; target < k; ++target) {
        int need = (k - target) % k;

        if (!reach[c][need]) continue;

        vector<int> sub;
        int cnt = c;
        int cur = need;
        bool ok = true;

        while (cnt > 0) {
            int idx = lastIdx[cnt][cur];
            if (idx == -1) {
                ok = false;
                break;
            }
            sub.push_back(idx);
            cur = prevRes[cnt][cur];
            --cnt;
        }

        if (ok) res[target] = sub;
    }

    return res;
}

bool allWitnesses(const vector<int>& vals) {
    vector<int> base = {8, 3, 5, 7};

    for (int k : base) {
        auto wit = buildWitness(k, vals);
        for (int r = 0; r < k; ++r) {
            if (wit[r].empty()) return false;
        }
    }

    return true;
}

struct ModWit {
    int k;
    vector<vector<int>> wit;
};

void printAnswer() {
    bool flip = false;
    if (val[1] > n / 2) flip = true;

    cout << "!";
    for (int i = 1; i <= n; ++i) {
        int out = flip ? (n + 1 - val[i]) : val[i];
        cout << ' ' << out;
    }
    cout << '\n' << flush;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    if (n <= 60) {
        vector<pair<int, int>> pairs;

        for (int t = 0; t < n / 2; ++t) {
            auto pr = findNextPair();
            pairs.push_back(pr);
            removedPos[pr.first] = removedPos[pr.second] = true;
        }

        int anchorPos = -1;
        int anchorValue = -1;

        for (int t = 0; t < n / 2; ++t) {
            int a = pairs[t].first;
            int b = pairs[t].second;
            int low = t + 1;
            int high = n - t;

            if (t == 0) {
                val[a] = low;
                val[b] = high;
                anchorPos = a;
                anchorValue = low;
            } else {
                vector<int> query = {a, anchorPos};
                int ans = askVector(query);

                int wantParity;
                if (ans == 1) wantParity = anchorValue & 1;
                else wantParity = (anchorValue & 1) ^ 1;

                if ((low & 1) == wantParity) {
                    val[a] = low;
                    val[b] = high;
                } else {
                    val[a] = high;
                    val[b] = low;
                }
            }
        }

        printAnswer();
        return 0;
    }

    vector<int> knownPositions;
    vector<int> knownValues;

    int anchorPos = -1;
    int anchorValue = -1;
    int t = 0;

    while (true) {
        auto pr = findNextPair();
        int a = pr.first;
        int b = pr.second;

        removedPos[a] = removedPos[b] = true;

        int low = t + 1;
        int high = n - t;

        if (t == 0) {
            val[a] = low;
            val[b] = high;
            anchorPos = a;
            anchorValue = low;
        } else {
            vector<int> query = {a, anchorPos};
            int ans = askVector(query);

            int wantParity;
            if (ans == 1) wantParity = anchorValue & 1;
            else wantParity = (anchorValue & 1) ^ 1;

            if ((low & 1) == wantParity) {
                val[a] = low;
                val[b] = high;
            } else {
                val[a] = high;
                val[b] = low;
            }
        }

        knownPositions.push_back(a);
        knownPositions.push_back(b);
        knownValues.push_back(val[a]);
        knownValues.push_back(val[b]);

        ++t;

        if (t == n / 2) {
            printAnswer();
            return 0;
        }

        if (allWitnesses(knownValues)) break;
    }

    int K = (int)knownValues.size();

    int maxExtra = min(n, K + 1);
    maxExtra = min(maxExtra, 30);

    vector<ModWit> mods;

    for (int k = 2; k <= maxExtra; ++k) {
        if (k - 1 > K) continue;

        auto wit = buildWitness(k, knownValues);

        bool any = false;
        for (int r = 0; r < k; ++r) {
            if (!wit[r].empty()) {
                any = true;
                break;
            }
        }

        if (any) {
            mods.push_back({k, wit});
        }
    }

    vector<char> used(n + 1, 0);
    for (int pos : knownPositions) {
        used[val[pos]] = 1;
    }

    vector<int> remainingValues;
    for (int v = 1; v <= n; ++v) {
        if (!used[v]) remainingValues.push_back(v);
    }

    vector<int> unknownPositions;
    for (int i = 1; i <= n; ++i) {
        if (!removedPos[i]) unknownPositions.push_back(i);
    }

    for (int x : unknownPositions) {
        vector<int> candidates = remainingValues;

        while (candidates.size() > 1) {
            int sz = (int)candidates.size();

            int bestWorst = INT_MAX;
            int bestMod = -1;
            int bestResidue = -1;

            for (int mi = 0; mi < (int)mods.size(); ++mi) {
                int k = mods[mi].k;
                vector<int> cnt(k, 0);

                for (int v : candidates) {
                    ++cnt[v % k];
                }

                for (int r = 0; r < k; ++r) {
                    if (mods[mi].wit[r].empty()) continue;

                    int c = cnt[r];
                    if (c == 0 || c == sz) continue;

                    int worst = max(c, sz - c);

                    if (worst < bestWorst) {
                        bestWorst = worst;
                        bestMod = mi;
                        bestResidue = r;
                    } else if (worst == bestWorst) {
                        if (bestMod == -1 || k < mods[bestMod].k) {
                            bestMod = mi;
                            bestResidue = r;
                        }
                    }
                }
            }

            int k = mods[bestMod].k;
            const vector<int>& wit = mods[bestMod].wit[bestResidue];

            vector<int> query;
            query.reserve(k);
            query.push_back(x);

            for (int idx : wit) {
                query.push_back(knownPositions[idx]);
            }

            int ans = askVector(query);

            vector<int> nextCandidates;

            if (ans == 1) {
                for (int v : candidates) {
                    if (v % k == bestResidue) nextCandidates.push_back(v);
                }
            } else {
                for (int v : candidates) {
                    if (v % k != bestResidue) nextCandidates.push_back(v);
                }
            }

            candidates.swap(nextCandidates);
        }

        int chosen = candidates[0];
        val[x] = chosen;

        auto it = find(remainingValues.begin(), remainingValues.end(), chosen);
        if (it != remainingValues.end()) {
            remainingValues.erase(it);
        }
    }

    printAnswer();
    return 0;
}
