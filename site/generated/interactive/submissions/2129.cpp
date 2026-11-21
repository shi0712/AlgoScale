#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    long long cnt;
    if (!(cin >> cnt)) return 0;
    if (cnt == -1) return 0;
    
    long long cur = 0; // sum of subtractions = original n
    while (cnt > 0) {
        // Subtract 1
        cout << "- 1" << endl;
        long long new_cnt;
        if (!(cin >> new_cnt)) return 0;
        if (new_cnt == -1) return 0;
        long long t = new_cnt - cnt + 1; // number of trailing zeros of current number
        cur += 1;
        cnt = new_cnt;
        if (t == 0) continue; // LSB was 1, nothing more to subtract this round
        
        // Subtract (2^t - 1) to clear lower t bits and remove the lowest set bit
        long long x = (1LL << t) - 1;
        cout << "- " << x << endl;
        if (!(cin >> new_cnt)) return 0;
        if (new_cnt == -1) return 0;
        cur += x;
        cnt = new_cnt;
    }
    
    cout << "! " << cur << endl;
    return 0;
}
