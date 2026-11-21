#include <bits/stdc++.h>
using namespace std;

long long ask_query(int i, int j, int k) {
    cout << "? " << i << " " << j << " " << k << endl;
    cout.flush();
    long long resp;
    if(!(cin >> resp)) exit(0);
    if(resp == -1) exit(0);
    return resp;
}

// Find candidates for min (if find_min) or max (if !find_min) among given indices
vector<int> find_extreme_candidates(const vector<int>& init, bool find_min) {
    vector<int> cand = init;
    while (cand.size() > 2) {
        vector<int> nxt;
        int groups = cand.size() / 4;
        for (int g = 0; g < groups; ++g) {
            int i1 = cand[4*g];
            int i2 = cand[4*g+1];
            int i3 = cand[4*g+2];
            int i4 = cand[4*g+3];
            long long q1 = ask_query(i1,i2,i3);
            long long q2 = ask_query(i1,i2,i4);
            long long q3 = ask_query(i1,i3,i4);
            long long q4 = ask_query(i2,i3,i4);
            if (find_min) {
                long long max_sum = max({q1,q2,q3,q4});
                int min_idx;
                if (q1 == max_sum) min_idx = i4;
                else if (q2 == max_sum) min_idx = i3;
                else if (q3 == max_sum) min_idx = i2;
                else min_idx = i1;
                nxt.push_back(min_idx);
            } else {
                long long min_sum = min({q1,q2,q3,q4});
                int max_idx;
                if (q1 == min_sum) max_idx = i4;
                else if (q2 == min_sum) max_idx = i3;
                else if (q3 == min_sum) max_idx = i2;
                else max_idx = i1;
                nxt.push_back(max_idx);
            }
        }
        // leftover
        for (size_t i = groups*4; i < cand.size(); ++i) {
            nxt.push_back(cand[i]);
        }
        cand.swap(nxt);
    }
    return cand; // size <= 2
}

// Find min candidates among 'init' using filler1 (largest) and filler2 (any other distinct) to fill groups
vector<int> find_min_candidates_excluding(const vector<int>& init, int filler1, int filler2) {
    vector<int> cand = init;
    while (cand.size() > 2) {
        vector<int> nxt;
        int groups = cand.size() / 4;
        for (int g = 0; g < groups; ++g) {
            int i1 = cand[4*g];
            int i2 = cand[4*g+1];
            int i3 = cand[4*g+2];
            int i4 = cand[4*g+3];
            long long q1 = ask_query(i1,i2,i3);
            long long q2 = ask_query(i1,i2,i4);
            long long q3 = ask_query(i1,i3,i4);
            long long q4 = ask_query(i2,i3,i4);
            long long max_sum = max({q1,q2,q3,q4});
            int min_idx;
            if (q1 == max_sum) min_idx = i4;
            else if (q2 == max_sum) min_idx = i3;
            else if (q3 == max_sum) min_idx = i2;
            else min_idx = i1;
            nxt.push_back(min_idx);
        }
        size_t leftover_start = groups*4;
        size_t leftover_cnt = cand.size() - leftover_start;
        if (leftover_cnt > 0) {
            vector<int> L;
            for (size_t i = leftover_start; i < cand.size(); ++i) L.push_back(cand[i]);
            // fill with filler indices (distinct)
            if (L.size() < 4) L.push_back(filler1);
            if (L.size() < 4) L.push_back(filler2);
            // ensure distinctness (they are distinct from L by construction)
            int i1 = L[0], i2 = L[1], i3 = L[2], i4 = L[3];
            long long q1 = ask_query(i1,i2,i3);
            long long q2 = ask_query(i1,i2,i4);
            long long q3 = ask_query(i1,i3,i4);
            long long q4 = ask_query(i2,i3,i4);
            long long max_sum = max({q1,q2,q3,q4});
            int min_idx;
            if (q1 == max_sum) min_idx = i4;
            else if (q2 == max_sum) min_idx = i3;
            else if (q3 == max_sum) min_idx = i2;
            else min_idx = i1;
            if (min_idx != filler1 && min_idx != filler2) {
                nxt.push_back(min_idx);
            } else {
                // fallback: push all leftover (they will be processed later)
                for (int x : L) {
                    if (x != filler1 && x != filler2) nxt.push_back(x);
                }
            }
        }
        cand.swap(nxt);
    }
    return cand;
}

// Find max candidates among 'init' using filler1 (smallest) and filler2 (any other distinct) to fill groups
vector<int> find_max_candidates_excluding(const vector<int>& init, int filler1, int filler2) {
    vector<int> cand = init;
    while (cand.size() > 2) {
        vector<int> nxt;
        int groups = cand.size() / 4;
        for (int g = 0; g < groups; ++g) {
            int i1 = cand[4*g];
            int i2 = cand[4*g+1];
            int i3 = cand[4*g+2];
            int i4 = cand[4*g+3];
            long long q1 = ask_query(i1,i2,i3);
            long long q2 = ask_query(i1,i2,i4);
            long long q3 = ask_query(i1,i3,i4);
            long long q4 = ask_query(i2,i3,i4);
            long long min_sum = min({q1,q2,q3,q4});
            int max_idx;
            if (q1 == min_sum) max_idx = i4;
            else if (q2 == min_sum) max_idx = i3;
            else if (q3 == min_sum) max_idx = i2;
            else max_idx = i1;
            nxt.push_back(max_idx);
        }
        size_t leftover_start = groups*4;
        size_t leftover_cnt = cand.size() - leftover_start;
        if (leftover_cnt > 0) {
            vector<int> L;
            for (size_t i = leftover_start; i < cand.size(); ++i) L.push_back(cand[i]);
            if (L.size() < 4) L.push_back(filler1);
            if (L.size() < 4) L.push_back(filler2);
            int i1 = L[0], i2 = L[1], i3 = L[2], i4 = L[3];
            long long q1 = ask_query(i1,i2,i3);
            long long q2 = ask_query(i1,i2,i4);
            long long q3 = ask_query(i1,i3,i4);
            long long q4 = ask_query(i2,i3,i4);
            long long min_sum = min({q1,q2,q3,q4});
            int max_idx;
            if (q1 == min_sum) max_idx = i4;
            else if (q2 == min_sum) max_idx = i3;
            else if (q3 == min_sum) max_idx = i2;
            else max_idx = i1;
            if (max_idx != filler1 && max_idx != filler2) {
                nxt.push_back(max_idx);
            } else {
                for (int x : L) {
                    if (x != filler1 && x != filler2) nxt.push_back(x);
                }
            }
        }
        cand.swap(nxt);
    }
    return cand;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if(!(cin >> n)) return 0;
    vector<int> allIdx(n);
    iota(allIdx.begin(), allIdx.end(), 1);
    // Find candidate minima and maxima
    vector<int> minCands = find_extreme_candidates(allIdx, true); // up to 2
    vector<int> maxCands = find_extreme_candidates(allIdx, false); // up to 2

    // Edge case: all equal values (min and max same)
    if (minCands.size() == 1 && maxCands.size() == 1 && minCands[0] == maxCands[0]) {
        // all values equal, query any triple to get 2*value
        int a = 1, b = 2, c = 3;
        if (a == minCands[0]) a = 4;
        if (b == minCands[0]) b = 5;
        if (c == minCands[0]) c = 6;
        long long sum = ask_query(a,b,c);
        long long val = sum / 2;
        vector<long long> ans(n+1, val);
        cout << "!";
        for (int i = 1; i <= n; ++i) cout << " " << ans[i];
        cout << endl;
        cout.flush();
        return 0;
    }

    bool found = false;
    int final_min = -1, final_max = -1;
    long long final_cmin = 0, final_cmax = 0;
    // Try all combinations
    for (int cand_min : minCands) {
        for (int cand_max : maxCands) {
            if (cand_min == cand_max) continue;
            // Prepare remaining indices
            vector<int> remaining;
            remaining.reserve(n);
            for (int i = 1; i <= n; ++i) {
                if (i == cand_min || i == cand_max) continue;
                remaining.push_back(i);
            }
            // second min candidates
            vector<int> secMinCands = find_min_candidates_excluding(remaining, cand_max, cand_min);
            for (int sec_min : secMinCands) {
                // third min candidates
                vector<int> rem2;
                for (int x : remaining) if (x != sec_min) rem2.push_back(x);
                vector<int> thirdMinCands = find_min_candidates_excluding(rem2, cand_max, cand_min);
                for (int third_min : thirdMinCands) {
                    // second max candidates
                    vector<int> secMaxCands = find_max_candidates_excluding(remaining, cand_min, cand_max);
                    for (int sec_max : secMaxCands) {
                        // third max candidates
                        vector<int> rem3;
                        for (int x : remaining) if (x != sec_max) rem3.push_back(x);
                        vector<int> thirdMaxCands = find_max_candidates_excluding(rem3, cand_min, cand_max);
                        for (int third_max : thirdMaxCands) {
                            // Now we have a,b,c,y,z
                            int a = cand_min;
                            int b = sec_min;
                            int c = third_min;
                            int y = sec_max;
                            int z = cand_max;
                            // Compute required queries
                            long long S_ac = ask_query(a,b,c); // a + c
                            long long S_az = ask_query(a,b,z); // a + z
                            long long S_cz = ask_query(c,y,z); // c + z
                            long long S_bz = ask_query(b,y,z); // b + z
                            long long S_ay = ask_query(a,b,y); // a + y
                            long long delta = S_cz - S_az; // c - a
                            // a = (S_ac - delta) / 2
                            long long numerator = S_ac - delta;
                            if (numerator % 2 != 0) continue;
                            long long a_val = numerator / 2;
                            long long z_val = S_az - a_val;
                            long long c_val = a_val + delta;
                            long long b_val = S_bz - z_val;
                            long long y_val = S_ay - a_val;
                            // Validate ordering and consistency
                            if (a_val < 0 || z_val < 0) continue;
                            if (!(a_val <= b_val && b_val <= c_val && c_val <= y_val && y_val <= z_val)) continue;
                            // Check sums match queries
                            if (a_val + c_val != S_ac) continue;
                            if (a_val + z_val != S_az) continue;
                            if (c_val + z_val != S_cz) continue;
                            if (b_val + z_val != S_bz) continue;
                            if (a_val + y_val != S_ay) continue;
                            // All good
                            final_min = a;
                            final_max = z;
                            final_cmin = a_val;
                            final_cmax = z_val;
                            found = true;
                            break;
                        }
                        if (found) break;
                    }
                    if (found) break;
                }
                if (found) break;
            }
            if (found) break;
        }
        if (found) break;
    }

    if (!found) {
        // Fallback: should not happen
        // As a fallback, we can assume minCands[0] and maxCands[0] are correct and compute using any second/third indices
        // For simplicity, just exit
        return 0;
    }

    // Now we have final_min, final_max, final_cmin, final_cmax
    // Sort all indices using binary insertion with known values
    vector<int> sortedIdx;
    sortedIdx.reserve(n);
    sortedIdx.push_back(final_min);
    sortedIdx.push_back(final_max);
    unordered_map<int, long long> value;
    value[final_min] = final_cmin;
    value[final_max] = final_cmax;

    // Insert remaining indices
    for (int i = 1; i <= n; ++i) {
        if (i == final_min || i == final_max) continue;
        // binary search position between 1 and sortedIdx.size()-1 (excluding position 0 which is min)
        int low = 1;
        int high = (int)sortedIdx.size();
        while (low < high) {
            int mid = (low + high) / 2;
            int piv = sortedIdx[mid];
            long long ans = ask_query(i, piv, final_min);
            long long max_val = ans - final_cmin;
            if (max_val == value[piv]) {
                // i <= piv
                high = mid;
            } else {
                // i > piv
                low = mid + 1;
            }
        }
        // Insert at position low
        // Compute its value using left neighbor
        int leftIdx = sortedIdx[low-1];
        long long ans_val = ask_query(i, leftIdx, final_min);
        long long ci = ans_val - final_cmin; // since leftIdx value <= ci
        value[i] = ci;
        sortedIdx.insert(sortedIdx.begin() + low, i);
    }

    // Prepare answer in original order
    vector<long long> ans(n+1);
    for (int i = 1; i <= n; ++i) ans[i] = value[i];
    cout << "!";
    for (int i = 1; i <= n; ++i) {
        cout << " " << ans[i];
    }
    cout << endl;
    cout.flush();
    return 0;
}
