#include <bits/stdc++.h>
using namespace std;

struct Bad {
    vector<int> g0, g1;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    if (!(cin >> N >> M)) return 0;

    auto finish = [&](vector<int> ans) {
        sort(ans.begin(), ans.end());
        cout << "!";
        for (int x : ans) cout << " " << x;
        cout << "\n";
        cout.flush();
    };

    if (M == 0) {
        finish({});
        return 0;
    }

    auto ask = [&](int x, int y) -> int {
        cout << "? " << x << " " << y << "\n";
        cout.flush();
        int r;
        cin >> r;
        if (r == -1) exit(0);
        return r;
    };

    int blockSize = M + 1;
    int B = N / blockSize;

    vector<Bad> bads;
    int sumMin = 0;
    int genuine = -1;

    for (int bi = 0; bi < B; ++bi) {
        int start = bi * blockSize + 1;
        int base = start;

        vector<int> g0, g1;
        g0.push_back(base);

        for (int j = 1; j <= M; ++j) {
            int coin = start + j;
            int r = ask(base, coin);
            if (r == 0) g0.push_back(coin);
            else g1.push_back(coin);
        }

        if (g1.empty()) {
            if (genuine == -1) genuine = base;
        } else {
            bads.push_back({g0, g1});
            int mn = min((int)g0.size(), (int)g1.size());
            sumMin += mn;

            if (sumMin == M) {
                vector<int> ans;
                for (auto &bd : bads) {
                    if (bd.g0.size() < bd.g1.size()) {
                        ans.insert(ans.end(), bd.g0.begin(), bd.g0.end());
                    } else {
                        ans.insert(ans.end(), bd.g1.begin(), bd.g1.end());
                    }
                }
                finish(ans);
                return 0;
            }
        }
    }

    vector<int> rem;
    for (int x = B * blockSize + 1; x <= N; ++x) {
        rem.push_back(x);
    }

    int b = (int)bads.size();
    int R = (int)rem.size();

    vector<unsigned int> candidates;

    for (int om = 0; om < (1 << b); ++om) {
        int total = 0;

        for (int i = 0; i < b; ++i) {
            if ((om >> i) & 1) {
                total += (int)bads[i].g0.size();
            } else {
                total += (int)bads[i].g1.size();
            }
        }

        if (total > M) continue;
        int need = M - total;
        if (need < 0 || need > R) continue;

        for (int sm = 0; sm < (1 << R); ++sm) {
            if (__builtin_popcount((unsigned)sm) != need) continue;

            unsigned int mask = 0;

            for (int i = 0; i < b; ++i) {
                if ((om >> i) & 1) {
                    mask |= (1u << i);
                }
            }

            for (int j = 0; j < R; ++j) {
                if ((sm >> j) & 1) {
                    mask |= (1u << (b + j));
                }
            }

            candidates.push_back(mask);
        }
    }

    int L = b + R;
    vector<int> varCoin(L);

    for (int i = 0; i < b; ++i) {
        varCoin[i] = bads[i].g0[0];
    }
    for (int j = 0; j < R; ++j) {
        varCoin[b + j] = rem[j];
    }

    while (candidates.size() > 1) {
        int bestI = -1, bestJ = -2;
        int bestScore = INT_MAX;
        int bestMin = -1;

        for (int i = 0; i < L; ++i) {
            int cnt0 = 0, cnt1 = 0;

            for (unsigned int mask : candidates) {
                int v = (mask >> i) & 1u;
                if (v == 0) ++cnt0;
                else ++cnt1;
            }

            if (cnt0 == 0 || cnt1 == 0) continue;

            int score = max(cnt0, cnt1);
            int mn = min(cnt0, cnt1);

            if (score < bestScore || (score == bestScore && mn > bestMin)) {
                bestScore = score;
                bestMin = mn;
                bestI = i;
                bestJ = -1;
            }
        }

        for (int i = 0; i < L; ++i) {
            for (int j = i + 1; j < L; ++j) {
                int cnt0 = 0, cnt1 = 0;

                for (unsigned int mask : candidates) {
                    int v = (((mask >> i) ^ (mask >> j)) & 1u);
                    if (v == 0) ++cnt0;
                    else ++cnt1;
                }

                if (cnt0 == 0 || cnt1 == 0) continue;

                int score = max(cnt0, cnt1);
                int mn = min(cnt0, cnt1);

                if (score < bestScore || (score == bestScore && mn > bestMin)) {
                    bestScore = score;
                    bestMin = mn;
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        int answer;

        if (bestJ == -1) {
            answer = ask(varCoin[bestI], genuine);
        } else {
            answer = ask(varCoin[bestI], varCoin[bestJ]);
        }

        vector<unsigned int> nextCandidates;

        for (unsigned int mask : candidates) {
            int v;

            if (bestJ == -1) {
                v = (mask >> bestI) & 1u;
            } else {
                v = (((mask >> bestI) ^ (mask >> bestJ)) & 1u);
            }

            if (v == answer) {
                nextCandidates.push_back(mask);
            }
        }

        candidates.swap(nextCandidates);
    }

    unsigned int mask = candidates[0];
    vector<int> ans;

    for (int i = 0; i < b; ++i) {
        if ((mask >> i) & 1u) {
            ans.insert(ans.end(), bads[i].g0.begin(), bads[i].g0.end());
        } else {
            ans.insert(ans.end(), bads[i].g1.begin(), bads[i].g1.end());
        }
    }

    for (int j = 0; j < R; ++j) {
        if ((mask >> (b + j)) & 1u) {
            ans.push_back(rem[j]);
        }
    }

    finish(ans);
    return 0;
}
