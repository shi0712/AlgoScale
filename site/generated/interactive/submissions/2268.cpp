#include <bits/stdc++.h>
using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if(!(cin >> n)) return 0;
    vector<int> A, B;
    A.push_back(0);
    vector<int> rem;
    rem.reserve(n);
    for(int i = 1; i < n; i++) rem.push_back(i);
    long long total = 1; // mushroom 0 is species A
    int idx = 0;
    int R = (int)rem.size();
    while(idx < R){
        bool useA = A.size() >= B.size();
        int p = useA ? (int)A.size() : (int)B.size();
        int t = min(p, R - idx);
        string s = "? ";
        s += to_string(2*t);
        for(int i = 0; i < t; i++){
            s += ' ';
            s += to_string(rem[idx+i]);
            s += ' ';
            s += to_string(useA ? A[i] : B[i]);
        }
        s += '\n';
        cout << s;
        cout.flush();
        long long f;
        if(!(cin >> f)) return 0;
        if(f < 0) return 0;
        int b1 = (int)(f & 1LL);
        int rest = (int)((f - b1) / 2);
        // useA: f = [u1=B] + 2*#(B among u2..ut)
        // useB: f = [u1=A] + 2*#(A among u2..ut)
        bool u1A = useA ? (b1 == 0) : (b1 == 1);
        total += (u1A ? 1 : 0) + (useA ? (t - 1 - rest) : rest);
        if(u1A) A.push_back(rem[idx]); else B.push_back(rem[idx]);
        if(t > 1){
            if(rest == 0){
                // all interior equal to separator species
                for(int i = 1; i < t; i++){
                    if(useA) A.push_back(rem[idx+i]);
                    else B.push_back(rem[idx+i]);
                }
            } else if(rest == t - 1){
                for(int i = 1; i < t; i++){
                    if(useA) B.push_back(rem[idx+i]);
                    else A.push_back(rem[idx+i]);
                }
            }
        }
        idx += t;
    }
    cout << "! " << total << "\n";
    cout.flush();
    return 0;
}
