#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> knownA, knownB, rem;
    knownA.push_back(0); // mushroom 0 is species A
    for (int i = 1; i < n; ++i) rem.push_back(i);

    long long totalA = 1, totalB = 0;
    long long usedK = 0;
    const long long K_LIMIT = 100000;

    auto sample_one = [&]() {
        int idx = rem.back();
        cout << "? 2 0 " << idx << '\n';
        cout.flush();
        usedK += 2;

        int ans;
        cin >> ans;
        if (ans < 0) exit(0);

        rem.pop_back();
        if (ans == 0) {
            knownA.push_back(idx);
            ++totalA;
        } else {
            knownB.push_back(idx);
            ++totalB;
        }
    };

    auto count_target = [&](const vector<int>& subset, const vector<int>& refvec) -> int {
        int q = (int)subset.size();
        cout << "? " << (2 * q + 1);
        for (int i = 0; i < q; ++i) {
            cout << ' ' << refvec[i] << ' ' << subset[i];
        }
        cout << ' ' << refvec[q] << '\n';
        cout.flush();

        usedK += 2 * q + 1;

        int ans;
        cin >> ans;
        if (ans < 0) exit(0);
        return ans / 2;
    };

    auto refine_batch = [&](const vector<int>& batch, int c, bool targetIsB,
                            vector<int>& targetVec, vector<int>& refVec) {
        int t = (int)batch.size();

        vector<int> pos(n, -1);
        for (int i = 0; i < t; ++i) pos[batch[i]] = i;

        vector<char> isTarget(t, 0);
        vector<int> remaining = batch;

        for (int it = 0; it < c; ++it) {
            vector<int> cur = remaining;

            while (cur.size() > 1) {
                int mid = (int)cur.size() / 2;
                vector<int> left(cur.begin(), cur.begin() + mid);

                int cnt = count_target(left, refVec);

                if (cnt > 0) {
                    cur = left;
                } else {
                    cur.assign(cur.begin() + mid, cur.end());
                }
            }

            int targetLabel = cur[0];
            isTarget[pos[targetLabel]] = 1;

            for (int i = 0; i < (int)remaining.size(); ++i) {
                if (remaining[i] == targetLabel) {
                    remaining.erase(remaining.begin() + i);
                    break;
                }
            }
        }

        for (int i = 0; i < t; ++i) {
            if (isTarget[i]) targetVec.push_back(batch[i]);
            else refVec.push_back(batch[i]);
        }
    };

    bool sampling = true;

    while (!rem.empty()) {
        int U = (int)rem.size();
        int a = (int)knownA.size();
        int b = (int)knownB.size();
        int s = max(a, b);

        if (sampling) {
            if (s < 2) {
                sample_one();
                continue;
            }

            bool skew = (min(a, b) <= 1);
            bool needSample = false;

            if (skew) {
                if (1LL * s * s < U) needSample = true;
                if (!needSample && 2LL * s * (s - 1) < U) needSample = true;
            } else {
                if (2LL * s * (s - 1) < U) needSample = true;
            }

            if (needSample) {
                sample_one();
                continue;
            } else {
                sampling = false;
            }
        }

        a = (int)knownA.size();
        b = (int)knownB.size();
        s = max(a, b);

        if (s < 2) {
            sampling = true;
            continue;
        }

        bool useA = (a >= b);
        int capacity = (useA ? a : b) - 1;

        if (capacity <= 0) {
            sampling = true;
            continue;
        }

        U = (int)rem.size();
        int t = min(U, capacity);

        if (t == 1) {
            int idx = rem.back();

            cout << "? 2 0 " << idx << '\n';
            cout.flush();
            usedK += 2;

            int ans;
            cin >> ans;
            if (ans < 0) return 0;

            rem.pop_back();

            if (ans == 0) {
                knownA.push_back(idx);
                ++totalA;
            } else {
                knownB.push_back(idx);
                ++totalB;
            }
            continue;
        }

        vector<int> batch(rem.end() - t, rem.end());
        rem.resize(rem.size() - t);

        vector<int>& ref = useA ? knownA : knownB;

        cout << "? " << (2 * t + 1);
        for (int i = 0; i < t; ++i) {
            cout << ' ' << ref[i] << ' ' << batch[i];
        }
        cout << ' ' << ref[t] << '\n';
        cout.flush();

        usedK += 2 * t + 1;

        int ans;
        cin >> ans;
        if (ans < 0) return 0;

        int opposite = ans / 2;
        int countA, countB;

        if (useA) {
            countB = opposite;
            countA = t - opposite;
        } else {
            countA = opposite;
            countB = t - opposite;
        }

        totalA += countA;
        totalB += countB;

        if (countA == t) {
            for (int x : batch) knownA.push_back(x);
        } else if (countB == t) {
            for (int x : batch) knownB.push_back(x);
        } else {
            int Urem = (int)rem.size();

            if (Urem > 0) {
                int c = -1;
                bool targetIsB = false;
                vector<int>* refPtr = nullptr;
                vector<int>* targetPtr = nullptr;

                if (countB <= countA && (int)knownA.size() >= t + 1) {
                    c = countB;
                    targetIsB = true;
                    refPtr = &knownA;
                    targetPtr = &knownB;
                } else if (countA <= countB && (int)knownB.size() >= t + 1) {
                    c = countA;
                    targetIsB = false;
                    refPtr = &knownB;
                    targetPtr = &knownA;
                }

                if (c > 0 && c < t) {
                    int a0 = (int)knownA.size();
                    int b0 = (int)knownB.size();

                    int max0 = max(a0, b0);
                    int den0 = max(1, max0 - 1);
                    int q0 = (Urem + den0 - 1) / den0;

                    int newA = a0 + countA;
                    int newB = b0 + countB;

                    int max1 = max(newA, newB);
                    int den1 = max(1, max1 - 1);
                    int q1 = (Urem + den1 - 1) / den1;

                    int benefit = q0 - q1;

                    int logt = 0;
                    while ((1 << logt) < t) ++logt;

                    int costQueries = c * max(1, logt);
                    long long estimatedK = 1LL * c * (2LL * t + 20);

                    int snew = max(newA, newB);
                    long long futureK = 0;

                    if (Urem > 0) {
                        if (snew < 2) {
                            futureK = 2LL * Urem;
                        } else {
                            int capnew = snew - 1;
                            if (capnew <= 1) {
                                futureK = 2LL * Urem;
                            } else {
                                long long qq = (Urem + capnew - 1) / capnew;
                                futureK = 2LL * Urem + qq;
                            }
                        }
                    }

                    if (benefit > costQueries &&
                        usedK + estimatedK + futureK <= K_LIMIT) {
                        refine_batch(batch, c, targetIsB, *targetPtr, *refPtr);
                    }
                }
            }
        }
    }

    cout << "! " << totalA << '\n';
    cout.flush();

    return 0;
}
