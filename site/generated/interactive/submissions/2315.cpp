#include <bits/stdc++.h>
using namespace std;

long double ask(long long p, long long q){
    printf("? %lld %lld\n", p, q);
    fflush(stdout);
    string a, b;
    if(!(cin >> a)) exit(0);
    if(a == "-1") exit(0);
    if(!(cin >> b)) exit(0);
    long double r = strtold(a.c_str(), nullptr);
    long double s = strtold(b.c_str(), nullptr);
    return r / s;
}

int main(){
    int n;
    if(!(cin >> n)) return 0;
    map<int,int> cnt;
    for(int i = 0; i < n; i++){
        int x, y;
        cin >> x >> y;
        cnt[x]++;
    }
    vector<int> xs;
    bool allGood = true;
    for(auto &kv : cnt){
        xs.push_back(kv.first);
        if(kv.second > 1) allGood = false;
    }
    int k = xs.size();
    long double area = 0;
    if(allGood){
        vector<long double> f(k, 0);
        for(int i = 1; i + 1 < k; i++) f[i] = ask(xs[i], 1);
        for(int i = 0; i + 1 < k; i++){
            area += (f[i] + f[i+1]) / 2 * (xs[i+1] - xs[i]);
        }
    } else {
        for(int i = 0; i + 1 < k; i++){
            long double v = ask((long long)xs[i] + xs[i+1], 2);
            area += v * (xs[i+1] - xs[i]);
        }
    }
    long long T = llroundl(area * 2);
    if(T % 2 == 0) printf("! %lld 1\n", T / 2);
    else printf("! %lld 2\n", T);
    fflush(stdout);
    return 0;
}
