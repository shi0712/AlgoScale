#include <bits/stdc++.h>
using namespace std;

int ask(int i, int j) {
    cout << "? " << i + 1 << ' ' << j + 1 << endl;
    int g;
    if (!(cin >> g) || g == -1) exit(0);
    return g;
}

[[noreturn]] void report(int i, int j) {
    cout << "! " << i + 1 << ' ' << j + 1 << endl;
    int verdict;
    if (!(cin >> verdict) || verdict == -1) exit(0);
    exit(0);
}

// For small permutations, keep the exact set of possibilities.
struct Small {
    int n;
    vector<pair<int,int>> pairs;
    vector<unsigned char> responses, zero;
    map<vector<int>, pair<int,int>> memo;

    pair<int,int> exact(const vector<int>& s) {
        int mask = 0;
        for (int v : s) mask |= 1 << zero[v];
        if (__builtin_popcount((unsigned)mask) <= 2) return {0,-1};
        auto it = memo.find(s);
        if (it != memo.end()) return it->second;
        pair<int,int> best = {INT_MAX,-1};
        int m = pairs.size();
        for (int q = 0; q < m; ++q) {
            vector<vector<int>> groups(n);
            for (int v : s) groups[responses[v*m+q]].push_back(v);
            int cost = s.size();
            for (auto &t : groups) {
                if (t.size() == s.size()) { cost = INT_MAX; break; }
                if (!t.empty()) cost += exact(t).first;
                if (cost >= best.first) break;
            }
            if (cost < best.first) best = {cost,q};
        }
        return memo[s] = best;
    }

    void run(int size) {
        n = size;
        for (int i = 0; i < n; ++i)
            for (int j = i+1; j < n; ++j) pairs.emplace_back(i,j);
        int m = pairs.size();
        vector<int> p(n);
        iota(p.begin(),p.end(),0);
        int gcds[10][10];
        for (int i=0;i<n;i++) for (int j=0;j<n;j++) gcds[i][j]=gcd(i,j);
        do {
            zero.push_back(find(p.begin(),p.end(),0)-p.begin());
            for (auto [i,j] : pairs) responses.push_back(gcds[p[i]][p[j]]);
        } while (next_permutation(p.begin(),p.end()));

        vector<int> s(zero.size());
        iota(s.begin(),s.end(),0);
        vector<bool> used(m);
        while (true) {
            int mask=0;
            for (int v:s) mask |= 1<<zero[v];
            if (__builtin_popcount((unsigned)mask)<=2)
                report(__builtin_ctz((unsigned)mask),31-__builtin_clz((unsigned)mask));
            int q=-1;
            if (n<=5) q=exact(s).second;
            else if (s.size()==zero.size()) q=0; // all pairs are symmetric
            else {
                double best=1e100;
                for (int k=0;k<m;k++) if (!used[k]) {
                    int freq[10][10]={};
                    for (int v:s) ++freq[responses[v*m+k]][zero[v]];
                    double score=0;
                    bool splits=false;
                    for (int g=1;g<n;g++) {
                        int *f=freq[g];
                        sort(f,f+n,greater<int>());
                        int tot=accumulate(f,f+n,0);
                        if (tot && tot<(int)s.size()) splits=true;
                        if (f[2]) {
                            score+=tot;
                            for (int h=2;h<n;h++) score+=(h/2)*f[h];
                            score+=0.015*tot*log((double)tot);
                        }
                    }
                    if (splits && score<best) best=score,q=k;
                }
            }
            used[q]=true;
            auto [a,b]=pairs[q];
            int g=ask(a,b);
            s.erase(remove_if(s.begin(),s.end(),[&](int v){
                return responses[v*m+q]!=g;
            }),s.end());
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin>>n) || n<2) return 0;
    if (n<=9) Small().run(n);

    int N=n-1;
    vector<int> label(n);
    iota(label.begin(),label.end(),0);
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    shuffle(label.begin(),label.end(),rng);

    vector<vector<int>> divisors(n);
    for (int d=2;d<=N;d++)
        for (int v=d;v<=N;v+=d) divisors[v].push_back(d);

    // L[i] divides p[i], even if p[i] is zero.
    vector<int> L(n,1), countDiv(n);
    vector<vector<pair<int,int>>> history(n);
    vector<double> evidence(n,0),priority(n,0);
    vector<bool> live(n,true);
    set<pair<double,int>> active;
    for (int i=0;i<n;i++) active.emplace(0,i);

    auto discard=[&](int i) {
        if (live[i]) {
            active.erase({priority[i],i});
            live[i]=false;
        }
    };
    auto update=[&](int i,int j,int g) {
        for (auto [k,h] : history[i])
            if (h==g) discard(i); // zero would give equal values at k and j
        history[i].emplace_back(j,g);
        int old=L[i];
        long long v=lcm((long long)old,(long long)g);
        if (v>N) report(label[i],label[i]);
        L[i]=int(v);
        if (v!=old) {
            for (auto [k,h] : history[i])
                if (h!=v) discard(k); // if k were zero, p[i] would be h
            for (int d:divisors[v]) if (old%d) {
                if (++countDiv[d]>N/d) {
                    // Too many known multiples to all have positive values.
                    for (int k=0;k<n;k++) if (L[k]%d) discard(k);
                }
            }
        } else if (g!=v) discard(j);

        if (live[i]) {
            active.erase({priority[i],i});
            // A large response is much more typical for a zero endpoint
            // than for two random positive values. This is only a ranking;
            // no position is discarded on statistical grounds.
            evidence[i]+=log((double)g/N)+0.48;
            priority[i]=log((double)v)+evidence[i];
            active.emplace(priority[i],i);
        }
    };

    while (active.size()>2) {
        auto it=active.rbegin();
        int i=it->second,j=-1;
        for (++it;it!=active.rend();++it) {
            int k=it->second;
            bool seen=false;
            for (auto [u,h]:history[i]) if (u==k) {seen=true;break;}
            if (!seen) {j=k;break;}
        }
        // A live vertex cannot have two queried live neighbors: both
        // responses would have to equal L[i], ruling out i itself.
        int g=ask(label[i],label[j]);
        if (2*g>N) report(label[i],label[j]);
        update(i,j,g);
        update(j,i,g);
    }
    report(label[active.begin()->second],label[active.rbegin()->second]);
}
