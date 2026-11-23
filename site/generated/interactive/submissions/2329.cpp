#include <bits/stdc++.h>
using namespace std;

int n;
map<string,int> cache_;
vector<char> inS;
int fS;
vector<int> found;

int ask(const string& s){
    bool allZero = true, allOne = true;
    for(char c: s){ if(c=='1') allZero=false; else allOne=false; }
    if(allZero || allOne) return 0;
    auto it = cache_.find(s);
    if(it!=cache_.end()) return it->second;
    cout << "? " << s << "\n" << flush;
    int r;
    if(!(cin >> r)) exit(0);
    if(r < 0) exit(0);
    cache_[s] = r;
    return r;
}

bool test(const vector<int>& T){
    string t(n,'0');
    for(int v: T) t[v]='1';
    int fT = ask(t);
    string st = t;
    for(int i=0;i<n;i++) if(inS[i]) st[i]='1';
    int fST = ask(st);
    return fS + fT - fST > 0;
}

bool solve(const vector<int>& T, bool known){
    if(T.empty()) return false;
    if(!known && !test(T)) return false;
    if(T.size()==1){ found.push_back(T[0]); return true; }
    size_t h = T.size()/2;
    vector<int> T1(T.begin(), T.begin()+h), T2(T.begin()+h, T.end());
    bool a = solve(T1, false);
    solve(T2, !a);
    return true;
}

int main(){
    if(!(cin >> n)) return 0;
    if(n==1){ cout << "! 1\n" << flush; return 0; }
    inS.assign(n,0);
    inS[0]=1;
    while(true){
        int cnt=0;
        for(int i=0;i<n;i++) cnt+=inS[i];
        if(cnt==n){ cout << "! 1\n" << flush; return 0; }
        string s(n,'0');
        for(int i=0;i<n;i++) if(inS[i]) s[i]='1';
        fS = ask(s);
        if(fS==0){ cout << "! 0\n" << flush; return 0; }
        vector<int> C;
        for(int i=0;i<n;i++) if(!inS[i]) C.push_back(i);
        found.clear();
        solve(C, true);
        for(int v: found) inS[v]=1;
    }
}
