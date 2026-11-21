#include <bits/stdc++.h>
using namespace std;

char readResponse() {
    string s;
    if (!(cin >> s)) exit(0);
    if (s == "E") exit(0);
    return s[0]; // 'Y' or 'N'
}

char query(int idx) {
    cout << "? " << idx << "\n" << flush;
    return readResponse();
}

void resetMemory() {
    cout << "R\n" << flush;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n, k;
    if (!(cin >> n >> k)) return 0;
    
    // Special case when k == 1
    if (k == 1) {
        vector<int> reps;
        for (int i = 1; i <= n; ++i) {
            bool found = false;
            for (int rep : reps) {
                resetMemory();
                query(rep); // load rep into memory, ignore answer
                char ans = query(i);
                if (ans == 'Y') {
                    found = true;
                    break;
                }
            }
            if (!found) reps.push_back(i);
        }
        cout << "! " << (int)reps.size() << "\n" << flush;
        return 0;
    }
    
    vector<int> globalReps; // representative indices of distinct values
    int blockSize = k; // block size <= k
    
    for (int blockStart = 1; blockStart <= n; blockStart += blockSize) {
        resetMemory();
        vector<int> blockReps;
        int blockEnd = min(blockStart + blockSize - 1, n);
        for (int i = blockStart; i <= blockEnd; ++i) {
            char ans = query(i);
            if (ans == 'N') {
                // first occurrence of this value within the block
                blockReps.push_back(i);
            }
            // else duplicate within block, ignore
        }
        // Merge block representatives into global set
        for (int idx : blockReps) {
            bool found = false;
            size_t start = 0;
            while (start < globalReps.size() && !found) {
                resetMemory();
                size_t end = min(start + (size_t)(k - 1), globalReps.size());
                for (size_t t = start; t < end; ++t) {
                    query(globalReps[t]); // load rep, ignore answer
                }
                char ans = query(idx);
                if (ans == 'Y') {
                    found = true;
                }
                start = end;
            }
            if (!found) {
                globalReps.push_back(idx);
            }
        }
    }
    
    cout << "! " << (int)globalReps.size() << "\n" << flush;
    return 0;
}
