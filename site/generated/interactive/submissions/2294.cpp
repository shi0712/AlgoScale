#include <bits/stdc++.h>
using namespace std;

static int rd(){
    int x;
    if(!(cin >> x)) exit(0);
    if(x == -1) exit(0);
    return x;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int M = rd();
    for(int u = 0; u < M; ++u){
        int X = M - 1 - u;
        for(;;){
            cout << "? 1 " << u << " " << X << "\n";
            cout.flush();
            int r = rd();
            if(r == 0) break;
        }
    }
    cout << "? 2\n";
    cout.flush();
    int N = rd();
    if(N <= 0) return 0;
    vector<int> lab(N);
    for(int i = 0; i < N; ++i){
        int v = rd();
        lab[i] = M - 1 - v;
    }
    vector<int> par(N, -1);
    vector<int> stk;
    stk.reserve(N);
    for(int i = 0; i < N; ++i){
        while(!stk.empty() && lab[stk.back()] > lab[i]) stk.pop_back();
        if(!stk.empty()) par[i] = stk.back();
        stk.push_back(i);
    }
    vector<int> ch(N, 0);
    for(int i = 1; i < N; ++i) ++ch[par[i]];
    vector<int> id(N, -1);
    for(int i = 0; i < N; ++i) if(ch[i] == 0) id[i] = lab[i];
    id[0] = N - 1;
    int nxt = M;
    for(int i = 1; i < N; ++i) if(ch[i] > 0) id[i] = nxt++;
    vector<int> R(N - 1, -1);
    for(int i = 1; i < N; ++i) R[id[i]] = id[par[i]];
    cout << "! " << N;
    for(int i = 0; i < N - 1; ++i) cout << " " << R[i];
    cout << "\n";
    cout.flush();
    return 0;
}
